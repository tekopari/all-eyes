#!/bin/bash
#
# Ravi Jagannathan
#
# Below link was used to write this shell script.
# https://help.ubuntu.com/8.04/serverguide/certificates-and-security.html
#
# NOTE:  All pass phrasae is "AllEyes"
mkdir -p cert/private
#
openssl req -new -x509 -keyout cert/private/cakey.pem -out cert/cacert.pem -days 7000 -config caconfig.cnf
openssl req -nodes -new -x509 -keyout serverkey.pem -out serverreq.pem -days 7000 -config caconf.cnf
openssl x509 -x509toreq -in serverreq.pem -signkey serverkey.pem -out servercertreq.pem
#
#
## Export the cnf file
#export OPENSSL_CONF=./caconfig.cnf
#echo ""
#echo "Begin:  Creating a self-signed certificate"
#echo ""
##
## Generate keyes for Certificate Signing Request (CSR)
#openssl genrsa -des3 -out server.key 1024
#echo ""
#echo "openssl-genrse:  Return Code: $?"
#echo ""
## Create insecure key to create CSR
#openssl rsa -in server.key -out server.key.insecure
#echo ""
#echo "openssl-creating insecure key:  Return Code: $?"
#echo ""
## Create the CSR
#openssl req -new -key server.key -out server.csr
#echo ""
#echo "openssl-CSR creation:  Return Code: $?"
#echo ""
## The below command will create a self-signed certificate
#echo ""
#echo "CREATING A SELF-SIGNED CERTIFICATE"
#echo "openssl-generate x509:  THIS COMMAND WILL ASK FOR PASSPHRASE!!!"
#echo ""
#openssl x509 -req -days 365 -in server.csr -signkey server.key -out server.crt
#echo "openssl-generate x509:  Return code: $?"
##
##
## To install the certificate, use the below commands.
## But we do not install it for the entire host.
## We are going to use it just for this application.
## Commands
## ========
##    sudo cp server.crt /etc/ssl/certs
##    sudo cp server.key /etc/ssl/private
