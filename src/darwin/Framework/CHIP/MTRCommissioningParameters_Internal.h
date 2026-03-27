/**
 *    Copyright (c) 2022-2024 Project CHIP Authors
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

/**
 * We want to be able to copy MTRCommissioningParameters, but not commit to that
 * as public API yet.
 */
@interface MTRCommissioningParameters () <NSCopying>

/**
 * Whether to prevent network scans, even in cases when we do not have network
 * credentials.
 *
 * This is needed to support the old commissioning API, which does not have a
 * way to respond to those network scans and continue commissioning once they
 * complete.
 *
 * Defaults to NO.
 */
@property (nonatomic, assign) BOOL preventNetworkScans;

@end
