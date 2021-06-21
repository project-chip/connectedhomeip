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

#include <core/CHIPError.h>
#include <support/CHIPMem.h>
#include <support/ErrorStr.h>

#include <CommissionerInitParams.h>
#include <DeviceCommissioner.h>
#include <PersistentStorage.h>
#include <RendezvousParameters.h>
#include <TransportType.h>

PyDoc_STRVAR(ChipInitDocument, "");
static PyObject * ChipInit(PyObject * self, PyObject * args)
{
    CHIP_ERROR err = chip::Platform::MemoryInit();
    if (err != CHIP_NO_ERROR)
    {
        PyErr_Format(PyExc_RuntimeError, "Init Memory failure: %s", chip::ErrorStr(err));
        return nullptr;
    }

    Py_RETURN_NONE;
}

static PyMethodDef ChipMethods[] = { { "Init", ChipInit, METH_NOARGS, ChipInitDocument }, { NULL, NULL, 0, NULL } };

PyDoc_STRVAR(ChipDocument, "");
static struct PyModuleDef PythonChipModule = {
    PyModuleDef_HEAD_INIT,
    "ChipBindings", /* m_name */
    ChipDocument,   /* m_doc */
    -1,             /* m_size */
    ChipMethods,    /* m_methods */
    NULL,           /* m_reload */
    NULL,           /* m_traverse */
    NULL,           /* m_clear */
    NULL            /* m_free */
};

PyMODINIT_FUNC PyInit_chip(void)
{
    PyObject * module = PyModule_Create(&PythonChipModule);
    if (!module)
    {
        return NULL;
    }

    // CommissionerInitParams
    if (PyType_Ready(&PythonCommissionerInitParamsType) < 0)
    {
        return NULL;
    }
    Py_INCREF(&PythonCommissionerInitParamsType);
    PyModule_AddObject(module, "CommissionerInitParams", (PyObject *) &PythonCommissionerInitParamsType);

    // DeviceCommissioner
    if (PyType_Ready(&PythonDeviceCommissionerType) < 0)
    {
        return NULL;
    }
    Py_INCREF(&PythonDeviceCommissionerType);
    PyModule_AddObject(module, "DeviceCommissioner", (PyObject *) &PythonDeviceCommissionerType);

    // PersistentStorage
    if (PyType_Ready(&PythonPersistentStorageType) < 0)
    {
        return NULL;
    }
    Py_INCREF(&PythonPersistentStorageType);
    PyModule_AddObject(module, "PersistentStorageInterface", (PyObject *) &PythonPersistentStorageType);

    // TransportType
    if (PyType_Ready(&PythonTransportTypeType) < 0)
    {
        return NULL;
    }
    Py_INCREF(&PythonTransportTypeType);
    PyObject * pyPythonTransportTypeType = reinterpret_cast<PyObject *>(&PythonTransportTypeType);
    PyModule_AddObject(module, "TransportType", pyPythonTransportTypeType);

    // TransportType enums
    PythonTransportType * udp = PyObject_New(PythonTransportType, &PythonTransportTypeType);
    udp->mType                = chip::Transport::Type::kUdp;
    PythonTransportType * tcp = PyObject_New(PythonTransportType, &PythonTransportTypeType);
    tcp->mType                = chip::Transport::Type::kTcp;
    PyObject * transport      = Py_BuildValue("{s:N,s:N}", "UDP", udp, "TCP", tcp);
    PyModule_AddObject(module, "Transport", transport);

    // RendezvousParameters
    if (PyType_Ready(&PythonRendezvousParametersType) < 0)
    {
        return NULL;
    }
    Py_INCREF(&PythonRendezvousParametersType);
    PyModule_AddObject(module, "RendezvousParameters", (PyObject *) &PythonRendezvousParametersType);

    return module;
}
