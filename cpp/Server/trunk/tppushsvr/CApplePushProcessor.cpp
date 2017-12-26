/*
 * CApplePushProcessor.cpp
 *
 *  Created on: 2016.4.9
 *      Author: root
 */

#include "CApplePushProcessor.h"
#include "AppPush.h"
#include "CLogThread.h"
#include "Config.h"
#include <boost/locale/conversion.hpp>
#include <boost/locale/encoding.hpp>


#define ANPS_MAXPAYLOAD_SIZE_V2     2048
#define ANPS_MAX_PKT_SIZE_V2        4096


deque<apns_sendBuff> ssl_client::m_deqSended;
deque<apns_sendBuff> ssl_client::m_deqSendList;
    
inline int Char2Int(char c)  
{  
    if (c >= '0'&& c <= '9')  
        return c - '0';  
    else if (c >= 'A' && c <= 'F')  
        return c - 'A' + 10;  
    else if (c >= 'a' && c <= 'f')  
        return c - 'a' + 10;  
    return -1;  
}  
inline bool HexString2AscString(const string& in, string& out)  
{  
    if (in.length() & 1)   
        return false;  
    static string ptn = "0123456789ABCDEFabcdef";  
    size_t found = in.find_first_not_of(ptn);  
    if (found != in.npos)  
        return false;   
    out.reserve(in.length() >> 1);  
    for (size_t pos = 0; pos < in.length(); pos+=2)  
        out.push_back((Char2Int(in[pos]) << 4) | Char2Int(in[pos+1]));   
    return true;  
}

CApplePushProcessor::CApplePushProcessor() 
    : m_identifier(0)
{	 
}

CApplePushProcessor::~CApplePushProcessor()
{
}

bool CApplePushProcessor::Init(Config &cfg, const char* Main)
{
    LOG_PRINT(log_info, "start:%s", Main);
    do
	{    
	    cfg.get(m_password, Main, "password");	
	    cfg.get(m_priority, Main, "priority");	
	    m_certPath = AppInstance()->getPath(PATH_TYPE_CONFIG) + cfg.getString(Main, "certFile");
		m_keyPath = cfg.getString(Main, "keyFile");
		if (!m_keyPath.empty()) m_keyPath = AppInstance()->getPath(PATH_TYPE_CONFIG) + m_keyPath;
	    cfg.get(m_ip, Main, "ip");	
	    cfg.get(m_port, Main, "port");	
	    cfg.get(m_time_to_live, Main, "time_to_live");	
	    cfg.get(m_Max_sendBuffList, Main, "Max_sendBuffList");		        
        if (m_certPath.empty() || m_ip.empty() || m_port.empty() 
			|| m_time_to_live.empty() || m_Max_sendBuffList == 0)
	    {
	        LOG_PRINT(log_error, "incorrect config");
	        break;
	    }
        m_io_service = &Application::get_io_service();
    	Timer::getInstance()->add(1, boost::bind(&CApplePushProcessor::handle_timeout, this), TIMES_UNLIMIT);
    	
    	return true;
    } while (0);
	return false;
}

void CApplePushProcessor::initssl()
{
    try {
        
        boost::asio::ip::tcp::resolver resolver(*m_io_service);
        boost::asio::ip::tcp::resolver::query query(m_ip.c_str(), m_port.c_str());
        boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);
        boost::asio::ssl::context ctx(boost::asio::ssl::context::tlsv1_client);
        //boost::asio::ssl::context ctx(boost::asio::ssl::context::tlsv12_client);
		if (!m_password.empty())
	        ctx.set_password_callback(boost::bind(&CApplePushProcessor::get_password, this));      
        ctx.use_certificate_file(m_certPath.c_str(), boost::asio::ssl::context::pem);
        ctx.use_rsa_private_key_file(m_certPath.c_str(), boost::asio::ssl::context::pem);
		if (!m_keyPath.empty())
	        ctx.use_private_key_file(m_keyPath.c_str(), boost::asio::ssl::context::pem);
        boost::mutex::scoped_lock lock(m_ssl_client_mutex_);    
        m_sslPtr.reset(new ssl_client(*m_io_service, ctx, iterator, m_Max_sendBuffList));
        m_sslPtr->async_connect();
    } catch(std::exception& e) {
        LOG_PRINT(log_error, "resolve hostname(%s) catch error, [%s]..", m_ip.c_str(), e.what());
    }

        
}

int CApplePushProcessor::send_msg(const string & alert, const map<uint32, string> & token_set, Json::Value &extras_data)
{
    if (token_set.empty()) return -1;
    
    Json::Value aps;
    aps["alert"] = Json::Value(alert);
    aps["sound"] = Json::Value("default");
    aps["badge"] = Json::Value(1);

    Json::Value iosData(extras_data);
    iosData["aps"] = aps;
    
    Json::FastWriter fast_writer;
	std::string payload = fast_writer.write(iosData);  
	                
    std::string token;
    std::map<uint32, std::string>::const_iterator iter = token_set.begin();
    uint32_t ExpiryEpochUTC = time(0) + atoi(m_time_to_live.c_str());
    LOG_PRINT(log_info, "Apple payload:%s", payload.c_str());   
    for (; iter != token_set.end(); ++iter)
    {
		if (!m_sslPtr->checkToken(iter->second))
		{
			LOG_PRINT(log_info, "token[%s] is invalid, continue.", iter->second.c_str());
			continue;
		}
        ++m_identifier;
        string deviceTokenBinary;
        HexString2AscString(iter->second, deviceTokenBinary);
        boost::mutex::scoped_lock lock(m_ssl_client_mutex_);   
        if(!APNSsendPayload_v2(iter->second.c_str(), deviceTokenBinary.c_str(), payload.c_str(), payload.size(), m_identifier, ExpiryEpochUTC, m_priority))
        {
            LOG_PRINT(log_error, "wrong payload:%s", payload.c_str());
        }                   
    } 
	return 0;
}

bool CApplePushProcessor::APNSsendPayload_v2(const char *deviceToken, const char *deviceTokenBinary, const char *payloadBuff, size_t payloadLength, uint32_t identifier, uint32_t ExpiryEpochUTC, uint8_t priority)
{
    if (!m_sslPtr || !deviceTokenBinary || !payloadBuff || !payloadLength) return false;
        
    if (payloadLength > ANPS_MAXPAYLOAD_SIZE_V2)
        return false;
    char binaryMessageBuff[ANPS_MAX_PKT_SIZE_V2]; 
    char *binaryMessagePt = binaryMessageBuff;
    size_t freelen = sizeof(binaryMessageBuff);
    
    binaryMessagePt = build_frame_hdr(binaryMessagePt, freelen);
    if (!binaryMessagePt) return false;
    binaryMessagePt = build_item(1, binaryMessagePt, freelen, deviceTokenBinary, ANPS_DEVICE_BINARY_SIZE);
    if (!binaryMessagePt) return false;    
    binaryMessagePt = build_item(2, binaryMessagePt, freelen, payloadBuff, payloadLength);
    if (!binaryMessagePt) return false;
    binaryMessagePt = build_item(3, binaryMessagePt, freelen, &identifier);
    if (!binaryMessagePt) return false;
    binaryMessagePt = build_item(4, binaryMessagePt, freelen, &ExpiryEpochUTC);
    if (!binaryMessagePt) return false;
    binaryMessagePt = build_item(5, binaryMessagePt, freelen, &priority);
    if (!binaryMessagePt) return false;
    
    ANPS_Frame_Hdr * pFrameHdr = (ANPS_Frame_Hdr *)binaryMessageBuff;
    pFrameHdr->frame_length = htonl(binaryMessagePt - binaryMessageBuff - sizeof(ANPS_Frame_Hdr));
    
    m_sslPtr->async_send(identifier, deviceToken, binaryMessageBuff, (binaryMessagePt - binaryMessageBuff));

  return true;
}

void CApplePushProcessor::handle_timeout()
{
	if(!m_sslPtr || m_sslPtr->needReset())
	{
		LOG_PRINT(log_warning, "reset the link");
        initssl();
	}
}
    
