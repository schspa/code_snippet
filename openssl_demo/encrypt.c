/**
 * OpenSSL RSA encrypt and decrpyt demo.
 */

#include <stdio.h>
#include <string.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/err.h>

#define BUF_SIZE   1024

#define SSL_RSA_FILE     "rk_rsa"
#define SSL_RSA_PUB_FILE "rk_rsa.pub"
#define RK_ERM_KEY_FILE  "rk_erm.key"

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
	char *str = NULL;
	char str_en[BUF_SIZE] = {0};

	if (argc < 2) {
		printf("Usage: %s <string to encrypt/decrypt>\n", argv[0]);
		return -1;
	}

	str = argv[1];
	printf("1-String: strlen=%ld, str=%s\n", strlen(str), str);

	rsa_priv = create_rsa_from_file(SSL_RSA_FILE, 0);
	if (rsa_priv == NULL) {
		fprintf(stderr, "ERROR: Get ssl rsa pub from file error, please run \"command.sh\" first!\n");
		return -1;
	}

	ret = RSA_private_encrypt(strlen(str), (unsigned char *)str, (unsigned char *)str_en, rsa_priv, RSA_PKCS1_PADDING);
	if (ret < 0) {
		ERR_print_errors_fp(stderr);
		goto end;
	}
	printf("2-Encrypt: strlen=%ld\n", strlen(str_en));

	/* Write encrypt string into file */
	FILE *fp = fopen(RK_ERM_KEY_FILE, "w");
	ret = fwrite(str_en, 1, strlen(str_en), fp);
	printf("Write %d bytes into file: %s, encrypt done!\n", ret, RK_ERM_KEY_FILE);
	fclose(fp);

end:
	if (rsa_priv)
		RSA_free(rsa_priv);

	return 0;
}
