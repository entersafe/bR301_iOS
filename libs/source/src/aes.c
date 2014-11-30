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


#include "aes.h"
#include <string.h>

#define Bits128			(16)
#define Bits192			(24)
#define Bits256			(32)
#define	NB					4		 // block size in 32-bit DWORDs.  Always 4 for AES.  (128 bits).

typedef unsigned char * PBYTE;
typedef unsigned char 	BYTE;
typedef unsigned short 	WORD;
typedef unsigned long   DWORD;

BYTE gfmultby01(BYTE b);
BYTE gfmultby02(BYTE b);
BYTE gfmultby03(BYTE b);
BYTE gfmultby09(BYTE b);
BYTE gfmultby0b(BYTE b);
BYTE gfmultby0d(BYTE b);
BYTE gfmultby0e(BYTE b);

void SubDWORD (BYTE* oneWord);
void RotDWORD (BYTE* oneWord);
void SubBytes (BYTE State[4][4]);
void MixColumns (BYTE State[4][4]);
void ShiftRows (BYTE State[4][4]);
void InvSubBytes (BYTE State[4][4]);
void InvShiftRows (BYTE State[4][4]);
void InvMixColumns (BYTE State[4][4]);
void SetNbNkNr (DWORD keySize,BYTE *Nr,BYTE* Nk);
void KeyExpansion (BYTE* w,BYTE*key,BYTE Nr,BYTE Nk);
void AddRoundKey (DWORD round,BYTE *w,BYTE State[4][4]);


const  BYTE independency_newAesSbox[16*16]=
{// populate the Sbox matrix
		/* 0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f */
		/*0*/  0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
		/*1*/  0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
		/*2*/  0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
		/*3*/  0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
		/*4*/  0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
		/*5*/  0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
		/*6*/  0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
		/*7*/  0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
		/*8*/  0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
		/*9*/  0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
		/*a*/  0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
		/*b*/  0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
		/*c*/  0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
		/*d*/  0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
		/*e*/  0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
		/*f*/  0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

const BYTE independency_newAesiSbox[16*16]=
{
	// populate the iSbox matrix
		/* 0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f */
		/*0*/  0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
		/*1*/  0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
		/*2*/  0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
		/*3*/  0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
		/*4*/  0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
		/*5*/  0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
		/*6*/  0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
		/*7*/  0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
		/*8*/  0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
		/*9*/  0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
		/*a*/  0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
		/*b*/  0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
		/*c*/  0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
		/*d*/  0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
		/*e*/  0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
		/*f*/  0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
};

const BYTE independency_newAesRcon[11*4]=
{
	0x00, 0x00, 0x00, 0x00,  
	0x01, 0x00, 0x00, 0x00,
	0x02, 0x00, 0x00, 0x00,
	0x04, 0x00, 0x00, 0x00,
	0x08, 0x00, 0x00, 0x00,
	0x10, 0x00, 0x00, 0x00,
	0x20, 0x00, 0x00, 0x00,
	0x40, 0x00, 0x00, 0x00,
	0x80, 0x00, 0x00, 0x00,
	0x1b, 0x00, 0x00, 0x00,
	0x36, 0x00, 0x00, 0x00
};


void AES_Memcpy(BYTE * d, BYTE * s, int Len)
{
	int i;
	for(i=0; i<Len; i++)
	{
		d[i]=s[i];
	}
	
	return;
}

void AES_Memset(BYTE * d, BYTE val, int Len)
{
	int i;
	for(i=0; i<Len; i++)
	{
		d[i]=val;
	}

	return;
}


void Aes_setKey(DWORD keysize,BYTE* keyBytes,BYTE *w,BYTE *key,BYTE* Nr,BYTE *Nk)
{
	SetNbNkNr(keysize,Nr,Nk);
	AES_Memcpy(key,keyBytes,keysize);
	KeyExpansion(w,key,*Nr,*Nk);
}

void SetNbNkNr(DWORD keySize,BYTE *Nr, BYTE* Nk)
{
	//Nb=4;
	if(keySize==Bits128)
	{
		*Nk=4;
		*Nr=10;
	}
	else if(keySize==Bits192)
	{
		*Nk=6;
		*Nr=12;
	}
	else if(keySize==Bits256)
	{
		*Nk=8;
		*Nr=14;
	}
}

void KeyExpansion(BYTE* w,BYTE *key,BYTE Nr,BYTE Nk)
{
	DWORD row;
	BYTE temp[ 4 ];
	AES_Memset(w,0,16*15);
	for( row=0;row<Nk;row++)
	{
		w[4*row+0] =  key[4*row];
		w[4*row+1] =  key[4*row+1];
		w[4*row+2] =  key[4*row+2];
		w[4*row+3] =  key[4*row+3];
	}
	
	for(row=Nk;row<4*(Nr+1);row++)
	{
		temp[0]=w[4*row-4];
		temp[1]=w[4*row-3];
		temp[2]=w[4*row-2];
		temp[3]=w[4*row-1];
		if(row%Nk==0)
		{
			RotDWORD(temp);
			SubDWORD(temp);
			temp[0] = ( BYTE )( (DWORD)temp[0] ^ (DWORD) independency_newAesRcon[4*(row/Nk)+0] );   
			temp[1] = ( BYTE )( (DWORD)temp[1] ^ (DWORD) independency_newAesRcon[4*(row/Nk)+1] );
			temp[2] = ( BYTE )( (DWORD)temp[2] ^ (DWORD) independency_newAesRcon[4*(row/Nk)+2] );
			temp[3] = ( BYTE )( (DWORD)temp[3] ^ (DWORD) independency_newAesRcon[4*(row/Nk)+3] );
    }
	else if ( Nk > 6 && (row % Nk == 4) )
    {
			SubDWORD(temp);
    }
        
        // w[row] = w[row-Nk] xor temp
        w[4*row+0] = ( BYTE ) ( (DWORD) w[4*(row-Nk)+0] ^ (DWORD)temp[0] );
				w[4*row+1] = ( BYTE ) ( (DWORD) w[4*(row-Nk)+1] ^ (DWORD)temp[1] );
				w[4*row+2] = ( BYTE ) ( (DWORD) w[4*(row-Nk)+2] ^ (DWORD)temp[2] );
				w[4*row+3] = ( BYTE ) ( (DWORD) w[4*(row-Nk)+3] ^ (DWORD)temp[3] );
	}  // for loop
	
}

void RotDWORD(BYTE* w)
{
	BYTE temp[ 4 ];
	temp[0] = w[1];
	temp[1] = w[2];
	temp[2] = w[3];
	temp[3] = w[0];
	AES_Memcpy( w, temp, 4 );
}


void SubDWORD(BYTE* oneWord)
{
	BYTE temp[ 4 ];
	DWORD j;
	for(j=0;j<4;j++)
	{
		temp[j] = independency_newAesSbox[16*(oneWord[j] >> 4)+(oneWord[j] & 0x0f)];
	}
	AES_Memcpy( oneWord, temp, 4 );
}


void AES_enc(BYTE* input, BYTE* output,BYTE *w,BYTE Nr)
{
	DWORD i;
	DWORD round;
	BYTE gState[4][4];
	AES_Memset(&gState[0][0],0,16);
	for( i=0;i<4*NB;i++)
	{
		gState[i%4][i/4]=input[i];
	}
	AddRoundKey(0,w,gState);
	
	for ( round = 1; round <= (Nr - 1); round++)
	{
        SubBytes(gState);
        ShiftRows(gState);
        MixColumns(gState);
        AddRoundKey(round,w,gState);
	}  // main round loop
	
	SubBytes(gState);
	ShiftRows(gState);
	AddRoundKey(Nr,w,gState);
	
	// output = state
	for (i = 0; i < (4 * NB); i++)
	{
        output[i] =  gState[i % 4][ i / 4];
	}

}

void AES_dec(BYTE* input,BYTE* output,BYTE *w,BYTE Nr)
{
	BYTE gState[4][4];
	DWORD i,round;
	AES_Memset(&gState[0][0],0,16);
	for ( i = 0; i < (4 * NB); i++)
	{
		gState[i % 4][ i / 4] = input[i];
	}
	
	AddRoundKey(Nr,w,gState);
	
	for ( round = Nr-1; round >= 1; round--)  // main round loop
	{
        InvShiftRows(gState);
        InvSubBytes(gState);
        AddRoundKey(round,w,gState);
        InvMixColumns(gState);
	}  // end main round loop for InvCipher
	
	InvShiftRows(gState);
	InvSubBytes(gState);
	AddRoundKey(0,w,gState);
	
	// output = state
	for (i = 0; i < (4 * NB); i++)
	{
        output[i] =  gState[i % 4][ i / 4];
	}
}

void AddRoundKey(DWORD round,BYTE *w,BYTE State[4][4])
{
	DWORD i,j;
	for(j=0;j<4;j++)
	{
		for(i=0;i<4;i++)
		{
			State[i][j]=(BYTE)((DWORD)State[i][j]^(DWORD)w[4*((round*4)+j)+i]);  
		}
	}
}

void SubBytes(BYTE State[4][4])                              //Page 103
{
	DWORD i,j;
	for(j=0;j<4;j++)
	{
		for(i=0;i<4;i++)
		{
			State[i][j]=independency_newAesSbox[State[i][j]];

		}
	}
}
void InvSubBytes(BYTE State[4][4])
{
	DWORD i,j;
	for(j=0;j<4;j++)
	{
		for(i=0;i<4;i++)
		{
			State[i][j]=independency_newAesiSbox[State[i][j]];
		}
	}

}

void ShiftRows(BYTE State[4][4])
{
	BYTE temp[4*4];                                        //Page105
	DWORD i,j;
	for(j=0;j<4;j++)
	{
		for(i=0;i<4;i++)
		{
			temp[4*i+j]=State[i][j];
		}
	}
	for(i=1;i<4;i++)
	{
		for(j=0;j<4;j++)
		{
			if(i==1)State[i][j]=temp[4*i+(j+1)%4];
			else if(i==2)State[i][j]=temp[4*i+(j+2)%4];
			else if(i==3)State[i][j]=temp[4*i+(j+3)%4];
		}
	}

}
void InvShiftRows(BYTE State[4][4])
{
	BYTE temp[4*4];
	DWORD i,j;
	for(j=0;j<4;j++)
	{
		for(i=0;i<4;i++)
		{
			temp[4*i+j]=State[i][j];
		}
	}
	for(i=1;i<4;i++)
	{
		for(j=0;j<4;j++)
		{
			if(i==1)State[i][j]=temp[4*i+(j+3)%4];
			else if(i==2)State[i][j]=temp[4*i+(j+2)%4];
			else if(i==3)State[i][j]=temp[4*i+(j+1)%4];
		}
	}
	
}
////////////////////////////////////////////////////////////////////////////////////////////////
void MixColumns(BYTE State[4][4])
{
	BYTE temp[4*4];
	DWORD i,j;
	for(j=0;j<4;j++)                    //2 3 1 1				Page107
	{									//1 2 3 1
		for(i=0;i<4;i++)				//1 1 2 3
		{								//3 1 1 2
			temp[4*i+j]=State[i][j];
		}
	}
	for(j=0;j<4;j++)
	{
		State[0][j] = (BYTE) ( (DWORD)gfmultby02(temp[0+j]) ^ (DWORD)gfmultby03(temp[4*1+j]) ^
			(DWORD)gfmultby01(temp[4*2+j]) ^ (DWORD)gfmultby01(temp[4*3+j]) );
		State[1][j] = (BYTE) ( (DWORD)gfmultby01(temp[0+j]) ^ (DWORD)gfmultby02(temp[4*1+j]) ^
			(DWORD)gfmultby03(temp[4*2+j]) ^ (DWORD)gfmultby01(temp[4*3+j]) );
		State[2][j] = (BYTE) ( (DWORD)gfmultby01(temp[0+j]) ^ (DWORD)gfmultby01(temp[4*1+j]) ^
			(DWORD)gfmultby02(temp[4*2+j]) ^ (DWORD)gfmultby03(temp[4*3+j]) );
		State[3][j] = (BYTE) ( (DWORD)gfmultby03(temp[0+j]) ^ (DWORD)gfmultby01(temp[4*1+j]) ^
			(DWORD)gfmultby01(temp[4*2+j]) ^ (DWORD)gfmultby02(temp[4*3+j]) );
	}
	
}
void InvMixColumns(BYTE State[4][4])
{
	BYTE temp[4*4];
	DWORD i,j;
	for (i = 0; i < 4; i++)  							// copy State DWORDo temp[]
	{
        for (j = 0; j < 4; j++)         				//0e 0b 0d 09 		 Page108
        {												//09 0e 0b 0d
			temp[4*i+j] =  State[i][j];					//0d 09 0e 0b
        }												//0b 0d 09 0e
	}
	
	for (j = 0; j < 4; j++)
	{
		State[0][j] = (BYTE) ( (DWORD)gfmultby0e(temp[j]) ^ (DWORD)gfmultby0b(temp[4+j]) ^
			(DWORD)gfmultby0d(temp[4*2+j]) ^ (DWORD)gfmultby09(temp[4*3+j]) );
		State[1][j] = (BYTE) ( (DWORD)gfmultby09(temp[j]) ^ (DWORD)gfmultby0e(temp[4+j]) ^
			(DWORD)gfmultby0b(temp[4*2+j]) ^ (DWORD)gfmultby0d(temp[4*3+j]) );
		State[2][j] = (BYTE) ( (DWORD)gfmultby0d(temp[j]) ^ (DWORD)gfmultby09(temp[4+j]) ^
			(DWORD)gfmultby0e(temp[4*2+j]) ^ (DWORD)gfmultby0b(temp[4*3+j]) );
		State[3][j] = (BYTE) ( (DWORD)gfmultby0b(temp[j]) ^ (DWORD)gfmultby0d(temp[4+j]) ^
			(DWORD)gfmultby09(temp[4*2+j]) ^ (DWORD)gfmultby0e(temp[4*3+j]) );
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////
BYTE gfmultby01(BYTE b)
{
	return b;
}
BYTE gfmultby02(BYTE b)
{
	if (b < 0x80)
        return (BYTE)(DWORD)(b <<1);
	else
        return (BYTE)( (DWORD)(b << 1) ^ (DWORD)(0x1b) );
}

BYTE gfmultby03(BYTE b)
{
	return (BYTE) ( (DWORD)gfmultby02(b) ^ (DWORD)b );
}

BYTE gfmultby09(BYTE b)
{
	return (BYTE)( (DWORD)gfmultby02(gfmultby02(gfmultby02(b))) ^ (DWORD)b );
}

BYTE gfmultby0b(BYTE b)
{
	return (BYTE)( (DWORD)gfmultby02(gfmultby02(gfmultby02(b))) ^
		(DWORD)gfmultby02(b) ^ (DWORD)b );
}

BYTE gfmultby0d(BYTE b)
{
	return (BYTE)( (DWORD)gfmultby02(gfmultby02(gfmultby02(b))) ^
		(DWORD)gfmultby02(gfmultby02(b)) ^ (DWORD)(b) );
}

BYTE gfmultby0e(BYTE b)
{
	return (BYTE)( (DWORD)gfmultby02(gfmultby02(gfmultby02(b))) ^
		(DWORD)gfmultby02(gfmultby02(b)) ^(DWORD)gfmultby02(b) );
}

void Ft_iR301U_AES_encrypt(PBYTE plaintext, PBYTE ciphertext, PBYTE key, int szKey)
{
	BYTE ww[16*15];
	BYTE aeskey[32];
	BYTE NNr,NNk;
	Aes_setKey(szKey, key, ww, aeskey, &NNr, &NNk);
	AES_enc(plaintext, ciphertext, ww, NNr);	
}

void Ft_iR301U_AES_decrypt(PBYTE ciphertext, PBYTE plaintext, PBYTE key, int szKey)
{
	BYTE ww[16*15];
	BYTE aeskey[32];
	BYTE NNr,NNk;
	Aes_setKey(szKey, key, ww, aeskey, &NNr, &NNk);
	AES_dec(ciphertext, plaintext, ww, NNr);	
}
