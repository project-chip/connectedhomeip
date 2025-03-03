/*
 *   Copyright (c) 2024 Project CHIP Authors
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

#include "HTTPSRequest.h"

#if CHIP_HAVE_CONFIG_H
#include <crypto/CryptoBuildConfig.h>
#endif // CHIP_HAVE_CONFIG_H

#include <lib/support/Base64.h>
#include <lib/support/SafeInt.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/logging/CHIPLogging.h>
#include <system/SystemError.h>

#ifdef CONFIG_ENABLE_HTTPS_REQUESTS
#if (CHIP_CRYPTO_OPENSSL || CHIP_CRYPTO_BORINGSSL)
#include <netdb.h>
#include <openssl/ssl.h>
#include <unistd.h>
#ifdef SHA256_DIGEST_LENGTH
#define USE_CHIP_CRYPTO 1
#endif
#endif //(CHIP_CRYPTO_OPENSSL || CHIP_CRYPTO_BORINGSSL)
#endif // CONFIG_ENABLE_HTTPS_REQUESTS

namespace {
constexpr const char * kHttpsPrefix        = "https://";
constexpr uint16_t kHttpsPort              = 443;
constexpr const char * kErrorJsonParse     = "Failed to parse JSON: ";
constexpr const char * kErrorHTTPSPrefix   = "URL must start with 'https://': ";
constexpr const char * kErrorHTTPSPort     = "Invalid port: 0";
constexpr const char * kErrorHTTPSHostName = "Invalid hostname: empty";
constexpr const char * kErrorBase64Decode  = "Error while decoding base64 data";
constexpr const char * kErrorSizeMismatch  = "The response size does not match the expected size: ";
} // namespace

namespace chip {
namespace tool {
namespace https {
namespace {
#ifndef USE_CHIP_CRYPTO
/**
 * @brief Stub implementation of HTTPSSessionHolder when neither OpenSSL nor BoringSSL is enabled.
 *
 * This class provides placeholder methods that log errors indicating the lack of SSL library support
 * and encourages contributions for new implementations.
 */
class HTTPSSessionHolder
{
public:
    CHIP_ERROR Init(std::string & hostname, uint16_t port, HttpsSecurityMode securityMode) { return LogNotImplementedError(); }

    CHIP_ERROR SendRequest(std::string & request) { return LogNotImplementedError(); }

    CHIP_ERROR ReceiveResponse(std::string & response) { return LogNotImplementedError(); }

private:
    CHIP_ERROR LogNotImplementedError() const
    {
#ifndef CONFIG_ENABLE_HTTPS_REQUESTS
        ChipLogError(chipTool, "HTTPS requests are disabled via build configuration (config_enable_https_requests=false).");
#elif !(CHIP_CRYPTO_OPENSSL || CHIP_CRYPTO_BORINGSSL)
        ChipLogError(chipTool,
                     "HTTPS requests are not available because neither OpenSSL nor BoringSSL is enabled. Contributions for "
                     "alternative implementations are welcome!");
#elif !defined(SHA256_DIGEST_LENGTH)
        ChipLogError(chipTool,
                     "HTTPS requests are not available because SHA256_DIGEST_LENGTH is not defined, meaning response integrity "
                     "verification via SHA-256 digest checking cannot be performed.");
#endif

        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
};
#else // USE_CHIP_CRYPTO
constexpr uint16_t kResponseBufferSize           = 4096;
constexpr const char * kErrorSendHTTPRequest     = "Failed to send HTTP request";
constexpr const char * kErrorReceiveHTTPResponse = "Failed to read HTTP response";
constexpr const char * kErrorConnection          = "Failed to connect to: ";
constexpr const char * kErrorSSLContextCreate    = "Failed to create SSL context";
constexpr const char * kErrorSSLObjectCreate     = "Failed to create SSL object";
constexpr const char * kErrorSSLHandshake        = "SSL handshake failed";
constexpr const char * kErrorDigestMismatch      = "The response digest does not match the expected digest";
class AddressInfoHolder
{
public:
    AddressInfoHolder(std::string & hostname, uint16_t port)
    {
        struct addrinfo hints                       = {};
        hints.ai_family                             = AF_INET;
        hints.ai_socktype                           = SOCK_STREAM;
        int err                                     = getaddrinfo(hostname.c_str(), std::to_string(port).c_str(), &hints, &mRes);
#if CHIP_ERROR_LOGGING
        constexpr const char * kErrorGetAddressInfo = "Failed to get address info: ";
        VerifyOrDo(nullptr != mRes, ChipLogError(chipTool, "%s%s", kErrorGetAddressInfo, gai_strerror(err)));
#else
        (void) err;
#endif
    }

    ~AddressInfoHolder()
    {
        if (mRes != nullptr)
        {
            freeaddrinfo(mRes);
        }
    }

    bool HasInfo() const { return mRes != nullptr; }
    struct addrinfo * Get() const { return mRes; }

private:
    struct addrinfo * mRes = nullptr;
};

class HTTPSSessionHolder
{
public:
    HTTPSSessionHolder(){};

    ~HTTPSSessionHolder()
    {
        if (nullptr != mContext)
        {
            SSL_free(mSSL);
            SSL_CTX_free(mContext);
        }

        if (mSock >= 0)
        {
            close(mSock);
        }

#if !defined(OPENSSL_IS_BORINGSSL)
        EVP_cleanup();
#endif
    }

    CHIP_ERROR Init(std::string & hostname, uint16_t port, HttpsSecurityMode securityMode)
    {
        int sock;
        VerifyOrReturnError(securityMode == HttpsSecurityMode::kDefault, CHIP_ERROR_NOT_IMPLEMENTED);
        ReturnErrorOnFailure(InitSocket(hostname, port, sock));
        ReturnErrorOnFailure(InitSSL(sock));
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SendRequest(std::string & request)
    {
        int written = SSL_write(mSSL, request.c_str(), (int) request.size());
        VerifyOrReturnError(written > 0, CHIP_ERROR_BAD_REQUEST, ChipLogError(chipTool, "%s", kErrorSendHTTPRequest));
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR ReceiveResponse(std::string & response)
    {
        char buffer[kResponseBufferSize];

        ssize_t n = -1;
        while ((n = SSL_read(mSSL, buffer, sizeof(buffer))) > 0)
        {
            VerifyOrReturnError(CanCastTo<size_t>(n), CHIP_ERROR_INVALID_ARGUMENT);
            response.append(buffer, static_cast<size_t>(n));
        }

        VerifyOrReturnError(n >= 0, CHIP_ERROR_INTERNAL, ChipLogError(chipTool, "%s", kErrorReceiveHTTPResponse));

        return CHIP_NO_ERROR;
    }

private:
    CHIP_ERROR InitSocket(std::string & hostname, uint16_t port, int & sock)
    {
        AddressInfoHolder addressInfoHolder(hostname, port);
        VerifyOrReturnError(addressInfoHolder.HasInfo(), CHIP_ERROR_NOT_CONNECTED);

        auto * res = addressInfoHolder.Get();
        for (struct addrinfo * p = res; p != nullptr; p = p->ai_next)
        {
            sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
            if (sock < 0)
            {
                continue; // Try the next address
            }

            if (connect(sock, p->ai_addr, p->ai_addrlen) != 0)
            {
                close(sock);
                sock = -1;
                continue; // Try the next address
            }

            return CHIP_NO_ERROR;
        }

        ChipLogError(chipTool, "%s%s:%u", kErrorConnection, hostname.c_str(), port);
        return CHIP_ERROR_NOT_CONNECTED;
    }

    CHIP_ERROR InitSSL(int sock)
    {
        SSL_load_error_strings();
        OpenSSL_add_ssl_algorithms();

        auto * context = SSL_CTX_new(TLS_client_method());
        VerifyOrReturnError(nullptr != context, CHIP_ERROR_NOT_CONNECTED, ChipLogError(chipTool, "%s", kErrorSSLContextCreate));

        auto * ssl = SSL_new(context);
        VerifyOrReturnError(nullptr != ssl, CHIP_ERROR_NOT_CONNECTED, ChipLogError(chipTool, "%s", kErrorSSLObjectCreate));

        SSL_set_fd(ssl, sock);
        VerifyOrReturnError(SSL_connect(ssl) > 0, CHIP_ERROR_NOT_CONNECTED, ChipLogError(chipTool, "%s", kErrorSSLHandshake));

        mContext = context;
        mSSL     = ssl;
        mSock    = sock;
        return CHIP_NO_ERROR;
    }

    SSL_CTX * mContext = nullptr;
    SSL * mSSL         = nullptr;
    int mSock          = -1;
};
#endif // USE_CHIP_CRYPTO

std::string BuildRequest(std::string & hostname, std::string & path)
{
    return "GET " + path + " HTTP/1.1\r\n" + //
        "Host: " + hostname + "\r\n" +       //
        "Accept: application/json\r\n" +     //
        "Connection: close\r\n\r\n";         //
}

CHIP_ERROR RemoveHeader(std::string & response)
{
    size_t headerEnd = response.find("\r\n\r\n");
    VerifyOrReturnError(std::string::npos != headerEnd, CHIP_ERROR_INVALID_ARGUMENT);

    auto body = response.substr(headerEnd + 4);
    response  = body;

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

#ifdef USE_CHIP_CRYPTO
    // Compute the SHA-256 hash of the response
    unsigned char responseDigest[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char *>(response.c_str()), response.size(), responseDigest);

    VerifyOrReturnError(memcmp(responseDigest, decodedData.Get(), SHA256_DIGEST_LENGTH) == 0, CHIP_ERROR_INVALID_ARGUMENT,
                        ChipLogError(chipTool, "%s", kErrorDigestMismatch));
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // USE_CHIP_CRYPTO

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
    if (position == std::string::npos)
    {
        hostAndPort = strippedUrl;
        outPath     = "/";
    }
    else
    {
        hostAndPort = strippedUrl.substr(0, position);
        outPath     = strippedUrl.substr(position);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ExtractHostAndPort(const std::string & hostAndPort, std::string & outHostName, uint16_t & outPort)
{
    size_t position = hostAndPort.find(':');
    if (position == std::string::npos)
    {
        outHostName = hostAndPort;
        outPort     = kHttpsPort;
    }
    else
    {
        outHostName     = hostAndPort.substr(0, position);
        auto portString = hostAndPort.substr(position + 1);
        outPort         = static_cast<uint16_t>(std::atoi(portString.c_str()));
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

    ChipLogDetail(chipTool, "HTTPS request to %s:%u%s", hostname.c_str(), port, path.c_str());

    std::string request = BuildRequest(hostname, path);
    std::string response;

    HTTPSSessionHolder session;
    ReturnErrorOnFailure(session.Init(hostname, port, securityMode));
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
