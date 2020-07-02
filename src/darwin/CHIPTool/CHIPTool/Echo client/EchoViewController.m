/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#import "EchoViewController.h"

#import <CHIP/CHIP.h>
#import <UIKit/UIKit.h>

#define RESULT_DISPLAY_DURATION 5.0 * NSEC_PER_SEC

@interface EchoViewController ()

@property (weak, nonatomic) IBOutlet UITextField * messageTextField;
@property (weak, nonatomic) IBOutlet UIButton * sendButton;

@end

@implementation EchoViewController

- (void)viewDidLoad
{
    [super viewDidLoad];

    // make the send button slightly prettier
    self.sendButton.layer.cornerRadius = 5;
    self.sendButton.clipsToBounds = YES;
}

- (void)dismissKeyboard
{
    [super dismissKeyboard];
    [self.messageTextField resignFirstResponder];
}

- (IBAction)sendAction:(id)sender
{
    NSString * msg = [self.messageTextField text];
    if (msg.length == 0) {
        msg = [self.messageTextField placeholder];
    }

    [self reconnectIfNeeded];

    // send message
    if ([self.chipController isConnected]) {
        NSError * error;
        BOOL didSend = [self.chipController sendMessage:[msg dataUsingEncoding:NSUTF8StringEncoding] error:&error];
        if (!didSend) {
            [self postResult:[@"Error: " stringByAppendingString:error.localizedDescription]];
        } else {
            [self postResult:@"Message Sent"];
        }
    } else {
        [self postResult:@"Controller not connected"];
    }
}
@end
