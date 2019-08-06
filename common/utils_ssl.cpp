#include "utils_ssl.h"

#include <stdio.h>
#include <string.h>
#include <string>
#include <stdexcept>

#include <glog/logging.h>
#include <openssl/md5.h>
#include <openssl/aes.h>
#include <openssl/bio.h>
#include <openssl/pem.h>

using namespace std;

namespace {
	// aes-128-cbc key, 16 bytes
	const string kAesKey = "3adf43tweaet4362";
} // namespace

string Utils::getStrMd5(const string& str)
{
	unsigned char buff[16] = { 0 };
	MD5_CTX ctx;
	MD5_Init(&ctx);
	MD5_Update(&ctx, str.data(), str.size());
	MD5_Final(buff, &ctx);

	// Transfer to hex string { 0x01, 0xab, 0x4f } => "01ab4f"
	char hex[33] = { 0 };
	for (int i = 0; i < 16; i++) {
		::sprintf(hex + i * 2, "%02x", buff[i]);
	}

	return string(hex);
}

int Utils::toBase64(const char* in, size_t len, char** out)
{
	BIO* b64;
	BIO* bio;
	BUF_MEM* bptr = NULL;
	size_t size = 0;
	
	if (in == NULL)
		return -1;
	
	b64 = BIO_new(BIO_f_base64());
	BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

	bio = BIO_new(BIO_s_mem());
	b64 = BIO_push(b64, bio);

	BIO_write(b64, in, len);
	BIO_flush(b64);

	BIO_get_mem_ptr(b64, &bptr);
	size = bptr->length;

	// !!! Need to be released by user
	char* buff = (char*)::malloc(size + 1);
	::memset(buff, 0, size + 1);

	memcpy(buff, bptr->data, bptr->length);
	*out = buff;

	BIO_free_all(b64);
	return size;
}

int Utils::fromBase64(const char* in, size_t len, char** out)
{
	BIO* b64;
	BIO* bio;
	BUF_MEM* bptr = NULL;
	int size = 0;

	if (in == NULL)
		return -1;

	// !!! Need to be released by user
	char* buff = (char*)::malloc(len + 1);
	::memset(buff, 0, len + 1);

	b64 = BIO_new(BIO_f_base64());
	BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

	bio = BIO_new_mem_buf(in, len);
	b64 = BIO_push(b64, bio);

	size = BIO_read(b64, buff, len);
	*out = buff;

	BIO_free_all(b64);
	return size;
}

string Utils::aesEncToBase64(const string& str)
{
	size_t outLen;
	if (str.size() % AES_BLOCK_SIZE == 0) {
		outLen = str.size();
	}
	else {
		outLen = str.size() + AES_BLOCK_SIZE - str.size() % AES_BLOCK_SIZE;
	}
	
	unsigned char* aesOut = (unsigned char*)::malloc(outLen + 1);
	::memset(aesOut, 0, outLen + 1);
	unsigned char iv[AES_BLOCK_SIZE] = { 0 };

	AES_KEY key;
	AES_set_encrypt_key(reinterpret_cast<const unsigned char*>(kAesKey.data()),
		kAesKey.size() * 8, &key);
	AES_cbc_encrypt(reinterpret_cast<const unsigned char*>(str.data()), aesOut, str.size(),
		&key, iv, AES_ENCRYPT);

	// ToBase64
	char* base64;
	int ret = toBase64(reinterpret_cast<char*>(aesOut), outLen, &base64);

	string base64Str;
	if (ret >= 0) {
		base64Str = string(base64, ret);
	}

	if (base64 != NULL) {
		::free(base64);
	}
	if (aesOut != NULL) {
		::free(aesOut);
	}
	return base64Str;
}

string Utils::aesDecFromBase64(const string& str)
{
	string retStr;
	char* base64Out;
	unsigned char* aesOut;
	do {
		// From Base64
		int ret = fromBase64(str.data(), str.size(), &base64Out);
		if (ret < 0) {
			break;
		}

		aesOut = (unsigned char*)::malloc(ret + 1);
		::memset(aesOut, 0, ret + 1);
		unsigned char iv[AES_BLOCK_SIZE] = { 0 };

		AES_KEY key;
		AES_set_decrypt_key(reinterpret_cast<const unsigned char*>(kAesKey.data()),
			kAesKey.size() * 8, &key);
		AES_cbc_encrypt(reinterpret_cast<const unsigned char*>(base64Out), aesOut, 
			::strlen(base64Out), &key, iv, AES_DECRYPT);

		retStr = string(reinterpret_cast<char*>(aesOut));

	} while (0);
	
	if (base64Out != NULL) {
		::free(base64Out);
	}
	if (aesOut != NULL) {
		::free(aesOut);
	}
	return retStr;
}
