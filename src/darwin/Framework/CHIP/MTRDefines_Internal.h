/**
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

#import <Matter/MTRDefines.h>

#if __has_attribute(objc_direct)
#define MTR_DIRECT __attribute__((objc_direct))
#else
#define MTR_DIRECT
#endif

#if __has_attribute(objc_direct_members)
#define MTR_DIRECT_MEMBERS __attribute__((objc_direct_members))
#else
#define MTR_DIRECT_MEMBERS
#endif
