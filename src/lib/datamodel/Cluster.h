/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 *    @file
 *      This file contains base class for CHIP Clusters
 *
 */

#ifndef CHIPBASECLUSTER_H_
#define CHIPBASECLUSTER_H_

#include <datamodel/Attribute.h>

namespace chip {
namespace DataModel {

/* TODO: To be converted to a template version or Kconfig later on */
static const uint8_t kMaxAttributesPerCluster = 10;
/**
 * @brief
 *   This class implements the cluster object that maintains its attributes. Typically specific
 *   cluster definitions will derive from this class and implement their functionality.
 */
class Cluster
{
public:
    uint16_t mClusterId;
    Attribute * mAttrs[kMaxAttributesPerCluster];

    Cluster(uint16_t clusterId) : mClusterId(clusterId), mAttrs() {}

    virtual ~Cluster()
    {
        for (int i = 0; i < kMaxAttributesPerCluster; i++)
        {
            if (mAttrs[i] != nullptr)
            {
                delete mAttrs[i];
                mAttrs[i] = nullptr;
            }
        }
    }

    /**
     * @brief
     *   Add attributes to this cluster
     *
     * @param attr Pointer to the attribute object being added
     */
    CHIP_ERROR AddAttribute(Attribute * attr)
    {
        for (int i = 0; i < kMaxAttributesPerCluster; i++)
        {
            if (mAttrs[i] == nullptr)
            {
                mAttrs[i] = attr;
                return CHIP_NO_ERROR;
            }
        }
        return CHIP_ERROR_INTERNAL;
    }

    /**
     * @brief
     *   Get pointer to the attribute object
     *
     * @param attrId the attribute identifer that we are looking for
     */
    Attribute * GetAttribute(uint16_t attrId)
    {
        for (int i = 0; i < kMaxAttributesPerCluster; i++)
        {
            if (mAttrs[i] && (mAttrs[i]->mAttrId == attrId))
            {
                return mAttrs[i];
            }
        }
        return nullptr;
    }

    /**
     * @brief
     *   Set an attribute to a value
     *
     * @param attrId the attribute identifer that should be set
     * @param value  the new value that the attribute should be updated with
     */
    virtual CHIP_ERROR Set(uint16_t attrId, const Value & value)
    {
        /* Just hand-off to update the value internally */
        auto attr = GetAttribute(attrId);
        if (attr)
        {
            return attr->Set(value);
        }
        return CHIP_ERROR_INTERNAL;
    }
};

} // namespace DataModel
} // namespace chip

#endif /* CHIPBASECLUSTER_H_ */
