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

#include <lib/core/CHIPBaseAttribute.h>

namespace chip {
namespace DataModel {

/* TODO: To be converted to a template version or Kconfig later on */
static const uint8_t kMaxAttributesPerCluster = 10;
class CHIPBaseCluster
{
public:
    uint16_t mClusterId;
    CHIPBaseAttribute * mAttrs[kMaxAttributesPerCluster];

    CHIPBaseCluster(uint16_t clusterId) : mClusterId(clusterId), mAttrs() {}

    virtual ~CHIPBaseCluster()
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

    int AddAttribute(CHIPBaseAttribute * attr)
    {
        for (int i = 0; i < kMaxAttributesPerCluster; i++)
        {
            if (mAttrs[i] == nullptr)
            {
                mAttrs[i] = attr;
                return SUCCESS;
            }
        }
        return FAIL;
    }

    CHIPBaseAttribute * GetAttribute(uint8_t attrId)
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

    virtual int Set(uint8_t attrId, const CHIPValue & value)
    {
        /* Just hand-off to update the value internally */
        auto attr = GetAttribute(attrId);
        if (attr)
        {
            return attr->Set(value);
        }
        return FAIL;
    }
};

} // namespace DataModel
} // namespace chip

#endif /* CHIPBASECLUSTER_H_ */
