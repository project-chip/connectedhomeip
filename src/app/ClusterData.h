/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2018 Google LLC.
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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
 *      This file forms the crux of the Cluster Data Management layer, providing
 *      various classes that manage and process data as it applies to Clusters and their
 *      associated schemas.
 *
 */

#pragma once

#include <app/util/basic-types.h>
#include <core/CHIPCore.h>
#include <core/CHIPTLVDebug.hpp>
#include <lib/core/CHIPError.h>

namespace chip {
namespace app {
typedef uint32_t AttributePathHandle;
typedef uint32_t ClusterDataHandle;
/**
 * A AttributePathHandle is a unique 32-bit numerical hash of a IM path relative to the root of a Cluster instance. It has two parts
 * to it:
 *
 * Some characteristics:
 *  - Every Cluster has its own Attribute path handle space.
 *  - Every unique IM attribute sub-path path will have a similarly unique AttributePathHandle.
 *  - With this construct, consumer SDK logic never has to deal with IM paths directly. Rather, their interactions with IM are
 *    conducted exclusively through these handles.
 *
 */

// 0 indicates a 'NULL' handle
constexpr AttributePathHandle kNullAttributePathHandle = 0;
// 1 indicates a handle that points to the root of the Cluster instance.
constexpr AttributePathHandle kRootAttributePathHandle = 1;
/* Offset from the value of a path that provides the index into the table for that handle
 * It is 2 since the 0 and 1 are special handles.
 */
constexpr AttributePathHandle kHandleTableOffset = 2;

inline bool IsRootAttributePathHandle(AttributePathHandle aHandle)
{
    return (aHandle == kRootAttributePathHandle);
}

inline bool IsNullAttributePathHandle(AttributePathHandle aHandle)
{
    return (aHandle == kNullAttributePathHandle);
}

struct ClusterInfo
{
    void Init(void) { this->ClearDirty(); }
    bool IsDirty(void) { return mDirty; }
    void SetDirty(void) { mDirty = true; }
    void ClearDirty(void) { mDirty = false; }

    ClusterDataHandle mClusterDataHandle = 0;
    bool mDirty                          = false;
};

/**
 *  @class ClusterSchemaEngine
 *
 *  @brief The schema engine takes schema information associated with a particular Cluster and provides facilities to parse and
 *         translate that into a form usable by the IM machinery. This includes methods to interpret/query the schema itself and
 *         methods to help read/write out data to/from TLV given a handle.
 *
 *         The schema itself is stored in tabular form, sufficiently described to allow for generic parsing/composition of IM
 *         paths/data for any given Cluster. These tables are what will be the eventual output of 'code-gen'
 */
class ClusterSchemaEngine
{
public:
    /* Provides information about a particular path handle including its parent Attribute schema handle,
     * its context tag and its name.
     */
    struct AttributeInfo
    {
        AttributePathHandle mParentHandle;
        uint8_t mContextTag;
    };

    /**
     *  @brief
     *    The main schema structure that houses the schema information.
     */
    struct Schema
    {
        chip::ClusterId mClusterId;                   //< The ID of the Cluster.
        const AttributeInfo * mpAttributeHandleTable; //< A pointer to the schema handle table, which provides parent info and
                                                      //<context tags for each schema handle.
        uint32_t mNumAttributePathHandleEntries;      //< The number of schema handles in this Cluster.
        uint32_t mTreeDepth;                          //< The max depth of this schema.
    };

    /* While Clusters can have deep nested structures, consumer SDK is only expected to provide
     * getters/setters for 'leaf' nodes in the schema. If one can visualize a schema as a tree (a directed graph where you can have
     * at most one parent for any given node) where branches indicate the presence of a nested structure, then this analogy fits in
     * quite nicely.
     */
    class ISetDataDelegate
    {
    public:
        /**
         * Given a path handle to a leaf node and a TLV reader, set the leaf data in the callee.
         *
         * @retval #CHIP_NO_ERROR On success.
         * @retval other           Was unable to read out data from the reader.
         */
        virtual CHIP_ERROR SetLeafData(AttributePathHandle aLeafHandle, chip::TLV::TLVReader & aReader) = 0;

        /**
         * Given a path handle to a node, a TLV reader, and an indication
         * of whether a null type was received, set the data in the callee.
         * IM will only call this function for handles that are nullable,
         * optional, ephemeral, or leafs. If aHandle is a non-leaf node
         * and is nullified, IM will not call SetData for its children.
         *
         * @param[in] aHandle       The AttributePathHandle.
         *
         * @param[inout] aReader    The TLV reader to read from.
         *
         *
         * @retval #CHIP_NO_ERROR On success.
         * @retval other           Was unable to read out data from the reader.
         */
        virtual CHIP_ERROR SetData(AttributePathHandle aHandle, chip::TLV::TLVReader & aReader) = 0;

        virtual ~ISetDataDelegate() = default;
    };

    class IGetDataDelegate
    {
    public:
        /**
         * Given a path handle to a leaf node and a TLV writer, get the data from the callee.
         *
         * @retval #CHIP_NO_ERROR On success.
         * @retval other           Was unable to retrieve data and write it into the writer.
         */
        virtual CHIP_ERROR GetLeafData(AttributePathHandle aLeafHandle, uint64_t aTagToWrite, chip::TLV::TLVWriter & aWriter) = 0;

        /**
         * Given a path handle to a node, a TLV writer, and booleans indicating whether the
         * value is null or not present, get the data from the Cluster source that will build a
         * report. If the path handle is not a leaf node, IM will handle writing values to
         * the writer (like opening containers etc). If a non-leaf
         * node is null or not present, IM will not call GetData for its children.
         *
         * This function will only be called for handles that are leafs. The expectation is that any Clusters with handles that
         * have those options enabled will implement appropriate logic to populate
         * aIsPresent.
         *
         * @param[in] aHandle       The AttributePathHandle in question.
         *
         * @param[in] aTagToWrite   The tag to write for the aHandle.
         *
         * @param[inout] aWriter    The writer to write TLV elements to.
         *
         * @param[out] aIsPresent   Is aHandle present? If no and if aHandle
         *                          is not a leaf, IM will skip over the
         *                          path and its children.
         *
         *
         * @retval #CHIP_NO_ERROR On success.
         * @retval other           Was unable to retrieve data and write it into the writer.
         */
        virtual CHIP_ERROR GetData(AttributePathHandle aHandle, uint64_t aTagToWrite, chip::TLV::TLVWriter & aWriter,
                                   bool & aIsPresent) = 0;
        virtual ~IGetDataDelegate()                   = default;
    };

    /**
     * Given a path handle and a reader positioned on the corresponding attribute data element, process the data buffer pointed to
     * by the reader and store it into the sink by invoking the SetLeafData call whenever a leaf data item is encountered.
     *
     * @retval #CHIP_NO_ERROR On success.
     * @retval other           Encountered errors parsing/processing the data.
     */
    CHIP_ERROR StoreData(AttributePathHandle aHandle, chip::TLV::TLVReader & aReader, ISetDataDelegate * apSetDataDelegate) const;

    /**
     * Given a path handle and a writer position on the corresponding attribute data element, retrieve leaf data from the source and
     * write it into the buffer pointed to by the writer in a schema compliant manner.
     *
     * @retval #CHIP_NO_ERROR On success.
     * @retval other           Encountered errors writing out the data.
     */
    CHIP_ERROR RetrieveData(AttributePathHandle aHandle, uint64_t aTagToWrite, chip::TLV::TLVWriter & aWriter,
                            IGetDataDelegate * apGetDataDelegate) const;
    /**********
     *
     * Schema Query Functions
     *
     *********/

    /**
     * Returns true if the handle refers to a leaf node in the schema tree.
     *
     * @retval bool
     */
    bool IsLeaf(AttributePathHandle aAttributeHandle) const;

    /**
     * Returns a pointer to the AttributeInfo structure describing a particular path handle.
     *
     * @retval AttributeInfo*
     */
    const AttributeInfo * GetMap(AttributePathHandle aHandle) const;

    /**
     * Returns the tag associated with a path handle. If it's a dictionary element, this function returns the ProfileTag. Otherwise,
     * it returns context tags.
     *
     * @retval uint64_t
     */
    uint64_t GetTag(AttributePathHandle aHandle) const;

    /**
     * Returns the parent handle of a given child path handle. Dictionary keys in the handle are preserved in the case where the
     * parent handle is also a dictionary element.
     *
     * @retval AttributePathHandle   Handle of the parent.
     */
    AttributePathHandle GetParent(AttributePathHandle aHandle) const;

    /**
     * Returns the first child handle associated with a particular parent.
     *
     * @retval AttributePathHandle   Handle of the first child.
     */
    AttributePathHandle GetFirstChild(AttributePathHandle aParentHandle) const;

    /**
     * Given a handle to an existing child, returns the next child handle associated with a particular parent.
     *
     * @retval AttributePathHandle   Handle of the next child.
     */
    AttributePathHandle GetNextChild(AttributePathHandle aParentId, AttributePathHandle aChildHandle) const;

    /**
     * Returns the cluster id of the associated Cluster.
     *
     * @retval Cluster id
     */
    ClusterId GetClusterId(void) const;

    /**
     * Checks if a given handle is a child of another handle. This can be an in-direct parent.
     *
     * @retval bool
     */
    bool IsParent(AttributePathHandle aChildHandle, AttributePathHandle aParentHandle) const;

    AttributePathHandle GetChildHandle(AttributePathHandle aParentHandle, uint8_t aContextTag) const;

    const Schema mSchema;
};

/*
 * @class  ClusterDataSink
 *
 * @brief  Base abstract class that represents a particular instance of a Cluster on a specific external resource (client).
 * Application developers are expected to subclass this to make a concrete sink that ingests data received from servers.
 *
 *         It takes in a pointer to a schema that it then uses to help decipher incoming TLV data from a publisher and invoke the
 *         relevant data setter calls to pass the data up to subclasses.
 */
class ClusterDataSink : protected ClusterSchemaEngine::ISetDataDelegate
{
public:
    ClusterDataSink(const ClusterSchemaEngine * apEngine);
    virtual ~ClusterDataSink() {}
    const ClusterSchemaEngine * GetSchemaEngine(void) const { return mpSchemaEngine; }

    /**
     * Given a reader that points to a data element conformant to a schema bound to this object, this method processes that data and
     * invokes the relevant SetLeafData call below for all leaf items in the buffer.
     *
     *
     * @retval #CHIP_NO_ERROR On success.
     * @retval other           Encountered errors writing out the data.
     */
    CHIP_ERROR StoreDataElement(AttributePathHandle aHandle, TLV::TLVReader & aReader);

    /**
     * Retrieves the current version of the data that resides in this sink.
     */
    DataVersion GetVersion(void) const { return mVersion; }

    /**
     * Returns a boolean value that determines whether the version is valid.
     */
    bool IsVersionValid(void) const { return mHasValidVersion; }

    virtual bool IsVersionNewer(DataVersion & aVersion) { return aVersion != mVersion || false == mHasValidVersion; }

    /* Subclass can invoke this to clear out their version */
    void ClearVersion(void);

protected: // ISetDataDelegate
    virtual CHIP_ERROR SetLeafData(AttributePathHandle aLeafHandle, chip::TLV::TLVReader & aReader) __OVERRIDE = 0;

    /*
     * Defaults to calling SetLeafData if aHandle is a leaf.
     */
    virtual CHIP_ERROR SetData(AttributePathHandle aHandle, chip::TLV::TLVReader & aReader) __OVERRIDE;

    // Set current version of the data in this sink.
    void SetVersion(uint64_t version);
    const ClusterSchemaEngine * mpSchemaEngine;

private:
    bool mHasValidVersion;
    DataVersion mVersion = 0;
};

class ClusterDataSource : private ClusterSchemaEngine::IGetDataDelegate
{
public:
    ClusterDataSource(const ClusterSchemaEngine * aEngine);
    virtual ~ClusterDataSource() {}
    const ClusterSchemaEngine * GetSchemaEngine(void) const { return mpSchemaEngine; }

    uint64_t GetVersion(void);

    CHIP_ERROR ReadData(AttributePathHandle aHandle, uint64_t aTagToWrite, chip::TLV::TLVWriter & aWriter);

    /* Interactions with the underlying data has to always be done within a locked context. This applies to both the app logic
     * (e.g., a server when modifying its source data) as well as to the core IM logic (when trying to access that published
     * data).  This is required of both servers and clients.
     */
    CHIP_ERROR Lock(void);
    CHIP_ERROR Unlock(void);

    // Set current version of the data in this source.
    void SetVersion(uint64_t version) { mVersion = version; }

protected: // IGetDataDelegate
    /*
     * Defaults to calling GetLeafData if aHandle is a leaf.
     */
    virtual CHIP_ERROR GetData(AttributePathHandle aHandle, uint64_t aTagToWrite, chip::TLV::TLVWriter & aWriter,
                               bool & aIsPresent) __OVERRIDE;

    virtual CHIP_ERROR GetLeafData(AttributePathHandle aLeafHandle, uint64_t aTagToWrite,
                                   chip::TLV::TLVWriter & aWriter) __OVERRIDE = 0;

    // Increment current version of the data in this source.
    void IncrementVersion(void);

    const ClusterSchemaEngine * mpSchemaEngine;

private:
    DataVersion mVersion;
    // Tracks whether SetDirty was called within a Lock/Unlock 'session'
    bool mSetDirtyCalled = false;
    bool mRootIsDirty    = false;
};
}; // namespace app
}; // namespace chip
