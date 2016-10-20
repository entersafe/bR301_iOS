/*
* Support for bR301(Bluetooth) smart card reader
*
* ft301u.h: header file for ft_ccid.c
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

/**
 * @file
 * @This isfeitian's private cmd.
 */

#ifndef __ft301u_h__
#define __ft301u_h__


#include "wintypes.h"

#ifdef __cplusplus
extern "C"
{
#endif
	LONG SCARD_CTL_CODE(unsigned int code);
	/*
	Function: FtGetSerialNum

	Parameters:
	hCard 			IN 		Connection made from SCardConnect(Ignore this parameter and just set to zero in iOS system)
	length			IN		length of buffer(>=8)
	buffer       	OUT		Serial number

	Description:
	This function userd to get serial number of iR301.
	*/

	LONG FtGetSerialNum(SCARDHANDLE hCard, unsigned int  length,
	                    char * buffer);
	/*
	 Function: FtWriteFlash

	 Parameters:
	 hCard          IN 		Connection made from SCardConnect(Ignore this parameter and just set to zero in iOS system)
	 bOffset		IN		Offset of flash to write
	 blength		IN		The length of data
	 buffer       	IN		The data for write

	 Description:
	 This function userd to write data to flash.
	 */
	LONG FtWriteFlash(SCARDHANDLE hCard,unsigned char bOffset, unsigned char blength,
	                  unsigned char buffer[]);
	/*
	 Function: FtReadFlash

	 Parameters:
	 hCard 			IN 		Connection made from SCardConnect(Ignore this parameter and just set to zero in iOS system)
	 bOffset		IN		Offset of flash to write
	 blength		IN		The length of read data
	 buffer       	OUT		The read data

	 Description:
	 This function used to read data from flash.
	 */
	LONG FtReadFlash(SCARDHANDLE hCard,unsigned char bOffset, unsigned char blength,
	                 unsigned char buffer[]);

	/*
	 Function: FtSetTimeout

	 Parameters:
	    hContext	IN	 Connection context to the PC/SC Resource Manager
	    dwTimeout 	IN	 New transmission timeout value of between 301 and card (millisecond )

	 Description:
	 The function New transmission timeout value of between 301 and card.

	 */
	LONG FtSetTimeout(SCARDCONTEXT hContext, DWORD dwTimeout);


	//for dukpt
	/*
	 Function: FtDukptInit

	 Parameters:
	 hCard 		IN 	 Connection made from SCardConnect(Ignore this parameter and just set to zero in iOS system)
	 encBuf 	IN	 Ciphertext use TDES_ECB_PKCS7/AES_ECB_PKCS7 (See "Key C" )
	 nLen       IN	 encBuf length(40(TDES_ECB_PKCS7 ciphertext length) 48(AES_ECB_PKCS7 ciphertext length))

	 Description:
	 Init iR301 new ipek and ksn for dukpt.

	 */
	LONG FtDukptInit(SCARDHANDLE hCard,unsigned char *encBuf,unsigned int nLen);


	/*
	 Function: FtDukptSetEncMod

	 Parameters:
	 hCard 		IN 	 Connection made from SCardConnect(Ignore this parameter and just set to zero in iOS system)
	 bEncrypt 	IN	 1: SCardTransmit  Encrypted
	                 0:SCardTransmit not Encrypted

	 bEncFunc	IN	 0: Encryption functioon use TDES_ECB_PKCS7
	                 1:Encryption functioon use AES_ECB_PKCS7

	 bEncType	IN	 1: SCardTransmit: Plaintext in Ciphertext out
	                 0: SCardTransmit: Ciphertext in Ciphertext out

	 Description:
	 Set the encryption mode of iR301 for dukpt.

	 */
	LONG FtDukptSetEncMod(SCARDHANDLE hCard,
	                      unsigned int bEncrypt,
	                      unsigned int bEncFunc,
	                      unsigned int bEncType);




	/*
	 Function: FtDukptGetKSN

	 Parameters:
	 hCard 		IN      Connection made from SCardConnect(Ignore this parameter and just set to zero in iOS system)
	 pnlength 	INOUT	IN: The size of ksn buffer(>=10)
	                    OUT: The real size(if successful has been 10)
	 buffer 	OUT     Buffer of ksn

	 Description:
	 Get Ksn from iR301 for dukpt.

	 */

	LONG FtDukptGetKSN(SCARDHANDLE hCard,unsigned int * pnlength,unsigned char *buffer);
	/*
	 Function: FtDidEnterBackground

	 Parameters:
	 bDidEnter 	IN	 must be set 1


	 Description:
	 Use this method to release monitor thread of reader status

	 */
	void FtDidEnterBackground(unsigned int bDidEnter);
	/*
	 Function: FtSle4442Cmd

	 Parameters:
	 hCard 			IN 		Connection made from SCardConnect(Ignore this parameter and just set to zero in iOS system)
	 pbCmd          IN      SLE4442 cmd(3 byte) which are listed in sle4442 cmd table
	 bLengthToRead  IN      The data length which reading from the card
	 bIsClockNum    IN      If bIsClockNum = 1, then bToReadLength should be '1' and that means the returned one byte is
	 the clock number for card completing the command.
	 If bIsClockNum = 0, firmware don't care it.
	 pbRecvBuffer   OUT		Return data
	 pcbRecvLength	INOUT	IN:length of pbRecvBuffer OUT: length of return data

	 Description:
	 This function userd to transmit data with SLE4442 card
	 */

	LONG FtSle4442Cmd(SCARDHANDLE hCard,LPCBYTE pbCmd,DWORD bLengthToRead,BYTE bIsClockNum,LPBYTE pbRecvBuffer,LPDWORD pcbRecvLength);
	LONG FtGetDevVer( SCARDCONTEXT hContext,char *firmwareRevision,char *hardwareRevision);
	/*
	 Function: FtGetLibVersion

	 Parameters:
	 buffer :buffer of libVersion


	 Description:
	 Get the Current Lib Version

	 */
	void FtGetLibVersion (char *buffer);

	/*
	     Function: FtGetCurrentReaderType

	     Parameters:
	     readerType : Get reader type


	     Description:
	     Get the Current reader type

	     */
	LONG FtGetCurrentReaderType(unsigned int *readerType);

#ifdef __cplusplus
}
#endif

#endif

