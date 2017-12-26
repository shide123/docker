#include "CryptoHandler.h"

cryptohandler::cryptohandler(){
}

cryptohandler::~cryptohandler(){
}

int cryptohandler::base64Encode(const std::string& sInBuffer, std::string& sOutBuffer)
{
	//char *str, int str_len, char *encode, int encode_len;
	BIO *bmem,*b64;
	BUF_MEM *bptr;
	char *encode;
	b64=BIO_new(BIO_f_base64());
	bmem=BIO_new(BIO_s_mem());
	b64=BIO_push(b64,bmem);
	BIO_write(b64,sInBuffer.c_str(),sInBuffer.length()); //encode
	BIO_flush(b64);
	BIO_get_mem_ptr(b64,&bptr);
	if(bptr->length>sInBuffer.length()){
		//DPRINTF("encode_len too small\n");
		//return -1;
	}
	encode = new char[bptr->length + 1];
	memcpy(encode,bptr->data,bptr->length);
	encode[bptr->length] = '\0';
//  write(1,encode,bptr->length);
	BIO_free_all(b64);

	sOutBuffer = (char *)encode;
	delete [] encode;

	return bptr->length;
}

int cryptohandler::base64Decode(const std::string& sInBuffer, std::string& sOutBuffer)
{
	int len=0;
	BIO *b64,*bmem;
	b64=BIO_new(BIO_f_base64());
	bmem=BIO_new_mem_buf((void*)sInBuffer.c_str(),sInBuffer.length());
	bmem=BIO_push(b64,bmem);
	char *decode = new char[sInBuffer.length()+1];
	len=BIO_read(bmem,decode,sInBuffer.length());
	decode[len]=0;
	BIO_free_all(bmem);
	sOutBuffer = (char*)decode;

	delete [] decode;
	return 0;
}

void cryptohandler::md5hash(const std::string& sInBuffer, std::string& sOutBuffer)
{
	MD5_CTX ctx;
	const char *data=sInBuffer.c_str();
	unsigned char md[16];
	char buf[33]={'\0'};
	char tmp[3]={'\0'};
	int i;

	MD5_Init(&ctx);
	MD5_Update(&ctx,data,strlen(data));
	MD5_Final(md,&ctx);

	for( i=0; i<16; i++ ){
		sprintf(tmp,"%02X",md[i]);
		strcat(buf,tmp);
	}
	//printf("%s\n",buf);

	sOutBuffer = string(buf);
}

void cryptohandler::rc4Encrypt(const std::string& sInBuffer, std::string& sOutBuffer, const std::string& sKey)
{
	RC4_KEY rc4key;
	unsigned char* tmp = new unsigned char[sInBuffer.length() + 1];
	memset(tmp, 0, sInBuffer.length() + 1);

	RC4_set_key(&rc4key, sKey.length(), (const unsigned char*)sKey.c_str());
	RC4(&rc4key, sInBuffer.length(), (const unsigned char*)sInBuffer.c_str(), tmp);

	sOutBuffer = (char*)tmp;

	delete [] tmp;
}

void cryptohandler::rc4Decrypt(const std::string& sInBuffer, std::string& sOutBuffer, const std::string& sKey)
{
	RC4_KEY rc4key;
	unsigned char* tmp = new unsigned char[sInBuffer.length() + 1];
	memset(tmp, 0, sInBuffer.length() + 1);

	RC4_set_key(&rc4key, sKey.length(), (const unsigned char*)sKey.c_str());
	RC4(&rc4key, sInBuffer.length(), (const unsigned char*)sInBuffer.c_str(), tmp);

	sOutBuffer = (char*)tmp;

	delete [] tmp;
}

int cryptohandler::AESEncrypt(const std::string& sInBuffer, std::string& sOutBuffer, const std::string& sKey)
{
	AES_KEY aes_key;
	int key_ret = 0;
	if((key_ret = AES_set_encrypt_key((const unsigned char*)sKey.c_str(), sKey.length() * 8, &aes_key)) < 0)
	{
		//printf("error:AESEncrypt %d\n", key_ret);
		return key_ret;
	}

	std::string data_bak = sInBuffer;
	unsigned int data_length = data_bak.length();
	int padding = 0;
	if (data_bak.length() % AES_BLOCK_SIZE > 0)
	{
		padding =  AES_BLOCK_SIZE - data_bak.length() % AES_BLOCK_SIZE;
	}
	data_length += padding;
	while (padding > 0)
	{
		data_bak += '\0';
		padding--;
	}
	for(unsigned int i = 0; i < data_length/AES_BLOCK_SIZE; i++)
	{
		std::string str16 = data_bak.substr(i*AES_BLOCK_SIZE, AES_BLOCK_SIZE);
		unsigned char out[AES_BLOCK_SIZE];
		memset(out, 0, AES_BLOCK_SIZE);
		AES_encrypt((const unsigned char*)str16.c_str(), out, &aes_key);
		sOutBuffer += std::string((const char*)out, AES_BLOCK_SIZE);
	}
	return 0;
}

int cryptohandler::AESDecrypt(const std::string& sInBuffer, std::string& sOutBuffer, const std::string& sKey)
{
	AES_KEY aes_key;
	int key_ret = 0;
	if((key_ret = AES_set_decrypt_key((const unsigned char*)sKey.c_str(), sKey.length() * 8, &aes_key)) < 0)
	{
		//printf("error:AESDecrypt [%d]\n", key_ret);
		return key_ret;
	}

	for(unsigned int i = 0; i < sInBuffer.length()/AES_BLOCK_SIZE; i++)
	{
		std::string str16 = sInBuffer.substr(i*AES_BLOCK_SIZE, AES_BLOCK_SIZE);
		unsigned char out[AES_BLOCK_SIZE];
		memset(out, 0, AES_BLOCK_SIZE);
		AES_decrypt((const unsigned char*)str16.c_str(), out, &aes_key);
		sOutBuffer += std::string((const char*)out, AES_BLOCK_SIZE);
	}

	return 0;
}

void cryptohandler::tokenEncrypt(const std::string& sInBuffer, std::string& sOutBuffer, const std::string& sKey)
{
	int source_length = sInBuffer.length();
	int pass_length = sKey.length();

	char* tmp_str = (char*)malloc((source_length + 1) * sizeof(char));
	memset(tmp_str, 0, source_length + 1);

	for(int i = 0; i < source_length; ++i)
	{
		tmp_str[i] = sInBuffer.c_str()[i]^sKey.c_str()[i%pass_length];
		if(tmp_str[i] == 0)              // 要考虑到XOR等于0的情况，如果等于0，就相当
		{                                // 于字符串就提前结束了， 这是不可以的。
			tmp_str[i] = sInBuffer.c_str()[i];      // 因此tmp_str[i]等于0的时候，保持原文不变
		}
	}
	tmp_str[source_length] = 0;

	sOutBuffer =  tmp_str;

	free(tmp_str);
}

void cryptohandler::getRandSeq(std::string& sOutBuffer, int length)
{
	sOutBuffer = "";
	int i = 0;
	char c;
	if(0 == length){
		return;
	}
	for(i = 0; i < length; i++)
	{
		int rand_num = (rand() % 10);
		c = '0' + rand_num;
		sOutBuffer += c;
	}
}
