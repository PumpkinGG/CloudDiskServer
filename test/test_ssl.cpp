#include "utils_ssl.h"
#include <iostream>
#include <string.h>

using namespace std;
using namespace Utils;

int main(int argc, char* argv[])
{
	string md5 = Utils::getStrMd5("1234567");
	cout << md5 << endl;
	cout << ((md5 == "fcea920f7412b5da7be0cf42b8c93759") ? "Pass" : "Not pass") << endl;

	cout << "test base64" << endl;
	string str("12345shi一下啊");
	char* enc, * dec;
	int size = Utils::toBase64(str.data(), str.size(), &enc);
	cout << enc << " size: " << size << endl;
	
	size = Utils::fromBase64(enc, strlen(enc), &dec);
	cout << dec << " size: " << size << endl;

	string aesEncStr = Utils::aesEncToBase64(str);
	cout << "AES encryption: " << aesEncStr << endl;

	string aesDecStr = Utils::aesDecFromBase64(aesEncStr);
	cout << "AES decryption: " << aesDecStr << endl;

	free(enc);
	free(dec);
	return 0;
}