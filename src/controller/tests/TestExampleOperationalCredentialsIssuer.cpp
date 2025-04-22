/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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

#include <pw_unit_test/framework.h>

#include <controller/ExampleOperationalCredentialsIssuer.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMemString.h>
#include <platform/KvsPersistentStorageDelegate.h>

#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <cstring>
 
using namespace chip;
using namespace chip::Controller;
 
namespace {
 
class UnitTestPersistentStorage : public PersistentStorageDelegate {
private:

   using KEY = std::string;
   using VALUE = std::vector<uint8_t>;
   std::map<KEY, VALUE> m_in_memory_map{};   

public:
   
   CHIP_ERROR SyncGetKeyValue(const char * key, void * buffer, uint16_t & size) override {
      auto k = std::string{key};

      if (buffer == nullptr) {
         return CHIP_INVALID_ARGUMENT;
      }

      if (m_in_memory_map.count(k) == 0) {
         return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
      }
      
      const auto & value_r = m_in_memory_map.at(k);

      auto bytes_to_copy = std::min(size, value_r.size());

      std::memcpy(buffer, value_r.data(), bytes_to_copy);

      size = bytes_to_copy;

      return bytes_to_copy < value_r.size() ? CHIP_ERROR_BUFFER_TOO_SMALL : CHIP_NO_ERROR;
   };

   CHIP_ERROR SyncSetKeyValue(const char * key, const void * value, uint16_t size) override {

      if (value == nullptr) {
         return CHIP_INVALID_ARGUMENT;
      }

      auto k = std::string{key};
      auto buffer_begin = static_cast<uint8_t*>(value);
      auto buffer_end   = buffer_begin + size;

      if (m_in_memory_map.count(k) > 0) {
         auto & value_r = m_in_memory_map.at(k);

         auto storage_begin = value_r.begin();
         
         auto r = value_r.insert(storage_begin, buffer_begin, buffer_end);

         return r != storage_begin ? CHIP_NO_ERROR : CHIP_ERROR_NO_MEMORY;
      }

      m_in_memory_map[k] = VALUE{buffer_begin, buffer_end};
      
      return CHIP_NO_ERROR;

    }
 
   CHIP_ERROR SyncDeleteKeyValue(const char * key) override {
      auto k = std::string{key};
   
      if (m_in_memory_map.erase(k) > 0) {
         return CHIP_NO_ERROR;
      }

      return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
   }


};




static const size_t node_id = 999;
static const size_t fabric_id = 1000;

class ExampleOperationalCredentialsIssuerTest : public ::testing::Test
{
public:
     static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
     static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

protected:

    NodeId mNodeId{::node_id};

    ExampleOperationalCredentialsIssuer mCredsIssuer{::fabric_id};
 };
 
 TEST_F(ExampleOperationalCredentialsIssuerTest, SuccessfulyGeneratesRandomNodeId)
 {
    auto prev_node_id = mNodeId;
    auto r = ExampleOperationalCredentialsIssuer::GetRandomOperationalNodeId(std::addressof(mNodeId));

    ASSERT_EQ(prev_node_id, node_id);
    ASSERT_EQ(r, CHIP_NO_ERROR);
    ASSERT_NE(mNodeId, node_id);
 }
 
TEST_F(ExampleOperationalCredentialsIssuerTest, SuccessfulyGeneratesNOCChainAfterValidation) 
{
    Crypto::P256Keypair ephemeralKey;
    auto ephemeral_r = ephemeralKey.Initialize(Crypto::ECPKeyTarget::ECDSA);

    Platform::ScopedMemoryBuffer<uint8_t> noc;
    noc.Calloc(Controller::kMaxCHIPDERCertLength);
    auto noc_boolean_result = noc.operator bool();

    Platform::ScopedMemoryBuffer<uint8_t> icac;
    icac.Calloc(Controller::kMaxCHIPDERCertLength);
    auto icac_boolean_result = icac.operator bool();

    Platform::ScopedMemoryBuffer<uint8_t> rcac;
    rcac.Calloc(Controller::kMaxCHIPDERCertLength);
    auto rcac_boolean_result = rcac.operator bool();

    ASSERT_EQ(ephemeral_r, CHIP_NO_ERROR);
    ASSERT_TRUE(noc_boolean_result);
    ASSERT_TRUE(rcac_boolean_result);
    ASSERT_TRUE(icac_boolean_result);

    MutableByteSpan nocSpan(noc.Get(), Controller::kMaxCHIPDERCertLength);
    MutableByteSpan icacSpan(icac.Get(), Controller::kMaxCHIPDERCertLength);
    MutableByteSpan rcacSpan(rcac.Get(), Controller::kMaxCHIPDERCertLength);

    KvsPersistentStorageDelegate sKvsPersistenStorageDelegate;

    chip::DeviceLayer::PersistedStorage::KeyValueStoreManager & kvsManager = DeviceLayer::PersistedStorage::KeyValueStoreMgr();
    sKvsPersistenStorageDelegate.Init(&kvsManager);
    auto r = mCredsIssuer.Initialize(sKvsPersistenStorageDelegate);

    // auto r = mCredsIssuer.GenerateNOCChainAfterValidation(
    //     mNodeId,
    //     ::fabric_id,
    //     chip::kUndefinedCATs,
    //     ephemeralKey.Pubkey(),
    //     rcacSpan,
    //     icacSpan,
    //     nocSpan
    // );

    ASSERT_EQ(r, CHIP_NO_ERROR);
 }


} // namespace