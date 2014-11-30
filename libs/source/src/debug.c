/*
 debug.c: log (or not) messages
 Copyright (C) 2003-2011   Ludovic Rousseau
 
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.
 
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.
 
	You should have received a copy of the GNU Lesser General Public License
	along with this library; if not, write to the Free Software Foundation,
	Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/*
 * $Id: debug.c 6760 2013-10-01 12:57:50Z rousseau $
 */


#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>  
#include <signal.h>  
#include <setjmp.h>  
#include <unistd.h>  

#include "ft_ccid.h"

#define DEBUG_BUF_SIZE ((256+20)*3+10)

#if defined(__MACH__) && defined(__FreeBSD__) && defined(__NetBSD__) && defined(__OpenBSD__) && defined(__DragonFly__)  
#define ERROR_SIGNAL SIGBUS  
#else  
#define ERROR_SIGNAL SIGSEGV  
#endif  

#ifdef FT_IR301_DEBUG
#define LOG_TO_STDERR  0
#else
#define LOG_TO_STDERR  1
#endif

static char DebugBuffer[DEBUG_BUF_SIZE];
static sigjmp_buf badreadjmpbuf;  

void log_msg(const char *fmt, ...)
{
#if LOG_TO_STDERR
	return;
#endif

	va_list argptr;
	va_start(argptr, fmt);
	(void)vsnprintf(DebugBuffer, DEBUG_BUF_SIZE, fmt, argptr);
	va_end(argptr);
	(void)fprintf(stderr, "%s\n", DebugBuffer);

}/* log_msg */

void Dpt(const char *fmt, ...)
{
#if LOG_TO_STDERR
    return;
#endif

	va_list argptr;
	va_start(argptr, fmt);
	(void)vsnprintf(DebugBuffer, DEBUG_BUF_SIZE, fmt, argptr);
	va_end(argptr);
	(void)fprintf(stderr, "%s\n", DebugBuffer);

}/* Dpt */

static void badreadfunc(int signo)  
{  
    /*write(STDOUT_FILENO, "catch\n", 6);*/  
    siglongjmp(badreadjmpbuf, 1);  
}

//*****************************************************************************
//															
//function		  : verifies that the calling process has read access to the specified range of memory
//name            : isbadreadptr
//parama		   
//	void *ptr       :	memory address
//	int length		:	size of block
//
//author          : 
//last edit date  : 
//last edit time  : 
//
//*****************************************************************************
int isbadreadptr(void *ptr, int length)  
{  
    struct sigaction sa, osa;  
    int ret = 0;  
    int ntmp ;
    char btmp;
    
    /*init new handler struct*/  
    sa.sa_handler = badreadfunc;  
    sigemptyset(&sa.sa_mask);  
    sa.sa_flags = 0;  
 
    /*retrieve old and set new handlers*/  
    if(sigaction(ERROR_SIGNAL, &sa, &osa)<0)  
        return (-1);  
    
    if(sigsetjmp(badreadjmpbuf, 1) == 0)  
    {  
        int i, hi=length/sizeof(int), remain=length%sizeof(int);  
        int* pi = (int *)ptr;  
        char* pc = (char*)ptr + hi;  
        for(i=0;i<hi;i++)  
        {  
            ntmp = *(pi+i);  
        }  
        for(i=0;i<remain;i++)  
        {  
            btmp = *(pc+i);  
        }  
        
    }  
    else  
    {  
        ret = 1;  
    }  
    
    /*restore prevouis signal actions*/  
    if(sigaction(ERROR_SIGNAL, &osa, NULL)<0)  
        return (-1);  
    
    return ret;  
}/* isbadreadptr */

//*****************************************************************************
//															
//function		  : Locate the error message
//name            : ccid_error
//parama		   
//	int error				:	error code
//	const char *file		:	file path
//	int line				: where the line number
//	const char *function    : where the function name
//
//author          : 
//last edit date  : 
//last edit time  : 
//
//*****************************************************************************
void ccid_error(int error, const char *file, int line, const char *function)
{
#if LOG_TO_STDERR
    return;
#endif
	const char *text;
	char var_text[30];
    
	switch (error)
	{
		case 0x00:
			text = "Command not supported or not allowed";
			break;
            
		case 0x01:
			text = "Wrong command length";
			break;
            
		case 0x05:
			text = "Invalid slot number";
			break;
            
		case 0xA2:
			text = "Card short-circuiting. Card powered off";
			break;
            
		case 0xA3:
			text = "ATR too long (> 33)";
			break;
            
		case 0xAB:
			text = "No data exchanged";
			break;
            
		case 0xB0:
			text = "Reader in EMV mode and T=1 message too long";
			break;
            
		case 0xBB:
			text = "Protocol error in EMV mode";
			break;
            
		case 0xBD:
			text = "Card error during T=1 exchange";
			break;
            
		case 0xBE:
			text = "Wrong APDU command length";
			break;
            
		case 0xE0:
			text = "Slot busy";
			break;
            
		case 0xEF:
			text = "PIN cancelled";
			break;
            
		case 0xF0:
			text = "PIN timeout";
			break;
            
		case 0xF2:
			text = "Busy with autosequence";
			break;
            
		case 0xF3:
			text = "Deactivated protocol";
			break;
            
		case 0xF4:
			text = "Procedure byte conflict";
			break;
            
		case 0xF5:
			text = "Class not supported";
			break;
            
		case 0xF6:
			text = "Protocol not supported";
			break;
            
		case 0xF7:
			text = "Invalid ATR checksum byte (TCK)";
			break;
            
		case 0xF8:
			text = "Invalid ATR first byte";
			break;
            
		case 0xFB:
			text = "Hardware error";
			break;
            
		case 0xFC:
			text = "Overrun error";
			break;
            
		case 0xFD:
			text = "Parity error during exchange";
			break;
            
		case 0xFE:
			text = "Card absent or mute";
			break;
            
		case 0xFF:
			text = "Activity aborted by Host";
			break;
            
		default:
			if ((error >= 1) && (error <= 127))
				(void)snprintf(var_text, sizeof(var_text), "error on byte %d",
                               error);
			else
				(void)snprintf(var_text, sizeof(var_text),
                               "Unknown CCID error: 0x%02X", error);
            
			text = var_text;
			break;
	}
	log_msg( "%s:%d:%s %s", file, line, function, text);
    
} /* ccid_error */