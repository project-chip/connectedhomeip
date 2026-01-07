#include "FilesystemStorageLocationProviderImpl.h"

#include <string>

namespace chip {
namespace DeviceLayer {

static const std::string factoryroot  = "/tmp";
static const std::string configroot   = "/tmp";
static const std::string countersroot = "/tmp";
static const std::string kvsroot      = "/tmp";

std::string DefaultFilesystemStorageLocationProviderImpl::GetFactoryDataLocation() const
{
    return factoryroot;
}

std::string DefaultFilesystemStorageLocationProviderImpl::GetConfigDataLocation() const
{
    return configroot;
}

std::string DefaultFilesystemStorageLocationProviderImpl::GetCountersDataLocation() const
{
    return countersroot;
}

std::string DefaultFilesystemStorageLocationProviderImpl::GetKVSDataLocation() const
{
    return countersroot;
}

DefaultFilesystemStorageLocationProviderImpl & DefaultFilesystemStorageLocationProviderImpl::GetDefaultInstance()
{
    static DefaultFilesystemStorageLocationProviderImpl sInstance;
    return sInstance;
}

} // namespace DeviceLayer
} // namespace chip
