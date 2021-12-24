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

/**
 * @file Basic implementation of a binding table.
 */

#include <app/util/binding-table.h>
#include <zap-generated/gen_config.h>

static EmberBindingTableEntry bindingTable[EMBER_BINDING_TABLE_SIZE];
static uint8_t sBindingTableSize;

EmberStatus emberGetBinding(uint8_t index, EmberBindingTableEntry * result)
{
    if (index >= EMBER_BINDING_TABLE_SIZE)
    {
        return EMBER_BAD_ARGUMENT;
    }

    *result = bindingTable[index];
    return EMBER_SUCCESS;
}

EmberStatus emberAppendBinding(EmberBindingTableEntry * result)
{
    if (sBindingTableSize >= EMBER_BINDING_TABLE_SIZE)
    {
        return EMBER_NO_BUFFERS;
    }

    bindingTable[sBindingTableSize++] = *result;
    return EMBER_SUCCESS;
}

EmberStatus emberClearBinding(void)
{
    for (uint8_t i = 0; i < sBindingTableSize; i++)
    {
        bindingTable[i].type = EMBER_UNUSED_BINDING;
    }
    sBindingTableSize = 0;

    return EMBER_SUCCESS;
}

uint8_t emberGetBindingTableSize(void)
{
    return sBindingTableSize;
}
