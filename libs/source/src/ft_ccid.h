/*
 * Support for bR301(Bluetooth) smart card reader
 *
 * Copyright (C) Feitian 2014, Ben <ben@ftsafe.com>
 *
 * Some part of code apply from pps.h file which is CCID open source code, all the reserved by (Copyright (C) 2000 2001 Carlos Prados <cprados@yahoo.com>)
 * This file has moved from CCID Driver by GNU license (http://pcsclite.alioth.debian.org/ccid.html)
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



#ifndef __HZ_CCID_H__
#define __HZ_CCID_H__

#include <stdio.h>
#include <stdbool.h>
#include "wintypes.h"
#include "proto-t1.h"
#include "winscard.h"
#include "string.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef long RESPONSECODE;
#define CCID_DRIVER_MAX_READERS 1

typedef enum READERTYPE{
    READER_UNKOWN = 0,
    READER_bR301,
    READER_iR301U_DOCK,
    READER_iR301U_LIGHTING
    
}READERTYPE;

typedef enum FTREADER_INTERNAL
{
    FT_READER_DEFAULT = 0x00,
    FT_READER_UA      = 0x01,
    FT_READER_UB      = 0x02,
    FT_READER_UB_LT   = 0x03,
    FT_READER_UC      = 0x04,
    FT_READER_UC_LT   = 0x05,
    FT_READER_UC_B    = 0x06,
    FT_READER_UC_LT_B = 0x07,
    FT_READER_UM      = 0x08,
    FT_READER_UD      = 0x09,
    FT_READER_UD_LT   = 0x0A,
    
}FTREADER_INTERNAL;


#define ReadPort ReadSerial
#define WritePort WriteSerial
    
//#ifndef BOOL
//#define BOOL signed char
//#endif
//
//#ifndef TRUE
//#define TRUE 1
//#endif
//    
//#ifndef FALSE
//#define FALSE 0
//#endif
    


/* Protocols */
#define T_0 	0
#define T_1 	1
#define T_RAW 3
/* Default communication read timeout in milliseconds */

#define DEFAULT_COM_READ_TIMEOUT (6000)    

/* bInterfaceProtocol for ICCD */
#define PROTOCOL_CCID		0	/* plain CCID */
#define PROTOCOL_ICCD_A	1	/* ICCD Version A */
#define PROTOCOL_ICCD_B	2	/* ICCD Version B */


/* Features from dwFeatures */
#define CCID_CLASS_AUTO_CONF_ATR	0x00000002
#define CCID_CLASS_AUTO_VOLTAGE		0x00000008
#define CCID_CLASS_AUTO_BAUD		0x00000020
#define CCID_CLASS_AUTO_PPS_PROP	0x00000040
#define CCID_CLASS_AUTO_PPS_CUR		0x00000080
#define CCID_CLASS_AUTO_IFSD		0x00000400
#define CCID_CLASS_CHARACTER		0x00000000
#define CCID_CLASS_TPDU				0x00010000
#define CCID_CLASS_SHORT_APDU		0x00020000
#define CCID_CLASS_EXTENDED_APDU	0x00040000
#define CCID_CLASS_EXCHANGE_MASK	0x00070000

/* Features from bPINSupport */
#define CCID_CLASS_PIN_VERIFY		0x01
#define CCID_CLASS_PIN_MODIFY		0x02


//Compatibility UA UB
#define MAX_BUFFER_SIZE_ENC     1024
#define CMD_BUF_SIZE MAX_BUFFER_SIZE_EXTENDED
#define CMD_BUF_SIZE_EACH (MAX_BUFFER_SIZE_ENC+10)
    
typedef enum {
	STATUS_NO_SUCH_DEVICE        = 0xF9,
	STATUS_SUCCESS               = 0xFA,
	STATUS_UNSUCCESSFUL          = 0xFB,
	STATUS_COMM_ERROR            = 0xFC,
	STATUS_DEVICE_PROTOCOL_ERROR = 0xFD,
	STATUS_COMM_NAK              = 0xFE,
	STATUS_SECONDARY_SLOT        = 0xFF
} status_t;

typedef struct
{
	/*
	 * CCID Sequence number
	 */
	unsigned char real_bSeq;

	/*
	 * VendorID << 16 + ProductID
	 */
	int readerID;

	/*
	 * Maximum message length
	 */
	unsigned int dwMaxCCIDMessageLength;

	/*
	 * Maximum IFSD
	 */
	int dwMaxIFSD;

	/*
	 * Features supported by the reader (directly from Class Descriptor)
	 */
	int dwFeatures;

	/*
	 * PIN support of the reader (directly from Class Descriptor)
	 */
	char bPINSupport;
	
	/*
	 * Display dimensions of the reader (directly from Class Descriptor)
	 */
	unsigned int wLcdLayout;

	/*
	 * Default Clock
	 */
	int dwDefaultClock;

	/*
	 * Max Data Rate
	 */
	unsigned int dwMaxDataRate;

	/*
	 * Number of available slots
	 */
	char bMaxSlotIndex;

	/*
	 * Slot in use
	 */
	char bCurrentSlotIndex;

	/*
	 * The array of data rates supported by the reader
	 */
	unsigned int *arrayOfSupportedDataRates;

	/*
	 * Read communication port timeout
	 * value is milliseconds
	 * this value can evolve dynamically if card request it (time processing).
	 */
	unsigned int readTimeout;

	/*
	 * Card protocol
	 */
	int cardProtocol;

	/*
	 * bInterfaceProtocol (CCID, ICCD-A, ICCD-B)
	 */
	int bInterfaceProtocol;


	/*
	 * GemCore SIM PRO slot status management
	 * The reader always reports a card present even if no card is inserted.
	 * If the Power Up fails the driver will report IFD_ICC_NOT_PRESENT instead
	 * of IFD_ICC_PRESENT
	 */
	int dwSlotStatus;

	/*
	 * bVoltageSupport (bit field)
	 * 1 = 5.0V
	 * 2 = 3.0V
	 * 4 = 1.8V
	 */
	int bVoltageSupport;

	
} _ccid_descriptor;


/* See CCID specs ch. 4.2.1 */
#define CCID_ICC_PRESENT_ACTIVE		0x00	/* 00 0000 00 */
#define CCID_ICC_PRESENT_INACTIVE	0x01	/* 00 0000 01 */
#define CCID_ICC_ABSENT				0x02	/* 00 0000 10 */
#define CCID_ICC_STATUS_MASK		0x03	/* 00 0000 11 */

#define CCID_COMMAND_FAILED			0x40	/* 01 0000 00 */
#define CCID_TIME_EXTENSION			0x80	/* 10 0000 00 */

#define STATUS_OFFSET			7
#define ERROR_OFFSET			8
#define CHAIN_PARAMETER_OFFSET	9
#define SIZE_GET_SLOT_STATUS	10


/* convert a 4 byte integer in USB format into an int */
#define dw2i(a, x) (unsigned int)((((((a[x+3] << 8) + a[x+2]) << 8) + a[x+1]) << 8) + a[x])


#define IFD_PARITY_ERROR 699
/////////////////////////////////////////

/*
 * List of defines available to ifdhandler
 */
#define	IFD_SUCCESS				0   /**< no error */
#define IFD_NEGOTIATE_PTS1		1   /**< negotiate PTS1 */
#define IFD_NEGOTIATE_PTS2		2   /**< negotiate PTS2 */
#define IFD_NEGOTIATE_PTS3    	4   /**< negotiate PTS3 */

#define IFD_POWER_UP				500 /**< power up the card */
#define IFD_POWER_DOWN				501 /**< power down the card */
#define IFD_RESET					502 /**< warm reset */
#define IFD_ERROR_TAG				600 /**< tag unknown */
#define IFD_ERROR_SET_FAILURE		601 /**< set failed */
#define IFD_ERROR_VALUE_READ_ONLY	602 /**< value is read only */
#define IFD_ERROR_PTS_FAILURE		605 /**< failed to negotiate PTS */
#define IFD_ERROR_NOT_SUPPORTED		606
#define IFD_PROTOCOL_NOT_SUPPORTED	607 /**< requested protocol not supported */
#define IFD_ERROR_POWER_ACTION		608 /**< power up failed */
#define IFD_ERROR_SWALLOW			609
#define IFD_ERROR_EJECT				610
#define IFD_ERROR_CONFISCATE		611
#define IFD_COMMUNICATION_ERROR		612 /**< generic error */
#define IFD_RESPONSE_TIMEOUT		613 /**< timeout */
#define IFD_NOT_SUPPORTED			614 /**< request is not supported */
#define IFD_ICC_PRESENT				615 /**< card is present */
#define IFD_ICC_NOT_PRESENT			616 /**< card is absent */
/**
 * The \ref IFD_NO_SUCH_DEVICE error must be returned by the driver when
 * it detects the reader is no more present. This will tell pcscd to
 * remove the reader from the list of available readers.
 */
#define IFD_NO_SUCH_DEVICE				617
#define IFD_ERROR_INSUFFICIENT_BUFFER	618 /**< buffer is too small */



//////////////////////////////////////////////////////////////////////////
//pps
//
/*
* Exported constants definition
*/

#define PPS_OK				0	/* Negotiation OK */
#define PPS_ICC_ERROR		1	/* Comunication error */
#define PPS_HANDSAKE_ERROR	2	/* Agreement not reached */
#define PPS_PROTOCOL_ERROR	3	/* Error starting protocol */
#define PPS_MAX_LENGTH		6

#define PPS_HAS_PPS1(block)	((block[1] & 0x10) == 0x10)
#define PPS_HAS_PPS2(block)	((block[1] & 0x20) == 0x20)
#define PPS_HAS_PPS3(block)	((block[1] & 0x40) == 0x40)

//////////////////////////////////////////////////////////////////////////
//atr
// 



/* Return values */
#define ATR_OK			0	/* ATR could be parsed and data returned */
#define ATR_NOT_FOUND	1	/* Data not present in ATR */
#define ATR_MALFORMED	2	/* ATR could not be parsed */
#define ATR_IO_ERROR	3	/* I/O stream error */
 
/* Paramenters */
#define ATR_MAX_SIZE 				33	/* Maximum size of ATR byte array */
#define ATR_MAX_HISTORICAL			15	/* Maximum number of historical bytes */
#define ATR_MAX_PROTOCOLS			7	/* Maximun number of protocols */
#define ATR_MAX_IB					4	/* Maximum number of interface bytes per protocol */
#define ATR_CONVENTION_DIRECT		0	/* Direct convention */
#define ATR_CONVENTION_INVERSE		1	/* Inverse convention */
#define ATR_PROTOCOL_TYPE_T0		0	/* Protocol type T=0 */
#define ATR_PROTOCOL_TYPE_T1		1	/* Protocol type T=1 */
#define ATR_PROTOCOL_TYPE_T2		2	/* Protocol type T=2 */
#define ATR_PROTOCOL_TYPE_T3		3	/* Protocol type T=3 */
#define ATR_PROTOCOL_TYPE_T14		14	/* Protocol type T=14 */
#define ATR_INTERFACE_BYTE_TA		0	/* Interface byte TAi */
#define ATR_INTERFACE_BYTE_TB		1	/* Interface byte TBi */
#define ATR_INTERFACE_BYTE_TC		2	/* Interface byte TCi */
#define ATR_INTERFACE_BYTE_TD		3	/* Interface byte TDi */
#define ATR_PARAMETER_F				0	/* Parameter F */
#define ATR_PARAMETER_D				1	/* Parameter D */
#define ATR_PARAMETER_I				2	/* Parameter I */
#define ATR_PARAMETER_P				3	/* Parameter P */
#define ATR_PARAMETER_N				4	/* Parameter N */
#define ATR_INTEGER_VALUE_FI		0	/* Integer value FI */
#define ATR_INTEGER_VALUE_DI		1	/* Integer value DI */
#define ATR_INTEGER_VALUE_II		2	/* Integer value II */
#define ATR_INTEGER_VALUE_PI1		3	/* Integer value PI1 */
#define ATR_INTEGER_VALUE_N			4	/* Integer value N */
#define ATR_INTEGER_VALUE_PI2		5	/* Integer value PI2 */

#define DUKPT_ENC_OFF           0x00 //default
#define DUKPT_ENC_ON            0x01
    
#define DUKPT_ENC_FUNC_3DES     0x00 //default
#define DUKPT_ENC_FUNC_AES      0x01
    
#define DUKPT_ENC_TYPE_DIR      0x00 //default
#define DUKPT_ENC_TYPE_UNDIR    0x01
    
/* Default values for paramenters */

#define ATR_DEFAULT_D	1
#define ATR_DEFAULT_N	0
#define ATR_DEFAULT_P	5
#define ATR_DEFAULT_I	50
#define ATR_DEFAULT_F	372

/*
 * Exported data types definition
 */

typedef struct
{
  unsigned length;
  BYTE TS;
  BYTE T0;
  struct
  {
    BYTE value;
    bool present;
  }
  ib[ATR_MAX_PROTOCOLS][ATR_MAX_IB], TCK;
  unsigned pn;
  BYTE hb[ATR_MAX_HISTORICAL];
  unsigned hbn;
} ATR_t;

//////////////////////////////////////////////////////////////////////////
typedef struct CCID_DESC
{
	/*
	 * ATR
	 */
	int nATRLength;
	UCHAR pcATRBuffer[MAX_ATR_SIZE];

	/*
	 * Card state
	 */
	UCHAR bPowerFlags;

	/*
	 * T=1 Protocol context
	 */
	t1_state_t t1;

	/* reader name passed to IFDHCreateChannelByName() */
	// char *readerName;
} CcidDesc;

/* Powerflag (used to detect quick insertion removals unnoticed by the
 * resource manager) */
/* Initial value */
#define POWERFLAGS_RAZ 			0x00
/* Flag set when a power up has been requested */
#define MASK_POWERFLAGS_PUP		0x01
/* Flag set when a power down is requested */
#define MASK_POWERFLAGS_PDWN	0x02
    
/*
 * Possible values :
 * 3 -> 1.8V, 3V, 5V
 * 2 -> 3V, 5V
 * 1 -> 5V only
 * 0 -> automatic (selection made by the reader)
 */
/*
 * To be safe we default to 5V
 * otherwise we would have to parse the ATR and get the value of TAi (i>2) when
 * in T=15
 */

#define VOLTAGE_AUTO	0
#define VOLTAGE_5V 		1
#define VOLTAGE_3V 		2
#define VOLTAGE_1_8V	3

////////////////////////////////////////////////////////////////////////////

extern RESPONSECODE CCID_Transmit(unsigned int reader_index, unsigned int tx_length,
                           const unsigned char tx_buffer[], unsigned short rx_length, unsigned char bBWI);
extern RESPONSECODE CCID_Receive(unsigned int reader_index, unsigned int *rx_length,
                                 unsigned char rx_buffer[], unsigned char *chain_parameter);
extern int isCharLevel(int reader_index);

extern RESPONSECODE CmdPowerOn(unsigned int reader_index, unsigned int * nlength,
                        unsigned char buffer[], int voltage);

extern RESPONSECODE CmdPowerOff(unsigned int reader_index);  
    
extern RESPONSECODE SetParameters(unsigned int reader_index, char protocol,
                                      unsigned int length, unsigned char buffer[]);
extern _ccid_descriptor *get_ccid_descriptor(unsigned int reader_index);

extern CcidDesc *get_ccid_slot(unsigned int reader_index);

extern RESPONSECODE CmdXfrBlock(unsigned int reader_index, unsigned int tx_length,
                             unsigned char tx_buffer[], unsigned int *rx_length,
                             unsigned char rx_buffer[], int protocol);
    
extern int ATR_InitFromArray (ATR_t * atr, const BYTE atr_buffer[ATR_MAX_SIZE], unsigned length);
    
extern int ATR_GetIntegerValue (ATR_t * atr, int name, BYTE * value);
    
extern int ATR_GetDefaultProtocol(ATR_t * atr, int *protocol);
    
extern int ATR_GetConvention (ATR_t * atr, int *convention);
    
extern int ATR_GetParameter (ATR_t * atr, int name, double *parameter);
    
extern RESPONSECODE  Scrd_Negotiate(unsigned int reader_index);
   
extern RESPONSECODE CmdGetSlotStatus(unsigned int reader_index, unsigned char buffer[]);  
    
extern int isbadreadptr(void *ptr, int length);

extern RESPONSECODE CmdGetSerialNum(unsigned int reader_index, unsigned int * pnlength,
                                        unsigned char buffer[]);
extern RESPONSECODE CmdWriteFlash(unsigned int reader_index,unsigned char bOffset, unsigned char blength,
                                      unsigned char buffer[]);
extern RESPONSECODE CmdReadFlash(unsigned int reader_index, unsigned char bOffset, unsigned char blength,
                                     unsigned char buffer[]);
extern RESPONSECODE CmdGetDevInfo(unsigned int reader_index, unsigned int * pnlength,
                               unsigned char buffer[]);
extern RESPONSECODE CmdGetDevVer(unsigned int reader_index,char *firmwareRevision,char *hardwareRevision);
    
extern  RESPONSECODE CmdWriteIKSN2IPEKFlash(unsigned int reader_index,unsigned char *encBuf,unsigned int nLen);

extern  RESPONSECODE CmdSetEncMod(unsigned int reader_index,unsigned int bEncrypt,unsigned int bEncFunc,
                                        unsigned int bEncType);    

extern  RESPONSECODE CmdGetKSN(unsigned int reader_index, unsigned int * pnlength,
                           unsigned char buffer[]);

extern  RESPONSECODE CmdEscape(unsigned int reader_index,
                          const unsigned char TxBuffer[], unsigned int TxLength,
                          unsigned char RxBuffer[], unsigned int *RxLength);
////////////////////////////////////////////////////////////////////////////
extern bool gIsOpen;
extern unsigned int gDevType;
extern unsigned int gIsReadData ;
extern unsigned int iR301_or_bR301;
extern unsigned int isDukpt;
extern void log_msg( const char *fmt, ...);
extern void Dpt( const char *fmt, ...);
extern void ccid_error(int error, const char *file, int line, const char *function);
    
   
#ifdef __APPLE__
#pragma pack(1)
#endif
    
/* restore default structure elements alignment */
#ifdef __APPLE__
#pragma pack()
#endif

#ifdef __cplusplus
    }
#endif

#endif //__HZ_CCID_H__
