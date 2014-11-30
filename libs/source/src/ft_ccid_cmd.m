/*
 * Support for bR301(Bluetooth) smart card reader
 *
 * ft_ccid_cmd.m: The code has moved from CCID Driver, which is open souce code by GNU license (http://pcsclite.alioth.debian.org/ccid.html)
 * Some parts has integrated private command, these command only using for Feitian bluetooth and iDock reader,like ComdGetSerialNum,CmdGetSerialNum,CmdGetDevInfo,CmdWriteFlash,CmdReadFlash,CmdWriteIKSN2IPEKFlash,CmdGetKSN,CmdSetEncMod
 *
 * And the code was apply commands.c file which is GNU CCID open source code
 *
 * $Id: commands.c 6879 2014-03-23 15:11:20Z rousseau $
 *
 * The file has packaged PC/SC API by Objective-C Lang, user can called Objective-C interface to monitor
 * card and reader event
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



#include "ft_ccid.h"
#include "wintypes.h"
#include "proto-t1.h"
#include  <pthread.h>

#import "EADSessionController.h"

static _ccid_descriptor s_ccidDevice[CCID_DRIVER_MAX_READERS]={0};

static CcidDesc  s_CcidSlots[CCID_DRIVER_MAX_READERS]={0};

#pragma mark ccid tool func
/*****************************************************************************
 *
 *					i2dw
 *
 ****************************************************************************/

static void i2dw(int value, unsigned char buffer[])
{
	buffer[0] = value & 0xFF;
	buffer[1] = (value >> 8) & 0xFF;
	buffer[2] = (value >> 16) & 0xFF;
	buffer[3] = (value >> 24) & 0xFF;
} /* i2dw */

/*****************************************************************************
*
*                  bei2i (big endian integer to host order interger)
*
****************************************************************************/

static unsigned int bei2i(unsigned char buffer[])
{
	return (buffer[0]<<24) + (buffer[1]<<16) + (buffer[2]<<8) + buffer[3];
}

/*****************************************************************************
 *
 *                  LOCK
 *
 ****************************************************************************/

//int  ftLock(int condition)
//{
//
//   EADSessionController *sessionController = [EADSessionController sharedController];
//    return  [sessionController myLock:condition];
//}


/*****************************************************************************
 *
 *                  UNLOCK
 *
 ****************************************************************************/

//int  ftUnLock(int condition)
//{
//
//    EADSessionController *sessionController = [EADSessionController sharedController];
//    return  [sessionController myUnLock:condition];
//}


/*****************************************************************************
 *
 *					get_ccid_descriptor
 *
 ****************************************************************************/
_ccid_descriptor *get_ccid_descriptor(unsigned int reader_index)
{
	return &s_ccidDevice[0];
} /* get_ccid_descriptor */


CcidDesc *get_ccid_slot(unsigned int reader_index)
{
	return &s_CcidSlots[0];
} /* get_ccid_slot */


/*****************************************************************************
 *
 *				ReadSerial: Receive bytes from the card reader
 *
 *****************************************************************************/
status_t ReadSerial(unsigned int reader_index,
	unsigned int *length, unsigned char *buffer)
{
 @try {
    
        int iRet=0;
    
        EADSessionController *sessionController = [EADSessionController sharedController];
        gIsReadData = 1;
        iRet = [sessionController readData: buffer withbytesToRead:length];
        gIsReadData = 0;
        if(iRet != 0)
        {
#ifdef FT_IR301_DEBUG
            //NSLog(@"==========read error\n");
#endif
            return STATUS_COMM_ERROR;
        }
        else
        {
#ifdef FT_IR301_DEBUG
            NSMutableData *tmpData = [NSMutableData data];
            [tmpData appendBytes:buffer length:*length];
        
            NSString* dataString= [NSString stringWithFormat:@"%@",tmpData];
            NSRange begin = [dataString rangeOfString:@"<"];
            NSRange end = [dataString rangeOfString:@">"];
            NSRange range = NSMakeRange(begin.location + begin.length, end.location- begin.location - 1);
            dataString = [dataString substringWithRange:range];
            NSLog(@"==========read:\n%@",dataString);
#endif
            return STATUS_SUCCESS;
        }
    }
    @catch (...) {
        
        return STATUS_COMM_ERROR;
    }
    @finally {
        
    }
   
}

/*****************************************************************************
 *
 *				WriteSerial: Send bytes to the card reader
 *
 *****************************************************************************/
status_t WriteSerial(unsigned int reader_index, unsigned int length,
                     unsigned char *buffer)
{
    @try {
               int iRet=0;
    
                EADSessionController *sessionController = [EADSessionController sharedController];
       
                iRet = [sessionController writeData: buffer withLength:length];
        
#ifdef FT_IR301_DEBUG
                NSMutableData *tmpData = [NSMutableData data];
                [tmpData appendBytes:buffer length:length];
        
                NSString* dataString= [NSString stringWithFormat:@"%@",tmpData];
                NSRange begin = [dataString rangeOfString:@"<"];
                NSRange end = [dataString rangeOfString:@">"];
                NSRange range = NSMakeRange(begin.location + begin.length, end.location- begin.location - 1);
                dataString = [dataString substringWithRange:range];
                NSLog(@"==========write:\n%@",dataString);
#endif
        
        
        if (iRet == 0 )
        {
            return STATUS_SUCCESS;
        }
        else 
        {
            
            return STATUS_COMM_ERROR;
        }
    
    }
    @catch (...) {

        return STATUS_COMM_ERROR;
    }
    @finally {
        
    }

}

#pragma mark feitian's private tool cmd

RESPONSECODE CmdGetDevVer(unsigned int reader_index,char *firmwareRevision,char *hardwareRevision)
{
    if (gIsOpen == NO) {
        return IFD_ERROR_TAG;
    }
    
    EADSessionController *sessionController = [EADSessionController sharedController];
    NSString *tmp=[[sessionController accessory] firmwareRevision];
    if (iR301_or_bR301 == 0)
    {
        if (tmp != nil) {
            strcpy(firmwareRevision,[tmp UTF8String]);
        }
        else{
            return IFD_ERROR_TAG;
        }
    }
    
    tmp=[[sessionController accessory] hardwareRevision];
    if (tmp != nil) {
        strcpy(hardwareRevision,[tmp UTF8String]);
    }
    else{
        return IFD_ERROR_TAG;
    }
    
    
    if (iR301_or_bR301 == 1)
    {
        unsigned char cmd[10+6]={0};
        unsigned char tmp[10+32]={0};
        status_t res;
        unsigned int length = 0;
        
        _ccid_descriptor *ccid_descriptor = get_ccid_descriptor(reader_index);
        cmd[0] = 0x6B; /* feitian private cmd */
        cmd[1] = 0x04;
        cmd[2] = cmd[3] = cmd[4] = 0;	/* dwLength */
        cmd[5] = ccid_descriptor->bCurrentSlotIndex;	/* slot number */
        cmd[6] = ccid_descriptor->real_bSeq++;
        cmd[7] = cmd[8] = cmd[9] = 0; /* RFU */
        cmd[10] = 0x5A;
        cmd[11] = 0xA5;
        cmd[12] = 0x20;
        cmd[13] = 0x00;
        
        res = WritePort(reader_index, 10+4, cmd);
        if (res != STATUS_SUCCESS)
            return IFD_COMMUNICATION_ERROR;
        
        length = sizeof(tmp);
        res = ReadPort(reader_index, &length, tmp);
        if (res != STATUS_SUCCESS)
            return IFD_COMMUNICATION_ERROR;
        
        if (length < STATUS_OFFSET+1)
        {
            return IFD_COMMUNICATION_ERROR;
        }
        
        if(length < 10+4)
        {
            
            return IFD_COMMUNICATION_ERROR;
        }
        
        sprintf(firmwareRevision, "%x.%02x",tmp[10],tmp[11]);
        
    }
    
	return IFD_SUCCESS;
    
	return IFD_SUCCESS;
}

/*****************************************************************************
 *
 *					CmdGetSerialNum  this is a feitian's private cmd
 *
 ****************************************************************************/
RESPONSECODE CmdGetSerialNum(unsigned int reader_index, unsigned int * pnlength,
                             unsigned char buffer[])
{
	unsigned char cmd[10+4]={0};
    unsigned char tmp[10+300]={0};
	status_t res;
	unsigned int length = 0;
	RESPONSECODE return_value = IFD_SUCCESS;
	_ccid_descriptor *ccid_descriptor = get_ccid_descriptor(reader_index);
    
	cmd[0] = 0x6B;                                  /* feitian private cmd */
	cmd[1] = 0x04; 
    cmd[2] = cmd[3] = cmd[4] = 0;                   /* dwLength */
	cmd[5] = ccid_descriptor->bCurrentSlotIndex;	/* slot number */
	cmd[6] = ccid_descriptor->real_bSeq++;
	cmd[7] = cmd[8] = cmd[9] = 0;                   /* RFU */
    cmd[10] = 0x5A;
    cmd[11] = 0xA5;
    cmd[12] = 0x31;
    cmd[13] = 0x31;
    
    
	res = WritePort(reader_index, 10+4, cmd);
	if (res != STATUS_SUCCESS)
		return IFD_COMMUNICATION_ERROR;
    
    length = *pnlength;
    
	res = ReadPort(reader_index, &length, tmp);
   
	if (res != STATUS_SUCCESS)
		return IFD_COMMUNICATION_ERROR;
    
	if (length < STATUS_OFFSET+1)
	{
		return IFD_COMMUNICATION_ERROR;
	}
    
	if (tmp[STATUS_OFFSET] & CCID_COMMAND_FAILED)
	{
        ccid_error(tmp[ERROR_OFFSET], __FILE__, __LINE__, __FUNCTION__);
        
		return IFD_COMMUNICATION_ERROR;
        
	}

    if(length < 10+ 16)
    {
     
        return IFD_COMMUNICATION_ERROR;
    }
    
    *pnlength = 16;
	memcpy(buffer, tmp+10, 16);
    
	return return_value;
} /* CmdGetSerialNum */


/*****************************************************************************
 *
 *					CmdGetDevInfo  this is a feitian's private cmd
 *
 ****************************************************************************/
RESPONSECODE CmdGetDevInfo(unsigned int reader_index, unsigned int * pnlength,
                             unsigned char buffer[])
{
	unsigned char cmd[10+4]={0};
    unsigned char tmp[10+300]={0};
	status_t res;
	unsigned int length = 0;
	RESPONSECODE return_value = IFD_SUCCESS;
	_ccid_descriptor *ccid_descriptor = get_ccid_descriptor(reader_index);
	cmd[0] = 0x6B;                                  /* feitian private cmd */
	cmd[1] = 0x02;
    cmd[2] = cmd[3] = cmd[4] = 0;                   /* dwLength */
	cmd[5] = ccid_descriptor->bCurrentSlotIndex;	/* slot number */
	cmd[6] = ccid_descriptor->real_bSeq++;
	cmd[7] = cmd[8] = cmd[9] = 0;                   /* RFU */
    cmd[10] = 0x5A;
    cmd[11] = 0xA1;
    
    
	res = WritePort(reader_index, 10+2, cmd);
	if (res != STATUS_SUCCESS)
		return IFD_COMMUNICATION_ERROR;
    
    length = *pnlength;
    
	res = ReadPort(reader_index, &length, tmp);
	if (res != STATUS_SUCCESS)
		return IFD_COMMUNICATION_ERROR;
    
	if (length < STATUS_OFFSET+1)
	{
		return IFD_COMMUNICATION_ERROR;
	}

	if (tmp[STATUS_OFFSET] & CCID_COMMAND_FAILED)
	{

        ccid_error(tmp[ERROR_OFFSET], __FILE__, __LINE__, __FUNCTION__);
        
		return IFD_COMMUNICATION_ERROR;
        
	}
    
    if(length < 10+ 10)
    {
        
        return IFD_COMMUNICATION_ERROR;
    }
    
    *pnlength = 10;
	memcpy(buffer, tmp+10, 10);
    
	return return_value;
} /* CmdGetDevInfo */

/*****************************************************************************
 *
 *					CmdWriteFlash this is a feitian's private cmd
 *
 ****************************************************************************/
RESPONSECODE CmdWriteFlash(unsigned int reader_index,unsigned char bOffset, unsigned char blength,unsigned char buffer[])
{
	unsigned char cmd[10+4]={0};
    unsigned char tmp[10+300]={0};
	status_t res;
	unsigned int length=sizeof(cmd);
    
	RESPONSECODE return_value = IFD_SUCCESS;
	_ccid_descriptor *ccid_descriptor = get_ccid_descriptor(reader_index);
    
    //first read flash
    if (bOffset == 0 && blength == 255) 
    {
        memcpy(tmp+10+bOffset,buffer,blength);
    }
    else 
    {
        return_value = CmdReadFlash( reader_index,0,255,tmp+10);
        
        if (IFD_SUCCESS != return_value) 
        {
            return return_value;
        }
        memcpy(tmp+10+bOffset,buffer,blength);
        
        bOffset = 0;
        blength = 255;
  
    }
       
	cmd[0] = 0x6B;                                  /* feitian private cmd */
	cmd[1] = 0x04; 
    cmd[2] = cmd[3] = cmd[4] = 0;                   /* dwLength */
	cmd[5] = ccid_descriptor->bCurrentSlotIndex;	/* slot number */
	cmd[6] = ccid_descriptor->real_bSeq++;
	cmd[7] = cmd[8] = cmd[9] = 0;                   /* RFU */
    cmd[10] = 0x5A;
    cmd[11] = 0xA6;
    cmd[12] = blength;
    cmd[13] = bOffset;
    
    
	res = WritePort(reader_index, 10+4, cmd);
	if (res != STATUS_SUCCESS)
		return IFD_COMMUNICATION_ERROR;
    
	res = ReadPort(reader_index, &length, cmd);
	if (res != STATUS_SUCCESS)
		return IFD_COMMUNICATION_ERROR;
    
	if (length < STATUS_OFFSET+1)
	{
		return IFD_COMMUNICATION_ERROR;
	}

	if (cmd[STATUS_OFFSET] & CCID_COMMAND_FAILED)
	{
        ccid_error(buffer[ERROR_OFFSET], __FILE__, __LINE__, __FUNCTION__);
   
		return IFD_COMMUNICATION_ERROR;
        
	}
    
    //now write flash data
    
	cmd[0] = 0x6B;                                  /* feitian private cmd */
	cmd[1] = blength; 
    cmd[2] = cmd[3] = cmd[4] = 0;                   /* dwLength */
	cmd[5] = ccid_descriptor->bCurrentSlotIndex;	/* slot number */
	cmd[6] = ccid_descriptor->real_bSeq++;
	cmd[7] = cmd[8] = cmd[9] = 0;                   /* RFU */
    
    memcpy(tmp, cmd, 10);
    
	res = WritePort(reader_index,blength+10, tmp);
	if (res != STATUS_SUCCESS)
		return IFD_COMMUNICATION_ERROR;
    
    length=sizeof(tmp);
	res = ReadPort(reader_index, &length, tmp);
	if (res != STATUS_SUCCESS)
		return IFD_COMMUNICATION_ERROR;
    
	if (length < STATUS_OFFSET+1)
	{
		return IFD_COMMUNICATION_ERROR;
	}
    
    if (tmp[STATUS_OFFSET] & CCID_COMMAND_FAILED)
	{
        ccid_error(cmd[ERROR_OFFSET], __FILE__, __LINE__, __FUNCTION__);
		return IFD_COMMUNICATION_ERROR;
        
	}

	return return_value;
} /* CmdWriteFlash */



/*****************************************************************************
 *
 *					CmdReadFlash this is a feitian's private cmd
 *
 ****************************************************************************/
RESPONSECODE CmdReadFlash(unsigned int reader_index, unsigned char bOffset, unsigned char blength,unsigned char buffer[])
{
	unsigned char cmd[10+300]={0};
	status_t res;
	unsigned int length=sizeof(cmd);
    
	RESPONSECODE return_value = IFD_SUCCESS;
	_ccid_descriptor *ccid_descriptor = get_ccid_descriptor(reader_index);
    
    
	cmd[0] = 0x6B;                                  /* feitian private cmd */
	cmd[1] = 0x04; 
    cmd[2] = cmd[3] = cmd[4] = 0;                   /* dwLength */
	cmd[5] = ccid_descriptor->bCurrentSlotIndex;	/* slot number */
	cmd[6] = ccid_descriptor->real_bSeq++;
	cmd[7] = 0;
	cmd[8] = cmd[9] = 0;                            /* RFU */
    cmd[10] = 0x5A;
    cmd[11] = 0xA7;
    cmd[12] = blength;
    cmd[13] = bOffset;
    
    
	res = WritePort(reader_index, 10+4, cmd);
	if (res != STATUS_SUCCESS)
		return IFD_COMMUNICATION_ERROR;
    
    
	res = ReadPort(reader_index, &length, cmd);
	if (res != STATUS_SUCCESS)
		return IFD_COMMUNICATION_ERROR;
    
	if (length < STATUS_OFFSET+1)
	{
		return IFD_COMMUNICATION_ERROR;
	}
    
	if (cmd[STATUS_OFFSET] & CCID_COMMAND_FAILED)
	{
        ccid_error(cmd[ERROR_OFFSET], __FILE__, __LINE__, __FUNCTION__);
        
		return IFD_COMMUNICATION_ERROR;
        
	}
    
    if(length < 10+ blength)
    {
        return IFD_COMMUNICATION_ERROR;
    }
    
    
	memcpy(buffer, cmd+10, blength);
    
	return return_value;
} /* CmdReadFlash */



#pragma mark feitian's private duktp cmd

/*****************************************************************************
 *
 *		CmdWriteIKSN2IPEKFlash this is a feitian's private cmd of dukpt
 *
 ****************************************************************************/
RESPONSECODE CmdWriteIKSN2IPEKFlash(unsigned int reader_index,unsigned char *encBuf,unsigned int nLen)
{
	unsigned char cmd[10+6]={0};
    unsigned char tmp[10+64]={0};
	status_t res;
	unsigned int length;
    
	RESPONSECODE return_value = IFD_SUCCESS;
	_ccid_descriptor *ccid_descriptor = get_ccid_descriptor(reader_index);
    
	cmd[0] = 0x6B;                                  /* feitian private cmd */
	cmd[1] = 4+nLen;
    cmd[2] = cmd[3] = cmd[4] = 0;                   /* dwLength */
	cmd[5] = ccid_descriptor->bCurrentSlotIndex;	/* slot number */
	cmd[6] = ccid_descriptor->real_bSeq++;
	cmd[7] = cmd[8] = cmd[9] = 0;                   /* RFU */
    cmd[10] = 0x5B;
    cmd[11] = 0x02;
    cmd[12] = 0x00;                                 //private lenght h
    cmd[13] = nLen;                                 //private lenght l
    
    memcpy(tmp, cmd, 14);
    memcpy(tmp+14, encBuf, nLen);
	res = WritePort(reader_index,10+4+nLen, tmp);
	if (res != STATUS_SUCCESS)
		return IFD_COMMUNICATION_ERROR;
    
    length=sizeof(tmp);
	res = ReadPort(reader_index, &length, tmp);
	if (res != STATUS_SUCCESS)
		return IFD_COMMUNICATION_ERROR;
    
	if (length < STATUS_OFFSET+1)
	{
		return IFD_COMMUNICATION_ERROR;
	}

	/*if (tmp[STATUS_OFFSET] & CCID_COMMAND_FAILED)
	{
        ccid_error(cmd[ERROR_OFFSET], __FILE__, __LINE__, __FUNCTION__);
		return IFD_COMMUNICATION_ERROR;
        
	}*/
    if(length < 10+ 6)
    {
        return IFD_COMMUNICATION_ERROR;
    }
    
    // Can not judge
#if 0
    if (tmp[10] != 0xB5 ||
        tmp[11] != 0x02 ||
        tmp[12] != 0x00 ||
        tmp[13] != 0x02 )
    {
        return IFD_COMMUNICATION_ERROR;
    }
#endif
    //
    if (tmp[14] != 0x90 ||
        tmp[15] != 0x00)
    {
        return IFD_COMMUNICATION_ERROR;
    }
	return return_value;
} /* CmdWriteIKSN2IPEKFlash */

/*****************************************************************************
 *
 *		CmdGetKSN  this is a feitian's private cmd of dukpt
 *
 ****************************************************************************/
RESPONSECODE CmdGetKSN(unsigned int reader_index, unsigned int * pnlength,
                             unsigned char buffer[])
{
	unsigned char cmd[10+32]={0};
    unsigned char tmp[10+64]={0};
	status_t res;
	unsigned int length = 0;
	RESPONSECODE return_value = IFD_SUCCESS;
	_ccid_descriptor *ccid_descriptor = get_ccid_descriptor(reader_index);
    
	cmd[0] = 0x6B;                                  /* feitian private cmd */
	cmd[1] = 0x04;
    cmd[2] = cmd[3] = cmd[4] = 0;                   /* dwLength */
	cmd[5] = ccid_descriptor->bCurrentSlotIndex;	/* slot number */
	cmd[6] = ccid_descriptor->real_bSeq++;
	cmd[7] = cmd[8] = cmd[9] = 0;                   /* RFU */
    cmd[10] = 0x5B;
    cmd[11] = 0x03;
    cmd[12] = 0x00;                                 //private lenght h
    cmd[13] = 0x00;                                 //private lenght l
    

	res = WritePort(reader_index, 10+4, cmd);
	if (res != STATUS_SUCCESS)
		return IFD_COMMUNICATION_ERROR;
    
    length = *pnlength;
    
	res = ReadPort(reader_index, &length, tmp);
	if (res != STATUS_SUCCESS)
		return IFD_COMMUNICATION_ERROR;
    
	if (length < STATUS_OFFSET+1)
	{
		return IFD_COMMUNICATION_ERROR;
	}

	/*if (tmp[STATUS_OFFSET] & CCID_COMMAND_FAILED)
	{
        ccid_error(buffer[ERROR_OFFSET], __FILE__, __LINE__, __FUNCTION__);
        
		return IFD_COMMUNICATION_ERROR;
	}*/
    
    if(length < 10+ 16)
    {
        return IFD_COMMUNICATION_ERROR;
    }
    // Can not judge
#if 0
    if (buffer[10] != 0xB5 ||
        buffer[11] != 0x03 ||
        buffer[12] != 0x00 ||
        buffer[13] != 0x0C ||
        buffer[24] != 0x90 ||
        buffer[25] != 0x00)
    {
       return IFD_COMMUNICATION_ERROR; 
    }
#endif
    
    *pnlength = 10;
	memcpy(buffer, tmp+14, 10);
    
	return return_value;
} /* CmdGetKSN */


/*****************************************************************************
 *
 *		CmdSetEncryptMod  this is a feitian's private cmd of dukpt
 *
 ****************************************************************************/
RESPONSECODE CmdSetEncMod(unsigned int reader_index,
                          unsigned int bEncrypt,
                          unsigned int bEncFunc,
                          unsigned int bEncType)
{
	unsigned char cmd[10+32]={0};
	status_t res;
	unsigned int length = 0;
	RESPONSECODE return_value = IFD_SUCCESS;
    BYTE Encrypt = (bEncrypt == DUKPT_ENC_OFF ? DUKPT_ENC_OFF: DUKPT_ENC_ON);
    BYTE EncFunc = (bEncFunc == DUKPT_ENC_FUNC_3DES ? DUKPT_ENC_FUNC_3DES: DUKPT_ENC_FUNC_AES);
    BYTE EncType = (bEncType == DUKPT_ENC_TYPE_DIR ? DUKPT_ENC_TYPE_DIR: DUKPT_ENC_TYPE_UNDIR);
    
	_ccid_descriptor *ccid_descriptor = get_ccid_descriptor(reader_index);
    
	cmd[0] = 0x6B;                                  /* feitian private cmd */
	cmd[1] = 0x07;
    cmd[2] = cmd[3] = cmd[4] = 0;                   /* dwLength */
	cmd[5] = ccid_descriptor->bCurrentSlotIndex;	/* slot number */
	cmd[6] = ccid_descriptor->real_bSeq++;
	cmd[7] = cmd[8] = cmd[9] = 0;                   /* RFU */
    cmd[10] = 0x5B;
    cmd[11] = 0x01;
    cmd[12] = 0x00;                                 //private lenght h
    cmd[13] = 0x03;                                 //private lenght
    cmd[14] = Encrypt;
    cmd[15] = EncFunc;
    cmd[16] = EncType;
    
	res = WritePort(reader_index, 10+7, cmd);
	if (res != STATUS_SUCCESS)
		return IFD_COMMUNICATION_ERROR;
    
    length = sizeof(cmd);
    
	res = ReadPort(reader_index, &length, cmd);

	if (res != STATUS_SUCCESS){
      
		return IFD_COMMUNICATION_ERROR;
    }
    
	if (length < STATUS_OFFSET+1)
	{
        
		return IFD_COMMUNICATION_ERROR;
	}

	/*if (cmd[STATUS_OFFSET] & CCID_COMMAND_FAILED)
	{
        ccid_error(cmd[ERROR_OFFSET], __FILE__, __LINE__, __FUNCTION__);
        
		return IFD_COMMUNICATION_ERROR;
	}*/
    
    if (length < 10 + 9) {
     
        return IFD_COMMUNICATION_ERROR;
    }
    
    
    // Can not judge
#if 0
    if (cmd[10] != 0xB5 ||
        cmd[11] != 0x01 ||
        cmd[12] != 0x00 ||
        cmd[13] != 0x05 ||
        cmd[17] != 0x90 ||
        cmd[18] != 0x00)
    {
        return IFD_COMMUNICATION_ERROR;
    }
#endif
    
    if (cmd[14] != Encrypt ||
        cmd[15] != EncFunc ||
        cmd[16] != EncType )
    {
        return IFD_ERROR_TAG;
    }
   
	return return_value;
} /* CmdSetEncryptMod */


#pragma mark ccid's sub func

/*****************************************************************************
 *
 *					CmdGetSlotStatus
 *
 ****************************************************************************/
RESPONSECODE CmdGetSlotStatus(unsigned int reader_index, unsigned char buffer[])
{
	unsigned char cmd[10];
	status_t res;
	unsigned int length;
	RESPONSECODE return_value = IFD_SUCCESS;
	_ccid_descriptor *ccid_descriptor = get_ccid_descriptor(reader_index);
    
	cmd[0] = 0x65;                                  /* GetSlotStatus */
	cmd[1] = cmd[2] = cmd[3] = cmd[4] = 0;          /* dwLength */
	cmd[5] = ccid_descriptor->bCurrentSlotIndex;	/* slot number */
	cmd[6] = ccid_descriptor->real_bSeq++;
	cmd[7] = cmd[8] = cmd[9] = 0;                   /* RFU */
    
	res = WritePort(reader_index, sizeof(cmd), cmd);
	if (res != STATUS_SUCCESS)
	{
		if (STATUS_NO_SUCH_DEVICE == res)
			return IFD_NO_SUCH_DEVICE;
		return IFD_COMMUNICATION_ERROR;
	}
    
	length = SIZE_GET_SLOT_STATUS;
	res = ReadPort(reader_index, &length, buffer);
	if (res != STATUS_SUCCESS)
		return IFD_COMMUNICATION_ERROR;
    
	if (length < STATUS_OFFSET+1)
	{
		return IFD_COMMUNICATION_ERROR;
	}
    
	if (buffer[STATUS_OFFSET] & CCID_COMMAND_FAILED)
	{
        /* card absent or mute is not an communication error */
        if ((buffer[ERROR_OFFSET] != 0xFE))
        {
            return_value = IFD_COMMUNICATION_ERROR;
        }
        else
        {
            ccid_descriptor->dwSlotStatus = IFD_ICC_NOT_PRESENT;
        }
	}
    
	return return_value;
} /* CmdGetSlotStatus */


/*****************************************************************************
 *
 *					CmdPowerOff
 *
 ****************************************************************************/
RESPONSECODE CmdPowerOff(unsigned int reader_index)
{
	unsigned char cmd[10];
	status_t res;
	unsigned int length;
	RESPONSECODE return_value = IFD_SUCCESS;
	_ccid_descriptor *ccid_descriptor = get_ccid_descriptor(reader_index);
    
	cmd[0] = 0x63;                                  /* IccPowerOff */
	cmd[1] = cmd[2] = cmd[3] = cmd[4] = 0;          /* dwLength */
	cmd[5] = ccid_descriptor->bCurrentSlotIndex;	/* slot number */
	cmd[6] = ccid_descriptor->real_bSeq++;
	cmd[7] = cmd[8] = cmd[9] = 0;                   /* RFU */
    
	res = WritePort(reader_index, sizeof(cmd), cmd);
	if (res != STATUS_SUCCESS)
		return IFD_COMMUNICATION_ERROR;
    
	length = sizeof(cmd);
	res = ReadPort(reader_index, &length, cmd);
	if (res != STATUS_SUCCESS)
		return IFD_COMMUNICATION_ERROR;
    
	if (length < STATUS_OFFSET+1)
	{
		return IFD_COMMUNICATION_ERROR;
	}
    
	if (cmd[STATUS_OFFSET] & CCID_COMMAND_FAILED)
	{
        if (cmd[ERROR_OFFSET] == 0xFE) {
            
            ccid_descriptor->dwSlotStatus = IFD_ICC_NOT_PRESENT;
        }
		return_value = IFD_COMMUNICATION_ERROR;
	}
	
	return return_value;
} /* CmdPowerOff */

/*****************************************************************************
 *
 *					Escape
 *
 ****************************************************************************/
RESPONSECODE CmdEscape(unsigned int reader_index,
                        const unsigned char TxBuffer[], unsigned int TxLength,
                        unsigned char RxBuffer[], unsigned int *RxLength)
{
    unsigned char cmd[10+300]={0};
	status_t res;
	unsigned int length=sizeof(cmd);
    unsigned int nLen=0;
    
	RESPONSECODE return_value = IFD_SUCCESS;
	_ccid_descriptor *ccid_descriptor = get_ccid_descriptor(reader_index);
    
    nLen = TxLength & 0xFF;
    
	cmd[0] = 0x6B; 
	cmd[1] = nLen;
    cmd[2] = cmd[3] = cmd[4] = 0;                   /* dwLength */
	cmd[5] = ccid_descriptor->bCurrentSlotIndex;	/* slot number */
	cmd[6] = ccid_descriptor->real_bSeq++;
	cmd[7] = 0;
	cmd[8] = cmd[9] = 0;                            /* RFU */
    memcpy(cmd+10, TxBuffer, nLen);
    
	res = WritePort(reader_index, 10+nLen, cmd);
	if (res != STATUS_SUCCESS)
		return IFD_COMMUNICATION_ERROR;
    
    
	res = ReadPort(reader_index, &length, cmd);
	if (res != STATUS_SUCCESS)
		return IFD_COMMUNICATION_ERROR;
    
	if (length < STATUS_OFFSET+1)
	{
		return IFD_COMMUNICATION_ERROR;
	}

	if (cmd[STATUS_OFFSET] & CCID_COMMAND_FAILED)
	{
        ccid_error(cmd[ERROR_OFFSET], __FILE__, __LINE__, __FUNCTION__);
        
		return IFD_COMMUNICATION_ERROR;
	}
    
    if (length > 10)
    {
        nLen = length -10;
        if(*RxLength < nLen)
        {
            return IFD_NOT_SUPPORTED;
        }
        memcpy(RxBuffer, cmd+10, nLen);
        *RxLength=nLen;
    }
    else
    {
        *RxLength = 0;
    }
    
	return return_value;

} /* Escape */


/*****************************************************************************
 *
 *					Escape
 *
 ****************************************************************************/
RESPONSECODE CmdEscape0(unsigned int reader_index,
                       const unsigned char TxBuffer[], unsigned int TxLength,
                       unsigned char RxBuffer[], unsigned int *RxLength)
{
	unsigned char *cmd_in, *cmd_out;
	status_t res;
	unsigned int length_in, length_out;
	RESPONSECODE return_value = IFD_SUCCESS;
	int old_read_timeout;
	_ccid_descriptor *ccid_descriptor = get_ccid_descriptor(reader_index);
    
	old_read_timeout = ccid_descriptor -> readTimeout;
	ccid_descriptor -> readTimeout = 30*1000;	/* 30 seconds */
    
again:
	/* allocate buffers */
	length_in = 10 + TxLength;
	if (NULL == (cmd_in = (BYTE*)malloc(length_in)))
	{
		return_value = IFD_COMMUNICATION_ERROR;
		goto end;
	}
    
	length_out = 10 + *RxLength;
	if (NULL == (cmd_out = (BYTE*)malloc(length_out)))
	{
		free(cmd_in);
		return_value = IFD_COMMUNICATION_ERROR;
		goto end;
	}
    
	cmd_in[0] = 0x6B;                               /* PC_to_RDR_Escape */
	i2dw(length_in - 10, cmd_in+1);                 /* dwLength */
	cmd_in[5] = ccid_descriptor->bCurrentSlotIndex;	/* slot number */
	cmd_in[6] = ccid_descriptor->real_bSeq;
	cmd_in[7] = cmd_in[8] = cmd_in[9] = 0;          /* RFU */
    
	/* copy the command */
	memcpy(&cmd_in[10], TxBuffer, TxLength);
    
	res = WritePort(reader_index, length_in, cmd_in);
	free(cmd_in);
	if (res != STATUS_SUCCESS)
	{
		free(cmd_out);
		return_value = IFD_COMMUNICATION_ERROR;
		goto end;
	}
    
	res = ReadPort(reader_index, &length_out, cmd_out);
    
	/* replay the command if NAK
	 * This (generally) happens only for the first command sent to the reader
	 * with the serial protocol so it is not really needed for all the other
	 * ReadPort() calls */
	if (STATUS_COMM_NAK == res)
	{
		free(cmd_out);
		goto again;
	}
    
	if (res != STATUS_SUCCESS)
	{
		free(cmd_out);
		return_value = IFD_COMMUNICATION_ERROR;
		goto end;
	}
    
	if (length_out < STATUS_OFFSET+1)
	{
		return_value = IFD_COMMUNICATION_ERROR;
		goto end;
	}
    
	if (cmd_out[STATUS_OFFSET] & CCID_COMMAND_FAILED)
	{
        if (cmd_out[ERROR_OFFSET] == 0xFE) {
            
            ccid_descriptor->dwSlotStatus = IFD_ICC_NOT_PRESENT;
        }
		return_value = IFD_COMMUNICATION_ERROR;
	}
    
	/* copy the response */
	length_out = dw2i(cmd_out, 1);
	if (length_out > *RxLength)
		length_out = *RxLength;
	*RxLength = length_out;
	memcpy(RxBuffer, &cmd_out[10], length_out);
    
	free(cmd_out);
    
end:
	ccid_descriptor -> readTimeout = old_read_timeout;
	return return_value;
} /* Escape */

/*****************************************************************************
 *
 *					CmdPowerOn
 *
 ****************************************************************************/
RESPONSECODE CmdPowerOn(unsigned int reader_index, unsigned int * nlength,
                        unsigned char buffer[], int voltage)
{
	unsigned char cmd[10];
	status_t res;
	int length=64;
	unsigned int atr_len;
	RESPONSECODE return_value = IFD_SUCCESS;
	_ccid_descriptor *ccid_descriptor = get_ccid_descriptor(reader_index);
    
	cmd[0] = 0x62;                                  /* IccPowerOn */
	cmd[1] = cmd[2] = cmd[3] = cmd[4] = 0;          /* dwLength */
	cmd[5] = ccid_descriptor->bCurrentSlotIndex;	/* slot number */
	cmd[6] = ccid_descriptor->real_bSeq++;
	cmd[7] = voltage;
	cmd[8] = cmd[9] = 0;                            /* RFU */
    
	res = WritePort(reader_index, sizeof(cmd), cmd);
	if (res != STATUS_SUCCESS)
		return IFD_COMMUNICATION_ERROR;
    
	/* reset available buffer size */
	/* needed if we go back after a switch to ISO mode */
	*nlength = length;
    
	res = ReadPort(reader_index, nlength, buffer);
	if (res != STATUS_SUCCESS)
		return IFD_COMMUNICATION_ERROR;
    
	if (*nlength < STATUS_OFFSET+1)
	{
		return IFD_COMMUNICATION_ERROR;
	}
    
	if (buffer[STATUS_OFFSET] & CCID_COMMAND_FAILED)
	{

        // here voltage
        //VOLTAGE_3V/*VOLTAGE_AUTO/*VOLTAGE_5V*/
        if (buffer[ERROR_OFFSET] == 0xFE) {
            
            ccid_descriptor->dwSlotStatus = IFD_ICC_NOT_PRESENT;
        }
		return IFD_COMMUNICATION_ERROR;
        
	}
    
	/* extract the ATR */
	atr_len = dw2i(buffer, 1);	/* ATR length */
	if (atr_len > *nlength)
		atr_len = *nlength;
	else
		*nlength = atr_len;
    
	memmove(buffer, buffer+10, atr_len);
    
	return return_value;
} /* CmdPowerOn */


/*****************************************************************************
 *
 *					SetParameters
 *
 ****************************************************************************/
RESPONSECODE SetParameters(unsigned int reader_index, char protocol,
                           unsigned int length, unsigned char buffer[])
{
	unsigned char cmd[10+CMD_BUF_SIZE_EACH];	/* CCID + APDU buffer */
	_ccid_descriptor *ccid_descriptor = get_ccid_descriptor(reader_index);
    
    
	cmd[0] = 0x61;                                  /* SetParameters */
	i2dw(length, cmd+1);                            /* APDU length */
	cmd[5] = ccid_descriptor->bCurrentSlotIndex;	/* slot number */
	cmd[6] = ccid_descriptor->real_bSeq++;
	cmd[7] = protocol;                              /* bProtocolNum */
	cmd[8] = cmd[9] = 0;                            /* RFU */
    
	/* check that the command is not too large */
	if (length > CMD_BUF_SIZE_EACH)
		return IFD_NOT_SUPPORTED;
    
	memcpy(cmd+10, buffer, length);
    
	if (WritePort(reader_index, 10+length, cmd) != STATUS_SUCCESS)
		return IFD_COMMUNICATION_ERROR;
    
	length = sizeof(cmd);
	if (ReadPort(reader_index, &length, cmd) != STATUS_SUCCESS)
		return IFD_COMMUNICATION_ERROR;
    
	if (length < STATUS_OFFSET+1)
	{
		
		return IFD_COMMUNICATION_ERROR;
	}
    
	if (cmd[STATUS_OFFSET] & CCID_COMMAND_FAILED)
	{
        ccid_error(cmd[ERROR_OFFSET], __FILE__, __LINE__, __FUNCTION__);
		if (0x00 == cmd[ERROR_OFFSET])	/* command not supported */
			return IFD_NOT_SUPPORTED;
		else
			if ((cmd[ERROR_OFFSET] >= 1) && (cmd[ERROR_OFFSET] <= 127))
            /* a parameter is not changeable */
				return IFD_SUCCESS;
			else if (cmd[ERROR_OFFSET] == 0xFE) 
                    
                    ccid_descriptor->dwSlotStatus = IFD_ICC_NOT_PRESENT;
            else
				return IFD_COMMUNICATION_ERROR;
	}
    
	return IFD_SUCCESS;
} /* SetParameters */




/*****************************************************************************
 *
 *					CCID_Transmit
 *
 ****************************************************************************/
RESPONSECODE CCID_Transmit(unsigned int reader_index, unsigned int tx_length,
	const unsigned char tx_buffer[], unsigned short rx_length, unsigned char bBWI)
{
	unsigned char cmd[10+CMD_BUF_SIZE_EACH];	/* CCID + APDU buffer */
	_ccid_descriptor *ccid_descriptor = get_ccid_descriptor(reader_index);
	status_t ret;
	
	cmd[0] = 0x6F;                                  /* XfrBlock */
	i2dw(tx_length, cmd+1);                         /* APDU length */
	cmd[5] = ccid_descriptor->bCurrentSlotIndex;	/* slot number */
	cmd[6] = ccid_descriptor->real_bSeq++;
	cmd[7] = bBWI;                                  /* extend block waiting timeout */
	cmd[8] = rx_length & 0xFF;                      /* Expected length, in character mode only */
	cmd[9] = (rx_length >> 8) & 0xFF;

	/* check that the command is not too large */
	if (tx_length > CMD_BUF_SIZE_EACH)
	{
		return IFD_NOT_SUPPORTED;
	}

    //type b
    if (1) {
        if (NULL == tx_buffer)
			 rx_length = 0x10;	/* bLevelParameter */
    }
    if (tx_length >0 ) {
        memcpy(cmd+10, tx_buffer, tx_length);
    }
	

	ret = WritePort(reader_index, 10+tx_length, cmd);
	if (STATUS_NO_SUCH_DEVICE == ret)
		return IFD_NO_SUCH_DEVICE;
	if (ret != STATUS_SUCCESS)
		return IFD_COMMUNICATION_ERROR;

	return IFD_SUCCESS;
} /* CCID_Transmit */



/*****************************************************************************
 *
 *					CCID_Receive
 *
 ****************************************************************************/
RESPONSECODE CCID_Receive(unsigned int reader_index, unsigned int *rx_length,
	unsigned char rx_buffer[], unsigned char *chain_parameter)
{
	unsigned char cmd[10+CMD_BUF_SIZE_EACH];	/* CCID + APDU buffer */
	unsigned int length;
	RESPONSECODE return_value = IFD_SUCCESS;
	status_t ret;

	//_ccid_descriptor *ccid_descriptor = get_ccid_descriptor(reader_index);
    //type B
	if(1)
    {
       // int r;
		unsigned char rx_tmp[4];
		//unsigned char *old_rx_buffer = NULL;
		//int old_rx_length = 0;
        
		/* read a nul block. buffer need to be at least 4-bytes */
		if (NULL == rx_buffer)
		{
			rx_buffer = rx_tmp;
			*rx_length = sizeof(rx_tmp);
		}
        
    }
time_request:
	length = sizeof(cmd);
	ret = ReadPort(reader_index, &length, cmd);
	if (ret != STATUS_SUCCESS)
	{
		if (STATUS_NO_SUCH_DEVICE == ret)
			return IFD_NO_SUCH_DEVICE;
		return IFD_COMMUNICATION_ERROR;
	}

	if (length < STATUS_OFFSET+1)
	{
		return IFD_COMMUNICATION_ERROR;
	}

	if (cmd[STATUS_OFFSET] & CCID_COMMAND_FAILED)
	{
		switch (cmd[ERROR_OFFSET])
		{
			case 0xEF:	/* cancel */
				if (*rx_length < 2)
					return IFD_COMMUNICATION_ERROR;
				rx_buffer[0]= 0x64;
				rx_buffer[1]= 0x01;
				*rx_length = 2;
				return IFD_SUCCESS;

			case 0xF0:	/* timeout */
				if (*rx_length < 2)
					return IFD_COMMUNICATION_ERROR;
				rx_buffer[0]= 0x64;
				rx_buffer[1]= 0x00;
				*rx_length = 2;
				return IFD_SUCCESS;

			case 0xFD:	/* Parity error during exchange */
                return IFD_PARITY_ERROR;
            case 0xFE:
                return IFD_COMMUNICATION_ERROR;
			default:
                return IFD_COMMUNICATION_ERROR;
		}
	}

	if (cmd[STATUS_OFFSET] & CCID_TIME_EXTENSION)
	{
		goto time_request;
	}

	/* we have read less (or more) data than the CCID frame says to contain */
	if (length-10 != dw2i(cmd, 1))
	{
		return_value = IFD_COMMUNICATION_ERROR;
	}

	length = dw2i(cmd, 1);
	if (length <= *rx_length)
		*rx_length = length;
	else
	{
		length = *rx_length;
		return_value = IFD_ERROR_INSUFFICIENT_BUFFER;
	}

	/* Kobil firmware bug. No support for chaining */
	if (length && (NULL == rx_buffer))
	{
		return_value = IFD_COMMUNICATION_ERROR;
	}
	else
		memcpy(rx_buffer, cmd+10, length);

	/* Extended case?
	 * Only valid for RDR_to_PC_DataBlock frames */
	if (chain_parameter)
		*chain_parameter = cmd[CHAIN_PARAMETER_OFFSET];

	return return_value;
} /* CCID_Receive */



/*****************************************************************************
 *
 *					CmdXfrBlockAPDU_extended
 *
 ****************************************************************************/
static RESPONSECODE CmdXfrBlockAPDU_extended(unsigned int reader_index,
                                             unsigned int tx_length, unsigned char tx_buffer[], unsigned int *rx_length,
                                             unsigned char rx_buffer[])
{
	RESPONSECODE return_value;
	_ccid_descriptor *ccid_descriptor = get_ccid_descriptor(reader_index);
	unsigned char chain_parameter;
	unsigned int local_tx_length, sent_length;
	unsigned int local_rx_length, received_length;
	int buffer_overflow = 0;
    
	/* send the APDU */
	sent_length = 0;
    
	/* we suppose one command is enough */
	chain_parameter = 0x00;
    
	local_tx_length = tx_length - sent_length;
	if (local_tx_length > CMD_BUF_SIZE)
	{
		local_tx_length = CMD_BUF_SIZE;
		/* the command APDU begins with this command, and continue in the next
		 * PC_to_RDR_XfrBlock */
		chain_parameter = 0x01;
	}
    //dukpt Encrypted communication, here is not to the subcontract
	if ((local_tx_length > ccid_descriptor->dwMaxCCIDMessageLength-10) && (isDukpt == 0))
	{
		local_tx_length = ccid_descriptor->dwMaxCCIDMessageLength-10;
		chain_parameter = 0x01;
	}
    
send_next_block:
	return_value = CCID_Transmit(reader_index, local_tx_length, tx_buffer,
                                 chain_parameter, 0);
	if (return_value != IFD_SUCCESS)
		return return_value;
    
	sent_length += local_tx_length;
	tx_buffer += local_tx_length;
    
	/* we just sent the last block (0x02) or only one block was needded (0x00) */
	if ((0x02 == chain_parameter) || (0x00 == chain_parameter))
		goto receive_block;
    
	/* read a nul block */
	return_value = CCID_Receive(reader_index, &local_rx_length, NULL, NULL);
	if (return_value != IFD_SUCCESS)
		return return_value;
    
	/* size of the next block */
	if (tx_length - sent_length > local_tx_length)
	{
		/* the abData field continues a command APDU and
		 * another block is to follow */
		chain_parameter = 0x03;
	}
	else
	{
		/* this abData field continues a command APDU and ends
		 * the APDU command */
		chain_parameter = 0x02;
        
		/* last (smaller) block */
		local_tx_length = tx_length - sent_length;
	}
    
	goto send_next_block;
    
receive_block:
	/* receive the APDU */
	received_length = 0;
    
receive_next_block:
	local_rx_length = *rx_length - received_length;
	return_value = CCID_Receive(reader_index, &local_rx_length, rx_buffer,
                                &chain_parameter);
	if (IFD_ERROR_INSUFFICIENT_BUFFER == return_value)
	{
		buffer_overflow = 1;
        
		/* we continue to read all the response APDU */
		return_value = IFD_SUCCESS;
	}
    
	if (return_value != IFD_SUCCESS)
		return return_value;
    
	/* advance in the reiceiving buffer */
	rx_buffer += local_rx_length;
	received_length += local_rx_length;
    
	switch (chain_parameter)
	{
            /* the response APDU begins and ends in this command */
		case 0x00:
            /* this abData field continues the response APDU and ends the response
             * APDU */
		case 0x02:
			break;
            
            /* the response APDU begins with this command and is to continue */
		case 0x01:
            /* this abData field continues the response APDU and another block is
             * to follow */
		case 0x03:
            /* empty abData field, continuation of the command APDU is expected in
             * next PC_to_RDR_XfrBlock command */
		case 0x10:
			/* send a nul block */
			/* set wLevelParameter to 0010h: empty abData field,
			 * continuation of response APDU is
			 * expected in the next RDR_to_PC_DataBlock. */
			return_value = CCID_Transmit(reader_index, 0, NULL, 0x10, 0);
			if (return_value != IFD_SUCCESS)
				return return_value;
            
			goto receive_next_block;
	}
    
	*rx_length = received_length;
    
	/* generate an overflow detected by pcscd */
	if (buffer_overflow)
		(*rx_length)++;
    
	return IFD_SUCCESS;
} /* CmdXfrBlockAPDU_extended */

/*****************************************************************************
 *
 *					CmdXfrBlockTPDU_T0
 *
 ****************************************************************************/
static RESPONSECODE CmdXfrBlockTPDU_T0(unsigned int reader_index,
                                       unsigned int tx_length, unsigned char tx_buffer[], unsigned int *rx_length,
                                       unsigned char rx_buffer[])
{
	RESPONSECODE return_value = IFD_SUCCESS;
	//_ccid_descriptor *ccid_descriptor = get_ccid_descriptor(reader_index);
    
	
    
	/* command length too big for CCID reader? */
#if 0
	if (tx_length > ccid_descriptor->dwMaxCCIDMessageLength-10)
	{
			return IFD_COMMUNICATION_ERROR;
	}
#else
    if (tx_length > MAX_BUFFER_SIZE)
	{
        return IFD_COMMUNICATION_ERROR;
	}
    
#endif
	/* command length too big for CCID driver? */
	if (tx_length > CMD_BUF_SIZE)
	{
		//DEBUG_CRITICAL3("Command too long (%d bytes) for max: %d bytes",
         //               tx_length, CMD_BUF_SIZE);
		return IFD_COMMUNICATION_ERROR;
	}
    
	return_value = CCID_Transmit(reader_index, tx_length, tx_buffer, 0, 0);
	if (return_value != IFD_SUCCESS)
		return return_value;
    
	return CCID_Receive(reader_index, rx_length, rx_buffer, NULL);
} /* CmdXfrBlockTPDU_T0 */





/*****************************************************************************
 *
 *					CmdXfrBlockTPDU_T1
 *
 ****************************************************************************/
static RESPONSECODE CmdXfrBlockTPDU_T1(unsigned int reader_index,
	unsigned int tx_length, unsigned char tx_buffer[], unsigned int *rx_length,
	unsigned char rx_buffer[])
{
	RESPONSECODE return_value = IFD_SUCCESS;
	int ret;


ret = t1_transceive(&((get_ccid_slot(reader_index)) -> t1), 0,
		tx_buffer, tx_length, rx_buffer, *rx_length);

	if (ret < 0)
		return_value = IFD_COMMUNICATION_ERROR;
	else
		*rx_length = ret;

	return return_value;
} /* CmdXfrBlockTPDU_T1 */


/*****************************************************************************
 *
 *					CmdXfrBlock
 *
 ****************************************************************************/
RESPONSECODE CmdXfrBlock(unsigned int reader_index, unsigned int tx_length,
	unsigned char tx_buffer[], unsigned int *rx_length,
	unsigned char rx_buffer[], int protocol)
{
	RESPONSECODE return_value = IFD_SUCCESS;
	//_ccid_descriptor *ccid_descriptor = get_ccid_descriptor(reader_index);

    //here a lot of protocol
	if (protocol == T_0)
	//	return_value = CmdXfrBlockCHAR_T0(reader_index, tx_length,
	//		tx_buffer, rx_length, rx_buffer);
    //  return_value = CmdXfrBlockTPDU_T0(reader_index,
    //      tx_length, tx_buffer, rx_length, rx_buffer);
        return_value = CmdXfrBlockAPDU_extended(reader_index,tx_length, tx_buffer, rx_length, rx_buffer);
	else
		if (protocol == T_1)
			return_value = CmdXfrBlockTPDU_T1(reader_index, tx_length,
				tx_buffer, rx_length, rx_buffer);
        else
        {
        
            return_value=IFD_ERROR_NOT_SUPPORTED;
        }
 		
	return return_value;
} /* CmdXfrBlock */














