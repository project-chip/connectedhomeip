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

#import "OnOffViewController.h"
#import <CHIP/CHIP.h>

@interface OnOffViewController ()
@property (weak, nonatomic) IBOutlet UIButton * onButton;
@property (weak, nonatomic) IBOutlet UIButton * offButton;
@property (weak, nonatomic) IBOutlet UIButton * toggleButton;
@property (readwrite) CHIPOnOff * onOff;

@end

@implementation OnOffViewController

- (void)viewDidLoad
{
    [super viewDidLoad];

    self.onOff = [[CHIPOnOff alloc] initWithDeviceController:self.chipController];

    // make the buttons slightly prettier
    self.onButton.layer.cornerRadius = 5;
    self.onButton.clipsToBounds = YES;
    self.offButton.layer.cornerRadius = 5;
    self.offButton.clipsToBounds = YES;
    self.toggleButton.layer.cornerRadius = 5;
    self.toggleButton.clipsToBounds = YES;
}

/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/
- (IBAction)onButtonTapped:(id)sender
{
    [self reconnectIfNeeded];

    [self.onOff lightOn];
}

- (IBAction)offButtonTapped:(id)sender
{
    [self reconnectIfNeeded];

    [self.onOff lightOff];
}

- (IBAction)toggleButtonTapped:(id)sender
{
    [self reconnectIfNeeded];

    [self.onOff toggleLight];
}

@end
