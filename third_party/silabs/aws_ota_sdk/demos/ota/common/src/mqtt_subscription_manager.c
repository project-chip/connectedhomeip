/*
 * AWS IoT Device SDK for Embedded C 202211.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * @file mqtt_subscription_manager.c
 * @brief Implementation of the API of a subscription manager for handling subscription callbacks
 * to topic filters in MQTT operations.
 */

/* Standard includes. */
#include <string.h>
#include <assert.h>

/* Include demo config. */
#include "demo_config.h"

/* Include header for the subscription manager. */
#include "mqtt_subscription_manager.h"

#include <stdbool.h>

#include "silabs_utils.h"
/**
 * @brief Represents a registered record of the topic filter and its associated callback
 * in the subscription manager registry.
 */
typedef struct SubscriptionManagerRecord
{
    const char * pTopicFilter;
    uint16_t topicFilterLength;
    SubscriptionManagerCallback_t callback;
} SubscriptionManagerRecord_t;

/**
 * @brief The default value for the maximum size of the callback registry in the
 * subscription manager.
 */
#ifndef MAX_SUBSCRIPTION_CALLBACK_RECORDS
    #define MAX_SUBSCRIPTION_CALLBACK_RECORDS    5
#endif

/**
 * @brief The registry to store records of topic filters and their subscription callbacks.
 */
static SubscriptionManagerRecord_t callbackRecordList[ MAX_SUBSCRIPTION_CALLBACK_RECORDS ] = { 0 };

/*-----------------------------------------------------------*/
static bool matchEndWildcardsSpecialCases( const char * pTopicFilter,
                                           uint16_t topicFilterLength,
                                           uint16_t filterIndex )
{
    bool matchFound = false;

    assert( pTopicFilter != NULL );
    assert( topicFilterLength != 0U );

    /* Check if the topic filter has 2 remaining characters and it ends in
     * "/#". This check handles the case to match filter "sport/#" with topic
     * "sport". The reason is that the '#' wildcard represents the parent and
     * any number of child levels in the topic name.*/
    if( ( topicFilterLength >= 3U ) &&
        ( filterIndex == ( topicFilterLength - 3U ) ) &&
        ( pTopicFilter[ filterIndex + 1U ] == '/' ) &&
        ( pTopicFilter[ filterIndex + 2U ] == '#' ) )

    {
        matchFound = true;
    }

    /* Check if the next character is "#" or "+" and the topic filter ends in
     * "/#" or "/+". This check handles the cases to match:
     *
     * - Topic filter "sport/+" with topic "sport/".
     * - Topic filter "sport/#" with topic "sport/".
     */
    if( ( filterIndex == ( topicFilterLength - 2U ) ) &&
        ( pTopicFilter[ filterIndex ] == '/' ) )
    {
        /* Check that the last character is a wildcard. */
        matchFound = ( pTopicFilter[ filterIndex + 1U ] == '+' ) ||
                     ( pTopicFilter[ filterIndex + 1U ] == '#' );
    }

    return matchFound;
}

static bool matchWildcards( const char * pTopicName,
                            uint16_t topicNameLength,
                            const char * pTopicFilter,
                            uint16_t topicFilterLength,
                            uint16_t * pNameIndex,
                            uint16_t * pFilterIndex,
                            bool * pMatch )
{
    bool shouldStopMatching = false;
    bool locationIsValidForWildcard;

    assert( pTopicName != NULL );
    assert( topicNameLength != 0U );
    assert( pTopicFilter != NULL );
    assert( topicFilterLength != 0U );
    assert( pNameIndex != NULL );
    assert( pFilterIndex != NULL );
    assert( pMatch != NULL );

    /* Wild card in a topic filter is only valid either at the starting position
     * or when it is preceded by a '/'.*/
    locationIsValidForWildcard = ( *pFilterIndex == 0u ) ||
                                 ( pTopicFilter[ *pFilterIndex - 1U ] == '/' );

    if( ( pTopicFilter[ *pFilterIndex ] == '+' ) && ( locationIsValidForWildcard == true ) )
    {
        bool nextLevelExistsInTopicName = false;
        bool nextLevelExistsinTopicFilter = false;

        /* Move topic name index to the end of the current level. The end of the
         * current level is identified by the last character before the next level
         * separator '/'. */
        while( *pNameIndex < topicNameLength )
        {
            /* Exit the loop if we hit the level separator. */
            if( pTopicName[ *pNameIndex ] == '/' )
            {
                nextLevelExistsInTopicName = true;
                break;
            }

            ( *pNameIndex )++;
        }

        /* Determine if the topic filter contains a child level after the current level
         * represented by the '+' wildcard. */
        if( ( *pFilterIndex < ( topicFilterLength - 1U ) ) &&
            ( pTopicFilter[ *pFilterIndex + 1U ] == '/' ) )
        {
            nextLevelExistsinTopicFilter = true;
        }

        /* If the topic name contains a child level but the topic filter ends at
         * the current level, then there does not exist a match. */
        if( ( nextLevelExistsInTopicName == true ) &&
            ( nextLevelExistsinTopicFilter == false ) )
        {
            *pMatch = false;
            shouldStopMatching = true;
        }

        /* If the topic name and topic filter have child levels, then advance the
         * filter index to the level separator in the topic filter, so that match
         * can be performed in the next level.
         * Note: The name index already points to the level separator in the topic
         * name. */
        else if( nextLevelExistsInTopicName == true )
        {
            ( *pFilterIndex )++;
        }
        else
        {
            /* If we have reached here, the the loop terminated on the
             * ( *pNameIndex < topicNameLength) condition, which means that have
             * reached past the end of the topic name, and thus, we decrement the
             * index to the last character in the topic name.*/
            ( *pNameIndex )--;
        }
    }

    /* '#' matches everything remaining in the topic name. It must be the
     * last character in a topic filter. */
    else if( ( pTopicFilter[ *pFilterIndex ] == '#' ) &&
             ( *pFilterIndex == ( topicFilterLength - 1U ) ) &&
             ( locationIsValidForWildcard == true ) )
    {
        /* Subsequent characters don't need to be checked for the
         * multi-level wildcard. */
        *pMatch = true;
        shouldStopMatching = true;
    }
    else
    {
        /* Any character mismatch other than '+' or '#' means the topic
         * name does not match the topic filter. */
        *pMatch = false;
        shouldStopMatching = true;
    }

    return shouldStopMatching;
}

static bool matchTopicFilter( const char * pTopicName,
                              uint16_t topicNameLength,
                              const char * pTopicFilter,
                              uint16_t topicFilterLength )
{
    bool matchFound = false, shouldStopMatching = false;
    uint16_t nameIndex = 0, filterIndex = 0;

    assert( pTopicName != NULL );
    assert( topicNameLength != 0 );
    assert( pTopicFilter != NULL );
    assert( topicFilterLength != 0 );

    // SILABS_LOG("matchTopicFilter topic name [%s]", pTopicName);
    SILABS_LOG("matchTopicFilter topic filter [%s]", pTopicFilter);

    while( ( nameIndex < topicNameLength ) && ( filterIndex < topicFilterLength ) )
    {
        /* Check if the character in the topic name matches the corresponding
         * character in the topic filter string. */
        if( pTopicName[ nameIndex ] == pTopicFilter[ filterIndex ] )
        {
            /* If the topic name has been consumed but the topic filter has not
             * been consumed, match for special cases when the topic filter ends
             * with wildcard character. */
            if( nameIndex == ( topicNameLength - 1U ) )
            {
                matchFound = matchEndWildcardsSpecialCases( pTopicFilter,
                                                            topicFilterLength,
                                                            filterIndex );
            }
            // SILABS_LOG("matchTopicFilter matchFound %d", matchFound);
        }
        else
        {
            /* Check for matching wildcards. */
            shouldStopMatching = matchWildcards( pTopicName,
                                                 topicNameLength,
                                                 pTopicFilter,
                                                 topicFilterLength,
                                                 &nameIndex,
                                                 &filterIndex,
                                                 &matchFound );
            // SILABS_LOG("matchTopicFilter shouldStopMatching %d", shouldStopMatching);
        }

        if( ( matchFound == true ) || ( shouldStopMatching == true ) )
        {
            break;
        }

        /* Increment indexes. */
        nameIndex++;
        filterIndex++;
    }

    if( matchFound == false )
    {
        /* If the end of both strings has been reached, they match. This represents the
         * case when the topic filter contains the '+' wildcard at a non-starting position.
         * For example, when matching either of "sport/+/player" OR "sport/hockey/+" topic
         * filters with "sport/hockey/player" topic name. */
        matchFound = ( nameIndex == topicNameLength ) &&
                     ( filterIndex == topicFilterLength );
        SILABS_LOG("matchTopicFilter nameI %d TNL %d FilI %d TFL %d", nameIndex, topicNameLength, filterIndex, topicFilterLength);
        SILABS_LOG("matchTopicFilter matchFound false case %d", matchFound);
    }

    return matchFound;
}

int MQTT_MatchTopic( const char * pTopicName,
                              const uint16_t topicNameLength,
                              const char * pTopicFilter,
                              const uint16_t topicFilterLength,
                              bool * pIsMatch )
{
    int status = 0;
    bool topicFilterStartsWithWildcard = false;
    bool matchStatus = false;

    if( ( pTopicName == NULL ) || ( topicNameLength == 0u ) )
    {
        SILABS_LOG( "Invalid paramater: Topic name should be non-NULL and its "
                    "length should be > 0: TopicName=%p, TopicNameLength=%hu",
                    ( void * ) pTopicName,
                    ( unsigned short ) topicNameLength );

        status = -1;
    }
    else if( ( pTopicFilter == NULL ) || ( topicFilterLength == 0u ) )
    {
        SILABS_LOG("Invalid paramater: Topic filter should be non-NULL and "
                    "its length should be > 0: TopicName=%p, TopicFilterLength=%hu",
                    ( void * ) pTopicFilter,
                    ( unsigned short ) topicFilterLength );
        status = -1;
    }
    else if( pIsMatch == NULL )
    {
        SILABS_LOG("Invalid paramater: Output parameter, pIsMatch, is NULL" );
        status = -1;
    }
    else
    {
        /* Check for an exact match if the incoming topic name and the registered
         * topic filter length match. */
        if( topicNameLength == topicFilterLength )
        {
            matchStatus = strncmp( pTopicName, pTopicFilter, topicNameLength ) == 0;
        }

        if( matchStatus == false )
        {
            /* If an exact match was not found, match against wildcard characters in
             * topic filter.*/

            /* Determine if topic filter starts with a wildcard. */
            topicFilterStartsWithWildcard = ( pTopicFilter[ 0 ] == '+' ) ||
                                            ( pTopicFilter[ 0 ] == '#' );

            /* Note: According to the MQTT 3.1.1 specification, incoming PUBLISH topic names
             * starting with "$" character cannot be matched against topic filter starting with
             * a wildcard, i.e. for example, "$SYS/sport" cannot be matched with "#" or
             * "+/sport" topic filters. */
            if( !( ( pTopicName[ 0 ] == '$' ) && ( topicFilterStartsWithWildcard == true ) ) )
            {
                matchStatus = matchTopicFilter( pTopicName, topicNameLength, pTopicFilter, topicFilterLength );
            }
        }

        /* Update the output parameter with the match result. */
        *pIsMatch = matchStatus;
        SILABS_LOG("mqtt_TopicMatch %d", matchStatus);
    }

    return status;
}

void SubscriptionManager_DispatchHandler( const char * pTopicFilter,
                                          uint16_t topicFilterLength, const void *pPayload, uint16_t payloadLength )
{
    bool matchStatus = false;
    size_t listIndex = 0u;

    assert( pTopicFilter != NULL );
    assert( topicFilterLength != 0 );
    assert( callbackRecordList != 0 );
    SILABS_LOG("DispatchHandler topic length %d payloadLength %d", topicFilterLength, payloadLength);
    /* Iterate through record list to find matching topics, and invoke their callbacks. */
    for( listIndex = 0; listIndex < MAX_SUBSCRIPTION_CALLBACK_RECORDS; listIndex++ )
    {
        // SILABS_LOG("dispatch handler CBTopicLength=%d", callbackRecordList[ listIndex ].topicFilterLength);
        if( ( callbackRecordList[ listIndex ].pTopicFilter != NULL ) &&
            ( MQTT_MatchTopic( pTopicFilter,
                               topicFilterLength,
                               callbackRecordList[ listIndex ].pTopicFilter,
                               callbackRecordList[ listIndex ].topicFilterLength,
                               &matchStatus ) == 0 ) &&
            ( matchStatus == true ) )
        {
            SILABS_LOG("Invoking subscription callback of matching topic filter: "
                       "TopicFilter=%.*s",
                       callbackRecordList[ listIndex ].topicFilterLength,
                       callbackRecordList[ listIndex ].pTopicFilter );

            /* Invoke the callback associated with the record as the topics match. */
            callbackRecordList[ listIndex ].callback(  pTopicFilter, topicFilterLength , pPayload, payloadLength);
        }
    }
}

/*-----------------------------------------------------------*/

SubscriptionManagerStatus_t SubscriptionManager_RegisterCallback( const char * pTopicFilter,
                                                                  uint16_t topicFilterLength,
                                                                  SubscriptionManagerCallback_t callback )
{
    assert( pTopicFilter != NULL );
    assert( topicFilterLength != 0 );
    assert( callback != NULL );

    SubscriptionManagerStatus_t returnStatus;
    size_t availableIndex = MAX_SUBSCRIPTION_CALLBACK_RECORDS;
    bool recordExists = false;
    size_t index = 0u;

    /* Search for the first available spot in the list to store the record, and also check if
     * a record for the topic filter already exists. */
    SILABS_LOG("==============subscription manager==================");
    while( ( recordExists == false ) && ( index < MAX_SUBSCRIPTION_CALLBACK_RECORDS ) )
    {
        /* Check if the index represents an empty spot in the registry. If we had already
         * found an empty spot in the list, we will not update it. */
        if( ( availableIndex == MAX_SUBSCRIPTION_CALLBACK_RECORDS ) &&
            ( callbackRecordList[ index ].pTopicFilter == NULL ) )
        {
            availableIndex = index;
        }

        /* Check if the current record's topic filter in the registry matches the topic filter
         * we are trying to register. */
        else if( ( callbackRecordList[ index ].topicFilterLength == topicFilterLength ) &&
                 ( strncmp( pTopicFilter, callbackRecordList[ index ].pTopicFilter, topicFilterLength )
                   == 0 ) )
        {
            recordExists = true;
        }

        index++;
    }

    if( recordExists == true )
    {
        /* The record for the topic filter already exists. */
        SILABS_LOG("Failed to register callback: Record for topic filter already exists: TopicFilter");

        returnStatus = SUBSCRIPTION_MANAGER_RECORD_EXISTS;
    }
    else if( availableIndex == MAX_SUBSCRIPTION_CALLBACK_RECORDS )
    {
        /* The registry is full. */
        SILABS_LOG( "Unable to register callback: Registry list is full: TopicFilter" );

        returnStatus = SUBSCRIPTION_MANAGER_REGISTRY_FULL;
    }
    else
    {
        callbackRecordList[ availableIndex ].pTopicFilter = pTopicFilter;
        callbackRecordList[ availableIndex ].topicFilterLength = topicFilterLength;
        callbackRecordList[ availableIndex ].callback = callback;

        returnStatus = SUBSCRIPTION_MANAGER_SUCCESS;

        SILABS_LOG("Added callback to registry: TopicFilter %s availableIndex %ld", pTopicFilter, availableIndex);
    }

    return returnStatus;
}

/*-----------------------------------------------------------*/

// void SubscriptionManager_RemoveCallback( const char * pTopicFilter,
//                                          uint16_t topicFilterLength )
// {
//     assert( pTopicFilter != NULL );
//     assert( topicFilterLength != 0 );

//     size_t index;
//     SubscriptionManagerRecord_t * pRecord = NULL;

//     /* Iterate through the records list to find the matching record. */
//     for( index = 0; index < MAX_SUBSCRIPTION_CALLBACK_RECORDS; index++ )
//     {
//         pRecord = &callbackRecordList[ index ];

//         /* Only match the non-empty records. */
//         if( pRecord->pTopicFilter != NULL )
//         {
//             if( ( topicFilterLength == pRecord->topicFilterLength ) &&
//                 ( strncmp( pTopicFilter, pRecord->pTopicFilter, topicFilterLength ) == 0 ) )
//             {
//                 break;
//             }
//         }
//     }

//     /* Delete the record by clearing the found entry in the records list. */
//     if( index < MAX_SUBSCRIPTION_CALLBACK_RECORDS )
//     {
//         pRecord->pTopicFilter = NULL;
//         pRecord->topicFilterLength = 0u;
//         pRecord->callback = NULL;

//         LogDebug( ( "Deleted callback record for topic filter: TopicFilter=%.*s",
//                     topicFilterLength,
//                     pTopicFilter ) );
//     }
//     else
//     {
//         LogWarn( ( "Attempted to remove callback for un-registered topic filter: TopicFilter=%.*s",
//                    topicFilterLength,
//                    pTopicFilter ) );
//     }
// }
/*-----------------------------------------------------------*/