/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#import "DeviceSelector.h"

#import "CHIPUIViewUtils.h"
#import "DefaultsUtils.h"

@interface DeviceSelector ()
@property (readwrite) CHIPDevice * chipDevice;
@property (readwrite) uint64_t nextDeviceID;
@end

@implementation DeviceSelector {
    NSMutableArray * _deviceList;
    UIPickerView * _devicePicker;
    NSUInteger _selectedDeviceIndex;
}

- (id)init
{
    if (self = [super init]) {
        _nextDeviceID = CHIPGetNextAvailableDeviceID();
        _selectedDeviceIndex = self.nextDeviceID + 1;
        [self refreshDeviceList];
        [self setupView];
    }
    return self;
}

- (void)refreshDeviceList
{
    _deviceList = [NSMutableArray new];
    for (uint64_t i = 0; i < _nextDeviceID; i++) {
        if (CHIPGetPairedDeviceWithID(i) != nil) {
            [_deviceList addObject:[@(i) stringValue]];
        }
    }
    _selectedDeviceIndex = 0;
}

- (void)selectDevice
{
    if ([_deviceList count] > 0) {
        uint64_t deviceID = [_deviceList[_selectedDeviceIndex] intValue];
        _chipDevice = CHIPGetPairedDeviceWithID(deviceID);
    }
}

- (CHIPDevice *)selectedDevice
{
    return _chipDevice;
}

- (void)setupView
{
    if ([_deviceList count] > 0) {
        self.text = [_deviceList objectAtIndex:_selectedDeviceIndex];
    }
    _devicePicker = [[UIPickerView alloc] initWithFrame:CGRectMake(0, 100, 0, 0)];
    self.inputView = _devicePicker;
    [_devicePicker setDataSource:self];
    [_devicePicker setDelegate:self];
    self.delegate = self;

    UIToolbar * deviceSelectButtonView = [[UIToolbar alloc] init];
    [deviceSelectButtonView sizeToFit];
    UIBarButtonItem * deviceSelectButton = [[UIBarButtonItem alloc] initWithTitle:@"Select Device"
                                                                            style:UIBarButtonItemStylePlain
                                                                           target:self
                                                                           action:@selector(deviceSelectClicked:)];
    UIBarButtonItem * flexible = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace
                                                                               target:self
                                                                               action:nil];
    [deviceSelectButtonView setItems:[NSArray arrayWithObjects:flexible, deviceSelectButton, nil]];
    self.inputAccessoryView = deviceSelectButtonView;

    [self selectDevice];
}

// MARK: UIPickerView

- (void)pickerView:(UIPickerView *)pickerView didSelectRow:(NSInteger)row inComponent:(NSInteger)component
{
    NSLog(@"%@", [_deviceList objectAtIndex:row]);
    self.text = [NSString stringWithFormat:@"%@", [_deviceList objectAtIndex:row]];
}

- (NSInteger)pickerView:(UIPickerView *)pickerView numberOfRowsInComponent:(NSInteger)component
{
    return [_deviceList count];
}

- (NSInteger)numberOfComponentsInPickerView:(UIPickerView *)pickerView
{
    return 1;
}

- (NSString *)pickerView:(UIPickerView *)pickerView titleForRow:(NSInteger)row forComponent:(NSInteger)component
{
    return [_deviceList objectAtIndex:row];
}

- (CGFloat)pickerView:(UIPickerView *)pickerView widthForComponent:(NSInteger)component
{
    return 200;
}

- (IBAction)deviceSelectClicked:(id)sender
{
    _selectedDeviceIndex = [_deviceList indexOfObject:self.text];
    [self resignFirstResponder];
    [self selectDevice];
}

// MARK: CHIPDeviceControllerDelegate
- (void)deviceControllerOnConnected
{
    NSLog(@"Status: Device connected");
}

- (void)deviceControllerOnError:(nonnull NSError *)error
{
}

@end
