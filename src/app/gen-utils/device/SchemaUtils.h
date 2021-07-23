/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *
 */

#pragma once

#include <core/CHIPCore.h>
#include <core/CHIPTLVDebug.hpp>
#include <support/CodeUtils.h>
#include <core/CHIPConfig.h>
#include <support/BitFlags.h>
#include <array>
#include <basic-types.h>
#include "SchemaTypes.h"
#include "core/CHIPTLVTypes.h"
#include "system/SystemPacketBuffer.h"
#include <support/PrivateHeap.h>
#include <Cluster.h>

namespace chip {
namespace app {

/*
 * @brief
 *
 * Thin wrapper around the PrivateHeap methods to provide a safe,
 * convenient way to allocate schema elements during TLV decode by
 * either providing an externally backed buffer, or an internally allocated
 * packetbuffer of maximal size.
 *
 */
class SchemaAllocator {
public:
  SchemaAllocator(void *heap, size_t size) : mBuf(heap) {
      PrivateHeapInit(mBuf, size);
  }

  SchemaAllocator() {
      mBackingPacketBuf = System::PacketBufferHandle::New(1024);
      mBuf = mBackingPacketBuf->Start();
      PrivateHeapInit(mBuf, mBackingPacketBuf->AvailableDataLength());
  }

  void *Alloc(size_t size) {
      return PrivateHeapAlloc(mBuf, size);
  }

private:
  System::PacketBufferHandle mBackingPacketBuf;
  void *mBuf;
};

CHIP_ERROR EncodeSchemaElement(chip::Span<const CompactFieldDescriptor> pDescriptor, void *buf, uint64_t tag, TLV::TLVWriter &writer, bool inArray = false);
CHIP_ERROR DecodeSchemaElement(chip::Span<const CompactFieldDescriptor> pDescriptor, void *buf, TLV::TLVReader &reader, SchemaAllocator *heap = nullptr, bool inArray = false);

template <typename GenType_t>
CHIP_ERROR EncodeSchemaElement(GenType_t &v, TLV::TLVWriter &writer, uint64_t tag)
{
    CHIP_ERROR err = EncodeSchemaElement({v.mDescriptor.FieldList.data(), v.mDescriptor.FieldList.size()}, &v, tag, writer);
    SuccessOrExit(err);

    err = writer.Finalize();

exit:
    return err;
}

template <typename GenType_t>
CHIP_ERROR DecodeSchemaElement(GenType_t &v, TLV::TLVReader &reader, SchemaAllocator *heap = nullptr)
{
    CHIP_ERROR err = DecodeSchemaElement({v.mDescriptor.FieldList.data(), v.mDescriptor.FieldList.size()}, &v, reader, heap);
    SuccessOrExit(err);

exit:
    return err;
}

/*
 * @brief
 *
 * A helper class for devices that use the device code generation format that makes it easier
 * to implement command dispatch using type-safe callbacks that achieve hands-free decoding of command requests
 * with private-heap assistance to manage complex structures and lists.
 */
class DeviceClusterServer : public ClusterServer  {
public:
    DeviceClusterServer(chip::ClusterId clusterId) : ClusterServer(clusterId) {}

    template <typename GenType_t>
    CHIP_ERROR AddResponse(InvokeResponder &responder, CommandParams &commandParams, GenType_t& resp) {
        return responder.AddResponse(commandParams, [&](chip::TLV::TLVWriter &writer, uint64_t tag) {
            return EncodeSchemaElement(resp, writer, tag);
        });
    }

    template <typename GenType_t, typename HandlerFunc>
    CHIP_ERROR DispatchCommand(CommandParams &commandParams, TLV::TLVReader &reader, InvokeResponder &invokeInteraction, HandlerFunc handlerFunc) {
        if (commandParams.CommandId == GenType_t::GetCommandId()) {
            GenType_t req;
            SchemaAllocator allocator;

            ReturnErrorOnFailure(DecodeSchemaElement(req, reader, &allocator));
            ReturnErrorOnFailure(handlerFunc(this, invokeInteraction, commandParams, &req, std::move(allocator)));
        }

        return CHIP_NO_ERROR;
    }
};

} // namespace app
} // namespace chip
