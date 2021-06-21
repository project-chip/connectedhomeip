#include <core/CHIPError.h>
#include <sstream> // __str__
#include <support/TypeTraits.h>

#include <functional>
#include <pybind11/pybind11.h>
#include <string>

#ifndef BINDER_PYBIND11_TYPE_CASTER
#define BINDER_PYBIND11_TYPE_CASTER
PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>)
PYBIND11_DECLARE_HOLDER_TYPE(T, T *)
PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_PyChip_ChipError(std::function<pybind11::module &(std::string const & namespace_)> & M)
{
    { // chip::ChipError file:core/CHIPError.h line:50
        pybind11::class_<chip::ChipError, std::shared_ptr<chip::ChipError>> cl(
            M("chip"), "ChipError",
            "This is a helper class for managing `CHIP_ERROR` numbers.\n\n At the top level, an error belongs to a `Range` and has "
            "an integral Value whose meaning depends on the `Range`.\n One, `Range::kSDK`, is used for the CHIP SDK's own errors; "
            "others encapsulate error codes from external sources\n (e.g. libraries, OS) into a `CHIP_ERROR`.\n\n CHIP SDK errors "
            "inside `Range::kSDK` consist of a component identifier given by `SdkPart` and an arbitrary small\n integer Code.");
        cl.def(pybind11::init([]() { return new chip::ChipError(); }));
        cl.def(pybind11::init<unsigned int>(), pybind11::arg("error"));

        cl.def(pybind11::init([](chip::ChipError const & o) { return new chip::ChipError(o); }));

        pybind11::enum_<chip::ChipError::Range>(cl, "Range", "Top-level error classification.")
            .value("kSDK", chip::ChipError::Range::kSDK)
            .value("kOS", chip::ChipError::Range::kOS)
            .value("kPOSIX", chip::ChipError::Range::kPOSIX)
            .value("kLwIP", chip::ChipError::Range::kLwIP)
            .value("kOpenThread", chip::ChipError::Range::kOpenThread)
            .value("kPlatform", chip::ChipError::Range::kPlatform);

        pybind11::enum_<chip::ChipError::SdkPart>(cl, "SdkPart", "Secondary classification of errors in `Range::kSDK`.")
            .value("kCore", chip::ChipError::SdkPart::kCore)
            .value("kInet", chip::ChipError::SdkPart::kInet)
            .value("kDevice", chip::ChipError::SdkPart::kDevice)
            .value("kASN1", chip::ChipError::SdkPart::kASN1)
            .value("kBLE", chip::ChipError::SdkPart::kBLE)
            .value("kApplication", chip::ChipError::SdkPart::kApplication);

        cl.def("__eq__", (bool (chip::ChipError::*)(const class chip::ChipError &) const) & chip::ChipError::operator==,
               "C++: chip::ChipError::operator==(const class chip::ChipError &) const --> bool", pybind11::arg("other"));
        cl.def("__ne__", (bool (chip::ChipError::*)(const class chip::ChipError &) const) & chip::ChipError::operator!=,
               "C++: chip::ChipError::operator!=(const class chip::ChipError &) const --> bool", pybind11::arg("other"));
        cl.def_static("IsSuccess", (bool (*)(unsigned int)) & chip::ChipError::IsSuccess,
                      "C++: chip::ChipError::IsSuccess(unsigned int) --> bool", pybind11::arg("error"));
        cl.def_static("IsSuccess", (bool (*)(class chip::ChipError)) & chip::ChipError::IsSuccess,
                      "C++: chip::ChipError::IsSuccess(class chip::ChipError) --> bool", pybind11::arg("error"));
    }
    // chip::RegisterCHIPLayerErrorFormatter() file:core/CHIPError.h line:2052
    M("chip").def("RegisterCHIPLayerErrorFormatter", (void (*)()) & chip::RegisterCHIPLayerErrorFormatter,
                  "C++: chip::RegisterCHIPLayerErrorFormatter() --> void");

    // chip::FormatCHIPError(char *, unsigned short, unsigned int) file:core/CHIPError.h line:2053
    M("chip").def("FormatCHIPError", (bool (*)(char *, unsigned short, unsigned int)) & chip::FormatCHIPError,
                  "C++: chip::FormatCHIPError(char *, unsigned short, unsigned int) --> bool", pybind11::arg("buf"),
                  pybind11::arg("bufSize"), pybind11::arg("err"));
}
