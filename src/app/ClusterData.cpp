/*
 *
 *    Copyright (c) 2016-2018 Nest Labs, Inc.
 *    Copyright (c) 2019-2020 Google, LLC.
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

/**
 *    @file
 *      This file forms the crux of the TDM layer (Cluster data management), providing
 *      various classes that manage and process data as it applies to Clusters and their
 *      associated schemas.
 *
 */

#include "support/CodeUtils.h"
#include "support/RandUtils.h"
#include <app/ClusterData.h>
#include <app/InteractionModelEngine.h>
#include <app/MessageDef/AttributeDataElement.h>
#include <app/reporting/ReportingEngine.h>

namespace chip {
namespace app {
uint64_t ClusterSchemaEngine::GetTag(AttributePathHandle aHandle) const
{
    return TLV::ContextTag(GetMap(aHandle)->mContextTag);
}

CHIP_ERROR ClusterSchemaEngine::RetrieveData(AttributePathHandle aAttributePathHandle, uint64_t aTagToWrite,
                                             chip::TLV::TLVWriter & aWriter, IGetDataDelegate * apGetDataDelegate) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (IsLeaf(aAttributePathHandle))
    {
        bool isPresent = true;

        err = apGetDataDelegate->GetData(aAttributePathHandle, aTagToWrite, aWriter, isPresent);
        SuccessOrExit(err);
        VerifyOrExit(isPresent, err = CHIP_ERROR_SCHEMA_MISMATCH);
    }
    else
    {
        TLV::TLVType type;
        err = aWriter.StartContainer(aTagToWrite, chip::TLV::kTLVType_Structure, type);
        SuccessOrExit(err);
        {
            AttributePathHandle childProperty;
            // Recursively iterate over all child nodes and call RetrieveData on them.
            for (childProperty = GetFirstChild(aAttributePathHandle); !IsNullAttributePathHandle(childProperty);
                 childProperty = GetNextChild(aAttributePathHandle, childProperty))
            {
                const AttributeInfo * childInfo = GetMap(childProperty);
                err = RetrieveData(childProperty, TLV::ContextTag(childInfo->mContextTag), aWriter, apGetDataDelegate);
                SuccessOrExit(err);
            }
        }

        err = aWriter.EndContainer(type);
    }

exit:
    return err;
}

CHIP_ERROR
ClusterSchemaEngine::StoreData(AttributePathHandle aHandle, TLV::TLVReader & aReader, ISetDataDelegate * apDelegate) const
{
    CHIP_ERROR err                   = CHIP_NO_ERROR;
    TLV::TLVType type                = chip::TLV::kTLVType_NotSpecified;
    AttributePathHandle curHandle    = aHandle;
    AttributePathHandle parentHandle = kNullAttributePathHandle;

    bool descending = true;

    if (IsLeaf(curHandle))
    {
        err = apDelegate->SetData(curHandle, aReader);
        SuccessOrExit(err);
    }
    else
    {
        // The crux of this loop is to iteratively parse out TLV and descend into the schema as necessary. The loop is bounded
        // by the return of the iterator handle (curHandle) back to the original start point (aHandle).
        //
        // The loop also has a notion of ascension and descension. Descension occurs when you go deeper into the schema tree while
        // ascension is returning back to a higher point in the tree.
        do
        {
            {
                if (!IsLeaf(curHandle))
                {
                    if (descending)
                    {
                        bool enterContainer = (aReader.GetType() != TLV::kTLVType_Null);
                        if (enterContainer)
                        {
                            err = aReader.EnterContainer(type);
                            SuccessOrExit(err);

                            parentHandle = curHandle;
                        }
                    }
                }
                else
                {
                    err = apDelegate->SetData(curHandle, aReader);
                    SuccessOrExit(err);

                    // Setting a leaf data can be interpreted as ascension since you are evaluating another node
                    // at the same level there-after by going back up to your parent and checking for more children.
                    descending = false;
                }
            }

            // Get the next element in this container.
            err = aReader.Next();
            VerifyOrExit((err == CHIP_NO_ERROR) || (err == CHIP_END_OF_TLV), );

            if (err == CHIP_END_OF_TLV)
            {
                // We've hit the end of the container - exit out and point our current handle to its parent.
                // In the process, restore the parentHandle as well.
                err = aReader.ExitContainer(type);
                SuccessOrExit(err);

                curHandle    = parentHandle;
                parentHandle = GetParent(curHandle);

                descending = false;
            }
            else
            {
                const uint64_t tag = aReader.GetTag();

                descending = true;

                curHandle = GetChildHandle(parentHandle, static_cast<uint8_t>(TLV::TagNumFromTag(tag)));

                if (IsNullAttributePathHandle(curHandle))
                {
                    err = CHIP_ERROR_TLV_TAG_NOT_FOUND;
                    break;
                }
            }
        } while (curHandle != aHandle);
    }

exit:
    return err;
}

AttributePathHandle ClusterSchemaEngine::GetFirstChild(AttributePathHandle aParentHandle) const
{
    return GetNextChild(aParentHandle, kRootAttributePathHandle);
}

bool ClusterSchemaEngine::IsParent(AttributePathHandle aChildHandle, AttributePathHandle aParentHandle) const
{
    bool retval = false;

    VerifyOrExit(aChildHandle != kNullAttributePathHandle && aParentHandle != kNullAttributePathHandle, );

    do
    {
        aChildHandle = GetParent(aChildHandle);

        if (aChildHandle == aParentHandle)
        {
            ExitNow(retval = true);
        }
    } while (aChildHandle != kNullAttributePathHandle);

exit:
    return retval;
}

AttributePathHandle ClusterSchemaEngine::GetParent(AttributePathHandle aHandle) const
{
    const AttributeInfo * handleMap = GetMap(aHandle);

    if (handleMap == nullptr)
    {
        return kNullAttributePathHandle;
    }
    return handleMap->mParentHandle;
}

AttributePathHandle ClusterSchemaEngine::GetNextChild(AttributePathHandle aParentHandle, AttributePathHandle aChildHandle) const
{
    uint32_t pathHandleIndex;

    // Starting from 1 root path handle after the child node that's been passed in, iterate till we find the next child belonging to
    // aParentId.
    for (pathHandleIndex = (uint32_t) aChildHandle - 1U; pathHandleIndex < mSchema.mNumAttributePathHandleEntries; pathHandleIndex++)
    {
        if (mSchema.mpAttributeHandleTable[pathHandleIndex].mParentHandle == aParentHandle)
        {
            break;
        }
    }

    if (pathHandleIndex == mSchema.mNumAttributePathHandleEntries)
    {
        return kNullAttributePathHandle;
    }
    else
    {
        return (static_cast<AttributePathHandle>(pathHandleIndex) + kHandleTableOffset);
    }
}

AttributePathHandle ClusterSchemaEngine::GetChildHandle(AttributePathHandle aParentHandle, uint8_t aContextTag) const
{
    for (AttributePathHandle childProperty = GetFirstChild(aParentHandle); !IsNullAttributePathHandle(childProperty);
         childProperty                     = GetNextChild(aParentHandle, childProperty))
    {
        if (mSchema.mpAttributeHandleTable[childProperty - kHandleTableOffset].mContextTag == aContextTag)
        {
            return childProperty;
        }
    }

    return kNullAttributePathHandle;
}

bool ClusterSchemaEngine::IsLeaf(AttributePathHandle aHandle) const
{
    // Root is by definition not a leaf. This also conveniently handles the cases where we have Clusters that
    // don't have any properties in them.
    if (aHandle == kRootAttributePathHandle)
    {
        return false;
    }
    else
    {
        for (unsigned int i = 0; i < mSchema.mNumAttributePathHandleEntries; i++)
        {
            if (mSchema.mpAttributeHandleTable[i].mParentHandle == aHandle)
            {
                return false;
            }
        }
        return true;
    }
}

const ClusterSchemaEngine::AttributeInfo * ClusterSchemaEngine::GetMap(AttributePathHandle aHandle) const
{
    if (aHandle < kHandleTableOffset || (aHandle >= (mSchema.mNumAttributePathHandleEntries + kHandleTableOffset)))
    {
        return nullptr;
    }

    return &mSchema.mpAttributeHandleTable[aHandle - kHandleTableOffset];
}

ClusterId ClusterSchemaEngine::GetClusterId(void) const
{
    return mSchema.mClusterId;
}

void ClusterDataSink::SetVersion(DataVersion aVersion)
{
    if (mHasValidVersion)
    {
        if (aVersion != mVersion)
        {
            ChipLogDetail(DataManagement, "Cluster version changed");
        }
    }
    else
    {
        ChipLogDetail(DataManagement, "Cluster version not changed");
    }
    mVersion         = aVersion;
    mHasValidVersion = true;
}

void ClusterDataSink::ClearVersion(void)
{
    ChipLogDetail(DataManagement, "Cluster %08x version: cleared", mpSchemaEngine->GetClusterId());
    mHasValidVersion = false;
}

ClusterDataSink::ClusterDataSink(const ClusterSchemaEngine * aEngine)
{
    mpSchemaEngine   = aEngine;
    mVersion         = 0;
    mHasValidVersion = 0;
}

CHIP_ERROR ClusterDataSink::StoreDataElement(AttributePathHandle aHandle, chip::TLV::TLVReader & aReader)
{
    AttributeDataElement::Parser parser;
    CHIP_ERROR err = CHIP_NO_ERROR;
    DataVersion versionInDataElement;

    VerifyOrExit(aHandle != kNullAttributePathHandle, err = CHIP_ERROR_INVALID_ARGUMENT);

    err = parser.Init(aReader);
    SuccessOrExit(err);

    err = parser.GetDataVersion(&versionInDataElement);
    SuccessOrExit(err);

    if (IsVersionNewer(versionInDataElement))
    {
        err = parser.GetData(&aReader);
        if (err == CHIP_END_OF_TLV)
        {
            err = CHIP_NO_ERROR;
        }
        else
        {
            SuccessOrExit(err);
            err = mpSchemaEngine->StoreData(aHandle, aReader, this);
        }
        // Only update the version number if the StoreData succeeded
        if (err == CHIP_NO_ERROR)
        {
            SetVersion(versionInDataElement);
        }
        else
        {
            // We need to clear this since we don't have a good version of data anymore.
            ClearVersion();
        }
    }
    else
    {
        ChipLogDetail(DataManagement, "<StoreData> no change");
    }

exit:
    return err;
}

CHIP_ERROR ClusterDataSink::SetData(AttributePathHandle aHandle, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (mpSchemaEngine->IsLeaf(aHandle))
    {
        err = SetLeafData(aHandle, aReader);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogDetail(DataManagement, "ahandle %u err: %d", aHandle, err);
        }
    }
    return err;
}

ClusterDataSource::ClusterDataSource(const ClusterSchemaEngine * aEngine)
{
    // Set the version to 0, indicating the lack of a valid version.
    SetVersion(0);

    mSetDirtyCalled = false;
    mpSchemaEngine  = aEngine;
}

uint64_t ClusterDataSource::GetVersion(void)
{
    // At the time of version retrieval, check to see if the version is still at the sentinel value of 0 (indicating 'no version')
    // set at construction. If it is, it means that the data source has not over-ridden the version to something other than 0,
    // indicating that it desires to use randomized data versions.
    while (mVersion == 0)
    {
        mVersion = GetRandU64();
    }

    return mVersion;
}

void ClusterDataSource::IncrementVersion()
{
    // By invoking GetVersion within here, we get the benefit of checking if the version is currently 0 and if so, randomize it.
    SetVersion(GetVersion() + 1);
}

CHIP_ERROR
ClusterDataSource::GetData(AttributePathHandle aHandle, uint64_t aTagToWrite, chip::TLV::TLVWriter & aWriter, bool & aIsPresent)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    aIsPresent = true;
    if (mpSchemaEngine->IsLeaf(aHandle))
    {
        err = GetLeafData(aHandle, aTagToWrite, aWriter);
    }

    return err;
}

CHIP_ERROR ClusterDataSource::ReadData(AttributePathHandle aHandle, uint64_t aTagToWrite, TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    err            = mpSchemaEngine->RetrieveData(aHandle, aTagToWrite, aWriter, this);
    return err;
}
} // namespace app
} // namespace chip
