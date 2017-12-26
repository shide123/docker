/*
 *  CApplePushProcessor.h
 *
 *  Created on: 2016年4月9日
 *      Author: zhangshaochen
 */

/*
 * IOS APNS version 3 (command=2) message format:
 * COMMAND|FRAMELEN|
 * ITEMID|ITEMLEN|TOKEN|
 * ITEMID|ITEMLEN|PAYLOAD|
 * ITEMID|ITEMLEN|IDENTIFIER|
 * ITEMID|ITEMLEN|EXPIRY|
 * ITEMID|ITEMLEN|PRIORITY|    
 * sizeof(uint8_t) + sizeof(uint32_t) + 
 * sizeof(uint8_t) + sizeof(uint16_t) + DEVICE_BINARY_SIZE + 
 * sizeof(uint8_t) + sizeof(uint16_t) + MAXPAYLOAD_SIZE + 
 * sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint32_t) + 
 * sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint32_t) + 
 * sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint8_t)
 * detail:https://developer.apple.com/library/ios/documentation/NetworkingInternet/Conceptual/RemoteNotificationsPG/Appendixes/BinaryProviderAPI.html#//apple_ref/doc/uid/TP40008194-CH106-SW8
 */

#ifndef __C_APPLE_PUSH_PROCESSOR_H__
#define __C_APPLE_PUSH_PROCESSOR_H__

#include <string>
#include <set>
#include <map>
#include "ssl_client.h"
#include "Config.h"

using namespace std;

class CApplePushProcessor
{
public:

	CApplePushProcessor();

	~CApplePushProcessor();
    
    bool Init(Config &cfg, const char* Main = "ApplePush");
    void initssl();
	int send_msg(const string & alert, const map<uint32, string> & token_set, Json::Value &extras_data);
	void handle_timeout();    
	
	inline char* build_frame_hdr(char* pPkt, size_t &pktlen, uint8_t command = 2, uint32_t frame_length = 0)
    {
        if (pPkt == NULL || pktlen < sizeof(ANPS_Frame_Hdr) + frame_length) return NULL;
        
        ANPS_Frame_Hdr * pFrameHdr = (ANPS_Frame_Hdr *)pPkt;
        pFrameHdr->command = command;
        pFrameHdr->frame_length = htonl(frame_length);
        
        pktlen = pktlen - sizeof(ANPS_Frame_Hdr);
        return pPkt + sizeof(ANPS_Frame_Hdr); 
    }
    
    template<typename DataType>
    inline char* build_item(uint8_t itemID, char* pPkt, size_t &pktlen, DataType *pData, uint16_t datalen = sizeof(DataType))
    {
        if (pPkt == NULL || pktlen < sizeof(ANPS_Item_Hdr) + datalen) return NULL;
        
        ANPS_Item_Hdr * pItemHdr = (ANPS_Item_Hdr *)pPkt;
        pItemHdr->itemID = itemID;   
        pItemHdr->item_data_length = htons(datalen);
        memcpy(pItemHdr->item_data, pData, datalen); 
        
        pktlen = pktlen - (sizeof(ANPS_Item_Hdr) + datalen);
        return pPkt + sizeof(ANPS_Item_Hdr) + datalen; 
    }
    
    inline char* build_item(uint8_t itemID, char* pPkt, size_t &pktlen, uint16_t *pData)
    {
        uint16_t datalen = sizeof(uint16_t);
        if (pPkt == NULL || pktlen < sizeof(ANPS_Item_Hdr) + datalen) return NULL;
        
        ANPS_Item_Hdr * pItemHdr = (ANPS_Item_Hdr *)pPkt;
        pItemHdr->itemID = itemID;   
        pItemHdr->item_data_length = htons(datalen);
        *((uint16_t *)pItemHdr->item_data) = htons(*pData);
        
        pktlen = pktlen - (sizeof(ANPS_Item_Hdr) + datalen);
        return pPkt + sizeof(ANPS_Item_Hdr) + datalen; 
    }
    
    inline char* build_item(uint8_t itemID, char* pPkt, size_t &pktlen, uint32_t *pData)
    {
        uint16_t datalen = sizeof(uint32_t);
        if (pPkt == NULL || pktlen < sizeof(ANPS_Item_Hdr) + datalen) return NULL;
        
        ANPS_Item_Hdr * pItemHdr = (ANPS_Item_Hdr *)pPkt;
        pItemHdr->itemID = itemID;   
        pItemHdr->item_data_length = htons(datalen);
       *((uint32_t *)pItemHdr->item_data) = htonl(*pData);
        
        pktlen = pktlen - (sizeof(ANPS_Item_Hdr) + datalen);
        return pPkt + sizeof(ANPS_Item_Hdr) + datalen; 
    }
                       

    bool APNSsendPayload_v2(const char *deviceToken, const char *deviceTokenBinary, const char *payloadBuff, size_t payloadLength, uint32_t identifier, uint32_t ExpiryEpochUTC, uint8_t priority = 10);
	
    string& get_password()
    {
         return m_password;
    } 

private:
       
	string m_time_to_live;
	
	string m_password;
	
	unsigned int m_identifier;
	
	char m_priority;
	
	boost::shared_ptr<ssl_client> m_sslPtr;
	boost::mutex m_ssl_client_mutex_;  
	string m_certPath;
	string m_keyPath; 
	string m_ip;
	string m_port;
	int m_Max_sendBuffList;
	boost::asio::io_service *m_io_service;

};

#endif /* __C_APPLE_PUSH_PROCESSOR_H__ */
