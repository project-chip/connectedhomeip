/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <stdlib.h>

#include <pw_unit_test/framework.h>

#include <platform/CHIPDeviceConfig.h>

int main(int argc, char * argv[])
{
    testing::InitGoogleTest(&argc, argv);
    if (argc == 2 && atoi(argv[1]) == 1)
    {
        return RUN_ALL_TESTS();
    }
    return 0;
}
