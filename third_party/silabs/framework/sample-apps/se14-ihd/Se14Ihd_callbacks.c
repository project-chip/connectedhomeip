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
 *
 *    Copyright (c) 2020 Silicon Labs
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
/***************************************************************************//**
 * @file
 * @brief This callback file is created for your convenience. You may add
 * application code to this file. If you regenerate this file over a previous
 * version, the previous version will be overwritten and any code you have
 * added will be lost.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/util.h"    // emberAfGetPageFrom8bitEncodedChanPg()
#include PLATFORM_HEADER

/** @brief Ncp Init
 *
 * This function is called when the network coprocessor is being initialized,
 * either at startup or upon reset.  It provides applications on opportunity to
 * perform additional configuration of the NCP.  The function is always called
 * twice when the NCP is initialized.  In the first invocation, memoryAllocation
 * will be true and the application should only issue EZSP commands that affect
 * memory allocation on the NCP.  For example, tables on the NCP can be resized
 * in the first call.  In the second invocation, memoryAllocation will be false
 * and the application should only issue EZSP commands that do not affect memory
 * allocation.  For example, tables on the NCP can be populated in the second
 * call.  This callback is not called on SoCs.
 *
 * @param memoryAllocation   Ver.: always
 */
void emberAfNcpInitCallback(boolean memoryAllocation)
{
}
