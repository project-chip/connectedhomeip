/**
 *    Copyright (c) 2024 Project CHIP Authors
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

#import "SwitchBindingViewController.h"
#import "DefaultsUtils.h"
#import "MatterDevice.h"

@implementation SwitchBindingViewController

- (void)viewDidLoad
{
    [super viewDidLoad];

    self.view.backgroundColor = [UIColor whiteColor];
    self.selectedIndex = -1;

    dispatch_queue_attr_t qos = dispatch_queue_attr_make_with_qos_class(DISPATCH_QUEUE_SERIAL,
        QOS_CLASS_USER_INITIATED,
        -1);
    _matterQueue = dispatch_queue_create("matterQueue", qos);

    
    MTRGetConnectedDeviceWithID(_deviceId, ^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
        MTRBaseClusterBinding * bind = [[MTRBaseClusterBinding alloc] initWithDevice:device
                                                                          endpointID:@1
                                                                               queue:self->_matterQueue];

        [bind readAttributeBindingWithParams:nil completion:^(NSArray * _Nullable value, NSError * _Nullable error) {
            NSMutableArray *aclArr = [[NSMutableArray alloc] initWithArray:value];
            
            for (MTRBindingClusterTargetStruct * s in aclArr) {
                [self->_lightDeviceList enumerateObjectsUsingBlock:^(MatterDevice * d, NSUInteger idx, BOOL * _Nonnull stop) {
                    if ([s.node longValue] == d.nodeId) {
                        self->_selectedIndex = idx;
                    }
                }];
            }
            
            dispatch_async(dispatch_get_main_queue(), ^{
                [self setupUI];
            });
        }];
    });
    
    MTRGetConnectedDeviceWithID(23, ^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
        MTRBaseClusterAccessControl * acl = [[MTRBaseClusterAccessControl alloc] initWithDevice:device endpointID:@0 queue:self->_matterQueue];
        
        [acl readAttributeACLWithParams:nil completion:^(NSArray * _Nullable value, NSError * _Nullable error) {
                    
        }];
    });
}

- (void)setupUI
{
    UILabel * titleLabel = [[UILabel alloc] initWithFrame:CGRectMake(20, 100, self.view.frame.size.width - 40, 30)];
    titleLabel.text = @"Select Binding Targets";
    titleLabel.textAlignment = NSTextAlignmentCenter;
    titleLabel.font = [UIFont boldSystemFontOfSize:18];
    [self.view addSubview:titleLabel];

    for (int i = 0; i < self.lightDeviceList.count; i++) {
        UIButton * radioButton = [UIButton buttonWithType:UIButtonTypeCustom];
        radioButton.frame = CGRectMake(20, 158 + i * 60, 44, 44);
        radioButton.tag = i;
        [radioButton setImage:[UIImage systemImageNamed:@"circle"] forState:UIControlStateNormal];
        [radioButton setImage:[UIImage systemImageNamed:@"checkmark.circle.fill"] forState:UIControlStateSelected];
        [radioButton addTarget:self action:@selector(selectBindingTarget:) forControlEvents:UIControlEventTouchUpInside];
        radioButton.selected = (radioButton.tag == self.selectedIndex);
        [self.view addSubview:radioButton];

        UIImageView * bulbIcon = [[UIImageView alloc] initWithImage:[UIImage systemImageNamed:@"lightbulb" withConfiguration:[UIImageSymbolConfiguration configurationWithPointSize:60]]];
        bulbIcon.tintColor = [UIColor blackColor];
        bulbIcon.frame = CGRectMake(60, 150 + i * 60, 60, 60);
        [self.view addSubview:bulbIcon];

        UILabel * targetLabel = [[UILabel alloc] initWithFrame:CGRectMake(140, 165 + i * 60, 200, 30)];
        targetLabel.text = ((MatterDevice *)_lightDeviceList[i]).produceName;
        targetLabel.font = [UIFont systemFontOfSize:16];
        [self.view addSubview:targetLabel];
    }

    self.cancelButton = [UIButton buttonWithType:UIButtonTypeSystem];
    self.cancelButton.frame = CGRectMake(50, self.view.frame.size.height - 100, 100, 40);
    [self.cancelButton setTitle:@"Cancel" forState:UIControlStateNormal];
    [self.cancelButton addTarget:self action:@selector(cancelAction) forControlEvents:UIControlEventTouchUpInside];
    self.cancelButton.tag = 100;
    [self.view addSubview:self.cancelButton];

    self.okButton = [UIButton buttonWithType:UIButtonTypeSystem];
    self.okButton.frame = CGRectMake(self.view.frame.size.width - 150, self.view.frame.size.height - 100, 100, 40);
    [self.okButton setTitle:@"OK" forState:UIControlStateNormal];
    [self.okButton addTarget:self action:@selector(okAction) forControlEvents:UIControlEventTouchUpInside];
    self.okButton.tag = 101;
    [self.view addSubview:self.okButton];
}

- (void)selectBindingTarget:(UIButton *)sender
{
    if (sender.isSelected) {
        sender.selected = false;
        self.selectedIndex = -1;
        
        return;
    }

    self.selectedIndex = sender.tag;

    for (UIView * subview in self.view.subviews) {
        if ([subview isKindOfClass:[UIButton class]]) {
            UIButton * btn = (UIButton *) subview;

            if (btn.tag < self.lightDeviceList.count) {
                btn.selected = (btn.tag == self.selectedIndex);
            }
        }
    }
}

- (void)cancelAction
{
    [self.navigationController popViewControllerAnimated:YES];
}

- (void)okAction
{
    if (self.selectedIndex >= 0) {
        [self bindLightToSwitch:((MatterDevice *)self.lightDeviceList[self.selectedIndex]).nodeId];
    } else {
        UIAlertController *alert = [UIAlertController alertControllerWithTitle:nil message:@"Do you want to unbind all?" preferredStyle:UIAlertControllerStyleAlert];
        UIAlertAction *okBtn = [UIAlertAction actionWithTitle:@"Yes" style:UIAlertActionStyleCancel handler:^(UIAlertAction * _Nonnull action) {
            MTRGetConnectedDeviceWithID(self->_deviceId, ^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
                MTRBaseClusterBinding * bind = [[MTRBaseClusterBinding alloc] initWithDevice:device
                                                                                  endpointID:@1
                                                                                       queue:self->_matterQueue];
                [bind writeAttributeBindingWithValue:@[] completion:^(NSError * _Nullable error) {
                    if (error) {
                        NSLog(@"Fail to unbind, error:%@", error.description);
                    } else {
                        NSLog(@"Success to unbind");
                        dispatch_async(dispatch_get_main_queue(), ^{
                            UIAlertController *alert = [UIAlertController alertControllerWithTitle:nil message:@"UnBind success!" preferredStyle:UIAlertControllerStyleAlert];
                            UIAlertAction *okBtn = [UIAlertAction actionWithTitle:@"Ok" style:UIAlertActionStyleCancel handler:^(UIAlertAction * _Nonnull action) {
                                [self.navigationController popViewControllerAnimated:YES];
                            }];
                            [alert addAction:okBtn];
                            [self presentViewController:alert animated:YES completion:nil];
                        });
                    }
                }];
            });
        }];
        [alert addAction:okBtn];
        [self presentViewController:alert animated:YES completion:nil];
    }
}

- (void)bindLightToSwitch:(uint64_t)lightNode
{
    
    MTRGetConnectedDeviceWithID(lightNode, ^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
        MTRBaseClusterAccessControl * acl = [[MTRBaseClusterAccessControl alloc] initWithDevice:device endpointID:@0 queue:self->_matterQueue];
        
        [acl readAttributeACLWithParams:nil completion:^(NSArray * _Nullable value, NSError * _Nullable error) {
            NSMutableArray *aclArr = [[NSMutableArray alloc] initWithArray:@[value.firstObject]];

            MTRAccessControlClusterAccessControlEntryStruct * lightAclEntry = [MTRAccessControlClusterAccessControlEntryStruct new];
            lightAclEntry.privilege = @3;
            lightAclEntry.authMode = @2;
            lightAclEntry.subjects = @[[NSNumber numberWithUnsignedLong:self->_deviceId]];
            lightAclEntry.fabricIndex = @1;
            
            [aclArr addObject:lightAclEntry];
            
            [acl writeAttributeACLWithValue:aclArr completion:^(NSError * _Nullable error) {
                if (error) {
                    NSLog(@"Fail to writeAttributeACLWithValue:%@, error:%@",aclArr, error.description);
                } else {
                    NSLog(@"Success to write ACL");
                }
            }];
        }];
    });
    
    MTRGetConnectedDeviceWithID(_deviceId, ^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
        MTRBaseClusterBinding * bind = [[MTRBaseClusterBinding alloc] initWithDevice:device
                                                                          endpointID:@1
                                                                               queue:self->_matterQueue];

        MTRBindingClusterTargetStruct * bindingTarget = [MTRBindingClusterTargetStruct new];
        bindingTarget.node = [NSNumber numberWithLong:lightNode];
        bindingTarget.endpoint = @1;
        bindingTarget.cluster = @6;

        [bind writeAttributeBindingWithValue:@[bindingTarget] completion:^(NSError * _Nullable error) {
            if (error) {
                NSLog(@"Fail to writeAttributeBindingWithValue:%@, error:%@",@[bindingTarget], error.description);
            } else {
                NSLog(@"Success to write binding");
                
                dispatch_async(dispatch_get_main_queue(), ^{
                    UIAlertController *alert = [UIAlertController alertControllerWithTitle:nil message:@"Bind success!" preferredStyle:UIAlertControllerStyleAlert];
                    UIAlertAction *okBtn = [UIAlertAction actionWithTitle:@"Ok" style:UIAlertActionStyleCancel handler:^(UIAlertAction * _Nonnull action) {
                        [self.navigationController popViewControllerAnimated:YES];
                    }];
                    [alert addAction:okBtn];
                    [self presentViewController:alert animated:YES completion:nil];
                });
            }
        }];
    });
}

@end
