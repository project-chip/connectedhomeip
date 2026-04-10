# Relative Humidity Measurement Cluster

This cluster uses the code-driven approach. The Ember attribute accessors for
`MeasuredValue`, `MinMeasuredValue`, `MaxMeasuredValue`, and `Tolerance` are no
longer available.

## Setting a new humidity reading

### BEFORE (using Ember accessors)

```cpp
RelativeHumidityMeasurement::Attributes::MeasuredValue::Set(endpointId, newValue);
```

### NOW (code-driven)

```cpp
CHIP_ERROR err = RelativeHumidityMeasurement::SetMeasuredValue(endpointId,
                     DataModel::MakeNullable(uint16_t(newValue)));
if (err != CHIP_NO_ERROR)
{
    // handle error
}
```

## Configuring min/max range and tolerance

`MinMeasuredValue`, `MaxMeasuredValue`, and `Tolerance` are fixed hardware
characteristics. They are configured once at startup via the `Config` struct and
cannot be changed at runtime.

The default values are read from the ZAP-configured Ember attribute store at
init time. If your app needs specific values, configure them in the ZAP file
(`.zap`) for the endpoint.

If you need to set them via code (e.g. in tests or for dynamic
endpoints), pass a `Config` at construction time:

```cpp
// Min/max only
RelativeHumidityMeasurementCluster::Config config;
config.minMeasuredValue = DataModel::MakeNullable(uint16_t(0));
config.maxMeasuredValue = DataModel::MakeNullable(uint16_t(10000));
auto cluster = RelativeHumidityMeasurementCluster(endpointId, config);

// With optional Tolerance attribute
RelativeHumidityMeasurementCluster::Config config;
config.minMeasuredValue = DataModel::MakeNullable(uint16_t(0));
config.maxMeasuredValue = DataModel::MakeNullable(uint16_t(10000));
config.WithTolerance(100);
auto cluster = RelativeHumidityMeasurementCluster(endpointId, config);
```

### BEFORE (using Ember accessors)

```cpp
RelativeHumidityMeasurement::Attributes::MinMeasuredValue::Set(endpointId, 0);
RelativeHumidityMeasurement::Attributes::MaxMeasuredValue::Set(endpointId, 10000);
```
