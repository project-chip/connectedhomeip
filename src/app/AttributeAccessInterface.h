/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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

#include <app/AttributeReportBuilder.h>
#include <app/AttributeValueDecoder.h>
#include <app/AttributeValueEncoder.h>
#include <lib/core/CHIPError.h>

/**
 * Callback class that clusters can implement in order to interpose custom
 * attribute-handling logic.  An AttributeAccessInterface instance is associated
 * with some specific cluster.  A single instance may be used for a specific
 * endpoint or for all endpoints.
 *
 * Instances of AttributeAccessInterface that are registered via
 * AttributeAccessInterfaceRegistry::Instance().Register will be consulted before taking the
 * normal attribute access codepath and can use that codepath as a fallback if desired.
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
     * @param [in] aPath indicates which exact data is being read.
     * @param [in] aEncoder the AttributeValueEncoder to use for encoding the
     *             data.
     *
     * The implementation can do one of three things:
     *
     * 1) Return a failure.  This is treated as a failed read and the error is
     *    returned to the client, by converting it to a StatusIB.
     * 2) Return success and attempt to encode data using aEncoder.  The data is
     *    returned to the client.
     * 3) Return success and not attempt to encode any data using aEncoder.  In
     *    this case, Ember attribute access will happen for the read. This may
     *    involve reading from the attribute store or external attribute
     *    callbacks.
     */
    virtual CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) = 0;

    /**
     * Callback for writing attributes.
     *
     * @param [in] aPath indicates which exact data is being written.
     * @param [in] aDecoder the AttributeValueDecoder to use for decoding the
     *             data.
     *
     * The implementation can do one of three things:
     *
     * 1) Return a failure.  This is treated as a failed write and the error is
     *    sent to the client, by converting it to a StatusIB.
     * 2) Return success and attempt to decode from aDecoder.  This is
     *    treated as a successful write.
     * 3) Return success and not attempt to decode from aDecoder.  In
     *    this case, Ember attribute access will happen for the write. This may
     *    involve writing to the attribute store or external attribute
     *    callbacks.
     */
    virtual CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) { return CHIP_NO_ERROR; }

    /**
     * Indicates the start of a series of list operations. This function will be called before the first Write operation of a series
     * of consequence attribute data of the same attribute.
     *
     * 1) This function will be called if the client tries to set a nullable list attribute to null.
     * 2) This function will only be called once for a series of consequent attribute data (regardless the kind of list operation)
     * of the same attribute.
     *
     * @param [in] aPath indicates the path of the modified list.
     */
    virtual void OnListWriteBegin(const ConcreteAttributePath & aPath) {}

    /**
     * Indicates the end of a series of list operations. This function will be called after the last Write operation of a series
     * of consequence attribute data of the same attribute.
     *
     * 1) This function will be called if the client tries to set a nullable list attribute to null.
     * 2) This function will only be called once for a series of consequent attribute data (regardless the kind of list operation)
     * of the same attribute.
     * 3) When aWriteWasSuccessful is true, the data written must be consistent or the list is untouched.
     *
     * @param [in] aPath indicates the path of the modified list
     * @param [in] aWriteWasSuccessful indicates whether the delivered list is complete.
     *
     */
    virtual void OnListWriteEnd(const ConcreteAttributePath & aPath, bool aWriteWasSuccessful) {}

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
    bool MatchesEndpoint(EndpointId aEndpointId) const { return mEndpointId.HasValue() && mEndpointId.Value() == aEndpointId; }

    /**
     * Check whether another AttributeAccessInterface wants to handle the same set of
     * attributes as we do.
     */
    bool Matches(const AttributeAccessInterface & aOther) const
    {
        return mClusterId == aOther.mClusterId &&
            (!mEndpointId.HasValue() || !aOther.mEndpointId.HasValue() || mEndpointId.Value() == aOther.mEndpointId.Value());
    }

protected:
    Optional<EndpointId> GetEndpointId() { return mEndpointId; }

private:
    Optional<EndpointId> mEndpointId;
    ClusterId mClusterId;
    AttributeAccessInterface * mNext = nullptr;
};

} // namespace app
} // namespace chip
