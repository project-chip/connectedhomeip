# This cluster is currently following a code driven approach.

# This means that the Accessors for the attribute MeasuredValue are no longer available.

# Now to set the value for this attribute the following code change applies:

# BEFORE (using the Accessors)

    app::Clusters::TemperatureMeasurement::Attributes::MeasuredValue::Set(1, value);

# CURRENT (using the code driven approach)

    auto temperatureMeasurement = app::Clusters::TemperatureMeasurement::FindClusterOnEndpoint(1);
    VerifyOrReturn(temperatureMeasurement != nullptr);

    CHIP_ERROR err = temperatureMeasurement->SetMeasuredValue(value);
    if (err == CHIP_NO_ERROR)
    {
        // SetMeasuredValue() succeeded
    }
    else
    {
        // SetMeasuredValue() failed
    }
