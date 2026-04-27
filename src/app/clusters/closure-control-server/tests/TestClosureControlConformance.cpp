/**
 *
 *    Copyright (c) 2025-2026 Project CHIP Authors
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

#include <app/clusters/closure-control-server/ClosureControlCluster.h>

using chip::app::Clusters::ClosureControl::ClusterConformance;
using chip::app::Clusters::ClosureControl::Feature;

/*
    ClusterConformance Valid Function Test Case
*/

TEST(TestClosureControlConformance, ValidWhenPositioningEnabled)
{
    ClusterConformance conformance;
    conformance.FeatureMap().Set(Feature::kPositioning);

    EXPECT_TRUE(conformance.IsValid());
}

TEST(TestClosureControlConformance, ValidWhenMotionLatchingEnabled)
{
    ClusterConformance conformance;
    conformance.FeatureMap().Set(Feature::kMotionLatching);

    EXPECT_TRUE(conformance.IsValid());
}

TEST(TestClosureControlConformance, InvalidWhenNeitherPositioningNorMotionLatchingEnabled)
{
    ClusterConformance conformance;

    EXPECT_FALSE(conformance.IsValid());
}

TEST(TestClosureControlConformance, ValidWhenSpeedAndPositioningEnabledAndInstantaneousDisabled)
{
    ClusterConformance conformance;
    conformance.FeatureMap().Set(Feature::kSpeed).Set(Feature::kPositioning);

    EXPECT_TRUE(conformance.IsValid());
}

TEST(TestClosureControlConformance, InvalidWhenSpeedEnabledButPositioningDisabled)
{
    ClusterConformance conformance;
    conformance.FeatureMap().Set(Feature::kSpeed);

    EXPECT_FALSE(conformance.IsValid());
}

TEST(TestClosureControlConformance, InvalidWhenSpeedAndInstantaneousBothEnabled)
{
    ClusterConformance conformance;
    conformance.FeatureMap().Set(Feature::kSpeed).Set(Feature::kPositioning).Set(Feature::kInstantaneous);

    EXPECT_FALSE(conformance.IsValid());
}

TEST(TestClosureControlConformance, ValidWhenVentilationAndPositioningEnabled)
{
    ClusterConformance conformance;
    conformance.FeatureMap().Set(Feature::kVentilation).Set(Feature::kPositioning);

    EXPECT_TRUE(conformance.IsValid());
}

TEST(TestClosureControlConformance, InvalidWhenVentilationEnabledButPositioningDisabled)
{
    ClusterConformance conformance;
    conformance.FeatureMap().Set(Feature::kVentilation);

    EXPECT_FALSE(conformance.IsValid());
}

TEST(TestClosureControlConformance, ValidWhenPedestrianAndPositioningEnabled)
{
    ClusterConformance conformance;
    conformance.FeatureMap().Set(Feature::kPedestrian).Set(Feature::kPositioning);

    EXPECT_TRUE(conformance.IsValid());
}

TEST(TestClosureControlConformance, InvalidWhenPedestrianEnabledButPositioningDisabled)
{
    ClusterConformance conformance;
    conformance.FeatureMap().Set(Feature::kPedestrian);

    EXPECT_FALSE(conformance.IsValid());
}

TEST(TestClosureControlConformance, ValidWhenCalibrationAndPositioningEnabled)
{
    ClusterConformance conformance;
    conformance.FeatureMap().Set(Feature::kCalibration).Set(Feature::kPositioning);

    EXPECT_TRUE(conformance.IsValid());
}

TEST(TestClosureControlConformance, InvalidWhenCalibrationEnabledButPositioningDisabled)
{
    ClusterConformance conformance;
    conformance.FeatureMap().Set(Feature::kCalibration);

    EXPECT_FALSE(conformance.IsValid());
}

TEST(TestClosureControlConformance, ValidWhenVentilationPedestrianCalibrationAndPositioningEnabled)
{
    ClusterConformance conformance;
    conformance.FeatureMap()
        .ClearAll()
        .Set(Feature::kVentilation)
        .Set(Feature::kPedestrian)
        .Set(Feature::kCalibration)
        .Set(Feature::kPositioning);

    EXPECT_TRUE(conformance.IsValid());
}

TEST(TestClosureControlConformance, InvalidWhenVentilationPedestrianCalibrationEnabledButPositioningDisabled)
{
    ClusterConformance conformance;
    conformance.FeatureMap().Set(Feature::kVentilation).Set(Feature::kPedestrian).Set(Feature::kCalibration);

    EXPECT_FALSE(conformance.IsValid());
}
