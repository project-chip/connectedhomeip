/*
 *    Copyright (c) 2021 Project CHIP Authors
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
#pragma once

#include <access/SubjectDescriptor.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>

namespace chip {

/// Allows for application hooks for processing attribute and command operations
class DataModelCallbacks
{
public:
    enum class OperationType
    {
        Read,
        Write
    };

    enum class OperationOrder
    {
        Pre,
        Post
    };

    virtual ~DataModelCallbacks() = default;

    /// This callback is called on attribute operations:
    /// - for reads and writes
    /// - both before and after attribute read/writes
    ///
    /// NOTE: PostRead is only called on read success.
    virtual void AttributeOperation(OperationType operation, OperationOrder order, const chip::app::ConcreteAttributePath & path) {}

    /// This callback is called once for every command dispatch, before the dispatch is actually
    /// done towards the receiver.
    ///
    /// This method is called once for every CommandDataIB (i.e. it may be called several times
    /// in the case of batch invoke, where a single `InvokeRequestMessage` may contain several
    /// CommandDataIB entries).
    ///
    /// Returning an error here will prevent the command to be dispatched further
    virtual CHIP_ERROR PreCommandReceived(const chip::app::ConcreteCommandPath & commandPath,
                                          const chip::Access::SubjectDescriptor & subjectDescriptor)
    {
        return CHIP_NO_ERROR;
    }

    /// This callback is called once for every command dispatch, after the dispatch is actually
    /// done towards the receiver.
    ///
    /// This method is called once for every CommandDataIB (i.e. it may be called several times
    /// in the case of batch invoke, where a single `InvokeRequestMessage` may contain several
    /// CommandDataIB entries).
    virtual void PostCommandReceived(const chip::app::ConcreteCommandPath & commandPath,
                                     const chip::Access::SubjectDescriptor & subjectDescriptor)
    {}

    static DataModelCallbacks * GetInstance();
    static DataModelCallbacks * SetInstance(DataModelCallbacks * newInstance);
};

} // namespace chip
