/**
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <lib/core/StringBuilderAdapters.h>
#include <pw_unit_test/framework.h>

#include <app/clusters/closure-control-server/closure-control-cluster-logic.h>

using chip::app::Clusters::ClosureControl::ClusterConformance;
using chip::app::Clusters::ClosureControl::Feature;

/*
    ClusterConformance Valid Function Test Case
*/

TEST(TestClosureControlClusterLogic, ValidWhenPositioningEnabled)
{
    ClusterConformance conformance;
    conformance.GetFeatureMap().Set(Feature::kPositioning);

    EXPECT_TRUE(conformance.Valid());
}

TEST(TestClosureControlClusterLogic, ValidWhenMotionLatchingEnabled)
{
    ClusterConformance conformance;
    conformance.GetFeatureMap().Set(Feature::kMotionLatching);

    EXPECT_TRUE(conformance.Valid());
}

TEST(TestClosureControlClusterLogic, InvalidWhenNeitherPositioningNorMotionLatchingEnabled)
{
    ClusterConformance conformance;

    EXPECT_FALSE(conformance.Valid());
}

TEST(TestClosureControlClusterLogic, ValidWhenSpeedAndPositioningEnabledAndInstantaneousDisabled)
{
    ClusterConformance conformance;
    conformance.GetFeatureMap().Set(Feature::kSpeed).Set(Feature::kPositioning);

    EXPECT_TRUE(conformance.Valid());
}

TEST(TestClosureControlClusterLogic, InvalidWhenSpeedEnabledButPositioningDisabled)
{
    ClusterConformance conformance;
    conformance.GetFeatureMap().Set(Feature::kSpeed);

    EXPECT_FALSE(conformance.Valid());
}

TEST(TestClosureControlClusterLogic, InvalidWhenSpeedAndInstantaneousBothEnabled)
{
    ClusterConformance conformance;
    conformance.GetFeatureMap().Set(Feature::kSpeed).Set(Feature::kPositioning).Set(Feature::kInstantaneous);

    EXPECT_FALSE(conformance.Valid());
}

TEST(TestClosureControlClusterLogic, ValidWhenVentilationAndPositioningEnabled)
{
    ClusterConformance conformance;
    conformance.GetFeatureMap().Set(Feature::kVentilation).Set(Feature::kPositioning);

    EXPECT_TRUE(conformance.Valid());
}

TEST(TestClosureControlClusterLogic, InvalidWhenVentilationEnabledButPositioningDisabled)
{
    ClusterConformance conformance;
    conformance.GetFeatureMap().Set(Feature::kVentilation);

    EXPECT_FALSE(conformance.Valid());
}

TEST(TestClosureControlClusterLogic, ValidWhenPedestrianAndPositioningEnabled)
{
    ClusterConformance conformance;
    conformance.GetFeatureMap().Set(Feature::kPedestrian).Set(Feature::kPositioning);

    EXPECT_TRUE(conformance.Valid());
}

TEST(TestClosureControlClusterLogic, InvalidWhenPedestrianEnabledButPositioningDisabled)
{
    ClusterConformance conformance;
    conformance.GetFeatureMap().Set(Feature::kPedestrian);

    EXPECT_FALSE(conformance.Valid());
}

TEST(TestClosureControlClusterLogic, ValidWhenCalibrationAndPositioningEnabled)
{
    ClusterConformance conformance;
    conformance.GetFeatureMap().Set(Feature::kCalibration).Set(Feature::kPositioning);

    EXPECT_TRUE(conformance.Valid());
}

TEST(TestClosureControlClusterLogic, InvalidWhenCalibrationEnabledButPositioningDisabled)
{
    ClusterConformance conformance;
    conformance.GetFeatureMap().Set(Feature::kCalibration);

    EXPECT_FALSE(conformance.Valid());
}

TEST(TestClosureControlClusterLogic, ValidWhenVentilationPedestrianCalibrationAndPositioningEnabled)
{
    ClusterConformance conformance;
    conformance.GetFeatureMap()
        .ClearAll()
        .Set(Feature::kVentilation)
        .Set(Feature::kPedestrian)
        .Set(Feature::kCalibration)
        .Set(Feature::kPositioning);

    EXPECT_TRUE(conformance.Valid());
}

TEST(TestClosureControlClusterLogic, InvalidWhenVentilationPedestrianCalibrationEnabledButPositioningDisabled)
{
    ClusterConformance conformance;
    conformance.GetFeatureMap().Set(Feature::kVentilation).Set(Feature::kPedestrian).Set(Feature::kCalibration);

    EXPECT_FALSE(conformance.Valid());
}
