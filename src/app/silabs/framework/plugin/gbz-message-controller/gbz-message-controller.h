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
 * @brief APIs and defines for the GBCS Message Controller plugin.
 *******************************************************************************
   ******************************************************************************/

#ifndef SILABS_GBZ_MESSAGE_CONTROLLER
#define SILABS_GBZ_MESSAGE_CONTROLLER

typedef enum {
  GCS06_MESSAGE_CODE  = 0x0070, // CHF
  GCS11_MESSAGE_CODE  = 0x0073, // CHF
  CS11_MESSAGE_CODE             = 0x0015,
  GCS01a_MESSAGE_CODE           = 0x006B,
  GCS01b_MESSAGE_CODE           = 0x00A3,
  GCS05_MESSAGE_CODE            = 0x006F,
  GCS07_MESSAGE_CODE            = 0x0071,
  GCS09_MESSAGE_CODE            = 0x0072,
  GCS13a_MESSAGE_CODE           = 0x0074,
  GCS13b_MESSAGE_CODE           = 0x00B8,
  GCS13c_MESSAGE_CODE           = 0x00B6,
  GCS14_MESSAGE_CODE            = 0x0075,
  GCS17_MESSAGE_CODE            = 0x0078,
  GCS21d_MESSAGE_CODE           = 0x009D,
  GCS21e_MESSAGE_CODE           = 0x009E,
  GCS21j_MESSAGE_CODE           = 0x00BF,
  GCS23_MESSAGE_CODE            = 0x007C,
  GCS25_MESSAGE_CODE            = 0x007E,
  GCS33_MESSAGE_CODE            = 0x0082,
  GCS38_MESSAGE_CODE            = 0x0084,
  GCS44_MESSAGE_CODE            = 0x0088,
  GCS46_MESSAGE_CODE            = 0x0089,
  GCS60_MESSAGE_CODE            = 0x008D,
  CS10a_MESSAGE_CODE            = 0x0014,
  CS10b_MESSAGE_CODE            = 0x00A1,
  GCS61_MESSAGE_CODE            = 0x00A0,
  GCS16a_MESSAGE_CODE           = 0x0077,
  GCS16b_MESSAGE_CODE           = 0x0096,
  GCS15b_MESSAGE_CODE           = 0x00C3,
  GCS15c_MESSAGE_CODE           = 0x0076,
  GCS15d_MESSAGE_CODE           = 0x00C4,
  GCS15e_MESSAGE_CODE           = 0x00C5,
  GCS21f_MESSAGE_CODE           = 0x009F,
  GCS21b_MESSAGE_CODE           = 0x00B5,
  GCS53_MESSAGE_CODE            = 0x008B,
  TEST_ENCRYPTED_MESSAGE_CODE   = 0xFFFE,
  TEST_MESSAGE_CODE             = 0xFFFF,
} GBCSUseCaseMessageCode;
// debug prints
#define emberAfPluginGbzMessageControllerPrint(...)    emberAfAppPrint(__VA_ARGS__)
#define emberAfPluginGbzMessageControllerPrintln(...)  emberAfAppPrintln(__VA_ARGS__)
#define emberAfPluginGbzMessageControllerDebugExec(x)  emberAfAppDebugExec(x)
#define emberAfPluginGbzMessageControllerPrintBuffer(buffer, len, withSpace) emberAfAppPrintBuffer(buffer, len, withSpace)

// offsets
#define GAS_PROXY_FUNCTION_GBZ_COMPONENT_EXT_HEADER_CONTROL_FIELD_OFFSET  (0)
#define GAS_PROXY_FUNCTION_GBZ_COMPONENT_EXT_HEADER_CLUSTER_ID_OFFSET     (2)
#define GAS_PROXY_FUNCTION_GBZ_COMPONENT_EXT_HEADER_GBZ_CMD_LENGTH_OFFSET (3)

#define GAS_PROXY_FUNCTION_GBZ_MESSAGE_COMMAND_HEADER_LENGTH              (3)
#define GAS_PROXY_FUNCTION_GBZ_MESSAGE_RESPONSE_HEADER_LENGTH             (3)
#define GAS_PROXY_FUNCTION_GBZ_MESSAGE_ALERT_HEADER_LENGTH                (9)
#define GAS_PROXY_FUNCTION_GBZ_COMPONENT_EXT_HEADER_FIELDS_LENGTH         (5)
#define GAS_PROXY_FUNCTION_GBZ_COMPONENT_ZCL_HEADER_LENGTH                (3)
#define GAS_PROXY_FUNCTION_GBZ_COMPONENT_FROM_DATE_TIME_LENGTH            (4)
#define GAS_PROXY_FUNCTION_GBZ_COMPONENT_ENCRYPTION_HEADER_FIELDS_LENGTH  (2)
#define GAS_PROXY_FUNCTION_GBZ_COMPONENT_ENCRYPTION_CIPHERED_INFO_LENGTH  (2)
#define GAS_PROXY_FUNCTION_GBZ_COMPONENT_EXT_HEADER_FIELDS_LENGTH         (5)
#define GAS_PROXY_FUNCTION_GBZ_COMPONENT_ENCRYPTION_HEADER_FIELDS_LENGTH  (2)
#define GAS_PROXY_FUNCTION_GBZ_COMPONENT_FROM_DATE_TIME_LENGTH            (4)
#define GAS_PROXY_FUNCTION_GBZ_COMPONENT_ENCRYPTION_CIPHERED_INFO_LENGTH  (2)
#define GAS_PROXY_FUNCTION_GBZ_COMPONENT_LAST_MSG_MASK       (0x01)
#define GAS_PROXY_FUNCTION_GBZ_COMPONENT_ENCRYPTED_MSG_MASK  (0x02)
#define GAS_PROXY_FUNCTION_GBZ_COMPONENT_FROM_DATE_TIME_MASK (0x10)

// Forward declaration
EmberAfStatus emberAfClusterSpecificCommandParse(EmberAfClusterCommand *cmd);

typedef enum {
  EMBER_AF_GBZ_NOT_LAST_UNENCRYPTED_MESSAGE = 0x00,
  EMBER_AF_GBZ_LAST_UNENCRYPTED_MESSAGE = 0x01,
  EMBER_AF_GBZ_NOT_LAST_ENCRYPTED_MESSAGE = 0x02,
  EMBER_AF_GBZ_LAST_ENCRYPTED_MESSAGE = 0x03,
} EmberAfGbzExtendedHeaderControlField;

typedef enum {
  EMBER_AF_GBZ_MESSAGE_COMMAND,
  EMBER_AF_GBZ_MESSAGE_RESPONSE,
  EMBER_AF_GBZ_MESSAGE_ALERT
} EmberAfGbzMessageType;

typedef struct {
  EmberAfClusterId  clusterId;
  uint8_t frameControl;
  uint8_t transactionSequenceNumber;
  uint8_t commandId;
  uint8_t * payload;
  uint16_t payloadLength;
  uint8_t direction;
  bool clusterSpecific;
  bool mfgSpecific;

  uint32_t fromDateTime;
  bool hasFromDateTime; // Zigbee UTC Time.
  bool encryption;
} EmberAfGbzZclCommand;

typedef struct {
  bool freeRequired;
  uint8_t * command;
  EmberAfGbzMessageType type;
  uint16_t alertCode;
  uint32_t alertTimestamp;
  uint16_t profileId;
  uint8_t nextComponentZclSequenceNumber;
  uint8_t componentsSize;
  uint8_t componentsParsed;
  uint16_t parseIndex; // index to the next byte for parsing.
  uint16_t length;
  uint16_t messageCode; // "Message Code" for the corresponding Non TOM Command.
} EmberAfGbzMessageParserState;

typedef struct {
  uint8_t * payload;
  uint8_t payloadLength;
} EmAfGbzPayloadHeader;

/*
 * @brief a link list that keeps track of raw data that represents appended
 *        GBZ Use Case Specific components.
 */
struct EmAfGbzUseCaseSpecificComponent{
  uint8_t * payload;
  uint16_t payloadLength;
  struct EmAfGbzUseCaseSpecificComponent * next;
};

typedef struct EmAfGbzUseCaseSpecificComponent EmAfGbzUseCaseSpecificComponent;

typedef struct {
  uint8_t * payload;
  uint16_t payloadLength;
  bool freeRequired;
} EmberAfGbzMessageCreatorResult;

typedef struct {
  bool allocateMemoryForResponses;

  // used when allocateMemoryForResponses is false
  uint8_t * command;
  uint16_t commandIndex; // index to the next byte for appending.
  uint16_t commandLength;

  // used when allocateMemoryForResponses is true
  EmAfGbzUseCaseSpecificComponent * responses;
  EmAfGbzUseCaseSpecificComponent * lastResponse;
  EmAfGbzPayloadHeader * header;

  // otherwise.
  uint8_t nextEncryptedComponentZclSequenceNumber;
  uint8_t nextComponentZclSequenceNumber;
  uint8_t nextAdditionalHeaderFrameCounter;
  uint8_t * componentsCount;
  uint8_t * lastExtHeaderControlField;
  uint16_t messageCode;

  // assembled result
  EmberAfGbzMessageCreatorResult result;
} EmberAfGbzMessageCreatorState;

/*
 * @brief Assembles appended ZCL responses into 1 big chunk of memory.
 *
 */
EmberAfGbzMessageCreatorResult * emberAfPluginGbzMessageControllerCreatorAssemble(EmberAfGbzMessageCreatorState * state);

/*
 * @brief Gets the default response byte from the GBZ ZCL Command.
 *
 * @param ZCL status.
 */
EmberAfStatus emberAfPluginGbzMessageControllerGetZclDefaultResponse(EmberAfGbzZclCommand * cmd);

/*
 * @brief Checks the GBZ parser structure for any non-parsed commands.
 *
 * @param state A structure containing the bookkeeping information of parsing GBZ messages.
 */
bool emberAfPluginGbzMessageControllerHasNextCommand(EmberAfGbzMessageParserState * state);

/*
 * @brief Cleans any resources allocated during the parsing of GBZ message.
 *
 * @param state A pre-allocated struct that will be updated to hold
 *   bookkeeping information of parsing GBZ messages.
 */
void emberAfPluginGbzMessageControllerParserCleanup(EmberAfGbzMessageParserState * state);

/*
 * @brief Initializes proper parsing states for decoding of GBZ messages.
 *
 * The GBZ message payload and payload length will be passed in as
 * arguments. Iterator functions, emberAfPluginGbzMessageControllerHasNextCommand() and
 * emberAfPluginGbzMessageControllerNextCommand() will be used to iterate through
 * each of the embedded ZCL functions.
 *
 * @param state            A pre-allocated structure that will be updated to hold
 *                         bookkeeping information for parsing GBZ messages.
 * @param gbzCommand       A pointer to GBZ messages.
 * @param gbzCommandLength Length of GBZ messages.
 * @param copyGbzCommand   A flag to indicate if the parser should be storing the GBZ
 *                         command locally for parsing.
 * @param messageCode      "Message Code" for the corresponding Non TOM command.
 *
 */
bool emberAfPluginGbzMessageControllerParserInit(EmberAfGbzMessageParserState * state,
                                                 EmberAfGbzMessageType type,
                                                 uint8_t * gbzCommand,
                                                 uint16_t gbzCommandLength,
                                                 bool copyGbzCommand,
                                                 uint16_t messageCode);

/*
 * @brief Appends a ZCL command to a given GBZ creator structure.
 *
 *
 * @param state A pre-allocated structure that will be updated to hold
 *   bookkeeping information of creating GBZ messages.
 * @param zclCmd A structure containing information for new ZCL command.
 *
 *  @return 0 - if the appending operation did not succeed
 *          else - number of appended bytes.
 */
uint16_t emberAfPluginGbzMessageControllerAppendCommand(EmberAfGbzMessageCreatorState * state, EmberAfGbzZclCommand * zclCmd);

/*
 * @brief Initializes proper states for construction of GBZ messages.
 *
 * Depending on the value of the argument (gbzCommand), the creator will
 * behave differently. If a NULL value is passed, the creator assumes the
 * user wants the API to allocate memory to store the appended responses.
 * Otherwise, the creator will use the provided buffer as the destination for
 * storing responses.
 *
 * Below is a general flow for the creation of ZCL messages into a GBZ
 * message.
 *
 *  1. emberAfPluginGbzMessageControllerCreatorInit() - create
 *  2. emberAfPluginGbzMessageControllerAppendCommand() - append
 *  3. emberAfPluginGbzMessageControllerCreatorAssemble() - assemble result.
 *  4. emberAfPluginGbzMessageControllerCreatorCleanup() - memory clean up
 *
 * @param state      A pre-allocated structure that will be updated to hold
 *                   bookkeeping information for creating GBZ messages.
 * @param type       GBZ payload type: command, response, or alert.
 * @param alertCode  When type is alert this field contains the alert code.
 * @param timestamp  When type is alert this field contains the UTC when the alert occurred.
 * @param gbzCommand NULL - if the user wants API to allocate memory to store
 *                          responses.
 *                   Otherwise - a pointer to destination buffer for GBZ messages
 * @param gbzCommandLength Length of GBZ messages. this argument is
 *                         ignored if gbzCommand is NULL.
 *
 */
uint16_t emberAfPluginGbzMessageControllerCreatorInit(EmberAfGbzMessageCreatorState * state,
                                                      EmberAfGbzMessageType type,
                                                      uint16_t alertCode,
                                                      uint32_t timestamp,
                                                      uint16_t messageCode,
                                                      uint8_t * gbzCommand,
                                                      uint16_t gbzCommandLength);

/*
 * @bref Returns commands size of the given parser structure.
 *
 * @param state A structure containing the bookkeeping information of parsing GBZ messages.
 */

uint8_t emberAfPluginGbzMessageControllerGetComponentSize(EmberAfGbzMessageParserState * state);

/*
 * @brief Cleans up/frees all the allocated memory used to
 *        store the overall GBZ response.
 */
void emberAfPluginGbzMessageControllerCreatorCleanup(EmberAfGbzMessageCreatorState * state);

/*
 * @brief Gets the next available ZCL command from the given GBZ parser structure.
 *
 * If any payload is encrypted, the decrypted data will overwrite the old
 * data.
 *
 * @param state A structure that retains the bookkeeping info of parsing GBZ messages.
 * @param gbzZclCommand A pre-allocated buffer that will be modified with the
 *   next available ZCL command's information.
 */
void emberAfPluginGbzMessageControllerNextCommand(EmberAfGbzMessageParserState * state, EmberAfGbzZclCommand * gbzZclCommand);

/*
 * @brief Prints out all information retained in a EmberAfGbzZclCommand structure.
 */
void emberAfPluginGbzMessageControllerPrintCommandInfo(EmberAfGbzZclCommand  * gbzZclCommand);

/*
 * @ Gets the length of the overall GBZ message.
 */
uint16_t emAfPluginGbzMessageControllerGetLength(EmberAfGbzZclCommand * cmd,
                                                 EmberAfGbzMessageData * msg);

/*
 * @ brief Indicates whether the ZCL payload will be encrypted or not.
 */
bool emberAfPluginGbzMessageControllerGetEncryptPayloadFlag(EmberAfGbzMessageCreatorState * state,
                                                            EmberAfGbzZclCommand * resp);

#endif // #ifndef _GBZ_MESSAGE_CONTROLLER_
