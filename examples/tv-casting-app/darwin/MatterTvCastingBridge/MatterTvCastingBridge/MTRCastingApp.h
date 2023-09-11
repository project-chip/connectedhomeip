/**
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "MTRDataSource.h"
#import "MatterError.h"

#ifndef MTRCastingApp_h
#define MTRCastingApp_h

/**
 * @brief MTRCastingApp represents an app that can cast content to a Casting Player.
 */
@interface MTRCastingApp : NSObject

/**
 * Returns a shared instance of the MTRCastingApp
 */
+ (MTRCastingApp * _Nullable)getSharedInstance;

/**
 * @brief Initializes the MTRCastingApp with appParameters
 *
 * @param dataSource provides all the parameters required to initialize the MTRCastingApp
 */
- (MatterError * _Nonnull)initializeWithDataSource:(id<MTRDataSource> _Nonnull)dataSource;

/**
 * @brief Starts the Matter server that the MTRCastingApp runs on and registers all the necessary delegates
 */
- (MatterError * _Nonnull)start;

/**
 * @brief Stops the Matter server that the MTRCastingApp runs on
 */
- (MatterError * _Nonnull)stop;

@end

#endif /* MTRCastingApp_h */
