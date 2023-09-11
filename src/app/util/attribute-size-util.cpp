/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
