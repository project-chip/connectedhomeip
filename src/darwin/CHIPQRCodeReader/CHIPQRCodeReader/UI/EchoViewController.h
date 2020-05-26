//
//  EchoViewController.h
//  CHIPQRCodeReader
//
//  Created by Sagar Dhawan on 5/21/20.
//  Copyright © 2020 CHIP. All rights reserved.
//

#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

@interface EchoViewController : UIViewController
@property (weak, nonatomic) IBOutlet UITextField *serverIPTextField;
@property (weak, nonatomic) IBOutlet UITextField *serverPortTextField;
@property (weak, nonatomic) IBOutlet UITextField *messageTextField;
@property (weak, nonatomic) IBOutlet UILabel *resultLabel;
@property (weak, nonatomic) IBOutlet UIButton *sendButton;

- (IBAction)sendAction:(id)sender;

@end

NS_ASSUME_NONNULL_END
