#include <lib/support/BitFlags.h>
#include <setup_payload/ManualSetupPayloadParser.h> // chip::ManualSetupPayloadParser
#include <setup_payload/QRCodeSetupPayloadParser.h>
#include <setup_payload/SetupPayload.h> // chip::SetupPayload
#include <sstream>                      // __str__

#include <controller/CHIPDeviceController.h>
#include <pybind11/complex.h>
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <string>

#ifndef BINDER_PYBIND11_TYPE_CASTER
#define BINDER_PYBIND11_TYPE_CASTER
PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>)
PYBIND11_DECLARE_HOLDER_TYPE(T, T *)
PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_PyChip_SetupPayload(std::function<pybind11::module &(std::string const & namespace_)> & M)
{
    {
        // chip::optionalQRCodeInfoType file:setup_payload/SetupPayload.h line:106
        pybind11::enum_<chip::optionalQRCodeInfoType>(M("chip"), "optionalQRCodeInfoType", pybind11::arithmetic(), "")
            .value("optionalQRCodeInfoTypeUnknown", chip::optionalQRCodeInfoTypeUnknown)
            .value("optionalQRCodeInfoTypeString", chip::optionalQRCodeInfoTypeString)
            .value("optionalQRCodeInfoTypeInt32", chip::optionalQRCodeInfoTypeInt32)
            .value("optionalQRCodeInfoTypeInt64", chip::optionalQRCodeInfoTypeInt64)
            .value("optionalQRCodeInfoTypeUInt32", chip::optionalQRCodeInfoTypeUInt32)
            .value("optionalQRCodeInfoTypeUInt64", chip::optionalQRCodeInfoTypeUInt64)
            .export_values();
    }
    { // chip::OptionalQRCodeInfo file:setup_payload/SetupPayload.h line:119
        pybind11::class_<chip::OptionalQRCodeInfo, std::shared_ptr<chip::OptionalQRCodeInfo>> cl(
            M("chip"), "OptionalQRCodeInfo", "A structure to hold optional QR Code info");
        cl.def(pybind11::init([]() { return new chip::OptionalQRCodeInfo(); }));
        cl.def(pybind11::init([](chip::OptionalQRCodeInfo const & o) { return new chip::OptionalQRCodeInfo(o); }));
        cl.def_readwrite("tag", &chip::OptionalQRCodeInfo::tag);
        cl.def_readwrite("type", &chip::OptionalQRCodeInfo::type);
        cl.def_readwrite("data", &chip::OptionalQRCodeInfo::data);
        cl.def_readwrite("int32", &chip::OptionalQRCodeInfo::int32);
    }
    { // chip::OptionalQRCodeInfoExtension file:setup_payload/SetupPayload.h line:131
        pybind11::class_<chip::OptionalQRCodeInfoExtension, std::shared_ptr<chip::OptionalQRCodeInfoExtension>,
                         chip::OptionalQRCodeInfo>
            cl(M("chip"), "OptionalQRCodeInfoExtension", "");
        cl.def(pybind11::init([]() { return new chip::OptionalQRCodeInfoExtension(); }));
        cl.def_readwrite("int64", &chip::OptionalQRCodeInfoExtension::int64);
        cl.def_readwrite("uint32", &chip::OptionalQRCodeInfoExtension::uint32);
        cl.def_readwrite("uint64", &chip::OptionalQRCodeInfoExtension::uint64);
    }
    {
        pybind11::class_<chip::BitFlags<chip::RendezvousInformationFlag, uint8_t>>(M("chip"), "BitFlags")
            .def(pybind11::init<>())
            .def("Raw", &chip::BitFlags<chip::RendezvousInformationFlag, uint8_t>::Raw)
            .def("RendezvousInformationFlag",
                 &chip::BitFlags<chip::RendezvousInformationFlag, uint8_t>::operator chip::RendezvousInformationFlag);
    }
    {
        pybind11::enum_<chip::RendezvousInformationFlag>(M("chip"), "RendezvousInformationFlag",
                                                         "The rendezvous type this device supports.")
            .value("kNone", chip::RendezvousInformationFlag::kNone)
            .value("kSoftAP", chip::RendezvousInformationFlag::kSoftAP)
            .value("kBLE", chip::RendezvousInformationFlag::kBLE)
            .value("kOnNetwork", chip::RendezvousInformationFlag::kOnNetwork);
    }
    { // chip::CommissioningFlow file:setup_payload/SetupPayload.h line:99
        pybind11::enum_<chip::CommissioningFlow>(M("chip"), "CommissioningFlow", "")
            .value("kStandard", chip::CommissioningFlow::kStandard)
            .value("kUserActionRequired", chip::CommissioningFlow::kUserActionRequired)
            .value("kCustom", chip::CommissioningFlow::kCustom);
    }
    { // chip::SetupPayload file:setup_payload/SetupPayload.h line:149
        pybind11::class_<chip::SetupPayload, std::shared_ptr<chip::SetupPayload>> cl(M("chip"), "SetupPayload", "");
        cl.def(pybind11::init([]() { return new chip::SetupPayload(); }));
        cl.def_readwrite("version", &chip::SetupPayload::version);
        cl.def_readwrite("vendorID", &chip::SetupPayload::vendorID);
        cl.def_readwrite("productID", &chip::SetupPayload::productID);
        cl.def_readwrite("commissioningFlow", &chip::SetupPayload::commissioningFlow);
        cl.def_readwrite("rendezvousInformation", &chip::SetupPayload::rendezvousInformation);
        cl.def_readwrite("discriminator", &chip::SetupPayload::discriminator);
        cl.def_readwrite("setUpPINCode", &chip::SetupPayload::setUpPINCode);
        cl.def("addOptionalVendorData",
               (int (chip::SetupPayload::*)(unsigned char, int)) & chip::SetupPayload::addOptionalVendorData,
               "A function to add an optional vendor data\n \n\n 7 bit [0-127] tag number\n \n\n String representation of data to "
               "add\n \n\n Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise\n\nC++: "
               "chip::SetupPayload::addOptionalVendorData(unsigned char, int) --> int",
               pybind11::arg("tag"), pybind11::arg("data"));
        cl.def("addOptionalVendorData",
               (int (chip::SetupPayload::*)(unsigned char, int)) & chip::SetupPayload::addOptionalVendorData,
               "A function to add an optional vendor data\n \n\n 7 bit [0-127] tag number\n \n\n Integer representation of data to "
               "add\n \n\n Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise\n\nC++: "
               "chip::SetupPayload::addOptionalVendorData(unsigned char, int) --> int",
               pybind11::arg("tag"), pybind11::arg("data"));
        cl.def("removeOptionalVendorData",
               (int (chip::SetupPayload::*)(unsigned char)) & chip::SetupPayload::removeOptionalVendorData,
               "A function to remove an optional vendor data\n \n\n 7 bit [0-127] tag number\n \n\n Returns a "
               "CHIP_ERROR_KEY_NOT_FOUND on error, CHIP_NO_ERROR otherwise\n\nC++: "
               "chip::SetupPayload::removeOptionalVendorData(unsigned char) --> int",
               pybind11::arg("tag"));
        cl.def("getAllOptionalVendorData", &chip::SetupPayload::getAllOptionalVendorData,
               "A function to retrieve the vector of OptionalQRCodeInfo infos\n \n\n Returns a vector of "
               "optionalQRCodeInfos\n\nC++: chip::SetupPayload::getAllOptionalVendorData() const --> int");
        cl.def("addSerialNumber", pybind11::overload_cast<std::string>(&chip::SetupPayload::addSerialNumber),
               "A function to add a string serial number\n \n\n string serial number\n \n\n Returns a CHIP_ERROR on error, "
               "CHIP_NO_ERROR otherwise\n\nC++: chip::SetupPayload::addSerialNumber(int) --> int",
               pybind11::arg("serialNumber"));
        cl.def("addSerialNumber", pybind11::overload_cast<uint32_t>(&chip::SetupPayload::addSerialNumber),
               "A function to add a uint32_t serial number\n \n\n uint32_t serial number\n \n\n Returns a CHIP_ERROR on error, "
               "CHIP_NO_ERROR otherwise\n\nC++: chip::SetupPayload::addSerialNumber(unsigned int) --> int",
               pybind11::arg("serialNumber"));
        cl.def("getSerialNumber", (int (chip::SetupPayload::*)(int &) const) & chip::SetupPayload::getSerialNumber,
               "A function to retrieve serial number as a string\n \n\n retrieved string serial number\n \n\n Returns a CHIP_ERROR "
               "on error, CHIP_NO_ERROR otherwise\n\nC++: chip::SetupPayload::getSerialNumber(int &) const --> int",
               pybind11::arg("outSerialNumber"));
        cl.def("removeSerialNumber", (int (chip::SetupPayload::*)()) & chip::SetupPayload::removeSerialNumber,
               "A function to remove the serial number from the payload\n \n\n Returns a CHIP_ERROR_KEY_NOT_FOUND on error, "
               "CHIP_NO_ERROR otherwise\n\nC++: chip::SetupPayload::removeSerialNumber() --> int");
        cl.def("isValidQRCodePayload", (bool (chip::SetupPayload::*)()) & chip::SetupPayload::isValidQRCodePayload,
               "C++: chip::SetupPayload::isValidQRCodePayload() --> bool");
        cl.def("isValidManualCode", (bool (chip::SetupPayload::*)()) & chip::SetupPayload::isValidManualCode,
               "C++: chip::SetupPayload::isValidManualCode() --> bool");
        cl.def("__eq__", (bool (chip::SetupPayload::*)(class chip::SetupPayload &)) & chip::SetupPayload::operator==,
               "C++: chip::SetupPayload::operator==(class chip::SetupPayload &) --> bool", pybind11::arg("input"));
    }
    { // chip::ManualSetupPayloadParser file:setup_payload/ManualSetupPayloadParser.h line:36
        pybind11::class_<chip::ManualSetupPayloadParser, std::shared_ptr<chip::ManualSetupPayloadParser>> cl(
            M("chip"), "ManualSetupPayloadParser",
            "A class that can be used to convert a decimal encoded payload to a SetupPayload object");
        cl.def(pybind11::init<const std::string &>());
        cl.def("populatePayload",
               (int (chip::ManualSetupPayloadParser::*)(class chip::SetupPayload &)) &
                   chip::ManualSetupPayloadParser::populatePayload,
               "C++: chip::ManualSetupPayloadParser::populatePayload(class chip::SetupPayload &) --> int",
               pybind11::arg("outPayload"));
    }
    {
        pybind11::class_<chip::QRCodeSetupPayloadParser> cl(M("chip"), "QRCodeSetupPayloadParser");
        cl.def(pybind11::init<const std::string &>());
        cl.def("populatePayload", &chip::QRCodeSetupPayloadParser::populatePayload);
    }
}
