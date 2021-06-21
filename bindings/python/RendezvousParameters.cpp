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

#include <RendezvousParameters.h>

#include <support/ErrorStr.h>

#include <Converter.h>
#include <TransportType.h>

static PyObject * PythonRendezvousParametersNew(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PythonRendezvousParameters *pyo = reinterpret_cast<PythonRendezvousParameters*>(type->tp_alloc(type, 0));
    new (&pyo->mParameters) chip::RendezvousParameters();
    return (PyObject *)pyo;
}

static void PythonRendezvousParametersDealloc(PyObject *self)
{
    PythonRendezvousParameters *pyo = reinterpret_cast<PythonRendezvousParameters*>(self);
    pyo->mParameters.~RendezvousParameters();

    PyTypeObject *tp = Py_TYPE(self);
    tp->tp_free(self);
}

PyDoc_STRVAR(SetSetupPINCodeDocument, "SetSetupPINCode(pin: unsigned long long)");
static PyObject * PythonRendezvousParametersSetSetupPINCode(PyObject *self, PyObject *args)
{
    PythonRendezvousParameters *pyo = reinterpret_cast<PythonRendezvousParameters*>(self);

    unsigned long long pin;

    // 1nd arg (I: (int) [unsigned int])
    if (!PyArg_ParseTuple(args, "K", &pin)) {
        return nullptr;
    }

    pyo->mParameters.SetSetupPINCode(pin);

    Py_RETURN_NONE;
}

PyDoc_STRVAR(SetPeerAddressDocument, "SetPeerAddress(type: int, address: IPv4Address or IPv6Address, port: unsigned short)");
static PyObject * PythonRendezvousParametersSetPeerAddress(PyObject *self, PyObject *args)
{
    PythonRendezvousParameters *pyo = reinterpret_cast<PythonRendezvousParameters*>(self);

    chip::Transport::Type type;
    chip::Inet::IPAddress address;
    unsigned short port;

    // O& (object) [converter, anything]
    // H (int) [unsigned short int]
    if (!PyArg_ParseTuple(args, "O&O&H", &PyObjectToTransportType, &type, &PyObjectToAddress, &address, &port)) {
        return nullptr;
    }

    chip::Transport::PeerAddress peer;
    peer.SetTransportType(type);
    peer.SetIPAddress(address);
    peer.SetPort(port);
    pyo->mParameters.SetPeerAddress(peer);

    Py_RETURN_NONE;
}

static PyMethodDef PythonRendezvousParametersMethods[] = {
    {"SetSetupPINCode", PythonRendezvousParametersSetSetupPINCode, METH_VARARGS, SetSetupPINCodeDocument},
    {"SetPeerAddress", PythonRendezvousParametersSetPeerAddress, METH_VARARGS, SetPeerAddressDocument},
    {NULL, NULL, 0, NULL}
};

PyTypeObject PythonRendezvousParametersType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "chip.RendezvousParameters",        /* tp_name */
    sizeof(PythonRendezvousParameters), /* tp_basicsize */
    0,                                /* tp_itemsize */
    PythonRendezvousParametersDealloc,  /* tp_dealloc */
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
    PythonRendezvousParametersMethods,  /* tp_methods */
    0,                                /* tp_members */
    0,                                /* tp_getset */
    0,                                /* tp_base */
    0,                                /* tp_dict */
    0,                                /* tp_descr_get */
    0,                                /* tp_descr_set */
    0,                                /* tp_dictoffset */
    0,                                /* tp_init */
    0,                                /* tp_alloc */
    PythonRendezvousParametersNew,      /* tp_new */
};
