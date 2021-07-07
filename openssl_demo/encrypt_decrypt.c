/**
 * OpenSSL RSA encrypt and decrpyt demo.
 */

#include <stdio.h>
#include <string.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/err.h>

#define BUF_SIZE   1024

#if 0
#define KEY_LENGTH 2048
#define PUB_EXP    3

int create_rsa(int is_public)
{
	RSA *keypair = NULL;
	BIO *keybio = NULL;
	size_t key_len = 0;
	char *keystr = NULL;

	keypair = RSA_generate_key(KEY_LENGTH, PUB_EXP, NULL, NULL);

	if (is_public) {
		// Public Key
		keybio = BIO_new(BIO_s_mem());
		PEM_write_bio_RSAPublicKey(keybio, keypair);
		key_len = BIO_pending(keybio);

		keystr = malloc(key_len + 1);
		BIO_read(keybio, keystr, key_len);
		keystr[key_len] = '\0';

		printf("Public Key:%ld\n%s\n", strlen(keystr), keystr);
	} else {
		// Private Key
		keybio = BIO_new(BIO_s_mem());
		PEM_write_bio_RSAPrivateKey(keybio, keypair, NULL, NULL, 0, NULL, NULL);
		key_len = BIO_pending(keybio);

		keystr = malloc(key_len + 1);
		BIO_read(keybio, keystr, key_len);
		keystr[key_len] = '\0';

		printf("Private Key:%ld\n%s\n", strlen(keystr), keystr);
	}

	return 0;
}

int main(int argc, char *argv[])
{
	create_rsa(0);
	create_rsa(1);
	return 0;
}

#else

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
	RSA *rsa_pub = NULL;
	int ret = 0;
	char str[BUF_SIZE] = {"hello world"};
	char str_en[BUF_SIZE] = {0};
	char str_de[BUF_SIZE] = {0};

	rsa_priv = create_rsa_from_file("test.key", 0);
	rsa_pub = create_rsa_from_file("test_pub.key", 1);
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
#endif
