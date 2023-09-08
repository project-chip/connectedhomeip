/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
