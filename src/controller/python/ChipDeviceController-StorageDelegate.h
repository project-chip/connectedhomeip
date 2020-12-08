#pragma once

#include <controller/CHIPPersistentStorageDelegate.h>

class PythonPersistentStorageDelegate;

typedef void (*GetKeyValueFunct)(const uint8_t * key, uint8_t * value, uint16_t * size);
typedef void (*SetKeyValueFunct)(const uint8_t * key, const uint8_t * value);
typedef void (*DeleteKeyValueFunct)(const uint8_t * key);

namespace chip {
namespace Controller {

// TODO: Implement this.
class PythonPersistentStorageDelegate : public PersistentStorageDelegate
{
public:
    PythonPersistentStorageDelegate() {}
    void SetDelegate(PersistentStorageResultDelegate * delegate) override{}
    void GetKeyValue(const char * key) override{}
    CHIP_ERROR GetKeyValue(const char * key, char * value, uint16_t & size) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    void SetKeyValue(const char * key, const char * value) override{}
    void DeleteKeyValue(const char * key) override{}
};

} // namespace Controller
} // namespace chip
