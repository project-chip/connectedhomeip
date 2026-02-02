# This cluster is currently following a code driven approach.

# This means that the Accessors for the attribute MeasuredValue are no longer available.

# Now to set the value for this attribute the following code change applies:

# BEFORE (using the Accessors)

    app::Clusters::IlluminanceMeasurement::Attributes::MeasuredValue::Set(1, static_cast<int16_t>(n));

# CURRENT (using the code driven approach)

    auto illuminanceMeasurement = app::Clusters::IlluminanceMeasurement::FindClusterOnEndpoint(1);
    if (illuminanceMeasurement != nullptr)
    {
        CHIP_ERROR err = illuminanceMeasurement->SetMeasuredValue(static_cast<int16_t>(n));
        if (err == CHIP_NO_ERROR)
        {
            // SetMeasuredValue() succeeded
        }
        else
        {
            // SetMeasuredValue() failed
        }
    }
