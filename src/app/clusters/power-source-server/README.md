# Power Source Cluster

The Power Source cluster is used to monitor and report the state of a power
source. A power source can be either a battery or a wired power source.

## Overview

This directory contains an implementation of Matter Power Source cluster. This
implementation does not handle the persistence of the `Order` attribute. Because
the `Order` attribute is read-only, only the direct user of the cluster can
change it, so it is expected of them to handle the persistence of the attribute.
This implementation also doesn't handle the storage of fixed string attributes
and the attribute `endpointList`. This implementation notifies the data model
when an attribute changes in the way that the spec mandates.

## Usage

The cluster needs to be created manually and registered directly using
`CodeDrivenDataModelProvider`. Creation can be done using
`RegisteredServerCluster` and `LazyRegisteredServerCluster` in
`src/app/server-cluster/SingleEndpointServerClusterRegistry.h`.

### How does the cluster work

Because of flash considerations the implementation is a template based on the
feature set and attribute set for things that the user wants to be supported.
This will be made more clear using examples

### Examples

-   ## Configuration for a wired power source besides mandatory attributes.

    This cluster supports `WiredAssessedInputVoltage` and `WiredNominalVoltage`
    optional attributes

    ```cpp
    #include <app/clusters/power-source-server/PowerSourceCluster.h>
    #include <app/server-cluster/SingleEndpointServerClusterRegistry.h>
    #include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>

    using namespace app;
    using namespace app::Clusters;

    constexpr EndpointId powerSourceEndpointId = 1;
    ```

    Define the template specialization to be used

    ```cpp
    // this is the "more correct" way to do this :)
    constexpr uint32_t GetOptionalAttributeBits()
    {
        PowerSource::PowerSourceOptionalAttributeSet optAttributes{};
        optAttributes.Set<PowerSource::Attributes::WiredAssessedInputVoltage::Id>();
        optAttributes.Set<PowerSource::Attributes::WiredNominalVoltage::Id>();
        return optAttributes.Raw();
    }

    constexpr uint32_t optionalAttributeBits = GetOptionalAttributeBits();

    // -- OR --

    constexpr uint32_t optionalAttributeBits = OptionalAttributeSet<
        PowerSource::Attributes::WiredAssessedInputVoltage::Id,
        PowerSource::Attributes::WiredNominalVoltage::Id>::All();

    constexpr uint32_t featureBits = BitFlags<PowerSource::Feature>(PowerSource::Feature::kWired).Raw();

    using MyWiredPowerSourceCluster = PowerSourceCluster<featureBits, optionalAttributeBits>;

    ```

    Create config object for the cluster, the cluster itself, and then use it

    ```cpp

    // the config
    MyWiredPowerSourceCluster::Config config("Wired Power Source (description)"_span, PowerSource::WiredCurrentTypeEnum::kDc);
    config.wiredNominalVoltage = 23000; // 23V

    // the cluster
    RegisteredServerCluster<MyWiredPowerSourceCluster> powerSourceInstance(powerSourceEndpointId, config);

    // register the cluster, this will call `Startup` on the cluster
    CodeDrivenDataModelProvider::Instance().Registry().Register(powerSourceInstance.Registration());

    // use the cluster
    powerSourceInstance.Cluster().SetWiredAssessedInputVoltage(20000); // 20V

    ```

    Note that functions associated with attributes that are not supported will
    throw a compile error.

-   ## Configuration for three batteries, one simple, one rechargeable, one replaceable

    Simple battery will use the `BatPercentRemaining` optional attribute.
    Rechargeable battery will use the `BatPercentRemaining`,
    `BatChargingCurrent` optional attributes. Replaceable battery will use the
    `BatTimeRemaining`, `BatCapacity` optional attributes.

    It will be very beneficial to use one template specialization for the three
    of these configurations.

    ```cpp
     #include <app/clusters/power-source-server/PowerSourceCluster.h>
     #include <app/server-cluster/SingleEndpointServerClusterRegistry.h>
     #include <platform/DefaultTimerDelegate.h>
     #include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>

     using namespace app;
     using namespace app::Clusters;

     constexpr EndpointId powerSourceEndpointId1 = 1;
     constexpr EndpointId powerSourceEndpointId2 = 2;
     constexpr EndpointId powerSourceEndpointId3 = 3;

     DefaultTimerDelegate timerDelegate;
    ```

    Define the template specialization to be used

    ```cpp
    constexpr uint32_t GetOptionalAttributeBits()
    {
        PowerSource::PowerSourceOptionalAttributeSet optAttributes{};
        // enable all optional attributes that are used in any of the configurations
        optAttributes.Set<PowerSource::Attributes::BatPercentRemaining::Id>();
        optAttributes.Set<PowerSource::Attributes::BatTimeRemaining::Id>();
        optAttributes.Set<PowerSource::Attributes::BatChargingCurrent::Id>();
        optAttributes.Set<PowerSource::Attributes::BatCapacity::Id>();
        return optAttributes.Raw();
    }

    constexpr uint32_t optionalAttributeBits = GetOptionalAttributeBits();

    // enable all features that are used in any of the configurations
    constexpr uint32_t featureBits = BitFlags<PowerSource::Feature>(
        PowerSource::Feature::kBattery,
        PowerSource::Feature::kRechargeable,
        PowerSource::Feature::kReplaceable
    ).Raw();

    using MyBatteryPowerSourceCluster = PowerSourceCluster<featureBits, optionalAttributeBits>;

    ```

    Create config objects for the clusters, the clusters, and then use them All
    three objects will have the same functionality in terms of accessible
    functions and config field members

    But it is possible to inform the cluster exactly what attributes are used or
    supported from the manufacturer side using the field
    `usedOptionalAttributes` in the config object. This will make the cluster to
    report specific information about what the cluster supports to the data
    model. This will mean, that technically the cluster will have the capability
    to support certain attributes, but if manufacturer can't support it, the
    data model will see them as unsupported.

    ```cpp

    // The attribute sets of configurations
    PowerSource::PowerSourceOptionalAttributeSet attrSet1{};
    attrSet1.Set<PowerSource::Attributes::BatPercentRemaining::Id>();

    PowerSource::PowerSourceOptionalAttributeSet attrSet2{};
    attrSet2.Set<PowerSource::Attributes::BatPercentRemaining::Id>();
    attrSet2.Set<PowerSource::Attributes::BatChargingCurrent::Id>();

    PowerSource::PowerSourceOptionalAttributeSet attrSet3{};
    attrSet3.Set<PowerSource::Attributes::BatTimeRemaining::Id>();
    attrSet3.Set<PowerSource::Attributes::BatCapacity::Id>();

    // the configs
    MyBatteryPowerSourceCluster::Config config1("Simple battery cluster"_span, PowerSource::BatReplaceabilityEnum::kUnspecified, timerDelegate);
    config1.batPercentRemaining = 200; // 100%, doubled percentage
    config1.usedOptionalAttributes = attrSet1;

    MyBatteryPowerSourceCluster::Config config2("Rechargeable battery cluster"_span, PowerSource::BatReplaceabilityEnum::kNotReplaceable, timerDelegate);
    // this will make the cluster report itself as rechargeable, otherwise it would be like a simple cluster to the data model
    config2.MakeRechargeable();
    config2.batPercentRemaining = 200;
    config2.batChargingCurrent = 5000; // 5A
    config2.usedOptionalAttributes = attrSet2;

    MyBatteryPowerSourceCluster::Config config3("Replaceable battery cluster"_span, PowerSource::BatReplaceabilityEnum::kUserReplaceable, timerDelegate);
    // make it replaceable
    config3.MakeReplaceable("Description for replacement"_span, /* quantity */ 1);
    config3.batTimeRemaining = 3600; // 1h estimated uptime
    config3.batCapacity = 2000; // 2000 mAh capacity
    config3.usedOptionalAttributes = attrSet3;

    // the clusters
    RegisteredServerCluster<MyBatteryPowerSourceCluster> simpleBatteryInstance(powerSourceEndpointId1, config1);
    RegisteredServerCluster<MyBatteryPowerSourceCluster> rechargeableBatteryInstance(powerSourceEndpointId2, config2);
    RegisteredServerCluster<MyBatteryPowerSourceCluster> replaceableBatteryInstance(powerSourceEndpointId3, config3);

    // register the clusters, this will call `Startup` on them
    CodeDrivenDataModelProvider::Instance().Registry().Register(simpleBatteryInstance.Registration());
    CodeDrivenDataModelProvider::Instance().Registry().Register(rechargeableBatteryInstance.Registration());
    CodeDrivenDataModelProvider::Instance().Registry().Register(replaceableBatteryInstance.Registration());

    // use the clusters ...
    replaceableBatteryInstance.Cluster().SetBatReplacementNeeded(true);

    ```

-   ## Configuration for one wired power source and one simple battery

    When it is needed to have both wired and battery power sources, it is not
    possible to do this with one template specialization by design.

    ```cpp
     #include <app/clusters/power-source-server/PowerSourceCluster.h>
     #include <app/server-cluster/SingleEndpointServerClusterRegistry.h>
     #include <platform/DefaultTimerDelegate.h>
     #include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>

     using namespace app;
     using namespace app::Clusters;

     constexpr EndpointId powerSourceEndpointId1 = 1;
     constexpr EndpointId powerSourceEndpointId2 = 2;

     DefaultTimerDelegate timerDelegate;
    ```

    Define the template specializations to be used

    ```cpp

    // enable all features that are used in any wired configuration (so just kWired)
    using MyWiredPowerSourceCluster = PowerSourceCluster<BitFlags<PowerSource::Feature>(PowerSource::Feature::kWired).Raw(), 0 /* no optional attributes */>;


    // enable kBattery for this type
    using MyBatteryPowerSourceCluster = PowerSourceCluster<BitFlags<PowerSource::Feature>(PowerSource::Feature::kBattery).Raw(), 0/* no optional attributes */>;

    ```

    Create config objects for the clusters, the clusters, and then use them

    ```cpp

    // the configs
    MyWiredPowerSourceCluster::Config config1("Wired cluster"_span, PowerSource::WiredCurrentTypeEnum::kAc);
    config1.status = PowerSource::PowerSourceStatusEnum::kUnavailable;

    MyBatteryPowerSourceCluster::Config config2("Simple battery cluster"_span, PowerSource::BatReplaceabilityEnum::kNotReplaceable, timerDelegate);

    // the clusters
    RegisteredServerCluster<MyWiredPowerSourceCluster> wiredInstance(powerSourceEndpointId1, config1);
    RegisteredServerCluster<MyBatteryPowerSourceCluster> batteryInstance(powerSourceEndpointId2, config2);

    // register the clusters, this will call `Startup` on them
    CodeDrivenDataModelProvider::Instance().Registry().Register(wiredInstance.Registration());
    CodeDrivenDataModelProvider::Instance().Registry().Register(batteryInstance.Registration());

    // use the clusters ...
    wiredInstance.Cluster().SetOrder(2);

    ```

### Notes

For simplicity there are 4 already named specializations of the class in the
`NamedPowerSourceClusters.h` -

-   `MinimalWiredPowerSourceCluster`
-   `MinimalBatteryPowerSourceCluster`
-   `FullWiredPowerSourceCluster` (supports ALL wired optional attributes)
-   `FullBatteryPowerSourceCluster` (supports ALL battery optional attributes,
    and is BOTH replaceable and rechargeable)

Instead of `RegisteredServerCluster` one can use `LazyRegisteredServerCluster`
which can defer the creation and overall provides dynamic lifetime management.

Some attributes can be can be set only in the config object, and not after the
clusters creation, because they are `Fixed` attributes.

Again, all this is to have the cluster to be the smallest possible type for the
functionality needed. So if you do not have any FLASH restrictions, you can use
`Full*PowerSourceCluster`s for everything.
