This cluster is currently following a code driven approach.

This means that the Accessors for the attribute MeasuredValue are no longer
available.

Now to set the value for this attribute the following code change applies:

# BEFORE (using the Accessors)

    app::Clusters::IlluminanceMeasurement::Attributes::MeasuredValue::Set(1, static_cast<uint16_t>(1000));

# CURRENT (using the code driven approach)

    CHIP_ERROR err = app::Clusters::IlluminanceMeasurement::SetMeasuredValue(1, static_cast<uint16_t>(1000));
    if (err == CHIP_NO_ERROR)
    {
        // SetMeasuredValue() succeeded
    }
    else
    {
        // SetMeasuredValue() failed
    }
