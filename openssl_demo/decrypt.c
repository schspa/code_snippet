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
	RSA *rsa_pub = NULL;
	int ret = 0;
	char str_en[BUF_SIZE] = {0};
	char str_de[BUF_SIZE] = {0};
	char *enc_file = NULL;

	if (argc < 2) {
		printf("Usage: %s <encrypt file>\n", argv[0]);
		return -1;
	}

	enc_file = argv[1];
	FILE *fp = fopen(enc_file, "r");
	if (fp == NULL) {
		perror("fopen");
		goto end;
	}
	
	/* read encryped string will return 0, but can get its content */
	ret = fread(str_en, sizeof(str_en), 1, fp);
	printf("1-String: ret=%d, strlen(str_en)=%ld\n", ret, strlen(str_en));

	rsa_pub = create_rsa_from_file(SSL_RSA_PUB_FILE, 1);
	if (rsa_pub == NULL) {
		fprintf(stderr, "ERROR: Get key from file error, please run \"command.sh\" first!\n");
		goto end;
	}

	ret = RSA_public_decrypt(strlen(str_en), (unsigned char *)str_en, (unsigned char *)str_de, rsa_pub, RSA_PKCS1_PADDING);
	if (ret < 0) {
		ERR_print_errors_fp(stderr);
		goto end;
	}
	printf("2-Decrpyt: ret=%d, strlen=%ld, str_de=%s\n", ret, strlen(str_de), str_de);

end:
	if (rsa_pub)
		RSA_free(rsa_pub);
	if (fp)
		fclose(fp);

	return 0;
}
