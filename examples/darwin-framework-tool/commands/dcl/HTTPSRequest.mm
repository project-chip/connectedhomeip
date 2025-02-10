/*
 *   Copyright (c) 2025 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include <commands/dcl/HTTPSRequest.h>

#include <crypto/CryptoBuildConfig.h>
#include <lib/support/Base64.h>
#include <lib/support/SafeInt.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/logging/CHIPLogging.h>
#include <system/SystemError.h>

#include <CommonCrypto/CommonDigest.h>
#include <Network/Network.h>

#include <unistd.h>

namespace {
constexpr const char * kHttpsPrefix = "https://";
constexpr uint16_t kHttpsPort = 443;
constexpr const char * kErrorJsonParse = "Failed to parse JSON: ";
constexpr const char * kErrorHTTPSPrefix = "URL must start with 'https://': ";
constexpr const char * kErrorHTTPSPort = "Invalid port: 0";
constexpr const char * kErrorHTTPSHostName = "Invalid hostname: empty";
constexpr const char * kErrorBase64Decode = "Error while decoding base64 data";
constexpr const char * kErrorSizeMismatch = "The response size does not match the expected size: ";
} // namespace

namespace chip {
namespace tool {
    namespace https {
        namespace {
            constexpr uint16_t kResponseBufferSize = 4096;
            constexpr uint64_t kConnectionTimeoutSeconds = 10;
            constexpr uint64_t kSendTimeoutSeconds = 10;
            constexpr uint64_t kReceiveTimeoutSeconds = 10;
            constexpr const char * kDispatchQueueName = "com.chip.httpsrequest";
            constexpr const char * kContentContextName = "httpsrequest";
            constexpr const char * kErrorSendHTTPRequest = "Failed to send HTTP request";
            constexpr const char * kErrorSendHTTPRequestTimeout = "Failed to send HTTP request: timeout";
            constexpr const char * kErrorReceiveHTTPResponse = "Failed to read HTTP response";
            constexpr const char * kErrorReceiveHTTPResponseTimeout = "Failed to read HTTP response: timeout";
            constexpr const char * kErrorConnection = "Failed to connect to: ";
            constexpr const char * kErrorConnectionTimeout = "Timeout connecting to: ";
            constexpr const char * kErrorConnectionUnknowState = "Unknown connection state";
            constexpr const char * kErrorDigestMismatch = "The response digest does not match the expected digest";
            class HTTPSSessionHolder {
            public:
                HTTPSSessionHolder() {};

                ~HTTPSSessionHolder()
                {
                    VerifyOrReturn(nullptr != mConnection);
                    nw_connection_cancel(mConnection);
                    mConnection = nullptr;
                }

                CHIP_ERROR Init(std::string & hostname, uint16_t port)
                {
                    __auto_type semaphore = dispatch_semaphore_create(0);
                    __block CHIP_ERROR result = CHIP_NO_ERROR;

                    __auto_type endpoint = nw_endpoint_create_host(hostname.c_str(), std::to_string(port).c_str());
                    nw_parameters_t parameters = nw_parameters_create_secure_tcp(NW_PARAMETERS_DEFAULT_CONFIGURATION, NW_PARAMETERS_DEFAULT_CONFIGURATION);

                    mConnection = nw_connection_create(endpoint, parameters);
                    VerifyOrReturnError(nullptr != mConnection, CHIP_ERROR_INTERNAL);

                    nw_connection_set_state_changed_handler(mConnection, ^(nw_connection_state_t state, nw_error_t error) {
                        switch (state) {
                        case nw_connection_state_waiting:
                        case nw_connection_state_preparing:
                            break;
                        case nw_connection_state_ready:
                            result = CHIP_NO_ERROR;
                            dispatch_semaphore_signal(semaphore);
                            break;
                        case nw_connection_state_failed:
                        case nw_connection_state_cancelled:
                            ChipLogError(chipTool, "%s%s", kErrorConnection, hostname.c_str());
                            result = CHIP_ERROR_NOT_CONNECTED;
                            dispatch_semaphore_signal(semaphore);
                        default:
                            ChipLogError(chipTool, "%s", kErrorConnectionUnknowState);
                            break;
                        }
                    });

                    __auto_type queue = dispatch_queue_create(kDispatchQueueName, DISPATCH_QUEUE_SERIAL);
                    nw_connection_set_queue(mConnection, queue);
                    nw_connection_start(mConnection);

                    if (dispatch_semaphore_wait(semaphore, dispatch_time(DISPATCH_TIME_NOW, kConnectionTimeoutSeconds * NSEC_PER_SEC)) != 0) {
                        ChipLogError(chipTool, "%s%s", kErrorConnectionTimeout, hostname.c_str());
                        return CHIP_ERROR_TIMEOUT;
                    }

                    nw_connection_set_state_changed_handler(mConnection, nullptr);
                    return result;
                }

                CHIP_ERROR SendRequest(std::string & request)
                {
                    __auto_type semaphore = dispatch_semaphore_create(0);
                    __block CHIP_ERROR result = CHIP_NO_ERROR;

                    __auto_type context = nw_content_context_create(kContentContextName);
                    __auto_type data = dispatch_data_create(request.c_str(), request.size(), dispatch_get_main_queue(), DISPATCH_DATA_DESTRUCTOR_DEFAULT);
                    nw_connection_send(mConnection, data, context, true, ^(nw_error_t error) {
                        if (nullptr != error) {
                            ChipLogError(chipTool, "%s", kErrorSendHTTPRequest);
                            result = CHIP_ERROR_BAD_REQUEST;
                        }
                        dispatch_semaphore_signal(semaphore);
                    });

                    if (dispatch_semaphore_wait(semaphore, dispatch_time(DISPATCH_TIME_NOW, kSendTimeoutSeconds * NSEC_PER_SEC)) != 0) {
                        ChipLogError(chipTool, "%s", kErrorSendHTTPRequestTimeout);
                        return CHIP_ERROR_TIMEOUT;
                    }

                    return result;
                }

                CHIP_ERROR ReceiveResponse(std::string & response)
                {
                    __auto_type semaphore = dispatch_semaphore_create(0);
                    __block CHIP_ERROR result = CHIP_NO_ERROR;
                    __block std::string receivedData;

                    nw_connection_receive(mConnection, 1, kResponseBufferSize, ^(dispatch_data_t content, nw_content_context_t context, bool isComplete, nw_error_t error) {
                        if (nullptr != error) {
                            ChipLogError(chipTool, "%s", kErrorReceiveHTTPResponse);
                            result = CHIP_ERROR_INTERNAL;
                        } else if (nullptr != content) {
                            size_t total_size = dispatch_data_get_size(content);
                            receivedData.reserve(total_size);

                            dispatch_data_apply(content, ^(dispatch_data_t region, size_t offset, const void * buffer, size_t size) {
                                receivedData.append(static_cast<const char *>(buffer), size);
                                return true;
                            });

                            result = CHIP_NO_ERROR;
                        }

                        dispatch_semaphore_signal(semaphore);
                    });

                    if (dispatch_semaphore_wait(semaphore, dispatch_time(DISPATCH_TIME_NOW, kReceiveTimeoutSeconds * NSEC_PER_SEC)) != 0) {
                        ChipLogError(chipTool, "%s", kErrorReceiveHTTPResponseTimeout);
                        return CHIP_ERROR_TIMEOUT;
                    }

                    response = receivedData;
                    return result;
                }

            private:
                nw_connection_t mConnection = nullptr;
            };

            std::string BuildRequest(std::string & hostname, std::string & path)
            {
                return "GET " + path + " HTTP/1.1\r\n" + //
                    "Host: " + hostname + "\r\n" + //
                    "Accept: application/json\r\n" + //
                    "Connection: close\r\n\r\n"; //
            }

            CHIP_ERROR RemoveHeader(std::string & response)
            {
                size_t headerEnd = response.find("\r\n\r\n");
                VerifyOrReturnError(std::string::npos != headerEnd, CHIP_ERROR_INVALID_ARGUMENT);

                auto body = response.substr(headerEnd + 4);
                response = body;

                return CHIP_NO_ERROR;
            }

            CHIP_ERROR MaybeCheckResponseSize(const std::string & response, const chip::Optional<uint32_t> & optionalExpectedSize)
            {
                VerifyOrReturnError(optionalExpectedSize.HasValue(), CHIP_NO_ERROR);
                VerifyOrReturnError(chip::CanCastTo<uint32_t>(response.size()), CHIP_ERROR_INVALID_ARGUMENT);

                uint32_t responseSize = static_cast<uint32_t>(response.size());
                uint32_t expectedSize = optionalExpectedSize.Value();
                VerifyOrReturnError(expectedSize == responseSize, CHIP_ERROR_INVALID_ARGUMENT,
                    ChipLogError(chipTool, "%s%u != %u", kErrorSizeMismatch, responseSize, expectedSize));
                return CHIP_NO_ERROR;
            }

            CHIP_ERROR MaybeCheckResponseDigest(const std::string & response, const chip::Optional<const char *> & optionalExpectedDigest)
            {
                VerifyOrReturnError(optionalExpectedDigest.HasValue(), CHIP_NO_ERROR);
                VerifyOrReturnError(CanCastTo<uint16_t>(strlen(optionalExpectedDigest.Value())), CHIP_ERROR_INVALID_ARGUMENT);

                const char * encodedData = optionalExpectedDigest.Value();
                uint16_t encodedDataSize = static_cast<uint16_t>(strlen(encodedData));

                size_t expectedMaxDecodedSize = BASE64_MAX_DECODED_LEN(encodedDataSize);
                chip::Platform::ScopedMemoryBuffer<uint8_t> decodedData;
                VerifyOrReturnError(decodedData.Calloc(expectedMaxDecodedSize + 1 /* for null */), CHIP_ERROR_INVALID_ARGUMENT);

                size_t decodedDataSize = chip::Base64Decode(encodedData, encodedDataSize, decodedData.Get());
                VerifyOrReturnError(0 != decodedDataSize, CHIP_ERROR_INVALID_ARGUMENT, ChipLogError(chipTool, "%s", kErrorBase64Decode));

                // Compute the SHA-256 hash of the response
                unsigned char responseDigest[CC_SHA256_DIGEST_LENGTH];
                CC_SHA256(reinterpret_cast<const unsigned char *>(response.c_str()), static_cast<CC_LONG>(response.size()), responseDigest);

                VerifyOrReturnError(memcmp(responseDigest, decodedData.Get(), CC_SHA256_DIGEST_LENGTH) == 0, CHIP_ERROR_INVALID_ARGUMENT,
                    ChipLogError(chipTool, "%s", kErrorDigestMismatch));

                return CHIP_NO_ERROR;
            }

            CHIP_ERROR ConvertResponseToJSON(std::string & body, Json::Value & jsonResponse)
            {

                Json::CharReaderBuilder readerBuilder;
                std::string errors;
                std::istringstream jsonStream(body);
                bool success = Json::parseFromStream(readerBuilder, jsonStream, &jsonResponse, &errors);
                VerifyOrReturnError(success, CHIP_ERROR_INTERNAL, ChipLogError(chipTool, "%s%s", kErrorJsonParse, errors.c_str()));

                return CHIP_NO_ERROR;
            }

            CHIP_ERROR ExtractHostAndPath(const std::string & url, std::string & hostAndPort, std::string & outPath)
            {
                VerifyOrReturnError(url.compare(0, strlen(kHttpsPrefix), kHttpsPrefix) == 0, CHIP_ERROR_INVALID_ARGUMENT,
                    ChipLogError(chipTool, "%s%s", kErrorHTTPSPrefix, url.c_str()));

                auto strippedUrl = url.substr(strlen(kHttpsPrefix));
                VerifyOrReturnError("" != strippedUrl, CHIP_ERROR_INVALID_ARGUMENT, ChipLogError(chipTool, "%s", kErrorHTTPSHostName));

                size_t position = strippedUrl.find('/');
                if (position == std::string::npos) {
                    hostAndPort = strippedUrl;
                    outPath = "/";
                } else {
                    hostAndPort = strippedUrl.substr(0, position);
                    outPath = strippedUrl.substr(position);
                }

                return CHIP_NO_ERROR;
            }

            CHIP_ERROR ExtractHostAndPort(const std::string & hostAndPort, std::string & outHostName, uint16_t & outPort)
            {
                size_t position = hostAndPort.find(':');
                if (position == std::string::npos) {
                    outHostName = hostAndPort;
                    outPort = kHttpsPort;
                } else {
                    outHostName = hostAndPort.substr(0, position);
                    auto portString = hostAndPort.substr(position + 1);
                    outPort = static_cast<uint16_t>(std::atoi(portString.c_str()));
                    VerifyOrReturnError(0 != outPort, CHIP_ERROR_INVALID_ARGUMENT, ChipLogError(chipTool, "%s", kErrorHTTPSPort));
                }

                return CHIP_NO_ERROR;
            }

            CHIP_ERROR ExtractHostNamePortPath(std::string url, std::string & outHostName, uint16_t & outPort, std::string & outPath)
            {
                std::string hostAndPort;
                ReturnErrorOnFailure(ExtractHostAndPath(url, hostAndPort, outPath));
                ReturnErrorOnFailure(ExtractHostAndPort(hostAndPort, outHostName, outPort));

                return CHIP_NO_ERROR;
            }
        } // namespace

        CHIP_ERROR Request(std::string url, Json::Value & jsonResponse, const Optional<uint32_t> & optionalExpectedSize,
            const Optional<const char *> & optionalExpectedDigest)
        {
            std::string hostname;
            uint16_t port;
            std::string path;
            ReturnErrorOnFailure(ExtractHostNamePortPath(url, hostname, port, path));
            return Request(hostname, port, path, jsonResponse, optionalExpectedSize, optionalExpectedDigest);
        }

        CHIP_ERROR Request(std::string hostname, uint16_t port, std::string path, Json::Value & jsonResponse,
            const Optional<uint32_t> & optionalExpectedSize, const Optional<const char *> & optionalExpectedDigest)
        {
            VerifyOrDo(port != 0, port = kHttpsPort);

            ChipLogDetail(chipTool, "HTTPS request to %s:%u%s", hostname.c_str(), port, path.c_str());

            std::string request = BuildRequest(hostname, path);
            std::string response;

            HTTPSSessionHolder session;
            ReturnErrorOnFailure(session.Init(hostname, port));
            ReturnErrorOnFailure(session.SendRequest(request));
            ReturnErrorOnFailure(session.ReceiveResponse(response));
            ReturnErrorOnFailure(RemoveHeader(response));
            ReturnErrorOnFailure(MaybeCheckResponseSize(response, optionalExpectedSize));
            ReturnErrorOnFailure(MaybeCheckResponseDigest(response, optionalExpectedDigest));
            ReturnErrorOnFailure(ConvertResponseToJSON(response, jsonResponse));
            return CHIP_NO_ERROR;
        }

    } // namespace https
} // namespace tool
} // namespace chip
