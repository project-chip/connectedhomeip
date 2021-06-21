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

#include <Converter.h>

#include <netinet/in.h>

int PyObjectToAddress(PyObject * pyo, chip::Inet::IPAddress * address)
{
    PyObject * pyVersion = PyObject_GetAttrString(pyo, "version");
    if (pyVersion == nullptr)
    {
        return false;
    }

    long version = PyLong_AsLong(pyVersion);
    Py_DECREF(pyVersion);
    if (PyErr_Occurred())
    {
        return false;
    }

    switch (version)
    {
    case 4: {
        PyObject * pyAddressBytes = PyObject_GetAttrString(pyo, "packed");
        if (pyAddressBytes == nullptr)
        {
            return false;
        }

        if (!PyBytes_Check(pyAddressBytes))
        {
            Py_DECREF(pyAddressBytes);
            return false;
        }

        in_addr addr;
        Py_ssize_t size = PyBytes_Size(pyAddressBytes);
        if (size != sizeof(addr))
        {
            Py_DECREF(pyAddressBytes);
            return false;
        }

        memcpy(&addr.s_addr, PyBytes_AsString(pyAddressBytes), sizeof(addr));
        Py_DECREF(pyAddressBytes);
        *address = chip::Inet::IPAddress::FromIPv4(addr);

        return true;
    }
    break;
    case 6: {
        PyObject * pyAddressBytes = PyObject_GetAttrString(pyo, "packed");
        if (pyAddressBytes == nullptr)
        {
            return false;
        }

        if (!PyBytes_Check(pyAddressBytes))
        {
            Py_DECREF(pyAddressBytes);
            return false;
        }

        in6_addr addr;
        Py_ssize_t size = PyBytes_Size(pyAddressBytes);
        if (size != sizeof(addr))
        {
            Py_DECREF(pyAddressBytes);
            return false;
        }

        memcpy(addr.s6_addr, PyBytes_AsString(pyAddressBytes), sizeof(addr));
        Py_DECREF(pyAddressBytes);
        *address = chip::Inet::IPAddress::FromIPv6(addr);

        return true;
    }
    break;
    default:
        return false;
    }
}
