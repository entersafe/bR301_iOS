/*
 * Support for bR301(Bluetooth) smart card reader
 *
 * Copyright (C) Feitian 2014, Ben <ben@ftsafe.com>
 *
 * This file has moved from PC/SC Lite APIs by GNU license (http://pcsclite.alioth.debian.org/pcsclite.html)
 * 
 * Based on PC/SC Lite API, Feitian have integrted own private command:
 * FtGetDevVer,FtGetSerialNum,FtWriteFlash,FtReadFlash,FtSetTimeout,FtDukptInit,FtDukptSetEncMod,FtDukptGetKSN,FtDidEnterBackground,FtSle4442Cmd,FtGetDevVer,FtGetLibVersion
 *
 * Some code we reference PCSCLite GNU source code, the copyright below:
 *
 * MUSCLE SmartCard Development ( http://pcsclite.alioth.debian.org/pcsclite.html )
 *
 * Copyright (C) 1999-2004
 *  David Corcoran <corcoran@musclecard.com>
 * Copyright (C) 2002-2011
 *  Ludovic Rousseau <ludovic.rousseau@free.fr>
 *
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:
 
 1. Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 3. The name of the author may not be used to endorse or promote products
 derived from this software without specific prior written permission.
 
 Changes to this license can be made only by the copyright author with
 explicit written consent.
 
 THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: winscard.c 6851 2014-02-14 15:43:32Z rousseau $

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

/*
 * Implementation of FtGetDevVer,FtGetSerialNum,FtWriteFlash
 *                  FtReadFlash,FtSetTimeout,FtDukptInit
 *                  FtDukptSetEncMod,FtDukptGetKSN,FtDidEnterBackground
 *                  FtSle4442Cmd,FtGetDevVer,FtGetLibVersion
 *
 * Copyright (C) 2013~2014, Feitian <www.ftsafe.com.cn>
 *
 */
#include "winscard.h"

#ifndef FALSE
#define FALSE 0
#define TRUE 1
#endif

#include <stdio.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "ft_ccid.h"
#import "EADSessionController.h"

extern unsigned int g_dwTimeOut;
extern unsigned int iR301_or_bR301;
extern BOOL bIsDidEnterBackground;
extern pthread_mutex_t CommunicatonMutex;

unsigned int isDukpt = 0;

char Ft_iR301U_Version[3]={0x01,0x30,0x00};
volatile int eStablishContextCount = 0;
volatile int eShCardHandleCount = 0;


SCARD_IO_REQUEST g_rgSCardT0Pci={T_0,0}, g_rgSCardT1Pci={T_1,0},
g_rgSCardRawPci={T_RAW,0};



#pragma mark PCSC APIs

LONG SCardEstablishContext(DWORD dwScope, /*@unused@*/ LPCVOID pvReserved1,
                           /*@unused@*/ LPCVOID pvReserved2, LPSCARDCONTEXT phContext)
{

    if ( SCARD_SCOPE_SYSTEM != dwScope )
    {
        return SCARD_E_INVALID_VALUE;
    }
    
    if (NULL == phContext) 
    {
        return SCARD_E_INVALID_PARAMETER ;
    }
   
   

    _ccid_descriptor *ccid_descriptor = get_ccid_descriptor(0);
    ccid_descriptor->dwMaxCCIDMessageLength = 272;
    ccid_descriptor->dwSlotStatus = IFD_ICC_NOT_PRESENT;
    

    pthread_mutex_lock(&CommunicatonMutex);
    eStablishContextCount = eStablishContextCount + 1;

    [[EADSessionController sharedController] RegisterAccessoryConnectNotification];
    pthread_mutex_unlock(&CommunicatonMutex);
    *phContext = (SCARDCONTEXT)ccid_descriptor;
    return SCARD_S_SUCCESS;
}


LONG SCardReleaseContext(SCARDCONTEXT hContext)
{

    if (0 == hContext)
    {
        return SCARD_E_INVALID_HANDLE;
    }
    
    hContext =  0;
   
    pthread_mutex_lock(&CommunicatonMutex);
    if (eStablishContextCount <= 0)
    {
        pthread_mutex_unlock(&CommunicatonMutex);
        return SCARD_S_SUCCESS;
    }
    eStablishContextCount = eStablishContextCount - 1;
    if (eStablishContextCount <= 0)
    {
        [[EADSessionController sharedController] UnRegisterAccessoryConnectNotification];
      
    }
    pthread_mutex_unlock(&CommunicatonMutex);

	return SCARD_S_SUCCESS;
}


LONG SCardIsValidContext(SCARDCONTEXT hContext)
{
    if (0 == hContext) 
    {
        return SCARD_E_INVALID_HANDLE;
    }
    
    _ccid_descriptor *ccid_descriptor = get_ccid_descriptor(0);
    if(isbadreadptr(ccid_descriptor,sizeof(_ccid_descriptor)))
    {
        return SCARD_E_INVALID_HANDLE;
    }
   
    return SCARD_S_SUCCESS;
}

LONG SCardListReaders(SCARDCONTEXT hContext,
                      /*@null@*/ /*@out@*/ LPCSTR mszGroups,
                      /*@null@*/ /*@out@*/ LPSTR mszReaders,
                      /*@out@*/ LPDWORD pcchReaders)
{
    int namelen = strlen("FT smartcard reader")+2;
  
    if (0 == hContext) 
    {
        return SCARD_E_INVALID_HANDLE;
    }

    if (pcchReaders == NULL){
       
		return SCARD_E_INVALID_PARAMETER;
    }
    pthread_mutex_lock(&CommunicatonMutex);
    EADSessionController *sessionController = [EADSessionController sharedController];
    pthread_mutex_unlock(&CommunicatonMutex);
    if(0 ==  [sessionController identifyAccessoryCount])
    {
        return SCARD_E_READER_UNAVAILABLE;
    }

    if (mszReaders != NULL)
    {
        if (*pcchReaders < namelen)
        {
            return SCARD_E_INSUFFICIENT_BUFFER ;
        }
        memcpy(mszReaders, "FT smartcard reader",namelen-1);
        mszReaders[namelen-1]=0;
    }
    *pcchReaders = namelen;

    return SCARD_S_SUCCESS;
}



LONG SCardConnect( SCARDCONTEXT hContext, LPCSTR szReader,
	DWORD dwShareMode, DWORD dwPreferredProtocols, LPSCARDHANDLE phCard,
	LPDWORD pdwActiveProtocol)
{
    unsigned char buf[256]={0};
    unsigned int len1 =sizeof(buf);
    long ReturnValue = 0;
    
    //////////////////////////////////////////////////////
    if ( NO == gIsOpen)
    {
        return SCARD_E_READER_UNAVAILABLE;
    }
    /////////////////////////////////////////////////////
    

    if (0 ==  hContext)
    {
        return SCARD_E_INVALID_HANDLE;
    }
    if (NULL ==  phCard) 
    {
        return SCARD_E_INVALID_PARAMETER;
    }

    if (pdwActiveProtocol == NULL)
    {
		return SCARD_E_INVALID_PARAMETER;
    }
    
    pthread_mutex_lock(&CommunicatonMutex);
    CcidDesc * CcidSlots = get_ccid_slot(0);
    if ( NO == gIsOpen)
    {
        pthread_mutex_unlock(&CommunicatonMutex);
        return SCARD_E_READER_UNAVAILABLE;
    }
    
    ReturnValue = CmdPowerOff(0);
    if (ReturnValue != IFD_SUCCESS)
    {
        [NSThread sleepForTimeInterval:0.002];
    }
    
    ReturnValue =  CmdPowerOn(0,&len1,buf,VOLTAGE_AUTO);

    if(IFD_SUCCESS != ReturnValue)
    {
        //add if poweron faile clear atr
        CcidSlots->nATRLength = 0;
        *CcidSlots->pcATRBuffer = '\0';
        pthread_mutex_unlock(&CommunicatonMutex);
        return SCARD_E_NOT_TRANSACTED;
        
    }else{
        
        if(len1 >= 1){
            CcidSlots->nATRLength = len1;
            memcpy(CcidSlots->pcATRBuffer,buf,len1);
            CcidSlots->bPowerFlags |= MASK_POWERFLAGS_PUP;
        }else{
            
            pthread_mutex_unlock(&CommunicatonMutex);
            return SCARD_E_READER_UNSUPPORTED;
        }
    }


    /* initialise T=1 context */
    (void)t1_init(&(CcidSlots->t1), 0);
    Scrd_Negotiate(0);
    _ccid_descriptor *ccid_descriptor = get_ccid_descriptor(0);
    int CardProtocol = ccid_descriptor->cardProtocol;


    if (CardProtocol == T_0)
    {
        *pdwActiveProtocol =  SCARD_PROTOCOL_T0;
    }
    else if(CardProtocol == T_1)
    {
        *pdwActiveProtocol =  SCARD_PROTOCOL_T1;
    }
    else
    {
        *pdwActiveProtocol = SCARD_PROTOCOL_UNDEFINED;
         pthread_mutex_unlock(&CommunicatonMutex);
        return SCARD_E_UNSUPPORTED_FEATURE;
    }
    
    *phCard = (SCARDHANDLE)CcidSlots;
    eShCardHandleCount = eShCardHandleCount + 1;
    pthread_mutex_unlock(&CommunicatonMutex);

	return SCARD_S_SUCCESS;
}


LONG SCardReconnect(SCARDHANDLE hCard, DWORD dwShareMode,
                    DWORD dwPreferredProtocols, DWORD dwInitialization,
                    LPDWORD pdwActiveProtocol)
{
    
    SCardDisconnect(hCard, dwInitialization);
    
    return SCardConnect(hCard,"iRockey 301",dwShareMode,dwPreferredProtocols,&hCard,pdwActiveProtocol);
    
}

LONG SCardDisconnect(SCARDHANDLE hCard, DWORD dwDisposition)
{
    long ReturnValue = 0;
    
    //////////////////////////////////////////////////////
    if ( NO == gIsOpen)
    {
        return SCARD_E_READER_UNAVAILABLE;
    }
    /////////////////////////////////////////////////////

    if (0 ==  hCard) 
    {
        return SCARD_E_INVALID_HANDLE;
    }
    
    
    if ((dwDisposition != SCARD_LEAVE_CARD)
        && (dwDisposition != SCARD_UNPOWER_CARD)
        && (dwDisposition != SCARD_RESET_CARD)
        && (dwDisposition != SCARD_EJECT_CARD))
    {
        return SCARD_E_INVALID_VALUE;
    }
    
    pthread_mutex_lock(&CommunicatonMutex);
    if (gIsOpen == NO)
    {
        pthread_mutex_unlock(&CommunicatonMutex);
        return SCARD_E_READER_UNAVAILABLE;
    }
    
    if (eShCardHandleCount <= 0)
    {
        pthread_mutex_unlock(&CommunicatonMutex);
        return SCARD_E_INVALID_TARGET;
    }
    
    if (dwDisposition == SCARD_LEAVE_CARD) {
        eShCardHandleCount =  eShCardHandleCount -1;
        pthread_mutex_unlock(&CommunicatonMutex);
        return SCARD_S_SUCCESS;
    }
    

    eShCardHandleCount = eShCardHandleCount - 1;

    
    if (eShCardHandleCount > 0)
    {
        pthread_mutex_unlock(&CommunicatonMutex);
        return SCARD_S_SUCCESS;
    }
    
    CcidDesc *CcidSlots = get_ccid_slot(0);
    CcidSlots->nATRLength = 0;
    *CcidSlots->pcATRBuffer = '\0';
    CcidSlots->bPowerFlags |= MASK_POWERFLAGS_PDWN;
    t1_release(&(CcidSlots->t1));
    
    ReturnValue = CmdPowerOff(0);
    isDukpt = 0;
    pthread_mutex_unlock(&CommunicatonMutex);
    
    if (ReturnValue != IFD_SUCCESS )
    {
        return SCARD_F_COMM_ERROR;
    }
    
    return SCARD_S_SUCCESS;
}


LONG SCardStatus(SCARDHANDLE hCard, LPSTR mszReaderNames,
                 LPDWORD pcchReaderLen, LPDWORD pdwState,
                 LPDWORD pdwProtocol, LPBYTE pbAtr, LPDWORD pcbAtrLen)
{
    
    unsigned char pcbuffer[SIZE_GET_SLOT_STATUS];
    unsigned char ATRBuffer[33]={0};
    int ATRLength = 0;
    int CardProtocol = 0;
    int namelen = strlen("iRockey 301")+2;
    unsigned int oldReadTimeout;
    RESPONSECODE return_value = IFD_COMMUNICATION_ERROR;
    
    if (pcchReaderLen != NULL )
    {
        if (mszReaderNames != NULL && *pcchReaderLen >= namelen)
        {
            strcpy(mszReaderNames, "iRockey 301");
            mszReaderNames[namelen -1]='\0';
            *pcchReaderLen = namelen;
        }
        else
        {
            *pcchReaderLen = 0;
            
            return SCARD_E_INSUFFICIENT_BUFFER;
        }
    }
    
    pthread_mutex_lock(&CommunicatonMutex);
    _ccid_descriptor *ccid_descriptor = get_ccid_descriptor(0);
    CcidDesc *CcidSlots = get_ccid_slot(0);
    
    if (gIsOpen == NO)
    {
        pthread_mutex_unlock(&CommunicatonMutex);
        return SCARD_E_READER_UNAVAILABLE;
    }
    
    /* save the current read timeout computed from card capabilities */
    /* use default timeout since the reader may not be present anymore */
    
    oldReadTimeout = g_dwTimeOut;
    if (iR301_or_bR301 == 0)
    {
        g_dwTimeOut= 600;
    }
    if (iR301_or_bR301 == 1)
    {
        g_dwTimeOut = 1500;
    }
    
    return_value = CmdGetSlotStatus(0, pcbuffer);
    
    /* set back the old timeout */
    g_dwTimeOut = oldReadTimeout;
    
    if (return_value != IFD_SUCCESS)
    {
        return_value = SCARD_E_NOT_TRANSACTED;
        pthread_mutex_unlock(&CommunicatonMutex);
        return (int)return_value;
    }
    
    return_value = IFD_COMMUNICATION_ERROR;
    switch (pcbuffer[STATUS_OFFSET] & CCID_ICC_STATUS_MASK)	/* bStatus */
    {
        case CCID_ICC_PRESENT_ACTIVE:
            return_value = SCARD_PRESENT;
            ccid_descriptor->dwSlotStatus = IFD_ICC_PRESENT;
            /* use default slot */
            break;
            
        case CCID_ICC_PRESENT_INACTIVE:
            return_value = SCARD_SWALLOWED;
            
            /* Reset ATR buffer */
            CcidSlots->nATRLength = 0;
            *CcidSlots->pcATRBuffer = '\0';
            /* Reset PowerFlags */
            CcidSlots->bPowerFlags = POWERFLAGS_RAZ;
            ccid_descriptor->dwSlotStatus = IFD_ICC_PRESENT;
            break;
            
        case CCID_ICC_ABSENT:
            /* Reset ATR buffer */
            CcidSlots->nATRLength = 0;
            *CcidSlots->pcATRBuffer = '\0';
            ccid_descriptor->dwSlotStatus = IFD_ICC_NOT_PRESENT;
            /* Reset PowerFlags */
            CcidSlots->bPowerFlags = POWERFLAGS_RAZ;
            return_value = SCARD_ABSENT;
            break;
    }
    
    if (pcbuffer[ERROR_OFFSET] == 0xFE)
    {
        return_value = SCARD_ABSENT;
    }
    
    if (return_value == SCARD_PRESENT)
    {
        
        ATRLength = CcidSlots->nATRLength;
        memcpy(ATRBuffer, CcidSlots->pcATRBuffer, ATRLength);
        CardProtocol = ccid_descriptor->cardProtocol;
        
    }
    
    if (pdwState != NULL)
    {
        *pdwState = (unsigned int)return_value;
    }
    
    
    if (pcbAtrLen != NULL)
    {
        
        if (*pcbAtrLen < ATRLength)
        {
            *pcbAtrLen = 0;
            pthread_mutex_unlock(&CommunicatonMutex);
            return SCARD_E_INSUFFICIENT_BUFFER;
        }
        if ( pbAtr !=  NULL)
        {
            
            memcpy(pbAtr,ATRBuffer ,ATRLength);
        }
        
        *pcbAtrLen  = ATRLength;
        
    }
    
    if (pdwProtocol != NULL)
    {
        
        if (CardProtocol == T_0) {
            *pdwProtocol =  SCARD_PROTOCOL_T0;
        }
        else if(CardProtocol == T_1){
            *pdwProtocol =  SCARD_PROTOCOL_T1;
        }
        else {
            *pdwProtocol = SCARD_PROTOCOL_UNDEFINED;
        }
        
    }
    
    pthread_mutex_unlock(&CommunicatonMutex);
    return SCARD_S_SUCCESS;
    
}


LONG SCardGetAttrib(SCARDHANDLE hCard, DWORD dwAttrId,
                    LPBYTE pbAttr, LPDWORD pcbAttrLen)
{
    int ATRLength = 0;
    unsigned char ATRBuffer[33]={0};
    
    if (0 ==  hCard)
    {
        return SCARD_E_INVALID_HANDLE;
    }
    
    if (pcbAttrLen == NULL || pbAttr == NULL)
    {
        return SCARD_E_INVALID_PARAMETER;
    }
    
    pthread_mutex_lock(&CommunicatonMutex);
    if (gIsOpen == NO)
    {
        pthread_mutex_unlock(&CommunicatonMutex);
        return SCARD_E_READER_UNAVAILABLE;
    }
    
    CcidDesc *CcidSlots = get_ccid_slot(0);
    if (CcidSlots->nATRLength> 0) {
        ATRLength = CcidSlots->nATRLength;
        memcpy(ATRBuffer, CcidSlots->pcATRBuffer, ATRLength);
    }
    pthread_mutex_unlock(&CommunicatonMutex);
    
    if(ATRLength > 0)
    {
        if (*pcbAttrLen < ATRLength )
        {
            *pcbAttrLen = 0;
            return SCARD_E_INVALID_PARAMETER;
        }
        memcpy(pbAttr,ATRBuffer,ATRLength);
        *pcbAttrLen = ATRLength;
    }
    else
    {
        return SCARD_E_INSUFFICIENT_BUFFER;
    }
    
    return SCARD_S_SUCCESS;;
}

LONG SCardBeginTransaction(SCARDHANDLE hCard)
{
    
    if (0 ==  hCard)
    {
        return SCARD_E_INVALID_HANDLE;
    }
    
    return SCARD_S_SUCCESS;
}

LONG SCardEndTransaction(SCARDHANDLE hCard, DWORD dwDisposition)
{
    
    if (0 ==  hCard) 
    {
        return SCARD_E_INVALID_HANDLE;
    }
    if ((dwDisposition != SCARD_LEAVE_CARD)
        && (dwDisposition != SCARD_UNPOWER_CARD)
        && (dwDisposition != SCARD_RESET_CARD)
        && (dwDisposition != SCARD_EJECT_CARD))
        return SCARD_E_INVALID_VALUE;
    
    return SCARD_S_SUCCESS;
}



////////////////////////////NULL func/////////////////////////////////////////
LONG  SCARD_CTL_CODE(unsigned int code)
{
    LONG nCode= (code + 0x31);
    return nCode;
}

LONG SCardControl(SCARDHANDLE hCard, DWORD dwControlCode,
                  LPCVOID pbSendBuffer, DWORD cbSendLength,
                  LPVOID pbRecvBuffer, DWORD cbRecvLength, LPDWORD lpBytesReturned)
{
    
    long rv;
    unsigned char buf[300];
    unsigned int  len =sizeof(buf);
    
    if (0 ==  hCard)
    {
        return SCARD_E_INVALID_HANDLE;
    }
    
    if (pbSendBuffer == NULL || pbRecvBuffer == NULL || lpBytesReturned == NULL)
        return SCARD_E_INVALID_PARAMETER;
    
    if (cbSendLength > 272 -10 )
    {
        return SCARD_E_INVALID_PARAMETER;
    }
    
    pthread_mutex_lock(&CommunicatonMutex);
    if (gIsOpen == NO)
    {
        pthread_mutex_unlock(&CommunicatonMutex);
        return SCARD_E_READER_UNAVAILABLE;
    }
    
    if (dwControlCode == SCARD_CTL_CODE(3500))
    {
        
        
        rv = CmdEscape(0, (unsigned char*)pbSendBuffer, cbSendLength, buf, &len);
        pthread_mutex_unlock(&CommunicatonMutex);
        if( IFD_SUCCESS != rv)
        {
            
            return SCARD_E_NOT_TRANSACTED;
        }
        
        if (cbRecvLength < len)
        {
            return SCARD_E_INSUFFICIENT_BUFFER;
        }
        *lpBytesReturned = len;
        memcpy(pbRecvBuffer,buf,len);
        
        return SCARD_S_SUCCESS;
    }
    pthread_mutex_unlock(&CommunicatonMutex);
    return SCARD_E_READER_UNSUPPORTED;
}


LONG SCardTransmit(SCARDHANDLE hCard, const SCARD_IO_REQUEST *pioSendPci,
                   LPCBYTE pbSendBuffer, DWORD cbSendLength,
                   SCARD_IO_REQUEST *pioRecvPci, LPBYTE pbRecvBuffer,
                   LPDWORD pcbRecvLength)
{
    
    long rv;
    unsigned char buf[CMD_BUF_SIZE];
    unsigned int len =sizeof(buf);
    
    //////////////////////////////////////////////////////
    if ( NO == gIsOpen)
    {
        return SCARD_E_READER_UNAVAILABLE;
    }
    /////////////////////////////////////////////////////


    if (0 ==  hCard)
    {
        return SCARD_E_INVALID_HANDLE;
    }
    
    if (NULL == pioSendPci)
    {
        return SCARD_E_INVALID_VALUE;
    }
    
    if (pbSendBuffer == NULL || pbRecvBuffer == NULL || pcbRecvLength == NULL)
    {
        return SCARD_E_INVALID_PARAMETER;
    }
    
    /*
     * Must at least have 2 status words even for SCardControl
     */
    if (*pcbRecvLength < 2)
    {
        return SCARD_E_INSUFFICIENT_BUFFER;
    }
    
    pthread_mutex_lock(&CommunicatonMutex);
    if (gIsOpen == NO)
    {
        pthread_mutex_unlock(&CommunicatonMutex);
        return SCARD_E_READER_UNAVAILABLE;
    }
    _ccid_descriptor *ccid_descriptor = get_ccid_descriptor(0);
    len = CMD_BUF_SIZE;
    if( FT_READER_UA != gDevType)
    {
        rv = CmdXfrBlock(0, (unsigned int)cbSendLength,(unsigned char*)pbSendBuffer, &len, buf,T_0);
    }
    else
    {
        rv = CmdXfrBlock(0, (unsigned int)cbSendLength,(unsigned char*)pbSendBuffer, &len, buf,ccid_descriptor->cardProtocol);
    }
    
#if 1
    /*Difference between the old UA and UB method is test error*/
    if (T_1 == ccid_descriptor->cardProtocol
        && IFD_SUCCESS != rv
        && FT_READER_DEFAULT == gDevType && iR301_or_bR301 == 0) {
        
        len = CMD_BUF_SIZE;
        rv = CmdXfrBlock(0, (unsigned int)cbSendLength,(unsigned char*)pbSendBuffer, &len, buf,ccid_descriptor->cardProtocol);
        if( IFD_SUCCESS == rv) {
            gDevType = FT_READER_UA;
        }
    }
#endif
    
    pthread_mutex_unlock(&CommunicatonMutex);
    if( IFD_SUCCESS != rv)
    {
        
        return SCARD_E_NOT_TRANSACTED;
    }
    
    if (*pcbRecvLength < len)
    {
        *pcbRecvLength = 0;
        return SCARD_E_INSUFFICIENT_BUFFER;
    }
    *pcbRecvLength = len;
    memcpy(pbRecvBuffer,buf,len);
    
    return SCARD_S_SUCCESS;
    
}

LONG SCardGetStatusChange(SCARDCONTEXT hContext,
                          DWORD dwTimeout,
                          LPSCARD_READERSTATE rgReaderStates, DWORD cReaders)
{
    LONG rv =0;
    DWORD dwState;
    
    if (rgReaderStates == NULL && cReaders > 0)
    {
        return  SCARD_E_INVALID_PARAMETER;
    }
    
    rv =  SCardStatus(NULL,NULL,NULL,&dwState,NULL,NULL,NULL);
    
    if(SCARD_S_SUCCESS != rv)
    {
        rgReaderStates->dwEventState = SCARD_STATE_EMPTY;
        return rv;
    }
    else
    {
        if (SCARD_ABSENT == dwState)
        {
            eShCardHandleCount = 0;
            isDukpt = 0;
            rgReaderStates->dwEventState = SCARD_STATE_EMPTY;
        }
        else
        {
            rgReaderStates->dwEventState = SCARD_STATE_PRESENT;
        }
    }
    
    return SCARD_S_SUCCESS;
}

LONG SCardListReaderGroups(SCARDCONTEXT hContext,
                           LPSTR mszGroups, LPDWORD pcchGroups)
{
    return SCARD_S_SUCCESS;
    
}

LONG SCardCancel(SCARDCONTEXT hContext)
{
    
    return SCARD_S_SUCCESS;
}



#pragma mark duktp
LONG FtGetDevVer( SCARDCONTEXT hContext,char *firmwareRevision,char *hardwareRevision)
{
    long rv=0;
    
    pthread_mutex_lock(&CommunicatonMutex);
    EADSessionController *sessionController = [EADSessionController sharedController];
    if (sessionController.identifyAccessoryCount == 0 ){
        pthread_mutex_unlock(&CommunicatonMutex);
        return SCARD_E_READER_UNAVAILABLE;
    }
    rv = CmdGetDevVer(0,firmwareRevision,hardwareRevision);
    pthread_mutex_unlock(&CommunicatonMutex);
    if (IFD_SUCCESS == rv)
    {
        return SCARD_S_SUCCESS;
    }
    else
    {
        return SCARD_E_NO_READERS_AVAILABLE;
    }
}

LONG FtDukptInit(SCARDHANDLE hCard,unsigned char *encBuf,unsigned int nLen)
{
    
    long rv=0;
    if (nLen  != 40 && nLen  != 48)
    {
        return SCARD_E_INVALID_PARAMETER;
    }
    pthread_mutex_lock(&CommunicatonMutex);

    if (gIsOpen == NO) {
        pthread_mutex_unlock(&CommunicatonMutex);
        return SCARD_E_READER_UNAVAILABLE;
    }
    rv= CmdWriteIKSN2IPEKFlash( 0,encBuf,nLen);
    pthread_mutex_unlock(&CommunicatonMutex);
    
    if (IFD_SUCCESS == rv) 
    {
        return SCARD_S_SUCCESS;
    }
    else 
    {
        return SCARD_F_COMM_ERROR;
    }
    return 0;

}


LONG FtDukptSetEncMod(SCARDHANDLE hCard,
                      unsigned int bEncrypt,
                      unsigned int bEncFunc,
                      unsigned int bEncType)
{
    long rv=0;
    
    pthread_mutex_lock(&CommunicatonMutex);

    if (gIsOpen == NO) {
        pthread_mutex_unlock(&CommunicatonMutex);
        return SCARD_E_READER_UNAVAILABLE;
    }

    rv= CmdSetEncMod(0,bEncrypt,bEncFunc,bEncType);
    pthread_mutex_unlock(&CommunicatonMutex);
    
    if (IFD_SUCCESS == rv) 
    {
        if (bEncrypt) {
             isDukpt = 1;
        }
        return SCARD_S_SUCCESS;
    }
    else 
    {
        return SCARD_F_COMM_ERROR;
    }
    return 0;
}

LONG FtDukptGetKSN(SCARDHANDLE hCard, unsigned int * pnlength,
              unsigned char *buffer)
{
    long rv=0;
    unsigned char temp[64];
    unsigned int nlength=sizeof(temp);
    
    if (* pnlength  < 10) 
    {
        return SCARD_E_INVALID_PARAMETER;
    }
    
    pthread_mutex_lock(&CommunicatonMutex);

    if (gIsOpen == NO) {
        pthread_mutex_unlock(&CommunicatonMutex);
        return SCARD_E_READER_UNAVAILABLE;
    }

    rv= CmdGetKSN( 0, &nlength,temp);
    pthread_mutex_unlock(&CommunicatonMutex);
    
    if (IFD_SUCCESS == rv) 
    {
        memcpy(buffer, temp,10);
        return SCARD_S_SUCCESS;
    }
    else 
    {
        return SCARD_F_COMM_ERROR;
    }   
}

#pragma mark feitian's private command of reader
void FtGetLibVersion (char *buffer)
{
    if (buffer == NULL) {
        return;
    }
    sprintf(buffer, " ST:%01x.%01x.%01x",Ft_iR301U_Version[0],Ft_iR301U_Version[1],Ft_iR301U_Version[2]);
}

LONG FtGetSerialNum(SCARDHANDLE hCard, unsigned int  length,
                              char * buffer)
{
    long rv=0;
    unsigned char temp[64]={0};
    unsigned int nlength=sizeof(temp);

    if (length < 16) 
    {
        return SCARD_E_INVALID_PARAMETER;
    }
    
    pthread_mutex_lock(&CommunicatonMutex);
    if (gIsOpen == 0) {
        pthread_mutex_unlock(&CommunicatonMutex);
        return SCARD_E_READER_UNAVAILABLE;
    }
   
    rv= CmdGetSerialNum( 0, &nlength,temp);
    
    pthread_mutex_unlock(&CommunicatonMutex);
  
    if (IFD_SUCCESS == rv) 
    {
        memcpy(buffer, temp,16);
        buffer[16]='\0';
        return SCARD_S_SUCCESS;
    }
    else 
    {
        return SCARD_F_COMM_ERROR;
    }
}

LONG FtWriteFlash(SCARDHANDLE hCard,unsigned char bOffset, unsigned char blength,
                  unsigned char buffer[])
{
    long rv=0;

    if (bOffset+blength > 255 || blength == 0) 
    {
        return SCARD_E_INVALID_PARAMETER;
    }
    
    pthread_mutex_lock(&CommunicatonMutex);

    if (gIsOpen == NO) {
        pthread_mutex_unlock(&CommunicatonMutex);
        return SCARD_E_READER_UNAVAILABLE;
    }

    rv= CmdWriteFlash(0,bOffset,blength,buffer);
    pthread_mutex_unlock(&CommunicatonMutex);

    if (IFD_SUCCESS == rv) 
    {
        return SCARD_S_SUCCESS;
    }
    else 
    {
        return SCARD_F_COMM_ERROR;
    }
}

LONG FtReadFlash(SCARDHANDLE hCard,unsigned char bOffset, unsigned char blength,
                  unsigned char buffer[])
{
    long rv=0;
    unsigned char temp[300];

    if (bOffset+blength > 255 || blength == 0)
    {
        return SCARD_E_INVALID_PARAMETER;
    }
    
    pthread_mutex_lock(&CommunicatonMutex);

    if (gIsOpen == NO) {
        pthread_mutex_unlock(&CommunicatonMutex);
        return SCARD_E_READER_UNAVAILABLE;
    }
    rv= CmdReadFlash( 0,bOffset,blength,temp);
    pthread_mutex_unlock(&CommunicatonMutex);

    if (IFD_SUCCESS == rv) 
    {
        memcpy(buffer, temp, blength);
        return SCARD_S_SUCCESS;
    }
    else 
    {
        return SCARD_F_COMM_ERROR;
    }
}

#pragma mark feitian's private sle4442 command


LONG FtSetTimeout(SCARDCONTEXT hContext, DWORD dwTimeout)
{
    if (0 == hContext)
    {
        return SCARD_E_INVALID_HANDLE;
    }
    
    if (dwTimeout >= 1 ) 
    {
       g_dwTimeOut = dwTimeout  ;
    }
    else
    {
        return SCARD_E_INVALID_VALUE;
    }
    
	return SCARD_S_SUCCESS;
}


void FtDidEnterBackground(unsigned int bDidEnter)
{
    
}


