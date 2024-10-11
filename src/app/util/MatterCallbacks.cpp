/*
 *    Copyright (c) 2024 Project CHIP Authors
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
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
#include "MatterCallbacks.h"

namespace chip {
namespace {

DataModelCallbacks gNoopCallbacks;
DataModelCallbacks * gInstance = &gNoopCallbacks;

} // namespace

DataModelCallbacks * DataModelCallbacks::GetInstance()
{
    return gInstance;
}

DataModelCallbacks * DataModelCallbacks::SetInstance(DataModelCallbacks * newInstance)
{
    return std::exchange(gInstance, newInstance);
}

} // namespace chip

/// Strong-link callbacks for backwards compatibiltiy error reporting
///
/// We used to have these as __attribute__(weak) linkage in previous releases, so
/// we define these as non-weak now so that we get compile errors in case anyone tries
/// to link with them on the assumption they work (because they do not anymore)
///
/// TODO(future): we should be able to remove these once we assume everyone is updated
///               to the latest code.
void MatterPreAttributeReadCallback(const chip::app::ConcreteAttributePath & attributePath) {}
void MatterPostAttributeReadCallback(const chip::app::ConcreteAttributePath & attributePath) {}
void MatterPreAttributeWriteCallback(const chip::app::ConcreteAttributePath & attributePath) {}
void MatterPostAttributeWriteCallback(const chip::app::ConcreteAttributePath & attributePath) {}
CHIP_ERROR MatterPreCommandReceivedCallback(const chip::app::ConcreteCommandPath & commandPath,
                                            const chip::Access::SubjectDescriptor & subjectDescriptor)
{
    return CHIP_NO_ERROR;
}
void MatterPostCommandReceivedCallback(const chip::app::ConcreteCommandPath & commandPath,
                                       const chip::Access::SubjectDescriptor & subjectDescriptor)
{}
