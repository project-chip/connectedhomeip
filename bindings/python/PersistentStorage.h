/*
 *    Copyright (c) 2021 Project CHIP Authors
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

#pragma once

#include <Python.h>

#include <core/CHIPPersistentStorageDelegate.h>

struct PythonPersistentStorage;

class PythonPersistentStorageDelegate : public chip::PersistentStorageDelegate
{
public:
    PythonPersistentStorageDelegate(PythonPersistentStorage * owner) : mOwner(owner) {}

    /////////// PersistentStorageDelegate Interface /////////
    CHIP_ERROR SyncGetKeyValue(const char * key, void * buffer, uint16_t & size) override;
    CHIP_ERROR SyncSetKeyValue(const char * key, const void * value, uint16_t size) override;
    CHIP_ERROR SyncDeleteKeyValue(const char * key) override;
private:
    PythonPersistentStorage * mOwner;
};

struct PythonPersistentStorage {
    PyObject_HEAD
    PythonPersistentStorageDelegate mDelegate;
};

extern PyTypeObject PythonPersistentStorageType;
