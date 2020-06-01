//
//  OnOffViewController.m
//  CHIPTool
//
//  Created by Bhaskar on 6/1/20.
//  Copyright © 2020 CHIP. All rights reserved.
//

#import "OnOffViewController.h"

@interface OnOffViewController ()

@end

@implementation OnOffViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
}

/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/
- (IBAction)onButtonTapped:(id)sender {
    NSLog(@"On tapped");
}

- (IBAction)offButtonTapped:(id)sender {
    NSLog(@"Off tapped");
}

- (IBAction)toggleButtonTapped:(id)sender {
    NSLog(@"Toggle tapped");
}

@end
