#include <functional>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>

#include <pybind11/pybind11.h>

typedef std::function<pybind11::module &(std::string const &)> ModuleGetter;

void bind_PyChip_SetupPayload(std::function<pybind11::module &(std::string const & namespace_)> & M);
void bind_PyChip_TransportType(std::function<pybind11::module &(std::string const & namespace_)> & M);
void bind_PyChip_ControllerInitParams(std::function<pybind11::module &(std::string const & namespace_)> & M);
void bind_PyChip_Rendezvous(std::function<pybind11::module &(std::string const & namespace_)> & M);
void bind_PyChip_ControllerDeviceCommissioner(std::function<pybind11::module &(std::string const & namespace_)> & M);
void bind_CHIPController_ChipExceptions(std::function<pybind11::module &(std::string const & namespace_)> & M);
void bind_PyChip_DeviceAddressUpdateDelegate(std::function<pybind11::module &(std::string const & namespace_)> & M);
void bind_PyChip_Memory(std::function<pybind11::module &(std::string const & namespace_)> & M);
void bind_PyChip_PlatFormManager(std::function<pybind11::module &(std::string const & namespace_)> & M);
void bind_PyChip_MDNS(std::function<pybind11::module &(std::string const & namespace_)> & M);
void bind_PyChip_Inet_IPAddress(std::function<pybind11::module &(std::string const & namespace_)> & M);
void bind_PyChip_BLEManager(std::function<pybind11::module &(std::string const & namespace_)> & M);

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
        { "", "chip" },         { "", "ChipExceptions" }, { "chip", "Controller" },  { "chip", "Transport" },
        { "chip", "Platform" }, { "chip", "app" },        { "chip", "DeviceLayer" }, { "chip::DeviceLayer", "Internal" },
        { "chip", "Mdns" },     { "chip", "Inet" },
    };
    for (auto & p : sub_modules)
        modules[p.first.size() ? p.first + "::" + p.second : p.second] =
            modules[p.first].def_submodule(p.second.c_str(), ("Bindings for " + p.first + "::" + p.second + " namespace").c_str());

    bind_PyChip_SetupPayload(M);
    bind_PyChip_TransportType(M);
    bind_PyChip_ControllerInitParams(M);
    bind_PyChip_Rendezvous(M);
    bind_PyChip_ControllerDeviceCommissioner(M);
    bind_CHIPController_ChipExceptions(M);
    bind_PyChip_DeviceAddressUpdateDelegate(M);
    bind_PyChip_Memory(M);
    bind_PyChip_PlatFormManager(M);
    bind_PyChip_MDNS(M);
    bind_PyChip_Inet_IPAddress(M);
    bind_PyChip_BLEManager(M);
}
