/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#ifndef MOCK_ATTRIBUTE_PERSISTENCE_PROVIDER_H_
#define MOCK_ATTRIBUTE_PERSISTENCE_PROVIDER_H_

#include <app/AttributePersistenceProvider.h>
#include <app/util/af.h>
#include <lib/support/Span.h>

namespace unify::matter_bridge {
namespace Test {
class MockAttributePersistenceProvider : public chip::app::AttributePersistenceProvider
{
public:
    MockAttributePersistenceProvider() {}
    CHIP_ERROR WriteValue(const chip::app::ConcreteAttributePath & aPath, const chip::ByteSpan & aValue) { return CHIP_NO_ERROR; }
    CHIP_ERROR ReadValue(const chip::app::ConcreteAttributePath & aPath, const EmberAfAttributeMetadata * aMetadata,
                         chip::MutableByteSpan & aValue)
    {
        return CHIP_NO_ERROR;
    }
};
} // namespace Test
} // namespace unify::matter_bridge

#endif // MOCK_ATTRIBUTE_PERSISTENCE_PROVIDER_H_
