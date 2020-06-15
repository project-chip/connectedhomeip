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
 * @brief This code simulates an ISA backchannel serial interface for a
 *   PC EZSP Host application.
 *   It creates a socket to listen for TCP connections so that scripts
 *   can treat a gateway application just like any other Insight adapter.
 *******************************************************************************
   ******************************************************************************/

#include PLATFORM_HEADER //compiler/micro specifics, types

#include "stack/include/ember-types.h"
#include "stack/include/error.h"

#include "hal/hal.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>   // for errno
#include <unistd.h>  // for close()

#include "app/framework/plugin-host/gateway/gateway-support.h"

//------------------------------------------------------------------------------
// GLOBALS

const bool backchannelSupported = true;

// The serial port number (0 or 1) plus this offset gives the port that the
// socket server will listen on. By default (to match the InSight Adapter
// behavior):
//    * serial port 0 -> server port 4900
//    * serial port 1 -> server port 4901
int backchannelSerialPortOffset = EMBER_AF_PLUGIN_GATEWAY_TCP_PORT_OFFSET;

#define SERVER_PORT_OFFSET (backchannelSerialPortOffset)

#define INVALID_FD -1

static int socketFd[] = { INVALID_FD, INVALID_FD };
static int clientFd[] = { INVALID_FD, INVALID_FD };

struct sockaddr_in clientConnections[2];

static bool debugOn = false;
static const char debugString[] = "backchannel";

// These will change
static int LOCAL_STDIN = 0;
static int LOCAL_STDOUT = 1;
static int LOCAL_STDERR = 2;

#define REMOTE_STDIN  0
#define REMOTE_STDOUT 1
#define REMOTE_STDERR 2

#define MAX_STRING_LENGTH 250

// Turned on via command-line options
bool backchannelEnable = false;

static FILE* clientOut[] = { NULL, NULL };

//------------------------------------------------------------------------------
// Forward Declarations

static int willConnectionBlock(bool server, uint8_t port, bool read);

#define willClientConnectionBlock(port) \
  willConnectionBlock(false, (port), false)
#define willServerConnectionBlock(port) \
  willConnectionBlock(true, (port), true)

static bool getNewConnection(uint8_t port);
static void unixError(const char* format, ...);
static void debugPrint(const char* formatString, ...);
static void infoPrint(const char* formatString, ...);
static int myPrintf(int fd, const char* formatString, ...);
static int myVprintf(int fd, const char* formatString, va_list ap);

//------------------------------------------------------------------------------
// API

EmberStatus backchannelStartServer(uint8_t port)
{
  struct sockaddr_in serverAddress;
  int flags;

  if (!backchannelEnable) {
    return EMBER_INVALID_CALL;
  }

  if (port > 1) {
    return EMBER_SERIAL_INVALID_PORT;
  }

  if (socketFd[port] != -1) {
    return EMBER_INVALID_CALL;
  }

  socketFd[port] = socket(AF_INET, SOCK_STREAM, 0);
  if (socketFd[port] < 0) {
    unixError("Error: Could not open socket");
    return EMBER_ERR_FATAL;
  }
  flags = 1; // Enable SO_REUSEADDR to reduce bind() complaints
  (void) setsockopt(socketFd[port], SOL_SOCKET, SO_REUSEADDR, &flags,
                    sizeof(flags));

  bzero(&(clientConnections[port]), sizeof(struct sockaddr_in));

  bzero((char *) &serverAddress, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_addr.s_addr = INADDR_ANY;
  serverAddress.sin_port = htons(SERVER_PORT_OFFSET + port);
  if (bind(socketFd[port],
           (struct sockaddr *) &serverAddress,
           sizeof(serverAddress)) < 0) {
    unixError("Error: Could not bind socket to %u", SERVER_PORT_OFFSET + port);
    return EMBER_ERR_FATAL;
  }
  if (0 > listen(socketFd[port], 0)) {
    unixError("Error: Could not mark socket as listening");
    return EMBER_ERR_FATAL;
  }
  infoPrint("Listening for connections on port %u", SERVER_PORT_OFFSET + port);

  return EMBER_SUCCESS;
}

EmberStatus backchannelClientConnectionCleanup(uint8_t port)
{
  if (!backchannelEnable) {
    return EMBER_INVALID_CALL;
  }
  if (port > 1) {
    return EMBER_SERIAL_INVALID_PORT;
  }
  if (clientFd[port] != INVALID_FD) {
    myPrintf(REMOTE_STDOUT,
             "Server closing client connection from %s:%u on port %u\n",
             inet_ntoa(clientConnections[port].sin_addr),
             ntohs(clientConnections[port].sin_port),
             SERVER_PORT_OFFSET + port);
    infoPrint("Server closing client connection from %s:%u on port %u\n",
              inet_ntoa(clientConnections[port].sin_addr),
              ntohs(clientConnections[port].sin_port),
              SERVER_PORT_OFFSET + port);
    close(clientFd[port]);
    clientFd[port] = INVALID_FD;
    bzero(&(clientConnections[port]), sizeof(struct sockaddr_in));
  }
  return EMBER_SUCCESS;
}

EmberStatus backchannelStopServer(uint8_t port)
{
  if (!backchannelEnable) {
    return EMBER_INVALID_CALL;
  }
  if (port > 1) {
    return EMBER_SERIAL_INVALID_PORT;
  }

  backchannelClientConnectionCleanup(port);

  if (socketFd[port] != INVALID_FD) {
    myPrintf(LOCAL_STDOUT, "Server closing socket connection %u\n",
             SERVER_PORT_OFFSET + port);
    close(socketFd[port]);
    socketFd[port] = INVALID_FD;
  }
  return EMBER_SUCCESS;
}

// Retrieves a single byte from the client connection.  Returns the number
// of bytes read, or -1 on error.  If no client connection currently exists,
// then it will block until one is established.  If an error is returned then
// it means an attempt was made to establish one but it failed.
EmberStatus backchannelReceive(uint8_t port, char* data)
{
  if (!backchannelEnable) {
    return EMBER_INVALID_CALL;
  }
  if (port > 1 || clientFd[port] == INVALID_FD) {
    return EMBER_SERIAL_INVALID_PORT;
  }

  int result = recv(clientFd[port], data, 1, 0);
  if (result == 1) {
    return EMBER_SUCCESS;
  } else if (result == 0 || errno != EAGAIN) {
    backchannelCloseConnection(port);
    return EMBER_ERR_FATAL;
  }

  return EMBER_ERR_FATAL;
}

EmberStatus backchannelSend(uint8_t port, uint8_t * data, uint8_t length)
{
  if (!backchannelEnable) {
    return EMBER_INVALID_CALL;
  }
  if (port > 1) {
    return EMBER_SERIAL_INVALID_PORT;
  }
  if (INVALID_FD != clientFd[port]
      && !willClientConnectionBlock(port)) {
    if (0 > send(clientFd[port], data, length, 0)) {
      backchannelCloseConnection(port);
      return EMBER_ERR_FATAL;
    }
    return EMBER_SUCCESS;
  }
  return EMBER_INVALID_CALL;
}

// Checks on the state of the current backchannel connection.
// If one doesn't exist, it can wait for a new connection and return
// the result.
BackchannelState backchannelCheckConnection(uint8_t port,
                                            bool waitForConnection)
{
  if (!backchannelEnable) {
    return CONNECTION_ERROR;
  } else if (clientFd[port] >= 0) {
    return CONNECTION_EXISTS;
  }

  bool getConnection = false;

  if (waitForConnection) {
    infoPrint("Waiting for client connection on port %u",
              port + SERVER_PORT_OFFSET);
    getConnection = true;
  } else if (!willServerConnectionBlock(port)) {
    getConnection = true;
  }

  if (getConnection) {
    return (getNewConnection(port)
            ? NEW_CONNECTION
            : CONNECTION_ERROR);
  }
  return NO_CONNECTION;
}

EmberStatus backchannelServerPrintf(const char* formatString, ...)
{
  int stat;
  va_list ap = { 0 };
  va_start(ap, formatString);
  stat = myVprintf(LOCAL_STDOUT, formatString, ap);
  va_end(ap);
  return (stat == 0 ? EMBER_SUCCESS : EMBER_ERR_FATAL);
}

EmberStatus backchannelClientPrintf(uint8_t port, const char* formatString, ...)
{
  EmberStatus status;
  va_list ap = { 0 };
  va_start(ap, formatString);
  status = backchannelClientVprintf(port, formatString, ap);
  va_end(ap);
  return status;
}

EmberStatus backchannelClientVprintf(uint8_t port,
                                     const char* formatString,
                                     va_list ap)
{
  if (clientFd[port] != INVALID_FD) {
    int stat;
    vfprintf(clientOut[port], formatString, ap);
    stat = fflush(clientOut[port]);
    if (stat != 0) {
      backchannelCloseConnection(port);
      return EMBER_ERR_FATAL;
    }
    return EMBER_SUCCESS;
  }
  //  debugPrint("Serial port %d not valid!\n", port);
  return EMBER_SERIAL_INVALID_PORT;
}

// Re-map STDIN, STDOUT, and STDERR to the client connection.
// This allows the software to use normal read() and write() calls to
// to receive and send data to the remote client.
EmberStatus backchannelMapStandardInputOutputToRemoteConnection(int port)
{
  FILE** streams[] = { &stdin, &stdout, &stderr };
  uint8_t i;

  int connectionFd;
  if (!backchannelEnable) {
    return EMBER_INVALID_CALL;
  }
  if (port > 1) {
    return EMBER_SERIAL_INVALID_PORT;
  }
  connectionFd = clientFd[port];

  if (INVALID_FD == connectionFd) {
    return EMBER_ERR_FATAL;
  }

  int* localFds[] = { &LOCAL_STDIN, &LOCAL_STDOUT, &LOCAL_STDERR };

  for (i = 0; i < 3; i++) {
    // First re-map FD's 0,1,2 to a new local FD.
    *localFds[i] = dup(i);
    if (*localFds[i] < 0) {
      unixError("Could not dup() %d", i);
      close(connectionFd);
      clientFd[port] = INVALID_FD;
      return EMBER_ERR_FATAL;
    }

    // Now re-map the remotely connected FD to 0,1,2
    int newFd = dup2(connectionFd, i);
    if (newFd < 0) {
      unixError("Could not dup2() %d", connectionFd);
      close(connectionFd);
      clientFd[port] = INVALID_FD;
      return EMBER_ERR_FATAL;
    }
    // Redirect the FILE* globals (stdin, stdout, stderr) to the new socket
    // We don't perform an fclose() first because that will also close the FD,
    // which will prevent this fdopen() call from working.
    *(streams[i]) = fdopen(i, (i == 0 ? "r" : "a"));
    if (*(streams[i]) == NULL) {
      unixError("Could not fdopen() %d", i);
      close(connectionFd);
      clientFd[port] = INVALID_FD;
      return EMBER_ERR_FATAL;
    }
  }
  return EMBER_SUCCESS;
}

EmberStatus backchannelCloseConnection(uint8_t port)
{
  EmberStatus status = EMBER_SUCCESS;
  if (INVALID_FD == clientFd[port]) {
    return EMBER_ERR_FATAL;
  }
  if (0 > close(clientFd[port])) {
    debugPrint("Error: Could not close socket: %s\n", strerror(errno));
    status = EMBER_ERR_FATAL;
  } else {
    infoPrint("Closed connection on port %d.", port);
  }
  clientFd[port] = -1;
  clientOut[port] = NULL;
  return status;
}

//------------------------------------------------------------------------------
// Internal Functions

// Returns 0 if a read/write will not block, 1 if it will, and -1 on error.
static int willConnectionBlock(bool server, uint8_t port, bool read)
{
  fd_set readSet;
  int fd = (server ? socketFd[port] : clientFd[port]);

  if (fd < 0) { // BugzId:12928
    return -1;  // No connection - return error
  }

  FD_ZERO(&readSet);
  FD_SET(fd, &readSet);

  fd_set writeSet;
  FD_ZERO(&writeSet);
  FD_SET(fd, &writeSet);

  struct timeval noWaiting = {
    0,   // seconds
    0,   // milliseconds
  };

  int status = select(fd + 1,                  // highest FD + 1 (per select())
                      read ? &readSet : NULL,  // read FD set
                      read ? NULL : &writeSet, // write FD set
                      NULL,                    // exception FD set
                      &noWaiting);             // timeout

  if (status < 0) {
    // Theoretically select() can return prematurely due to a signal
    // and give an error of EINTR.  Because we do not wait,
    // we assume this is unlikely.
    unixError("select() Failed!");
    return -1;
  }

  // Invert the return code.  If select() saw the state of an FD change
  // before the timeout, it will return 1.  Otherwise it will return 0.
  // So if an FD is ready, we say it won't block (0).
  return (status ? 0 : 1);
}

// Accepts a new TCP connection.  Returns the state of the connection.
static bool getNewConnection(uint8_t port)
{
  socklen_t clientLength;
  struct sockaddr_in clientAddress;

  clientLength = sizeof(clientAddress);
  clientFd[port] = accept(socketFd[port],
                          (struct sockaddr *) &clientAddress,
                          &clientLength);
  debugPrint("New Client FD for port %d is %d", port, clientFd[port]);
  if (clientFd[port] >= 0) {
    // Code for making the port non-blocking
    //      flags = fcntl(clientFd[port], F_GETFL);
    //      if (fcntl(clientFd[port], F_SETFL, flags | O_NONBLOCK) < 0) {
    //        unixError("Error: Could not make connection non-blocking");
    //        backchannelCloseConnection(port);
    //        return NO_CONNECTION;
    //      }

    clientOut[port] = fdopen(clientFd[port], "a");
    if (clientOut[port] == NULL) {
      unixError("Error: Could not open stream to client connection.");
      backchannelCloseConnection(port);
      return false;
    }

    memcpy(&(clientConnections[port]), &clientAddress, sizeof(struct sockaddr_in));
    infoPrint("New connection from client %s:%u on port %u",
              inet_ntoa(clientConnections[port].sin_addr),
              ntohs(clientConnections[port].sin_port),
              SERVER_PORT_OFFSET + port);
    return true;
  }
  return false;
}

//------------------------------------------------------------------------------
// Internal Printing routines

// Print an error message plus the associated 'errno' string
static void unixError(const char* format, ...)
{
  va_list ap = { 0 };
  va_start(ap, format);
  myVprintf(LOCAL_STDERR, format, ap);
  va_end(ap);
  myPrintf(LOCAL_STDERR, ": %s\n", strerror(errno));
}

static void debugPrint(const char* formatString, ...)
{
  if (debugOn) {
    va_list ap = { 0 };
    myPrintf(LOCAL_STDERR, "[%s] ", debugString);
    va_start(ap, formatString);
    myVprintf(LOCAL_STDERR, formatString, ap);
    va_end(ap);
    myPrintf(LOCAL_STDERR, "\n");
  }
}

static void infoPrint(const char* formatString, ...)
{
  va_list ap = { 0 };
  va_start(ap, formatString);
  myVprintf(LOCAL_STDOUT, formatString, ap);
  va_end(ap);
  myPrintf(LOCAL_STDOUT, "\n");
}

// Because we mess around with FDs, we need our own printing routine
// for handling where messages are printed to.

static int myPrintf(int fd, const char* formatString, ...)
{
  va_list ap = { 0 };
  int returnCode;
  va_start(ap, formatString);
  returnCode = myVprintf(fd, formatString, ap);
  va_end(ap);
  return returnCode;
}

// Returns 0 on success, 1 on error
static int myVprintf(int fd, const char* formatString, va_list ap)
{
  int length;
  char string[MAX_STRING_LENGTH];
  length = vsnprintf(string, MAX_STRING_LENGTH - 1, formatString, ap);
  string[length] = '\0';
  return (length != write(fd, string, length));
}
