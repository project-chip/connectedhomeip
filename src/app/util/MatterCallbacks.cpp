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

// The defines below are using link-time callback and should be removed
//
// TODO: applications should be converted to use DataModelCallbacks instead
//       of relying on weak linkage
void __attribute__((weak)) MatterPreAttributeReadCallback(const chip::app::ConcreteAttributePath & attributePath) {}
void __attribute__((weak)) MatterPostAttributeReadCallback(const chip::app::ConcreteAttributePath & attributePath) {}
void __attribute__((weak)) MatterPreAttributeWriteCallback(const chip::app::ConcreteAttributePath & attributePath) {}
void __attribute__((weak)) MatterPostAttributeWriteCallback(const chip::app::ConcreteAttributePath & attributePath) {}
CHIP_ERROR __attribute__((weak)) MatterPreCommandReceivedCallback(const chip::app::ConcreteCommandPath & commandPath,
                                                                  const chip::Access::SubjectDescriptor & subjectDescriptor)
{
    return CHIP_NO_ERROR;
}
void __attribute__((weak)) MatterPostCommandReceivedCallback(const chip::app::ConcreteCommandPath & commandPath,
                                                             const chip::Access::SubjectDescriptor & subjectDescriptor)
{}

namespace chip {
namespace {

class WeakRedirectCallbacks : public DataModelCallbacks
{
public:
    void AttributeOperation(OperationType operation, OperationOrder order, const chip::app::ConcreteAttributePath & path) override
    {
        switch (operation)
        {
        case OperationType::Read:
            switch (order)
            {
            case OperationOrder::Pre:
                MatterPreAttributeReadCallback(path);
                break;
            case OperationOrder::Post:
                MatterPostAttributeReadCallback(path);
                break;
            }
            break;
        case OperationType::Write:
            switch (order)
            {
            case OperationOrder::Pre:
                MatterPreAttributeWriteCallback(path);
                break;
            case OperationOrder::Post:
                MatterPostAttributeWriteCallback(path);
                break;
            }
            break;
        }
    }

    CHIP_ERROR PreCommandReceived(const chip::app::ConcreteCommandPath & commandPath,
                                  const chip::Access::SubjectDescriptor & subjectDescriptor) override
    {
        return MatterPreCommandReceivedCallback(commandPath, subjectDescriptor);
    }

    void PostCommandReceived(const chip::app::ConcreteCommandPath & commandPath,
                             const chip::Access::SubjectDescriptor & subjectDescriptor) override
    {

        MatterPostCommandReceivedCallback(commandPath, subjectDescriptor);
    }
};

WeakRedirectCallbacks gWeakCallbacks;
DataModelCallbacks * gInstance = &gWeakCallbacks;

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
