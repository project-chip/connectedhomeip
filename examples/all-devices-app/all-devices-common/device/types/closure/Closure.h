/*
 *    Copyright (c) 2026 Project CHIP Authors
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
<<<<<<< HEAD
=======
#include <app/clusters/identify-server/IdentifyCluster.h>
#include <device/api/Interface.h>
    >>>>>>> 8fd95842fc2 (Restyled by whitespace)
#include <app/clusters/closure-control-server/ClosureControlCluster.h>
#include <app/clusters/closure-control-server/ClosureControlClusterDelegate.h>
#include <app/clusters/identify-server/IdentifyCluster.h>
#include <device/api/Interface.h>
#include <device/types/closure-panel/ClosureSematicTags.h>
    namespace chip
{
    namespace app {

    class Closure : public DeviceInterface
    {
    public:
        struct Config
        {
            Span<const EndpointComposition::SemanticTag> tags = {};
            bool withPositioning                              = false;
            bool withMotionLatching                           = false;
            std::optional<BitFlags<Clusters::ClosureControl::LatchControlModesBitmap>> latchControlModes;
            bool withInstantaneous    = false;
            bool withSpeed            = false;
            bool withPedestrian       = false;
            bool withCalibration      = false;
            bool withManuallyOperable = false;
            bool withAccess           = false;
            DataModel::Nullable<Clusters::ClosureControl::GenericOverallCurrentState> initialOverallCurrentState;
        };
        Closure(Config config, TimerDelegate & Tdelegate, Clusters::IdentifyDelegate & Idelegate,
                Clusters::ClosureControl::ClosureControlClusterDelegate & CCdelegate);
        ~Closure() = default;
        CHIP_ERROR Register(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                            EndpointComposition composition) override;
        void Unregister(CodeDrivenDataModelProvider & provider) override;
        Clusters::IdentifyCluster & IdentifyCluster()
        {
            VerifyOrDie(mIdentifyCluster.IsConstructed());
            return mIdentifyCluster.Cluster();
        }
        Clusters::ClosureControl::ClosureControlCluster & ClosureControlCluster()
        {
            VerifyOrDie(mClosureControlCluster.IsConstructed());
            return mClosureControlCluster.Cluster();
        }

        EndpointId GetEndpointId() { return mEndpointRegistration.endpointEntry.id; }

    private:
        virtual bool RegistersAccessDevicePanel() const                      = 0;
        virtual CHIP_ERROR RegisterParts(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                                         EndpointComposition composition)    = 0;
        virtual void UnregisterParts(CodeDrivenDataModelProvider & provider) = 0;

        const Config mConfig;
        TimerDelegate & mTimerDelegate;
        Clusters::IdentifyDelegate & mIdentifyDelegate;
        Clusters::ClosureControl::ClosureControlClusterDelegate & mClosureControlClusterDelegate;
        LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
        LazyRegisteredServerCluster<Clusters::ClosureControl::ClosureControlCluster> mClosureControlCluster;
    };

<<<<<<< HEAD
    } // namespace app
} // namespace chip
=======
    } //
} //
>>>>>>> 8fd95842fc2 (Restyled by whitespace)
