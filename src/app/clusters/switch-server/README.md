# This cluster is currently following a code driven approach.

# This means that the Accessors for the attribute CurrentPosition are no longer available.

# Now to set the value for this attribute the following code change applies:

# BEFORE (using the Accessors)

    app::Clusters::Switch::Attributes::CurrentPosition::Set(1, position);

# CURRENT (using the code driven approach)

    auto switchCluster = app::Clusters::Switch::FindClusterOnEndpoint(1);
    VerifyOrReturn(switchCluster != nullptr);

    CHIP_ERROR err = switchCluster->SetCurrentPosition(position);
    if (err == CHIP_NO_ERROR)
    {
        // SetCurrentPosition() succeeded.
    }
    else
    {
        // SetCurrentPosition() failed.
    }

# Attributes with Quality F (Fixed) are configuration values that the cluster takes from ember and can not be changed.

# Attributes with Quality F (Fixed) maintain the Accessors but only for reading the value (getters).

# Also the code to register the events needs to change, for example:

# BEFORE

    Clusters::SwitchServer::Instance().OnSwitchLatch(1, position);

# CURRENT

    auto switchCluster = app::Clusters::Switch::FindClusterOnEndpoint(1);
    VerifyOrReturn(switchCluster != nullptr);

    auto event = switchCluster->OnSwitchLatch(position);
    if (event.has_value())
    {
        OnSwitchLatch() succeeded.
    }
    else
    {
        OnSwitchLatch() failed.
    }
