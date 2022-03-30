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

#import "RootViewController.h"
#import "BindingsViewController.h"
#import "EnumerateViewController.h"
#import "FabricUIViewController.h"
#import "MultiAdminViewController.h"
#import "OnOffViewController.h"
#import "QRCodeViewController.h"
#import "TemperatureSensorViewController.h"
#import "UnpairDevicesViewController.h"
#import "WiFiViewController.h"

@implementation RootViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    self.self.navigationItem.title = @"Connected Home over IP";
    [self setUpTableView];
}

- (void)setUpTableView
{
    self.tableView = [[UITableView alloc] initWithFrame:self.view.bounds style:UITableViewStylePlain];
    self.tableView.delegate = self;
    self.tableView.dataSource = self;
    [self.view addSubview:self.tableView];
    self.options = @[
        @"QRCode scanner", @"Enumeration", @"Light on / off cluster", @"Temperature Sensor", @"Bindings", @"WiFi Configuration",
        @"Enable Pairing", @"Unpair Devices", @"Fabric Management"
    ];
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return _options.count;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString * cellIdentifier = @"CHIPToolOptionCell";

    UITableViewCell * cell = [self.tableView dequeueReusableCellWithIdentifier:cellIdentifier];

    if (cell == nil) {
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:cellIdentifier];
    }
    cell.textLabel.text = [_options objectAtIndex:indexPath.row];
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath;
{
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
    switch (indexPath.row) {
    case 0:
        [self pushQRCodeScannerWithSkipCheck:NO];
        break;
    case 1:
        [self pushEnumeration];
        break;
    case 2:
        [self pushLightOnOffCluster];
        break;
    case 3:
        [self pushTemperatureSensor];
        break;
    case 4:
        [self pushBindings];
        break;
    case 5:
        [self pushNetworkConfiguration];
        break;
    case 6:
        [self pushMultiAdmin];
        break;
    case 7:
        [self pushUnpairDevices];
        break;
    case 8:
        [self pushFabric];
        break;
    default:
        break;
    }
}

- (void)pushFabric
{
    FabricUIViewController * controller = [FabricUIViewController new];
    [self.navigationController pushViewController:controller animated:YES];
}

- (void)pushBindings
{
    BindingsViewController * controller = [BindingsViewController new];
    [self.navigationController pushViewController:controller animated:YES];
}

- (void)pushTemperatureSensor
{
    TemperatureSensorViewController * controller = [TemperatureSensorViewController new];
    [self.navigationController pushViewController:controller animated:YES];
}

- (void)pushNetworkConfiguration
{
    WiFiViewController * controller = [WiFiViewController new];
    [self.navigationController pushViewController:controller animated:YES];
}

- (void)pushQRCodeScannerWithSkipCheck:(BOOL)skipIosCheck
{
    if (skipIosCheck) {
        QRCodeViewController * controller = [QRCodeViewController new];
        [self.navigationController pushViewController:controller animated:YES];
    } else {
        if (@available(iOS 15.4, *)) {
            // Device using the required iOS version (>= 15.4)
            [self pushQRCodeScannerWithSkipCheck:YES];
        } else {
            // Device NOT using the required iOS version (< 15.4)
            // Show a warning, but let the user continue
            UIAlertController * alertController =
                [UIAlertController alertControllerWithTitle:@"Warning"
                                                    message:@"QRCode scanner to pair a matter device requires iOS >= 15.4"
                                             preferredStyle:UIAlertControllerStyleAlert];
            __weak typeof(self) weakSelf = self;
            [alertController addAction:[UIAlertAction actionWithTitle:@"I understand"
                                                                style:UIAlertActionStyleDefault
                                                              handler:^(UIAlertAction * action) {
                                                                  typeof(self) strongSelf = weakSelf;
                                                                  if (strongSelf) {
                                                                      [strongSelf pushQRCodeScannerWithSkipCheck:YES];
                                                                  }
                                                              }]];
            [self presentViewController:alertController animated:YES completion:nil];
        }
    }
}

- (void)pushEnumeration
{
    EnumerateViewController * controller = [EnumerateViewController new];
    [self.navigationController pushViewController:controller animated:YES];
}

- (void)pushMultiAdmin
{
    MultiAdminViewController * controller = [MultiAdminViewController new];
    [self.navigationController pushViewController:controller animated:YES];
}

- (void)pushLightOnOffCluster
{
    OnOffViewController * controller = [OnOffViewController new];
    [self.navigationController pushViewController:controller animated:YES];
}

- (void)pushUnpairDevices
{
    UnpairDevicesViewController * controller = [UnpairDevicesViewController new];
    [self.navigationController pushViewController:controller animated:YES];
}
@end
