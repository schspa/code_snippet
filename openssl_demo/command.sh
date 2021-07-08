#!/bin/bash

function generate_key()
{
	# Generate RSA keys
	openssl genrsa -out rk_rsa 1024

	# Extra RSA public keys from test.key
	openssl rsa -in rk_rsa -pubout -out rk_rsa.pub
}

function encrypt()
{
	echo "Hello World! $(date +"%Y/%m/%d %H:%M:%S")" > test.txt

	# Encrypt with Public Key
	openssl rsautl -encrypt -in test.txt -inkey rk_rsa.pub -pubin -out test_en.txt

	# Dectype with Private Key
	openssl rsautl -decrypt -in test_en.txt -inkey rk_rsa -out test_de.txt
}

function signature()
{
	echo "Hello World! $(date +"%Y/%m/%d %H:%M:%S")" > test.txt

	# Encrypt with Private Key
	openssl rsautl -sign -in test.txt -inkey rk_rsa -out test_en2.txt

	# Dectype with Public Key
	openssl rsautl -verify -in test_en2.txt -inkey rk_rsa.pub -pubin -out test_de2.txt
}

############
# main
generate_key
#encrypt
#signature
