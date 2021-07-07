#!/bin/bash

function generate_key()
{
	# Generate RSA keys
	openssl genrsa -out test.key 1024

	# Extra RSA public keys from test.key
	openssl rsa -in test.key -pubout -out test_pub.key
}

function encrypt()
{
	echo "Hello World! $(date +"%Y/%m/%d %H:%M:%S")" > hello.txt

	# Encrypt with Public Key
	openssl rsautl -encrypt -in hello.txt -inkey test_pub.key -pubin -out hello.en.txt

	# Dectype with Private Key
	openssl rsautl -decrypt -in hello.en.txt -inkey test.key -out hello.de.txt
}

function signatue()
{
	echo "Hello World! $(date +"%Y/%m/%d %H:%M:%S")" > hello.txt

	# Encrypt with Public Key
	# Encrypt with Private Key
	openssl rsautl -sign -in hello.txt -inkey test.key -out hello.en2.txt

	# Dectype with Public Key
	openssl rsautl -verify -in hello.en2.txt -inkey test_pub.key -pubin -out hello.de2.txt
}

############
# main
generate_key
