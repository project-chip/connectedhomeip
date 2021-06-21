#include <app/util/basic-types.h>
#include <core/PeerId.h>
#include <inet/IPAddress.h>
#include <inet/InetLayer.h>
#include <mdns/Resolver.h>
#include <sstream> // __str__
#include <sys/socket.h>
#include <system/SystemLayer.h>
#include <system/SystemStats.h>

#include <functional>
#include <pybind11/pybind11.h>
#include <string>

#ifndef BINDER_PYBIND11_TYPE_CASTER
#define BINDER_PYBIND11_TYPE_CASTER
PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>)
PYBIND11_DECLARE_HOLDER_TYPE(T, T *)
PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

// chip::Mdns::ResolverDelegate file:mdns/Resolver.h line:107
struct PyCallBack_chip_Mdns_ResolverDelegate : public chip::Mdns::ResolverDelegate
{
    using chip::Mdns::ResolverDelegate::ResolverDelegate;

    void OnNodeIdResolved(const struct chip::Mdns::ResolvedNodeData & a0) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload =
            pybind11::get_overload(static_cast<const chip::Mdns::ResolverDelegate *>(this), "OnNodeIdResolved");
        if (overload)
        {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
            if (pybind11::detail::cast_is_temporary_value_reference<void>::value)
            {
                static pybind11::detail::override_caster_t<void> caster;
                return pybind11::detail::cast_ref<void>(std::move(o), caster);
            }
            else
                return pybind11::detail::cast_safe<void>(std::move(o));
        }
        pybind11::pybind11_fail("Tried to call pure virtual function \"ResolverDelegate::OnNodeIdResolved\"");
    }
    void OnNodeIdResolutionFailed(const class chip::PeerId & a0, int a1) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload =
            pybind11::get_overload(static_cast<const chip::Mdns::ResolverDelegate *>(this), "OnNodeIdResolutionFailed");
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
        pybind11::pybind11_fail("Tried to call pure virtual function \"ResolverDelegate::OnNodeIdResolutionFailed\"");
    }
    void OnNodeDiscoveryComplete(const struct chip::Mdns::DiscoveredNodeData & a0) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload =
            pybind11::get_overload(static_cast<const chip::Mdns::ResolverDelegate *>(this), "OnNodeDiscoveryComplete");
        if (overload)
        {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
            if (pybind11::detail::cast_is_temporary_value_reference<void>::value)
            {
                static pybind11::detail::override_caster_t<void> caster;
                return pybind11::detail::cast_ref<void>(std::move(o), caster);
            }
            else
                return pybind11::detail::cast_safe<void>(std::move(o));
        }
        pybind11::pybind11_fail("Tried to call pure virtual function \"ResolverDelegate::OnNodeDiscoveryComplete\"");
    }
};

// chip::Mdns::Resolver file:mdns/Resolver.h line:123
struct PyCallBack_chip_Mdns_Resolver : public chip::Mdns::Resolver
{
    using chip::Mdns::Resolver::Resolver;

    int StartResolver(class chip::Inet::InetLayer * a0, unsigned short a1) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const chip::Mdns::Resolver *>(this), "StartResolver");
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
        pybind11::pybind11_fail("Tried to call pure virtual function \"Resolver::StartResolver\"");
    }
    int SetResolverDelegate(class chip::Mdns::ResolverDelegate * a0) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload =
            pybind11::get_overload(static_cast<const chip::Mdns::Resolver *>(this), "SetResolverDelegate");
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
        pybind11::pybind11_fail("Tried to call pure virtual function \"Resolver::SetResolverDelegate\"");
    }
    int ResolveNodeId(const class chip::PeerId & a0, chip::Inet::IPAddressType a1) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const chip::Mdns::Resolver *>(this), "ResolveNodeId");
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
        pybind11::pybind11_fail("Tried to call pure virtual function \"Resolver::ResolveNodeId\"");
    }
    int FindCommissionableNodes(struct chip::Mdns::DiscoveryFilter a0) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload =
            pybind11::get_overload(static_cast<const chip::Mdns::Resolver *>(this), "FindCommissionableNodes");
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
        pybind11::pybind11_fail("Tried to call pure virtual function \"Resolver::FindCommissionableNodes\"");
    }
    int FindCommissioners(struct chip::Mdns::DiscoveryFilter a0) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const chip::Mdns::Resolver *>(this), "FindCommissioners");
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
        pybind11::pybind11_fail("Tried to call pure virtual function \"Resolver::FindCommissioners\"");
    }
};

void bind_PyChip_MDNS(std::function<pybind11::module &(std::string const & namespace_)> & M)
{
    { // chip::Mdns::ResolvedNodeData file:mdns/Resolver.h line:31
        pybind11::class_<chip::Mdns::ResolvedNodeData, std::shared_ptr<chip::Mdns::ResolvedNodeData>> cl(M("chip::Mdns"),
                                                                                                         "ResolvedNodeData", "");
        cl.def(pybind11::init([]() { return new chip::Mdns::ResolvedNodeData(); }));
        cl.def(pybind11::init([](chip::Mdns::ResolvedNodeData const & o) { return new chip::Mdns::ResolvedNodeData(o); }));
        cl.def_readwrite("mPeerId", &chip::Mdns::ResolvedNodeData::mPeerId);
        cl.def_readwrite("mInterfaceId", &chip::Mdns::ResolvedNodeData::mInterfaceId);
        cl.def_readwrite("mAddress", &chip::Mdns::ResolvedNodeData::mAddress);
        cl.def_readwrite("mPort", &chip::Mdns::ResolvedNodeData::mPort);
    }
    { // chip::Mdns::DiscoveredNodeData file:mdns/Resolver.h line:42
        pybind11::class_<chip::Mdns::DiscoveredNodeData, std::shared_ptr<chip::Mdns::DiscoveredNodeData>> cl(
            M("chip::Mdns"), "DiscoveredNodeData", "");
        cl.def(pybind11::init([]() { return new chip::Mdns::DiscoveredNodeData(); }));
        cl.def(pybind11::init([](chip::Mdns::DiscoveredNodeData const & o) { return new chip::Mdns::DiscoveredNodeData(o); }));
        cl.def_readwrite("longDiscriminator", &chip::Mdns::DiscoveredNodeData::longDiscriminator);
        cl.def_readwrite("vendorId", &chip::Mdns::DiscoveredNodeData::vendorId);
        cl.def_readwrite("productId", &chip::Mdns::DiscoveredNodeData::productId);
        cl.def_readwrite("additionalPairing", &chip::Mdns::DiscoveredNodeData::additionalPairing);
        cl.def_readwrite("commissioningMode", &chip::Mdns::DiscoveredNodeData::commissioningMode);
        cl.def_readwrite("deviceType", &chip::Mdns::DiscoveredNodeData::deviceType);
        cl.def_readwrite("rotatingIdLen", &chip::Mdns::DiscoveredNodeData::rotatingIdLen);
        cl.def_readwrite("pairingHint", &chip::Mdns::DiscoveredNodeData::pairingHint);
        cl.def_readwrite("numIPs", &chip::Mdns::DiscoveredNodeData::numIPs);
        cl.def("Reset", (void (chip::Mdns::DiscoveredNodeData::*)()) & chip::Mdns::DiscoveredNodeData::Reset,
               "C++: chip::Mdns::DiscoveredNodeData::Reset() --> void");
        cl.def("IsHost", (bool (chip::Mdns::DiscoveredNodeData::*)(const char *) const) & chip::Mdns::DiscoveredNodeData::IsHost,
               "C++: chip::Mdns::DiscoveredNodeData::IsHost(const char *) const --> bool", pybind11::arg("host"));
        cl.def("IsValid", (bool (chip::Mdns::DiscoveredNodeData::*)() const) & chip::Mdns::DiscoveredNodeData::IsValid,
               "C++: chip::Mdns::DiscoveredNodeData::IsValid() const --> bool");
    }
    // chip::Mdns::DiscoveryFilterType file:mdns/Resolver.h line:88
    pybind11::enum_<chip::Mdns::DiscoveryFilterType>(M("chip::Mdns"), "DiscoveryFilterType", "")
        .value("kNone", chip::Mdns::DiscoveryFilterType::kNone)
        .value("kShort", chip::Mdns::DiscoveryFilterType::kShort)
        .value("kLong", chip::Mdns::DiscoveryFilterType::kLong)
        .value("kVendor", chip::Mdns::DiscoveryFilterType::kVendor)
        .value("kDeviceType", chip::Mdns::DiscoveryFilterType::kDeviceType)
        .value("kCommissioningMode", chip::Mdns::DiscoveryFilterType::kCommissioningMode)
        .value("kCommissioningModeFromCommand", chip::Mdns::DiscoveryFilterType::kCommissioningModeFromCommand)
        .value("kCommissioner", chip::Mdns::DiscoveryFilterType::kCommissioner);

    ;

    { // chip::Mdns::DiscoveryFilter file:mdns/Resolver.h line:99
        pybind11::class_<chip::Mdns::DiscoveryFilter, std::shared_ptr<chip::Mdns::DiscoveryFilter>> cl(M("chip::Mdns"),
                                                                                                       "DiscoveryFilter", "");
        cl.def(pybind11::init([]() { return new chip::Mdns::DiscoveryFilter(); }));
        cl.def(pybind11::init<enum chip::Mdns::DiscoveryFilterType, unsigned short>(), pybind11::arg("newType"),
               pybind11::arg("newCode"));

        cl.def(pybind11::init([](chip::Mdns::DiscoveryFilter const & o) { return new chip::Mdns::DiscoveryFilter(o); }));
        cl.def_readwrite("type", &chip::Mdns::DiscoveryFilter::type);
        cl.def_readwrite("code", &chip::Mdns::DiscoveryFilter::code);
    }
    { // chip::Mdns::ResolverDelegate file:mdns/Resolver.h line:107
        pybind11::class_<chip::Mdns::ResolverDelegate, std::shared_ptr<chip::Mdns::ResolverDelegate>,
                         PyCallBack_chip_Mdns_ResolverDelegate>
            cl(M("chip::Mdns"), "ResolverDelegate", "Groups callbacks for CHIP service resolution requests");
        cl.def(pybind11::init([]() { return new PyCallBack_chip_Mdns_ResolverDelegate(); }));
        cl.def("OnNodeIdResolved",
               (void (chip::Mdns::ResolverDelegate::*)(const struct chip::Mdns::ResolvedNodeData &)) &
                   chip::Mdns::ResolverDelegate::OnNodeIdResolved,
               "Called when a requested CHIP node ID has been successfully resolved\n\nC++: "
               "chip::Mdns::ResolverDelegate::OnNodeIdResolved(const struct chip::Mdns::ResolvedNodeData &) --> void",
               pybind11::arg("nodeData"));
        cl.def("OnNodeIdResolutionFailed",
               (void (chip::Mdns::ResolverDelegate::*)(const class chip::PeerId &, int)) &
                   chip::Mdns::ResolverDelegate::OnNodeIdResolutionFailed,
               "Called when a CHIP node ID resolution has failed\n\nC++: "
               "chip::Mdns::ResolverDelegate::OnNodeIdResolutionFailed(const class chip::PeerId &, int) --> void",
               pybind11::arg("peerId"), pybind11::arg("error"));
        cl.def("OnNodeDiscoveryComplete",
               (void (chip::Mdns::ResolverDelegate::*)(const struct chip::Mdns::DiscoveredNodeData &)) &
                   chip::Mdns::ResolverDelegate::OnNodeDiscoveryComplete,
               "C++: chip::Mdns::ResolverDelegate::OnNodeDiscoveryComplete(const struct chip::Mdns::DiscoveredNodeData &) --> void",
               pybind11::arg("nodeData"));
        cl.def(
            "assign",
            (class chip::Mdns::ResolverDelegate & (chip::Mdns::ResolverDelegate::*) (const class chip::Mdns::ResolverDelegate &) ) &
                chip::Mdns::ResolverDelegate::operator=,
            "C++: chip::Mdns::ResolverDelegate::operator=(const class chip::Mdns::ResolverDelegate &) --> class "
            "chip::Mdns::ResolverDelegate &",
            pybind11::return_value_policy::automatic, pybind11::arg(""));
    }
    { // chip::Mdns::Resolver file:mdns/Resolver.h line:123
        pybind11::class_<chip::Mdns::Resolver, std::shared_ptr<chip::Mdns::Resolver>, PyCallBack_chip_Mdns_Resolver> cl(
            M("chip::Mdns"), "Resolver", "Interface for resolving CHIP services");
        cl.def(pybind11::init([]() { return new PyCallBack_chip_Mdns_Resolver(); }));
        cl.def("StartResolver",
               (int (chip::Mdns::Resolver::*)(class chip::Inet::InetLayer *, unsigned short)) & chip::Mdns::Resolver::StartResolver,
               "Ensures that the resolver is started.\n Must be called before any ResolveNodeId calls.\n\n Unsual name to allow "
               "base MDNS classes to implement both Advertiser and Resolver interfaces.\n\nC++: "
               "chip::Mdns::Resolver::StartResolver(class chip::Inet::InetLayer *, unsigned short) --> int",
               pybind11::arg("inetLayer"), pybind11::arg("port"));
        cl.def("SetResolverDelegate",
               (int (chip::Mdns::Resolver::*)(class chip::Mdns::ResolverDelegate *)) & chip::Mdns::Resolver::SetResolverDelegate,
               "Registers a resolver delegate if none has been registered before\n\nC++: "
               "chip::Mdns::Resolver::SetResolverDelegate(class chip::Mdns::ResolverDelegate *) --> int",
               pybind11::arg("delegate"));
        cl.def("ResolveNodeId",
               (int (chip::Mdns::Resolver::*)(const class chip::PeerId &, chip::Inet::IPAddressType)) &
                   chip::Mdns::Resolver::ResolveNodeId,
               "Requests resolution of a node ID to its address\n\nC++: chip::Mdns::Resolver::ResolveNodeId(const class "
               "chip::PeerId &, chip::Inet::IPAddressType) --> int",
               pybind11::arg("peerId"), pybind11::arg("type"));
        cl.def(
            "FindCommissionableNodes", [](chip::Mdns::Resolver & o) -> int { return o.FindCommissionableNodes(); }, "");
        cl.def("FindCommissionableNodes",
               (int (chip::Mdns::Resolver::*)(struct chip::Mdns::DiscoveryFilter)) & chip::Mdns::Resolver::FindCommissionableNodes,
               "C++: chip::Mdns::Resolver::FindCommissionableNodes(struct chip::Mdns::DiscoveryFilter) --> int",
               pybind11::arg("filter"));
        cl.def(
            "FindCommissioners", [](chip::Mdns::Resolver & o) -> int { return o.FindCommissioners(); }, "");
        cl.def("FindCommissioners",
               (int (chip::Mdns::Resolver::*)(struct chip::Mdns::DiscoveryFilter)) & chip::Mdns::Resolver::FindCommissioners,
               "C++: chip::Mdns::Resolver::FindCommissioners(struct chip::Mdns::DiscoveryFilter) --> int", pybind11::arg("filter"));
        cl.def_static("Instance", (class chip::Mdns::Resolver & (*) ()) & chip::Mdns::Resolver::Instance,
                      "Provides the system-wide implementation of the service resolver\n\nC++: chip::Mdns::Resolver::Instance() "
                      "--> class chip::Mdns::Resolver &",
                      pybind11::return_value_policy::automatic);
        cl.def("assign",
               (class chip::Mdns::Resolver & (chip::Mdns::Resolver::*) (const class chip::Mdns::Resolver &) ) &
                   chip::Mdns::Resolver::operator=,
               "C++: chip::Mdns::Resolver::operator=(const class chip::Mdns::Resolver &) --> class chip::Mdns::Resolver &",
               pybind11::return_value_policy::automatic, pybind11::arg(""));
    }
}
