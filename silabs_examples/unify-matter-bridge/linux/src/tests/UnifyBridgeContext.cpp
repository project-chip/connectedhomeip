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
#include "UnifyBridgeContext.h"
#include "MockAttributePersistenceProvider.h"
#include <app/util/DataModelHandler.h>

namespace unify::matter_bridge {
namespace Test {

CHIP_ERROR UnifyBridgeContext::Init()
{
    ReturnErrorOnFailure(Super::Init());

    MockAttributePersistenceProvider persistence;
    chip::app::SetAttributePersistenceProvider(&persistence);
    InitDataModelHandler(&GetExchangeManager());

    return CHIP_NO_ERROR;
}

void UnifyBridgeContext::Shutdown()
{
    Super::Shutdown();
}

} // namespace Test
} // namespace unify::matter_bridge
