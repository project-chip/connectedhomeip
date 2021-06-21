#include <core/CHIPError.h>
#include <sstream> // __str__
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>

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
    { // chip::ErrorFormatter file:support/ErrorStr.h line:35
        pybind11::class_<chip::ErrorFormatter, std::shared_ptr<chip::ErrorFormatter>> cl(M("chip"), "ErrorFormatter", "");
        cl.def(pybind11::init([]() { return new chip::ErrorFormatter(); }));
    }
    // chip::ErrorStr(unsigned int) file:support/ErrorStr.h line:43
    M("chip").def("ErrorStr", (const char * (*) (CHIP_ERROR)) & chip::ErrorStr,
                  "C++: chip::ErrorStr(unsigned int) --> const char *", pybind11::return_value_policy::automatic,
                  pybind11::arg("err"));

    // chip::RegisterErrorFormatter(struct chip::ErrorFormatter *) file:support/ErrorStr.h line:44
    M("chip").def("RegisterErrorFormatter", (void (*)(struct chip::ErrorFormatter *)) & chip::RegisterErrorFormatter,
                  "C++: chip::RegisterErrorFormatter(struct chip::ErrorFormatter *) --> void", pybind11::arg("errFormatter"));

    // chip::DeregisterErrorFormatter(struct chip::ErrorFormatter *) file:support/ErrorStr.h line:45
    M("chip").def("DeregisterErrorFormatter", (void (*)(struct chip::ErrorFormatter *)) & chip::DeregisterErrorFormatter,
                  "C++: chip::DeregisterErrorFormatter(struct chip::ErrorFormatter *) --> void", pybind11::arg("errFormatter"));

    // chip::FormatError(char *, unsigned short, const char *, unsigned int, const char *) file:support/ErrorStr.h line:46
    M("chip").def("FormatError", (void (*)(char *, unsigned short, const char *, unsigned int, const char *)) & chip::FormatError,
                  "C++: chip::FormatError(char *, unsigned short, const char *, unsigned int, const char *) --> void",
                  pybind11::arg("buf"), pybind11::arg("bufSize"), pybind11::arg("subsys"), pybind11::arg("err"),
                  pybind11::arg("desc"));
}
