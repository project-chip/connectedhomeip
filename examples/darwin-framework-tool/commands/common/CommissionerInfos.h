/*
 *   Copyright (c) 2025 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

inline constexpr char kIdentityAlpha[] = "alpha";
inline constexpr char kIdentityBeta[] = "beta";
inline constexpr char kIdentityGamma[] = "gamma";
inline constexpr char kControllerIdPrefix[] = "8DCADB14-AF1F-45D0-B084-00000000000";

static NSNumber * GetCommissionerFabricId(const char * identity)
{
    if (strcmp(identity, kIdentityAlpha) == 0) {
        return @(1);
    } else if (strcmp(identity, kIdentityBeta) == 0) {
        return @(2);
    } else if (strcmp(identity, kIdentityGamma) == 0) {
        return @(3);
    } else {
        NSLog(@"Unknown commissioner name: %s. Supported names are [%s, %s, %s]", identity, kIdentityAlpha, kIdentityBeta, kIdentityGamma);
        exit(EXIT_FAILURE);
    }

    return @(0); // This should never happens.
}
