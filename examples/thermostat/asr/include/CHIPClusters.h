/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

// Prevent multiple inclusion
#pragma once

#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/ids/Commands.h>

#include <controller/CHIPCluster.h>
#include <lib/core/CHIPCallback.h>
#include <lib/support/Span.h>

namespace chip {
namespace Controller {

class DLL_EXPORT IdentifyCluster : public ClusterBase
{
public:
    IdentifyCluster(Messaging::ExchangeManager & exchangeManager, const SessionHandle & session, EndpointId endpoint) :
        ClusterBase(exchangeManager, session, endpoint)
    {}
    ~IdentifyCluster() {}
};

class DLL_EXPORT DescriptorCluster : public ClusterBase
{
public:
    DescriptorCluster(Messaging::ExchangeManager & exchangeManager, const SessionHandle & session, EndpointId endpoint) :
        ClusterBase(exchangeManager, session, endpoint)
    {}
    ~DescriptorCluster() {}
};

class DLL_EXPORT OtaSoftwareUpdateProviderCluster : public ClusterBase
{
public:
    OtaSoftwareUpdateProviderCluster(Messaging::ExchangeManager & exchangeManager, const SessionHandle & session,
                                     EndpointId endpoint) :
        ClusterBase(exchangeManager, session, endpoint)
    {}
    ~OtaSoftwareUpdateProviderCluster() {}
};

class DLL_EXPORT TemperatureMeasurementCluster : public ClusterBase
{
public:
    TemperatureMeasurementCluster(Messaging::ExchangeManager & exchangeManager, const SessionHandle & session,
                                  EndpointId endpoint) :
        ClusterBase(exchangeManager, session, endpoint)
    {}
    ~TemperatureMeasurementCluster() {}
};

class DLL_EXPORT RelativeHumidityMeasurementCluster : public ClusterBase
{
public:
    RelativeHumidityMeasurementCluster(Messaging::ExchangeManager & exchangeManager, const SessionHandle & session,
                                       EndpointId endpoint) :
        ClusterBase(exchangeManager, session, endpoint)
    {}
    ~RelativeHumidityMeasurementCluster() {}
};

} // namespace Controller
} // namespace chip
