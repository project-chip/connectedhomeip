/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#include EMBER_AF_API_BUFFER_MANAGEMENT
#include EMBER_AF_API_BUFFER_QUEUE
#include EMBER_AF_API_EVENT_QUEUE
#include EMBER_AF_API_HAL
#include "thread-callbacks.h"
#include EMBER_AF_API_ZCL_CORE
#include "zcl-core-reporting.h"

#ifdef EM_ZCL_ENDPOINT_CLUSTERS_WITH_REPORTABLE_ATTRIBUTES
  #if EM_ZCL_ENDPOINT_CLUSTERS_WITH_REPORTABLE_ATTRIBUTES > EMBER_ZCL_REPORTING_CONFIGURATIONS_TABLE_SIZE
// Catch reporting configuration table size errors at compile time- the table must be at least
// big enough to hold a 'default report configuration' for every ep/cluster that has one or
// more reportable attributes. If the table size is increased further, custom reporting
// configurations can also be saved.
    #error Reporting table size is too small. Increase the size through ZCL Core plugin options.
  #endif
#endif

#define S_TO_MS(s) ((s) * MILLISECOND_TICKS_PER_SECOND)

// Spec 16_07008-30: 3.11.3.2 Maximum Reporting Interval-
// ... If the Maximum Reporting Interval is set to 0, there is no periodic
// reporting, but change based reporting is still operational.
#define MAXIMUM_INTERVAL_S_DISABLE_PERIODIC_REPORTING  0x0000

// Spec 16_07008-30: 2.6.3 Notification Generation startup delay-
// At startup, devices SHALL wait a random delay between X and Y seconds
// before beginning to send notifications.
#define NOTIFY_STARTUP_DELAY_S_MIN    5   // Min and Max limits as per (draft) Spec 16_07008-32: 2.6.3.
#define NOTIFY_STARTUP_DELAY_S_MAX    15  //TODO?- provide a zcl-core plugin option so that manufacturer can increase this.

// Defines for nv reporting configuration mirror table.
// (the nv table mirrors the ram-based configurations buffer list).
#define NV_CONFIG_TABLE_NULL_INDEX         0xFF

// Defines for nv reporting configuration storage.
#define retrieveNvConfigurationEntry(entry, i) \
  halCommonGetIndexedToken(&entry, TOKEN_ZCL_CORE_REPORTING_CONFIGURATIONS_TABLE, i)
#define saveNvConfigurationEntry(entry, i) \
  halCommonSetIndexedToken(TOKEN_ZCL_CORE_REPORTING_CONFIGURATIONS_TABLE, i, &entry)

// Define type for ram-based reporting configurations.
typedef struct {
  EmberZclEndpointId_t endpointId;
  EmberZclClusterSpec_t clusterSpec;
  EmberZclReportingConfigurationId_t reportingConfigurationId;
  size_t sizeAttrFlags;
  size_t sizeReportableChanges;
  size_t sizeLastValues;
  uint32_t lastTimeMs;
  uint16_t minimumIntervalS;
  uint16_t maximumIntervalS;
} Configuration_t;
// The Configuration_t is followed by
// [AttrFlags][ReportableChanges][LowThresholds][HighThresholds][LastValues] byte arrays.
// Low and High Thresholds have the same size and are tracked by the same offset counter
// as ReportableChanges. Each AttrFlags entry is a single byte, in the rest of these arrays
// each entry occupies attribute->size bytes.
// There is an AttrFlags and a LastValues entry for every local reportable attribute.
// There is a reportableChanges, LowThresholds and HighThresholds entry for every local
// reportable analog attribute. However, the contents of a ReportableChanges entry are valid
// only if the AttrFlags entry for this attribute has ATTR_PRESENT sent; the contents of
// LowThresholds and HighThresholds are valid only if the  AttrFlags entry has the
// LOW/HIGH_THRESHOLD_PRESENT flags set (ATTR_PRESENT would also be set in this case).

// Bitmask used in AttrFlags. Note that Reportable Change is always present for
// analog attributes (BDB 3.1.1.3.3.1)
#define ATTR_PRESENT 0x1
#define LOW_THRESHOLD_PRESENT 0x02
#define HIGH_THRESHOLD_PRESENT 0x04

#define EMBER_ZCLIP_STRUCT Configuration_t
static const ZclipStructSpec configurationSpec[] = {
  EMBER_ZCLIP_OBJECT(sizeof(EMBER_ZCLIP_STRUCT),
                     2,     // fieldCount
                     NULL), // names
  EMBER_ZCLIP_FIELD_NAMED_MANDATORY(EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, minimumIntervalS, "n"),
  EMBER_ZCLIP_FIELD_NAMED_MANDATORY(EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, maximumIntervalS, "x"),
};
#undef EMBER_ZCLIP_STRUCT

typedef struct {
  uint8_t uri[EMBER_ZCL_URI_MAX_LENGTH];
} Binding_t;
#define EMBER_ZCLIP_STRUCT Binding_t
static const ZclipStructSpec bindingSpec[] = {
  EMBER_ZCLIP_OBJECT(sizeof(EMBER_ZCLIP_STRUCT),
                     1,     // fieldCount
                     NULL), // names
  EMBER_ZCLIP_FIELD_NAMED_MANDATORY(EMBER_ZCLIP_TYPE_MAX_LENGTH_STRING, uri, "u"),
};
#undef EMBER_ZCLIP_STRUCT

#define ZCL_CORE_REPORTING_EVENT_NAME_STR  "zcl core reporting"

typedef struct {
  Event event;
  EmberZclEndpointId_t endpointId;
  EmberZclClusterSpec_t clusterSpec;
  EmberZclReportingConfigurationId_t reportingConfigurationId;
  // NULL_BUFFER when a notification should be made and something else when the
  // notification in the Buffer should be sent.
  Buffer notification;
} NotificationEvent;
enum {
  REPORTABLE_CHANGE = 0,
  LOW_THRESHOLD     = 1,
  HIGH_THRESHOLD    = 2,
}; // Used in hasReportableChangeOrThresholdCrossing() parameter
extern EventQueue emAppEventQueue;

static Buffer configurations = NULL_BUFFER;
static uint16_t notifyStartupDelaySecs = 0;  //TODO- Could be a uint8_t depending on max limit required by spec.

static void eventHandler(const NotificationEvent *event);
static void makeNotification(const NotificationEvent *event);
static void sendNotification(const NotificationEvent *event);
static void eventMarker(NotificationEvent *event);
static bool makeNotificationPredicate(NotificationEvent *event,
                                      const Configuration_t *configuration);
static NotificationEvent *cancelMakeNotification(const Configuration_t *configuration);
static void scheduleMakeNotification(const Configuration_t *configuration,
                                     uint32_t delayMs);
static bool sendNotificationPredicate(NotificationEvent *event,
                                      const Configuration_t *configuration);
static NotificationEvent *cancelSendNotification(const Configuration_t *configuration);
static void scheduleSendNotification(const Configuration_t *configuration,
                                     Buffer notification);
static bool reportingEventsPredicate(NotificationEvent *event);
static void cancelReportingEvents(void);
static bool addReportingConfiguration(Buffer buffer, bool nvUpdate);
static EmberZclStatus_t decodeReportingConfigurationOta(const EmZclContext_t *context,
                                                        Buffer buffer);
static bool decodeBindingUri(const EmZclContext_t *context,
                             EmberZclBindingEntry_t *entry);
static void setDefaultReportableValue(const EmZclAttributeEntry_t *attribute,
                                      uint8_t *buffer);
static Buffer makeReportingConfiguration(EmberZclEndpointId_t endpointId,
                                         const EmberZclClusterSpec_t *clusterSpec);
static void resetToDefaultReportingConfiguration(Configuration_t *configuration);
static void deleteReportingConfiguration(const Configuration_t *configuration);
static Configuration_t *findReportingConfiguration(EmberZclEndpointId_t endpointId,
                                                   const EmberZclClusterSpec_t *clusterSpec,
                                                   EmberZclReportingConfigurationId_t reportingConfigurationId);
static bool matchReportingConfiguration(EmberZclEndpointId_t endpointId,
                                        const EmberZclClusterSpec_t *clusterSpec,
                                        EmberZclReportingConfigurationId_t reportingConfigurationId,
                                        const Configuration_t *configuration);

static bool isValidReportingConfiguration(const Configuration_t *configuration);
static bool hasPeriodicReporting(const Configuration_t *configuration);
static bool hasReportableChangesOrThresholdCrossings(const Configuration_t *configuration);
static bool hasReportableChangeOrThresholdCrossing(const uint8_t *oldValue,
                                                   const uint8_t *reportableChange,
                                                   const uint8_t *newValue,
                                                   const EmZclAttributeEntry_t *attribute,
                                                   uint8_t checkType);
static bool findReportableAttributeIdInOtaConfigMap(CborState *state,
                                                    const EmZclAttributeEntry_t *searchAttr);
// TODO- Uncomment this function when EMBER_ZCLIP_TYPE_FLOAT is defined.
//static bool convertFloatToDblFloatFormat(const uint8_t *input,
//                                         uint8_t size,
//                                         double *output);

static void restoreReportingConfigurationsFromNv(void);
static bool updateNvReportingConfiguration(const Configuration_t *configuration,
                                           bool allowAdd);
static void deleteAllNvReportingConfigurations(void);
static void initialiseReportingConfigurations(void);
static void deleteConfigurationEntry(const Configuration_t *configuration,
                                     bool nvUpdate);

static EventActions actions = {
  .queue = &emAppEventQueue,
  .handler = (void (*)(struct Event_s *))eventHandler,
  .marker = (void (*)(struct Event_s *))eventMarker,
  .name = ZCL_CORE_REPORTING_EVENT_NAME_STR,
};

// Private functions -----------------------------------------------------------

// GET .../r
static void getReportingConfigurationIdsHandler(const EmZclContext_t *context)
{
  CborState state;
  uint8_t buffer[EM_ZCL_MAX_PAYLOAD_SIZE];
  if (!emCborEncodeIndefiniteArrayStart(&state, buffer, sizeof(buffer))) {
    emZclRespond500InternalServerError(context->info);
    return;
  }

  Buffer finger = emBufferQueueHead(&configurations);
  while (finger != NULL_BUFFER) {
    const Configuration_t *configuration
      = (const Configuration_t *)emGetBufferPointer(finger);
    if (context->endpoint->endpointId == configuration->endpointId
        && emberZclAreClusterSpecsEqual(&context->clusterSpec,
                                        &configuration->clusterSpec)
        && !emCborEncodeValue(&state,
                              EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                              sizeof(configuration->reportingConfigurationId),
                              (const uint8_t *)&configuration->reportingConfigurationId)) {
      emZclRespond500InternalServerError(context->info);
      return;
    }
    finger = emBufferQueueNext(&configurations, finger);
  }

  if (emCborEncodeBreak(&state)) {
    emZclRespond205ContentCborState(context->info, &state);
  } else {
    emZclRespond500InternalServerError(context->info);
  }
}

// POST .../r
static void addReportingConfigurationHandler(const EmZclContext_t *context)
{
  Buffer buffer = makeReportingConfiguration(context->endpoint->endpointId,
                                             &context->clusterSpec);
  if (buffer == NULL_BUFFER) {
    emZclRespond500InternalServerError(context->info);
    return;
  }

  const Configuration_t *configuration
    = (const Configuration_t *)emGetBufferPointer(buffer);
  EmberZclStatus_t decodeStatus
    = decodeReportingConfigurationOta(context, buffer);
  if (decodeStatus != EMBER_ZCL_STATUS_SUCCESS) {
    emZclRespond400BadRequest(context->info);   // 16-07008-069, section 3.11.4
    return;
  }

  if (!addReportingConfiguration(buffer, true)) {  //nvUpdate=true
    emberZclRespondWithStatus(context->info,
                              EMBER_COAP_CODE_500_INTERNAL_SERVER_ERROR,
                              EMBER_ZCL_STATUS_INSUFFICIENT_SPACE);
    return;
  }

  // 16-07008-040, section 3.11.4:
  // The response payload SHALL be empty if any of the following are true:
  //   The Binding Destination URI parameter was not included in the request.
  //   The Binding Destination URI parameter was included in the request
  //    but the attempt to create a binding entry failed.
  // Otherwise, a binding entry was created, and the payload SHALL be set to
  // a CBOR-encoded text string representing the URI Path.

  EmberZclApplicationDestination_t destination = {
    .data.endpointId = context->endpoint->endpointId,
    .type = EMBER_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT,
  };

  uint8_t uriPathReporting[EMBER_ZCL_URI_PATH_MAX_LENGTH];
  emZclReportingConfigurationIdToUriPath(&destination,
                                         &context->clusterSpec,
                                         configuration->reportingConfigurationId,
                                         uriPathReporting);

  EmberZclBindingEntry_t entry = { 0 };
  if (!decodeBindingUri(context, &entry)) {
    // binding entry not requested or we were not able to parse the URI
    // so do not send any payload
    emZclRespond201Created(context->info, uriPathReporting);
    return;
  }

  entry.reportingConfigurationId = configuration->reportingConfigurationId;
  EmberZclBindingId_t bindingId = emberZclAddBinding(&entry);
  if (bindingId == EMBER_ZCL_BINDING_NULL) {
    // binding entry requested but we were unable to create it
    // so do not send any payload
    emZclRespond201Created(context->info, uriPathReporting);
    return;
  }

  CborState state;
  uint8_t payloadBuffer[EM_ZCL_MAX_PAYLOAD_SIZE];
  uint8_t uriPathBinding[EMBER_ZCL_URI_PATH_MAX_LENGTH];
  emZclBindingIdToUriPath(&destination,
                          &context->clusterSpec,
                          bindingId,
                          uriPathBinding);

  if (emCborEncodeIndefiniteMapStart(&state, payloadBuffer, sizeof(payloadBuffer))
      && emCborEncodeValue(&state,
                           EMBER_ZCLIP_TYPE_STRING,
                           0, // size - ignored
                           (const uint8_t *)"u")
      && emCborEncodeValue(&state,
                           EMBER_ZCLIP_TYPE_STRING,
                           0, // size - ignored
                           uriPathBinding)
      && emCborEncodeBreak(&state)) {
    // binding successfully created so send CBOR-encoded string in payload
    emZclRespond201CreatedCborState(context->info, uriPathReporting, &state);
  } else {
    emZclRespond500InternalServerError(context->info);
  }
}

// GET .../r/R
static void getReportingConfigurationHandler(const EmZclContext_t *context)
{
  const Configuration_t *configuration
    = findReportingConfiguration(context->endpoint->endpointId,
                                 &context->clusterSpec,
                                 context->reportingConfigurationId);
  if (configuration == NULL) {
    assert(false);
    emZclRespond500InternalServerError(context->info);
    return;
  }

  CborState state;
  uint8_t buffer[128];

  if (!emCborEncodeIndefiniteMapStart(&state, buffer, sizeof(buffer))
      || !emCborEncodeValue(&state,
                            EMBER_ZCLIP_TYPE_STRING,
                            0, // size - ignored
                            (const uint8_t *)"n")
      || !emCborEncodeValue(&state,
                            EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                            sizeof(configuration->minimumIntervalS),
                            (const uint8_t *)&configuration->minimumIntervalS)
      || !emCborEncodeValue(&state,
                            EMBER_ZCLIP_TYPE_STRING,
                            0, // size - ignored
                            (const uint8_t *)"x")
      || !emCborEncodeValue(&state,
                            EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                            sizeof(configuration->maximumIntervalS),
                            (const uint8_t *)&configuration->maximumIntervalS)
      || !emCborEncodeValue(&state,
                            EMBER_ZCLIP_TYPE_STRING,
                            0, // size - ignored
                            (const uint8_t *)"a")
      || !emCborEncodeIndefiniteMap(&state)) {
    emZclRespond500InternalServerError(context->info);
    return;
  }

  // For layout description see comment following the definition of Configuration_t
  const uint8_t *attrFlags = ((uint8_t *)configuration  + sizeof(Configuration_t));
  const uint8_t *reportableChanges = attrFlags  + configuration->sizeAttrFlags;
  const uint8_t *lowThresholds = reportableChanges + configuration->sizeReportableChanges;
  const uint8_t *highThresholds = lowThresholds + configuration->sizeReportableChanges;
  size_t   offsetAttrFlags = 0;
  size_t   offsetReportableChange = 0;

  for (size_t i = 0; i < EM_ZCL_ATTRIBUTE_COUNT; i++) {
    const EmZclAttributeEntry_t *attribute = &emZclAttributeTable[i];
    int32_t compare
      = emberZclCompareClusterSpec(attribute->clusterSpec,
                                   &configuration->clusterSpec);
    if (compare > 0) {
      break;
    } else if ((compare == 0)
               && emZclIsAttributeLocal(attribute)
               && emZclIsAttributeReportable(attribute)) {
      if ((*(attrFlags + offsetAttrFlags)) & ATTR_PRESENT) {
        if (!emCborEncodeValue(&state,
                               EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                               sizeof(attribute->attributeId),
                               (const uint8_t *)&attribute->attributeId)
            || !emCborEncodeIndefiniteMap(&state)) {
          emZclRespond500InternalServerError(context->info);
          return;
        }

        if (emZclIsAttributeAnalog(attribute)) {
          assert(offsetReportableChange + attribute->size
                 <= configuration->sizeReportableChanges);

          // For analog attribute we add "r":repValue to the attribute map.
          if (!emCborEncodeValue(&state,
                                 EMBER_ZCLIP_TYPE_STRING,
                                 0,               // size - ignored
                                 (const uint8_t *)"r")
              || !emCborEncodeValue(&state,
                                    attribute->type,
                                    attribute->size,
                                    reportableChanges + offsetReportableChange)) {
            emZclRespond500InternalServerError(context->info);
            return;
          }
          // If Low Threshold is present add "l":lowThreshold to the attribute map.
          if (((*(attrFlags + offsetAttrFlags)) & LOW_THRESHOLD_PRESENT)) {
            if (!emCborEncodeValue(&state,
                                   EMBER_ZCLIP_TYPE_STRING,
                                   0,                   // size - ignored
                                   (const uint8_t *)"l")
                || !emCborEncodeValue(&state,
                                      attribute->type,
                                      attribute->size,
                                      lowThresholds + offsetReportableChange)) {
              emZclRespond500InternalServerError(context->info);
              return;
            }
          }
          // If High Threshold is present add "l":lowThreshold to the attribute map.
          if (((*(attrFlags + offsetAttrFlags)) & HIGH_THRESHOLD_PRESENT)) {
            if (!emCborEncodeValue(&state,
                                   EMBER_ZCLIP_TYPE_STRING,
                                   0,                   // size - ignored
                                   (const uint8_t *)"h")
                || !emCborEncodeValue(&state,
                                      attribute->type,
                                      attribute->size,
                                      highThresholds + offsetReportableChange)) {
              emZclRespond500InternalServerError(context->info);
              return;
            }
          }
        }           // if analog
        if (!emCborEncodeBreak(&state)) {
          emZclRespond500InternalServerError(context->info);
          return;
        }
      } // if ATTR_PRESENT
        // Advance offset regrdless of whether the attribute is present
      if (emZclIsAttributeAnalog(attribute)) {
        offsetReportableChange += attribute->size;              // also tracks low/high thresholds
      }
      offsetAttrFlags++;
    }   // attribute reportable
  }

  // The first break closes the inner map for "a" and the second closes
  // the outer map.
  if (emCborEncodeBreak(&state)
      && emCborEncodeBreak(&state)) {
    emZclRespond205ContentCborState(context->info, &state);
  } else {
    emZclRespond500InternalServerError(context->info);
  }
}

// PUT .../r/R
static void updateReportingConfigurationHandler(const EmZclContext_t *context)
{
  Configuration_t *configuration
    = findReportingConfiguration(context->endpoint->endpointId,
                                 &context->clusterSpec,
                                 context->reportingConfigurationId);
  if (configuration == NULL) {
    assert(false);
    emZclRespond500InternalServerError(context->info);
    return;
  }

  // BDB Spec 16-07008-071 3.12.3: Binding destination SHALL NOT be included
  Binding_t binding = {
    .uri = { 0 },
  };
  if (emCborDecodeOneStruct(context->payload,
                            context->payloadLength,
                            bindingSpec,
                            &binding)) {
    emZclRespond400BadRequest(context->info);
    return;
  }

  // Size of Low/High thresholds is sizeReportableChanges, hence 3*sizeReportableChanges
  Buffer buffer = emFillBuffer((const uint8_t *)configuration,
                               (sizeof(Configuration_t) + configuration->sizeAttrFlags
                                + 3 * configuration->sizeReportableChanges
                                + configuration->sizeLastValues));
  if (buffer == NULL_BUFFER) {
    emZclRespond500InternalServerError(context->info);
    return;
  }

  EmberZclStatus_t status = decodeReportingConfigurationOta(context, buffer);
  if (status == EMBER_ZCL_STATUS_SUCCESS) {
    MEMCOPY(configuration,
            emGetBufferPointer(buffer),
            emGetBufferLength(buffer));
    updateNvReportingConfiguration(configuration, false); // Config changed so update nv mirror (no add allowed).
    scheduleMakeNotification(configuration, 0);
    emZclRespond204ChangedWithStatus(context->info, status);
  } else {
    emZclRespond400BadRequest(context->info);
  }
}

// DELETE .../r/R
static void removeReportingConfigurationHandler(const EmZclContext_t *context)
{
  Configuration_t *configuration
    = findReportingConfiguration(context->endpoint->endpointId,
                                 &context->clusterSpec,
                                 context->reportingConfigurationId);
  if (configuration == NULL) {
    assert(false);
    emZclRespond500InternalServerError(context->info);
    return;
  }

  // Deleting a default reporting configuration just means resetting it to its
  // original state.  Any other reporting configuration is permanently deleted.
  if (configuration->reportingConfigurationId
      == EMBER_ZCL_REPORTING_CONFIGURATION_DEFAULT) {
    resetToDefaultReportingConfiguration(configuration);
    updateNvReportingConfiguration(configuration, false); // Config changed so update nv mirror (no add allowed).
    scheduleMakeNotification(configuration, 0);
  } else {
    deleteReportingConfiguration(configuration);
  }
  emZclRespond202Deleted(context->info);
}

static bool addReportingConfiguration(Buffer buffer, bool nvUpdate)
{
  Configuration_t *configuration
    = (Configuration_t *)emGetBufferPointer(buffer);

  EmberZclReportingConfigurationId_t reportingConfigurationId
    = EMBER_ZCL_REPORTING_CONFIGURATION_DEFAULT;
  bool added = false;

  Buffer tmp = configurations;
  configurations = NULL_BUFFER;

  while (!emBufferQueueIsEmpty(&tmp)) {
    Buffer finger = emBufferQueueRemoveHead(&tmp);
    const Configuration_t *fingee
      = (const Configuration_t *)emGetBufferPointer(finger);

    if (!added
        && configuration->endpointId == fingee->endpointId
        && emberZclAreClusterSpecsEqual(&configuration->clusterSpec,
                                        &fingee->clusterSpec)) {
      if (reportingConfigurationId < fingee->reportingConfigurationId) {
        configuration->reportingConfigurationId = reportingConfigurationId;
        emBufferQueueAdd(&configurations, buffer);
        added = true;
      } else if (reportingConfigurationId
                 < EMBER_ZCL_REPORTING_CONFIGURATION_NULL) {
        reportingConfigurationId++;
      }
    }
    emBufferQueueAdd(&configurations, finger);
  }

  if (!added) {
    if (reportingConfigurationId == EMBER_ZCL_REPORTING_CONFIGURATION_NULL) {
      return false;
    }
    configuration->reportingConfigurationId = reportingConfigurationId;
    emBufferQueueAdd(&configurations, buffer);
  }

  // Check if the configuration has at least one active reportable attribute.
  if (configuration->sizeLastValues != 0) {
    if (nvUpdate) {
      if (!updateNvReportingConfiguration(configuration, true)) {
        // We could not add the configuration to the nv table so delete the
        // ram-based entry to keep ram-based/nv configurations in sync).
        deleteConfigurationEntry(configuration, false);
        return false;
      }
    }

    // Start sending notifications right away for this reporting configuration.
    scheduleMakeNotification(configuration, 0);
  }

  return true;
}

static bool decodeBindingUri(const EmZclContext_t *context,
                             EmberZclBindingEntry_t *entry)
{
  Binding_t binding = {
    .uri = { 0 },
  };

  if (!emCborDecodeOneStruct(context->payload,
                             context->payloadLength,
                             bindingSpec,
                             &binding)) {
    return false;
  }

  entry->clusterSpec = context->clusterSpec; // not in URI
  entry->endpointId = context->endpoint->endpointId;

  return emZclUriToBindingEntry(binding.uri, entry, false);
}

static EmberZclStatus_t decodeReportingConfigurationOta(const EmZclContext_t *context,
                                                        Buffer buffer)
{
  Configuration_t *configuration
    = (Configuration_t *)emGetBufferPointer(buffer);
  if ((context->payloadLength != 0
       && !emCborDecodeOneStruct(context->payload,
                                 context->payloadLength,
                                 configurationSpec,
                                 configuration))
      || !isValidReportingConfiguration(configuration)) {
    // Spec 16-07008-071: 3.11.4 - If any conditions imposed upon non attribute fields
    // are not met the device SHALL respond with a 4.00 Bad Request with No Content.
    return EMBER_ZCL_STATUS_INVALID_FIELD;
  }

  // Decode attribute specific reportable fields from the "a":{<attrId>{ }} map.
  // For layout description see comment following the definition of Configuration_t
  uint8_t *attrFlags = ((uint8_t *)configuration + sizeof(Configuration_t));
  uint8_t *reportableChanges = attrFlags + configuration->sizeAttrFlags;
  uint8_t *lowThresholds = reportableChanges + configuration->sizeReportableChanges;
  uint8_t *highThresholds = lowThresholds + configuration->sizeReportableChanges;
  size_t offsetReportableChange = 0; // Also tracks offset into low/high Thresholds
  size_t offsetAttrFlags = 0;

  bool foundResult = false;
  for (size_t i = 0; i < EM_ZCL_ATTRIBUTE_COUNT; i++) {
    const EmZclAttributeEntry_t *attribute = &emZclAttributeTable[i];
    int32_t compare
      = emberZclCompareClusterSpec(attribute->clusterSpec,
                                   &context->clusterSpec);
    if (compare > 0) {
      break;
    } else if ((compare == 0)
               && (emZclIsAttributeLocal(attribute))
               && (emZclIsAttributeReportable(attribute))) {
      // Decode from the start of the payload for each reportable attribute.
      CborState state;
      emCborDecodeStart(&state,
                        context->payload,
                        context->payloadLength);

      bool foundAttributeIdInMap = findReportableAttributeIdInOtaConfigMap(&state, attribute);

      if (foundAttributeIdInMap) {
        foundResult = true;
        *(attrFlags + offsetAttrFlags) = ATTR_PRESENT;
      } else {
        *(attrFlags + offsetAttrFlags) = 0x0;
      }

      if (emZclIsAttributeAnalog(attribute)) {
        assert(offsetReportableChange + attribute->size <= configuration->sizeReportableChanges);
        if (foundAttributeIdInMap) {
          bool reportableChangeFound = false;

          if (emCborDecodeMap(&state)) {
            while (!emCborPeekSequenceEnd(&state)) {
              uint8_t type = emCborDecodePeek(&state, NULL);
              if (type == CBOR_TEXT) {
                uint8_t keyName[CBOR_TEXT_KEY_MAX_LENGTH];
                if (!emCborDecodeValue(&state,
                                       EMBER_ZCLIP_TYPE_MAX_LENGTH_STRING,
                                       sizeof(keyName),
                                       keyName)) {
                  return EMBER_ZCL_STATUS_INVALID_FIELD;
                }
                if (strcmp((const char *)keyName, "r") == 0) {
                  if (!emCborDecodeValue(&state, EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                                         attribute->size, reportableChanges + offsetReportableChange)) {
                    return EMBER_ZCL_STATUS_INVALID_FIELD;
                  }
                  reportableChangeFound = true;
                } else if (strcmp((const char *)keyName, "l") == 0) {
                  if (!emCborDecodeValue(&state, EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                                         attribute->size, lowThresholds + offsetReportableChange)) {
                    return EMBER_ZCL_STATUS_INVALID_FIELD;
                  }
                  *(attrFlags + offsetAttrFlags) |= LOW_THRESHOLD_PRESENT;
                } else if (strcmp((const char *)keyName, "h") == 0) {
                  if (!emCborDecodeValue(&state, EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                                         attribute->size, highThresholds + offsetReportableChange)) {
                    return EMBER_ZCL_STATUS_INVALID_FIELD;
                  }
                  *(attrFlags + offsetAttrFlags) |= HIGH_THRESHOLD_PRESENT;
                } else {
                  return EMBER_ZCL_STATUS_INVALID_FIELD;
                }
              } else {         // some unknown key
                return EMBER_ZCL_STATUS_INVALID_FIELD;
              }
            }       // while()
          } else {
            return EMBER_ZCL_STATUS_INVALID_FIELD;
          }
          if (!reportableChangeFound) {
            // Reportable Change SHALL be included
            return EMBER_ZCL_STATUS_INVALID_FIELD;
          }
        }
        // offsetReportableChange is also the offset into low/highThresholds
        offsetReportableChange += attribute->size;
      } else {
        // For discreet attributes r,h and l fields SHALL be omitted
        // Send error if we get anything other than an empty map
        if (!(emCborDecodeMap(&state) && emCborPeekSequenceEnd(&state))) {
          return EMBER_ZCL_STATUS_INVALID_FIELD;
        }
      }
      offsetAttrFlags++; // attrFlags tracks all local reportable attributes
    } // reportable attr
  }

  if (!foundResult) {
    // Spec 16_07008-060: 3.11.4 - "If the attribute map does not contain at
    // least one reportable attribute ID the device SHALL respond with a 4.00
    // Bad Request." Note- this can be an empty map, e.g. "a":{<attrId>:{}}
    return EMBER_ZCL_STATUS_NOT_FOUND;
  }

  return EMBER_ZCL_STATUS_SUCCESS;
}

static Buffer makeReportingConfiguration(EmberZclEndpointId_t endpointId,
                                         const EmberZclClusterSpec_t *clusterSpec)
{
  size_t sizeAttrFlags = 0;
  size_t sizeReportableChanges = 0;
  size_t sizeLastValues = 0;

  for (size_t i = 0; i < EM_ZCL_ATTRIBUTE_COUNT; i++) {
    const EmZclAttributeEntry_t *attribute = &emZclAttributeTable[i];
    int32_t compare
      = emberZclCompareClusterSpec(attribute->clusterSpec, clusterSpec);
    if (compare > 0) {
      break;
    } else if (compare == 0
               && emZclIsAttributeLocal(attribute)
               && emZclIsAttributeReportable(attribute)) {
      if (emZclIsAttributeAnalog(attribute)) {
        sizeReportableChanges += attribute->size;
      }
      sizeAttrFlags++;
      sizeLastValues += attribute->size;
    }
  }

  Buffer buffer = NULL_BUFFER;
  // Size of Low/High thresholds is sizeReportableChanges, hence 3*sizeReportableChanges
  buffer = emAllocateBuffer(sizeof(Configuration_t) + sizeAttrFlags
                            + 3 * sizeReportableChanges
                            + sizeLastValues);

  if (buffer != NULL_BUFFER) {
    Configuration_t *configuration
      = (Configuration_t *)emGetBufferPointer(buffer);
    configuration->endpointId = endpointId;
    configuration->clusterSpec = *clusterSpec;
    configuration->reportingConfigurationId
      = EMBER_ZCL_REPORTING_CONFIGURATION_NULL;
    configuration->sizeAttrFlags = sizeAttrFlags;
    configuration->sizeReportableChanges = sizeReportableChanges;
    configuration->sizeLastValues = sizeLastValues;
    configuration->lastTimeMs = 0;

    resetToDefaultReportingConfiguration(configuration);
  }

  return buffer;
}

static void resetToDefaultReportingConfiguration(Configuration_t *configuration)
{
  // 13-0402-13, section 6.7: A default report configuration (with a maximum
  // reporting interval either of 0x0000 or in the range 0x003d to 0xfffe)
  // SHALL exist for every implemented attribute that is specified as
  // reportable.
  EmberZclReportingConfiguration_t defaultReportingConfiguration = {
    .minimumIntervalS = 0x0000, // no minimum interval
    .maximumIntervalS = 0xFFFE, // maximum report every 65534 seconds
  };
  emberZclGetDefaultReportingConfigurationCallback(configuration->endpointId,
                                                   &configuration->clusterSpec,
                                                   &defaultReportingConfiguration);
  configuration->minimumIntervalS
    = defaultReportingConfiguration.minimumIntervalS;
  configuration->maximumIntervalS
    = defaultReportingConfiguration.maximumIntervalS;
  assert(isValidReportingConfiguration(configuration));

  // For layout description see comment following the definition of Configuration_t
  uint8_t *attrFlags = ((uint8_t *)configuration  + sizeof(Configuration_t));
  uint8_t *reportableChanges = attrFlags + configuration->sizeAttrFlags;
  uint8_t *lowThresholds = reportableChanges + configuration->sizeReportableChanges;
  uint8_t *highThresholds = lowThresholds + configuration->sizeReportableChanges;
  uint8_t *lastValues = highThresholds + configuration->sizeReportableChanges;
  size_t   offsetAttrFlags = 0;
  size_t   offsetReportableChange = 0;
  size_t   offsetLastValue = 0;

  for (size_t i = 0; i < EM_ZCL_ATTRIBUTE_COUNT; i++) {
    const EmZclAttributeEntry_t *attribute = &emZclAttributeTable[i];
    int32_t compare
      = emberZclCompareClusterSpec(attribute->clusterSpec,
                                   &configuration->clusterSpec);
    if (compare > 0) {
      break;
    } else if (compare == 0
               && emZclIsAttributeLocal(attribute)
               && emZclIsAttributeReportable(attribute)) {
      if (emZclIsAttributeAnalog(attribute)) {
        assert(offsetReportableChange + attribute->size
               <= configuration->sizeReportableChanges);

        setDefaultReportableValue(attribute,
                                  (reportableChanges
                                   + offsetReportableChange));

        // Callback may modify the default reportable value for the analog attribute.
        emberZclGetDefaultReportableChangeCallback(configuration->endpointId,
                                                   &configuration->clusterSpec,
                                                   attribute->attributeId,
                                                   (reportableChanges
                                                    + offsetReportableChange),
                                                   attribute->size);

        // Set the defaults for low/high threasholds to be the same as default reportable
        // change. Technically not needed as attrFlags tracks the content validity.
        // TODO: Rename the function
        // TODO: Do we need a callback for the threshold values, similar to the ReportableChange one?
        setDefaultReportableValue(attribute, (lowThresholds + offsetReportableChange));
        setDefaultReportableValue(attribute, (highThresholds + offsetReportableChange));

        offsetReportableChange += attribute->size;
      }

      // Initialize attrFlags to PRESENT. Every reportable attribute is included in the default report
      *(attrFlags + offsetAttrFlags) = ATTR_PRESENT;
      offsetAttrFlags++;

      assert(offsetLastValue + attribute->size
             <= configuration->sizeLastValues);

      // Initialise attribute lastValue with the present attribute value.
      if (emZclReadAttributeEntry(configuration->endpointId,
                                  attribute,
                                  (lastValues + offsetLastValue),
                                  attribute->size)
          != EMBER_ZCL_STATUS_SUCCESS) {
        MEMSET(lastValues + offsetLastValue, 0x00, attribute->size);
      }
      offsetLastValue += attribute->size;
    }
  }
}

static void setDefaultReportableValue(const EmZclAttributeEntry_t *attribute,
                                      uint8_t *buffer)
{
  // Sets the appropriate analog attribute default reportable value in the
  // input buffer.

  if (!emZclIsAttributeAnalog(attribute)) {
    assert(false);
    return; // Invalid call.
  }

  switch (attribute->type) {
    case EMBER_ZCLIP_TYPE_INTEGER:
      // Set buffer MS byte sign bit for analog signed integer types.
      MEMSET(buffer, 0x00, attribute->size);
  #if BIGENDIAN_CPU
      buffer[0] = 0x80; // Set top bit of MS byte.
  #else
      buffer[attribute->size - 1] = 0x80; // Set top bit of MS byte.
  #endif
      break;
    case EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER:
      //case EMBER_ZCLIP_TYPE_UTC: //TODO-
      MEMSET(buffer, 0x00, attribute->size);
      break;
    //case EMBER_ZCLIP_TYPE_FLOAT: //TODO-
    //  MEMSET(buffer, 0xFF, attribute->size);
    //  break;
    //case EMBER_ZCLIP_TYPE_TIME_OF_DAY: //TODO-
    //case EMBER_ZCLIP_TYPE_DATE_DATA: //TODO-
    //  MEMSET(buffer, 0xFF, attribute->size);
    //  break;
    default:
      // undefined analog type.
      assert(false);
  }
}

static void deleteReportingConfiguration(const Configuration_t *configuration)
{
  // Removes the specified reporting configuration.
  // Note- this fn is permitted to delete Default configurations (Id0) so care
  // should be taken that these are re-created as necessary.

  deleteConfigurationEntry(configuration, true); // delete ram-based and nv configuration entries.
  cancelMakeNotification(configuration);   // cancel any pending events.
  cancelSendNotification(configuration);

  // 16-07008-069, section 3.12.6: For report configurations with an identifier that does not match
  // the default report configuration identifier, a device SHALL permanently remove the attribute
  // report configuration entry. The device SHALL then iterate through each entry in the binding table
  // and update any entry that references deleted attribute report configuration to point to the null
  // report configuration
  if (configuration->reportingConfigurationId != EMBER_ZCL_REPORTING_CONFIGURATION_DEFAULT) {
    for (EmberZclBindingId_t i = 0; i < EMBER_ZCL_BINDING_TABLE_SIZE; i++) {
      EmberZclBindingEntry_t entry;
      if (emberZclGetBinding(i, &entry)) {
        if ((configuration->endpointId == entry.endpointId)
            && (emberZclAreClusterSpecsEqual(&configuration->clusterSpec,
                                             &entry.clusterSpec))
            && (configuration->reportingConfigurationId
                == entry.reportingConfigurationId)) {
          entry.reportingConfigurationId = EMBER_ZCL_REPORTING_CONFIGURATION_NULL;
          emberZclSetBinding(i, &entry);
        }
      }
    }
  }
}

static Configuration_t *findReportingConfiguration(EmberZclEndpointId_t endpointId,
                                                   const EmberZclClusterSpec_t *clusterSpec,
                                                   EmberZclReportingConfigurationId_t reportingConfigurationId)
{
  // Finds the first reporting configuration in the ram-based configurations
  // buffer list which matches the specified search spec.

  if (configurations != NULL_BUFFER) {
    Buffer finger = emBufferQueueHead(&configurations);
    while (finger != NULL_BUFFER) {
      Configuration_t *configuration
        = (Configuration_t *)emGetBufferPointer(finger);
      if (matchReportingConfiguration(endpointId,
                                      clusterSpec,
                                      reportingConfigurationId,
                                      configuration)) {
        return configuration;
      }
      finger = emBufferQueueNext(&configurations, finger);
    }
  }
  return NULL;
}

static bool matchReportingConfiguration(EmberZclEndpointId_t endpointId,
                                        const EmberZclClusterSpec_t *clusterSpec,
                                        EmberZclReportingConfigurationId_t reportingConfigurationId,
                                        const Configuration_t *configuration)
{
  // Checks if the reporting configuration matches the search spec.

  return (endpointId == configuration->endpointId
          && emberZclAreClusterSpecsEqual(clusterSpec,
                                          &configuration->clusterSpec)
          && (reportingConfigurationId
              == configuration->reportingConfigurationId));
}

static void eventHandler(const NotificationEvent *event)
{
  if (event->notification == NULL_BUFFER) {
    makeNotification(event);
  } else {
    sendNotification(event);
  }
}

static void makeNotification(const NotificationEvent *event)
{
  Configuration_t *configuration
    = findReportingConfiguration(event->endpointId,
                                 &event->clusterSpec,
                                 event->reportingConfigurationId);
  if ((configuration == NULL)
      || (configuration->sizeLastValues == 0)) {
    assert(false);
    return;
  }

  // If this reporting configuration has periodic reporting, then we want to
  // let this event fire again after the maximum interval.
  scheduleMakeNotification(configuration,
                           (hasPeriodicReporting(configuration)
                            ? (configuration->maximumIntervalS
                               * MILLISECOND_TICKS_PER_SECOND)
                            : UINT32_MAX)); // forever

  // emZclDestinationToUri gives a URI ending with the endpoint id.
  // We want to add just the the cluster id to that.
  EmberZclDestination_t destination = {
    .network = {
      .uid = emZclUid,
      .flags = EMBER_ZCL_HAVE_UID_FLAG,
      .port = EMBER_COAP_PORT,
    },
    .application = {
      .data.endpointId = configuration->endpointId,
      .type = EMBER_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT,
    },
  };
  uint8_t uri[EMBER_ZCL_URI_MAX_LENGTH] = { 0 };
  uint8_t *finger = uri;
  finger += emZclDestinationToUri(&destination, finger);
  *finger++ = '/';
  emZclClusterToUriPath(NULL, &configuration->clusterSpec, finger);

  // TODO: 16-07008-026, section 3.14.1 says "t" is included in the map for
  // timestamp, and UID or URI.  The timestamp is problematic, because not all
  // devices track time.  Fortunately, it is optional, so it is simply omitted
  // here.
  CborState state;
  uint8_t buffer[EM_ZCL_MAX_PAYLOAD_SIZE];
  if (!emCborEncodeIndefiniteMapStart(&state, buffer, sizeof(buffer))
      || !emCborEncodeValue(&state,
                            EMBER_ZCLIP_TYPE_STRING,
                            0, // size - ignored
                            (const uint8_t *)"r")
      || !emCborEncodeValue(&state,
                            EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                            sizeof(configuration->reportingConfigurationId),
                            (const uint8_t *)&configuration->reportingConfigurationId)
      //|| !emCborEncodeValue(&state,
      //                      EMBER_ZCLIP_TYPE_STRING,
      //                      0, // size - ignored
      //                      (const uint8_t *)"t")
      //|| !emCborEncodeValue(&state,
      //                      EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
      //                      sizeof(timestamp),
      //                      (const uint8_t *)&timestamp)
      || !emCborEncodeValue(&state,
                            EMBER_ZCLIP_TYPE_STRING,
                            0, // size - ignored
                            (const uint8_t *)"u")
      || !emCborEncodeValue(&state,
                            EMBER_ZCLIP_TYPE_STRING,
                            0, // size - ignored
                            uri)
      || !emCborEncodeValue(&state,
                            EMBER_ZCLIP_TYPE_STRING,
                            0, // size - ignored
                            (const uint8_t *)"a")
      || !emCborEncodeIndefiniteMap(&state)) {
    return;
  }

  // For layout description see comment following the definition of Configuration_t
  uint8_t *attrFlags = ((uint8_t *)configuration + sizeof(Configuration_t));
  uint8_t *lastValues = (attrFlags + configuration->sizeAttrFlags + 3 * (configuration->sizeReportableChanges));
  size_t offsetAttrFlags = 0;
  size_t offsetLastValue = 0;
  for (size_t i = 0; i < EM_ZCL_ATTRIBUTE_COUNT; i++) {
    const EmZclAttributeEntry_t *attribute = &emZclAttributeTable[i];
    int32_t compare
      = emberZclCompareClusterSpec(attribute->clusterSpec,
                                   &configuration->clusterSpec);
    if (compare > 0) {
      break;
    } else if (compare == 0
               && emZclIsAttributeLocal(attribute)
               && emZclIsAttributeReportable(attribute)) {
      assert(offsetLastValue + attribute->size  <= configuration->sizeLastValues);
      if (((*(attrFlags + offsetAttrFlags)) & ATTR_PRESENT)
          && !emZclReadEncodeAttributeKeyValue(&state,
                                               configuration->endpointId,
                                               attribute,
                                               lastValues + offsetLastValue,
                                               attribute->size)) {
        return;
      }
      offsetAttrFlags++;
      offsetLastValue += attribute->size;
    }
  }
  if (!emCborEncodeBreak(&state)) { // Close the attribute map.
    return;
  }

  // Add bindingId placeholder value of 254, actual binding id value will be set
  // in sendNotification(). NOTE: sendNotification() assumes that the bindingId
  // is always the last field in the notification map).
  uint8_t bindingId = 254;  // encoded as two bytes (0x18 0xFE) in cbor payload.
  if (!emCborEncodeValue(&state,
                         EMBER_ZCLIP_TYPE_STRING,
                         0, // size - ignored
                         (const uint8_t *)"b")
      || !emCborEncodeValue(&state,
                            EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                            sizeof(bindingId),
                            (const uint8_t *)&bindingId)) {
    return;
  }

  if (!emCborEncodeBreak(&state)) { // Close the outer map.
    return;
  }

  configuration->lastTimeMs = halCommonGetInt32uMillisecondTick();
  Buffer notification = emFillBuffer(buffer, emCborEncodeSize(&state));
  if (notification == NULL_BUFFER) {
    return;
  }

  scheduleSendNotification(configuration, notification);
}

static void sendNotification(const NotificationEvent *event)
{
  const Configuration_t *configuration
    = findReportingConfiguration(event->endpointId,
                                 &event->clusterSpec,
                                 event->reportingConfigurationId);

  if ((configuration == NULL)
      || (configuration->sizeLastValues == 0)) {
    assert(false);
    return;
  }

  for (EmberZclBindingId_t bindingId = 0;
       bindingId < EMBER_ZCL_BINDING_TABLE_SIZE;
       bindingId++) {
    EmberZclBindingEntry_t entry = { 0 };
    if (emberZclGetBinding(bindingId, &entry)
        && event->endpointId == entry.endpointId
        && emberZclAreClusterSpecsEqual(&event->clusterSpec, &entry.clusterSpec)
        && event->reportingConfigurationId == entry.reportingConfigurationId) {
      EmberZclClusterSpec_t clusterSpec;
      emberZclReverseClusterSpec(&event->clusterSpec, &clusterSpec);
      uint8_t uriPath[EMBER_ZCL_URI_PATH_MAX_LENGTH];
      emZclNotificationToUriPath(&entry.destination.application,
                                 &clusterSpec,
                                 uriPath);

      // Get the notification (cbor) payload and modify the bindingId
      // value encoded in the "b":Id field.  Note: We assume that makeNotification()
      // encodes the "b":id field at the END of the cbor map and that a dummy id
      // value consisting of TWO cbor bytes has been encoded. (This assumption
      // greatly reduces the processing effort required here to encode the
      // real binding id).
      CborState state;
      emCborEncodeStart(&state,
                        emGetBufferPointer(event->notification),
                        emGetBufferLength(event->notification)); // length includes "b" + 2 bindingId placeholder bytes.
      state.finger = (uint8_t *)state.end - 2; // Set ptr to just before the 2 bindingId placeholder bytes.
      assert(*((uint8_t *)state.finger - 1) == 'b'); // Check payload is aligned as expected.
      if (!emCborEncodeValue(&state,
                             EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                             sizeof(bindingId),
                             (const uint8_t *)&bindingId)) { // Encode the real bindingId value.
        return;
      }

      EmberZclDestination_t destination;
      emZclReadDestinationFromBinding(&entry, &destination);
      emZclSend(&destination.network,
                EMBER_COAP_CODE_POST,
                uriPath,
                state.start,
                emCborEncodeSize(&state),
                NULL, // handler
                NULL, // application data
                0,    // application data length
                true); // do skip retry
    }
  }
}

static void eventMarker(NotificationEvent *event)
{
  emMarkBuffer(&event->notification);
}

static bool makeNotificationPredicate(NotificationEvent *event,
                                      const Configuration_t *configuration)
{
  return (configuration->endpointId == event->endpointId
          && emberZclAreClusterSpecsEqual(&configuration->clusterSpec,
                                          &event->clusterSpec)
          && (configuration->reportingConfigurationId
              == event->reportingConfigurationId)
          && event->notification == NULL_BUFFER);
}

static NotificationEvent *cancelMakeNotification(const Configuration_t *configuration)
{
  return (NotificationEvent *)emberFindEvent(actions.queue,
                                             &actions,
                                             (EventPredicate)makeNotificationPredicate,
                                             (void *)configuration);
}

static void scheduleMakeNotification(const Configuration_t *configuration,
                                     uint32_t delayMs)
{
  NotificationEvent *event = cancelMakeNotification(configuration);
  if (delayMs != UINT32_MAX && configuration->sizeLastValues != 0) {
    if (event == NULL) {
      Buffer buffer = emAllocateBuffer(sizeof(NotificationEvent));
      if (buffer != NULL_BUFFER) {
        event = (NotificationEvent *)emGetBufferPointer(buffer);
      }
    }
    if (event != NULL) {
      event->event.actions = &actions;
      event->event.next = NULL;
      event->endpointId = configuration->endpointId;
      event->clusterSpec = configuration->clusterSpec;
      event->reportingConfigurationId = configuration->reportingConfigurationId;
      event->notification = NULL_BUFFER;
      emberEventSetDelayMs((Event *)event, delayMs);
    }
  }
}

static bool sendNotificationPredicate(NotificationEvent *event,
                                      const Configuration_t *configuration)
{
  return (configuration->endpointId == event->endpointId
          && emberZclAreClusterSpecsEqual(&configuration->clusterSpec,
                                          &event->clusterSpec)
          && (configuration->reportingConfigurationId
              == event->reportingConfigurationId)
          && event->notification != NULL_BUFFER);
}

static NotificationEvent *cancelSendNotification(const Configuration_t *configuration)
{
  return (NotificationEvent *)emberFindEvent(actions.queue,
                                             &actions,
                                             (EventPredicate)sendNotificationPredicate,
                                             (void *)configuration);
}

static void scheduleSendNotification(const Configuration_t *configuration,
                                     Buffer notification)
{
  assert(configuration->sizeLastValues != 0);

  NotificationEvent *event = cancelSendNotification(configuration);
  if (event == NULL) {
    Buffer buffer = emAllocateBuffer(sizeof(NotificationEvent));
    if (buffer != NULL_BUFFER) {
      event = (NotificationEvent *)emGetBufferPointer(buffer);
    }
  }
  if (event != NULL) {
    event->event.actions = &actions;
    event->event.next = NULL;
    event->endpointId = configuration->endpointId;
    event->clusterSpec = configuration->clusterSpec;
    event->reportingConfigurationId = configuration->reportingConfigurationId;
    event->notification = notification;

    // Note- notification 'backoff' parameter removed (Spec 16_07008-30: 2.6.3).

    // Spec 16_07008-30: 2.6.3 Notification Generation- At startup, devices
    // SHALL wait a random delay between X and Y seconds before beginning to
    // send notifications. Devices SHALL NOT send any reports prior to the delay
    // expiring.
    if (notifyStartupDelaySecs == 0) {
      // Note- notifyStartupDelaySecs is global so will be set one time only.
      notifyStartupDelaySecs = (halCommonGetRandom()
                                % (NOTIFY_STARTUP_DELAY_S_MAX - NOTIFY_STARTUP_DELAY_S_MIN))
                               + NOTIFY_STARTUP_DELAY_S_MIN;
    }
    uint32_t timeNowMs = halCommonGetInt32uMillisecondTick(); // Get time since startup in mS.
    uint32_t delayMs;
    delayMs = (timeNowMs < S_TO_MS(notifyStartupDelaySecs)
               ? S_TO_MS(notifyStartupDelaySecs) - timeNowMs
               : 0);   // Send immediately.
    emberEventSetDelayMs((Event *)event, delayMs);
  }
}

static bool reportingEventsPredicate(NotificationEvent *event)
{
  // Match on event name.
  return (strcmp((const char *)event->event.actions->name,
                 ZCL_CORE_REPORTING_EVENT_NAME_STR)
          == 0);
}

static void cancelReportingEvents(void)
{
  // Cancel all reporting events.
  emberFindAllEvents(actions.queue,
                     &actions,
                     (EventPredicate)reportingEventsPredicate,
                     (void *)NULL);
}

static bool isValidReportingConfiguration(const Configuration_t *configuration)
{
  // Spec 16_07008-30:
  // 3.11.3.1 Minimum Reporting Interval- ... The value of Minimum Reporting
  // Interval SHALL be less than or equal to the value of Maximum Reporting Interval.
  // 3.11.3.2 Maximum Reporting Interval- ... If the Maximum Reporting Interval
  // is set to 0, there is no periodic reporting, but change based reporting is
  // still operational.
  // NOTE: ZCL R07 is slated to specify the following resolution for CCB 2266:
  // Disallow setting the field to 0 without the minimum reporting interval
  // field set to 0xffff. Return an error when this isn't the case.

  if (configuration->maximumIntervalS == MAXIMUM_INTERVAL_S_DISABLE_PERIODIC_REPORTING) {
    if (configuration->minimumIntervalS == 0xFFFF) {
      return true;
    }
  } else {
    if (configuration->minimumIntervalS <= configuration->maximumIntervalS) {
      return true;
    }
  }

  return false;
}

static bool hasPeriodicReporting(const Configuration_t *configuration)
{
  return (configuration->maximumIntervalS
          != MAXIMUM_INTERVAL_S_DISABLE_PERIODIC_REPORTING);
}

static bool hasReportableChangesOrThresholdCrossings(const Configuration_t *configuration)
{
  if (configuration->sizeLastValues == 0) {
    return false;
  }

  // For layout description see comment following the definition of Configuration_t
  const uint8_t *attrFlags = (const uint8_t *)configuration + sizeof(Configuration_t);
  const uint8_t *reportableChanges = attrFlags + configuration->sizeAttrFlags;
  const uint8_t *lowThresholds = reportableChanges + configuration->sizeReportableChanges;
  const uint8_t *highThresholds = lowThresholds + configuration->sizeReportableChanges;
  const uint8_t *lastValues = highThresholds + configuration->sizeReportableChanges;
  size_t offsetAttrFlags = 0;
  size_t offsetReportableChange = 0;
  size_t offsetLastValue = 0;

  for (size_t i = 0; i < EM_ZCL_ATTRIBUTE_COUNT; i++) {
    const EmZclAttributeEntry_t *attribute = &emZclAttributeTable[i];
    int32_t compare = emberZclCompareClusterSpec(attribute->clusterSpec,
                                                 &configuration->clusterSpec);
    if (compare > 0) {
      break;
    } else if (compare == 0
               && emZclIsAttributeLocal(attribute)
               && emZclIsAttributeReportable(attribute)) {
      if (emZclIsAttributeAnalog(attribute)) {
        assert(offsetReportableChange + attribute->size   <= configuration->sizeReportableChanges);
      }
      assert(offsetLastValue + attribute->size <= configuration->sizeLastValues);

      uint8_t newValue[EMBER_ZCL_ATTRIBUTE_MAX_SIZE];
      // Examine attribute value only if the attribute is present in the report config
      if (((*(attrFlags + offsetAttrFlags)) & ATTR_PRESENT)
          && (emZclReadAttributeEntry(configuration->endpointId, attribute, newValue, sizeof(newValue)) == EMBER_ZCL_STATUS_SUCCESS)) {
        // offsetReportableChange tracks offset into all these: reportableChanges, lowThresholds, lowThresholds

        if (hasReportableChangeOrThresholdCrossing(lastValues + offsetLastValue, reportableChanges + offsetReportableChange,
                                                   newValue, attribute, REPORTABLE_CHANGE)) {
          return true;
        }
        if (((*(attrFlags + offsetAttrFlags)) & LOW_THRESHOLD_PRESENT)
            && hasReportableChangeOrThresholdCrossing(lastValues + offsetLastValue, lowThresholds + offsetReportableChange,
                                                      newValue, attribute, LOW_THRESHOLD)) {
          return true;
        }
        if (((*(attrFlags + offsetAttrFlags)) & HIGH_THRESHOLD_PRESENT)
            && hasReportableChangeOrThresholdCrossing(lastValues + offsetLastValue, highThresholds + offsetReportableChange,
                                                      newValue, attribute, HIGH_THRESHOLD)) {
          return true;
        }
      }
      if (emZclIsAttributeAnalog(attribute)) {
        offsetReportableChange += attribute->size;
      }
      offsetLastValue += attribute->size;
      offsetAttrFlags++;
    }
  }

  return false;
}

static bool hasReportableChangeOrThresholdCrossing(const uint8_t * oldValue,
                                                   const uint8_t * compareValue,
                                                   const uint8_t * newValue,
                                                   const EmZclAttributeEntry_t * attribute,
                                                   uint8_t checkType)
{
  // Depending on the checkType parameter value this function tests whether the change
  // in attribute warrants generating a report:
  //   - REPORTABLE_CHANGE: compareValue carries Reportable Change, return TRUE if
  //                        attribute change exceeds Reportable Change
  //   - LOW_THRESHOLD: compareValue carries Low Threshold, return TRUE if the
  //                        attribute moved below Low Threshold
  //   - HIGH_THRESHOLD: compareValue carries High Threshold, return TRUE if the
  //                        attribute moved above High Threshold

  // 07-5123-06, section 2.5.11.2.2: If the attribute has a 'discrete' data
  // type, a report SHALL be generated when the attribute undergoes any change
  // of value.
  // 07-5123-06, section 2.5.11.2.3: If the attribute has an 'analog' data
  // type, a report SHALL be generated when the attribute undergoes a change of
  // value, in a positive or negative direction, equal to or greater than the
  // Reportable Change for that attribute (see 2.5.7.1.7).  The change is
  // measured from the value of the attribute when the Reportable Change is
  // configured, and thereafter from the previously reported value of the
  // attribute.
  // 16-07008-071: Section 3.11.3.3.2: A transition of the attribute from a
  // value at or above the low threshold to a value below the low threshold SHALL
  // result in a notification being issued. Section 3.11.3.3.3: A transition of the
  // attribute from a value at or below the high threshold to a value above the high
  // threshold SHALL result in a notification being issued.
  if (emZclIsAttributeAnalog(attribute)) {
    switch (attribute->type) {
      case EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER:
      case EMBER_ZCLIP_TYPE_INTEGER: {
        //case EMBER_ZCLIP_TYPE_UTC: { //TODO-
        // Allocate local storage for processing numeric integer attribute types-
        // (Buffer SIZE+1 allows us to add a sign-extension byte which is necessary
        // for absolute difference caculation to return the correct result).
        uint8_t oldBuffer[EMBER_ZCL_ATTRIBUTE_MAX_SIZE + 1];
        uint8_t newBuffer[EMBER_ZCL_ATTRIBUTE_MAX_SIZE + 1];
        uint8_t compareBuffer[EMBER_ZCL_ATTRIBUTE_MAX_SIZE + 1];
        uint8_t diffBuffer[EMBER_ZCL_ATTRIBUTE_MAX_SIZE + 1];

        size_t size = attribute->size + 1;         // incr size to include one sign-extension byte.

        emZclSignExtendAttributeBuffer(oldBuffer,
                                       size,
                                       oldValue,
                                       attribute->size,
                                       attribute->type);
        emZclSignExtendAttributeBuffer(newBuffer,
                                       size,
                                       newValue,
                                       attribute->size,
                                       attribute->type);
        emZclSignExtendAttributeBuffer(compareBuffer,
                                       size,
                                       compareValue,
                                       attribute->size,
                                       attribute->type);

        if (checkType == REPORTABLE_CHANGE) {
          emZclGetAbsDifference(oldBuffer, newBuffer, diffBuffer, size);

          // Check if value in diff buffer > reportable buffer (both hold absolute
          // +ve values at this stage so check can be bytewise starting at MS byte).
          for (int i = size - 1; i >= 0; i--) {
            uint8_t reportableValueByte = compareBuffer[i];
            uint8_t diffValueByte = diffBuffer[i];
            if (reportableValueByte > diffValueByte) {
              return false;
            }
            if (diffValueByte > reportableValueByte) {
              return true;
            }
          }
        }

        if (checkType == LOW_THRESHOLD) {
          // Return FALSE if (old < low_threshold) or (new >= low_threshold)
          // emZclGetAbsDifference(A, B, diff, size) returns TRUE if A < B.
          // diff will be all zeros if A == B
          if (emZclGetAbsDifference(oldBuffer, compareBuffer, diffBuffer, size)
              || emZclGetAbsDifference(compareBuffer, newBuffer, diffBuffer, size)) {
            return false;
          }
          for (int i = size - 1; i >= 0; i--) {
            if (diffBuffer[i] != 0) {
              return true;               // new != low_threshold
            }
          }
          return false;           // diff is zero, i.e. new == low_threshold
        }

        if (checkType == HIGH_THRESHOLD) {
          // Return FALSE if (old > high_threshold) or (new <= high_threshold)
          // emZclGetAbsDifference(A, B, diff, size) returns TRUE if A < B,
          // diff will be all zeros if A == B
          if (emZclGetAbsDifference(compareBuffer, oldBuffer, diffBuffer, size)
              || emZclGetAbsDifference(newBuffer, compareBuffer, diffBuffer, size)) {
            return false;
          }
          for (int i = size - 1; i >= 0; i--) {
            if (diffBuffer[i] != 0) {
              return true;               // new != high_threshold
            }
          }
          return false;           // diff is zero, i.e. new == high_threshold
        }

        break;
      }

      // TODO- Uncomment this block when EMBER_ZCLIP_TYPE_FLOAT is defined.
      //case EMBER_ZCLIP_TYPE_FLOAT: {
      //  // Convert input old,new,reportable fp values (2,4 or 8 byte) to double
      //  // float format. Simple fp arithmetic can then be used to check for
      //  // reportable change.
      //  double floatOld, floatNew, floatReportable;
      //  if ((!convertFloatToDblFloatFormat(old, attribute->size, &floatOld))
      //      || (!convertFloatToDblFloatFormat(new, attribute->size, &floatNew))
      //      || (!convertFloatToDblFloatFormat(reportable, attribute->size, &floatReportable))) {
      //    return false;
      //  }
      //  if (floatReportable < 0.0) {
      //    return false;
      //  }
      //  return (((floatNew - floatOld) > floatReportable)
      //          || ((floatOld - floatNew) > floatReportable));
      //}

      // TODO- Uncomment this block when EMBER_ZCLIP_TYPE_TIME_OF_DAY and
      // EMBER_ZCLIP_TYPE_DATE_DATA are defined.
      //case EMBER_ZCLIP_TYPE_TIME_OF_DAY:
      //case EMBER_ZCLIP_TYPE_DATE_DATA: {
      //  // These attribute types have individual byte sub-fields arranged with the
      //  // most significant field in the MS byte. To check for reportable change
      //  // we check the difference against the reportable change value on a
      //  // bytewise basis starting at the MS byte.
      //  for (uint8_t i = attribute->size - 1; i < attribute->size; i--) {
      //    uint8_t tmpOld = ((uint8_t *)oldValue)[i];
      //    uint8_t tmpNew = ((uint8_t *)newValue)[i];
      //    uint8_t tmpReportable = ((uint8_t *)reportableChange)[i];
      //    if ((tmpOld == 0xFF)
      //        || (tmpNew == 0xFF)
      //        || (tmpReportable = 0xFF)) {
      //      continue;  // Ignore this subfield.
      //    }
      //    uint8_t diffValueByte = (tmpNew > tmpOld)
      //                             ? tmpNew - tmpOld
      //                             : tmpOld - tmpNew;
      //    if (tmpReportable > diffValueByte) {
      //      return false;
      //    }
      //    if (diffValueByte > tmpReportable) {
      //      return true;
      //    }
      //  }
      //  return false;
      //}

      default:
        // undefined analog type.
        assert(false);
        return false;
    }
  } else {
    // Discrete attribute type (includes BOOLEAN type).
    return (MEMCOMPARE(oldValue, newValue, attribute->size) != 0);
  }

  return false;
}

static bool findReportableAttributeIdInOtaConfigMap(CborState * state,
                                                    const EmZclAttributeEntry_t * searchAttr)
{
  // Returns true if the specified reportable attributeId is found in the
  // ota reporting configuration map, e.g. { "a":{<attrId>:{}} }

  if (!emZclIsAttributeReportable(searchAttr)) {
    assert(false);
    return false;
  }

  if (emCborDecodeMap(state)) {
    EmberZclAttributeId_t configAttrId;
    bool foundAttributeMap = false;
    while (true) {
      uint8_t type = emCborDecodePeek(state, NULL);
      if (foundAttributeMap) {
        if (type == CBOR_UNSIGNED) {
          // Decode the attributeId key value.
          if (!emCborDecodeValue(state,
                                 EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                                 sizeof(EmberZclAttributeId_t),
                                 (uint8_t *)&configAttrId)) {
            break;
          }
          if (configAttrId == searchAttr->attributeId) {
            return true;
          }
        }
      } else {
        if (type == CBOR_TEXT) {
          uint8_t keyName[CBOR_TEXT_KEY_MAX_LENGTH];
          if (!emCborDecodeValue(state,
                                 EMBER_ZCLIP_TYPE_MAX_LENGTH_STRING,
                                 sizeof(keyName),
                                 keyName)) {
            break;
          }
          if (strcmp((const char *)keyName, "a") == 0) {
            if (!emCborDecodeMap(state)) {
              break;
            }
            foundAttributeMap = true;
            continue;
          }
        }
      }
      // Skip the value.
      if (!emCborDecodeSkipValue(state)) {
        break;
      }
    }
  }

  return false;
}

// TODO- Uncomment this function when EMBER_ZCLIP_TYPE_FLOAT is defined.
//static bool convertFloatToDblFloatFormat(const uint8_t *input,
//                                         uint8_t size,
//                                         double *output) {
//  // Converts floating point value in input buffer to IEEE-754 DOUBLE-PREC format.
//  // Valid floating point input formats are:-
//  // SEMI-PREC: 2 bytes comprising- sign bit, 5-bit exponent, 10-bit mantissa.
//  // SINGLE PREC: 4 bytes comprising- sign bit, 8-bit exponent, 23-bit mantissa.
//  // DOUBLE-PREC: 8 bytes comprising- sign bit, 11-bit exponent, 52-bit mantissa.
//  // Returns false if an invalid size value is passed in.
//
//  uint8_t sign = (input[size-1] & 0x80) ? 1 : 0;
//  int16_t exponent;
//  uint64_t mantissa;
//
//  switch (size) {
//  case 2:
//    // SEMI-PREC: 2 bytes comprising- sign bit, 5-bit exponent, 10-bit mantissa.
//    MEMCOPY((uint8_t *)&exponent, &input[size-1], 1);
//    exponent = (exponent >> 2) & 0x001F; // get 5-bit exponent.
//    exponent = exponent - 15; // subtract bias offset get the real exponent.
//    exponent = exponent + 1023; // add bias offset for the output exponent.
//    MEMCOPY((uint8_t *)&mantissa, &input[size-2], 2);
//    mantissa = mantissa & (uint64_t)0x3FF; // get 10-bit mantissa.
//    mantissa = mantissa << (52 - 10);  // scale for output mantissa.
//    break;
//  case 4:
//    // SINGLE PREC: 4 bytes comprising- sign bit, 8-bit exponent, 23-bit mantissa
//    MEMCOPY((uint8_t *)&exponent, &input[size-2], 2);
//    exponent = (exponent >> 7) & 0x00FF; // get 8-bit exponent.
//    exponent = exponent - 127; // subtract bias offset get the real exponent.
//    exponent = exponent + 1023; // add bias offset for the output exponent.
//    MEMCOPY((uint8_t *)&mantissa, &input[size-4], 4);
//    mantissa = mantissa & (uint64_t)0x7FFFFF; // get 23-bit mantissa.
//    mantissa = mantissa << (52 - 23);  // scale for output mantissa.
//    break;
//  case 8:
//    // DOUBLE-PREC: 8 bytes comprising- sign bit, 11-bit exponent, 52-bit mantissa.
//    // Input and Output formats are the same so no translation is required.
//    MEMCOPY((uint8_t *)output, input, size);
//    return true;
//  default:
//    // Invalid floating point format.
//    assert(false);
//    return false;
//  }
//
//  // Convert to float double format and set output.
//  uint64_t tmp64;
//  tmp64 = ((uint64_t)sign << 63)
//            + ((uint64_t)exponent << 52)
//            + mantissa;
//  MEMCOPY((uint8_t *)output, (uint8_t *)&tmp64, sizeof(tmp64));
//
//  return true;
//}

static void restoreReportingConfigurationsFromNv(void)
{
  // Restores the contents of the nv reporting configuration table into the
  // corresponding entry in the ram-based configuration buffer list (Ram-based
  // configuration entries are created if they do not already exist).

  for (uint8_t i = 0; i < EMBER_ZCL_REPORTING_CONFIGURATIONS_TABLE_SIZE; i++) {
    EmZclNvReportingConfiguration_t nvConfiguration;
    retrieveNvConfigurationEntry(nvConfiguration, i);
    if (nvConfiguration.endpointId == EMBER_ZCL_ENDPOINT_NULL) {
      continue;
    }

    // Attempt to find a matching ram-based reporting configuration.
    Buffer buffer = NULL_BUFFER;
    Configuration_t *configuration
      = findReportingConfiguration(nvConfiguration.endpointId,
                                   &nvConfiguration.clusterSpec,
                                   nvConfiguration.reportingConfigurationId);
    if (configuration == NULL) {
      // Not found- create a new ram-based configuration.
      buffer = makeReportingConfiguration(nvConfiguration.endpointId,
                                          &nvConfiguration.clusterSpec);
      if (buffer == NULL_BUFFER) {
        assert(false);
        return;
      }
      configuration = (Configuration_t *)emGetBufferPointer(buffer);
    }

    // Update the ram-based configuration from nv.
    if (configuration->sizeAttrFlags
        != nvConfiguration.sizeAttrFlags) {
      assert(false);
      return;
    }
    if (configuration->sizeReportableChanges
        != nvConfiguration.sizeReportableChanges) {
      assert(false);
      return;
    }
    configuration->minimumIntervalS = nvConfiguration.minimumIntervalS;
    configuration->maximumIntervalS = nvConfiguration.maximumIntervalS;

    MEMCOPY(((uint8_t *)configuration + sizeof(Configuration_t)), //== location of ram-based attrFlags buffer.
            nvConfiguration.attrFlags,
            nvConfiguration.sizeAttrFlags);

    MEMCOPY(((uint8_t *)configuration + sizeof(Configuration_t)
             + configuration->sizeAttrFlags),                     //== location of ram-based reportableChanges buffer.
            nvConfiguration.reportableChanges,
            nvConfiguration.sizeReportableChanges);

    MEMCOPY(((uint8_t *)configuration + sizeof(Configuration_t) + configuration->sizeAttrFlags
             + configuration->sizeReportableChanges),             //== location of ram-based lowThresholds buffer.
            nvConfiguration.lowThresholds,
            nvConfiguration.sizeReportableChanges);               // lowThresholds' size is sizeReportableChanges

    MEMCOPY(((uint8_t *)configuration + sizeof(Configuration_t)  + configuration->sizeAttrFlags
             + 2 * configuration->sizeReportableChanges),         //== location of ram-based highThresholds buffer.
            nvConfiguration.highThresholds,
            nvConfiguration.sizeReportableChanges);               // highThresholds' size is sizeReportableChanges

    if (buffer != NULL_BUFFER) {
      // Add the restored ram-based configuration to the configurations buffer list.
      if (!addReportingConfiguration(buffer, false)) {         //nvUpdate=false
        assert(false);
        return;
      }
    }

    // Ensure the ram-based configuration id is the same as that in the nv.
    configuration->reportingConfigurationId = nvConfiguration.reportingConfigurationId;
  }
}

static bool updateNvReportingConfiguration(const Configuration_t * configuration,
                                           bool allowAdd)
{
  // Modifies (or creates) the specified nv reporting configuration entry
  // in the nv reporting configuration mirror table.

  uint8_t index = NV_CONFIG_TABLE_NULL_INDEX;
  EmZclNvReportingConfiguration_t nvConfiguration;

  // Search for the specified nv reporting configuration.
  for (uint8_t i = 0; i < EMBER_ZCL_REPORTING_CONFIGURATIONS_TABLE_SIZE; i++) {
    retrieveNvConfigurationEntry(nvConfiguration, i);
    if (matchReportingConfiguration(nvConfiguration.endpointId,
                                    &nvConfiguration.clusterSpec,
                                    nvConfiguration.reportingConfigurationId,
                                    configuration)) {
      index = i;
      break;
    } else if ((index == NV_CONFIG_TABLE_NULL_INDEX)
               && (nvConfiguration.endpointId == EMBER_ZCL_ENDPOINT_NULL)
               && (allowAdd)) {
      index = i;         // records the first unused nv config entry in the table.
    }
  }
  if (index >= NV_CONFIG_TABLE_NULL_INDEX) {
    // The entry was not found (or the nv config table is full).
    return false;
  }

  retrieveNvConfigurationEntry(nvConfiguration, index);

  // Modify the nv reporting configuration entry (update all fields as
  // this could be a new entry).
  nvConfiguration.endpointId = configuration->endpointId;
  nvConfiguration.clusterSpec = configuration->clusterSpec;
  nvConfiguration.reportingConfigurationId = configuration->reportingConfigurationId;
  nvConfiguration.minimumIntervalS = configuration->minimumIntervalS;
  nvConfiguration.maximumIntervalS = configuration->maximumIntervalS;
  if (configuration->sizeAttrFlags > EM_ZCL_MAX_SIZE_NV_REPORTABLE_ATTRIBUTES) {
    assert(false);         // attrFlags buffer in ram-based configuration is too big to store in nv structure!
    return false;
  }
  if (configuration->sizeReportableChanges > EM_ZCL_MAX_SIZE_NV_REPORTABLE_CHANGES) {
    assert(false);         // reportableChanges buffer in ram-based configuration is too big to store in nv structure!
    return false;
  }
  nvConfiguration.sizeAttrFlags = configuration->sizeAttrFlags;
  nvConfiguration.sizeReportableChanges = configuration->sizeReportableChanges;

  MEMCOPY(nvConfiguration.attrFlags,
          ((uint8_t *)configuration + sizeof(Configuration_t)),  //== location of ram-based attrFlags.
          nvConfiguration.sizeAttrFlags);

  MEMCOPY(nvConfiguration.reportableChanges,
          ((uint8_t *)configuration + sizeof(Configuration_t)
           + configuration->sizeAttrFlags),                     //== location of ram-based reportableChanges.
          nvConfiguration.sizeReportableChanges);

  MEMCOPY(nvConfiguration.lowThresholds,
          ((uint8_t *)configuration + sizeof(Configuration_t) + configuration->sizeAttrFlags
           + configuration->sizeReportableChanges),             //== location of ram-based lowThresholds.
          nvConfiguration.sizeReportableChanges);

  MEMCOPY(nvConfiguration.highThresholds,
          ((uint8_t *)configuration + sizeof(Configuration_t) + configuration->sizeAttrFlags
           + 2 * configuration->sizeReportableChanges),         //== location of ram-based highThresholds.
          nvConfiguration.sizeReportableChanges);

  // Save the modified nv configuration entry.
  saveNvConfigurationEntry(nvConfiguration, index);

  return true;
}

static void deleteAllNvReportingConfigurations(void)
{
  // Deletes all non-volatile reporting configurations.

  EmZclNvReportingConfiguration_t nvConfiguration;
  for (uint8_t i = 0; i < EMBER_ZCL_REPORTING_CONFIGURATIONS_TABLE_SIZE; i++) {
    retrieveNvConfigurationEntry(nvConfiguration, i);
    // Mark the nv reporting configuration entry as unused and save it.
    nvConfiguration.endpointId = EMBER_ZCL_ENDPOINT_NULL;
    saveNvConfigurationEntry(nvConfiguration, i);
  }
}

static void initialiseReportingConfigurations(void)
{
  // Restores ram-based configuration entries from nv.
  // Creates ram-based default reporting configurations for all ep/clusters
  // if they do not already exist (and saves them to nv).
  restoreReportingConfigurationsFromNv();

  // A default report configuration must exist for each ep/cluster combo so
  // we create new ram-based default configurations as necessary and save to nv.
  for (size_t i = 0; i < emZclEndpointCount; i++) {
    const EmZclEndpointEntry_t *endpoint = &emZclEndpointTable[i];
    for (size_t j = 0; endpoint->clusterSpecs[j] != NULL; j++) {
      const EmberZclClusterSpec_t *clusterSpec = endpoint->clusterSpecs[j];
      // Search for an existing matching ram-based default configuration.
      if (!findReportingConfiguration(endpoint->endpointId,
                                      clusterSpec,
                                      EMBER_ZCL_REPORTING_CONFIGURATION_DEFAULT)) {
        // Not found- make and add a ram-based default reporting configuration.
        Buffer buffer = makeReportingConfiguration(endpoint->endpointId,
                                                   clusterSpec);
        if ((buffer == NULL_BUFFER)
            || (!addReportingConfiguration(buffer, true))) {         // nvUpdate=true
          assert(false);
          return;
        }
      }
    }
  }
}

static void deleteConfigurationEntry(const Configuration_t * configuration,
                                     bool nvUpdate)
{
  // Deletes the specified configuration entry from the ram-based configurations
  // buffer list and the nv configuration mirror table.

  // Always delete the ram-based reporting configuration entry.
  Buffer tmp = configurations;
  configurations = NULL_BUFFER;
  while (!emBufferQueueIsEmpty(&tmp)) {
    Buffer finger = emBufferQueueRemoveHead(&tmp);
    const Configuration_t *fingee
      = (const Configuration_t *)emGetBufferPointer(finger);
    if (configuration != fingee) {
      emBufferQueueAdd(&configurations, finger);
    }
  }

  if (nvUpdate) {
    // Delete the corresponding nv reporting configuration entry.
    EmZclNvReportingConfiguration_t nvConfiguration;
    uint8_t i;
    for (i = 0; i < EMBER_ZCL_REPORTING_CONFIGURATIONS_TABLE_SIZE; i++) {
      retrieveNvConfigurationEntry(nvConfiguration, i);
      if (matchReportingConfiguration(nvConfiguration.endpointId,
                                      &nvConfiguration.clusterSpec,
                                      nvConfiguration.reportingConfigurationId,
                                      configuration)) {
        break;
      }
    }
    if (i < EMBER_ZCL_REPORTING_CONFIGURATIONS_TABLE_SIZE) {
      // Mark the nv reporting configuration entry as unused and save it.
      nvConfiguration.endpointId = EMBER_ZCL_ENDPOINT_NULL;
      saveNvConfigurationEntry(nvConfiguration, i);
    }
  }
}

// Public functions ------------------------------------------------------------

void emZclReportingMarkApplicationBuffersHandler(void)
{
  emMarkBuffer(&configurations);
}

void emZclReportingNetworkStatusHandler(EmberNetworkStatus newNetworkStatus,
                                        EmberNetworkStatus oldNetworkStatus,
                                        EmberJoinFailureReason reason)
{
  // If the device is no longer associated with a network, its reporting
  // configurations are removed.  If we end up joined and attached and we have
  // no configurations, make the set of default reporting configurations.  It
  // is possible there are no reportable attributes, in which case we'll waste
  // a bit of time trying to make the default reportable connection whenever
  // the node reattaches after losing its connection temporarily.
  switch (newNetworkStatus) {
    case EMBER_NO_NETWORK:
      // The buffer mgmt system resets buffers on a network down transition so
      // the following configuration clean-up must not use any ram-based
      // configuration pointers otherwise a buffer assert may result.
      configurations = NULL_BUFFER;
      deleteAllNvReportingConfigurations();
      cancelReportingEvents();         // Cancel any pending reporting events.
      break;
    case EMBER_JOINED_NETWORK_ATTACHED:
      if (configurations == NULL_BUFFER) {
        initialiseReportingConfigurations();
      }
      break;
    default:
      break;
  }
}

void emZclReportingPostAttributeChangeHandler(EmberZclEndpointId_t endpointId,
                                              const EmberZclClusterSpec_t * clusterSpec,
                                              EmberZclAttributeId_t attributeId,
                                              const void *buffer,
                                              size_t bufferLength)
{
  // Any change to an attribute forces a reexamination of the report timing.
  Buffer finger = emBufferQueueHead(&configurations);
  while (finger != NULL_BUFFER) {
    const Configuration_t *configuration
      = (const Configuration_t *)emGetBufferPointer(finger);
    if (endpointId == configuration->endpointId
        && emberZclAreClusterSpecsEqual(clusterSpec,
                                        &configuration->clusterSpec)) {
      uint32_t nowMs = halCommonGetInt32uMillisecondTick();
      uint32_t delayMs = UINT32_MAX;         // forever
      uint32_t elapsedMs = elapsedTimeInt32u(configuration->lastTimeMs, nowMs);
      if (hasReportableChangesOrThresholdCrossings(configuration)) {
        delayMs = (elapsedMs < S_TO_MS(configuration->minimumIntervalS)
                   ? S_TO_MS(configuration->minimumIntervalS) - elapsedMs
                   : 0);
      } else if (hasPeriodicReporting(configuration)) {
        delayMs = (elapsedMs < S_TO_MS(configuration->maximumIntervalS)
                   ? S_TO_MS(configuration->maximumIntervalS) - elapsedMs
                   : 0);
      }
      scheduleMakeNotification(configuration, delayMs);
    }
    finger = emBufferQueueNext(&configurations, finger);
  }
}

bool emZclHasReportingConfiguration(EmberZclEndpointId_t endpointId,
                                    const EmberZclClusterSpec_t * clusterSpec,
                                    EmberZclReportingConfigurationId_t reportingConfigurationId)
{
  return (findReportingConfiguration(endpointId,
                                     clusterSpec,
                                     reportingConfigurationId)
          != NULL);
}

// zcl/e/XX/<cluster>/r:
//   GET: return list of reporting configuration ids.
//   POST: add reporting configuration.
//   OTHER: not allowed.
void emZclUriClusterReportingConfigurationHandler(EmZclContext_t * context)
{
  if (context->groupId != EMBER_ZCL_GROUP_NULL) {
    emZclRespond404NotFound(context->info);
    return;
  }
  switch (context->code) {
    case EMBER_COAP_CODE_GET:
      getReportingConfigurationIdsHandler(context);
      break;
    case EMBER_COAP_CODE_POST:
      addReportingConfigurationHandler(context);
      break;
    default:
      assert(false);
      break;
  }
}

// zcl/e/XX/<cluster>/r/XX:
//   GET: return reporting configuration.
//   PUT: replace reporting configuration.
//   DELETE: remove reporting configuration.
//   OTHER: not allowed.
void emZclUriClusterReportingConfigurationIdHandler(EmZclContext_t * context)
{
  if (context->groupId != EMBER_ZCL_GROUP_NULL) {
    emZclRespond404NotFound(context->info);
    return;
  }
  switch (context->code) {
    case EMBER_COAP_CODE_GET:
      getReportingConfigurationHandler(context);
      break;
    case EMBER_COAP_CODE_PUT:
      updateReportingConfigurationHandler(context);
      break;
    case EMBER_COAP_CODE_DELETE:
      removeReportingConfigurationHandler(context);
      break;
    default:
      assert(false);
      break;
  }
}

void emberZclReportingConfigurationsFactoryReset(EmberZclEndpointId_t endpointId)
{
  // Loops thru all ram-based configurations and deletes any configurations
  // (including default reporting configurations) with the specified endpointId.
  // Default reporting configurations are recreated with their initial values.

  if (configurations != NULL_BUFFER) {
    Buffer finger = emBufferQueueHead(&configurations);
    while (finger != NULL_BUFFER) {
      Configuration_t *configuration
        = (Configuration_t *)emGetBufferPointer(finger);
      if (configuration->endpointId == endpointId) {
        deleteReportingConfiguration(configuration);         // deletes ram-based and nv config entries for ep.
        finger = emBufferQueueHead(&configurations);
      } else {
        finger = emBufferQueueNext(&configurations, finger);
      }
    }

    // Call init to recreate default reporting configs for the endpoint.
    initialiseReportingConfigurations();
  }
}
