/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <map>
#include <string>

#include <lib/core/CHIPPersistentStorageDelegate.h>

class PythonPersistentStorageDelegate;

namespace chip {
namespace Controller {

class PythonPersistentStorageDelegate : public PersistentStorageDelegate
{
public:
    PythonPersistentStorageDelegate() {}
    CHIP_ERROR SyncGetKeyValue(const char * key, void * buffer, uint16_t & size) override;
    CHIP_ERROR SyncSetKeyValue(const char * key, const void * value, uint16_t size) override;
    CHIP_ERROR SyncDeleteKeyValue(const char * key) override;

private:
    std::map<std::string, std::string> mStorage;
};

namespace Python {

using PyObject = void;

using SyncSetKeyValueCb    = void (*)(PyObject * appContext, const char * key, const void * value, uint16_t size);
using SetGetKeyValueCb     = void (*)(PyObject * appContext, const char * key, char * value, uint16_t * size, bool * isFound);
using SyncDeleteKeyValueCb = void (*)(PyObject * appContext, const char * key);

class StorageAdapter : public PersistentStorageDelegate
{
public:
    StorageAdapter(PyObject * context, SyncSetKeyValueCb setCb, SetGetKeyValueCb getCb, SyncDeleteKeyValueCb deleteCb)
    {
        mSetKeyCb    = setCb;
        mGetKeyCb    = getCb;
        mDeleteKeyCb = deleteCb;
        mContext     = context;
    }

    CHIP_ERROR SyncGetKeyValue(const char * key, void * buffer, uint16_t & size) override;
    CHIP_ERROR SyncSetKeyValue(const char * key, const void * value, uint16_t size) override;
    CHIP_ERROR SyncDeleteKeyValue(const char * key) override;

private:
    SyncSetKeyValueCb mSetKeyCb;
    SetGetKeyValueCb mGetKeyCb;
    SyncDeleteKeyValueCb mDeleteKeyCb;
    PyObject * mContext;
};

} // namespace Python
} // namespace Controller
} // namespace chip
