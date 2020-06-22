//
//  OnOffViewController.m
//  CHIPTool
//
//  Created by Bhaskar on 6/1/20.
//  Copyright © 2020 CHIP. All rights reserved.
//

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

    [self postResult:@"Turning light on"];
}

- (IBAction)offButtonTapped:(id)sender
{
    [self reconnectIfNeeded];

    [self.onOff lightOff];

    [self postResult:@"Turning light off"];
}

- (IBAction)toggleButtonTapped:(id)sender
{
    [self reconnectIfNeeded];

    [self.onOff toggleLight];

    [self postResult:@"Toggling light"];
}

@end
