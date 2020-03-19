/*
 *
 *    <COPYRIGHT>
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
 * This is a sample unit test.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

//#include "standalone/TapInterface.c"

void TapInterface_Init_test1()
{
    printf("---Running Test--- %s\n", __FUNCTION__);
    // assert(TapInterface_Init(NULL, NULL, NULL) == ERR_ARG);
}

int main()
{
    printf("---Running Test--- tests from %s\n", __FILE__);
    TapInterface_Init_test1();
    return 0;
}
