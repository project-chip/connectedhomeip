/**
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
 *
 *    Copyright (c) 2020 Silicon Labs
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
/***************************************************************************//**
 * @file
 * @brief Security test code and stubs for unit tests of the Afv2 code.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "callback.h"

#include "app/framework/test/test-framework.h"
#include "plugin/serial/ember-printf-convert.h"

#include "app/framework/test/test-framework-security.h"

#include <stdlib.h>

//------------------------------------------------------------------------------
// Globals

EmberKeyStruct testFrameworkTrustCenterLinkKey = {
  (EMBER_KEY_HAS_OUTGOING_FRAME_COUNTER
   | EMBER_KEY_HAS_INCOMING_FRAME_COUNTER
   | EMBER_KEY_HAS_PARTNER_EUI64),
  EMBER_TRUST_CENTER_LINK_KEY,
  {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
  },  // key data
  0,  // outgoing frame counter
  0,  // incoming frame counter
  0,  // sequence number
  {
    1, 2, 3, 4, 5, 6, 7, 8,  // EUI64 (the Trust Center)
  }
};

EmberKeyStruct testFrameworkCurrentNwkKey = {
  EMBER_KEY_HAS_SEQUENCE_NUMBER
  | EMBER_KEY_HAS_OUTGOING_FRAME_COUNTER,
  EMBER_CURRENT_NETWORK_KEY,
  {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
  },
  0,  // out FC
  0,  // in FC
  20, // Sequence Number
  {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  }
};

EmberKeyStruct testFrameworkNextNwkKey = {
  EMBER_KEY_HAS_SEQUENCE_NUMBER
  | EMBER_KEY_HAS_OUTGOING_FRAME_COUNTER,
  EMBER_NEXT_NETWORK_KEY,
  {
    0x0d, 0x0e, 0x0a, 0x0d, 0x0b, 0x0e, 0x0e, 0x0f,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  },
  0,  // out FC
  0,  // in FC
  19, // Sequence Number (older than current)
  {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  }
};

EmberKeyData testFrameworkRandomlyGeneratedKey = {
  0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
  0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB
};

bool testFrameworkGenerateKeyFails = false;

EmberKeyStruct testFrameworkKeyTable[] = {
  // Index 0
  {
    EMBER_KEY_HAS_OUTGOING_FRAME_COUNTER
    | EMBER_KEY_HAS_INCOMING_FRAME_COUNTER
    | EMBER_KEY_HAS_PARTNER_EUI64
    | EMBER_KEY_IS_AUTHORIZED,
    EMBER_TRUST_CENTER_LINK_KEY,
    {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
    },
    0, // out FC
    0, // in  FC
    0, // sequence number (not applicable)
    {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
    }
  },

  // Index 1
  {
    EMBER_KEY_HAS_OUTGOING_FRAME_COUNTER
    | EMBER_KEY_HAS_INCOMING_FRAME_COUNTER
    | EMBER_KEY_HAS_PARTNER_EUI64
    | EMBER_KEY_IS_AUTHORIZED
    | EMBER_KEY_PARTNER_IS_SLEEPY,
    EMBER_TRUST_CENTER_LINK_KEY,
    {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02
    },
    0, // out FC
    0, // in  FC
    0, // sequence number (not applicable)
    {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02
    }
  },

  // Index 2
  {
    EMBER_KEY_HAS_OUTGOING_FRAME_COUNTER
    | EMBER_KEY_HAS_INCOMING_FRAME_COUNTER
    // This key is not authorized
    // | EMBER_KEY_IS_AUTHORIZED
    | EMBER_KEY_HAS_PARTNER_EUI64,
    EMBER_TRUST_CENTER_LINK_KEY,
    {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03
    },
    0, // out FC
    0, // in  FC
    0, // sequence number (not applicable)
    {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03
    }
  },

  // Index 3
  {
    EMBER_KEY_HAS_OUTGOING_FRAME_COUNTER
    | EMBER_KEY_HAS_INCOMING_FRAME_COUNTER
    | EMBER_KEY_HAS_PARTNER_EUI64
    | EMBER_KEY_IS_AUTHORIZED,
    EMBER_TRUST_CENTER_LINK_KEY,
    {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04
    },
    0, // out FC
    0, // in  FC
    0, // sequence number (not applicable)
    {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04
    }
  },

  // Terminator
  {
    0, 0, {}, 0xFFFFFFFFUL, 0xFFFFFFFFUL, 0, {}
  }
};

EmberKeyData testFrameworkZeroKey = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#define NODE_1 0x1234
#define NODE_2 0xBEEF
#define NODE_3 0xFADD
#define NODE_4 0xDEAD

EmberNodeId testFrameworkKeyTableNodeId[] = {
  NODE_1,
  NODE_2,
  NODE_3,
  NODE_4,
  EMBER_NULL_NODE_ID,
};

bool emberKeySwitchExpected = false;
bool emberKeySwitchReceived = false;

//------------------------------------------------------------------------------
// Functions

EmberStatus emberBroadcastNetworkKeySwitch()
{
  expect(emberKeySwitchExpected);
  debug("Sending NWK key switch to 0xFFFF\n");
  note(".");
  emberKeySwitchReceived = true;
  return EMBER_SUCCESS;
}

EmberStatus emberGetKey(EmberKeyType type,
                        EmberKeyStruct* keyStruct)
{
  EmberKeyStruct* source = NULL;

  switch (type) {
    case EMBER_CURRENT_NETWORK_KEY:
      source = &testFrameworkCurrentNwkKey;
      break;
    case EMBER_NEXT_NETWORK_KEY:
      source = &testFrameworkNextNwkKey;
      break;
    case EMBER_TRUST_CENTER_LINK_KEY:
      source = &testFrameworkTrustCenterLinkKey;
      break;
    default:
      note("emberGetKey() stub has no key data defined for type: %d\n", type);
      assert(source != NULL);
      break;
  }
  MEMMOVE(keyStruct,
          source,
          sizeof(EmberKeyStruct));
  return EMBER_SUCCESS;
}

EmberStatus emberGenerateRandomKey(EmberKeyData* keyAddress)
{
  MEMMOVE(keyAddress,
          &testFrameworkRandomlyGeneratedKey,
          sizeof(EmberKeyData));
  note(".");
  return (testFrameworkGenerateKeyFails
          ? EMBER_INSUFFICIENT_RANDOM_DATA
          : EMBER_SUCCESS);
}

uint8_t emberAfGetKeyTableSize(void)
{
  static uint8_t i = 0;

  if (i == 0) {
    while (testFrameworkKeyTable[i].outgoingFrameCounter != 0xFFFFFFFFUL) {
      if (testFrameworkKeyTableNodeId[i] == EMBER_NULL_NODE_ID) {
        assert("testFrameworkKeyTableNodeId[] is smaller than testFrameworkKeyTable[]" == 0);
      }
      i++;
    }
  }

  // If this isn't true, you need to update the #define
  assert(TEST_FRAMEWORK_KEY_TABLE_SIZE == i);

  return i;
}

EmberStatus emberGetKeyTableEntry(uint8_t index, EmberKeyStruct *result)
{
  if (emberAfGetKeyTableSize() <= index) {
    return EMBER_INDEX_OUT_OF_RANGE;
  }

  MEMMOVE(result,
          &(testFrameworkKeyTable[index]),
          sizeof(EmberKeyStruct));
  return EMBER_SUCCESS;
}

EmberStatus emSetKeyTableEntry(bool erase,
                               uint8_t index,
                               EmberEUI64 address,
                               EmberKeyData* keyData)
{
  if (emberAfGetKeyTableSize() <= index) {
    return EMBER_INDEX_OUT_OF_RANGE;
  }

  if (erase) {
    MEMSET(&(testFrameworkKeyTable[index]),
           0,
           sizeof(EmberKeyStruct));
    return EMBER_SUCCESS;
  }

  // We don't do any of the normal checking
  // that the stack provides.  We blindly allow
  // a key to be set to any value and with any
  // data.  For example we don't make sure
  // that the address isn't a duplicate of another
  // key that already exists.

  MEMMOVE(testFrameworkKeyTable[index].partnerEUI64,
          address,
          EUI64_SIZE);
  MEMMOVE(emberKeyContents(&(testFrameworkKeyTable[index].key)),
          emberKeyContents(keyData),
          EMBER_ENCRYPTION_KEY_SIZE);

  testFrameworkKeyTable[index].incomingFrameCounter = 0;
  testFrameworkKeyTable[index].outgoingFrameCounter = 0;

  // Easiest to assume if it is in the key table, then it is the TC
  // setting keys for devices in its network.
  testFrameworkKeyTable[index].type = EMBER_TRUST_CENTER_LINK_KEY;
  return EMBER_SUCCESS;
}
