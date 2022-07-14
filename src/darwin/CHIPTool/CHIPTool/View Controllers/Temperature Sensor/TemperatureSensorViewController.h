//
//  TemperatureSensorViewController.h
//  CHIPTool
//
//  Created by Shana Azria on 07/10/2020.
//  Copyright © 2020 CHIP. All rights reserved.
//

#import <Matter/Matter.h>
#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

@interface TemperatureSensorViewController : UIViewController

/**
 * Return the current controller, if any.
 */
+ (nullable TemperatureSensorViewController *)currentController;

@end

NS_ASSUME_NONNULL_END
