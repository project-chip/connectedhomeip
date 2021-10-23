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
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>
#include <sstream>

#include <functional>
#include <pybind11/pybind11.h>
#include <string>

#ifndef BINDER_PYBIND11_TYPE_CASTER
#define BINDER_PYBIND11_TYPE_CASTER
PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>)
PYBIND11_DECLARE_HOLDER_TYPE(T, T *)
PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_PyChip_ErrorStr(std::function<pybind11::module &(std::string const & namespace_)> & M)
{
    {
        pybind11::class_<chip::ErrorFormatter, std::shared_ptr<chip::ErrorFormatter>> cl(M("chip"), "ErrorFormatter", "");
        cl.def(pybind11::init([]() { return new chip::ErrorFormatter(); }));
    }
    M("chip").def("ErrorStr", (const char * (*) (CHIP_ERROR)) & chip::ErrorStr,
                  "C++: chip::ErrorStr(unsigned int) --> const char *", pybind11::return_value_policy::automatic,
                  pybind11::arg("err"));

    M("chip").def("RegisterErrorFormatter", (void (*)(struct chip::ErrorFormatter *)) & chip::RegisterErrorFormatter,
                  "C++: chip::RegisterErrorFormatter(struct chip::ErrorFormatter *) --> void", pybind11::arg("errFormatter"));

    M("chip").def("DeregisterErrorFormatter", (void (*)(struct chip::ErrorFormatter *)) & chip::DeregisterErrorFormatter,
                  "C++: chip::DeregisterErrorFormatter(struct chip::ErrorFormatter *) --> void", pybind11::arg("errFormatter"));

    M("chip").def("FormatError", (void (*)(char *, unsigned short, const char *, unsigned int, const char *)) & chip::FormatError,
                  "C++: chip::FormatError(char *, unsigned short, const char *, unsigned int, const char *) --> void",
                  pybind11::arg("buf"), pybind11::arg("bufSize"), pybind11::arg("subsys"), pybind11::arg("err"),
                  pybind11::arg("desc"));
}
