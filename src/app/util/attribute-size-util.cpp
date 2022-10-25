/**
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

#include <app/util/af.h>

#include <app-common/zap-generated/attribute-type.h>

using namespace chip;

static const uint8_t attributeSizes[] = {
#include <app-common/zap-generated/attribute-size.h>
};

uint8_t emberAfGetDataSize(uint8_t dataType)
{
    for (unsigned i = 0; (i + 1) < sizeof(attributeSizes); i += 2)
    {
        if (attributeSizes[i] == dataType)
        {
            return attributeSizes[i + 1];
        }
    }

    return 0;
}
