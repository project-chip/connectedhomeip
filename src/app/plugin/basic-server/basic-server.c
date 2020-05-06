/***************************************************************************/ /**
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
 *
 ******************************************************************************/

#ifdef ZAP_TEST
#include "utest.h"
#endif

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include ZAP_AF_API_STACK
#ifdef ZAP_AF_API_DEBUG_PRINT
#include ZAP_AF_API_DEBUG_PRINT
#endif
#include ZAP_AF_API_ZCL_CORE

void zapClusterBasicServerCommandResetToFactoryDefaultsRequestHandler(const ZapCommandContext_t *context,
                                                                      const ZapClusterBasicServerCommandResetToFactoryDefaultsRequest_t *request)
{
  zapCorePrintln("RX: ResetToFactoryDefaults");

  zapSendDefaultResponse(context, ZAP_STATUS_SUCCESS);

  zapResetAttributes(context->endpointId);
  zapReportingConfigurationsFactoryReset(context->endpointId);
}
