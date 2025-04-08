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

    dispatch_queue_attr_t qos = dispatch_queue_attr_make_with_qos_class(DISPATCH_QUEUE_SERIAL,
        QOS_CLASS_USER_INITIATED,
        -1);
    _matterQueue = dispatch_queue_create("matterQueue", qos);
    _deviceList = [NSMutableArray new];
    
    self.chipController = InitializeMTR();
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];

    [self updatePairedDevices];
    
    [self.chipController startBrowseForCommissionables:self queue:_matterQueue];
}

// Update DeviceList

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

// MARK: MTRCommissionableBrowserDelegate

- (void)controller:(MTRDeviceController *)controller didFindCommissionableDevice:(nonnull MTRCommissionableBrowserResult *)device
{
    
}

- (void)controller:(MTRDeviceController *)controller didFindNetworkRecoverableDevice:(nonnull MTRNetworkRecoverableBrowserResult *)device
{
    for (MatterDevice * d in _deviceList) {
        if ([d.recoveryId isEqualToNumber: device.recoveryID]) {
            d.isNetworkRecoverable = true;
            [self updateDevice:d];
            
            dispatch_async(dispatch_get_main_queue(),
                ^{
                    [self.tableView reloadData];
                });
        }
    }
}

- (void)controller:(nonnull MTRDeviceController *)controller didRemoveCommissionableDevice:(nonnull MTRCommissionableBrowserResult *)device {

}

- (void)updatePairedDevices
{
    uint64_t nextDeviceID = MTRGetNextAvailableDeviceID();
    for (uint64_t i = 0; i < nextDeviceID; i++) {
        if (MTRIsDevicePaired(i)) {

            [self updateDevice:[[MatterDevice alloc] initWithNodeId:i]];

            dispatch_async(dispatch_get_main_queue(),
                ^{
                    [self.tableView reloadData];
                });

            MTRGetConnectedDeviceWithID(i,
                ^(MTRBaseDevice * _Nullable device,
                    NSError * _Nullable error) {
                    if (error) {
                        NSString * resultLog = [[NSString alloc] initWithFormat:@"Unable to get connected device: Error: %@",
                                                                 error];
                        NSLog(@"%@", resultLog);
                        return;
                    }

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
                                            
                                            dispatch_async(dispatch_get_main_queue(),
                                                ^{
                                                    [self.tableView reloadData];
                                                });
                                        }];

                                        [onOff subscribeAttributeOnOffWithParams:[[MTRSubscribeParams alloc] initWithMinInterval:@(0) maxInterval:@(10)] subscriptionEstablished:^{
                                            NSLog(@"OnOff subscription sstablished...");
                                        } reportHandler:^(NSNumber * _Nullable value,
                                            NSError * _Nullable error) {
                                            MatterDevice * d = [self getDeviceWithNodeId:i];
                                            d.deviceType = deviceType.deviceType;
                                            d.onOff = [value boolValue];
                                            [self updateDevice:d];

                                            dispatch_async(dispatch_get_main_queue(),
                                                ^{
                                                    [self.tableView reloadData];
                                                });
                                        }];

                                        MTRBaseClusterBasicInformation * basicCluster = [[MTRBaseClusterBasicInformation alloc] initWithDevice:device endpointID:@0 queue:self->_matterQueue];
                                        [basicCluster readAttributeProductNameWithCompletion:^(NSString * _Nullable value, NSError * _Nullable error) {
                                            MatterDevice * d = [self getDeviceWithNodeId:i];
                                            d.deviceType = deviceType.deviceType;
                                            d.produceName = value;
                                            [self updateDevice:d];

                                            dispatch_async(dispatch_get_main_queue(),
                                                ^{
                                                    [self.tableView reloadData];
                                                });
                                        }];
                                    } break;
                                    case 259: {
                                        MatterDevice * d = [self getDeviceWithNodeId:i];
                                        d.deviceType = @259;
                                        
                                        MTRBaseClusterBasicInformation * basicCluster = [[MTRBaseClusterBasicInformation alloc] initWithDevice:device endpointID:@0 queue:self->_matterQueue];
                                        [basicCluster readAttributeProductNameWithCompletion:^(NSString * _Nullable value, NSError * _Nullable error) {
                                            d.produceName = value;
                                            [self updateDevice:d];

                                            dispatch_async(dispatch_get_main_queue(),
                                                ^{
                                                    [self.tableView reloadData];
                                                });
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
                    cell.textLabel.text = @"loading...";
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
                cell.textLabel.text = @"loading...";
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
