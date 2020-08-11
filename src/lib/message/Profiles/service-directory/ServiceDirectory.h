/*
 *
 *    Copyright (c) 2019-2020 Google LLC.
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *      Declarations for the CHIP Service Directory Profile and the
 *      corresponding protocol. See ServiceDirectory.cpp for more
 *      details and the document:
 *
 *          Nest CHIP - Service Directory Protocol
 *
 *      for a protocol description.
 */

#ifndef _WSD_PROFILE_H
#define _WSD_PROFILE_H

#include <core/CHIPCore.h>
#include <message/CHIPMessageLayer.h>
#include <support/DLLUtil.h>
#include <support/ErrorStr.h>
#include <support/logging/CHIPLogging.h>

#if CHIP_CONFIG_ENABLE_SERVICE_DIRECTORY

/**
 *   @namespace chip::Profiles::ServiceDirectory
 *
 *   @brief
 *     This namespace includes all interfaces within CHIP for the
 *     CHIP Service Directory profile, which includes the
 *     corresponding, protocol of the same name.
 */

namespace chip {
namespace Profiles {
namespace ServiceDirectory {

struct ServiceConnectBeginArgs;

/**
 *  CHIP message types used in this profile
 *
 */
enum
{
    kMsgType_ServiceEndpointQuery    = 0x00, ///< Service Endpoint Query message type
    kMsgType_ServiceEndpointResponse = 0x01  ///< Service Endpoint Response message type
};

enum
{
    kConnectRequestPoolSize = 4, ///< the number of simultaneous connect requests
};

/**
 *  Masks for the control byte of the service endpoint response frame
 *
 */
enum
{
    kMask_DirectoryLen       = 0x0F, ///< Length of the directory
    kMask_Redirect           = 0x10, ///< Redirect flag
    kMask_SuffixTablePresent = 0x20, ///< Suffix table present flag
    kMask_TimeFieldsPresent  = 0x40, ///< Time fields present flag
};

/**
 *  Masks and values for the control byte of the directory
 *  list field of the service endpoint response frame.
 *
 */
enum
{
    kMask_HostPortListLen            = 0x07, ///< Length of the host/port list
    kMask_DirectoryEntryType         = 0xC0, ///< Entry Type
    kDirectoryEntryType_SingleNode   = 0x00, ///< A zero value means this entry is a node ID
    kDirectoryEntryType_HostPortList = 0x40, ///< This entry is a list of host/port pairs
};

/**
 *  Masks and values for the control byte in each host/port
 *  list item
 *
 */
enum
{
    kMask_HostIdType           = 0x03, ///< The type of host ID
    kHostIdType_FullyQualified = 0x00, ///< The host ID is all there
    kHostIdType_Composite      = 0x01, ///< The host ID needs to be matched with a suffix
    kMask_SuffixIndexPresent   = 0x04, ///< A suffix index is present
    kMask_PortIdPresent        = 0x08  ///< A port ID is present
};

/**
 *  Status code
 *
 */
enum
{
    kStatus_DirectoryUnavailable = 0x0051 ///< Directory is not available
};

/**
 *  Manager states
 *
 */
enum
{
    kServiceMgrState_Initial   = 0,
    kServiceMgrState_Resolving = 1,
    kServiceMgrState_Waiting   = 2,
    kServiceMgrState_Resolved  = 3
};

#if CHIP_CONFIG_PERSIST_SERVICE_DIRECTORY
enum
{
    /** Version number associated with persisted service directory data
     *
     * By convention, the version number of persisted service directory
     * data is the same as the message type number of the SD response
     * message.  This reflects the fact that the persisted data is an
     * exact copy of the payload of that message.
     */
    kPersistedServiceDirVersion = kMsgType_ServiceEndpointResponse
};

namespace Platform {
extern CHIP_ERROR LoadPersistentServiceDir(uint8_t * buf, uint16_t bufsize, uint16_t & len, uint8_t version);
extern CHIP_ERROR StorePersistentServiceDir(uint8_t * buf, uint16_t len, uint8_t version);
extern CHIP_ERROR ClearPersistentServiceDir();
extern bool IsPersistentServiceDirPresent(uint8_t version);
} // namespace Platform
#endif // CHIP_CONFIG_PERSIST_SERVICE_DIRECTORY

#define kServiceEndpoint_Directory (0x18B4300200000001ull)           ///< Directory profile endpoint
#define kServiceEndpoint_SoftwareUpdate (0x18B4300200000002ull)      ///< Software update profile endpoint
#define kServiceEndpoint_Data_Management (0x18B4300200000003ull)     ///< Core CHIP data management protocol endpoint
#define kServiceEndpoint_Log_Upload (0x18B4300200000004ull)          ///< Bulk data transfer profile endpoint for log uploads
#define kServiceEndpoint_TimeService (0x18B4300200000005ull)         ///< Time service endpoint
#define kServiceEndpoint_ServiceProvisioning (0x18B4300200000010ull) ///< Service provisioning profile endpoint
#define kServiceEndpoint_ChipTunneling (0x18B4300200000011ull)       ///< CHIP tunneling endpoint
#define kServiceEndpoint_CoreRouter (0x18B4300200000012ull)          ///< Core router endpoint
#define kServiceEndpoint_FileDownload (0x18B4300200000013ull)        ///< File download profile endpoint
#define kServiceEndpoint_Bastion (0x18B4300200000014ull)             ///< Nest Bastion service endpoint
#define kServiceEndpoint_DeviceOperationalCA                                                                                       \
    (0x18B4300200000016ull) ///< Nest device operational certification authority service endpoint

/**
 * @class ChipServiceManager
 *
 * @brief The manager object for the CHIP service directory.
 *
 * The CHIP service manager is the main interface for
 * applications to the directory service. As such, it hides the
 * complications inherent in looking up the directory entry
 * associated with a service endpoint, doing DNS lookup on one or
 * more of the host names found there, attempting to connect,
 * securing the connection and so on. It may also manage a cache
 * of service directory information.
 */
class DLL_EXPORT ChipServiceManager
{
public:
    /**
     * @typedef RootDirectoryAccessor
     *
     * @brief An accessor function for the root directory info.
     *
     * You gotta start somewhere and with the service directory
     * you gotta start with a stub directory that contains the
     * address of a server you can hit to get at everything
     * else. Since the disposition and provenance of this
     * information is likely to vary from device to device, we
     * provide an accessor callback here.
     *
     * @param [out] aDirectory  A pointer to a buffer to write
     *   the directory information.
     *
     * @param [in]  aLength     The length of the given buffer in bytes.
     *
     * @return #CHIP_NO_ERROR on success, otherwise the loading process
     *   would be aborted.
     */
    typedef CHIP_ERROR (*RootDirectoryAccessor)(uint8_t * aDirectory, uint16_t aLength);

    /**
     * @typedef StatusHandler
     *
     * @brief A handler for error and status conditions.
     *
     * A user of the service manager may be informed of problems in trying
     * to execute a connect request in one of two ways. It may receive a
     * status report from the service or it may receive an internally
     * generated CHIP_ERROR. In either case, the information comes through
     * this callback.
     *
     * @param [in] anAppState A pointer to an application object that was
     *   passed in to the corresponding conect() call.
     *
     * @param [in] anError An CHIP error code indicating error happened
     *   in the process of trying to execute the connect request. This
     *   shall be #CHIP_NO_ERROR in the case where no error arose and a
     *   status report is available.
     *
     * @param [in] aStatusReport A pointer to a status report generated
     *   by the remote directory service. This argument shall be NULL in
     *   the case where there was no status report and an internal error
     *   is passed in the previous argument.
     *
     */
    typedef void (*StatusHandler)(void * anAppState, CHIP_ERROR anError, StatusReport * aStatusReport);

    /**
     * @typedef OnServiceEndpointQueryEndWithTimeInfo
     *
     * @brief An application callback to deliver time values from a service
     *   directory response.
     *
     * This is called when we get time information from service directory
     * query response Note this callback would only happen if a response is
     * successfully parsed and time information is included
     *
     * @param [in]  timeQueryReceiptMsec    The number of msec since POSIX epoch,
     *   when the query was received at server side.
     *
     * @param [in]  timeProcessMsec         The number of msec spent on processing
     *   this query.
     */
    typedef void (*OnServiceEndpointQueryEndWithTimeInfo)(uint64_t timeQueryReceiptMsec, uint32_t timeProcessMsec);

    /**
     * @typedef OnServiceEndpointQueryBegin
     *
     * @brief An application callback to mark the time of an outgoing service
     *   directory query.
     *
     * This is called when we are about to send out service endpoint query
     * request. This is used to match with OnServiceEndpointQueryEnd to
     * compensate for message flight time.
     *
     */
    typedef void (*OnServiceEndpointQueryBegin)(void);

    /**
     * @typedef OnConnectBegin
     *
     * @brief An application callback made immediately prior to connection establishment.
     *
     * This callback can be used by applications to observe and optionally alter the
     * arguments passed to #ChipConnection::Connect() during the course of establishing
     * a service connection.  This callback will be called both for the connection to the
     * target service endpoint, as well as the connection to the Service Directory endpoint
     * in the event that a directory lookup must be performed.
     */
    typedef void (*OnConnectBegin)(struct ServiceConnectBeginArgs & args);

    ChipServiceManager(void);
    ~ChipServiceManager(void);

    CHIP_ERROR init(ChipExchangeManager * aExchangeMgr, uint8_t * aCache, uint16_t aCacheLen, RootDirectoryAccessor aAccessor,
                    ChipAuthMode aDirAuthMode                                                  = kChipAuthMode_Unauthenticated,
                    OnServiceEndpointQueryBegin aServiceEndpointQueryBegin                     = NULL,
                    OnServiceEndpointQueryEndWithTimeInfo aServiceEndpointQueryEndWithTimeInfo = NULL,
                    OnConnectBegin aConnectBegin                                               = NULL);

    CHIP_ERROR connect(uint64_t aServiceEp, ChipAuthMode aAuthMode, void * aAppState, StatusHandler aStatusHandler,
                       ChipConnection::ConnectionCompleteFunct aConnectionCompleteHandler, const uint32_t aConnectTimeoutMsecs = 0,
                       const InterfaceId aConnectIntf = INET_NULL_INTERFACEID);

    CHIP_ERROR lookup(uint64_t aServiceEp, HostPortList * outHostPortList);
    CHIP_ERROR lookup(uint64_t aServiceEp, uint8_t * aControlByte, uint8_t ** aDirectoryEntry);

    CHIP_ERROR replaceOrAddCacheEntry(uint16_t port, const char * hostName, uint8_t hostLen, uint64_t serviceEndpointId);

    void cancel(uint64_t aServiceEp, void * aAppState);

    void unresolve(CHIP_ERROR aError);
    void unresolve(void);

    void reset(CHIP_ERROR aError);
    void reset(void);

    void relocate(CHIP_ERROR aError);
    void relocate(void);

    // Handler methods for the query/response transaction

    void onConnectionComplete(CHIP_ERROR aError);
    void onConnectionClosed(CHIP_ERROR aError);
    void onResponseReceived(uint32_t aProfileId, uint8_t aMsgType, PacketBuffer * aMsg);
    void onResponseTimeout(void);

    // Clear cache and reset state so that the next connect request
    // goes to the service directory endpoint first
    void clearCache(void);

    void SetConnectBeginCallback(OnConnectBegin aConnectBegin);

    enum
    {
        /**
         *  @brief
         *    Number of milliseconds a response must be received for the
         *    directory query before the exchange context times out.
         */
        kChip_DefaultSendTimeout = 15000
    };

    /**
     *  @class ConnectRequest
     *
     *  @brief This class represents a single transaction managed by the service manager.
     */
    class ConnectRequest
    {
    public:
        CHIP_ERROR init(ChipServiceManager * aManager, const uint64_t & aServiceEp, ChipAuthMode aAuthMode, void * aAppState,
                        StatusHandler aStatusHandler, ChipConnection::ConnectionCompleteFunct aCompleteHandler,
                        const uint32_t aConnectTimeoutMsecs, const InterfaceId aConnIntf);

        void free(void);

        void finalize(void);

        /**
         *  This function tests if this connect request is currently in use to
         *  connect to a particular service endpoint for a particualr
         *  application entity.
         *
         *  @param[in] aServiceEp   A service endpoint ID to be compared with
         *    what this connect request holds.
         *
         *  @param[in] aAppState    A pointer to application state, which is
         *    used to compare with what this connect request holds.
         *
         *  @return true if the test passes, false otherwise.
         */
        inline bool isAllocatedTo(const uint64_t & aServiceEp, void * aAppState)
        {
            return (mServiceEp == aServiceEp && mAppState == aAppState);
        }

        /**
         *  This function tests if the connect request is not currently
         *  allocated.
         *
         *  @return true if the test passes, false otherwise.
         */
        inline bool isFree(void) { return (mServiceEp == 0 && !mAppState); }

        void onConnectionComplete(CHIP_ERROR aError);

        // data members (basically the connect call arguments)

        uint64_t mServiceEp;
        ChipAuthMode mAuthMode;
        void * mAppState;

        /// A connection to stash here while it's awaiting completion.
        ChipConnection * mConnection;

        uint32_t mConnectTimeoutMsecs; ///< the timeout for the Connect call to succeed or return an error.
        InterfaceId mConnIntf;         ///< the interface over which the connection is to be set up.

        /// A pointer to a function which would be called when a status report is
        /// received.
        StatusHandler mStatusHandler;

        /// A pointer to a function which would be called when a connection to
        /// destination service endpoint has been completed.
        ChipConnection::ConnectionCompleteFunct mConnectionCompleteHandler;
    };

private:
    struct Extent
    {
        uint8_t * base;
        size_t length;
    };

    void freeConnectRequests(void);
    void finalizeConnectRequests(void);
    ConnectRequest * getAvailableRequest(void);

    CHIP_ERROR lookupAndConnect(ChipConnection * aConnection, uint64_t aServiceEp, ChipAuthMode aAuthMode, void * aAppState,
                                ChipConnection::ConnectionCompleteFunct aHandler, const uint32_t aConnectTimeoutMsecs = 0,
                                const InterfaceId aConnectIntf = INET_NULL_INTERFACEID);

#if CHIP_CONFIG_PERSIST_SERVICE_DIRECTORY
    CHIP_ERROR loadPersistentServiceDirIntoCache();
#endif
    CHIP_ERROR unpackPacketBuffer(PacketBuffer * aMsg, bool useTimePresent, bool * redir = NULL);
    CHIP_ERROR cacheDirectory(MessageIterator &, uint8_t, uint8_t *&);
    CHIP_ERROR cacheSuffixes(MessageIterator &, uint8_t, uint8_t *&);
    CHIP_ERROR calculateEntryLength(uint8_t * entryStart, uint8_t entryCtrlByte, uint16_t * entryLen);
    /*
     *  A group of methods that clear up working state and free
     *  resources - generally in the case of a failure. one of
     *  them calls the error handler and the other calls the
     *  status handler.
     *
     */

    void fail(CHIP_ERROR aError);
    void transactionsErrorOut(CHIP_ERROR);
    void transactionsReportStatus(StatusReport & aReport);
    void cleanupExchangeContext(void);
    void cleanupExchangeContext(CHIP_ERROR aErr);
    void clearWorkingState(void);

    /**
     *  This method clears the cache state of the manager including the
     *  "relocated" flag.
     */
    inline void clearCacheState(void)
    {
        mCacheState   = kServiceMgrState_Initial;
        mWasRelocated = false;
    }

    CHIP_ERROR handleTimeInfo(MessageIterator & itMsg);

    // data members

    ConnectRequest mConnectRequestPool[kConnectRequestPoolSize];
    ChipExchangeManager * mExchangeManager; ///< the exchange manager to use for everything
    ChipConnection * mConnection;           ///< a connection to stash here while it's awaiting completion
    ExchangeContext * mExchangeContext;     ///< the exchange context specifically for directory profile exchanges
    RootDirectoryAccessor mAccessor;        ///< how to get at the root directory
    Extent mDirectory;                      ///< the working directory
    Extent mSuffixTable;                    ///< the (optional) suffix table
    Extent mCache;                          ///< all of this stuff needs to be cached somewhere in memory
    uint8_t mCacheState;                    ///< and the state of the cache | initial, resolving, resolved |
    bool mWasRelocated;                     ///< true iff the service manager has been relocated once.
    ChipAuthMode mDirAuthMode;              ///< the authentication mode to use when talking to the directory service.
    uint32_t mDirAndSuffTableSize;          ///< the size of the directory and suffix table  in the cache.

    /**
     *  Callback happens right before we send out the service endpoint query request
     */
    OnServiceEndpointQueryBegin mServiceEndpointQueryBegin;

    /**
     *  Callback happens right after we receive a service endpoint query response with time
     *  information
     */
    OnServiceEndpointQueryEndWithTimeInfo mServiceEndpointQueryEndWithTimeInfo;

    /**
     * Callback happens immediately prior to connection establishment.
     */
    OnConnectBegin mConnectBegin;
};

/**
 * Arguments passed to the ChipServiceManager::OnConnectBegin callback.
 */
struct ServiceConnectBeginArgs
{
    ServiceConnectBeginArgs(uint64_t inServiceEndpoint, ChipConnection * inConnection, HostPortList * inEndpointHostPortList,
                            InterfaceId inConnectIntf, ChipAuthMode inAuthMode, uint8_t inDNSOptions);
    /** The service endpoint to which the connect is being established. */
    const uint64_t ServiceEndpoint;

    /** The #ChipConnection object that will be used to establish the connection. */
    ChipConnection * const Connection;

    /** A HostPortList object containing the hostname and port information for connection. */
    HostPortList * const EndpointHostPortList;

    /** The network interface over which the connection should be established. */
    InterfaceId ConnectIntf;

    /** The desired authentication mode for the connection. */
    ChipAuthMode AuthMode;

    /** A set of options controlling how hostname resolution is performed. */
    uint8_t DNSOptions;
};

/**
 * Set a callback function to be called immediately prior to connection establishment.
 *
 *  @param [in] aConnectBegin       A pointer to the callback function.  A value of NULL
 *                                  disables the callback.
 */
inline void ChipServiceManager::SetConnectBeginCallback(OnConnectBegin aConnectBegin)
{
    mConnectBegin = aConnectBegin;
}

}; // namespace ServiceDirectory
}; // namespace Profiles
}; // namespace chip
}
; // nl

#endif //  CHIP_CONFIG_ENABLE_SERVICE_DIRECTORY
#endif // _WSD_PROFILE_H
