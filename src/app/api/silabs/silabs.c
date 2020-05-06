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
  return emberZclDefaultResponse(context, status);
}

// Reporting Configuration Stubs
void zapReportingConfigurationsFactoryReset(ZapEndpointId_t endpointId)
{
  emberZclReportingConfigurationsFactoryReset(endpointId);
}

//Attribute Management Stubs
void zapResetAttributes(ZapEndpointId_t endpointId)
{
  emberZclResetAttributes(endpointId);
}

ZapStatus_t zapReadAttribute(ZapEndpointId_t endpointId,
                             const ZapClusterSpec_t *clusterSpec,
                             ZapAttributeId_t attributeId,
                             void *buffer,
                             size_t bufferLength)
{
  return emberZclReadAttribute(endpointId, clusterSpec, attributeId, buffer, bufferLength);
}

ZapStatus_t zapWriteAttribute(ZapEndpointId_t endpointId,
                              const ZapClusterSpec_t *clusterSpec,
                              ZapAttributeId_t attributeId,
                              const void *buffer,
                              size_t bufferLength)
{
  return emberZclWriteAttribute(endpointId, clusterSpec, attributeId, buffer, bufferLength);
}

//Event Management Stubs
EventQueue emAppEventQueue;

Event *zapEventFind(EventQueue *queue,
                    EventActions *actions,
                    EventPredicate predicate,
                    void *data,
                    bool all)
{
  return emFindEvents(queue, actions, predicate, data, all);
}

void zapEventSetDelayMs(Event *event, uint32_t delay)
{
  emberEventSetDelayMs(event, delay);
}

// Buffer Management Stubs
uint8_t *zapGetBufferPointer(Buffer buffer)
{
  return emGetBufferPointer(buffer);
}

Buffer zapReallyAllocateBuffer(uint16_t length, bool unused)
{
  return emReallyAllocateBuffer(length, unused);
}