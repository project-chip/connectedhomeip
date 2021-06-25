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
#include <gen/gen_config.h>

static EmberBindingTableEntry bindingTable[EMBER_BINDING_TABLE_SIZE];

EmberStatus emberGetBinding(uint8_t index, EmberBindingTableEntry * result)
{
    if (index >= EMBER_BINDING_TABLE_SIZE)
    {
        return EMBER_BAD_ARGUMENT;
    }

    *result = bindingTable[index];
    return EMBER_SUCCESS;
}

EmberStatus emberSetBinding(uint8_t index, EmberBindingTableEntry * result)
{
    if (index >= EMBER_BINDING_TABLE_SIZE)
    {
        return EMBER_BAD_ARGUMENT;
    }

    bindingTable[index] = *result;
    return EMBER_SUCCESS;
}

EmberStatus emberDeleteBinding(uint8_t index)
{
    if (index >= EMBER_BINDING_TABLE_SIZE)
    {
        return EMBER_BAD_ARGUMENT;
    }

    bindingTable[index].type = EMBER_UNUSED_BINDING;
    return EMBER_SUCCESS;
}
