#!/bin/bash
sudo openssl s_client -connect 127.0.0.1:6000 -cert ./clientcert81.pem -CAfile ./cacert81.pem -state -debug

