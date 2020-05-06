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

#include "zap.h"

// Default Response Stubs
ZapStatus_t zapSendDefaultResponse(const ZapCommandContext_t *context, ZapStatus_t status)
{
  return ZAP_STATUS_SUCCESS;
}

// Reporting Configuration Stubs
void zapReportingConfigurationsFactoryReset(ZapEndpointId_t endpointId)
{
  return;
}

//Attribute Management Stubs
void zapResetAttributes(ZapEndpointId_t endpointId)
{
  return;
}

ZapStatus_t zapReadAttribute(ZapEndpointId_t endpointId,
                             const ZapClusterSpec_t *clusterSpec,
                             ZapAttributeId_t attributeId,
                             void *buffer,
                             size_t bufferLength)
{
  return ZAP_STATUS_SUCCESS;
}

ZapStatus_t zapWriteAttribute(ZapEndpointId_t endpointId,
                              const ZapClusterSpec_t *clusterSpec,
                              ZapAttributeId_t attributeId,
                              const void *buffer,
                              size_t bufferLength)
{
  return ZAP_STATUS_SUCCESS;
}

//Event Management Stubs
EventQueue emAppEventQueue;

Event *zapEventFind(EventQueue *queue,
                    EventActions *actions,
                    EventPredicate predicate,
                    void *data,
                    bool all)
{
  return 0;
}

void zapEventSetDelayMs(Event *event, uint32_t delay)
{
  return;
}

// Buffer Management Stubs
uint8_t *zapGetBufferPointer(Buffer buffer)
{
  return 0;
}

Buffer zapReallyAllocateBuffer(uint16_t length, bool unused)
{
  return 0;
}