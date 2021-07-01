/**
 * OpenSSL RSA encrypt and decrpyt demo.
 */

#include <stdio.h>
#include <string.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/err.h>

#define BUF_SIZE 1024

RSA *get_key_from_file(const char *key, int is_public)
{
	RSA *rsa = NULL;
	FILE *fp = NULL;

	fp = fopen(key, "rb");
	if (fp == NULL)
	{
		perror("fopen");
		goto end;
	}

	if (is_public)
	{
		rsa = PEM_read_RSA_PUBKEY(fp, NULL, NULL, NULL);
		if (rsa == NULL) {
			perror("PEM_read_RSA_PUBKEY");
			ERR_print_errors_fp(stderr);
			goto end;
		}
	} else {
		rsa = PEM_read_RSAPrivateKey(fp, NULL, NULL, NULL);
		if (rsa == NULL) {
			perror("PEM_read_RSAPrivateKey");
			ERR_print_errors_fp(stderr);
			goto end;
		}
	}

end:
	if (fp)
		fclose(fp);
	return rsa;
}

int main(int argc, char *argv[])
{
	RSA *rsa_priv = NULL;
	RSA *rsa_pub = NULL;
	int ret = 0;
	char str[BUF_SIZE] = {"hello world"};
	char str_en[BUF_SIZE] = {0};
	char str_de[BUF_SIZE] = {0};

	rsa_priv = get_key_from_file("test.key", 0);
	rsa_pub = get_key_from_file("test_pub.key", 1);
	if (rsa_priv == NULL || rsa_pub == NULL) {
		fprintf(stderr, "ERROR: Get key from file error, please run \"command.sh\" first!\n");
		return -1;
	}

	printf("1-Original: ret=%d, strlen=%ld, str=%s\n", ret, strlen(str), str);

	ret = RSA_private_encrypt(strlen(str), (unsigned char *)str, (unsigned char *)str_en, rsa_priv, RSA_PKCS1_PADDING);
	if (ret < 0) {
		ERR_print_errors_fp(stderr);
		goto end;
	}
	printf("2-Encrypt: ret=%d, strlen=%ld\n", ret, strlen(str_en));

	ret = RSA_public_decrypt(strlen(str_en), (unsigned char *)str_en, (unsigned char *)str_de, rsa_pub, RSA_PKCS1_PADDING);
	if (ret < 0) {
		ERR_print_errors_fp(stderr);
		goto end;
	}
	printf("3-Decrpyt: ret=%d, strlen=%ld, str_de=%s\n", ret, strlen(str_de), str_de);

end:
	if (rsa_priv)
		RSA_free(rsa_priv);
	if (rsa_pub)
		RSA_free(rsa_pub);

	return 0;
}
