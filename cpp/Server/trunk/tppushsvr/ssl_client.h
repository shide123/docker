/*
 * ssl_client.h
 *
 *  Created on: 2016年4月9日
 *      Author: root
 */

#ifndef __C_SSL_CLIENT_H__
#define __C_SSL_CLIENT_H__

#include <pthread.h>
#include "CLogThread.h"
#include "json/json.h"
#include <cstdlib>  
#include <boost/bind.hpp>  
#include <boost/asio.hpp>  
#include <boost/asio/ssl.hpp>  
#include <boost/thread/mutex.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <queue>
#include <string>
using namespace std;

enum { max_length = 1024 };  

#pragma pack(1)
//苹果推送消息头 
struct ANPS_Frame_Hdr
{
	uint8   command;        //Populate with the number 2
    uint32  frame_length;   //The size of the frame data.
    char    frame_data[0];  //The frame contains the body, structured as a series of items.
};
//苹果推送帧头 
struct ANPS_Item_Hdr
{
	uint8   itemID;             //The item identifier
    uint16  item_data_length;   //The size of the item data.
    char    item_data[0];       //The value for the item.
};
//苹果推送应答
struct ANPS_Res
{
	uint8   command;    //
    uint8   status;     //error code 0~10 128,255
    uint32  identifier; //send by ourselves
};
#pragma pack()

#define ANPS_DEVICE_BINARY_SIZE  32
#define ANPS_MAXPAYLOAD_SIZE     512
#define ANPS_MAX_PKT_SIZE        1024

using namespace std;

/*
  For save the data which send to Apple.  
  the identifier is used to resend the pkt
*/
class apns_sendBuff
{ 
public:  
    apns_sendBuff()
        :m_identifier(0)
        ,m_data(NULL)
        ,m_len(0)
    {
        memset(m_deviceToken, 0, sizeof(m_deviceToken));
    }
    
    apns_sendBuff(uint32_t identifier)
        :m_identifier(identifier)
        ,m_data(NULL)
        ,m_len(0)
    {
        memset(m_deviceToken, 0, sizeof(m_deviceToken));
    }
    
    apns_sendBuff(uint32_t identifier, const char * deviceToken, const char * data, size_t len)
    :m_identifier(identifier)
    {
        m_data = new char[len];
        memcpy(m_data, data, len);
        strncpy(m_deviceToken, deviceToken, sizeof(m_deviceToken));
        m_len = len;
    }

	apns_sendBuff(const apns_sendBuff &other)
	{
		copyFrom(other);
	}
    
    ~apns_sendBuff()
    {
        if (m_data) delete [] m_data;
    }
    
    void init(uint32_t identifier, const char * deviceToken, const char * data, size_t len)
    {
        m_identifier = identifier;
		if (len && data)
		{
			m_len = len;
			m_data = new char[len];
			memcpy(m_data, data, len);
		}
		if (deviceToken)
	        strncpy(m_deviceToken, deviceToken, sizeof(m_deviceToken));
    }

	apns_sendBuff &operator=(const apns_sendBuff &other)
	{
		if (&other != this)
			copyFrom(other);

		return *this;
	}

	void copyFrom(const apns_sendBuff &other)
	{
		if (&other == this)
			return;

		if (other.m_len)
		{
			m_len = other.m_len;
			m_identifier = other.m_identifier;
			m_data = new char[m_len];
			memcpy(m_data, other.m_data, m_len);
			strncpy(m_deviceToken, other.m_deviceToken, sizeof(m_deviceToken));
		}
		else
		{
			init(0, NULL, NULL, 0);
		}
	}
     
    bool operator==(const apns_sendBuff & in) const
    {
        if (&in == this) return true;
        return in.m_identifier == m_identifier;        
    }
    bool operator<(const apns_sendBuff & in) const
    {
        if (&in == this) return false;
        return in.m_identifier<m_identifier;        
    }
    bool operator>(const apns_sendBuff & in) const
    {
        if (&in == this) return false;
        return in.m_identifier>m_identifier;        
    }
    
    char * buff()
    {
        return m_data;
    }
    
    size_t& size()
    {
        return m_len;
    }
    
    uint32_t& identifier()
    {
        return m_identifier;
    }
    
    uint32_t m_identifier;
    char m_deviceToken[65];
    char * m_data;
    size_t m_len;
};

class apns_error
{
public:
	apns_error(uint8 err_no):m_errno(err_no)
	{
		switch (m_errno)
		{
		case 0:
			m_error = "No errors encountered";
			break;
		case 1:
			m_error = "Processing error";
			break;
		case 2:
			m_error = "Missing device token";
			break;
		case 3:
			m_error = "Missing topic";
			break;
		case 4:
			m_error = "Missing payload";
			break;
		case 5:
			m_error = "Invalid token size";
			break;
		case 6:
			m_error = "Invalid topic size";
			break;
		case 7:
			m_error = "Invalid payload size";
			break;
		case 8:
			m_error = "Invalid token";
			break;
		case 10:
			m_error = "Shutdown";
			break;
		case 255:
			m_error = "None (unknown)";
			break;
		default:
			m_error = "unknown error";
			break;
		}
	}

	~apns_error(){}

	int err_no()
	{
		return m_errno;
	}
	const char *error()
	{
		return m_error;
	}
private:
	uint8 m_errno;
	const char *m_error;
};
/*
  An ssl client for connect the APNS , it is make special for resend the error APNS pkt,
  so don't use it for another ssl.  
  To deal with the reconnect the link and save the history pkt.
  Sometimes when the boost close ssl client and reconnect ,
  at the handshake step will get the "unknown alert type" error , not stop.
  For this use is_connected to notice the outside timer(CApplePushProcessor::handle_timeout) to restart this class
  The history pkt are save in the static m_apns_sendBuffList,m_apns_iter
*/
class ssl_client : public boost::enable_shared_from_this<ssl_client>
                 , private boost::noncopyable 
{  
public:  
	enum CONNECT_STATUS
	{
		e_disconnected = 0,
		e_connecting,
		e_handshaking,
		e_handshaked,
	};

    ssl_client( boost::asio::io_service& io_service,  
                boost::asio::ssl::context& context,  
                boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
                int Max_sendBuffList)  
    : socket_(io_service, context) 
    , m_Max_sendBuffList(Max_sendBuffList)    
    , endpoint_iterator_(endpoint_iterator)    
    , reconnect_callback_impl_(io_service)
	, recv_size(0)
    , is_post_reconnect_req_( false)   
    {  
        socket_.set_verify_mode(boost::asio::ssl::verify_peer);  
        socket_.set_verify_callback(  
        boost::bind(&ssl_client::verify_certificate, _1, _2)); 
		m_status = e_disconnected;
		m_reset_flag = false;
    }  
  
    static bool verify_certificate(bool preverified, boost::asio::ssl::verify_context& ctx)  
    {  
      // The verify callback can be used to check whether the certificate that is  
      // being presented is valid for the peer. For example, RFC 2818 describes  
      // the steps involved in doing this for HTTPS. Consult the OpenSSL  
      // documentation for more details. Note that the callback is called once  
      // for each certificate in the certificate chain, starting from the root  
      // certificate authority.  
    
      // normally, Apple will send a three size certificate chain,this fun will be called three times
        char subject_name[256];
        X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());

        X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
        LOG_PRINT(log_debug, " Verifying %s", subject_name);

        char issuer_name[256] = {};
        X509_NAME_oneline(X509_get_issuer_name(cert),issuer_name,256);
        LOG_PRINT(log_debug, " issuer name: %s", issuer_name);

        char peer_CN[256] = {0};
        X509_NAME_get_text_by_NID(X509_get_subject_name(cert), NID_commonName, peer_CN, 255);

        int err = X509_STORE_CTX_get_error(ctx.native_handle());
        if ( err == X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY || err == X509_V_ERR_CERT_UNTRUSTED )
        {
            LOG_PRINT(log_debug, "success");
            return true;           // this is ok!
        }

        LOG_PRINT(log_warning, "failed err=%d", err);
        return preverified;  
    }  

    void async_connect()
    {
        LOG_PRINT(log_debug, " start %lu", pthread_self());
		{
			boost::mutex::scoped_lock lock(m_status_mutex);
			if ( m_status > e_disconnected )
				return ;
			m_status == e_connecting;
		}
        
        boost::asio::ip::tcp::resolver::iterator  endpoint_iterator = endpoint_iterator_;
        boost::asio::async_connect(socket_.lowest_layer(),endpoint_iterator,
            boost::bind(&ssl_client::handle_connect, shared_from_this(),
            boost::asio::placeholders::error, ++endpoint_iterator));
    }
      
    void handle_connect(const boost::system::error_code& error, boost::asio::ip::tcp::resolver::iterator endpoint_iterator)  
    {
        boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
        if (!error)  
		{
            LOG_PRINT(log_info, " success %lu", pthread_self());
            socket_.async_handshake(boost::asio::ssl::stream_base::client,  
                boost::bind(&ssl_client::handle_handshake, shared_from_this(),  
				boost::asio::placeholders::error));	
			set_status(e_handshaking);
        }
		else if (boost::asio::error::already_started == error)
		{
			LOG_PRINT(log_info, " connect to %s:%u error:[%s],", endpoint.address().to_string().c_str(), endpoint.port(), boost::system::system_error(error).what());
		}
        else if (endpoint_iterator != boost::asio::ip::tcp::resolver::iterator())
        {     
			LOG_PRINT(log_info, " connect to %s:%u failed:[%s], try another", endpoint.address().to_string().c_str(), endpoint.port(), boost::system::system_error(error).what());
            socket_.lowest_layer().async_connect(endpoint,
                boost::bind(&ssl_client::handle_connect, shared_from_this(),
                boost::asio::placeholders::error, ++endpoint_iterator));
        }
        else
        {
			set_reset_flag();
            LOG_PRINT(log_error, "ssl_client connect failed %s, need reset %lu", boost::system::system_error(error).what(), pthread_self());  
        }
          
    }  
  
    void handle_handshake(const boost::system::error_code& error)  
    {  
        if ( !error )
        {
            LOG_PRINT(log_info, " success %lu", pthread_self());

            set_status(e_handshaked);
			boost::mutex::scoped_lock lock(m_mtxSendList);
			if (!m_deqSendList.empty())
			{
				async_send(m_deqSendList.front().buff(), m_deqSendList.front().size());
			}

			async_recv();
        }
        else
        {
            LOG_PRINT(log_error, "%s %lu", boost::system::system_error(error).what(), pthread_self());
            disconnect();
			set_reset_flag();
        }
    }
   
    bool async_send(const char* szbuffer ,boost::int32_t buffer_size)
    {
        
        if ( !szbuffer || buffer_size  <= 0)
            return false;
        if (e_disconnected == m_status) 
        {
            LOG_PRINT(log_info, " handshake is not ok set_reconnect_handler %lu", pthread_self());
            set_reconnect_handler();
        }
        else
        {
            LOG_PRINT(log_info, " start %lu", pthread_self());
            boost::asio::async_write(socket_ ,
                boost::asio::buffer(szbuffer/*request_*/, buffer_size),
                boost::bind(&ssl_client::handle_write, shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
        }
        return true;
    }
    
    void handle_write(const boost::system::error_code& error, size_t bytes_transferred)
    {
        if (!error)
        {   
            LOG_PRINT(log_info, " start %lu", pthread_self());
            boost::mutex::scoped_lock lock(m_mtxSendList);
			if (m_deqSendList.empty())
			{
				LOG_PRINT(log_warning, "m_deqSendList is empty.");
				return;
			}
			m_deqSended.push_back(m_deqSendList.front());
			m_deqSendList.pop_front();

			//clean old data
			if (m_Max_sendBuffList > 0 && m_deqSendList.size() > m_Max_sendBuffList)
			{
				int count = 0;
				do
				{
					m_deqSendList.pop_front();
					count++;
				}while (m_deqSendList.size() > m_Max_sendBuffList);

				LOG_PRINT(log_error, "sendBuffList size %d is so large, pop count %d.", m_deqSendList.size() + count, count);
			}

            if (!m_deqSendList.empty())
                async_send(m_deqSendList.front().buff(), m_deqSendList.front().size());
        }
        else
        {
            LOG_PRINT(log_error, "%s %lu", boost::system::system_error(error).what(), pthread_self());
            set_reconnect_handler();
        }
    }
    
    bool async_send(uint32_t identifier, const char *deviceToken, const char* szbuffer ,boost::int32_t buffer_size)
    {
        
        if ( !szbuffer || buffer_size  <= 0)
            return false;
        boost::mutex::scoped_lock lock(m_mtxSendList); 
        bool IsWriting = !m_deqSendList.empty();
        m_deqSendList.push_back(apns_sendBuff() );
        m_deqSendList.back().init(identifier, deviceToken, szbuffer, buffer_size);
        
        if (!IsWriting) 
            async_send(m_deqSendList.front().buff(), m_deqSendList.front().size());

        return true;
    }

    bool async_recv()
    {
        socket_.async_read_some(
            boost::asio::buffer(reply_, sizeof(reply_) - recv_size),
            boost::bind(&ssl_client::handle_read, shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));

        return true;
    } 
  
    void handle_read(const boost::system::error_code& error,
        size_t bytes_transferred)
    {
        do
        {
            if ( error || !bytes_transferred)
            {
				LOG_PRINT(log_error, "handle_read error: %s %lu", boost::system::system_error(error).what(), pthread_self());     
                break;
            }

			recv_size += bytes_transferred;

			if (recv_size < 6)
			{
				async_recv();
				return;
			}
			else // apple response
            {
                ANPS_Res *pRes = (ANPS_Res *)reply_;
				LOG_PRINT(log_error, "read failed res, command:%d status:%d error:[%s] identifier:%u %lu", (int)pRes->command, (int)pRes->status, apns_error(pRes->status).error(), ntohl(pRes->identifier), pthread_self());
                boost::mutex::scoped_lock lock(m_mtxSendList);
                
                deque<apns_sendBuff>::iterator iter;                      
                iter = find(m_deqSended.begin(), m_deqSended.end(), apns_sendBuff(ntohl(pRes->identifier)));
                
                //if found the error pkt, change the send iter to the ,error pkt next one
                if (iter != m_deqSended.end())
                {
                    LOG_PRINT(log_error, "found the failed token %s %lu", iter->m_deviceToken, pthread_self());
					if (8 == pRes->status)
						m_sInvalidToken.insert(iter->m_deviceToken);
                    
                    //delete the pkt before the error pkt,they all send success
					m_deqSended.erase(m_deqSended.begin(), ++iter);
					if (!m_deqSended.empty())
					{
						bool needSend = true;
						deque<apns_sendBuff>::iterator itInsert = m_deqSendList.begin();
						if (!m_deqSendList.empty())
						{
							needSend = false;
							itInsert++;
						}

						m_deqSendList.insert(itInsert, m_deqSended.begin(), m_deqSended.end());
						m_deqSended.clear();
					}
                }
            }
        }while(false);

        set_reconnect_handler();
    }  

    void set_reconnect_handler()
    {
        LOG_PRINT(log_debug, " start %lu", pthread_self());
        
        if ( !set_post_reconnect_req( true ) ) {
            return ;
        }
        LOG_PRINT(log_debug, " enter %lu", pthread_self());
        reconnect();
//        reconnect_callback_impl_.expires_from_now(boost::posix_time::seconds(5));
//        reconnect_callback_impl_.async_wait(boost::bind(&ssl_client::reset_reconnect_flag,
//            shared_from_this(), boost::asio::placeholders::error));
    }
    
    void reset_reconnect_flag(const boost::system::error_code& error )
    {
        if (e_disconnected == m_status) return;
        LOG_PRINT(log_debug, " start %lu", pthread_self());
        set_post_reconnect_req( false );
        if (is_handshake_ok()) return;
        LOG_PRINT(log_debug, " enter %lu", pthread_self());  
        if ( error != boost::asio::error::operation_aborted )       // Timer was not cancelled, take necessary action.
        {
            reconnect();
        }
        else
        {
             LOG_PRINT(log_error, "%s %lu", boost::system::system_error(error).what(), pthread_self()); 
        }
    }
    
    void disconnect()
    {
        try
        {
            LOG_PRINT(log_info, " disconnect %lu", pthread_self());
            socket_.lowest_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both);
            socket_.lowest_layer().close();
			set_status(e_disconnected);
            usleep(100*1000);
        }
        catch (std::exception* e)
        {
			set_reset_flag();
            LOG_PRINT(log_error, "%s", string(e->what()).c_str());
        }
        catch(boost::exception& e)
        {
			set_reset_flag();
            LOG_PRINT(log_error, "%s", string(boost::diagnostic_information(e)).c_str());
        }
    }

    void reconnect()
    {
        boost::mutex::scoped_lock lock(m_reconnect_mutex_);
        LOG_PRINT(log_info, " reconnect %lu", pthread_self());
        disconnect();
        async_connect();
		set_post_reconnect_req( false );
    }
   
	bool checkToken(std::string strToken)  { return m_sInvalidToken.count(strToken) == 0; }
	bool is_disconnected()
	{
		return (e_disconnected == m_status);
	}
    bool is_handshake_ing()
	{
		return (e_handshaking == m_status);
	}
    bool is_handshake_ok()
	{
		return (e_handshaked == m_status);
	}
	void set_status(CONNECT_STATUS s)
	{
		boost::mutex::scoped_lock lock(m_status_mutex);
		m_status = s;

	}

    bool set_post_reconnect_req(bool b){
		boost::mutex::scoped_lock lock(m_set_reconnect_mutex_);
		if (b == is_post_reconnect_req_)
			return false;

		is_post_reconnect_req_ = b;
		return true;
	}

	bool needReset()
	{
		return m_reset_flag;
	}

    
private:  
	bool set_reset_flag()
	{
		m_reset_flag = true;
	}

    boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_;
    boost::asio::ip::tcp::resolver::iterator    endpoint_iterator_;
    boost::asio::deadline_timer reconnect_callback_impl_;
               
    //save the pkt send to APNS,
    //use to resend the pkt. When APNS return the error,APNS will throw all data send after the error pkt;
	static deque<apns_sendBuff> m_deqSended;
	static deque<apns_sendBuff> m_deqSendList;
	//Point to the pkt,which current need send;
	boost::mutex m_mtxSendList;
	int m_Max_sendBuffList;
	boost::mutex m_reconnect_mutex_;

	CONNECT_STATUS m_status;
	boost::mutex m_status_mutex;
                
    char reply_[max_length];            //only get error pkt,APNS will send the reply,then close the link
	int recv_size;

	boost::mutex m_set_reconnect_mutex_;
    bool    is_post_reconnect_req_;     // 已经投递了重连请求

	std::set<std::string> m_sInvalidToken;
	bool m_reset_flag;
};


#endif /* __C_SSL_CLIENT_H__ */  
