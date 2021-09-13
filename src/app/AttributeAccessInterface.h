/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app/ClusterInfo.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPTLV.h>
#include <lib/core/Optional.h>

/**
 * Callback class that clusters can implement in order to interpose custom
 * attribute-handling logic.  An AttributeAccessInterface instance is associated
 * with some specific cluster.  A single instance may be used for a specific
 * endpoint or for all endpoints.
 *
 * Instances of AttributeAccessInterface that are registered via
 * registerAttributeAccessOverride will be consulted before taking the normal
 * attribute access codepath and can use that codepath as a fallback if desired.
 */
namespace chip {
namespace app {

class AttributeAccessInterface
{
public:
    /**
     * aEndpointId can be Missing to indicate that this object is meant to be
     * used with all endpoints.
     */
    AttributeAccessInterface(Optional<EndpointId> aEndpointId, ClusterId aClusterId) :
        mEndpointId(aEndpointId), mClusterId(aClusterId)
    {}
    virtual ~AttributeAccessInterface() {}

    /**
     * Callback for reading attributes.
     *
     * @param [in] aClusterInfo indicates which exact data is being read.
     * @param [in] aTLVWriter the tlv writer to put the data into.  The data
     *                        must use the AttributeDataElement::kCsTag_Data
     *                        context tag.
     * @param [out] aDataRead whether we actually tried to provide data.  If
     *                        this function returns success and aDataRead is
     *                        false, the AttributeAccessInterface did not try
     *                        to provide any data.  In this case, normal
     *                        attribute access will happen for the read.  This
     *                        may involve reading from the attribute store or
     *                        external attribute callbacks.
     */
    virtual CHIP_ERROR Read(ClusterInfo & aClusterInfo, TLV::TLVWriter * aWriter, bool * aDataRead) = 0;

    /**
     * Mechanism for keeping track of a chain of AttributeAccessInterfaces.
     */
    void SetNext(AttributeAccessInterface * aNext) { mNext = aNext; }
    AttributeAccessInterface * GetNext() const { return mNext; }

    /**
     * Check whether a this AttributeAccessInterface is relevant for a
     * particular endpoint+cluster.  An AttributeAccessInterface will be used
     * for a read from a particular cluster only when this function returns
     * true.
     */
    bool Matches(EndpointId aEndpointId, ClusterId aClusterId) const
    {
        return (!mEndpointId.HasValue() || mEndpointId.Value() == aEndpointId) && mClusterId == aClusterId;
    }

    /**
     * Check whether an AttributeAccessInterface is relevant for a particular
     * specific endpoint.  This is used to clean up overrides registered for an
     * endpoint that becomes disabled.
     */
    bool MatchesExactly(EndpointId aEndpointId) const { return mEndpointId.HasValue() && mEndpointId.Value() == aEndpointId; }

private:
    Optional<EndpointId> mEndpointId;
    ClusterId mClusterId;
    AttributeAccessInterface * mNext = nullptr;
};

} // namespace app
} // namespace chip
