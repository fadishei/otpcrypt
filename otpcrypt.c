#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <inttypes.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include "dcp_bootstream_ioctl.h"

// cleanup vars
static int dcpboot_opened = 0;

// cmd line args
static int cbc = 0, decrypt = 0;
static uint8_t iv[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// other global vars
static int dcpboot;

void print_help()
{
	printf("usage: otpcrypt [options]\n");
	printf("encrypts data read from standard input using iMX23 dcp with otp key and writes the result to stdout\n");
	printf("options:\n");
	printf("-c:      use cbc mode instead of ecb mode\n");
	printf("-d:      perform data decryption instead of encryption\n");
	printf("-i <iv>: use specific iv in hex (default=00000000000000000000000000000000)\n");
}

void cleanup(int ret)
{
	if (dcpboot_opened)
		close(dcpboot);
	exit(ret);
}

void cleanup_perror(int ret, const char *s)
{
	perror(s);
	cleanup(ret);
}

void cleanup_error(int ret, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	cleanup(ret);
}

int digit(char c)
{
	if (c>='0' && c<='9')
		return c - '0';
	if (c>='a' && c<='f')
		return c - 'a' + 10;
	if (c>='A' && c<='F')
		return c - 'A' + 10;
	else
	{
		cleanup_error(4, "hex digit expected");
		return -1;
	}
}

void memxor(uint8_t *dst, uint8_t *src, int n)
{
	int i;
	for (i=0; i<n; i++)
		dst[i] ^= src[i];
}

void print(uint8_t *buf, int len)
{
	int i;
	for (i=0; i<len; i++)
		putc(buf[i], stdout);
}

void decrypt16(uint8_t *buf)
{
	if(ioctl(dcpboot, DBS_DEC, buf) != 0)
		cleanup_error(3, "can not ioctl dcpboot for decryption");
}

int unpad(uint8_t *data)
{
	int i;
	int candid = 16-data[15];
	for (i=candid; i<16; i++)
		if (data[i] != data[15])
			return 16;
	return candid;
}

void decrypt_loop()
{
	int c, i;
	static uint8_t data[16];
	static uint8_t prev[16];

	i = 0;
	c = getc(stdin);
	while (c != EOF)
	{
		data[i] = c;
		i = (i+1)%16;
		c=getc(stdin);
		if (i==0)
		{
			if (cbc)
				memcpy(prev, data, 16);
			decrypt16(data);
			if (cbc)
				memxor(data, iv, 16);
			if (c == EOF)
			{
				print(data, unpad(data));
				break;
			}
			else
			{
				print(data, 16);
				if (cbc)
					memcpy(iv, prev, 16);
			}
		}
	}
	if (i != 0)
		cleanup_error(5, "invalid encrypted data length");
}

void encrypt16(uint8_t *buf)
{
	if(ioctl(dcpboot, DBS_ENC, buf) != 0)
		cleanup_error(3, "can not ioctl dcpboot for encryption");
}

void pad(uint8_t *buf, int i)
{
	int ps = 16 - i;
	for (; i<16; i++)
		buf[i] = cbc? (ps^buf[i]): ps;
}

void encrypt_loop()
{
	int c, i;
	static uint8_t data[16];

	if (cbc)
		memcpy(data, iv, 16);
	i = 0;
	while ((c=getc(stdin)) != EOF)
	{
		data[i] = cbc? (c^data[i]):c;
		i = (i+1)%16;
		if (i==0)
		{
			encrypt16(data);
			print(data, 16);
		}
	}
	pad(data, i);
	encrypt16(data);
	print(data, 16);
}

int main(int argc, char *argv[])
{
	int c, i, j;

	while ((c=getopt(argc, argv,"+cdi:h")) != -1)
	{
		switch(c)
		{
			case 'c':
				cbc = 1;
				break;
			case 'd':
				decrypt = 1;
				break;
			case 'i':
				for (j=0, i=0; i<strlen(optarg) && j<16; i++, j++)
				{
					iv[j] = 16*digit(optarg[i]);
					i++;
					if (i<strlen(optarg))
						iv[j] = iv[j] + digit(optarg[i]);
				}
				if (i<strlen(optarg))
					cleanup_error(7, "iv should be at most 32 hex digits");
				//for (i=0; i<16; i++)
				//	fprintf(stderr, "%02x", iv[i]);
				//fprintf(stderr, "\n");
				break;
			case 'h':
				print_help();
				cleanup(0);
				break;
			default:
				print_help();
				cleanup(1);
				break;
		}
	}
	dcpboot = open("/dev/dcpboot", O_RDWR);
	if (dcpboot == -1)
		cleanup_error(2, "can not open dcpboot");
	dcpboot_opened = 1;

	if (decrypt)
		decrypt_loop();
	else
		encrypt_loop();

	cleanup(0);
	return (0);
}
