 #pragma once
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/md5.h>
#include <openssl/rc4.h>
#include <openssl/sha.h>
#include <openssl/aes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <iostream>
#include <iosfwd>
#include <string>

using namespace std;

class cryptohandler
{
public:
	cryptohandler();
	~cryptohandler();
	static void md5hash(const std::string& sInBuffer, std::string& sOutBuffer);
	static void rc4Encrypt(const std::string& sInBuffer, std::string& sOutBuffer, const std::string& sKey);
	static void rc4Decrypt(const std::string& sInBuffer, std::string& sOutBuffer, const std::string& sKey);
	static int AESEncrypt(const std::string& sInBuffer, std::string& sOutBuffer, const std::string& sKey);
	static int AESDecrypt(const std::string& sInBuffer, std::string& sOutBuffer, const std::string& sKey);
	static int base64Encode(const std::string& sInBuffer, std::string& sOutBuffer);
	static int base64Decode(const std::string& sInBuffer, std::string& sOutBuffer);
	static void tokenEncrypt(const std::string& sInBuffer, std::string& sOutBuffer, const std::string& sKey);
	static void getRandSeq(std::string& sOutBuffer, int length);
};
