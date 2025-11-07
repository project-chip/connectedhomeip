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
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <lib/support/logging/CHIPLogging.h>
#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <sys/stat.h>
#include <vector>

PushAVUploader::PushAVUploader() : mIsRunning(false) {}

PushAVUploader::~PushAVUploader()
{
    // Ensure final MPD upload during uploader thread termination to persist media data before shutdown.
    if (!mMPDPath.first.empty() && !mMPDPath.second.empty())
    {
        ChipLogProgress(Camera, "Uploading final MPD to server before shutdown");
        UploadData(mMPDPath);
    }

    Stop();

    while (!mAvData.empty())
    {
        std::pair<std::string, std::string> uploadJob = std::move(mAvData.front());
        mAvData.pop();

        std::error_code ec;
        if (!std::filesystem::remove(uploadJob.first, ec))
        {
            ChipLogError(Camera, "Failed to delete file: %s, error: %s", uploadJob.first.c_str(), ec.message().c_str());
        }
        else
        {
            ChipLogDetail(Camera, "Successfully deleted file: %s", uploadJob.first.c_str());
        }
    }
}

// Helper function to convert certificate from DER format to PEM format
std::string DerCertToPem(const std::vector<uint8_t> & derData)
{
    const unsigned char * p = derData.data();
    X509 * cert             = d2i_X509(nullptr, &p, derData.size());
    if (!cert)
    {
        ChipLogError(Camera, "Failed to parse DER certificate of size: %ld", derData.size());
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
        ChipLogError(Camera, "Failed to parse DER ECDSA private key of size: %ld", derData.size());
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

void SaveCertToFile(const std::string & certData, const std::string & filePath)
{
    std::ofstream out(filePath, std::ios::binary);
    if (!out)
    {
        ChipLogDetail(Camera, "Failed to open file: %s", filePath.c_str());
        return;
    }
    out.write(certData.data(), certData.size());
    out.close();

    // Set file permissions to read/write for owner only (0600)
    if (chmod(filePath.c_str(), S_IRUSR | S_IWUSR) != 0)
    {
        ChipLogError(Camera, "Failed to set permissions for file: %s", filePath.c_str());
    }
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

void PushAVUploader::AddUploadData(std::string & filename, std::string & url)
{
    ChipLogProgress(Camera, "Added file name %s to queue", filename.c_str());
    std::lock_guard<std::mutex> lock(mQueueMutex);
    auto data = make_pair(filename, url);
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
        ChipLogError(Camera, "Zero buffer size = %ld nmemb = %ld %ld\n", size, nmemb, size * nmemb);
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
        return;
    }
    file.close();
    PushAvUploadInfo upload;
    upload.mData = (char *) std::malloc(size);
    memcpy(upload.mData, buffer.data(), size);
    upload.mSize                = static_cast<long>(size);
    upload.mBytesRead           = 0;
    struct curl_slist * headers = nullptr;

    // Determine content type based on file extension
    std::string contentType = "application/*"; // Default fallback

    // Extract file extension from full path using std::filesystem
    std::filesystem::path extension = data.first.extension();
    if (extension == ".mpd")
    {
        contentType = "application/dash+xml"; // Manifest file
    }
    else if (extension == ".m4s")
    {
        contentType = "video/iso.segment"; // Media segment
    }
    else if (extension == ".init")
    {
        contentType = "video/mp4"; // Initialization segment
    }

    std::string contentTypeHeader = "Content-Type: " + contentType;
    headers                       = curl_slist_append(headers, contentTypeHeader.c_str());

    // Extract the filename from the full path
    std::string fullPath = data.first;
    std::string filename = "";
    std::string baseUrl  = data.second;
    std::string fullUrl;

    // Declare all variables that are used after goto cleanup
    std::string rootCertPEM;
    std::string clientCertPEM;
    std::string derKeyToPemstr;
    bool isMPDFile;
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
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, static_cast<curl_off_t>(size));
#ifndef TLS_CLUSTER_NOT_ENABLED

    // TODO: The logic to provide DER-formatted certificates and keys in memory (blob) format to curl is currently unstable. As a
    // temporary workaround, PEM-format files are being provided as input to curl.

    rootCertPEM   = DerCertToPem(mCertBuffer.mRootCertBuffer);
    clientCertPEM = DerCertToPem(mCertBuffer.mClientCertBuffer);
    if (!mCertBuffer.mIntermediateCertBuffer.empty())
    {
        clientCertPEM.append("\n"); // Add newline separator between certs in PEM format
    }
    for (size_t i = 0; i < mCertBuffer.mIntermediateCertBuffer.size(); ++i)
    {
        clientCertPEM.append(DerCertToPem(mCertBuffer.mIntermediateCertBuffer[i]) + "\n");
    }
    derKeyToPemstr = ConvertECDSAPrivateKey_DER_to_PEM(mCertBuffer.mClientKeyBuffer);

    SaveCertToFile(rootCertPEM, "/tmp/root.pem");
    SaveCertToFile(clientCertPEM, "/tmp/dev.pem");

    // Logic to save PEM format to file
    SaveCertToFile(derKeyToPemstr, "/tmp/dev.key");

    curl_easy_setopt(curl, CURLOPT_CAINFO, "/tmp/root.pem");
    curl_easy_setopt(curl, CURLOPT_SSLCERT, "/tmp/dev.pem");
    curl_easy_setopt(curl, CURLOPT_SSLKEY, "/tmp/dev.key");

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
        ChipLogError(Camera, "CURL upload  failed [%s] %s", data.first.c_str(), curl_easy_strerror(res));
    }
    else
    {
        ChipLogDetail(Camera, "CURL uploaded file  %s size: %ld", data.first.c_str(), size);
    }

    if (extension != ".mpd")
    {
        if (!std::filesystem::remove(data.first, ec))
        {
            ChipLogError(Camera, "Failed to delete file: %s, error: %s", data.first.c_str(), ec.message().c_str());
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
