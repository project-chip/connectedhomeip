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

#include <Python.h>

#include <CommissionerInitParams.h>

#include <controller/CHIPDeviceController.h>
#include <support/ErrorStr.h>

#include <PersistentStorage.h>

static PyObject * PythonCommissionerInitParamsNew(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PythonCommissionerInitParams *pyo = reinterpret_cast<PythonCommissionerInitParams *>(type->tp_alloc(type, 0));
    new (&pyo->mOpCredsIssuer) chip::Controller::ExampleOperationalCredentialsIssuer();
    new (&pyo->mPrarms) chip::Controller::CommissionerInitParams();
    return (PyObject *)pyo;
}

static void PythonCommissionerInitParamsDealloc(PyObject *self)
{
    PythonCommissionerInitParams *pyo = reinterpret_cast<PythonCommissionerInitParams*>(self);
    pyo->mPrarms.~CommissionerInitParams();
    pyo->mOpCredsIssuer.~ExampleOperationalCredentialsIssuer();

    PyTypeObject *tp = Py_TYPE(self);
    tp->tp_free(self);
}


PyDoc_STRVAR(SetPersistentStorageDocument, "SetPersistentStorage()");
static PyObject * PythonCommissionerInitParamsSetPersistentStorage(PyObject *self, PyObject *args)
{
    PythonCommissionerInitParams *pyo = reinterpret_cast<PythonCommissionerInitParams*>(self);
    PyObject * pyStorage;

    // 1nd arg (O!: PyObject *): storage
    if (!PyArg_ParseTuple(args, "O!", &PythonPersistentStorageType, &pyStorage)) {
        return nullptr;
    }


    chip::PersistentStorageDelegate * storage = &reinterpret_cast<PythonPersistentStorage*>(pyStorage)->mDelegate;

    // TODO: wrap a real OpCredsIssuer in python.
    CHIP_ERROR err = pyo->mOpCredsIssuer.Initialize(*storage);
    if (err != CHIP_NO_ERROR) {
        PyErr_Format(PyExc_RuntimeError, "OpCredsIssuer.Initialize failed: %s.", chip::ErrorStr(err));
        return nullptr;
    }
    pyo->mPrarms.operationalCredentialsDelegate = &pyo->mOpCredsIssuer;

    pyo->mPrarms.storageDelegate = storage;

    Py_RETURN_NONE;
}

static PyMethodDef PythonCommissionerInitParamsMethods[] = {
    {"SetPersistentStorage", PythonCommissionerInitParamsSetPersistentStorage, METH_VARARGS, SetPersistentStorageDocument},
    {NULL, NULL, 0, NULL}
};

PyTypeObject PythonCommissionerInitParamsType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "chip.CommissionerInitParams",        /* tp_name */
    sizeof(PythonCommissionerInitParams), /* tp_basicsize */
    0,                                /* tp_itemsize */
    PythonCommissionerInitParamsDealloc,  /* tp_dealloc */
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
    0,                                /* tp_flags */
    0,                                /* tp_doc */
    0,                                /* tp_traverse */
    0,                                /* tp_clear */
    0,                                /* tp_richcompare */
    0,                                /* tp_weaklistoffset */
    0,                                /* tp_iter */
    0,                                /* tp_iternext */
    PythonCommissionerInitParamsMethods,  /* tp_methods */
    0,                                /* tp_members */
    0,                                /* tp_getset */
    0,                                /* tp_base */
    0,                                /* tp_dict */
    0,                                /* tp_descr_get */
    0,                                /* tp_descr_set */
    0,                                /* tp_dictoffset */
    0,                                /* tp_init */
    0,                                /* tp_alloc */
    PythonCommissionerInitParamsNew,      /* tp_new */
};
