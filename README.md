otpcrypt
========
A command line tool for hardware-accelerated AES encryption/decryption on iMX233 using the key stored in OTP-ROM.

Compilation
-------------
On your board (e.g. iMX233-OLinuxino) compile and install the program:

    # make
    # make install

Usage
---------
There are a few command line parameters for otpcrypt:

    -c         Work in CBC mode instead of ECB mode
    -d         Perform decryption instead of encryption
    -i <IV>    Use the specified CBC mode hex IV (Init Vector). default is 00000000000000000000000000000000.
    -h         Get help

Examples
----------
Encrypt/Decrypt data using AES128 CBC using the specified IV and the key in OTP:

    # echo Pomegranate | otpcrypt -c -i 48C56AA1FAF6D6F6C9E80170234D9B57 | base64
    99UWQqU9Jj68GNPWBTBD2g==

    # echo 99UWQqU9Jj68GNPWBTBD2g== | base64 -d | otpcrypt -c -d -i 48C56AA1FAF6D6F6C9E80170234D9B57
    Pomegranate

    # echo Pomegranate | otpcrypt -c -i 48C56AA1FAF6D6F6C9E80170234D9B57 | otpcrypt -c -d -i 48C56AA1FAF6D6F6C9E80170234D9B57
    Pomegranate

Compare and cross-validate the results with OpenSSL encryption tool:

    # echo Pomegranate | openssl enc -aes-128-cbc -K 00000000000000000000000000000000 -iv 48C56AA1FAF6D6F6C9E80170234D9B57 | base64
    99UWQqU9Jj68GNPWBTBD2g==

    # echo Pomegranate | otpcrypt -c -i 48C56AA1FAF6D6F6C9E80170234D9B57 | openssl enc -d -aes-128-cbc -K 00000000000000000000000000000000 -iv 48C56AA1FAF6D6F6C9E80170234D9B57
    Pomegranate

    # echo Pomegranate | openssl enc -aes-128-cbc -K 00000000000000000000000000000000 -iv 48C56AA1FAF6D6F6C9E80170234D9B57 | otpcrypt -c -d -i 48C56AA1FAF6D6F6C9E80170234D9B57
    Pomegranate
