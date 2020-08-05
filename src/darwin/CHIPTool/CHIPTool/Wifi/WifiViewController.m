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

#import "WifiViewController.h"
#import "DefaultsUtils.h"

@implementation WifiViewController

- (void)viewDidLoad
{
    [super viewDidLoad];

    [self.networkPassword setSecureTextEntry:YES];

    self.saveButton.layer.cornerRadius = 5;
    self.saveButton.clipsToBounds = YES;

    self.clearButton.layer.cornerRadius = 5;
    self.clearButton.clipsToBounds = YES;

    UITapGestureRecognizer * tap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(dismissKeyboard)];
    [self.view addGestureRecognizer:tap];
}

- (void)viewWillAppear:(BOOL)animated
{
    [self fillNetworkConfigWithDefaults];
}

- (void)dismissKeyboard
{
    [super dismissKeyboard];
    [self.networkSSID resignFirstResponder];
    [self.networkPassword resignFirstResponder];
}

- (void)fillNetworkConfigWithDefaults
{
    NSString * networkSSID = CHIPGetDomainValueForKey(kCHIPToolDefaultsDomain, kNetworkSSIDDefaultsKey);
    if ([networkSSID length] > 0) {
        self.networkSSID.text = networkSSID;
    }

    NSString * networkPassword = CHIPGetDomainValueForKey(kCHIPToolDefaultsDomain, kNetworkPasswordDefaultsKey);
    if ([networkPassword length] > 0) {
        self.networkPassword.text = networkPassword;
    }
}

- (IBAction)saveCredientials:(id)sender
{
    if ([self.networkSSID.text length] > 0) {
        CHIPSetDomainValueForKey(kCHIPToolDefaultsDomain, kNetworkSSIDDefaultsKey, self.networkSSID.text);
    }

    if ([self.networkPassword.text length] > 0) {
        CHIPSetDomainValueForKey(kCHIPToolDefaultsDomain, kNetworkPasswordDefaultsKey, self.networkPassword.text);
    }

    [self.navigationController popViewControllerAnimated:YES];
}

- (IBAction)clearCredientials:(id)sender
{
    if ([self.networkSSID.text length] > 0) {
        CHIPSetDomainValueForKey(kCHIPToolDefaultsDomain, kNetworkSSIDDefaultsKey, nil);
    }

    if ([self.networkPassword.text length] > 0) {
        CHIPSetDomainValueForKey(kCHIPToolDefaultsDomain, kNetworkPasswordDefaultsKey, nil);
    }

    self.networkSSID.text = @"";
    self.networkPassword.text = @"";
}
@end
