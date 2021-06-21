#include <functional>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>

#include <pybind11/pybind11.h>

typedef std::function<pybind11::module &(std::string const &)> ModuleGetter;

void bind_CHIPController_ChipExceptions(std::function<pybind11::module &(std::string const & namespace_)> & M);

PYBIND11_MODULE(PyChip, root_module)
{
    root_module.doc() = "PyChip module";

    std::map<std::string, pybind11::module> modules;
    ModuleGetter M = [&](std::string const & namespace_) -> pybind11::module & {
        auto it = modules.find(namespace_);
        if (it == modules.end())
            throw std::runtime_error("Attempt to access pybind11::module for namespace " + namespace_ +
                                     " before it was created!!!");
        return it->second;
    };

    modules[""] = root_module;

    std::vector<std::pair<std::string, std::string>> sub_modules{
        { "", "ChipExceptions" }
    };
    for (auto & p : sub_modules)
        modules[p.first.size() ? p.first + "::" + p.second : p.second] =
            modules[p.first].def_submodule(p.second.c_str(), ("Bindings for " + p.first + "::" + p.second + " namespace").c_str());

    bind_CHIPController_ChipExceptions(M);
}
