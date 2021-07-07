/**
 * OpenSSL RSA encrypt and decrpyt demo.
 */

#include <stdio.h>
#include <string.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/err.h>

#define BUF_SIZE   1024

RSA *create_rsa_from_file(const char *key_file, int is_public)
{
	RSA *rsa = NULL;
	FILE *fp = NULL;

	fp = fopen(key_file, "rb");
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
	int ret = 0;
	char str[BUF_SIZE] = {"hello world"};
	char str_en[BUF_SIZE] = {0};

	rsa_priv = create_rsa_from_file("test.key", 0);
	if (rsa_priv == NULL) {
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

	FILE *fp = fopen("./hello_en.txt", "w+");
	fwrite(str_en, 1, strlen(str_en), fp);
	fclose(fp);

end:
	if (rsa_priv)
		RSA_free(rsa_priv);

	return 0;
}
