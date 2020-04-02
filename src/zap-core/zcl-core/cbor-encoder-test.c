/***************************************************************************//**
 *
 *    <COPYRIGHT>
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
 *
 ******************************************************************************
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include "core/ember-stack.h"
#include "zclip-struct.h"
#include "cbor.h"

typedef struct {
  char *label;
  uint8_t zclipType;            // EMBER_ZCLIP_TYPE_...
  uint16_t cborPayloadLen;
  uint8_t *cborPayload;
  uint8_t valueLen;
  bool decodeValueResult;
  uint8_t expectedReadStatus;   // EmZclCoreCborValueReadStatus_t
} DecodeTestCase;

static uint8_t uns0[] = { 0x00 };
static uint8_t uns0Max[] = { 0x17 };
static uint8_t uns8Zero[] = { 0x18, 0x00 };
static uint8_t uns8Min[] = { 0x18, 0x18 };
static uint8_t uns8Max[] = { 0x18, 0xFF };
static uint8_t uns16Zero[] = { 0x19, 0x00, 0x00 };
static uint8_t uns16v8Max[] = { 0x19, 0x00, 0xFF };
static uint8_t uns16Min[] = { 0x19, 0x01, 0x00 };
static uint8_t uns16Max[] = { 0x19, 0xFF, 0xFF };
static uint8_t uns32Zero[] = { 0x1A, 0x00, 0x00, 0x00, 0x00 };
static uint8_t uns32v8Max[] = { 0x1A, 0x00, 0x00, 0x00, 0xFF };
static uint8_t uns32v16Min[] = { 0x1A, 0x00, 0x00, 0x01, 0x00 };
static uint8_t uns32v16Max[] = { 0x1A, 0x00, 0x00, 0xFF, 0xFF };
static uint8_t uns32Min[] = { 0x1A, 0x00, 0x01, 0x00, 0x00 };
static uint8_t uns32Max[] = { 0x1A, 0xFF, 0xFF, 0xFF, 0xFF };
//static uint8_t uns64Zero[] = { 0x1B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static uint8_t pos0[] = { 0x00 };
static uint8_t pos0Max[] = { 0x17 };
static uint8_t pos8Zero[] = { 0x18, 0x00 };
static uint8_t pos8Min[] = { 0x18, 0x18 };
static uint8_t pos8Max[] = { 0x18, 0x7F };
static uint8_t pos8TooBig[] = { 0x18, 0x80 };
static uint8_t pos16Zero[] = { 0x19, 0x00, 0x00 };
static uint8_t pos16v8Max[] = { 0x19, 0x00, 0x7F };
static uint8_t pos16Min[] = { 0x19, 0x00, 0x80 };
static uint8_t pos16Max[] = { 0x19, 0x7F, 0xFF };
static uint8_t pos16TooBig[] = { 0x19, 0x80, 0x00 };
static uint8_t pos32Zero[] = { 0x1A, 0x00, 0x00, 0x00, 0x00 };
static uint8_t pos32v8Max[] = { 0x1A, 0x00, 0x00, 0x00, 0x7F };
static uint8_t pos32v16Min[] = { 0x1A, 0x00, 0x00, 0x00, 0x80 };
static uint8_t pos32v16Max[] = { 0x1A, 0x00, 0x00, 0x7F, 0xFF };
static uint8_t pos32Min[] = { 0x1A, 0x00, 0x00, 0x80, 0x00 };
static uint8_t pos32Max[] = { 0x1A, 0x7F, 0xFF, 0xFF, 0xFF };
static uint8_t pos32TooBig[] = { 0x1A, 0x80, 0x00, 0x00, 0x00 };
//static uint8_t pos64Zero[] = { 0x1B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static uint8_t neg0[] = { 0x20 };
static uint8_t neg0Max[] = { 0x37 };
static uint8_t neg8Zero[] = { 0x38, 0x00 };
static uint8_t neg8Min[] = { 0x38, 0x18 };
static uint8_t neg8Max[] = { 0x38, 0x7F };
static uint8_t neg8TooBig[] = { 0x38, 0x80 };
static uint8_t neg16Zero[] = { 0x39, 0x00, 0x00 };
static uint8_t neg16v8Max[] = { 0x39, 0x00, 0x7F };
static uint8_t neg16Min[] = { 0x39, 0x00, 0x80 };
static uint8_t neg16Max[] = { 0x39, 0x7F, 0xFF };
static uint8_t neg16TooBig[] = { 0x39, 0x80, 0x00 };
static uint8_t neg32Zero[] = { 0x3A, 0x00, 0x00, 0x00, 0x00 };
static uint8_t neg32v8Max[] = { 0x3A, 0x00, 0x00, 0x00, 0x7F };
static uint8_t neg32v16Min[] = { 0x3A, 0x00, 0x00, 0x00, 0x80 };
static uint8_t neg32v16Max[] = { 0x3A, 0x00, 0x00, 0x7F, 0xFF };
static uint8_t neg32Min[] = { 0x3A, 0x00, 0x00, 0x80, 0x00 };
static uint8_t neg32Max[] = { 0x3A, 0x7F, 0xFF, 0xFF, 0xFF };
static uint8_t neg32TooBig[] = { 0x3A, 0x80, 0x00, 0x00, 0x00 };
//static uint8_t neg64Zero[] = { 0x3B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static uint8_t bin0[] = { 0x40 };
static uint8_t bin1[] = { 0x41, 0x55 };
static uint8_t bin2[] = { 0x42, 0x55, 0xAA };
static uint8_t bin254[] =
{
  0x58, 0xFE,
  0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF,
  0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF,
  0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF,
  0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF,
  0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF,
  0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF,
  0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF,
  0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF,
  0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF,
  0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF,
  0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF,
  0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF,
  0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF,
  0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF,
  0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF,
  0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE,
};
static uint8_t bin255[] =
{
  0x58, 0xFF,
  0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF,
  0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF,
  0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF,
  0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF,
  0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF,
  0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF,
  0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF,
  0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF,
  0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF,
  0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF,
  0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF,
  0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF,
  0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF,
  0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF,
  0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF,
  0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF,
};

static uint8_t txt0[] = { 0x60 };
static uint8_t txt1[] = { 0x61, 0x4A };
static uint8_t txt2[] = { 0x62, 0x4A, 0x50 };
static uint8_t txt254[] =
{
  0x78, 0xFE,
  0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50,
  0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50,
  0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50,
  0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50,
  0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50,
  0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50,
  0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50,
  0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50,
  0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50,
  0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50,
  0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50,
  0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50,
  0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50,
  0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50,
  0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50,
  0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A,
};
static uint8_t txt255[] =
{
  0x78, 0xFF,
  0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50,
  0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50,
  0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50,
  0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50,
  0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50,
  0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50,
  0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50,
  0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50,
  0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50,
  0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50,
  0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50,
  0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50,
  0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50,
  0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50,
  0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50,
  0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50, 0x4A, 0x50,
};

static uint8_t arr0[] = { 0x80 };

static uint8_t map0[] = { 0xA0 };

static uint8_t spcFalse[] = { 0xF4 };
static uint8_t spcTrue[] = { 0xF5 };
static uint8_t spcNil[] = { 0xF6 };

#define POS_TEST_CASE(label, zclipType, cborPayload, valueLen) \
  { (label), (zclipType), COUNTOF(cborPayload), (cborPayload), (valueLen), true, EM_ZCL_CORE_CBOR_VALUE_READ_SUCCESS }
#define NEG_TEST_CASE(label, zclipType, cborPayload, valueLen, expectedReadStatus) \
  { (label), (zclipType), COUNTOF(cborPayload), (cborPayload), (valueLen), false, (expectedReadStatus) }
#define NEG_TEST_CASE_PAYLOADLEN(label, zclipType, cborPayloadLen, cborPayload, valueLen, expectedReadStatus) \
  { (label), (zclipType), (cborPayloadLen), (cborPayload), (valueLen), false, (expectedReadStatus) }

static DecodeTestCase decodeValueTestCases[] = {
  //
  // Positive test cases.
  //
  // All of the following are true:
  //
  // - The CBOR data type is compatible with the specified EMBER_ZCLIP_TYPE.
  // - The CBOR payload is long enough to contain a complete data value.
  // - The value is decoded successfully.
  // - The decoded value fits within the specified value buffer.
  //

  // Boolean.
  POS_TEST_CASE("PTC-bool-val.01", EMBER_ZCLIP_TYPE_BOOLEAN, spcFalse, 1),
  POS_TEST_CASE("PTC-bool-val.02", EMBER_ZCLIP_TYPE_BOOLEAN, spcTrue, 1),

  // Unsigned integer.
  //
  // Values (regardless of CBOR encoding) that fit into uint8_t.
  POS_TEST_CASE("PTC-uns-08-fit.01", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, uns0, 1),
  POS_TEST_CASE("PTC-uns-08-fit.02", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, uns0Max, 1),
  POS_TEST_CASE("PTC-uns-08-fit.03", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, uns8Zero, 1),
  POS_TEST_CASE("PTC-uns-08-fit.04", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, uns8Min, 1),
  POS_TEST_CASE("PTC-uns-08-fit.05", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, uns8Max, 1),
  POS_TEST_CASE("PTC-uns-08-fit.06", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, uns16Zero, 1),
  POS_TEST_CASE("PTC-uns-08-fit.07", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, uns16v8Max, 1),
  POS_TEST_CASE("PTC-uns-08-fit.08", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, uns32Zero, 1),
  POS_TEST_CASE("PTC-uns-08-fit.09", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, uns32v8Max, 1),
  //
  // Values (regardless of CBOR encoding) that fit into uint16_t.
  POS_TEST_CASE("PTC-uns-16-fit.01", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, uns0, 2),
  POS_TEST_CASE("PTC-uns-16-fit.02", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, uns0Max, 2),
  POS_TEST_CASE("PTC-uns-16-fit.03", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, uns8Zero, 2),
  POS_TEST_CASE("PTC-uns-16-fit.04", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, uns8Min, 2),
  POS_TEST_CASE("PTC-uns-16-fit.05", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, uns8Max, 2),
  POS_TEST_CASE("PTC-uns-16-fit.06", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, uns16Zero, 2),
  POS_TEST_CASE("PTC-uns-16-fit.07", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, uns16v8Max, 1),
  POS_TEST_CASE("PTC-uns-16-fit.08", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, uns16Min, 2),
  POS_TEST_CASE("PTC-uns-16-fit.09", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, uns16Max, 2),
  POS_TEST_CASE("PTC-uns-16-fit.10", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, uns32Zero, 2),
  POS_TEST_CASE("PTC-uns-16-fit.11", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, uns32v16Min, 2),
  POS_TEST_CASE("PTC-uns-16-fit.12", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, uns32v16Max, 2),
  //
  //Values (regardless of CBOR encoding) that fit into uint32_t.
  POS_TEST_CASE("PTC-uns-32-fit.01", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, uns0, 4),
  POS_TEST_CASE("PTC-uns-32-fit.02", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, uns0Max, 4),
  POS_TEST_CASE("PTC-uns-32-fit.03", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, uns8Zero, 4),
  POS_TEST_CASE("PTC-uns-32-fit.04", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, uns8Min, 4),
  POS_TEST_CASE("PTC-uns-32-fit.05", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, uns8Max, 4),
  POS_TEST_CASE("PTC-uns-32-fit.06", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, uns16Zero, 4),
  POS_TEST_CASE("PTC-uns-32-fit.08", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, uns16Min, 4),
  POS_TEST_CASE("PTC-uns-32-fit.09", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, uns16Max, 4),
  POS_TEST_CASE("PTC-uns-32-fit.10", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, uns32Zero, 4),
  POS_TEST_CASE("PTC-uns-32-fit.11", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, uns32Min, 4),
  POS_TEST_CASE("PTC-uns-32-fit.12", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, uns32Max, 4),

  // Signed Integer, nonnegative values.
  //
  // PositiveOrZero values (regardless of CBOR encoding) that fit into int8_t.
  POS_TEST_CASE("PTC-poz-08-fit.01", EMBER_ZCLIP_TYPE_INTEGER, pos0, 1),
  POS_TEST_CASE("PTC-poz-08-fit.02", EMBER_ZCLIP_TYPE_INTEGER, pos0Max, 1),
  POS_TEST_CASE("PTC-poz-08-fit.03", EMBER_ZCLIP_TYPE_INTEGER, pos8Zero, 1),
  POS_TEST_CASE("PTC-poz-08-fit.04", EMBER_ZCLIP_TYPE_INTEGER, pos8Min, 1),
  POS_TEST_CASE("PTC-poz-08-fit.05", EMBER_ZCLIP_TYPE_INTEGER, pos8Max, 1),
  POS_TEST_CASE("PTC-poz-08-fit.06", EMBER_ZCLIP_TYPE_INTEGER, pos16Zero, 1),
  POS_TEST_CASE("PTC-poz-08-fit.07", EMBER_ZCLIP_TYPE_INTEGER, pos16v8Max, 1),
  POS_TEST_CASE("PTC-poz-08-fit.08", EMBER_ZCLIP_TYPE_INTEGER, pos32Zero, 1),
  POS_TEST_CASE("PTC-poz-08-fit.09", EMBER_ZCLIP_TYPE_INTEGER, pos32v8Max, 1),
  //
  // PositiveOrZero values (regardless of CBOR encoding) that fit into int16_t.
  POS_TEST_CASE("PTC-poz-16-fit.01", EMBER_ZCLIP_TYPE_INTEGER, pos0, 2),
  POS_TEST_CASE("PTC-poz-16-fit.02", EMBER_ZCLIP_TYPE_INTEGER, pos0Max, 2),
  POS_TEST_CASE("PTC-poz-16-fit.03", EMBER_ZCLIP_TYPE_INTEGER, pos8Zero, 2),
  POS_TEST_CASE("PTC-poz-16-fit.04", EMBER_ZCLIP_TYPE_INTEGER, pos8Min, 2),
  POS_TEST_CASE("PTC-poz-16-fit.05", EMBER_ZCLIP_TYPE_INTEGER, pos8Max, 2),
  POS_TEST_CASE("PTC-poz-16-fit.06", EMBER_ZCLIP_TYPE_INTEGER, pos16Zero, 2),
  POS_TEST_CASE("PTC-poz-16-fit.07", EMBER_ZCLIP_TYPE_INTEGER, pos16v8Max, 2),
  POS_TEST_CASE("PTC-poz-16-fit.08", EMBER_ZCLIP_TYPE_INTEGER, pos16Min, 2),
  POS_TEST_CASE("PTC-poz-16-fit.09", EMBER_ZCLIP_TYPE_INTEGER, pos16Max, 2),
  POS_TEST_CASE("PTC-poz-16-fit.10", EMBER_ZCLIP_TYPE_INTEGER, pos32Zero, 2),
  POS_TEST_CASE("PTC-poz-16-fit.11", EMBER_ZCLIP_TYPE_INTEGER, pos32v16Min, 2),
  POS_TEST_CASE("PTC-poz-16-fit.12", EMBER_ZCLIP_TYPE_INTEGER, pos32v16Max, 2),
  //
  // PositiveOrZero values (regardless of CBOR encoding) that fit into int32_t.
  POS_TEST_CASE("PTC-poz-32-fit.01", EMBER_ZCLIP_TYPE_INTEGER, pos0, 4),
  POS_TEST_CASE("PTC-poz-32-fit.02", EMBER_ZCLIP_TYPE_INTEGER, pos0Max, 4),
  POS_TEST_CASE("PTC-poz-32-fit.03", EMBER_ZCLIP_TYPE_INTEGER, pos8Zero, 4),
  POS_TEST_CASE("PTC-poz-32-fit.04", EMBER_ZCLIP_TYPE_INTEGER, pos8Min, 4),
  POS_TEST_CASE("PTC-poz-32-fit.05", EMBER_ZCLIP_TYPE_INTEGER, pos8Max, 4),
  POS_TEST_CASE("PTC-poz-32-fit.06", EMBER_ZCLIP_TYPE_INTEGER, pos16Zero, 4),
  POS_TEST_CASE("PTC-poz-32-fit.07", EMBER_ZCLIP_TYPE_INTEGER, pos16Min, 4),
  POS_TEST_CASE("PTC-poz-32-fit.08", EMBER_ZCLIP_TYPE_INTEGER, pos16Max, 4),
  POS_TEST_CASE("PTC-poz-32-fit.09", EMBER_ZCLIP_TYPE_INTEGER, pos32Zero, 4),
  POS_TEST_CASE("PTC-poz-32-fit.10", EMBER_ZCLIP_TYPE_INTEGER, pos32Min, 4),
  POS_TEST_CASE("PTC-poz-32-fit.11", EMBER_ZCLIP_TYPE_INTEGER, pos32Max, 4),

  // Signed Integer, negative values.
  //
  // Negative values (regardless of CBOR encoding) that fit into int8_t.
  POS_TEST_CASE("PTC-neg-08-fit.01", EMBER_ZCLIP_TYPE_INTEGER, neg0, 1),
  POS_TEST_CASE("PTC-neg-08-fit.02", EMBER_ZCLIP_TYPE_INTEGER, neg0Max, 1),
  POS_TEST_CASE("PTC-neg-08-fit.03", EMBER_ZCLIP_TYPE_INTEGER, neg8Zero, 1),
  POS_TEST_CASE("PTC-neg-08-fit.04", EMBER_ZCLIP_TYPE_INTEGER, neg8Min, 1),
  POS_TEST_CASE("PTC-neg-08-fit.05", EMBER_ZCLIP_TYPE_INTEGER, neg8Max, 1),
  POS_TEST_CASE("PTC-neg-08-fit.06", EMBER_ZCLIP_TYPE_INTEGER, neg16Zero, 1),
  POS_TEST_CASE("PTC-neg-08-fit.07", EMBER_ZCLIP_TYPE_INTEGER, neg16v8Max, 1),
  POS_TEST_CASE("PTC-neg-08-fit.08", EMBER_ZCLIP_TYPE_INTEGER, neg32Zero, 1),
  POS_TEST_CASE("PTC-neg-08-fit.09", EMBER_ZCLIP_TYPE_INTEGER, neg32v8Max, 1),
  //
  // Negative values (regardless of CBOR encoding) that fit into int16_t.
  POS_TEST_CASE("PTC-neg-16-fit.01", EMBER_ZCLIP_TYPE_INTEGER, neg0, 2),
  POS_TEST_CASE("PTC-neg-16-fit.02", EMBER_ZCLIP_TYPE_INTEGER, neg0Max, 2),
  POS_TEST_CASE("PTC-neg-16-fit.03", EMBER_ZCLIP_TYPE_INTEGER, neg8Zero, 2),
  POS_TEST_CASE("PTC-neg-16-fit.04", EMBER_ZCLIP_TYPE_INTEGER, neg8Min, 2),
  POS_TEST_CASE("PTC-neg-16-fit.05", EMBER_ZCLIP_TYPE_INTEGER, neg8Max, 2),
  POS_TEST_CASE("PTC-neg-16-fit.06", EMBER_ZCLIP_TYPE_INTEGER, neg16Zero, 2),
  POS_TEST_CASE("PTC-neg-16-fit.07", EMBER_ZCLIP_TYPE_INTEGER, neg16v8Max, 2),
  POS_TEST_CASE("PTC-neg-16-fit.08", EMBER_ZCLIP_TYPE_INTEGER, neg16Min, 2),
  POS_TEST_CASE("PTC-neg-16-fit.09", EMBER_ZCLIP_TYPE_INTEGER, neg16Max, 2),
  POS_TEST_CASE("PTC-neg-16-fit.10", EMBER_ZCLIP_TYPE_INTEGER, neg32Zero, 2),
  POS_TEST_CASE("PTC-neg-16-fit.11", EMBER_ZCLIP_TYPE_INTEGER, neg32v16Min, 2),
  POS_TEST_CASE("PTC-neg-16-fit.12", EMBER_ZCLIP_TYPE_INTEGER, neg32v16Max, 2),
  //
  // Negative values (regardless of CBOR encoding) that fit into int32_t.
  POS_TEST_CASE("PTC-neg-32-fit.01", EMBER_ZCLIP_TYPE_INTEGER, neg0, 4),
  POS_TEST_CASE("PTC-neg-32-fit.02", EMBER_ZCLIP_TYPE_INTEGER, neg0Max, 4),
  POS_TEST_CASE("PTC-neg-32-fit.03", EMBER_ZCLIP_TYPE_INTEGER, neg8Zero, 4),
  POS_TEST_CASE("PTC-neg-32-fit.04", EMBER_ZCLIP_TYPE_INTEGER, neg8Min, 4),
  POS_TEST_CASE("PTC-neg-32-fit.05", EMBER_ZCLIP_TYPE_INTEGER, neg8Max, 4),
  POS_TEST_CASE("PTC-neg-32-fit.06", EMBER_ZCLIP_TYPE_INTEGER, neg16Zero, 4),
  POS_TEST_CASE("PTC-neg-32-fit.07", EMBER_ZCLIP_TYPE_INTEGER, neg16Min, 4),
  POS_TEST_CASE("PTC-neg-32-fit.08", EMBER_ZCLIP_TYPE_INTEGER, neg16Max, 4),
  POS_TEST_CASE("PTC-neg-32-fit.09", EMBER_ZCLIP_TYPE_INTEGER, neg32Zero, 4),
  POS_TEST_CASE("PTC-neg-32-fit.10", EMBER_ZCLIP_TYPE_INTEGER, neg32Min, 4),
  POS_TEST_CASE("PTC-neg-32-fit.11", EMBER_ZCLIP_TYPE_INTEGER, neg32Max, 4),

  // Binary string, fixed length.
  POS_TEST_CASE("PTC-bfl-fit.01", EMBER_ZCLIP_TYPE_FIXED_LENGTH_BINARY, bin0, 0),
  POS_TEST_CASE("PTC-bfl-fit.02", EMBER_ZCLIP_TYPE_FIXED_LENGTH_BINARY, bin1, 1),
  POS_TEST_CASE("PTC-bfl-fit.03", EMBER_ZCLIP_TYPE_FIXED_LENGTH_BINARY, bin2, 2),

  // Binary string, variable length.
  POS_TEST_CASE("PTC-bin-fit.01", EMBER_ZCLIP_TYPE_BINARY, bin0, 2),
  POS_TEST_CASE("PTC-bin-fit.02", EMBER_ZCLIP_TYPE_BINARY, bin1, 2),
  POS_TEST_CASE("PTC-bin-fit.03", EMBER_ZCLIP_TYPE_BINARY, bin2, 2),
  POS_TEST_CASE("PTC-bin-fit.04", EMBER_ZCLIP_TYPE_BINARY, bin254, 255),

  // Binary string, length-prefixed.
  POS_TEST_CASE("PTC-blp-fit.01", EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_BINARY, bin0, 2),
  POS_TEST_CASE("PTC-blp-fit.02", EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_BINARY, bin1, 2),
  POS_TEST_CASE("PTC-blp-fit.03", EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_BINARY, bin2, 2),
  POS_TEST_CASE("PTC-blp-fit.04", EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_BINARY, bin254, 255),

  // Text string, null terminated. Note: valueLen includes capacity for null terminator.
  POS_TEST_CASE("PTC-tx0-fit.01", EMBER_ZCLIP_TYPE_MAX_LENGTH_STRING, txt0, 1),
  POS_TEST_CASE("PTC-tx0-fit.02", EMBER_ZCLIP_TYPE_MAX_LENGTH_STRING, txt1, 2),
  POS_TEST_CASE("PTC-tx0-fit.03", EMBER_ZCLIP_TYPE_MAX_LENGTH_STRING, txt2, 3),

  // Text string, variable length.
  POS_TEST_CASE("PTC-txt-fit.01", EMBER_ZCLIP_TYPE_UINT8_LENGTH_STRING, txt0, 0),
  POS_TEST_CASE("PTC-txt-fit.02", EMBER_ZCLIP_TYPE_UINT8_LENGTH_STRING, txt1, 1),
  POS_TEST_CASE("PTC-txt-fit.03", EMBER_ZCLIP_TYPE_UINT8_LENGTH_STRING, txt2, 2),
  POS_TEST_CASE("PTC-txt-fit.04", EMBER_ZCLIP_TYPE_UINT8_LENGTH_STRING, txt254, 255),

  // Text string, length-prefixed.
  POS_TEST_CASE("PTC-tlp-fit.01", EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_STRING, txt0, 1),
  POS_TEST_CASE("PTC-tlp-fit.02", EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_STRING, txt1, 2),
  POS_TEST_CASE("PTC-tlp-fit.03", EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_STRING, txt2, 3),
  POS_TEST_CASE("PTC-tlp-fit.03", EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_STRING, txt254, 255),

  //
  // Negative test cases.
  //
  // One of the following causes a decoding failure:
  //
  // - The payload is empty.
  // - The CBOR data type IS NOT compatible with the specified EMBER_ZCLIP_TYPE.
  // - The CBOR payload IS NOT long enough to contain the LENGTH of the data value.
  // - The CBOR payload IS NOT long enough to contain a complete data value.
  // - The value DOES NOT decode successfully.
  // - The decoded value DOES NOT fit within the specified length of the value buffer.
  //

  // Empty payload.
  //
  NEG_TEST_CASE_PAYLOADLEN("NTC-payload-empty.01", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, 0, uns8Zero, 4, EM_ZCL_CORE_CBOR_VALUE_READ_ERROR),

  // Boolean.
  //
  // Wrong CBOR type. For boolean, this is anything that isn't CBOR true/false.
  NEG_TEST_CASE("NTC-bool-type.01", EMBER_ZCLIP_TYPE_BOOLEAN, uns0, 1, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-bool-type.02", EMBER_ZCLIP_TYPE_BOOLEAN, neg0, 1, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-bool-type.03", EMBER_ZCLIP_TYPE_BOOLEAN, bin0, 1, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-bool-type.04", EMBER_ZCLIP_TYPE_BOOLEAN, txt0, 1, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-bool-type.05", EMBER_ZCLIP_TYPE_BOOLEAN, arr0, 1, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-bool-type.06", EMBER_ZCLIP_TYPE_BOOLEAN, map0, 1, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-bool-type.07", EMBER_ZCLIP_TYPE_BOOLEAN, spcNil, 1, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),

  // Unsigned integer.
  //
  // Wrong CBOR type.
  NEG_TEST_CASE("NTC-uns-type.01", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, neg0, 4, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-uns-type.02", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, bin0, 4, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-uns-type.03", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, txt0, 4, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-uns-type.04", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, arr0, 1, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-uns-type.05", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, map0, 1, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-uns-type.06", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, spcFalse, 4, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-uns-type.07", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, spcTrue, 4, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-uns-type.08", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, spcNil, 4, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  //
  // CBOR payload not long enough to contain a complete data value.
  NEG_TEST_CASE_PAYLOADLEN("NTC-uns-payload-uflow.01", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, 1, uns32Max, 4, EM_ZCL_CORE_CBOR_VALUE_READ_ERROR),
  NEG_TEST_CASE_PAYLOADLEN("NTC-uns-payload-uflow.01", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, COUNTOF(uns32Max) - 1, uns32Max, 4, EM_ZCL_CORE_CBOR_VALUE_READ_ERROR),
  //
  // Value too big to fit into valueLen-specified size.
  NEG_TEST_CASE("NTC-uns-val-toobig.01", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, uns16Min, 1, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE),
  NEG_TEST_CASE("NTC-uns-val-toobig.02", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, uns16Max, 1, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE),
  NEG_TEST_CASE("NTC-uns-val-toobig.03", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, uns32v16Min, 1, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE),
  NEG_TEST_CASE("NTC-uns-val-toobig.04", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, uns32v16Max, 1, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE),
  NEG_TEST_CASE("NTC-uns-val-toobig.05", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, uns32Min, 1, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE),
  NEG_TEST_CASE("NTC-uns-val-toobig.06", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, uns32Max, 1, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE),
  NEG_TEST_CASE("NTC-uns-val-toobig.07", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, uns32Min, 2, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE),
  NEG_TEST_CASE("NTC-uns-val-toobig.08", EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, uns32Max, 2, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE),

  // Signed Integer.
  //
  // Wrong CBOR type.
  NEG_TEST_CASE("NTC-int-type.01", EMBER_ZCLIP_TYPE_INTEGER, bin0, 4, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-int-type.02", EMBER_ZCLIP_TYPE_INTEGER, txt0, 4, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-int-type.03", EMBER_ZCLIP_TYPE_INTEGER, arr0, 1, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-int-type.04", EMBER_ZCLIP_TYPE_INTEGER, map0, 1, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-int-type.05", EMBER_ZCLIP_TYPE_INTEGER, spcFalse, 4, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-int-type.06", EMBER_ZCLIP_TYPE_INTEGER, spcTrue, 4, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-int-type.07", EMBER_ZCLIP_TYPE_INTEGER, spcNil, 4, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  //
  // CBOR payload not long enough to contain a complete data value.
  NEG_TEST_CASE_PAYLOADLEN("NTC-poz-payload-uflow.01", EMBER_ZCLIP_TYPE_INTEGER, 1, pos32Max, 4, EM_ZCL_CORE_CBOR_VALUE_READ_ERROR),
  NEG_TEST_CASE_PAYLOADLEN("NTC-poz-payload-uflow.02", EMBER_ZCLIP_TYPE_INTEGER, COUNTOF(pos32Max) - 1, pos32Max, 4, EM_ZCL_CORE_CBOR_VALUE_READ_ERROR),
  //
  // PositiveOrZero value too big to fit into valueLen-specified size.
  NEG_TEST_CASE("NTC-poz-fit.01", EMBER_ZCLIP_TYPE_INTEGER, pos8TooBig, 1, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE),
  NEG_TEST_CASE("NTC-poz-fit.02", EMBER_ZCLIP_TYPE_INTEGER, pos16Min, 1, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE),
  NEG_TEST_CASE("NTC-poz-fit.03", EMBER_ZCLIP_TYPE_INTEGER, pos16Max, 1, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE),
  NEG_TEST_CASE("NTC-poz-fit.04", EMBER_ZCLIP_TYPE_INTEGER, pos32v16Min, 1, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE),
  NEG_TEST_CASE("NTC-poz-fit.05", EMBER_ZCLIP_TYPE_INTEGER, pos32v16Max, 1, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE),
  NEG_TEST_CASE("NTC-poz-fit.06", EMBER_ZCLIP_TYPE_INTEGER, pos32Min, 1, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE),
  NEG_TEST_CASE("NTC-poz-fit.07", EMBER_ZCLIP_TYPE_INTEGER, pos32Max, 1, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE),
  NEG_TEST_CASE("NTC-poz-fit.08", EMBER_ZCLIP_TYPE_INTEGER, pos16TooBig, 2, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE),
  NEG_TEST_CASE("NTC-poz-fit.09", EMBER_ZCLIP_TYPE_INTEGER, pos32Min, 2, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE),
  NEG_TEST_CASE("NTC-poz-fit.10", EMBER_ZCLIP_TYPE_INTEGER, pos32Max, 2, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE),
  NEG_TEST_CASE("NTC-poz-fit.11", EMBER_ZCLIP_TYPE_INTEGER, pos32TooBig, 4, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE),
  //
  // CBOR payload not long enough to contain a complete data value.
  NEG_TEST_CASE_PAYLOADLEN("NTC-neg-payload-uflow.01", EMBER_ZCLIP_TYPE_INTEGER, 1, neg32Max, 4, EM_ZCL_CORE_CBOR_VALUE_READ_ERROR),
  NEG_TEST_CASE_PAYLOADLEN("NTC-neg-payload-uflow.02", EMBER_ZCLIP_TYPE_INTEGER, COUNTOF(neg32Max) - 1, neg32Max, 4, EM_ZCL_CORE_CBOR_VALUE_READ_ERROR),
  //
  // Negative value too big to fit into valueLen-specified size.
  NEG_TEST_CASE("NTC-neg-fit.01", EMBER_ZCLIP_TYPE_INTEGER, neg8TooBig, 1, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE),
  NEG_TEST_CASE("NTC-neg-fit.02", EMBER_ZCLIP_TYPE_INTEGER, neg16Min, 1, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE),
  NEG_TEST_CASE("NTC-neg-fit.03", EMBER_ZCLIP_TYPE_INTEGER, neg16Max, 1, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE),
  NEG_TEST_CASE("NTC-neg-fit.04", EMBER_ZCLIP_TYPE_INTEGER, neg32v16Min, 1, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE),
  NEG_TEST_CASE("NTC-neg-fit.05", EMBER_ZCLIP_TYPE_INTEGER, neg32v16Max, 1, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE),
  NEG_TEST_CASE("NTC-neg-fit.06", EMBER_ZCLIP_TYPE_INTEGER, neg32Min, 1, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE),
  NEG_TEST_CASE("NTC-neg-fit.07", EMBER_ZCLIP_TYPE_INTEGER, neg32Max, 1, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE),
  NEG_TEST_CASE("NTC-neg-fit.08", EMBER_ZCLIP_TYPE_INTEGER, neg16TooBig, 2, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE),
  NEG_TEST_CASE("NTC-neg-fit.09", EMBER_ZCLIP_TYPE_INTEGER, neg32Min, 2, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE),
  NEG_TEST_CASE("NTC-neg-fit.10", EMBER_ZCLIP_TYPE_INTEGER, neg32Max, 2, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE),
  NEG_TEST_CASE("NTC-neg-fit.11", EMBER_ZCLIP_TYPE_INTEGER, neg32TooBig, 4, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE),

  // Binary string, fixed length.
  //
  // Wrong CBOR type.
  NEG_TEST_CASE("NTC-bfl-type.01", EMBER_ZCLIP_TYPE_FIXED_LENGTH_BINARY, uns0, 4, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-bfl-type.02", EMBER_ZCLIP_TYPE_FIXED_LENGTH_BINARY, neg0, 4, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-bfl-type.03", EMBER_ZCLIP_TYPE_FIXED_LENGTH_BINARY, txt0, 4, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-bfl-type.04", EMBER_ZCLIP_TYPE_FIXED_LENGTH_BINARY, arr0, 1, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-bfl-type.05", EMBER_ZCLIP_TYPE_FIXED_LENGTH_BINARY, map0, 1, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-bfl-type.06", EMBER_ZCLIP_TYPE_FIXED_LENGTH_BINARY, spcFalse, 4, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-bfl-type.07", EMBER_ZCLIP_TYPE_FIXED_LENGTH_BINARY, spcTrue, 4, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-bfl-type.08", EMBER_ZCLIP_TYPE_FIXED_LENGTH_BINARY, spcNil, 4, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  //
  // CBOR payload not long enough to contain a complete data value.
  NEG_TEST_CASE_PAYLOADLEN("NTC-bfl-payload-uflow.01", EMBER_ZCLIP_TYPE_FIXED_LENGTH_BINARY, 1, bin2, 2, EM_ZCL_CORE_CBOR_VALUE_READ_ERROR),
  NEG_TEST_CASE_PAYLOADLEN("NTC-bfl-payload-uflow.02", EMBER_ZCLIP_TYPE_FIXED_LENGTH_BINARY, COUNTOF(bin2) - 1, bin2, 2, EM_ZCL_CORE_CBOR_VALUE_READ_ERROR),
  //
  // Value too small/large to fit into valueLen-specified fixed-size binary.
  NEG_TEST_CASE("NTC-bfl-size.01", EMBER_ZCLIP_TYPE_FIXED_LENGTH_BINARY, bin0, 1, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_SMALL),
  NEG_TEST_CASE("NTC-bfl-size.02", EMBER_ZCLIP_TYPE_FIXED_LENGTH_BINARY, bin1, 0, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE),
  NEG_TEST_CASE("NTC-bfl-size.03", EMBER_ZCLIP_TYPE_FIXED_LENGTH_BINARY, bin1, 2, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_SMALL),
  NEG_TEST_CASE("NTC-bfl-size.04", EMBER_ZCLIP_TYPE_FIXED_LENGTH_BINARY, bin2, 0, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE),
  NEG_TEST_CASE("NTC-bfl-size.05", EMBER_ZCLIP_TYPE_FIXED_LENGTH_BINARY, bin2, 1, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE),
  NEG_TEST_CASE("NTC-bfl-size.06", EMBER_ZCLIP_TYPE_FIXED_LENGTH_BINARY, bin2, 3, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_SMALL),

  // Binary string, variable length.
  //
  // Wrong CBOR type.
  NEG_TEST_CASE("NTC-bin-type.01", EMBER_ZCLIP_TYPE_BINARY, uns0, 4, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-bin-type.02", EMBER_ZCLIP_TYPE_BINARY, neg0, 4, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-bin-type.03", EMBER_ZCLIP_TYPE_BINARY, txt0, 4, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-bin-type.04", EMBER_ZCLIP_TYPE_BINARY, arr0, 1, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-bin-type.05", EMBER_ZCLIP_TYPE_BINARY, map0, 1, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-bin-type.06", EMBER_ZCLIP_TYPE_BINARY, spcFalse, 4, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-bin-type.07", EMBER_ZCLIP_TYPE_BINARY, spcTrue, 4, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-bin-type.08", EMBER_ZCLIP_TYPE_BINARY, spcNil, 4, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  //
  // CBOR payload not long enough to contain a complete data value.
  NEG_TEST_CASE_PAYLOADLEN("NTC-bin-payload-uflow.01", EMBER_ZCLIP_TYPE_BINARY, 1, bin2, 2, EM_ZCL_CORE_CBOR_VALUE_READ_ERROR),
  NEG_TEST_CASE_PAYLOADLEN("NTC-bin-payload-uflow.02", EMBER_ZCLIP_TYPE_BINARY, COUNTOF(bin2) - 1, bin2, 2, EM_ZCL_CORE_CBOR_VALUE_READ_ERROR),
  //
  // Value too large to fit into valueLen-specified capacity.
  NEG_TEST_CASE("NTC-bin-size.01", EMBER_ZCLIP_TYPE_BINARY, bin255, 255, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE),

  // Binary string, length-prefixed.
  //
  // Wrong CBOR type.
  NEG_TEST_CASE("NTC-blp-type.01", EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_BINARY, uns0, 4, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-blp-type.02", EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_BINARY, neg0, 4, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-blp-type.03", EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_BINARY, txt0, 4, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-blp-type.04", EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_BINARY, arr0, 1, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-blp-type.05", EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_BINARY, map0, 1, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-blp-type.06", EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_BINARY, spcFalse, 4, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-blp-type.07", EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_BINARY, spcTrue, 4, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-blp-type.08", EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_BINARY, spcNil, 4, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  //
  // CBOR payload not long enough to contain a complete data value.
  NEG_TEST_CASE_PAYLOADLEN("NTC-blp-payload-uflow.01", EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_BINARY, 1, bin2, 2, EM_ZCL_CORE_CBOR_VALUE_READ_ERROR),
  NEG_TEST_CASE_PAYLOADLEN("NTC-blp-payload-uflow.02", EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_BINARY, COUNTOF(bin2) - 1, bin2, 2, EM_ZCL_CORE_CBOR_VALUE_READ_ERROR),
  //
  // Value too large to fit into valueLen-specified capacity.
  NEG_TEST_CASE("NTC-blp-size.01", EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_BINARY, bin255, 255, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE),

  // Text string, null terminated. Note: valueLen includes capacity for null terminator.
  //
  // Wrong CBOR type.
  NEG_TEST_CASE("NTC-tx0-type.01", EMBER_ZCLIP_TYPE_MAX_LENGTH_STRING, uns0, 8, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-tx0-type.02", EMBER_ZCLIP_TYPE_MAX_LENGTH_STRING, neg0, 8, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-tx0-type.03", EMBER_ZCLIP_TYPE_MAX_LENGTH_STRING, bin0, 8, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-tx0-type.04", EMBER_ZCLIP_TYPE_MAX_LENGTH_STRING, arr0, 1, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-tx0-type.05", EMBER_ZCLIP_TYPE_MAX_LENGTH_STRING, map0, 1, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-tx0-type.06", EMBER_ZCLIP_TYPE_MAX_LENGTH_STRING, spcFalse, 4, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-tx0-type.07", EMBER_ZCLIP_TYPE_MAX_LENGTH_STRING, spcTrue, 4, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-tx0-type.08", EMBER_ZCLIP_TYPE_MAX_LENGTH_STRING, spcNil, 4, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  //
  // CBOR payload not long enough to contain a complete data value.
  NEG_TEST_CASE_PAYLOADLEN("NTC-tx0-payload-uflow.01", EMBER_ZCLIP_TYPE_MAX_LENGTH_STRING, 1, txt2, 3, EM_ZCL_CORE_CBOR_VALUE_READ_ERROR),
  NEG_TEST_CASE_PAYLOADLEN("NTC-tx0-payload-uflow.02", EMBER_ZCLIP_TYPE_MAX_LENGTH_STRING, COUNTOF(txt2) - 1, txt2, 3, EM_ZCL_CORE_CBOR_VALUE_READ_ERROR),
  //
  // Value too large to fit into valueLen-specified capacity.
  NEG_TEST_CASE("NTC-tx0-size.01", EMBER_ZCLIP_TYPE_MAX_LENGTH_STRING, txt1, 1, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE),
  NEG_TEST_CASE("NTC-tx0-size.02", EMBER_ZCLIP_TYPE_MAX_LENGTH_STRING, txt2, 2, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE),

  // Text string, variable-length.
  //
  // Wrong CBOR type.
  NEG_TEST_CASE("NTC-txt-type.01", EMBER_ZCLIP_TYPE_UINT8_LENGTH_STRING, uns0, 8, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-txt-type.02", EMBER_ZCLIP_TYPE_UINT8_LENGTH_STRING, neg0, 8, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-txt-type.03", EMBER_ZCLIP_TYPE_UINT8_LENGTH_STRING, bin0, 8, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-txt-type.04", EMBER_ZCLIP_TYPE_UINT8_LENGTH_STRING, arr0, 1, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-txt-type.05", EMBER_ZCLIP_TYPE_UINT8_LENGTH_STRING, map0, 1, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-txt-type.06", EMBER_ZCLIP_TYPE_UINT8_LENGTH_STRING, spcFalse, 4, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-txt-type.07", EMBER_ZCLIP_TYPE_UINT8_LENGTH_STRING, spcTrue, 4, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-txt-type.08", EMBER_ZCLIP_TYPE_UINT8_LENGTH_STRING, spcNil, 4, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  //
  // CBOR payload not long enough to contain a complete data value.
  NEG_TEST_CASE_PAYLOADLEN("NTC-txt-payload-uflow.01", EMBER_ZCLIP_TYPE_UINT8_LENGTH_STRING, 1, txt2, 3, EM_ZCL_CORE_CBOR_VALUE_READ_ERROR),
  NEG_TEST_CASE_PAYLOADLEN("NTC-txt-payload-uflow.02", EMBER_ZCLIP_TYPE_UINT8_LENGTH_STRING, COUNTOF(txt2) - 1, txt2, 3, EM_ZCL_CORE_CBOR_VALUE_READ_ERROR),
  //
  // Value too large to fit into valueLen-specified capacity.
  NEG_TEST_CASE("NTC-txt-size.01", EMBER_ZCLIP_TYPE_UINT8_LENGTH_STRING, txt255, 255, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE),

  // Text string, length-prefixed.
  //
  // Wrong CBOR type.
  NEG_TEST_CASE("NTC-tlp-type.01", EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_STRING, uns0, 8, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-tlp-type.02", EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_STRING, neg0, 8, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-tlp-type.03", EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_STRING, bin0, 8, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-tlp-type.04", EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_STRING, arr0, 1, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-tlp-type.05", EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_STRING, map0, 1, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-tlp-type.06", EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_STRING, spcFalse, 4, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-tlp-type.07", EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_STRING, spcTrue, 4, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  NEG_TEST_CASE("NTC-tlp-type.08", EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_STRING, spcNil, 4, EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE),
  //
  // CBOR payload not long enough to contain a complete data value.
  NEG_TEST_CASE_PAYLOADLEN("NTC-tlp-payload-uflow.01", EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_STRING, 1, txt2, 3, EM_ZCL_CORE_CBOR_VALUE_READ_ERROR),
  NEG_TEST_CASE_PAYLOADLEN("NTC-tlp-payload-uflow.02", EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_STRING, COUNTOF(txt2) - 1, txt2, 3, EM_ZCL_CORE_CBOR_VALUE_READ_ERROR),
  //
  // Value too large to fit into valueLen-specified capacity.
  NEG_TEST_CASE("NTC-tlp-size.01", EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_STRING, txt255, 255, EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE),
};

static bool debug = true; // true: do all cases / print all errors, instead of asserting on error

static void doDecodeValueTests()
{
  // For each test case, decode the EMBER_ZCLIP_TYPE... from the payload into the value buffer
  // of valueLen size, and confirm that the test result (boolean) and readStatus are as expected.
  uint8_t value[256];
  bool firstAfterSuccess = true; // just for output newline-ing
  int i;
  for (i = 0; i < COUNTOF(decodeValueTestCases); ++i) {
    DecodeTestCase *tcase = &decodeValueTestCases[i];
    CborState state;
    emCborDecodeStart(&state, tcase->cborPayload, tcase->cborPayloadLen);
    bool decodeResult = emCborDecodeValue(&state, tcase->zclipType, tcase->valueLen, value);
    if (decodeResult != tcase->decodeValueResult || state.readStatus != tcase->expectedReadStatus) {
      if (firstAfterSuccess) {
        fputc('\n', stderr);
        firstAfterSuccess = false;
      }
      fprintf(stderr, "%s: expected %s/%d, actual %s/%d\n",
              tcase->label,
              (tcase->decodeValueResult ? "true" : "false"),
              tcase->expectedReadStatus,
              (decodeResult ? "true" : "false"),
              state.readStatus);
      if (!debug) {
        assert(decodeResult == tcase->decodeValueResult);
        assert(state.readStatus == tcase->expectedReadStatus);
      }
    } else {
      fputc('.', stderr);
      firstAfterSuccess = true;
    }
  }
}

//----------------------------------------------------------------
// Test code.  These are from AFV6 tests and are not exhaustive.  Among
// other things, they do not test indefinite length arrays and maps.

static uint8_t cbor0[] = {
  0xA3, 0x61, 0x6E, 0x09, 0x61, 0x78, 0x18, 0x6E,
  0x61, 0x61, 0xA1, 0x00, 0xA1, 0x61, 0x72, 0x18,
  0x6F
};
static uint8_t result0[] = "{\"n\":9,\"x\":110,\"a\":{0:{\"r\":111}}}";

static uint8_t cbor1[] = {
  0xA2, 0x61, 0x72, 0x00, 0x61, 0x75, 0x73, 0x63,
  0x6F, 0x61, 0x70, 0x3A, 0x2F, 0x2F, 0x5B, 0x3A,
  0x3A, 0x5D, 0x2F, 0x7A, 0x63, 0x6C, 0x2F, 0x65,
  0x2F, 0x31
};
static uint8_t result1[] = "{\"r\":0,\"u\":\"coap://[::]/zcl/e/1\"}";

static uint8_t cbor2[] = {
  0xA4, 0x61, 0x6E, 0x0A, 0x61, 0x78, 0x18, 0x3C,
  0x61, 0x61, 0xA1, 0x00, 0xA0, 0x61, 0x75, 0x73,
  0x63, 0x6F, 0x61, 0x70, 0x3A, 0x2F, 0x2F, 0x5B,
  0x3A, 0x3A, 0x5D, 0x2F, 0x7A, 0x63, 0x6C, 0x2F,
  0x65, 0x2F, 0x31
};
static uint8_t result2[] =
  "{\"n\":10,\"x\":60,\"a\":{0:{}},\"u\":\"coap://[::]/zcl/e/1\"}";

static void printTest(void)
{
  uint8_t buf[1024];
  typedef struct {
    uint8_t *cbor;
    uint8_t cborLength;
    uint8_t *result;
  } test_t;
  test_t tests[] = { { cbor0, sizeof(cbor0), result0 },
                     { cbor1, sizeof(cbor1), result1 },
                     { cbor2, sizeof(cbor2), result2 } };
  for (uint8_t i = 0; i < sizeof(tests) / sizeof(test_t); i++) {
    if (emCborToString(tests[i].cbor, tests[i].cborLength, buf, sizeof(buf))
        != strlen(tests[i].result)) {
      fprintf(stderr, "CBOR Test %d: Failed parsing %s\n", i, tests[i].result);
      assert(!debug);
    } else if (memcmp(buf, tests[i].result, strlen(tests[i].result)) != 0) {
      fprintf(stderr, "CBOR Test %d: payload (%s) does not match expected value (%s)\n", i, buf, tests[i].result);
      assert(!debug);
    } else {
      fputc('.', stderr);
    }
  }
}

int main(int argc, char *argv[])
{
  fprintf(stderr, "[%s ", argv[0]);

  doDecodeValueTests();

  printTest();

  fprintf(stderr, " done]\n");
  return 0;
}
