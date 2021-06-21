#include <controller/DeviceAddressUpdateDelegate.h> // chip::Controller::DeviceAddressUpdateDelegate
#include <sstream>                                  // __str__

#include <controller/CHIPDeviceController.h>
#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <controller/OperationalCredentialsDelegate.h>
#include <core/CHIPPersistentStorageDelegate.h>
#include <core/PeerId.h>
#include <functional>
#include <pybind11/pybind11.h>
#include <string>
#include <support/Span.h>

#ifndef BINDER_PYBIND11_TYPE_CASTER
#define BINDER_PYBIND11_TYPE_CASTER
PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>)
PYBIND11_DECLARE_HOLDER_TYPE(T, T *)
PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

// chip::Controller::OperationalCredentialsDelegate file:controller/OperationalCredentialsDelegate.h line:32
struct PyCallBack_chip_Controller_OperationalCredentialsDelegate : public chip::Controller::OperationalCredentialsDelegate
{
    using chip::Controller::OperationalCredentialsDelegate::OperationalCredentialsDelegate;

    int GenerateNodeOperationalCertificate(const chip::Optional<chip::NodeId> & a0, chip::FabricId a1, const chip::ByteSpan & a2,
                                           const chip::ByteSpan & a3,
                                           chip::Callback::Callback<chip::Controller::NOCGenerated> * a4) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(
            static_cast<const chip::Controller::OperationalCredentialsDelegate *>(this), "GenerateNodeOperationalCertificate");
        if (overload)
        {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2, a3, a4);
            if (pybind11::detail::cast_is_temporary_value_reference<int>::value)
            {
                static pybind11::detail::override_caster_t<int> caster;
                return pybind11::detail::cast_ref<int>(std::move(o), caster);
            }
            else
                return pybind11::detail::cast_safe<int>(std::move(o));
        }
        pybind11::pybind11_fail(
            "Tried to call pure virtual function \"OperationalCredentialsDelegate::GenerateNodeOperationalCertificate\"");
    }

    int GetIntermediateCACertificate(chip::FabricId a0, chip::MutableByteSpan & a1) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(
            static_cast<const chip::Controller::OperationalCredentialsDelegate *>(this), "GetIntermediateCACertificate");
        if (overload)
        {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
            if (pybind11::detail::cast_is_temporary_value_reference<int>::value)
            {
                static pybind11::detail::override_caster_t<int> caster;
                return pybind11::detail::cast_ref<int>(std::move(o), caster);
            }
            else
                return pybind11::detail::cast_safe<int>(std::move(o));
        }
        pybind11::pybind11_fail(
            "Tried to call pure virtual function \"OperationalCredentialsDelegate::GetIntermediateCACertificate\"");
    }
    int GetRootCACertificate(chip::FabricId a0, chip::MutableByteSpan & a1) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(
            static_cast<const chip::Controller::OperationalCredentialsDelegate *>(this), "GetRootCACertificate");
        if (overload)
        {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
            if (pybind11::detail::cast_is_temporary_value_reference<int>::value)
            {
                static pybind11::detail::override_caster_t<int> caster;
                return pybind11::detail::cast_ref<int>(std::move(o), caster);
            }
            else
                return pybind11::detail::cast_safe<int>(std::move(o));
        }
        pybind11::pybind11_fail("Tried to call pure virtual function \"OperationalCredentialsDelegate::GetRootCACertificate\"");
    }
};

// chip::Controller::DeviceAddressUpdateDelegate file:controller/DeviceAddressUpdateDelegate.h line:28
struct PyCallBack_chip_Controller_DeviceAddressUpdateDelegate : public chip::Controller::DeviceAddressUpdateDelegate
{
    using chip::Controller::DeviceAddressUpdateDelegate::DeviceAddressUpdateDelegate;

    void OnAddressUpdateComplete(chip::NodeId a0, CHIP_ERROR a1) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(
            static_cast<const chip::Controller::DeviceAddressUpdateDelegate *>(this), "OnAddressUpdateComplete");
        if (overload)
        {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
            if (pybind11::detail::cast_is_temporary_value_reference<void>::value)
            {
                static pybind11::detail::override_caster_t<void> caster;
                return pybind11::detail::cast_ref<void>(std::move(o), caster);
            }
            else
                return pybind11::detail::cast_safe<void>(std::move(o));
        }
        pybind11::pybind11_fail("Tried to call pure virtual function \"DeviceAddressUpdateDelegate::OnAddressUpdateComplete\"");
    }
};
// chip::Controller::ExampleOperationalCredentialsIssuer file:controller/ExampleOperationalCredentialsIssuer.h line:41
struct PyCallBack_chip_Controller_ExampleOperationalCredentialsIssuer : public chip::Controller::ExampleOperationalCredentialsIssuer
{
    using chip::Controller::ExampleOperationalCredentialsIssuer::ExampleOperationalCredentialsIssuer;
    // onst Optional<NodeId> & nodeId, FabricId fabricId, const ByteSpan & csr,
    //   const ByteSpan & DAC, Callback::Callback<NOCGenerated> * onNOCGenerated
    int GenerateNodeOperationalCertificate(const chip::Optional<chip::NodeId> & a0, const chip::FabricId a1,
                                           const chip::ByteSpan & a2, const chip::ByteSpan & a3,
                                           chip::Callback::Callback<chip::Controller::NOCGenerated> * a4) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(
            static_cast<const chip::Controller::ExampleOperationalCredentialsIssuer *>(this), "GenerateNodeOperationalCertificate");
        if (overload)
        {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2, a3, a4);
            if (pybind11::detail::cast_is_temporary_value_reference<int>::value)
            {
                static pybind11::detail::override_caster_t<int> caster;
                return pybind11::detail::cast_ref<int>(std::move(o), caster);
            }
            else
                return pybind11::detail::cast_safe<int>(std::move(o));
        }
        pybind11::pybind11_fail(
            "Tried to call pure virtual function \"ExampleOperationalCredentialsIssuer::GenerateNodeOperationalCertificate\"");
    }
    int GetRootCACertificate(chip::FabricId a0, chip::MutableByteSpan & a1) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(
            static_cast<const chip::Controller::ExampleOperationalCredentialsIssuer *>(this), "GetRootCACertificate");
        if (overload)
        {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
            if (pybind11::detail::cast_is_temporary_value_reference<int>::value)
            {
                static pybind11::detail::override_caster_t<int> caster;
                return pybind11::detail::cast_ref<int>(std::move(o), caster);
            }
            else
                return pybind11::detail::cast_safe<int>(std::move(o));
        }
        pybind11::pybind11_fail(
            "Tried to call pure virtual function \"ExampleOperationalCredentialsIssuer::GetRootCACertificate\"");
    }
};
// chip::PersistentStorageDelegate file:core/CHIPPersistentStorageDelegate.h line:26
struct PyCallBack_chip_PersistentStorageDelegate : public chip::PersistentStorageDelegate
{
    using chip::PersistentStorageDelegate::PersistentStorageDelegate;

    int SyncGetKeyValue(const char * a0, void * a1, uint16_t & a2) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload =
            pybind11::get_overload(static_cast<const chip::PersistentStorageDelegate *>(this), "SyncGetKeyValue");
        if (overload)
        {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2);
            if (pybind11::detail::cast_is_temporary_value_reference<int>::value)
            {
                static pybind11::detail::override_caster_t<int> caster;
                return pybind11::detail::cast_ref<int>(std::move(o), caster);
            }
            else
                return pybind11::detail::cast_safe<int>(std::move(o));
        }
        pybind11::pybind11_fail("Tried to call pure virtual function \"PersistentStorageDelegate::SyncGetKeyValue\"");
    }
    int SyncSetKeyValue(const char * a0, const void * a1, uint16_t a2) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload =
            pybind11::get_overload(static_cast<const chip::PersistentStorageDelegate *>(this), "SyncSetKeyValue");
        if (overload)
        {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2);
            if (pybind11::detail::cast_is_temporary_value_reference<int>::value)
            {
                static pybind11::detail::override_caster_t<int> caster;
                return pybind11::detail::cast_ref<int>(std::move(o), caster);
            }
            else
                return pybind11::detail::cast_safe<int>(std::move(o));
        }
        pybind11::pybind11_fail("Tried to call pure virtual function \"PersistentStorageDelegate::SyncSetKeyValue\"");
    }
    int SyncDeleteKeyValue(const char * a0) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload =
            pybind11::get_overload(static_cast<const chip::PersistentStorageDelegate *>(this), "SyncDeleteKeyValue");
        if (overload)
        {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
            if (pybind11::detail::cast_is_temporary_value_reference<int>::value)
            {
                static pybind11::detail::override_caster_t<int> caster;
                return pybind11::detail::cast_ref<int>(std::move(o), caster);
            }
            else
                return pybind11::detail::cast_safe<int>(std::move(o));
        }
        pybind11::pybind11_fail("Tried to call pure virtual function \"PersistentStorageDelegate::SyncDeleteKeyValue\"");
    }
};
void bind_PyChip_DeviceAddressUpdateDelegate(std::function<pybind11::module &(std::string const & namespace_)> & M)
{
    { // chip::Controller::OperationalCredentialsDelegate file:controller/OperationalCredentialsDelegate.h line:32
        pybind11::class_<chip::Controller::OperationalCredentialsDelegate,
                         std::shared_ptr<chip::Controller::OperationalCredentialsDelegate>,
                         PyCallBack_chip_Controller_OperationalCredentialsDelegate>
            cl(M("chip::Controller"), "OperationalCredentialsDelegate", "Callbacks for CHIP operational credentials generation");
        cl.def(pybind11::init([]() { return new PyCallBack_chip_Controller_OperationalCredentialsDelegate(); }));
        cl.def(pybind11::init<PyCallBack_chip_Controller_OperationalCredentialsDelegate const &>());
        cl.def(
            "GenerateNodeOperationalCertificate",
            (int (chip::Controller::OperationalCredentialsDelegate::*)(const class chip::PeerId &,
                                                                       const class chip::Span<const unsigned char> &, long long,
                                                                       unsigned char *, unsigned int, unsigned int &)) &
                chip::Controller::OperationalCredentialsDelegate::GenerateNodeOperationalCertificate,
            "This function generates an operational certificate for the given node.\n   The API generates the certificate in X.509 "
            "DER format.\n\n   The delegate is expected to use the certificate authority whose certificate\n   is returned in "
            "`GetIntermediateCACertificate()` or `GetRootCACertificate()`\n   API calls.\n\n \n       Node ID and Fabric ID of the "
            "target device.\n \n\n          Certificate Signing Request from the node in DER format.\n \n\n Serial number to "
            "assign to the new certificate.\n \n\n      The API will fill in the generated cert in this buffer. The buffer is "
            "allocated by the caller.\n \n\n  The size of certBuf buffer.\n \n\n  The size of the actual certificate that was "
            "written in the certBuf.\n\n \n CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.\n\nC++: "
            "chip::Controller::OperationalCredentialsDelegate::GenerateNodeOperationalCertificate(const class chip::PeerId &, "
            "const class chip::Span<const unsigned char> &, long long, unsigned char *, unsigned int, unsigned int &) --> int",
            pybind11::arg("peerId"), pybind11::arg("csr"), pybind11::arg("serialNumber"), pybind11::arg("certBuf"),
            pybind11::arg("certBufSize"), pybind11::arg("outCertLen"));
        cl.def(
            "GetIntermediateCACertificate",
            (int (chip::Controller::OperationalCredentialsDelegate::*)(unsigned long long, unsigned char *, unsigned int,
                                                                       unsigned int &)) &
                chip::Controller::OperationalCredentialsDelegate::GetIntermediateCACertificate,
            "This function returns the intermediate certificate authority (ICA) certificate corresponding to the\n   provided "
            "fabric ID. Intermediate certificate authority is optional. If the controller\n   application does not require ICA, "
            "this API call will return `CHIP_ERROR_INTERMEDIATE_CA_NOT_REQUIRED`.\n\n   The returned certificate is in X.509 DER "
            "format.\n\n \n    Fabric ID for which the certificate is being requested.\n \n\n     The API will fill in the cert in "
            "this buffer. The buffer is allocated by the caller.\n \n\n The size of certBuf buffer.\n \n\n The size of the actual "
            "certificate that was written in the certBuf.\n\n \n CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error "
            "code.\n         CHIP_ERROR_INTERMEDIATE_CA_NOT_REQUIRED is not a critical error. It indicates that ICA is not "
            "needed.\n\nC++: chip::Controller::OperationalCredentialsDelegate::GetIntermediateCACertificate(unsigned long long, "
            "unsigned char *, unsigned int, unsigned int &) --> int",
            pybind11::arg("fabricId"), pybind11::arg("certBuf"), pybind11::arg("certBufSize"), pybind11::arg("outCertLen"));
        cl.def("GetRootCACertificate",
               (int (chip::Controller::OperationalCredentialsDelegate::*)(unsigned long long, unsigned char *, unsigned int,
                                                                          unsigned int &)) &
                   chip::Controller::OperationalCredentialsDelegate::GetRootCACertificate,
               "This function returns the root certificate authority (root CA) certificate corresponding to the\n   provided "
               "fabric ID.\n\n   The returned certificate is in X.509 DER format.\n\n \n    Fabric ID for which the certificate is "
               "being requested.\n \n\n     The API will fill in the cert in this buffer. The buffer is allocated by the caller.\n "
               "\n\n The size of certBuf buffer.\n \n\n The size of the actual certificate that was written in the certBuf.\n\n \n "
               "CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.\n\nC++: "
               "chip::Controller::OperationalCredentialsDelegate::GetRootCACertificate(unsigned long long, unsigned char *, "
               "unsigned int, unsigned int &) --> int",
               pybind11::arg("fabricId"), pybind11::arg("certBuf"), pybind11::arg("certBufSize"), pybind11::arg("outCertLen"));
        cl.def("assign",
               (class chip::Controller::OperationalCredentialsDelegate &
                (chip::Controller::OperationalCredentialsDelegate::*) (const class chip::Controller::
                                                                           OperationalCredentialsDelegate &) ) &
                   chip::Controller::OperationalCredentialsDelegate::operator=,
               "C++: chip::Controller::OperationalCredentialsDelegate::operator=(const class "
               "chip::Controller::OperationalCredentialsDelegate &) --> class chip::Controller::OperationalCredentialsDelegate &",
               pybind11::return_value_policy::automatic, pybind11::arg(""));
    }
    { // chip::Controller::DeviceAddressUpdateDelegate file:controller/DeviceAddressUpdateDelegate.h line:28
        pybind11::class_<chip::Controller::DeviceAddressUpdateDelegate,
                         std::shared_ptr<chip::Controller::DeviceAddressUpdateDelegate>,
                         PyCallBack_chip_Controller_DeviceAddressUpdateDelegate>
            cl(M("chip::Controller"), "DeviceAddressUpdateDelegate", "Callbacks for CHIP device address resolution");
        cl.def(pybind11::init([]() { return new PyCallBack_chip_Controller_DeviceAddressUpdateDelegate(); }));
        cl.def("OnAddressUpdateComplete",
               (void (chip::Controller::DeviceAddressUpdateDelegate::*)(unsigned long long, int)) &
                   chip::Controller::DeviceAddressUpdateDelegate::OnAddressUpdateComplete,
               "C++: chip::Controller::DeviceAddressUpdateDelegate::OnAddressUpdateComplete(unsigned long long, int) --> void",
               pybind11::arg("nodeId"), pybind11::arg("error"));
        cl.def("assign",
               (class chip::Controller::DeviceAddressUpdateDelegate &
                (chip::Controller::DeviceAddressUpdateDelegate::*) (const class chip::Controller::DeviceAddressUpdateDelegate &) ) &
                   chip::Controller::DeviceAddressUpdateDelegate::operator=,
               "C++: chip::Controller::DeviceAddressUpdateDelegate::operator=(const class "
               "chip::Controller::DeviceAddressUpdateDelegate &) --> class chip::Controller::DeviceAddressUpdateDelegate &",
               pybind11::return_value_policy::automatic, pybind11::arg(""));
    }
    { // chip::Controller::ExampleOperationalCredentialsIssuer file:controller/ExampleOperationalCredentialsIssuer.h line:41
        pybind11::class_<chip::Controller::ExampleOperationalCredentialsIssuer,
                         std::shared_ptr<chip::Controller::ExampleOperationalCredentialsIssuer>,
                         PyCallBack_chip_Controller_ExampleOperationalCredentialsIssuer,
                         chip::Controller::OperationalCredentialsDelegate>
            cl(M("chip::Controller"), "ExampleOperationalCredentialsIssuer", "");
        cl.def(pybind11::init([]() { return new chip::Controller::ExampleOperationalCredentialsIssuer(); },
                              []() { return new PyCallBack_chip_Controller_ExampleOperationalCredentialsIssuer(); }));
        cl.def(pybind11::init([](PyCallBack_chip_Controller_ExampleOperationalCredentialsIssuer const & o) {
            return new PyCallBack_chip_Controller_ExampleOperationalCredentialsIssuer(o);
        }));
        cl.def(pybind11::init([](chip::Controller::ExampleOperationalCredentialsIssuer const & o) {
            return new chip::Controller::ExampleOperationalCredentialsIssuer(o);
        }));
        cl.def("GenerateNodeOperationalCertificate",
               (int (chip::Controller::ExampleOperationalCredentialsIssuer::*)(
                   const class chip::PeerId &, const class chip::Span<const unsigned char> &, long long, unsigned char *,
                   unsigned int, unsigned int &)) &
                   chip::Controller::ExampleOperationalCredentialsIssuer::GenerateNodeOperationalCertificate,
               "C++: chip::Controller::ExampleOperationalCredentialsIssuer::GenerateNodeOperationalCertificate(const class "
               "chip::PeerId &, const class chip::Span<const unsigned char> &, long long, unsigned char *, unsigned int, unsigned "
               "int &) --> int",
               pybind11::arg("peerId"), pybind11::arg("csr"), pybind11::arg("serialNumber"), pybind11::arg("certBuf"),
               pybind11::arg("certBufSize"), pybind11::arg("outCertLen"));
        cl.def("GetRootCACertificate",
               (int (chip::Controller::ExampleOperationalCredentialsIssuer::*)(unsigned long long, unsigned char *, unsigned int,
                                                                               unsigned int &)) &
                   chip::Controller::ExampleOperationalCredentialsIssuer::GetRootCACertificate,
               "C++: chip::Controller::ExampleOperationalCredentialsIssuer::GetRootCACertificate(unsigned long long, unsigned char "
               "*, unsigned int, unsigned int &) --> int",
               pybind11::arg("fabricId"), pybind11::arg("certBuf"), pybind11::arg("certBufSize"), pybind11::arg("outCertLen"));
        cl.def("Initialize",
               (int (chip::Controller::ExampleOperationalCredentialsIssuer::*)(class chip::PersistentStorageDelegate &)) &
                   chip::Controller::ExampleOperationalCredentialsIssuer::Initialize,
               "Initialize the issuer with the keypair in the storage.\n        If the storage doesn't have one, it'll create one, "
               "and it to the storage.\n\n \n  A reference to the storage, where the keypair is stored.\n                     The "
               "object of ExampleOperationalCredentialsIssuer doesn't hold\n                     on the reference of storage.\n\n "
               "\n Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise\n\nC++: "
               "chip::Controller::ExampleOperationalCredentialsIssuer::Initialize(class chip::PersistentStorageDelegate &) --> int",
               pybind11::arg("storage"));
        cl.def("SetIssuerId",
               (void (chip::Controller::ExampleOperationalCredentialsIssuer::*)(unsigned int)) &
                   chip::Controller::ExampleOperationalCredentialsIssuer::SetIssuerId,
               "C++: chip::Controller::ExampleOperationalCredentialsIssuer::SetIssuerId(unsigned int) --> void",
               pybind11::arg("id"));
        cl.def("SetCurrentEpoch",
               (void (chip::Controller::ExampleOperationalCredentialsIssuer::*)(unsigned int)) &
                   chip::Controller::ExampleOperationalCredentialsIssuer::SetCurrentEpoch,
               "C++: chip::Controller::ExampleOperationalCredentialsIssuer::SetCurrentEpoch(unsigned int) --> void",
               pybind11::arg("epoch"));
        cl.def("SetCertificateValidityPeriod",
               (void (chip::Controller::ExampleOperationalCredentialsIssuer::*)(unsigned int)) &
                   chip::Controller::ExampleOperationalCredentialsIssuer::SetCertificateValidityPeriod,
               "C++: chip::Controller::ExampleOperationalCredentialsIssuer::SetCertificateValidityPeriod(unsigned int) --> void",
               pybind11::arg("validity"));
        cl.def("assign",
               (class chip::Controller::ExampleOperationalCredentialsIssuer &
                (chip::Controller::ExampleOperationalCredentialsIssuer::*) (const class chip::Controller::
                                                                                ExampleOperationalCredentialsIssuer &) ) &
                   chip::Controller::ExampleOperationalCredentialsIssuer::operator=,
               "C++: chip::Controller::ExampleOperationalCredentialsIssuer::operator=(const class "
               "chip::Controller::ExampleOperationalCredentialsIssuer &) --> class "
               "chip::Controller::ExampleOperationalCredentialsIssuer &",
               pybind11::return_value_policy::automatic, pybind11::arg(""));
    }
    { // chip::PersistentStorageDelegate file:core/CHIPPersistentStorageDelegate.h line:26
        pybind11::class_<chip::PersistentStorageDelegate, std::shared_ptr<chip::PersistentStorageDelegate>,
                         PyCallBack_chip_PersistentStorageDelegate>
            cl(M("chip"), "PersistentStorageDelegate", "");
        cl.def(pybind11::init([]() { return new PyCallBack_chip_PersistentStorageDelegate(); }));
        cl.def("SyncGetKeyValue",
               (int (chip::PersistentStorageDelegate::*)(const char *, void *, unsigned short &)) &
                   chip::PersistentStorageDelegate::SyncGetKeyValue,
               "This is a synchronous Get API, where the value is returned via the output\n   buffer.\n\n   This API can be used "
               "to retrieve a byte buffer value from the storage.\n   There is no implied data format and and data will be "
               "stored/fetched binary.\n   Caller is responsible to take care of any special formatting needs (e.g. byte\n   "
               "order, null terminators, consistency checks or versioning).\n\n \n Key to lookup\n \n\n Value for the key\n \n\n "
               "Input value buffer size, output length of value.\n                 The output length could be larger than input "
               "value. In\n                 such cases, the user should allocate the buffer large\n                 enough (>= "
               "output length), and call the API again.\n\nC++: chip::PersistentStorageDelegate::SyncGetKeyValue(const char *, "
               "void *, unsigned short &) --> int",
               pybind11::arg("key"), pybind11::arg("buffer"), pybind11::arg("size"));
        cl.def("SyncSetKeyValue",
               (int (chip::PersistentStorageDelegate::*)(const char *, const void *, unsigned short)) &
                   chip::PersistentStorageDelegate::SyncSetKeyValue,
               "Set the value for the key to a byte buffer.\n\n \n Key to be set\n \n\n Value to be set\n \n\n Size of the "
               "Value\n\nC++: chip::PersistentStorageDelegate::SyncSetKeyValue(const char *, const void *, unsigned short) --> int",
               pybind11::arg("key"), pybind11::arg("value"), pybind11::arg("size"));
        cl.def("SyncDeleteKeyValue",
               (int (chip::PersistentStorageDelegate::*)(const char *)) & chip::PersistentStorageDelegate::SyncDeleteKeyValue,
               "Deletes the value for the key\n\n \n Key to be deleted\n\nC++: "
               "chip::PersistentStorageDelegate::SyncDeleteKeyValue(const char *) --> int",
               pybind11::arg("key"));
        cl.def("assign",
               (class chip::PersistentStorageDelegate &
                (chip::PersistentStorageDelegate::*) (const class chip::PersistentStorageDelegate &) ) &
                   chip::PersistentStorageDelegate::operator=,
               "C++: chip::PersistentStorageDelegate::operator=(const class chip::PersistentStorageDelegate &) --> class "
               "chip::PersistentStorageDelegate &",
               pybind11::return_value_policy::automatic, pybind11::arg(""));
    }
}
