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

#include <DeviceCommissioner.h>

#include <controller/CHIPDeviceController.h>
#include <support/ErrorStr.h>

#include <CommissionerInitParams.h>
#include <RendezvousParameters.h>

static PyObject * PythonDeviceCommissionerNew(PyTypeObject * type, PyObject * args, PyObject * kwds)
{
    uint64_t nodeId;
    PyObject * initParameters;

    // 1st arg (K: uint64_t): node id
    // 2nd arg (O!: PyObject *): init parameters
    if (!PyArg_ParseTuple(args, "KO!", &nodeId, &PythonCommissionerInitParamsType, &initParameters))
    {
        return nullptr;
    }

    PythonDeviceCommissioner * pyo = reinterpret_cast<PythonDeviceCommissioner *>(type->tp_alloc(type, 0));

    new (&pyo->mCommissioner) chip::Controller::DeviceCommissioner();
    CHIP_ERROR err = pyo->mCommissioner.Init(nodeId, reinterpret_cast<PythonCommissionerInitParams *>(initParameters)->mPrarms);
    if (err != CHIP_NO_ERROR)
    {
        pyo->~PythonDeviceCommissioner();
        PyErr_Format(PyExc_RuntimeError, "DeviceCommissioner Init failed: %s.", chip::ErrorStr(err));
        type->tp_free(pyo);
        return nullptr;
    }
    return (PyObject *) pyo;
}

static void PythonDeviceCommissionerDealloc(PyObject * self)
{
    PythonDeviceCommissioner * pyo = reinterpret_cast<PythonDeviceCommissioner *>(self);
    pyo->mCommissioner.Shutdown();
    pyo->mCommissioner.~DeviceCommissioner();

    PyTypeObject * tp = Py_TYPE(self);
    tp->tp_free(self);
}

PyDoc_STRVAR(ServiceEventsDocument, "ServiceEvents()");
static PyObject * PythonDeviceCommissionerServiceEvents(PyObject * self, PyObject * args)
{
    PythonDeviceCommissioner * pyo = reinterpret_cast<PythonDeviceCommissioner *>(self);
    pyo->mCommissioner.ServiceEvents();
    Py_RETURN_NONE;
}

PyDoc_STRVAR(PairDeviceDocument, "PairDevice(remoteDeviceId: long long, parameters: RendezvousParameters)");
static PyObject * PythonDeviceCommissionerPairDevice(PyObject * self, PyObject * args)
{
    PythonDeviceCommissioner * pyo = reinterpret_cast<PythonDeviceCommissioner *>(self);

    unsigned long long remoteDeviceId;
    PyObject * pyRendezvousParameters;

    // K (int) [unsigned long long]
    // O! (object) [typeobject, PyObject *]
    if (!PyArg_ParseTuple(args, "KO!", &remoteDeviceId, &PythonRendezvousParametersType, &pyRendezvousParameters))
    {
        return nullptr;
    }

    pyo->mCommissioner.PairDevice(remoteDeviceId,
                                  reinterpret_cast<PythonRendezvousParameters *>(pyRendezvousParameters)->mParameters);

    Py_RETURN_NONE;
}

static PyMethodDef PythonDeviceCommissionerMethods[] = {
    { "ServiceEvents", PythonDeviceCommissionerServiceEvents, METH_NOARGS, ServiceEventsDocument },
    { "PairDevice", PythonDeviceCommissionerPairDevice, METH_VARARGS, PairDeviceDocument },
    { NULL, NULL, 0, NULL }
};

PyTypeObject PythonDeviceCommissionerType = {
    PyVarObject_HEAD_INIT(NULL, 0) "chip.DeviceCommissioner", /* tp_name */
    sizeof(PythonDeviceCommissioner),                         /* tp_basicsize */
    0,                                                        /* tp_itemsize */
    PythonDeviceCommissionerDealloc,                          /* tp_dealloc */
    0,                                                        /* tp_vectorcall_offset */
    0,                                                        /* tp_getattr */
    0,                                                        /* tp_setattr */
    0,                                                        /* tp_as_async */
    0,                                                        /* tp_repr */
    0,                                                        /* tp_as_number */
    0,                                                        /* tp_as_sequence */
    0,                                                        /* tp_as_mapping */
    0,                                                        /* tp_hash */
    0,                                                        /* tp_call */
    0,                                                        /* tp_str */
    0,                                                        /* tp_getattro */
    0,                                                        /* tp_setattro */
    0,                                                        /* tp_as_buffer */
    0,                                                        /* tp_flags */
    0,                                                        /* tp_doc */
    0,                                                        /* tp_traverse */
    0,                                                        /* tp_clear */
    0,                                                        /* tp_richcompare */
    0,                                                        /* tp_weaklistoffset */
    0,                                                        /* tp_iter */
    0,                                                        /* tp_iternext */
    PythonDeviceCommissionerMethods,                          /* tp_methods */
    0,                                                        /* tp_members */
    0,                                                        /* tp_getset */
    0,                                                        /* tp_base */
    0,                                                        /* tp_dict */
    0,                                                        /* tp_descr_get */
    0,                                                        /* tp_descr_set */
    0,                                                        /* tp_dictoffset */
    0,                                                        /* tp_init */
    0,                                                        /* tp_alloc */
    PythonDeviceCommissionerNew,                              /* tp_new */
};
