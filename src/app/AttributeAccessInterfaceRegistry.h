/**
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceCache.h>

#include <cstddef>
#include <iterator>

namespace chip {
namespace app {

class AttributeAccessInterfaceRegistry
{
public:
    class Iterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = AttributeAccessInterface;
        using pointer           = value_type *;
        using reference         = value_type &;

        explicit Iterator(AttributeAccessInterface * value) : mValue(value) {}
        Iterator(Iterator &&)                  = default;
        Iterator(const Iterator &)             = default;
        Iterator & operator=(const Iterator &) = default;
        Iterator & operator=(Iterator &&)      = default;

        reference operator*() const { return *mValue; }
        pointer operator->() const { return mValue; }

        Iterator & operator++()
        {
            if (mValue != nullptr)
            {
                mValue = mValue->GetNext();
            }
            return *this;
        }

        Iterator operator++(int)
        {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        friend bool operator==(const Iterator & a, const Iterator & b) { return a.mValue == b.mValue; }
        friend bool operator!=(const Iterator & a, const Iterator & b) { return a.mValue != b.mValue; }

    private:
        AttributeAccessInterface * mValue;
    };

    /// Iterator is for single for loop iterations (do not store it as Register/Unregister may
    /// change registry content)
    Iterator begin() { return Iterator(mAttributeAccessOverrides); }
    Iterator end() { return Iterator(nullptr); }

    /**
     * Register an attribute access override.  It will remain registered until the
     * endpoint it's registered for is disabled (or until shutdown if it's
     * registered for all endpoints) or until it is explicitly unregistered.
     * Registration will fail if there is an already-registered override for the
     * same set of attributes.
     *
     * @return false if there is an existing override that the new one would
     *               conflict with.  In this case the override is not registered.
     * @return true if registration was successful.
     */
    bool Register(AttributeAccessInterface * attrOverride);

    /**
     * Unregister an attribute access override (for example if the object
     * implementing AttributeAccessInterface is being destroyed).
     */
    void Unregister(AttributeAccessInterface * attrOverride);

    /**
     * Unregister all attribute access interfaces that match this given endpoint.
     */
    void UnregisterAllForEndpoint(EndpointId endpointId);

    /**
     *  Get the registered attribute access override. nullptr when attribute access override is not found.
     */
    AttributeAccessInterface * Get(EndpointId aEndpointId, ClusterId aClusterId);

    static AttributeAccessInterfaceRegistry & Instance();

private:
    AttributeAccessInterface * mAttributeAccessOverrides = nullptr;
    AttributeAccessInterfaceCache mAttributeAccessInterfaceCache;
};

} // namespace app
} // namespace chip
