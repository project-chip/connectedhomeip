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
#include <functional>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>

#include <pybind11/pybind11.h>

typedef std::function<pybind11::module &(std::string const &)> ModuleGetter;

void bind_PyChip_ErrorStr(std::function<pybind11::module &(std::string const & namespace_)> & M);
void bind_PyChip_ChipError(std::function<pybind11::module &(std::string const & namespace_)> & M);

PYBIND11_MODULE(PyChip, root_module)
{
    root_module.doc() = "PyChip module";

    std::map<std::string, pybind11::module> modules;
    ModuleGetter M = [&](std::string const & namespace_) -> pybind11::module & {
        auto it = modules.find(namespace_);
        if (it == modules.end())
            throw std::runtime_error("Attempt to access pybind11::module for namespace " + namespace_ + " that does not exist!!!");
        return it->second;
    };

    modules[""] = root_module;

    std::vector<std::pair<std::string, std::string>> sub_modules{ { "", "chip" } };
    for (auto & p : sub_modules)
        modules[p.first.size() ? p.first + "::" + p.second : p.second] =
            modules[p.first].def_submodule(p.second.c_str(), ("Bindings for " + p.first + "::" + p.second + " namespace").c_str());

    bind_PyChip_ErrorStr(M);
    bind_PyChip_ChipError(M);
}
