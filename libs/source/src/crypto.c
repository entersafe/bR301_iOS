/*
 * Support for bR301(Bluetooth) smart card reader
 *
 * Copyright (C) Feitian 2014, Ben <ben@ftsafe.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <string.h>
#include "crypto.h"

#define DES_BLOCK_SIZE 8

int PKCS7_Padding(unsigned char *pbMsg, int nMsgLen, int szblk)
{
	int nPadLen;

	nPadLen = szblk - nMsgLen % szblk;
	//pading
	if (nPadLen) {
		memset( pbMsg+nMsgLen, nPadLen, nPadLen );
	}

	return (nMsgLen+nPadLen);
}


int PKCS7_Padding_Check(unsigned char *pbMsg, int nMsgLen, int szblk)
{
	unsigned char i;
	unsigned char *p = pbMsg + nMsgLen - 1;
	unsigned char nPadLen = *p;

	if(0 != nMsgLen%szblk   || 0 == nPadLen || nPadLen > szblk) {
		return 0;
	}

	for (i = 0; i < nPadLen; ++i) {
		if(nPadLen != *p--) {
			return 0;
		}
	}

	return 1;
}

static void xor(unsigned char *a, unsigned char *b, unsigned char *r, int len)
{
	while(len--) {
		*r++ = *a++ ^ *b++;
	}

	return;
}


void AES_ECB(unsigned char *inBlk, int len, unsigned char *outBlk,  unsigned char *key, int keyLen, int mode)
{
	int i;

	len -= len % AES_BLOCK_SIZE;

	for (i = 0; i < len; i += AES_BLOCK_SIZE ) {
		if ( mode == MODE_ENCRYPT ) {
			Ft_iR301U_AES_encrypt( inBlk, outBlk, key, keyLen );
		} else {
			Ft_iR301U_AES_decrypt( inBlk, outBlk, key, keyLen );
		}
		inBlk += AES_BLOCK_SIZE;
		outBlk += AES_BLOCK_SIZE;
	}

	return;
}



void TDES(unsigned char * inBlk, unsigned char * outBlk,  unsigned char *key, int keyLen,  int mode)
{

	unsigned char *key1, *key2, *key3;

	switch ( keyLen ) {
	case 8:
		key1 = key;
		key2 = key;
		key3 = key;
		break;
	case 16:
		key1 = key;
		key2 = key + 8;
		key3 = key;
		break;
	case 24:
		if (MODE_DECRYPT != mode) {
			key1 = key + 0;
			key2 = key + 8;
			key3 = key + 16;
		} else {
			key1 = key + 16;
			key2 = key + 8;
			key3 = key +  0;
		}
		break;
	}

	memcpy(outBlk, inBlk, 8);
	DES_C_DES(key1, outBlk, mode);
	DES_C_DES(key2, outBlk, !mode);
	DES_C_DES(key3, outBlk, mode);

	return;
}


void TDES_ECB(unsigned char *inBlk, int len, unsigned char *outBlk,  unsigned char *key, int keyLen, int mode)
{
	int i = 0;

	for (i = 0; i < (len/DES_BLOCK_SIZE); i++) {
		TDES(inBlk, outBlk, key, keyLen, mode);
		inBlk   += DES_BLOCK_SIZE;
		outBlk  += DES_BLOCK_SIZE;
	}

	return;
}


int TDES_ECB_PKCS7(unsigned char *inBuf, unsigned int inLen, unsigned char *outBuf,  unsigned int *outLen, unsigned char *key, unsigned int keyLen, int mode)
{
	int nLastBlk = inLen % DES_BLOCK_SIZE;
	unsigned char *outLastBlk = outBuf+inLen-nLastBlk;

	TDES_ECB(inBuf, inLen-nLastBlk, outBuf, key, keyLen, mode);

	if(MODE_ENCRYPT == mode) {
		memcpy(outLastBlk, inBuf+inLen-nLastBlk, nLastBlk);
		PKCS7_Padding(outLastBlk, nLastBlk, DES_BLOCK_SIZE);
		TDES_ECB(outLastBlk, DES_BLOCK_SIZE, outLastBlk, key, keyLen, mode);

		*outLen = (inLen/DES_BLOCK_SIZE + 1) * DES_BLOCK_SIZE;
	} else {
		if (0 == PKCS7_Padding_Check(outBuf, inLen, DES_BLOCK_SIZE)) {
			return 0;
		}

		*outLen = inLen - *(outLastBlk-1);
	}

	return 1;
}


int AES_ECB_PKCS7(unsigned char *inBuf, unsigned int inLen, unsigned char *outBuf, unsigned  int *outLen, unsigned char *key, unsigned int keyLen, int mode)
{
	int nLastBlk = inLen % AES_BLOCK_SIZE;
	unsigned char *outLastBlk = outBuf+inLen-nLastBlk;

	AES_ECB(inBuf, inLen-nLastBlk, outBuf, key, keyLen, mode);

	if(MODE_ENCRYPT == mode) {
		memcpy(outLastBlk, inBuf+inLen-nLastBlk, nLastBlk);
		PKCS7_Padding(outLastBlk, nLastBlk, AES_BLOCK_SIZE);
		AES_ECB(outLastBlk, AES_BLOCK_SIZE, outLastBlk, key, keyLen, mode);

		*outLen = (inLen/AES_BLOCK_SIZE + 1) * AES_BLOCK_SIZE;
	} else {
		if (0 == PKCS7_Padding_Check(outBuf, inLen, AES_BLOCK_SIZE)) {
			return 0;
		}

		*outLen = inLen - *(outLastBlk-1);
	}

	return 1;

}














