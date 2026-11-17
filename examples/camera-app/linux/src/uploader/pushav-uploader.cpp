/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "pushav-uploader.h"
#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <lib/support/logging/CHIPLogging.h>
#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <sys/stat.h>
#include <system_error>
#include <unistd.h>
#include <vector>

PushAVUploader::PushAVUploader() : mIsRunning(false) {}

PushAVUploader::~PushAVUploader()
{
    std::pair<std::string, std::string> lastUploadJob;
    {
        std::lock_guard<std::mutex> lock(mQueueMutex);
        while (mAvData.size() > 1)
        {
            mAvData.pop();
        }

        if (!mAvData.empty())
        {
            lastUploadJob = std::move(mAvData.front());
            mAvData.pop();
        }
    }

    if (!lastUploadJob.first.empty() && !lastUploadJob.second.empty())
    {
        const std::filesystem::path filePath(lastUploadJob.first);

        if (filePath.extension() == ".mpd" || filePath.extension() == ".upload")
        {
            UploadData(lastUploadJob);
        }
    }

    Stop();
}

// Helper function to convert certificate from DER format to PEM format
std::string DerCertToPem(const std::vector<uint8_t> & derData)
{
    const unsigned char * p = derData.data();
    X509 * cert             = d2i_X509(nullptr, &p, derData.size());
    if (!cert)
    {
        ChipLogError(Camera, "Failed to parse DER certificate of size: %zu", derData.size());
        return "";
    }

    BIO * bio = BIO_new(BIO_s_mem());
    if (!bio)
    {
        X509_free(cert);
        ChipLogError(Camera, "Failed to allocate BIO");
        return "";
    }

    if (!PEM_write_bio_X509(bio, cert))
    {
        BIO_free(bio);
        X509_free(cert);
        ChipLogError(Camera, "Failed to write PEM certificate");
        return "";
    }

    BUF_MEM * bptr;
    BIO_get_mem_ptr(bio, &bptr);
    std::string pem(bptr->data, bptr->length);

    BIO_free(bio);
    X509_free(cert);

    return pem;
}

// Helper function to convert ECDSA private key from DER format to PEM format
std::string ConvertECDSAPrivateKey_DER_to_PEM(const std::vector<uint8_t> & derData)
{
    const unsigned char * p = derData.data();

    EVP_PKEY * pkey = d2i_AutoPrivateKey(nullptr, &p, derData.size());
    if (!pkey)
    {
        ChipLogError(Camera, "Failed to parse DER ECDSA private key of size: %zu", derData.size());
        return "";
    }

    // Write PEM to memory BIO
    BIO * bio = BIO_new(BIO_s_mem());
    if (!bio)
    {
        EVP_PKEY_free(pkey);
        ChipLogError(Camera, "Failed to allocate BIO");
        return "";
    }

    // Use PEM_write_bio_PrivateKey to write the EVP_PKEY in PEM format.
    // This function handles the key type automatically.
    if (!PEM_write_bio_PrivateKey(bio, pkey, nullptr, nullptr, 0, nullptr, nullptr))
    {
        BIO_free(bio);
        EVP_PKEY_free(pkey);
        ChipLogError(Camera, "Failed to convert ECDSA key to PEM format");
        return "";
    }

    // Extract PEM string
    char * pemData = nullptr;
    long pemLen    = BIO_get_mem_data(bio, &pemData);
    std::string pemStr(pemData, pemLen);

    BIO_free(bio);
    EVP_PKEY_free(pkey);

    return pemStr;
}

// Helper function to read and print file content to log
std::string readAndPrintFile(const std::string & filePath)
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
        ChipLogDetail(Camera, "Failed to open file: %s", filePath.c_str());
        return "";
    }

    // Read the entire file content
    std::string fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Print the file content
    ChipLogDetail(Camera, "File:%s", filePath.c_str());
    ChipLogDetail(Camera, "Content:%s", fileContent.c_str());

    file.close();
    return fileContent;
}

// Creates the file with mode 0600 so the content is never visible with wider permissions.
// O_EXCL and O_NOFOLLOW make the call fail instead of reusing or following an existing path.
bool SaveCertToFile(const std::string & certData, const std::string & filePath)
{
    int fd = open(filePath.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, S_IRUSR | S_IWUSR);
    if (fd < 0)
    {
        ChipLogError(Camera, "Failed to create file %s: %s", filePath.c_str(), strerror(errno));
        return false;
    }

    size_t written = 0;
    while (written < certData.size())
    {
        ssize_t n = write(fd, certData.data() + written, certData.size() - written);
        if (n < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            ChipLogError(Camera, "Failed to write file %s: %s", filePath.c_str(), strerror(errno));
            close(fd);
            return false;
        }
        written += static_cast<size_t>(n);
    }

    if (close(fd) != 0)
    {
        ChipLogError(Camera, "Failed to close file %s: %s", filePath.c_str(), strerror(errno));
        return false;
    }
    return true;
}

// Overwrites the file content with zeros so the key does not stay in freed disk blocks. Best effort.
static void ZeroFile(const std::string & filePath)
{
    int fd = open(filePath.c_str(), O_WRONLY | O_NOFOLLOW | O_CLOEXEC);
    if (fd < 0)
    {
        return;
    }

    struct stat st;
    if (fstat(fd, &st) == 0)
    {
        const std::vector<char> zeros(static_cast<size_t>(st.st_size), 0);
        size_t written = 0;
        while (written < zeros.size())
        {
            ssize_t n = write(fd, zeros.data() + written, zeros.size() - written);
            if (n < 0 && errno == EINTR)
            {
                continue;
            }
            if (n <= 0)
            {
                break;
            }
            written += static_cast<size_t>(n);
        }
        fsync(fd);
    }
    close(fd);
}

bool PushAVUploader::TlsFiles::EnsureCreated(const PushAVCertBuffer & certBuffer)
{
    std::lock_guard<std::mutex> lock(mMutex);
    if (mReady)
    {
        return true;
    }

    std::error_code ec;
    std::string dirTemplate = (std::filesystem::temp_directory_path(ec) / "pushav-tls-XXXXXX").string();
    if (ec)
    {
        ChipLogError(Camera, "Failed to get temporary directory: %s", ec.message().c_str());
        return false;
    }
    if (mkdtemp(dirTemplate.data()) == nullptr)
    {
        ChipLogError(Camera, "Failed to create TLS directory %s: %s", dirTemplate.c_str(), strerror(errno));
        return false;
    }
    mDir         = dirTemplate;
    mRootPath    = mDir + "/root.pem";
    mDevCertPath = mDir + "/dev.pem";
    mDevKeyPath  = mDir + "/dev.key";

    // TODO: The logic to provide DER-formatted certificates and keys in memory (blob) format to curl is currently unstable. As a
    // temporary workaround, PEM-format files are being provided as input to curl.
    const std::string rootCertPEM = DerCertToPem(certBuffer.mRootCertBuffer);
    std::string clientCertPEM     = DerCertToPem(certBuffer.mClientCertBuffer);
    bool converted                = !rootCertPEM.empty() && !clientCertPEM.empty();
    for (const auto & intermediate : certBuffer.mIntermediateCertBuffer)
    {
        const std::string intermediatePEM = DerCertToPem(intermediate);
        converted                         = converted && !intermediatePEM.empty();
        clientCertPEM.append("\n" + intermediatePEM);
    }
    const std::string keyPEM = ConvertECDSAPrivateKey_DER_to_PEM(certBuffer.mClientKeyBuffer);
    converted                = converted && !keyPEM.empty();

    if (converted && SaveCertToFile(rootCertPEM, mRootPath) && SaveCertToFile(clientCertPEM, mDevCertPath) &&
        SaveCertToFile(keyPEM, mDevKeyPath))
    {
        mReady = true;
        return true;
    }

    ChipLogError(Camera, "Failed to prepare TLS files in %s", mDir.c_str());
    RemoveLocked();
    return false;
}

PushAVUploader::TlsFiles::~TlsFiles()
{
    std::lock_guard<std::mutex> lock(mMutex);
    RemoveLocked();
}

void PushAVUploader::TlsFiles::RemoveLocked()
{
    if (mDir.empty())
    {
        return;
    }

    ZeroFile(mDevKeyPath);
    for (const std::string & path : { mRootPath, mDevCertPath, mDevKeyPath })
    {
        if (unlink(path.c_str()) != 0 && errno != ENOENT)
        {
            ChipLogError(Camera, "Failed to delete %s: %s", path.c_str(), strerror(errno));
        }
    }
    if (rmdir(mDir.c_str()) != 0)
    {
        ChipLogError(Camera, "Failed to remove directory %s: %s", mDir.c_str(), strerror(errno));
    }
    mDir.clear();
    mReady = false;
}

void PushAVUploader::ProcessQueue()
{
    while (mIsRunning)
    {
        std::pair<std::string, std::string> uploadJob;
        {
            std::lock_guard<std::mutex> lock(mQueueMutex);
            if (!mAvData.empty())
            {
                uploadJob = std::move(mAvData.front());
                mAvData.pop();
            }
        }
        if (!uploadJob.first.empty() && !uploadJob.second.empty())
        {
            UploadData(uploadJob);
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

void PushAVUploader::Start()
{
    if (!mIsRunning)
    {
        mIsRunning      = true;
        mUploaderThread = std::thread(&PushAVUploader::ProcessQueue, this);
    }
}

void PushAVUploader::Stop()
{
    if (mIsRunning)
    {
        mIsRunning = false;
        if (mUploaderThread.joinable())
        {
            mUploaderThread.join();
        }
    }
}

void PushAVUploader::AddUploadData(const std::string & filename, const std::string & url)
{
    ChipLogProgress(Camera, "Added file name %s to queue", filename.c_str());
    std::lock_guard<std::mutex> lock(mQueueMutex);
    auto data = std::make_pair(filename, url);
    mAvData.push(data);
}

size_t PushAvUploadCb(void * ptr, size_t size, size_t nmemb, void * stream)
{
    int bufferSize            = (int) (size * nmemb);
    PushAvUploadInfo * upload = (PushAvUploadInfo *) stream;
    if (ptr == NULL)
    {
        ChipLogError(Camera, "Invalid destination pointer");
        return 0;
    }
    if ((size == 0) || (nmemb == 0) || ((size * nmemb) < 1))
    {
        ChipLogError(Camera, "Zero buffer size = %zu nmemb = %zu %zu\n", size, nmemb, size * nmemb);
        return 0;
    }
    long remaining          = upload->mSize - upload->mBytesRead;
    unsigned long copyChunk = (unsigned long) ((remaining < bufferSize) ? remaining : bufferSize);

    if (copyChunk)
    {
        memcpy(ptr, upload->mData + upload->mBytesRead, copyChunk);
        upload->mBytesRead += copyChunk;
    }
    return (size_t) copyChunk;
}

std::string ProcessInitUploadPath(std::string path, const std::vector<std::string> & streamIdNameMap)
{
    auto result = std::move(path);
    // Replace stream ID placeholder #__X__# with stream name
    const auto startPos = result.find("#__");
    const auto endPos   = result.find("__#");
    if (startPos != std::string::npos && endPos != std::string::npos && startPos + 4 == endPos)
    {
        const int streamId = result[startPos + 3] - '0';
        if (streamId >= 0 && streamId < static_cast<int>(streamIdNameMap.size()))
        {
            // Path traversal check - reject if stream name contains dangerous characters
            if (streamIdNameMap[streamId].find("..") != std::string::npos ||
                streamIdNameMap[streamId].find('/') != std::string::npos)
            {
                ChipLogError(Camera, "Invalid stream name '%s' detected, rejecting to prevent path traversal",
                             streamIdNameMap[streamId].c_str());
                return path;
            }
            result.replace(startPos, 7, streamIdNameMap[streamId] + "/" + streamIdNameMap[streamId]);
        }
        else
        {
            ChipLogError(Camera, "Stream ID %d not found in streamIdNameMap", streamId);
            return path;
        }
    }
    ChipLogDetail(Camera, "Processed init upload path to %s", result.c_str());
    return result;
}

std::string ProcessM4SUploadPath(std::string path, const std::vector<std::string> & streamIdNameMap)
{
    auto result = std::move(path);

    // Replace stream ID placeholder #__X__# with stream name
    const auto startPos = result.find("#__");
    const auto endPos   = result.find("__#");

    if (startPos != std::string::npos && endPos != std::string::npos && startPos + 4 == endPos)
    {
        const int streamId = result[startPos + 3] - '0';

        if (streamId >= 0 && streamId < static_cast<int>(streamIdNameMap.size()))
        {
            // Path traversal check - reject if stream name contains dangerous characters
            if (streamIdNameMap[streamId].find("..") != std::string::npos ||
                streamIdNameMap[streamId].find('/') != std::string::npos)
            {
                ChipLogError(Camera, "Invalid stream name '%s' detected, rejecting to prevent path traversal",
                             streamIdNameMap[streamId].c_str());
                return path;
            }
            result.replace(startPos, 7, streamIdNameMap[streamId] + "/");
        }
        else
        {
            return path;
        }
    }

    // FFmpeg names segment files starting from 0001, but the spec requires segment
    // numbering to start from 1001. Add offset of 1000 to convert file segment
    // numbers to spec-compliant segment numbers in the upload path.
    // e.g., segment_0001.m4s on disk -> segment_1001.m4s on server
    const std::string segPrefix = "segment_";
    const auto segPos           = result.find(segPrefix);
    if (segPos != std::string::npos)
    {
        const auto numStart = segPos + segPrefix.length();
        const auto numEnd   = result.find(".m4s", numStart);
        if (numEnd != std::string::npos)
        {
            std::string numStr = result.substr(numStart, numEnd - numStart);
            bool valid         = !numStr.empty();
            for (char c : numStr)
            {
                if (!std::isdigit(static_cast<unsigned char>(c)))
                {
                    valid = false;
                    break;
                }
            }
            if (valid)
            {
                char * endPtr = nullptr;
                long segNum   = std::strtol(numStr.c_str(), &endPtr, 10);
                // Per spec, SegmentNumber is uint16 (range 1001-65535). In practice,
                // SegmentNumber will never exceed 9999 due to session duration limits
                // (5 min max / 500ms min segment duration = ~600 segments max per session).
                // Verify the parsed value is non-negative and that adding the +1000 offset
                // won't exceed the spec's uint16 type limit.
                if (endPtr == numStr.c_str() || segNum < 0 || segNum > UINT16_MAX - 1000)
                {
                    ChipLogError(Camera, "Segment number out of range: %s", numStr.c_str());
                    return path;
                }
                segNum += 1000; // Offset: 1 -> 1001, 2 -> 1002, etc.
                result.replace(numStart, numEnd - numStart, std::to_string(segNum));
            }
            else
            {
                ChipLogError(Camera, "Failed to parse segment number from path: %s", result.c_str());
                return path;
            }
        }
    }

    ChipLogDetail(Camera, "Updated M4S upload path to %s", result.c_str());
    return result;
}

void PushAVUploader::UploadData(std::pair<std::string, std::string> data)
{
    CURL * curl = curl_easy_init();
    if (!curl)
    {
        ChipLogError(Camera, "Failed to initialize CURL");
        return;
    }

    std::ifstream file(data.first.c_str(), std::ios::binary);
    if (!file)
    {
        ChipLogError(Camera, "Failed to open file %s", data.first.c_str());
        return;
    }
    file.seekg(0, std::ios::end);
    unsigned long size = (unsigned long) file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), static_cast<std::streamsize>(size)))
    {
        ChipLogError(Camera, "Failed to read file into buffer");
        file.close();
        return;
    }
    file.close();

    PushAvUploadInfo upload;
    upload.mData = (char *) std::malloc(size);
    if (!upload.mData)
    {
        ChipLogError(Camera, "Failed to allocate memory for upload data");
        return;
    }
    memcpy(upload.mData, buffer.data(), size);
    upload.mSize                = static_cast<long>(size);
    upload.mBytesRead           = 0;
    struct curl_slist * headers = nullptr;

    // Determine content type based on file extension
    std::string contentType = "application/*"; // Default fallback
    std::string fullPath    = data.first;
    // Extract file extension from full path using std::filesystem
    std::filesystem::path filePath(data.first);
    std::filesystem::path extension = filePath.extension();
    // .upload files are modified MPD snapshots - treat as MPD and strip .upload for remote URL
    bool isUploadMpd    = (extension == ".upload");
    bool isMpdExtension = (extension == ".mpd");
    if (isUploadMpd)
    {
        // Check if the base filename (without .upload) is an MPD file
        std::string baseName = filePath.stem().string();
        if (baseName.size() >= 4 && baseName.substr(baseName.size() - 4) == ".mpd")
        {
            contentType = "application/dash+xml";
            // Strip .upload suffix so remote URL uses .mpd extension
            static constexpr size_t kUploadSuffixLen = 7; // strlen(".upload")
            fullPath                                 = fullPath.substr(0, fullPath.size() - kUploadSuffixLen);
        }
    }
    else if (isMpdExtension)
    {
        contentType = "application/dash+xml"; // Manifest file
    }
    else if (extension == ".m4s")
    {
        contentType = "video/iso.segment"; // Media segment
        fullPath    = ProcessM4SUploadPath(data.first, mStreamIdNameMap);
    }
    else if (extension == ".init")
    {
        contentType = "video/mp4"; // Initialization segment
        fullPath    = ProcessInitUploadPath(data.first, mStreamIdNameMap);
    }

    std::string contentTypeHeader = "Content-Type: " + contentType;
    headers                       = curl_slist_append(headers, contentTypeHeader.c_str());

    // Extract the filename from the full path
    std::string filename = "";
    std::string baseUrl  = data.second;
    std::string fullUrl;

    // Declare all variables that are used after goto cleanup
    std::error_code ec;
    size_t sessionPos;
    CURLcode res;

    sessionPos = fullPath.find("/session_");
    if (sessionPos != std::string::npos)
    {
        filename = fullPath.substr(sessionPos + 1);
    }
    else
    {
        ChipLogError(Camera,
                     "Invalid file path: %s. Expected to contain "
                     "'session_<SessionNumber>/<TrackName>/segment_<SegmentNumber>.<SegmentExtension>' pattern. Skipping upload.",
                     fullPath.c_str());
        goto cleanup;
    }
    if (baseUrl.back() != '/')
    {
        baseUrl += "/";
    }
    fullUrl = baseUrl + filename;

    ChipLogProgress(Camera, "Uploading file: %s to URL: %s", filename.c_str(), fullUrl.c_str());

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_URL, fullUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, static_cast<curl_off_t>(size));
#ifndef TLS_CLUSTER_NOT_ENABLED

    if (!mTlsFiles.EnsureCreated(mCertBuffer))
    {
        ChipLogError(Camera, "TLS files are not available, skipping upload of %s", data.first.c_str());
        goto cleanup;
    }

    curl_easy_setopt(curl, CURLOPT_CAINFO, mTlsFiles.RootPath().c_str());
    curl_easy_setopt(curl, CURLOPT_SSLCERT, mTlsFiles.DevCertPath().c_str());
    curl_easy_setopt(curl, CURLOPT_SSLKEY, mTlsFiles.DevKeyPath().c_str());

    //  curl_blob rootBlob   = { mCertBuffer.mRootCertBuffer.data(), mCertBuffer.mRootCertBuffer.size(), CURL_BLOB_COPY };
    //  curl_blob clientBlob = { mCertBuffer.mClientCertBuffer.data(), mCertBuffer.mClientCertBuffer.size(), CURL_BLOB_COPY };
    //  curl_blob keyBlob    = { mCertBuffer.mClientKeyBuffer.data(), mCertBuffer.mClientKeyBuffer.size(), CURL_BLOB_COPY };

    // curl_easy_setopt(curl, CURLOPT_CAINFO_BLOB, &rootBlob);
    // curl_easy_setopt(curl, CURLOPT_SSLCERT_BLOB, &clientBlob);
    // curl_easy_setopt(curl, CURLOPT_SSLKEY_BLOB, &keyBlob);
#else
    // TODO: The else block is for testing purpose. It should be removed once the TLS cluster integration is stable.
    curl_easy_setopt(curl, CURLOPT_CAINFO, mCertPath.mRootCert.c_str());
    curl_easy_setopt(curl, CURLOPT_SSLCERT, mCertPath.mDevCert.c_str());
    curl_easy_setopt(curl, CURLOPT_SSLKEY, mCertPath.mDevKey.c_str());
#endif
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, PushAvUploadCb);
    curl_easy_setopt(curl, CURLOPT_READDATA, &upload);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK)
    {
        ChipLogError(Camera, "CURL upload failed [%s] %s, retrying...", data.first.c_str(), curl_easy_strerror(res));
        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            ChipLogError(Camera, "CURL upload failed again [%s] %s", data.first.c_str(), curl_easy_strerror(res));
        }
    }

    if (res == CURLE_OK)
    {
        ChipLogDetail(Camera, "CURL uploaded file  %s size: %zu", data.first.c_str(), static_cast<size_t>(size));
    }

    // Delete file after upload, except for .mpd files which are kept (FFmpeg may still be writing).
    // .upload files (modified MPD snapshots) are always deleted after upload.
    if (!isMpdExtension || isUploadMpd)
    {
        if (!std::filesystem::remove(data.first, ec))
        {
            ChipLogError(Camera, "Failed to delete file: %s, error code: %d, error: %s, category: %s. May cause file accumulation.",
                         data.first.c_str(), ec.value(), ec.message().c_str(), ec.category().name());
        }
        else
        {
            ChipLogDetail(Camera, "Successfully deleted file: %s", data.first.c_str());
        }
    }

cleanup:
    if (upload.mData)
    {
        std::free(upload.mData);
        upload.mData = nullptr;
    }
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
}
