//
//  OnOffViewController.m
//  CHIPTool
//
//  Created by Bhaskar on 6/1/20.
//  Copyright © 2020 CHIP. All rights reserved.
//

#import "OnOffViewController.h"

@interface OnOffViewController ()
@property (weak, nonatomic) IBOutlet UIButton * onButton;
@property (weak, nonatomic) IBOutlet UIButton * offButton;
@property (weak, nonatomic) IBOutlet UIButton * toggleButton;

@end

@implementation OnOffViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
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
    NSLog(@"On tapped");
}

- (IBAction)offButtonTapped:(id)sender
{
    NSLog(@"Off tapped");
}

- (IBAction)toggleButtonTapped:(id)sender
{
    NSLog(@"Toggle tapped");
}

@end
