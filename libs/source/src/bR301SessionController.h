//
//  bR301SessionController.h
//  bR301SessionController
//
//  Created by Ben on 12/2/15.
//  Copyright © 2015 FTSAFE. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <ExternalAccessory/ExternalAccessory.h>
#import "ReaderInterface.h"

@interface bR301SessionController : NSObject <EAAccessoryDelegate, NSStreamDelegate> {
    
    //    id<ReaderInterfaceDelegate> detegate;
}

+ (bR301SessionController *)sharedController;

- (void)  setDelegate:(id<ReaderInterfaceDelegate>)Delegate;

-(void) RegisterAccessoryConnectNotification;
-(void) UnRegisterAccessoryConnectNotification;
- (int)identifyAccessoryCount;
- (int)writeData:(unsigned char *)data withLength:(unsigned int) len;
- (int)readData:(unsigned char *) data withbytesToRead:(unsigned int*)bytesToRead;


//@property (atomic,strong) NSRecursiveLock *theLock;//atomic多线程访问互斥
@property (nonatomic, strong) EAAccessory *accessory;
@property (nonatomic, strong) NSString *protocolString;
@property (nonatomic, strong) EASession *session;
@property (nonatomic, strong)  NSThread *thread;
@property (nonatomic, strong)  NSThread *CardStatusDidChange;
@property (nonatomic, strong)  id<ReaderInterfaceDelegate> detegate;

@end
