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
#import "CHIPUIViewUtils.h"
#import "DefaultsUtils.h"
#import "EnumerateViewController.h"
#import "FabricUIViewController.h"
#import "MultiAdminViewController.h"
#import "OnOffViewController.h"
#import "QRCodeViewController.h"
#import "SwitchBindingViewController.h"
#import "TemperatureSensorViewController.h"
#import "UnpairDevicesViewController.h"
#import "WiFiViewController.h"
#import "MatterDevice.h"

@implementation RootViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    self.self.navigationItem.title = @"Connected Home over IP";
    [self setUpTableView];

    _matterQueue = dispatch_queue_create("matterQueue", DISPATCH_QUEUE_SERIAL);
    _deviceList = [NSMutableArray new];
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];

    [self updatePairedDevices];
        
    self.updateTimer = [NSTimer scheduledTimerWithTimeInterval:5.0
                                                        target:self
                                                      selector:@selector(updatePairedDevices)
                                                      userInfo:nil
                                                       repeats:YES];
    
    self.chipController = InitializeMTR();
    [self.chipController setDeviceControllerDelegate:self queue:_matterQueue];
    
    self.discoverTimer = [NSTimer scheduledTimerWithTimeInterval:5.0
                                                        target:self
                                                      selector:@selector(discoverRecoverableNodes)
                                                      userInfo:nil
                                                       repeats:YES];
}

- (void)viewWillDisappear:(BOOL)animated
{
    [super viewWillDisappear:animated];

    [self.updateTimer invalidate];
    [self.discoverTimer invalidate];
    
    [self.chipController removeDeviceControllerDelegate:self];
}

- (void)updateDevice:(MatterDevice *)device
{
    NSUInteger index = [_deviceList indexOfObjectPassingTest:^BOOL(MatterDevice * obj, NSUInteger idx, BOOL *stop) {
        return obj.nodeId == device.nodeId;
    }];
    
    if (index != NSNotFound) {
        _deviceList[index] = device;
    } else {
        [_deviceList addObject:device];
    }
    
    dispatch_async(dispatch_get_main_queue(),
        ^{
            [self.tableView reloadData];
        });
}

- (void)discoverRecoverableNodes {
    [self.chipController discoverRecoverableNodes:self queue:_matterQueue timeout:3];
}

- (MatterDevice *)getDeviceWithNodeId:(uint64_t)nodeId
{
    for (MatterDevice * device in _deviceList) {
        if (device.nodeId == nodeId) {
            return device;
        }
    }
    
    return nil;
}

- (void)retrieveAndSendWiFiCredentials:(MatterDevice*)device
{
    UIAlertController * alertController =
        [UIAlertController alertControllerWithTitle:@"WiFi Configuration"
                                            message:@"Input network SSID and password that your phone is connected to."
                                     preferredStyle:UIAlertControllerStyleAlert];
    [alertController addTextFieldWithConfigurationHandler:^(UITextField * textField) {
        textField.placeholder = @"Network SSID";
        textField.clearButtonMode = UITextFieldViewModeWhileEditing;
        textField.borderStyle = UITextBorderStyleRoundedRect;

        NSString * networkSSID = MTRGetDomainValueForKey(MTRToolDefaultsDomain, kNetworkSSIDDefaultsKey);
        if ([networkSSID length] > 0) {
            textField.text = networkSSID;
        }
    }];
    [alertController addTextFieldWithConfigurationHandler:^(UITextField * textField) {
        [textField setSecureTextEntry:YES];
        textField.placeholder = @"Password";
        textField.clearButtonMode = UITextFieldViewModeWhileEditing;
        textField.borderStyle = UITextBorderStyleRoundedRect;
        textField.secureTextEntry = YES;

        NSString * networkPassword = MTRGetDomainValueForKey(MTRToolDefaultsDomain, kNetworkPasswordDefaultsKey);
        if ([networkPassword length] > 0) {
            textField.text = networkPassword;
        }
    }];
    [alertController addAction:[UIAlertAction actionWithTitle:@"Cancel"
                                                        style:UIAlertActionStyleDefault
                                                      handler:^(UIAlertAction * action) {
                                                      }]];

    __weak typeof(self) weakSelf = self;
    [alertController
        addAction:[UIAlertAction actionWithTitle:@"Send"
                                           style:UIAlertActionStyleDefault
                                         handler:^(UIAlertAction * action) {
                                             typeof(self) strongSelf = weakSelf;
                                             if (strongSelf) {
                                                 NSArray * textfields = alertController.textFields;
                                                 UITextField * networkSSID = textfields[0];
                                                 UITextField * networkPassword = textfields[1];
                                                 if ([networkSSID.text length] > 0) {
                                                     MTRSetDomainValueForKey(
                                                         MTRToolDefaultsDomain, kNetworkSSIDDefaultsKey, networkSSID.text);
                                                 }

                                                 if ([networkPassword.text length] > 0) {
                                                     MTRSetDomainValueForKey(
                                                         MTRToolDefaultsDomain, kNetworkPasswordDefaultsKey, networkPassword.text);
                                                 }
                                                 NSLog(@"New SSID: %@ Password: %@", networkSSID.text, networkPassword.text);

                                                 UIAlertController *loading = [UIAlertController alertControllerWithTitle:nil
                                                                                                                   message:@"Network recover in progress...\n\n"
                                                                                                            preferredStyle:UIAlertControllerStyleAlert];
                                                 UIActivityIndicatorView *indicator = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleMedium];
                                                 indicator.translatesAutoresizingMaskIntoConstraints = NO;
                                                 [loading.view addSubview:indicator];
                                                 [indicator.centerXAnchor constraintEqualToAnchor:loading.view.centerXAnchor].active = YES;
                                                 [indicator.bottomAnchor constraintEqualToAnchor:loading.view.bottomAnchor constant:-20].active = YES;
                                                 [indicator startAnimating];

                                                 strongSelf.loadingAlert = loading;
                                                 [strongSelf presentViewController:loading animated:YES completion:nil];
                                                 
                                                 NSError * error;

                                                 if(![strongSelf.chipController recoverDevice:device.nodeId recoveryIdentifier:device.recoveryId wifiSSID:networkSSID.text wifiCredentials:networkPassword.text error:&error]){
                                                     NSLog(@"Failed to recover device %llu, with error %@", device.nodeId, error);
                                                 }
                                                 
                                             }
                                         }]];
    [self presentViewController:alertController animated:YES completion:nil];
}

// MARK: MTRCommissionableBrowserDelegate

- (void)controller:(MTRDeviceController *)controller didFindCommissionableDevice:(nonnull MTRCommissionableBrowserResult *)device
{
    
}

- (void)controller:(MTRDeviceController *)controller didFindNetworkRecoverableDevice:(nonnull MTRNetworkRecoverableBrowserResult *)device
{
    for (MatterDevice * d in _deviceList) {
        if (d.recoveryId == [device.recoveryID unsignedLongLongValue]) {
            if (!d.isNetworkRecoverable) {
                d.isNetworkRecoverable = true;
                [self updateDevice:d];
            }
        }
    }
}

- (void)controller:(nonnull MTRDeviceController *)controller didRemoveCommissionableDevice:(nonnull MTRCommissionableBrowserResult *)device {

}

// MARK: MTRDeviceControllerDelegate
- (void)controller:(MTRDeviceController *)controller
    networkRecoverComplete:(nonnull NSError *)error nodeID:(nonnull NSNumber *)nodeID
{
    dispatch_async(dispatch_get_main_queue(), ^{
        if (self.loadingAlert) {
            [self dismissViewControllerAnimated:YES completion:^{
                self.loadingAlert = nil;

                if (error == nil) {
                    MatterDevice * d = [self getDeviceWithNodeId:[nodeID unsignedLongLongValue]];
                    if (d != nil && d.isNetworkRecoverable) {
                        d.isNetworkRecoverable = false;
                        [self updateDevice:d];
                    }
                    
                }
                
                NSString *title = error ? @"Error" : @"Success";
                NSString *message = error ? error.localizedDescription : @"The network has been recovered successfully!";
                UIAlertController *resultAlert = [UIAlertController alertControllerWithTitle:title
                                                                                     message:message
                                                                              preferredStyle:UIAlertControllerStyleAlert];
                [resultAlert addAction:[UIAlertAction actionWithTitle:@"OK" style:UIAlertActionStyleDefault handler:nil]];
                [self presentViewController:resultAlert animated:YES completion:nil];
            }];
        }
    });
}

- (void)updatePairedDevices
{
    uint64_t nextDeviceID = MTRGetNextAvailableDeviceID();
    for (uint64_t i = 0; i < nextDeviceID; i++) {
        if (MTRIsDevicePaired(i)) {

            MatterDevice * d = [self getDeviceWithNodeId:i];
            if (d == nil) {
                [self updateDevice:[[MatterDevice alloc] initWithNodeId:i]];
            }

            MTRGetConnectedDeviceWithID(i,
                ^(MTRBaseDevice * _Nullable device,
                    NSError * _Nullable error) {
                    if (error) {
                        NSString * resultLog = [[NSString alloc] initWithFormat:@"Unable to get connected device: Error: %@",
                                                                 error];
                        NSLog(@"%@", resultLog);
                        return;
                    }
                
                MTRBaseClusterGeneralCommissioning * commissioningCluster = [[MTRBaseClusterGeneralCommissioning alloc] initWithDevice:device endpointID:@0 queue:self->_matterQueue];
                [commissioningCluster readAttributeRecoveryIdentifierWithCompletion:^(NSData * _Nullable value, NSError * _Nullable error) {
                    if (value != nil) {
                        unsigned long long receivedLongLongValue = 0;
                        [value getBytes:&receivedLongLongValue length:8];
                        MTRSetRecoveryIdentifier(i, receivedLongLongValue);
                        
                        MatterDevice * d = [self getDeviceWithNodeId:i];
                        d.recoveryId = receivedLongLongValue;
                        [self updateDevice:d];
                    }
                }];
                

                    MTRBaseClusterDescriptor * descriptorCluster = [[MTRBaseClusterDescriptor alloc] initWithDevice:device
                                                                                                         endpointID:@0
                                                                                                              queue:self->_matterQueue];
                    NSLog(@"Reading parts list to get list of endpoints in use...");
                    [descriptorCluster readAttributePartsListWithCompletion:^(
                        NSArray<NSNumber *> * _Nullable endpointsInUse, NSError * _Nullable error) {
                        if (error) {
                            NSString * resultLog = [[NSString alloc] initWithFormat:@"Unable to read parts list: Error: %@",
                                                                     error];
                            NSLog(@"%@",
                                resultLog);
                            return;
                        }

                        NSString * resultLog = [[NSString alloc] initWithFormat:@"Got list of endpoints in Use: %@",
                                                                 endpointsInUse];
                        NSLog(@"%@",
                            resultLog);

                        for (NSNumber * endpoint in endpointsInUse) {
                            MTRBaseClusterDescriptor * descriptorCluster =
                                [[MTRBaseClusterDescriptor alloc] initWithDevice:device
                                                                      endpointID:endpoint
                                                                           queue:self->_matterQueue];
                            [descriptorCluster readAttributeDeviceTypeListWithCompletion:^(
                                NSArray * _Nullable value, NSError * _Nullable error) {
                                if (error) {
                                    NSString * resultLog = [[NSString alloc]
                                        initWithFormat:@"Unable to read device list for Endpoint:%@ Error: %@",
                                        endpoint,
                                        error];
                                    NSLog(@"%@",
                                        resultLog);
                                    return;
                                }

                                for (MTRDescriptorClusterDeviceTypeStruct * deviceType in value) {
                                    switch ([deviceType.deviceType integerValue]) {
                                    case 268:
                                    case 269: {
                                        MTRBaseClusterOnOff * onOff = [[MTRBaseClusterOnOff alloc] initWithDevice:device
                                                                                                       endpointID:@1
                                                                                                            queue:self->_matterQueue];
                                        [onOff readAttributeOnOffWithCompletion:^(NSNumber * _Nullable value,
                                            NSError * _Nullable error) {
                                            MatterDevice * d = [self getDeviceWithNodeId:i];
                                            d.deviceType = deviceType.deviceType;
                                            d.onOff = [value boolValue];
                                            [self updateDevice:d];
                                        }];

                                        [onOff subscribeAttributeOnOffWithParams:[[MTRSubscribeParams alloc] initWithMinInterval:@(0) maxInterval:@(10)] subscriptionEstablished:^{
                                            NSLog(@"OnOff subscription sstablished...");
                                        } reportHandler:^(NSNumber * _Nullable value,
                                            NSError * _Nullable error) {
                                            MatterDevice * d = [self getDeviceWithNodeId:i];
                                            d.deviceType = deviceType.deviceType;
                                            d.onOff = [value boolValue];
                                            [self updateDevice:d];
                                        }];

                                        MTRBaseClusterBasicInformation * basicCluster = [[MTRBaseClusterBasicInformation alloc] initWithDevice:device endpointID:@0 queue:self->_matterQueue];
                                        [basicCluster readAttributeProductNameWithCompletion:^(NSString * _Nullable value, NSError * _Nullable error) {
                                            if (value != nil) {
                                                MTRSetDeviceName(i, value);
                                            }
                                            MatterDevice * d = [self getDeviceWithNodeId:i];
                                            d.deviceType = deviceType.deviceType;
                                            d.produceName = value;
                                            [self updateDevice:d];
                                        }];
                                    } break;
                                    case 259: {
                                        MatterDevice * d = [self getDeviceWithNodeId:i];
                                        d.deviceType = @259;
                                        
                                        MTRBaseClusterBasicInformation * basicCluster = [[MTRBaseClusterBasicInformation alloc] initWithDevice:device endpointID:@0 queue:self->_matterQueue];
                                        [basicCluster readAttributeProductNameWithCompletion:^(NSString * _Nullable value, NSError * _Nullable error) {
                                            if (value != nil) {
                                                MTRSetDeviceName(i, value);
                                            }
                                            d.produceName = value;
                                            [self updateDevice:d];
                                        }];
                                    } break;
                                    default:
                                        break;
                                    }
                                    break;
                                }
                            }];
                        }
                    }];
                });
        } else {
            MatterDevice * d = [self getDeviceWithNodeId:i];
            if (d != nil) {
                [_deviceList removeObject:d];
                
                dispatch_async(dispatch_get_main_queue(),
                    ^{
                        [self.tableView reloadData];
                    });
            }
        }
    }
}

- (void)setUpTableView
{
    self.tableView = [[UITableView alloc] initWithFrame:self.view.bounds style:UITableViewStylePlain];
    self.tableView.delegate = self;
    self.tableView.dataSource = self;
    [self.view addSubview:self.tableView];
//    self.options = @[
//        @"QRCode scanner", @"Enumeration", @"Light on / off cluster", @"Temperature Sensor", @"Bindings", @"WiFi Configuration",
//        @"Enable Pairing", @"Unpair Devices", @"Fabric Management"
//    ];
    self.options = @[
        @"QRCode scanner", @"Enumeration", @"Unpair Devices", @"Fabric Management"
    ];
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 2;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    switch (section) {
    case 1:
            return _deviceList.count;
    default:
        return _options.count;
    }
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    if (indexPath.section == 0) {
        static NSString * cellIdentifier = @"CHIPToolOptionCell";

        UITableViewCell * cell = [self.tableView dequeueReusableCellWithIdentifier:cellIdentifier];

        if (cell == nil) {
            cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:cellIdentifier];
        }
        cell.textLabel.text = [_options objectAtIndex:indexPath.row];
        return cell;
    } else {
        static NSString * cellIdentifier = @"CHIPToolDeviceCell";

        UITableViewCell * cell = [self.tableView dequeueReusableCellWithIdentifier:cellIdentifier];

        if (cell == nil) {
            cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleSubtitle reuseIdentifier:cellIdentifier];
            cell.selectionStyle = UITableViewCellSelectionStyleNone;
        }
        
        MatterDevice * device = _deviceList[indexPath.row];
        
        switch ([device.deviceType integerValue]) {
            case 0:
            case 268:
            case 269:
            {
                cell.imageView.image = [UIImage systemImageNamed:device.onOff ? @"lightbulb.fill" : @"lightbulb" withConfiguration:[UIImageSymbolConfiguration configurationWithPointSize:60]];
                UIColor * onColor = [UIColor colorWithRed:0.0 green:0.5 blue:0.0 alpha:1.0];
                UIColor * offColor = [UIColor blackColor];
                cell.imageView.tintColor = device.onOff ? onColor : offColor;
                if (device.produceName != nil) {
                    cell.textLabel.text = device.produceName;
                } else {
                    cell.textLabel.text = @"";
                }
                if (device.isNetworkRecoverable) {
                    cell.imageView.image = [UIImage systemImageNamed:@"network.slash" withConfiguration:[UIImageSymbolConfiguration configurationWithPointSize:60]];
                    cell.imageView.tintColor = [UIColor colorWithRed:0.5 green:0 blue:0 alpha:1.0];
                    cell.detailTextLabel.textColor = [UIColor colorWithRed:0 green:0.5 blue:0 alpha:1.0];
                    cell.detailTextLabel.text = @"Click to recover network";
                } else {
                    cell.detailTextLabel.text = @"";
                }
            }
                break;
                
            case 259:
            {
                cell.imageView.image = [UIImage systemImageNamed:@"lightswitch.on.square" withConfiguration:[UIImageSymbolConfiguration configurationWithPointSize:60]];
                cell.imageView.tintColor = [UIColor blackColor];
            }
                break;
            default:
            {
                cell.textLabel.text = @"UNKNOWN";
            }
                break;
        }
        
        return cell;
    }
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath;
{
    [tableView deselectRowAtIndexPath:indexPath animated:YES];

    switch (indexPath.section) {
    case 0: {
        switch (indexPath.row) {
        case 0:
            [self pushQRCodeScannerWithSkipCheck:NO];
            break;
        case 1:
            [self pushEnumeration];
            break;
//        case 2:
//            [self pushLightOnOffCluster];
//            break;
//        case 3:
//            [self pushTemperatureSensor];
//            break;
//        case 4:
//            [self pushBindings];
//            break;
//        case 5:
//            [self pushNetworkConfiguration];
//            break;
//        case 6:
//            [self pushMultiAdmin];
//            break;
//        case 7:
//            [self pushUnpairDevices];
//            break;
//        case 8:
//            [self pushFabric];
//            break;
          case 2:
              [self pushUnpairDevices];
              break;
          case 3:
              [self pushFabric];
              break;
        default:
            break;
        }
    } break;
    case 1: {
        
        MatterDevice * device = _deviceList[indexPath.row];
        
        if (device.isNetworkRecoverable) {
            [self retrieveAndSendWiFiCredentials: device];
            return;;
        }
        
        switch ([device.deviceType integerValue]) {
            case 268:
            case 269:
            {
                if (MTRGetConnectedDeviceWithID(device.nodeId,
                        ^(MTRBaseDevice * _Nullable chipDevice,
                            NSError * _Nullable error) {
                            if (chipDevice) {
                                MTRBaseClusterOnOff * onOff = [[MTRBaseClusterOnOff alloc] initWithDevice:chipDevice
                                                                                               endpointID:@1
                                                                                                    queue:self->_matterQueue];
                                [onOff toggleWithCompletion:^(NSError * error) {
                                    NSString * resultString = (error != nil)
                                        ? [NSString stringWithFormat:@"An error occurred: 0x%02lx",
                                                    error.code]
                                        : @"Toggle command success";
                                    NSLog(@"%@", resultString);
                                }];

                            } else {
                                NSLog(@"Failed to establish a connection with the device");
                            }
                        })) {
                    NSLog(@"Waiting for connection with the device");
                } else {
                    NSLog(@"Failed to trigger the connection with the device");
                }
            }
                break;
                
            case 259:
            {
                [self pushSwitchBinding:device.nodeId];
            }
                break;
            default:
                break;
        }
    }
    default:
        break;
    }
}

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
    if (indexPath.section == 0) {
        return 44;
    }

    return 80;
    ;
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

- (void)pushSwitchBinding:(uint64_t)value
{
    SwitchBindingViewController * controller = [SwitchBindingViewController new];
    controller.deviceId = value;
    NSMutableArray *filteredDeviceList = [NSMutableArray new];
    [_deviceList enumerateObjectsUsingBlock:^(MatterDevice * obj, NSUInteger idx, BOOL * _Nonnull stop) {
        if ([obj.deviceType isEqualToNumber:@268] || [obj.deviceType isEqualToNumber:@269]) {
            [filteredDeviceList addObject:obj];
        }
    }];
    controller.lightDeviceList = [filteredDeviceList copy];
    [self.navigationController pushViewController:controller animated:YES];
}
@end
