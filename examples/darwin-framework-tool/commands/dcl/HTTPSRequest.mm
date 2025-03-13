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
#include <Foundation/Foundation.h>

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

@interface NSURLSessionDelegateAllowAll : NSObject <NSURLSessionDelegate>
@end

@implementation NSURLSessionDelegateAllowAll
- (void)URLSession:(NSURLSession *)session didReceiveChallenge:(NSURLAuthenticationChallenge *)challenge completionHandler:(void (^)(NSURLSessionAuthChallengeDisposition disposition, NSURLCredential * credential))completionHandler
{
    completionHandler(NSURLSessionAuthChallengeUseCredential, [NSURLCredential credentialForTrust:challenge.protectionSpace.serverTrust]);
}
@end

namespace chip {
namespace tool {
    namespace https {
        namespace {
            constexpr uint64_t kSendTimeoutSeconds = 10;
            constexpr const char * kErrorReceiveHTTPResponse = "Failed to read HTTP response: ";
            constexpr const char * kErrorReceiveHTTPResponseTimeout = "Failed to read HTTP response (timeout): ";
            constexpr const char * kErrorDigestMismatch = "The response digest does not match the expected digest";

            CHIP_ERROR SendRequest(const std::string & hostname, uint16_t port, const std::string & path, HttpsSecurityMode securityMode, std::string & response)
            {
                std::string urlString = (securityMode == HttpsSecurityMode::kDisableHttps ? "http://" : "https://") + hostname + ":" + std::to_string(port) + path;
                __auto_type * requestURL = [NSURL URLWithString:[NSString stringWithUTF8String:urlString.c_str()]];
                __auto_type * urlRequest = [NSMutableURLRequest requestWithURL:requestURL];
                [urlRequest setHTTPMethod:@"GET"];
                [urlRequest setValue:@"application/json" forHTTPHeaderField:@"Accept"];
                [urlRequest setValue:@"close" forHTTPHeaderField:@"Connection"];

                __auto_type * config = [NSURLSessionConfiguration defaultSessionConfiguration];

                NSURLSession * session;
                if (securityMode == HttpsSecurityMode::kDisableValidation) {
                    session = [NSURLSession sessionWithConfiguration:config
                                                            delegate:(id<NSURLSessionDelegate>) [[NSURLSessionDelegateAllowAll alloc] init]
                                                       delegateQueue:nil];
                } else {
                    session = [NSURLSession sessionWithConfiguration:config];
                }

                dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
                __block CHIP_ERROR rv = CHIP_NO_ERROR;
                __block std::string receivedData;
                NSURLSessionDataTask * task = [session dataTaskWithRequest:urlRequest
                                                         completionHandler:^(NSData * data, NSURLResponse * resp, NSError * error) {
                                                             if (error) {
                                                                 ChipLogError(chipTool, "%s%s", kErrorReceiveHTTPResponse, [[error localizedDescription] UTF8String]);
                                                                 rv = CHIP_ERROR_BAD_REQUEST;
                                                             } else {
                                                                 receivedData.assign((const char *) [data bytes], [data length]);
                                                                 rv = CHIP_NO_ERROR;
                                                             }
                                                             dispatch_semaphore_signal(semaphore);
                                                         }];

                [task resume];

                if (dispatch_semaphore_wait(semaphore, dispatch_time(DISPATCH_TIME_NOW, kSendTimeoutSeconds * NSEC_PER_SEC)) != 0) {
                    ChipLogError(chipTool, "%s%s", kErrorReceiveHTTPResponseTimeout, hostname.c_str());
                    return CHIP_ERROR_TIMEOUT;
                }

                response = receivedData;
                return rv;
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
            const Optional<const char *> & optionalExpectedDigest, HttpsSecurityMode securityMode)
        {
            std::string hostname;
            uint16_t port;
            std::string path;
            ReturnErrorOnFailure(ExtractHostNamePortPath(url, hostname, port, path));
            return Request(hostname, port, path, jsonResponse, optionalExpectedSize, optionalExpectedDigest, securityMode);
        }

        CHIP_ERROR Request(std::string hostname, uint16_t port, std::string path, Json::Value & jsonResponse,
            const Optional<uint32_t> & optionalExpectedSize, const Optional<const char *> & optionalExpectedDigest,
            HttpsSecurityMode securityMode)
        {
            VerifyOrDo(port != 0, port = kHttpsPort);

            char const * protocol;
            switch (securityMode) {
            case HttpsSecurityMode::kDefault:
                protocol = "HTTPS";
                break;
            case HttpsSecurityMode::kDisableValidation:
                protocol = "HTTPS (no validation)";
                break;
            case HttpsSecurityMode::kDisableHttps:
                protocol = "HTTP";
                break;
            }
            ChipLogDetail(chipTool, "%s request to %s:%u%s", protocol, hostname.c_str(), port, path.c_str());

            std::string response;
            ReturnErrorOnFailure(SendRequest(hostname, port, path, securityMode, response));
            ReturnErrorOnFailure(MaybeCheckResponseSize(response, optionalExpectedSize));
            ReturnErrorOnFailure(MaybeCheckResponseDigest(response, optionalExpectedDigest));
            ReturnErrorOnFailure(ConvertResponseToJSON(response, jsonResponse));
            return CHIP_NO_ERROR;
        }

    } // namespace https
} // namespace tool
} // namespace chip
