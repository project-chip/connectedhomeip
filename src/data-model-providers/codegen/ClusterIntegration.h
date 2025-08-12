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
        /// When this is called, the caller has ensured that the endpointID was valid, that optionalAttributeBits are loaded
        /// from ember according to supported attributes and that the feature map for the underlying cluster ID has been loaded.
        virtual ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned zeroBasedArrayIndex,
                                                               uint32_t optionalAttributeBits, uint32_t featureMap) = 0;

        // Find the previously created cluster on the given path
        virtual ServerClusterInterface & FindRegistration(unsigned zeroBasedArrayIndex) = 0;

        // Destroy/free the given registration
        virtual void DestroyRegistration(unsigned zeroBasedArrayIndex) = 0;
    };

    struct RegisterServerOptions
    {
        EndpointId endpointId;
        ClusterId clusterId;
        uint16_t fixedClusterServerEndpointCount; // data to aid in ember 0-based indexing
        uint16_t maxEndpointCount;                // this is how many endpoints are supported by the delegates (0-based index)
        bool fetchFeatureMap;                     // read feature map attribute from ember
        uint32_t fetchOptionalAttributes;         // read the enabling of the first 32 optional attributes from ember
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
    /// Typical implementation is that this gets called in `emberAf....ClusterServerInitCallback`
    static void RegisterServer(const RegisterServerOptions & options, Delegate & delegate);

    struct UnregisterServerOptions
    {
        EndpointId endpointId;
        ClusterId clusterId;
        uint16_t fixedClusterServerEndpointCount; // data to aid in ember 0-based indexing
        uint16_t maxEndpointCount;                // this is how many endpoints are supported by the delegates (0-based index)
    };

    /// Typical implementation is that this gets called in `Matter....ClusterServerShutdownCallback`
    static void UnregisterServer(const UnregisterServerOptions & options, Delegate & delegate);
};

} // namespace chip::app
