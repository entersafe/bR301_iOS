/*
 * Support for bR301(Bluetooth) smart card reader
 *
 * ReaderInterface.m: The implement source code of ReaderInterface.h
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

#import <Foundation/Foundation.h>
#import "ReaderInterface.h"
#import "EADSessionController.h"
#import "ft_ccid.h"

@implementation ReaderInterface

- (id)init
{
    self = [super init];
    if (self) {
    }
    
    return self;
}

- (BOOL)isReaderAttached;
{
    return gIsOpen;
}

- (BOOL)isCardAttached;
{
    
    LONG rv;
	
	int readerCount=0;
	SCARD_READERSTATE rgReaderStates[1];
  
    _ccid_descriptor *ccid_descriptor = get_ccid_descriptor(0);
    
    SCARDCONTEXT ContxtHandle;
    
    ContxtHandle = (SCARDCONTEXT)ccid_descriptor;
        
    rv = SCardGetStatusChange(ContxtHandle, INFINITE, rgReaderStates, readerCount);
    if(rv!=SCARD_S_SUCCESS)
    {
        return NO;
    }
    else
    {
        if(rgReaderStates[0].dwEventState & SCARD_STATE_EMPTY)
        {
                return NO;
        }
        else
        {
                return YES;
        }
	}

    return NO;
}

- (void)  setDelegate:(id<ReaderInterfaceDelegate>)delegate
{
    EADSessionController *sessionController = [EADSessionController sharedController];
    
    [sessionController setDelegate:delegate];
    
}
@end

