/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2003-2004, Apple Computer, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1.  Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of its
 *     contributors may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef DNSSD_IPC_H
#define DNSSD_IPC_H

#include "dns_sd.h"
#include "mdns_porting.h"
//
// Common cross platform services
//
#if defined(WIN32)
#   include <winsock2.h>
#   define dnssd_InvalidSocket  INVALID_SOCKET
#   define dnssd_SocketValid(s) ((s) != INVALID_SOCKET)
#   define dnssd_EWOULDBLOCK    WSAEWOULDBLOCK
#   define dnssd_EINTR          WSAEINTR
#   define dnssd_ECONNRESET     WSAECONNRESET
#   define dnssd_sock_t         SOCKET
#   define dnssd_socklen_t      int
#   define dnssd_close(sock)    closesocket(sock)
#   define dnssd_errno          WSAGetLastError()
#   define dnssd_strerror(X)    win32_strerror(X)
#   define ssize_t              int
//FIXED #   define getpid               _getpid
//FIXED#   define unlink               _unlink
extern char *win32_strerror(int inErrorCode);
#else
//#   include <sys/types.h>
//#   include <unistd.h>
//#   include <sys/un.h>
#   include <string.h>
#   include <stdio.h>
#   include <stdlib.h>
//#   include <sys/stat.h>
#   include <lwip/sockets.h>
//#   include <sys/socket.h>
//#   include <netinet/in.h>
//#   include <arpa/inet.h>
#   define dnssd_InvalidSocket  -1
#   define dnssd_SocketValid(s) ((s) >= 0)
#   define dnssd_EWOULDBLOCK    EWOULDBLOCK
#   define dnssd_EINTR          EINTR
#   define dnssd_ECONNRESET     ECONNRESET
#   define dnssd_EPIPE          EPIPE
#   define dnssd_sock_t         int
#   define dnssd_socklen_t      unsigned int
#   define dnssd_close(sock)    close(sock)
#   define dnssd_errno          errno
#   define dnssd_strerror(X)    strerror(X)
#endif

#if defined(USE_TCP_LOOPBACK)
#   define AF_DNSSD             AF_INET
#   define MDNS_TCP_SERVERADDR  "127.0.0.1"
#   define MDNS_TCP_SERVERPORT  5354
#   define LISTENQ              5
#   define dnssd_sockaddr_t     struct sockaddr_in
#else
#   define AF_DNSSD             AF_LOCAL
#   ifndef MDNS_UDS_SERVERPATH
#       define MDNS_UDS_SERVERPATH  "/var/run/mDNSResponder"
#   endif
#   define LISTENQ              100
// longest legal control path length
#   define MAX_CTLPATH          256
#   define dnssd_sockaddr_t     struct sockaddr_un
#endif

// Compatibility workaround
#ifndef AF_LOCAL
#define AF_LOCAL    AF_UNIX
#endif

// General UDS constants
#define TXT_RECORD_INDEX ((uint32_t)(-1))   // record index for default text record

// IPC data encoding constants and types
#define VERSION 1
#define IPC_FLAGS_NOREPLY 1 // set flag if no asynchronous replies are to be sent to client

// Structure packing macro. If we're not using GNUC, it's not fatal. Most compilers naturally pack the on-the-wire
// structures correctly anyway, so a plain "struct" is usually fine. In the event that structures are not packed
// correctly, our compile-time assertion checks will catch it and prevent inadvertent generation of non-working code.
#ifndef packedstruct
 #if ((__GNUC__ > 2) || ((__GNUC__ == 2) && (__GNUC_MINOR__ >= 9)))
  #define packedstruct struct __attribute__((__packed__))
  #define packedunion  union  __attribute__((__packed__))
 #else
  #define packedstruct struct
  #define packedunion  union
 #endif
#endif

typedef enum
{
    request_op_none = 0,    // No request yet received on this connection
    connection_request = 1, // connected socket via DNSServiceConnect()
    reg_record_request,     // reg/remove record only valid for connected sockets
    remove_record_request,
    enumeration_request,
    reg_service_request,
    browse_request,
    resolve_request,
    query_request,
    reconfirm_record_request,
    add_record_request,
    update_record_request,
    setdomain_request,      // Up to here is in Tiger and B4W 1.0.3
    getproperty_request,    // New in B4W 1.0.4
    port_mapping_request,   // New in Leopard and B4W 2.0
    addrinfo_request,
    send_bpf,               // New in SL
    getpid_request,
    release_request,
    connection_delegate_request,

    cancel_request = 63
} request_op_t;

typedef enum
{
    enumeration_reply_op = 64,
    reg_service_reply_op,
    browse_reply_op,
    resolve_reply_op,
    query_reply_op,
    reg_record_reply_op,    // Up to here is in Tiger and B4W 1.0.3
    getproperty_reply_op,   // New in B4W 1.0.4
    port_mapping_reply_op,  // New in Leopard and B4W 2.0
    addrinfo_reply_op
} reply_op_t;

#if defined(_WIN64)
#   pragma pack(push,4)
#endif

// Define context object big enough to hold a 64-bit pointer,
// to accomodate 64-bit clients communicating with 32-bit daemon.
// There's no reason for the daemon to ever be a 64-bit process, but its clients might be
typedef packedunion
{
    void *context;
    uint32_t u32[2];
} client_context_t;

typedef packedstruct
{
    uint32_t version;
    uint32_t datalen;
    uint32_t ipc_flags;
    uint32_t op;        // request_op_t or reply_op_t
    client_context_t client_context; // context passed from client, returned by server in corresponding reply
    uint32_t reg_index;            // identifier for a record registered via DNSServiceRegisterRecord() on a
    // socket connected by DNSServiceCreateConnection().  Must be unique in the scope of the connection, such that and
    // index/socket pair uniquely identifies a record.  (Used to select records for removal by DNSServiceRemoveRecord())
} ipc_msg_hdr;

#if defined(_WIN64)
#   pragma pack(pop)
#endif

// routines to write to and extract data from message buffers.
// caller responsible for bounds checking.
// ptr is the address of the pointer to the start of the field.
// it is advanced to point to the next field, or the end of the message

void put_uint32(const uint32_t l, char **ptr);
uint32_t get_uint32(const char **ptr, const char *end);

void put_uint16(uint16_t s, char **ptr);
uint16_t get_uint16(const char **ptr, const char *end);

#define put_flags put_uint32
#define get_flags get_uint32

#define put_error_code put_uint32
#define get_error_code get_uint32

int put_string(const char *str, char **ptr);
int get_string(const char **ptr, const char *const end, char *buffer, int buflen);

void put_rdata(const int rdlen, const unsigned char *rdata, char **ptr);
const char *get_rdata(const char **ptr, const char *end, int rdlen);  // return value is rdata pointed to by *ptr -
// rdata is not copied from buffer.

void ConvertHeaderBytes(ipc_msg_hdr *hdr);

struct CompileTimeAssertionChecks_dnssd_ipc
{
    // Check that the compiler generated our on-the-wire packet format structure definitions
    // properly packed, without adding padding bytes to align fields on 32-bit or 64-bit boundaries.
    char assert0[(sizeof(client_context_t) ==  8) ? 1 : -1];
    char assert1[(sizeof(ipc_msg_hdr)      == 28) ? 1 : -1];
};

#endif // DNSSD_IPC_H
