# Key Management Tool #
The All-Eyes project uses X.509 certificates for securing communication likes between services.  In order to facilitate this we needed software to manage our certificates and private keys.  The Portecle project is an open source project that provides a simple interface for performing this function.

## Features ##
  * Create, load, save, and convert keystores
  * Generate RSA key pair entries with self-signed certificates
  * Import X.509 certificate files as trusted certificates
  * Import key pairs from PKCS #12 files.
  * Clone and change the password of key pair entries and keystores.
  * View the details of certificates
  * Export the keystore entries in a variety of formats.
  * Generate and view certification requests
  * Import Certificate Authority (CA) replies.
  * Change the password of key pair entries and keystores.
  * Delete, clone, and rename keystore entries.
  * View the details of certificate revocation list (CRL) files.

## Prerequisites ##
Portecle is Java based and has a prerequisite that Java SE 6
or later is installed on your machine.

Installation
  * Change directory into your Ubuntu home directory:
> > cd
  * Download the Portecle software with the command:
> > wget http://sourceforge.net/projects/portecle/files/latest/download -O portecle-1.7.zip
  * Unzip the Portecle software:
> > unzip portecle-1.7.zip
  * Edit your ./.bashrc and add the following line to it:
> > alias portecle='(cd ~/portecle-1.7; java -cp ./portecle.jar:./bcprov.jar net.sf.portecle.FPortecle)&'

## Running the tool ##
  * Activate changes by logging in to the shell (i.e. su {username})
  * Type the alias “portecle” to invoke the tool

**///TBRT\_NOTE.   Require details on creating the key files. ////**

## Additional details ##
  * http://portecle.sourceforge.net/
  * http://sourceforge.net/projects/portecle/
  * Readme file in the portecle-1.7 directory