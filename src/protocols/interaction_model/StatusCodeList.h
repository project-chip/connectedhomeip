/*
 *    Copyright (c) 2022 Project CHIP Authors
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

/**
 * This file is designed to be included multiple times, hence does not use
 * include guards or #pragma once.  Consumers must define the
 * CHIP_IM_STATUS_CODE(name, spec_name, value) macro to do whatever they want,
 * include this file, then undefine the macro.
 */

/// WARNING: If you touch this list,
///          please update src/controller/python/chip/interaction_model/__init__.py
///          please update src/controller/java/src/chip/devicecontroller/model/Status.java
///          please update src/controller/java/src/matter/controller/model/Status.kt

// clang-format off
CHIP_IM_STATUS_CODE(Success               , SUCCESS                 , 0x0)
CHIP_IM_STATUS_CODE(Failure               , FAILURE                 , 0x01)
CHIP_IM_STATUS_CODE(InvalidSubscription   , INVALID_SUBSCRIPTION    , 0x7d)
CHIP_IM_STATUS_CODE(UnsupportedAccess     , UNSUPPORTED_ACCESS      , 0x7e)
CHIP_IM_STATUS_CODE(UnsupportedEndpoint   , UNSUPPORTED_ENDPOINT    , 0x7f)
CHIP_IM_STATUS_CODE(InvalidAction         , INVALID_ACTION          , 0x80)
CHIP_IM_STATUS_CODE(UnsupportedCommand    , UNSUPPORTED_COMMAND     , 0x81)
CHIP_IM_STATUS_CODE(Deprecated82          , Deprecated82            , 0x82)
CHIP_IM_STATUS_CODE(Deprecated83          , Deprecated83            , 0x83)
CHIP_IM_STATUS_CODE(Deprecated84          , Deprecated84            , 0x84)
CHIP_IM_STATUS_CODE(InvalidCommand        , INVALID_COMMAND         , 0x85)
CHIP_IM_STATUS_CODE(UnsupportedAttribute  , UNSUPPORTED_ATTRIBUTE   , 0x86)
CHIP_IM_STATUS_CODE(ConstraintError       , CONSTRAINT_ERROR        , 0x87)
CHIP_IM_STATUS_CODE(UnsupportedWrite      , UNSUPPORTED_WRITE       , 0x88)
CHIP_IM_STATUS_CODE(ResourceExhausted     , RESOURCE_EXHAUSTED      , 0x89)
CHIP_IM_STATUS_CODE(Deprecated8a          , Deprecated8a            , 0x8a)
CHIP_IM_STATUS_CODE(NotFound              , NOT_FOUND               , 0x8b)
CHIP_IM_STATUS_CODE(UnreportableAttribute , UNREPORTABLE_ATTRIBUTE  , 0x8c)
CHIP_IM_STATUS_CODE(InvalidDataType       , INVALID_DATA_TYPE       , 0x8d)
CHIP_IM_STATUS_CODE(Deprecated8e          , Deprecated8e            , 0x8e)
CHIP_IM_STATUS_CODE(UnsupportedRead       , UNSUPPORTED_READ        , 0x8f)
CHIP_IM_STATUS_CODE(Deprecated90          , Deprecated90            , 0x90)
CHIP_IM_STATUS_CODE(Deprecated91          , Deprecated91            , 0x91)
CHIP_IM_STATUS_CODE(DataVersionMismatch   , DATA_VERSION_MISMATCH   , 0x92)
CHIP_IM_STATUS_CODE(Deprecated93          , Deprecated93            , 0x93)
CHIP_IM_STATUS_CODE(Timeout               , TIMEOUT                 , 0x94)
CHIP_IM_STATUS_CODE(Reserved95            , Reserved95              , 0x95)
CHIP_IM_STATUS_CODE(Reserved96            , Reserved96              , 0x96)
CHIP_IM_STATUS_CODE(Reserved97            , Reserved97              , 0x97)
CHIP_IM_STATUS_CODE(Reserved98            , Reserved98              , 0x98)
CHIP_IM_STATUS_CODE(Reserved99            , Reserved99              , 0x99)
CHIP_IM_STATUS_CODE(Reserved9a            , Reserved9a              , 0x9a)
CHIP_IM_STATUS_CODE(Busy                  , BUSY                    , 0x9c)
CHIP_IM_STATUS_CODE(AccessRestricted      , ACCESS_RESTRICTED       , 0x9d)
CHIP_IM_STATUS_CODE(Deprecatedc0          , Deprecatedc0            , 0xc0)
CHIP_IM_STATUS_CODE(Deprecatedc1          , Deprecatedc1            , 0xc1)
CHIP_IM_STATUS_CODE(Deprecatedc2          , Deprecatedc2            , 0xc2)
CHIP_IM_STATUS_CODE(UnsupportedCluster    , UNSUPPORTED_CLUSTER     , 0xc3)
CHIP_IM_STATUS_CODE(Deprecatedc4          , Deprecatedc4            , 0xc4)
CHIP_IM_STATUS_CODE(NoUpstreamSubscription, NO_UPSTREAM_SUBSCRIPTION, 0xc5)
CHIP_IM_STATUS_CODE(NeedsTimedInteraction , NEEDS_TIMED_INTERACTION , 0xc6)
CHIP_IM_STATUS_CODE(UnsupportedEvent      , UNSUPPORTED_EVENT       , 0xc7)
CHIP_IM_STATUS_CODE(PathsExhausted        , PATHS_EXHAUSTED         , 0xc8)
CHIP_IM_STATUS_CODE(TimedRequestMismatch  , TIMED_REQUEST_MISMATCH  , 0xc9)
CHIP_IM_STATUS_CODE(FailsafeRequired      , FAILSAFE_REQUIRED       , 0xca)
CHIP_IM_STATUS_CODE(InvalidInState        , INVALID_IN_STATE        , 0xcb)
CHIP_IM_STATUS_CODE(NoCommandResponse     , NO_COMMAND_RESPONSE     , 0xcc)
CHIP_IM_STATUS_CODE(WriteIgnored          , WRITE_IGNORED           , 0xF0) // non-spec error code and use only internally
// clang-format on
