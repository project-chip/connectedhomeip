/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
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

#include <stdio.h>
#include <stdlib.h>

#include <support/UnitTestRegistration.h>

using namespace ::chip;

int main()
{
    printf("Open IoT SDK unit-tests run...\r\n");
    int status = RunRegisteredUnitTests();
    printf("Test status: %d\r\n", status);
    printf("Open IoT SDK unit-tests completed\r\n");

    return status;
}
