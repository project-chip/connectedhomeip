/*
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
#pragma once

#include <app/server-cluster/ServerClusterInterfaceRegistry.h>

#include <cstdint>

namespace chip::app {

/// Handles reusable code for integrating server cluster interfaces with ember:
///   - loads ember metadata (optional attributes and feature maps)
///   - calls corresponding register/unregister calls on the Codegen Data Model Provider.
///
/// Uses an underlying delegate to handle constructor/destructor calls for actual cluster implementations.
class CodegenClusterIntegration
{
public:
    /// Handles the creation/destruction of server clusters when using codegen integration.
    ///
    /// The purpose of this class is to call ServerClusterInterfaces constructors and
    /// destructors once generic ember processing has been completed.
    ///
    /// Logic is generally split into:
    ///   - generic fetching and validating data from the ember framework like
    ///     0-based array index from endpoint id, optional attribute fetching, feature map fetching
    ///   - registering/unregistering ServerClusterInterfaces
    ///   - determining how to create/destroy classes (the purpose of this delegate)
    class Delegate
    {
    public:
        virtual ~Delegate() = default;

        /// Create the given registration for an endpoint
        ///
        /// When this is called, the caller has ensured that the endpointId was valid, that optionalAttributeBits are loaded
        /// from ember according to supported attributes and that the feature map for the underlying cluster ID has been loaded.
        ///
        /// NOTE: optionalAttributeBits is intended for low id attributes since it supports attribute bits up to 31 only. It is
        ///       intended for use with `OptionalAttributeSet` and NOT all clusters support this. Specific examples:
        ///        - LevelControl::StartupCurrentLevel has ID 0x4000
        ///        - OnOff attributes GlobalSceneControl, OnTime, OffWaitTime, StartupOnOff have ID >= 0x4000
        ///        - ColorControl, Thermostat and DoorLock have many attributes with high ID as well
        ///
        /// Use optionalAttributes only if its usage makes sense for the cluster. In many instances it does,
        /// however it is not a generic rule. Usage of it must be double-checked as sufficient.
        ///
        /// Method is assumed to never fail: this is expected to call a constructor and not fail.
        virtual ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned emberEndpointIndex,
                                                               uint32_t optionalAttributeBits, uint32_t featureMap) = 0;

        /// Find the previously created cluster on the given path
        ///
        /// This is assumed to never fail as `CreateRegistration` is assumed to never fail.
        /// This will be called only after `CreateRegistration`.
        virtual ServerClusterInterface & FindRegistration(unsigned emberEndpointIndex) = 0;

        /// Free up resources for this index, generally expected to call a destructor/free resources
        /// as applicable.
        ///
        /// It is assumed that this is called as the couterpart of `CreateRegistration`.
        virtual void ReleaseRegistration(unsigned emberEndpointIndex) = 0;
    };

    // Note on indexing:
    //   - The methods here use emberAfGetClusterServerEndpointIndex to convert
    //     an endpointId to a linear index [0; maxEndpointCount) from ember.
    //     For this ember requires the fixedClusterServerEndpointCount.
    struct RegisterServerOptions
    {
        EndpointId endpointId;
        ClusterId clusterId;
        uint16_t fixedClusterServerEndpointCount; // Number of fixed endpoints in ember configuration.
        uint16_t maxEndpointCount;                // This is how many endpoints are supported by the delegate (0-based index).
        bool fetchFeatureMap;                     // Read feature map attribute from ember.
        bool fetchOptionalAttributes;             // Read the enabling of the first 32 optional attributes from ember.
    };

    /// Loads required data from ember and calls `CreateRegistration` once all the data
    /// has been validated. Validation includes:
    ///  - cluster exists on the given endpoint and a valid index could be found for it
    ///  - feature map could be loaded (if applicable, otherwise it will be set to 0)
    ///  - optional attributes were loaded
    ///
    /// The returned `CreateRegistration` value will be used to register to the codegen
    /// data model provider registry
    ///
    /// In case of errors, this method will log the error and return (error state is not
    /// returned to the caller as it is generally not actionable/fixable)
    ///
    /// Typical implementation is that this gets called in `emberAf....ClusterServerInitCallback`
    static void RegisterServer(const RegisterServerOptions & options, Delegate & delegate);

    struct UnregisterServerOptions
    {
        EndpointId endpointId;
        ClusterId clusterId;
        uint16_t fixedClusterServerEndpointCount; // Number of fixed endpoints in ember configuration.
        uint16_t maxEndpointCount;                // This is how many endpoints are supported by the delegate (0-based index).
    };

    /// A typical implementation is that this gets called in `Matter....ClusterServerShutdownCallback`.
    ///
    /// In case of errors, this method will log the error and return (error state is not
    /// returned to the caller as it is generally not actionable/fixable)
    static void UnregisterServer(const UnregisterServerOptions & options, Delegate & delegate);
};

} // namespace chip::app
