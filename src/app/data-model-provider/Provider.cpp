/*
 *    Copyright (c) 2026 Project CHIP Authors
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
#include "platform/LockTracker.h"
#include <app/data-model-provider/Provider.h>

namespace chip::app::DataModel {

void Provider::RegisterAttributeChangeListener(AttributeChangeListener & listener)
{
    assertChipStackLockedByCurrentThread();

    listener.SetNextAttributeChangeListener(mAttributeChangeListenersHead);
    mAttributeChangeListenersHead = &listener;
}

void Provider::UnregisterAttributeChangeListener(AttributeChangeListener & listener)
{
    assertChipStackLockedByCurrentThread();

    // If any active iterator is about to process this listener, advance it
    // to the next one to avoid processing a removed listener.
    ActiveIterator * active = mActiveIterators;
    while (active)
    {
        if (active->expectedNext == &listener)
        {
            active->expectedNext = listener.GetNextAttributeChangeListener();
        }
        active = active->nextIterator;
    }

    if (mAttributeChangeListenersHead == &listener)
    {
        mAttributeChangeListenersHead = listener.GetNextAttributeChangeListener();
        listener.SetNextAttributeChangeListener(nullptr);
        return;
    }

    AttributeChangeListener * current = mAttributeChangeListenersHead;
    while (current && (current->GetNextAttributeChangeListener() != &listener))
    {
        current = current->GetNextAttributeChangeListener();
    }

    if (current)
    {
        current->SetNextAttributeChangeListener(listener.GetNextAttributeChangeListener());
        listener.SetNextAttributeChangeListener(nullptr);
    }
}

void Provider::NotifyAttributeChanged(const ConcreteAttributePath & path, AttributeChangeType type)
{
    assertChipStackLockedByCurrentThread();

    // Register this iteration on the stack of active iterators.
    // This allows UnregisterAttributeChangeListener to update us if needed.
    ActiveIterator iter;
    iter.expectedNext = mAttributeChangeListenersHead;
    iter.nextIterator = mActiveIterators;
    mActiveIterators  = &iter;

    while (iter.expectedNext)
    {
        AttributeChangeListener * current = iter.expectedNext;
        iter.expectedNext                 = current->GetNextAttributeChangeListener();
        current->OnAttributeChanged(path, type);
    }

    mActiveIterators = iter.nextIterator;
}

void Provider::NotifyEndpointChanged(EndpointId endpointId, EndpointChangeType type)
{
    assertChipStackLockedByCurrentThread();

    // Register this iteration on the stack of active iterators.
    // This allows UnregisterAttributeChangeListener to update us if needed.
    ActiveIterator iter;
    iter.expectedNext = mAttributeChangeListenersHead;
    iter.nextIterator = mActiveIterators;
    mActiveIterators  = &iter;

    while (iter.expectedNext)
    {
        AttributeChangeListener * current = iter.expectedNext;
        iter.expectedNext                 = current->GetNextAttributeChangeListener();
        current->OnEndpointChanged(endpointId, type);
    }

    mActiveIterators = iter.nextIterator;
}

} // namespace chip::app::DataModel
