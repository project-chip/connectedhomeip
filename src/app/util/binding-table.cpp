/* See Project CHIP LICENSE file for licensing information. */

/**
 * @file Basic implementation of a binding table.
 */

#include "binding-table.h"
#include "gen/gen_config.h"

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
