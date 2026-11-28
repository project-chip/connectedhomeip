/**
 *
 *    Copyright (c) 2024-2026 Project CHIP Authors
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

#import "MCCastingApp.h"

#import <Foundation/Foundation.h>

#ifndef MCCastingApp_Internal_h
#define MCCastingApp_Internal_h

@interface MCCastingApp ()

/**
 * @brief (Private internal method) Updates the MCCommissionableDataProvider stored in this CastingApp with the updated CommissionableData
 * to be used for the next commissioning session. Calling this function is mandatory for the
 * Casting Player/Commissioner-Generated passcode commissioning flow, since the commissioning session's PAKE
 * verifier needs to be updated with the user entered passcode.
 * Calling this function is also mandatory for Fixed Passcode Commissioning.
 * The update is serialized on the Matter work queue.
 */
- (NSError * _Nullable)updateCommissionableDataProvider;

@end

#endif /*  MCCastingApp_Internal_h */
