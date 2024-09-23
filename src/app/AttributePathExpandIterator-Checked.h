/*
 *    Copyright (c) 2024 Project CHIP Authors
 *    All rights reserved.
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

#pragma once

#include <app/AttributePathExpandIterator-DataModel.h>
#include <app/AttributePathExpandIterator-Ember.h>

namespace chip {
namespace app {

class AttributePathExpandIteratorChecked
{
public:
    AttributePathExpandIteratorChecked(DataModel::Provider * dataModel, SingleLinkedListNode<AttributePathParams> * attributePath);

    bool Next();
    bool Get(ConcreteAttributePath & aPath);
    void ResetCurrentCluster();
    void ResetTo(SingleLinkedListNode<AttributePathParams> * paths);

private:
    AttributePathExpandIteratorDataModel mDataModelIterator;
    AttributePathExpandIteratorEmber mEmberIterator;

    void CheckOutputsIdentical(const char * msg);
};

} // namespace app
} // namespace chip
