/*
 * Support for bR301(Bluetooth) smart card reader
 *
 * Copyright (C) 2014, Ben <ben@ftsafe.com>
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

#import <UIKit/UIKit.h>
#include "winscard.h"

#import "ReaderInterface.h"

@interface disopWindow : UIViewController <UITextFieldDelegate,UIAlertViewDelegate,UITableViewDelegate,UITableViewDataSource,ReaderInterfaceDelegate>{
    
    IBOutlet UIButton* dropList;
    IBOutlet UIButton* infoBut;
    IBOutlet UISwitch *cardState;
    
	IBOutlet UITextField *commandText;    
    IBOutlet UITextView *ATR_Label;
    IBOutlet UITextView *disTextView;
    
    IBOutlet UILabel* APDU_Label;
    IBOutlet UILabel* cardState_Latel;
    
    IBOutlet UIImageView *disResp;
    IBOutlet UIImageView *apduInput;
    
    SCARDHANDLE  gCardHandle;

    UIView *showInfoView;
    UIView *clearView;

}
@property (nonatomic, readonly) ReaderInterface *readInf;
-(IBAction) showInfo;
-(IBAction) powerOnFun:(id)sender;
-(IBAction) powerOffFun:(id)sender;
-(IBAction) sendCommandFun:(id)sender;
-(IBAction) textFieldDone:(id)sender; 

-(IBAction) limitCharacter:(id)sender;
-(IBAction)runBtnPressed:(id)sender;
-(void) moveToDown;


@property (nonatomic, retain) IBOutlet UIButton *powerOn;
@property (nonatomic, retain) IBOutlet UIButton *powerOff;
@property (nonatomic, retain) IBOutlet UIButton *sendCommand;


@property (nonatomic, retain) IBOutlet UIButton *getSerialNo;
@property (nonatomic, retain) IBOutlet UIButton *getCardState;
@property (nonatomic, retain) IBOutlet UIButton *writeFlash;
@property (nonatomic, retain) IBOutlet UIButton *readFlash;


@property (nonatomic,retain) UIButton* runCommand;
@property (nonatomic,retain) NSArray* listData;
@property (nonatomic,retain) NSArray* showInfoData;

@end
