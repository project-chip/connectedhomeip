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

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <PersistentStorage.h>

#include <new>

#include <support/ErrorStr.h>

#include <PythonGil.h>

static PyObject * PythonPersistentStorageNew(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PythonPersistentStorage *pyo = reinterpret_cast<PythonPersistentStorage*>(type->tp_alloc(type, 0));
    new (&pyo->mDelegate) PythonPersistentStorageDelegate(pyo);
    return (PyObject *)pyo;
}

static void PythonPersistentStorageDealloc(PyObject *self)
{
    PythonPersistentStorage *pyo = reinterpret_cast<PythonPersistentStorage*>(self);
    pyo->mDelegate.~PythonPersistentStorageDelegate();

    PyTypeObject *tp = Py_TYPE(self);
    tp->tp_free(self);
}

CHIP_ERROR PythonPersistentStorageDelegate::SyncGetKeyValue(const char * key, void * buffer, uint16_t & size)
{
    PythonGil lock;

    // s (str or None) [const char *]
    PyObject *result = PyObject_CallMethod(&mOwner->ob_base, "SyncGetKeyValue", "(s)", key);
    if (result == nullptr) {
        PyErr_Print();
        return CHIP_ERROR_INTERNAL;
    }

    if (result == Py_None) {
        Py_DECREF(result);
        return CHIP_ERROR_KEY_NOT_FOUND;
    }

    if (!PyBytes_Check(result)) {
        fprintf(stderr, "PythonPersistentStorage::SyncGetKeyValue returns wrong type, expect bytes");
        Py_DECREF(result);
        return CHIP_ERROR_INTERNAL;
    }

    Py_ssize_t outSize = PyBytes_Size(result);
    if (outSize > size) {
        fprintf(stderr, "PythonPersistentStorage::SyncGetKeyValue not enough space");
        size = outSize;
        Py_DECREF(result);
        return CHIP_ERROR_NO_MEMORY;
    }

    memcpy(buffer, PyBytes_AsString(result), outSize);
    size = outSize;

    Py_DECREF(result);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PythonPersistentStorageDelegate::SyncSetKeyValue(const char * key, const void * value, uint16_t size)
{
    PythonGil lock;

    // s (str or None) [const char *]
    // y# (bytes) [const char *, int or Py_ssize_t]
    PyObject *result = PyObject_CallMethod(&mOwner->ob_base, "SyncSetKeyValue", "(sy#)", key, value, size);
    if (result == nullptr) {
        PyErr_Print();
        return CHIP_ERROR_INTERNAL;
    }

    Py_DECREF(result);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PythonPersistentStorageDelegate::SyncDeleteKeyValue(const char * key)
{
    PythonGil lock;

    // s (str or None) [const char *]
    PyObject *result = PyObject_CallMethod(&mOwner->ob_base, "SyncDeleteKeyValue", "(s)", key);
    if (result == nullptr) {
        PyErr_Print();
        return CHIP_ERROR_INTERNAL;
    }

    Py_DECREF(result);
    return CHIP_NO_ERROR;
}

static PyMethodDef PythonPersistentStorageMethods[] = {
    {NULL, NULL, 0, NULL}
};

PyTypeObject PythonPersistentStorageType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "chip.PersistentStorageInterface",        /* tp_name */
    sizeof(PythonPersistentStorage), /* tp_basicsize */
    0,                                /* tp_itemsize */
    PythonPersistentStorageDealloc,  /* tp_dealloc */
    0,                                /* tp_vectorcall_offset */
    0,                                /* tp_getattr */
    0,                                /* tp_setattr */
    0,                                /* tp_as_async */
    0,                                /* tp_repr */
    0,                                /* tp_as_number */
    0,                                /* tp_as_sequence */
    0,                                /* tp_as_mapping */
    0,                                /* tp_hash */
    0,                                /* tp_call */
    0,                                /* tp_str */
    0,                                /* tp_getattro */
    0,                                /* tp_setattro */
    0,                                /* tp_as_buffer */
    Py_TPFLAGS_BASETYPE,              /* tp_flags */
    0,                                /* tp_doc */
    0,                                /* tp_traverse */
    0,                                /* tp_clear */
    0,                                /* tp_richcompare */
    0,                                /* tp_weaklistoffset */
    0,                                /* tp_iter */
    0,                                /* tp_iternext */
    PythonPersistentStorageMethods,  /* tp_methods */
    0,                                /* tp_members */
    0,                                /* tp_getset */
    0,                                /* tp_base */
    0,                                /* tp_dict */
    0,                                /* tp_descr_get */
    0,                                /* tp_descr_set */
    0,                                /* tp_dictoffset */
    0,                                /* tp_init */
    0,                                /* tp_alloc */
    PythonPersistentStorageNew,      /* tp_new */
};
