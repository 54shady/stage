#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>

char ss[SHA256_DIGEST_LENGTH] = {};
char *sha256(const char *str)
{
	unsigned char hash[SHA256_DIGEST_LENGTH];
	SHA256_CTX sha256;
	SHA256_Init(&sha256);
	SHA256_Update(&sha256, str, strlen(str));
	SHA256_Final(hash, &sha256);
	for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
	{
		printf("%x ", (int)hash[i]);
		ss[i] = (char)hash[i];
	}
	printf("\n");
	return ss;
}

/*
 * gcc sha256-demo.c `pkg-config --libs openssl`
 */
int main()
{

	sha256("test");
	sha256("test2");

	return 0;

}
