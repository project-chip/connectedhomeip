'''
/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
'''

from ctypes import *
from .ChipStack import *
from .ChipExceptions import *

__all__ = ["ChipCluster"]

class ChipCluster:
    def __init__(self, chipstack, devctl):
        self._ChipStack = chipstack
        self._devCtrl = devctl
        pass

    def SendCommand(self, nodeId: int):
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_SendCommand(nodeId)
        )

    def ListClusters(self):
        return {
            "BarrierControl": {
                "BarrierControlGoToPercent": [
                    ("int", "percentOpen"),
                ],
                "BarrierControlStop": [
                ],
            },
            "Basic": {
                "ResetToFactoryDefaults": [
                ],
            },
            "ColorControl": {
                "MoveColor": [
                    ("int", "rateX"),
                    ("int", "rateY"),
                    ("int", "optionsMask"),
                    ("int", "optionsOverride"),
                ],
                "MoveColorTemperature": [
                    ("int", "moveMode"),
                    ("int", "rate"),
                    ("int", "colorTemperatureMinimum"),
                    ("int", "colorTemperatureMaximum"),
                    ("int", "optionsMask"),
                    ("int", "optionsOverride"),
                ],
                "MoveHue": [
                    ("int", "moveMode"),
                    ("int", "rate"),
                    ("int", "optionsMask"),
                    ("int", "optionsOverride"),
                ],
                "MoveSaturation": [
                    ("int", "moveMode"),
                    ("int", "rate"),
                    ("int", "optionsMask"),
                    ("int", "optionsOverride"),
                ],
                "MoveToColor": [
                    ("int", "colorX"),
                    ("int", "colorY"),
                    ("int", "transitionTime"),
                    ("int", "optionsMask"),
                    ("int", "optionsOverride"),
                ],
                "MoveToColorTemperature": [
                    ("int", "colorTemperature"),
                    ("int", "transitionTime"),
                    ("int", "optionsMask"),
                    ("int", "optionsOverride"),
                ],
                "MoveToHue": [
                    ("int", "hue"),
                    ("int", "direction"),
                    ("int", "transitionTime"),
                    ("int", "optionsMask"),
                    ("int", "optionsOverride"),
                ],
                "MoveToHueAndSaturation": [
                    ("int", "hue"),
                    ("int", "saturation"),
                    ("int", "transitionTime"),
                    ("int", "optionsMask"),
                    ("int", "optionsOverride"),
                ],
                "MoveToSaturation": [
                    ("int", "saturation"),
                    ("int", "transitionTime"),
                    ("int", "optionsMask"),
                    ("int", "optionsOverride"),
                ],
                "StepColor": [
                    ("int", "stepX"),
                    ("int", "stepY"),
                    ("int", "transitionTime"),
                    ("int", "optionsMask"),
                    ("int", "optionsOverride"),
                ],
                "StepColorTemperature": [
                    ("int", "stepMode"),
                    ("int", "stepSize"),
                    ("int", "transitionTime"),
                    ("int", "colorTemperatureMinimum"),
                    ("int", "colorTemperatureMaximum"),
                    ("int", "optionsMask"),
                    ("int", "optionsOverride"),
                ],
                "StepHue": [
                    ("int", "stepMode"),
                    ("int", "stepSize"),
                    ("int", "transitionTime"),
                    ("int", "optionsMask"),
                    ("int", "optionsOverride"),
                ],
                "StepSaturation": [
                    ("int", "stepMode"),
                    ("int", "stepSize"),
                    ("int", "transitionTime"),
                    ("int", "optionsMask"),
                    ("int", "optionsOverride"),
                ],
                "StopMoveStep": [
                    ("int", "optionsMask"),
                    ("int", "optionsOverride"),
                ],
            },
            "DoorLock": {
                "ClearAllPins": [
                ],
                "ClearAllRfids": [
                ],
                "ClearHolidaySchedule": [
                    ("int", "scheduleId"),
                ],
                "ClearPin": [
                    ("int", "userId"),
                ],
                "ClearRfid": [
                    ("int", "userId"),
                ],
                "ClearWeekdaySchedule": [
                    ("int", "scheduleId"),
                    ("int", "userId"),
                ],
                "ClearYeardaySchedule": [
                    ("int", "scheduleId"),
                    ("int", "userId"),
                ],
                "GetHolidaySchedule": [
                    ("int", "scheduleId"),
                ],
                "GetLogRecord": [
                    ("int", "logIndex"),
                ],
                "GetPin": [
                    ("int", "userId"),
                ],
                "GetRfid": [
                    ("int", "userId"),
                ],
                "GetUserType": [
                    ("int", "userId"),
                ],
                "GetWeekdaySchedule": [
                    ("int", "scheduleId"),
                    ("int", "userId"),
                ],
                "GetYeardaySchedule": [
                    ("int", "scheduleId"),
                    ("int", "userId"),
                ],
                "LockDoor": [
                    ("str", "pin"),
                ],
                "SetHolidaySchedule": [
                    ("int", "scheduleId"),
                    ("int", "localStartTime"),
                    ("int", "localEndTime"),
                    ("int", "operatingModeDuringHoliday"),
                ],
                "SetPin": [
                    ("int", "userId"),
                    ("int", "userStatus"),
                    ("int", "userType"),
                    ("str", "pin"),
                ],
                "SetRfid": [
                    ("int", "userId"),
                    ("int", "userStatus"),
                    ("int", "userType"),
                    ("str", "id"),
                ],
                "SetUserType": [
                    ("int", "userId"),
                    ("int", "userType"),
                ],
                "SetWeekdaySchedule": [
                    ("int", "scheduleId"),
                    ("int", "userId"),
                    ("int", "daysMask"),
                    ("int", "startHour"),
                    ("int", "startMinute"),
                    ("int", "endHour"),
                    ("int", "endMinute"),
                ],
                "SetYeardaySchedule": [
                    ("int", "scheduleId"),
                    ("int", "userId"),
                    ("int", "localStartTime"),
                    ("int", "localEndTime"),
                ],
                "UnlockDoor": [
                    ("str", "pin"),
                ],
                "UnlockWithTimeout": [
                    ("int", "timeoutInSeconds"),
                    ("str", "pin"),
                ],
            },
            "Groups": {
                "AddGroup": [
                    ("int", "groupId"),
                    ("str", "groupName"),
                ],
                "AddGroupIfIdentifying": [
                    ("int", "groupId"),
                    ("str", "groupName"),
                ],
                "GetGroupMembership": [
                    ("int", "groupCount"),
                    ("int", "groupList"),
                ],
                "RemoveAllGroups": [
                ],
                "RemoveGroup": [
                    ("int", "groupId"),
                ],
                "ViewGroup": [
                    ("int", "groupId"),
                ],
            },
            "IasZone": {
            },
            "Identify": {
                "Identify": [
                    ("int", "identifyTime"),
                ],
                "IdentifyQuery": [
                ],
            },
            "LevelControl": {
                "Move": [
                    ("int", "moveMode"),
                    ("int", "rate"),
                    ("int", "optionMask"),
                    ("int", "optionOverride"),
                ],
                "MoveToLevel": [
                    ("int", "level"),
                    ("int", "transitionTime"),
                    ("int", "optionMask"),
                    ("int", "optionOverride"),
                ],
                "MoveToLevelWithOnOff": [
                    ("int", "level"),
                    ("int", "transitionTime"),
                ],
                "MoveWithOnOff": [
                    ("int", "moveMode"),
                    ("int", "rate"),
                ],
                "Step": [
                    ("int", "stepMode"),
                    ("int", "stepSize"),
                    ("int", "transitionTime"),
                    ("int", "optionMask"),
                    ("int", "optionOverride"),
                ],
                "StepWithOnOff": [
                    ("int", "stepMode"),
                    ("int", "stepSize"),
                    ("int", "transitionTime"),
                ],
                "Stop": [
                    ("int", "optionMask"),
                    ("int", "optionOverride"),
                ],
                "StopWithOnOff": [
                ],
            },
            "OnOff": {
                "Off": [
                ],
                "On": [
                ],
                "Toggle": [
                ],
            },
            "Scenes": {
                "AddScene": [
                    ("int", "groupId"),
                    ("int", "sceneId"),
                    ("int", "transitionTime"),
                    ("str", "sceneName"),
                    ("int", "clusterId"),
                    ("int", "length"),
                    ("int", "value"),
                ],
                "GetSceneMembership": [
                    ("int", "groupId"),
                ],
                "RecallScene": [
                    ("int", "groupId"),
                    ("int", "sceneId"),
                    ("int", "transitionTime"),
                ],
                "RemoveAllScenes": [
                    ("int", "groupId"),
                ],
                "RemoveScene": [
                    ("int", "groupId"),
                    ("int", "sceneId"),
                ],
                "StoreScene": [
                    ("int", "groupId"),
                    ("int", "sceneId"),
                ],
                "ViewScene": [
                    ("int", "groupId"),
                    ("int", "sceneId"),
                ],
            },
            "TemperatureMeasurement": {
            },
        }

    def PrepareCommand(self, cluster: str, command: str, endpoint: int, groupid: int, args):
        func = getattr(self, "Cluster{}_Command{}".format(cluster, command))
        if not func:
            raise UnknownCluster(cluster)
        func(endpoint, groupid, args)

    def ClusterBarrierControl_CommandBarrierControlGoToPercent(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        percentOpen = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_BarrierControl_BarrierControlGoToPercent(
                ZCLendpoint, ZCLgroupid, percentOpen
            )
        )

    def ClusterBarrierControl_CommandBarrierControlStop(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_BarrierControl_BarrierControlStop(
                ZCLendpoint, ZCLgroupid
            )
        )

    def ClusterBasic_CommandResetToFactoryDefaults(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_Basic_ResetToFactoryDefaults(
                ZCLendpoint, ZCLgroupid
            )
        )

    def ClusterColorControl_CommandMoveColor(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1 + 1 + 1 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        rateX = int(args[argCnt])
        argCnt += 1
        rateY = int(args[argCnt])
        argCnt += 1
        optionsMask = int(args[argCnt])
        argCnt += 1
        optionsOverride = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_ColorControl_MoveColor(
                ZCLendpoint, ZCLgroupid, rateX, rateY, optionsMask, optionsOverride
            )
        )

    def ClusterColorControl_CommandMoveColorTemperature(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1 + 1 + 1 + 1 + 1 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        moveMode = int(args[argCnt])
        argCnt += 1
        rate = int(args[argCnt])
        argCnt += 1
        colorTemperatureMinimum = int(args[argCnt])
        argCnt += 1
        colorTemperatureMaximum = int(args[argCnt])
        argCnt += 1
        optionsMask = int(args[argCnt])
        argCnt += 1
        optionsOverride = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_ColorControl_MoveColorTemperature(
                ZCLendpoint, ZCLgroupid, moveMode, rate, colorTemperatureMinimum, colorTemperatureMaximum, optionsMask, optionsOverride
            )
        )

    def ClusterColorControl_CommandMoveHue(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1 + 1 + 1 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        moveMode = int(args[argCnt])
        argCnt += 1
        rate = int(args[argCnt])
        argCnt += 1
        optionsMask = int(args[argCnt])
        argCnt += 1
        optionsOverride = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_ColorControl_MoveHue(
                ZCLendpoint, ZCLgroupid, moveMode, rate, optionsMask, optionsOverride
            )
        )

    def ClusterColorControl_CommandMoveSaturation(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1 + 1 + 1 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        moveMode = int(args[argCnt])
        argCnt += 1
        rate = int(args[argCnt])
        argCnt += 1
        optionsMask = int(args[argCnt])
        argCnt += 1
        optionsOverride = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_ColorControl_MoveSaturation(
                ZCLendpoint, ZCLgroupid, moveMode, rate, optionsMask, optionsOverride
            )
        )

    def ClusterColorControl_CommandMoveToColor(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1 + 1 + 1 + 1 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        colorX = int(args[argCnt])
        argCnt += 1
        colorY = int(args[argCnt])
        argCnt += 1
        transitionTime = int(args[argCnt])
        argCnt += 1
        optionsMask = int(args[argCnt])
        argCnt += 1
        optionsOverride = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_ColorControl_MoveToColor(
                ZCLendpoint, ZCLgroupid, colorX, colorY, transitionTime, optionsMask, optionsOverride
            )
        )

    def ClusterColorControl_CommandMoveToColorTemperature(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1 + 1 + 1 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        colorTemperature = int(args[argCnt])
        argCnt += 1
        transitionTime = int(args[argCnt])
        argCnt += 1
        optionsMask = int(args[argCnt])
        argCnt += 1
        optionsOverride = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_ColorControl_MoveToColorTemperature(
                ZCLendpoint, ZCLgroupid, colorTemperature, transitionTime, optionsMask, optionsOverride
            )
        )

    def ClusterColorControl_CommandMoveToHue(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1 + 1 + 1 + 1 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        hue = int(args[argCnt])
        argCnt += 1
        direction = int(args[argCnt])
        argCnt += 1
        transitionTime = int(args[argCnt])
        argCnt += 1
        optionsMask = int(args[argCnt])
        argCnt += 1
        optionsOverride = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_ColorControl_MoveToHue(
                ZCLendpoint, ZCLgroupid, hue, direction, transitionTime, optionsMask, optionsOverride
            )
        )

    def ClusterColorControl_CommandMoveToHueAndSaturation(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1 + 1 + 1 + 1 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        hue = int(args[argCnt])
        argCnt += 1
        saturation = int(args[argCnt])
        argCnt += 1
        transitionTime = int(args[argCnt])
        argCnt += 1
        optionsMask = int(args[argCnt])
        argCnt += 1
        optionsOverride = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_ColorControl_MoveToHueAndSaturation(
                ZCLendpoint, ZCLgroupid, hue, saturation, transitionTime, optionsMask, optionsOverride
            )
        )

    def ClusterColorControl_CommandMoveToSaturation(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1 + 1 + 1 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        saturation = int(args[argCnt])
        argCnt += 1
        transitionTime = int(args[argCnt])
        argCnt += 1
        optionsMask = int(args[argCnt])
        argCnt += 1
        optionsOverride = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_ColorControl_MoveToSaturation(
                ZCLendpoint, ZCLgroupid, saturation, transitionTime, optionsMask, optionsOverride
            )
        )

    def ClusterColorControl_CommandStepColor(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1 + 1 + 1 + 1 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        stepX = int(args[argCnt])
        argCnt += 1
        stepY = int(args[argCnt])
        argCnt += 1
        transitionTime = int(args[argCnt])
        argCnt += 1
        optionsMask = int(args[argCnt])
        argCnt += 1
        optionsOverride = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_ColorControl_StepColor(
                ZCLendpoint, ZCLgroupid, stepX, stepY, transitionTime, optionsMask, optionsOverride
            )
        )

    def ClusterColorControl_CommandStepColorTemperature(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1 + 1 + 1 + 1 + 1 + 1 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        stepMode = int(args[argCnt])
        argCnt += 1
        stepSize = int(args[argCnt])
        argCnt += 1
        transitionTime = int(args[argCnt])
        argCnt += 1
        colorTemperatureMinimum = int(args[argCnt])
        argCnt += 1
        colorTemperatureMaximum = int(args[argCnt])
        argCnt += 1
        optionsMask = int(args[argCnt])
        argCnt += 1
        optionsOverride = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_ColorControl_StepColorTemperature(
                ZCLendpoint, ZCLgroupid, stepMode, stepSize, transitionTime, colorTemperatureMinimum, colorTemperatureMaximum, optionsMask, optionsOverride
            )
        )

    def ClusterColorControl_CommandStepHue(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1 + 1 + 1 + 1 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        stepMode = int(args[argCnt])
        argCnt += 1
        stepSize = int(args[argCnt])
        argCnt += 1
        transitionTime = int(args[argCnt])
        argCnt += 1
        optionsMask = int(args[argCnt])
        argCnt += 1
        optionsOverride = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_ColorControl_StepHue(
                ZCLendpoint, ZCLgroupid, stepMode, stepSize, transitionTime, optionsMask, optionsOverride
            )
        )

    def ClusterColorControl_CommandStepSaturation(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1 + 1 + 1 + 1 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        stepMode = int(args[argCnt])
        argCnt += 1
        stepSize = int(args[argCnt])
        argCnt += 1
        transitionTime = int(args[argCnt])
        argCnt += 1
        optionsMask = int(args[argCnt])
        argCnt += 1
        optionsOverride = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_ColorControl_StepSaturation(
                ZCLendpoint, ZCLgroupid, stepMode, stepSize, transitionTime, optionsMask, optionsOverride
            )
        )

    def ClusterColorControl_CommandStopMoveStep(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        optionsMask = int(args[argCnt])
        argCnt += 1
        optionsOverride = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_ColorControl_StopMoveStep(
                ZCLendpoint, ZCLgroupid, optionsMask, optionsOverride
            )
        )

    def ClusterDoorLock_CommandClearAllPins(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_DoorLock_ClearAllPins(
                ZCLendpoint, ZCLgroupid
            )
        )

    def ClusterDoorLock_CommandClearAllRfids(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_DoorLock_ClearAllRfids(
                ZCLendpoint, ZCLgroupid
            )
        )

    def ClusterDoorLock_CommandClearHolidaySchedule(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        scheduleId = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_DoorLock_ClearHolidaySchedule(
                ZCLendpoint, ZCLgroupid, scheduleId
            )
        )

    def ClusterDoorLock_CommandClearPin(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        userId = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_DoorLock_ClearPin(
                ZCLendpoint, ZCLgroupid, userId
            )
        )

    def ClusterDoorLock_CommandClearRfid(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        userId = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_DoorLock_ClearRfid(
                ZCLendpoint, ZCLgroupid, userId
            )
        )

    def ClusterDoorLock_CommandClearWeekdaySchedule(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        scheduleId = int(args[argCnt])
        argCnt += 1
        userId = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_DoorLock_ClearWeekdaySchedule(
                ZCLendpoint, ZCLgroupid, scheduleId, userId
            )
        )

    def ClusterDoorLock_CommandClearYeardaySchedule(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        scheduleId = int(args[argCnt])
        argCnt += 1
        userId = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_DoorLock_ClearYeardaySchedule(
                ZCLendpoint, ZCLgroupid, scheduleId, userId
            )
        )

    def ClusterDoorLock_CommandGetHolidaySchedule(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        scheduleId = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_DoorLock_GetHolidaySchedule(
                ZCLendpoint, ZCLgroupid, scheduleId
            )
        )

    def ClusterDoorLock_CommandGetLogRecord(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        logIndex = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_DoorLock_GetLogRecord(
                ZCLendpoint, ZCLgroupid, logIndex
            )
        )

    def ClusterDoorLock_CommandGetPin(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        userId = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_DoorLock_GetPin(
                ZCLendpoint, ZCLgroupid, userId
            )
        )

    def ClusterDoorLock_CommandGetRfid(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        userId = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_DoorLock_GetRfid(
                ZCLendpoint, ZCLgroupid, userId
            )
        )

    def ClusterDoorLock_CommandGetUserType(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        userId = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_DoorLock_GetUserType(
                ZCLendpoint, ZCLgroupid, userId
            )
        )

    def ClusterDoorLock_CommandGetWeekdaySchedule(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        scheduleId = int(args[argCnt])
        argCnt += 1
        userId = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_DoorLock_GetWeekdaySchedule(
                ZCLendpoint, ZCLgroupid, scheduleId, userId
            )
        )

    def ClusterDoorLock_CommandGetYeardaySchedule(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        scheduleId = int(args[argCnt])
        argCnt += 1
        userId = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_DoorLock_GetYeardaySchedule(
                ZCLendpoint, ZCLgroupid, scheduleId, userId
            )
        )

    def ClusterDoorLock_CommandLockDoor(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        pin = args[argCnt].encode("utf-8")
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_DoorLock_LockDoor(
                ZCLendpoint, ZCLgroupid, pin
            )
        )

    def ClusterDoorLock_CommandSetHolidaySchedule(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1 + 1 + 1 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        scheduleId = int(args[argCnt])
        argCnt += 1
        localStartTime = int(args[argCnt])
        argCnt += 1
        localEndTime = int(args[argCnt])
        argCnt += 1
        operatingModeDuringHoliday = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_DoorLock_SetHolidaySchedule(
                ZCLendpoint, ZCLgroupid, scheduleId, localStartTime, localEndTime, operatingModeDuringHoliday
            )
        )

    def ClusterDoorLock_CommandSetPin(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1 + 1 + 1 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        userId = int(args[argCnt])
        argCnt += 1
        userStatus = int(args[argCnt])
        argCnt += 1
        userType = int(args[argCnt])
        argCnt += 1
        pin = args[argCnt].encode("utf-8")
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_DoorLock_SetPin(
                ZCLendpoint, ZCLgroupid, userId, userStatus, userType, pin
            )
        )

    def ClusterDoorLock_CommandSetRfid(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1 + 1 + 1 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        userId = int(args[argCnt])
        argCnt += 1
        userStatus = int(args[argCnt])
        argCnt += 1
        userType = int(args[argCnt])
        argCnt += 1
        id = args[argCnt].encode("utf-8")
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_DoorLock_SetRfid(
                ZCLendpoint, ZCLgroupid, userId, userStatus, userType, id
            )
        )

    def ClusterDoorLock_CommandSetUserType(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        userId = int(args[argCnt])
        argCnt += 1
        userType = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_DoorLock_SetUserType(
                ZCLendpoint, ZCLgroupid, userId, userType
            )
        )

    def ClusterDoorLock_CommandSetWeekdaySchedule(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1 + 1 + 1 + 1 + 1 + 1 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        scheduleId = int(args[argCnt])
        argCnt += 1
        userId = int(args[argCnt])
        argCnt += 1
        daysMask = int(args[argCnt])
        argCnt += 1
        startHour = int(args[argCnt])
        argCnt += 1
        startMinute = int(args[argCnt])
        argCnt += 1
        endHour = int(args[argCnt])
        argCnt += 1
        endMinute = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_DoorLock_SetWeekdaySchedule(
                ZCLendpoint, ZCLgroupid, scheduleId, userId, daysMask, startHour, startMinute, endHour, endMinute
            )
        )

    def ClusterDoorLock_CommandSetYeardaySchedule(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1 + 1 + 1 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        scheduleId = int(args[argCnt])
        argCnt += 1
        userId = int(args[argCnt])
        argCnt += 1
        localStartTime = int(args[argCnt])
        argCnt += 1
        localEndTime = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_DoorLock_SetYeardaySchedule(
                ZCLendpoint, ZCLgroupid, scheduleId, userId, localStartTime, localEndTime
            )
        )

    def ClusterDoorLock_CommandUnlockDoor(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        pin = args[argCnt].encode("utf-8")
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_DoorLock_UnlockDoor(
                ZCLendpoint, ZCLgroupid, pin
            )
        )

    def ClusterDoorLock_CommandUnlockWithTimeout(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        timeoutInSeconds = int(args[argCnt])
        argCnt += 1
        pin = args[argCnt].encode("utf-8")
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_DoorLock_UnlockWithTimeout(
                ZCLendpoint, ZCLgroupid, timeoutInSeconds, pin
            )
        )

    def ClusterGroups_CommandAddGroup(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        groupId = int(args[argCnt])
        argCnt += 1
        groupName = args[argCnt].encode("utf-8")
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_Groups_AddGroup(
                ZCLendpoint, ZCLgroupid, groupId, groupName
            )
        )

    def ClusterGroups_CommandAddGroupIfIdentifying(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        groupId = int(args[argCnt])
        argCnt += 1
        groupName = args[argCnt].encode("utf-8")
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_Groups_AddGroupIfIdentifying(
                ZCLendpoint, ZCLgroupid, groupId, groupName
            )
        )

    def ClusterGroups_CommandGetGroupMembership(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        groupCount = int(args[argCnt])
        argCnt += 1
        groupList = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_Groups_GetGroupMembership(
                ZCLendpoint, ZCLgroupid, groupCount, groupList
            )
        )

    def ClusterGroups_CommandRemoveAllGroups(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_Groups_RemoveAllGroups(
                ZCLendpoint, ZCLgroupid
            )
        )

    def ClusterGroups_CommandRemoveGroup(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        groupId = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_Groups_RemoveGroup(
                ZCLendpoint, ZCLgroupid, groupId
            )
        )

    def ClusterGroups_CommandViewGroup(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        groupId = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_Groups_ViewGroup(
                ZCLendpoint, ZCLgroupid, groupId
            )
        )

    def ClusterIdentify_CommandIdentify(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        identifyTime = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_Identify_Identify(
                ZCLendpoint, ZCLgroupid, identifyTime
            )
        )

    def ClusterIdentify_CommandIdentifyQuery(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_Identify_IdentifyQuery(
                ZCLendpoint, ZCLgroupid
            )
        )

    def ClusterLevelControl_CommandMove(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1 + 1 + 1 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        moveMode = int(args[argCnt])
        argCnt += 1
        rate = int(args[argCnt])
        argCnt += 1
        optionMask = int(args[argCnt])
        argCnt += 1
        optionOverride = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_LevelControl_Move(
                ZCLendpoint, ZCLgroupid, moveMode, rate, optionMask, optionOverride
            )
        )

    def ClusterLevelControl_CommandMoveToLevel(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1 + 1 + 1 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        level = int(args[argCnt])
        argCnt += 1
        transitionTime = int(args[argCnt])
        argCnt += 1
        optionMask = int(args[argCnt])
        argCnt += 1
        optionOverride = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_LevelControl_MoveToLevel(
                ZCLendpoint, ZCLgroupid, level, transitionTime, optionMask, optionOverride
            )
        )

    def ClusterLevelControl_CommandMoveToLevelWithOnOff(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        level = int(args[argCnt])
        argCnt += 1
        transitionTime = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_LevelControl_MoveToLevelWithOnOff(
                ZCLendpoint, ZCLgroupid, level, transitionTime
            )
        )

    def ClusterLevelControl_CommandMoveWithOnOff(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        moveMode = int(args[argCnt])
        argCnt += 1
        rate = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_LevelControl_MoveWithOnOff(
                ZCLendpoint, ZCLgroupid, moveMode, rate
            )
        )

    def ClusterLevelControl_CommandStep(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1 + 1 + 1 + 1 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        stepMode = int(args[argCnt])
        argCnt += 1
        stepSize = int(args[argCnt])
        argCnt += 1
        transitionTime = int(args[argCnt])
        argCnt += 1
        optionMask = int(args[argCnt])
        argCnt += 1
        optionOverride = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_LevelControl_Step(
                ZCLendpoint, ZCLgroupid, stepMode, stepSize, transitionTime, optionMask, optionOverride
            )
        )

    def ClusterLevelControl_CommandStepWithOnOff(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1 + 1 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        stepMode = int(args[argCnt])
        argCnt += 1
        stepSize = int(args[argCnt])
        argCnt += 1
        transitionTime = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_LevelControl_StepWithOnOff(
                ZCLendpoint, ZCLgroupid, stepMode, stepSize, transitionTime
            )
        )

    def ClusterLevelControl_CommandStop(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        optionMask = int(args[argCnt])
        argCnt += 1
        optionOverride = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_LevelControl_Stop(
                ZCLendpoint, ZCLgroupid, optionMask, optionOverride
            )
        )

    def ClusterLevelControl_CommandStopWithOnOff(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_LevelControl_StopWithOnOff(
                ZCLendpoint, ZCLgroupid
            )
        )

    def ClusterOnOff_CommandOff(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_OnOff_Off(
                ZCLendpoint, ZCLgroupid
            )
        )

    def ClusterOnOff_CommandOn(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_OnOff_On(
                ZCLendpoint, ZCLgroupid
            )
        )

    def ClusterOnOff_CommandToggle(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_OnOff_Toggle(
                ZCLendpoint, ZCLgroupid
            )
        )

    def ClusterScenes_CommandAddScene(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1 + 1 + 1 + 1 + 1 + 1 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        groupId = int(args[argCnt])
        argCnt += 1
        sceneId = int(args[argCnt])
        argCnt += 1
        transitionTime = int(args[argCnt])
        argCnt += 1
        sceneName = args[argCnt].encode("utf-8")
        argCnt += 1
        clusterId = int(args[argCnt])
        argCnt += 1
        length = int(args[argCnt])
        argCnt += 1
        value = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_Scenes_AddScene(
                ZCLendpoint, ZCLgroupid, groupId, sceneId, transitionTime, sceneName, clusterId, length, value
            )
        )

    def ClusterScenes_CommandGetSceneMembership(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        groupId = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_Scenes_GetSceneMembership(
                ZCLendpoint, ZCLgroupid, groupId
            )
        )

    def ClusterScenes_CommandRecallScene(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1 + 1 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        groupId = int(args[argCnt])
        argCnt += 1
        sceneId = int(args[argCnt])
        argCnt += 1
        transitionTime = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_Scenes_RecallScene(
                ZCLendpoint, ZCLgroupid, groupId, sceneId, transitionTime
            )
        )

    def ClusterScenes_CommandRemoveAllScenes(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        groupId = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_Scenes_RemoveAllScenes(
                ZCLendpoint, ZCLgroupid, groupId
            )
        )

    def ClusterScenes_CommandRemoveScene(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        groupId = int(args[argCnt])
        argCnt += 1
        sceneId = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_Scenes_RemoveScene(
                ZCLendpoint, ZCLgroupid, groupId, sceneId
            )
        )

    def ClusterScenes_CommandStoreScene(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        groupId = int(args[argCnt])
        argCnt += 1
        sceneId = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_Scenes_StoreScene(
                ZCLendpoint, ZCLgroupid, groupId, sceneId
            )
        )

    def ClusterScenes_CommandViewScene(self, ZCLendpoint: int, ZCLgroupid: int, args):
        totArgs = 0 + 1 + 1
        if len(args) != totArgs:
            raise InvalidArgumentCount(totArgs, len(args))
        argCnt = 0
        groupId = int(args[argCnt])
        argCnt += 1
        sceneId = int(args[argCnt])
        argCnt += 1
        self._ChipStack.Call(
            lambda: self._dmLib.chip_ime_AppendCommand_Scenes_ViewScene(
                ZCLendpoint, ZCLgroupid, groupId, sceneId
            )
        )

    def InitLib(self, dmLib):
        self._dmLib = dmLib
        self._dmLib.chip_ime_SendCommand.argtypes = [c_uint64]
        self._dmLib.chip_ime_SendCommand.restype = c_uint32
        # Cluster BarrierControl
        # Cluster BarrierControl Command BarrierControlGoToPercent
        self._dmLib.chip_ime_AppendCommand_BarrierControl_BarrierControlGoToPercent.argtypes = [c_uint8, c_uint16, c_uint8]
        self._dmLib.chip_ime_AppendCommand_BarrierControl_BarrierControlGoToPercent.restype = c_uint32
        # Cluster BarrierControl Command BarrierControlStop
        self._dmLib.chip_ime_AppendCommand_BarrierControl_BarrierControlStop.argtypes = [c_uint8, c_uint16]
        self._dmLib.chip_ime_AppendCommand_BarrierControl_BarrierControlStop.restype = c_uint32
        # Cluster Basic
        # Cluster Basic Command ResetToFactoryDefaults
        self._dmLib.chip_ime_AppendCommand_Basic_ResetToFactoryDefaults.argtypes = [c_uint8, c_uint16]
        self._dmLib.chip_ime_AppendCommand_Basic_ResetToFactoryDefaults.restype = c_uint32
        # Cluster ColorControl
        # Cluster ColorControl Command MoveColor
        self._dmLib.chip_ime_AppendCommand_ColorControl_MoveColor.argtypes = [c_uint8, c_uint16, c_int16, c_int16, c_uint8, c_uint8]
        self._dmLib.chip_ime_AppendCommand_ColorControl_MoveColor.restype = c_uint32
        # Cluster ColorControl Command MoveColorTemperature
        self._dmLib.chip_ime_AppendCommand_ColorControl_MoveColorTemperature.argtypes = [c_uint8, c_uint16, c_uint8, c_uint16, c_uint16, c_uint16, c_uint8, c_uint8]
        self._dmLib.chip_ime_AppendCommand_ColorControl_MoveColorTemperature.restype = c_uint32
        # Cluster ColorControl Command MoveHue
        self._dmLib.chip_ime_AppendCommand_ColorControl_MoveHue.argtypes = [c_uint8, c_uint16, c_uint8, c_uint8, c_uint8, c_uint8]
        self._dmLib.chip_ime_AppendCommand_ColorControl_MoveHue.restype = c_uint32
        # Cluster ColorControl Command MoveSaturation
        self._dmLib.chip_ime_AppendCommand_ColorControl_MoveSaturation.argtypes = [c_uint8, c_uint16, c_uint8, c_uint8, c_uint8, c_uint8]
        self._dmLib.chip_ime_AppendCommand_ColorControl_MoveSaturation.restype = c_uint32
        # Cluster ColorControl Command MoveToColor
        self._dmLib.chip_ime_AppendCommand_ColorControl_MoveToColor.argtypes = [c_uint8, c_uint16, c_uint16, c_uint16, c_uint16, c_uint8, c_uint8]
        self._dmLib.chip_ime_AppendCommand_ColorControl_MoveToColor.restype = c_uint32
        # Cluster ColorControl Command MoveToColorTemperature
        self._dmLib.chip_ime_AppendCommand_ColorControl_MoveToColorTemperature.argtypes = [c_uint8, c_uint16, c_uint16, c_uint16, c_uint8, c_uint8]
        self._dmLib.chip_ime_AppendCommand_ColorControl_MoveToColorTemperature.restype = c_uint32
        # Cluster ColorControl Command MoveToHue
        self._dmLib.chip_ime_AppendCommand_ColorControl_MoveToHue.argtypes = [c_uint8, c_uint16, c_uint8, c_uint8, c_uint16, c_uint8, c_uint8]
        self._dmLib.chip_ime_AppendCommand_ColorControl_MoveToHue.restype = c_uint32
        # Cluster ColorControl Command MoveToHueAndSaturation
        self._dmLib.chip_ime_AppendCommand_ColorControl_MoveToHueAndSaturation.argtypes = [c_uint8, c_uint16, c_uint8, c_uint8, c_uint16, c_uint8, c_uint8]
        self._dmLib.chip_ime_AppendCommand_ColorControl_MoveToHueAndSaturation.restype = c_uint32
        # Cluster ColorControl Command MoveToSaturation
        self._dmLib.chip_ime_AppendCommand_ColorControl_MoveToSaturation.argtypes = [c_uint8, c_uint16, c_uint8, c_uint16, c_uint8, c_uint8]
        self._dmLib.chip_ime_AppendCommand_ColorControl_MoveToSaturation.restype = c_uint32
        # Cluster ColorControl Command StepColor
        self._dmLib.chip_ime_AppendCommand_ColorControl_StepColor.argtypes = [c_uint8, c_uint16, c_int16, c_int16, c_uint16, c_uint8, c_uint8]
        self._dmLib.chip_ime_AppendCommand_ColorControl_StepColor.restype = c_uint32
        # Cluster ColorControl Command StepColorTemperature
        self._dmLib.chip_ime_AppendCommand_ColorControl_StepColorTemperature.argtypes = [c_uint8, c_uint16, c_uint8, c_uint16, c_uint16, c_uint16, c_uint16, c_uint8, c_uint8]
        self._dmLib.chip_ime_AppendCommand_ColorControl_StepColorTemperature.restype = c_uint32
        # Cluster ColorControl Command StepHue
        self._dmLib.chip_ime_AppendCommand_ColorControl_StepHue.argtypes = [c_uint8, c_uint16, c_uint8, c_uint8, c_uint8, c_uint8, c_uint8]
        self._dmLib.chip_ime_AppendCommand_ColorControl_StepHue.restype = c_uint32
        # Cluster ColorControl Command StepSaturation
        self._dmLib.chip_ime_AppendCommand_ColorControl_StepSaturation.argtypes = [c_uint8, c_uint16, c_uint8, c_uint8, c_uint8, c_uint8, c_uint8]
        self._dmLib.chip_ime_AppendCommand_ColorControl_StepSaturation.restype = c_uint32
        # Cluster ColorControl Command StopMoveStep
        self._dmLib.chip_ime_AppendCommand_ColorControl_StopMoveStep.argtypes = [c_uint8, c_uint16, c_uint8, c_uint8]
        self._dmLib.chip_ime_AppendCommand_ColorControl_StopMoveStep.restype = c_uint32
        # Cluster DoorLock
        # Cluster DoorLock Command ClearAllPins
        self._dmLib.chip_ime_AppendCommand_DoorLock_ClearAllPins.argtypes = [c_uint8, c_uint16]
        self._dmLib.chip_ime_AppendCommand_DoorLock_ClearAllPins.restype = c_uint32
        # Cluster DoorLock Command ClearAllRfids
        self._dmLib.chip_ime_AppendCommand_DoorLock_ClearAllRfids.argtypes = [c_uint8, c_uint16]
        self._dmLib.chip_ime_AppendCommand_DoorLock_ClearAllRfids.restype = c_uint32
        # Cluster DoorLock Command ClearHolidaySchedule
        self._dmLib.chip_ime_AppendCommand_DoorLock_ClearHolidaySchedule.argtypes = [c_uint8, c_uint16, c_uint8]
        self._dmLib.chip_ime_AppendCommand_DoorLock_ClearHolidaySchedule.restype = c_uint32
        # Cluster DoorLock Command ClearPin
        self._dmLib.chip_ime_AppendCommand_DoorLock_ClearPin.argtypes = [c_uint8, c_uint16, c_uint16]
        self._dmLib.chip_ime_AppendCommand_DoorLock_ClearPin.restype = c_uint32
        # Cluster DoorLock Command ClearRfid
        self._dmLib.chip_ime_AppendCommand_DoorLock_ClearRfid.argtypes = [c_uint8, c_uint16, c_uint16]
        self._dmLib.chip_ime_AppendCommand_DoorLock_ClearRfid.restype = c_uint32
        # Cluster DoorLock Command ClearWeekdaySchedule
        self._dmLib.chip_ime_AppendCommand_DoorLock_ClearWeekdaySchedule.argtypes = [c_uint8, c_uint16, c_uint8, c_uint16]
        self._dmLib.chip_ime_AppendCommand_DoorLock_ClearWeekdaySchedule.restype = c_uint32
        # Cluster DoorLock Command ClearYeardaySchedule
        self._dmLib.chip_ime_AppendCommand_DoorLock_ClearYeardaySchedule.argtypes = [c_uint8, c_uint16, c_uint8, c_uint16]
        self._dmLib.chip_ime_AppendCommand_DoorLock_ClearYeardaySchedule.restype = c_uint32
        # Cluster DoorLock Command GetHolidaySchedule
        self._dmLib.chip_ime_AppendCommand_DoorLock_GetHolidaySchedule.argtypes = [c_uint8, c_uint16, c_uint8]
        self._dmLib.chip_ime_AppendCommand_DoorLock_GetHolidaySchedule.restype = c_uint32
        # Cluster DoorLock Command GetLogRecord
        self._dmLib.chip_ime_AppendCommand_DoorLock_GetLogRecord.argtypes = [c_uint8, c_uint16, c_uint16]
        self._dmLib.chip_ime_AppendCommand_DoorLock_GetLogRecord.restype = c_uint32
        # Cluster DoorLock Command GetPin
        self._dmLib.chip_ime_AppendCommand_DoorLock_GetPin.argtypes = [c_uint8, c_uint16, c_uint16]
        self._dmLib.chip_ime_AppendCommand_DoorLock_GetPin.restype = c_uint32
        # Cluster DoorLock Command GetRfid
        self._dmLib.chip_ime_AppendCommand_DoorLock_GetRfid.argtypes = [c_uint8, c_uint16, c_uint16]
        self._dmLib.chip_ime_AppendCommand_DoorLock_GetRfid.restype = c_uint32
        # Cluster DoorLock Command GetUserType
        self._dmLib.chip_ime_AppendCommand_DoorLock_GetUserType.argtypes = [c_uint8, c_uint16, c_uint16]
        self._dmLib.chip_ime_AppendCommand_DoorLock_GetUserType.restype = c_uint32
        # Cluster DoorLock Command GetWeekdaySchedule
        self._dmLib.chip_ime_AppendCommand_DoorLock_GetWeekdaySchedule.argtypes = [c_uint8, c_uint16, c_uint8, c_uint16]
        self._dmLib.chip_ime_AppendCommand_DoorLock_GetWeekdaySchedule.restype = c_uint32
        # Cluster DoorLock Command GetYeardaySchedule
        self._dmLib.chip_ime_AppendCommand_DoorLock_GetYeardaySchedule.argtypes = [c_uint8, c_uint16, c_uint8, c_uint16]
        self._dmLib.chip_ime_AppendCommand_DoorLock_GetYeardaySchedule.restype = c_uint32
        # Cluster DoorLock Command LockDoor
        self._dmLib.chip_ime_AppendCommand_DoorLock_LockDoor.argtypes = [c_uint8, c_uint16, c_char_p]
        self._dmLib.chip_ime_AppendCommand_DoorLock_LockDoor.restype = c_uint32
        # Cluster DoorLock Command SetHolidaySchedule
        self._dmLib.chip_ime_AppendCommand_DoorLock_SetHolidaySchedule.argtypes = [c_uint8, c_uint16, c_uint8, c_uint32, c_uint32, c_uint8]
        self._dmLib.chip_ime_AppendCommand_DoorLock_SetHolidaySchedule.restype = c_uint32
        # Cluster DoorLock Command SetPin
        self._dmLib.chip_ime_AppendCommand_DoorLock_SetPin.argtypes = [c_uint8, c_uint16, c_uint16, c_uint8, c_uint8, c_char_p]
        self._dmLib.chip_ime_AppendCommand_DoorLock_SetPin.restype = c_uint32
        # Cluster DoorLock Command SetRfid
        self._dmLib.chip_ime_AppendCommand_DoorLock_SetRfid.argtypes = [c_uint8, c_uint16, c_uint16, c_uint8, c_uint8, c_char_p]
        self._dmLib.chip_ime_AppendCommand_DoorLock_SetRfid.restype = c_uint32
        # Cluster DoorLock Command SetUserType
        self._dmLib.chip_ime_AppendCommand_DoorLock_SetUserType.argtypes = [c_uint8, c_uint16, c_uint16, c_uint8]
        self._dmLib.chip_ime_AppendCommand_DoorLock_SetUserType.restype = c_uint32
        # Cluster DoorLock Command SetWeekdaySchedule
        self._dmLib.chip_ime_AppendCommand_DoorLock_SetWeekdaySchedule.argtypes = [c_uint8, c_uint16, c_uint8, c_uint16, c_uint8, c_uint8, c_uint8, c_uint8, c_uint8]
        self._dmLib.chip_ime_AppendCommand_DoorLock_SetWeekdaySchedule.restype = c_uint32
        # Cluster DoorLock Command SetYeardaySchedule
        self._dmLib.chip_ime_AppendCommand_DoorLock_SetYeardaySchedule.argtypes = [c_uint8, c_uint16, c_uint8, c_uint16, c_uint32, c_uint32]
        self._dmLib.chip_ime_AppendCommand_DoorLock_SetYeardaySchedule.restype = c_uint32
        # Cluster DoorLock Command UnlockDoor
        self._dmLib.chip_ime_AppendCommand_DoorLock_UnlockDoor.argtypes = [c_uint8, c_uint16, c_char_p]
        self._dmLib.chip_ime_AppendCommand_DoorLock_UnlockDoor.restype = c_uint32
        # Cluster DoorLock Command UnlockWithTimeout
        self._dmLib.chip_ime_AppendCommand_DoorLock_UnlockWithTimeout.argtypes = [c_uint8, c_uint16, c_uint16, c_char_p]
        self._dmLib.chip_ime_AppendCommand_DoorLock_UnlockWithTimeout.restype = c_uint32
        # Cluster Groups
        # Cluster Groups Command AddGroup
        self._dmLib.chip_ime_AppendCommand_Groups_AddGroup.argtypes = [c_uint8, c_uint16, c_uint16, c_char_p]
        self._dmLib.chip_ime_AppendCommand_Groups_AddGroup.restype = c_uint32
        # Cluster Groups Command AddGroupIfIdentifying
        self._dmLib.chip_ime_AppendCommand_Groups_AddGroupIfIdentifying.argtypes = [c_uint8, c_uint16, c_uint16, c_char_p]
        self._dmLib.chip_ime_AppendCommand_Groups_AddGroupIfIdentifying.restype = c_uint32
        # Cluster Groups Command GetGroupMembership
        self._dmLib.chip_ime_AppendCommand_Groups_GetGroupMembership.argtypes = [c_uint8, c_uint16, c_uint8, c_uint16]
        self._dmLib.chip_ime_AppendCommand_Groups_GetGroupMembership.restype = c_uint32
        # Cluster Groups Command RemoveAllGroups
        self._dmLib.chip_ime_AppendCommand_Groups_RemoveAllGroups.argtypes = [c_uint8, c_uint16]
        self._dmLib.chip_ime_AppendCommand_Groups_RemoveAllGroups.restype = c_uint32
        # Cluster Groups Command RemoveGroup
        self._dmLib.chip_ime_AppendCommand_Groups_RemoveGroup.argtypes = [c_uint8, c_uint16, c_uint16]
        self._dmLib.chip_ime_AppendCommand_Groups_RemoveGroup.restype = c_uint32
        # Cluster Groups Command ViewGroup
        self._dmLib.chip_ime_AppendCommand_Groups_ViewGroup.argtypes = [c_uint8, c_uint16, c_uint16]
        self._dmLib.chip_ime_AppendCommand_Groups_ViewGroup.restype = c_uint32
        # Cluster IasZone
        # Cluster Identify
        # Cluster Identify Command Identify
        self._dmLib.chip_ime_AppendCommand_Identify_Identify.argtypes = [c_uint8, c_uint16, c_uint16]
        self._dmLib.chip_ime_AppendCommand_Identify_Identify.restype = c_uint32
        # Cluster Identify Command IdentifyQuery
        self._dmLib.chip_ime_AppendCommand_Identify_IdentifyQuery.argtypes = [c_uint8, c_uint16]
        self._dmLib.chip_ime_AppendCommand_Identify_IdentifyQuery.restype = c_uint32
        # Cluster LevelControl
        # Cluster LevelControl Command Move
        self._dmLib.chip_ime_AppendCommand_LevelControl_Move.argtypes = [c_uint8, c_uint16, c_uint8, c_uint8, c_uint8, c_uint8]
        self._dmLib.chip_ime_AppendCommand_LevelControl_Move.restype = c_uint32
        # Cluster LevelControl Command MoveToLevel
        self._dmLib.chip_ime_AppendCommand_LevelControl_MoveToLevel.argtypes = [c_uint8, c_uint16, c_uint8, c_uint16, c_uint8, c_uint8]
        self._dmLib.chip_ime_AppendCommand_LevelControl_MoveToLevel.restype = c_uint32
        # Cluster LevelControl Command MoveToLevelWithOnOff
        self._dmLib.chip_ime_AppendCommand_LevelControl_MoveToLevelWithOnOff.argtypes = [c_uint8, c_uint16, c_uint8, c_uint16]
        self._dmLib.chip_ime_AppendCommand_LevelControl_MoveToLevelWithOnOff.restype = c_uint32
        # Cluster LevelControl Command MoveWithOnOff
        self._dmLib.chip_ime_AppendCommand_LevelControl_MoveWithOnOff.argtypes = [c_uint8, c_uint16, c_uint8, c_uint8]
        self._dmLib.chip_ime_AppendCommand_LevelControl_MoveWithOnOff.restype = c_uint32
        # Cluster LevelControl Command Step
        self._dmLib.chip_ime_AppendCommand_LevelControl_Step.argtypes = [c_uint8, c_uint16, c_uint8, c_uint8, c_uint16, c_uint8, c_uint8]
        self._dmLib.chip_ime_AppendCommand_LevelControl_Step.restype = c_uint32
        # Cluster LevelControl Command StepWithOnOff
        self._dmLib.chip_ime_AppendCommand_LevelControl_StepWithOnOff.argtypes = [c_uint8, c_uint16, c_uint8, c_uint8, c_uint16]
        self._dmLib.chip_ime_AppendCommand_LevelControl_StepWithOnOff.restype = c_uint32
        # Cluster LevelControl Command Stop
        self._dmLib.chip_ime_AppendCommand_LevelControl_Stop.argtypes = [c_uint8, c_uint16, c_uint8, c_uint8]
        self._dmLib.chip_ime_AppendCommand_LevelControl_Stop.restype = c_uint32
        # Cluster LevelControl Command StopWithOnOff
        self._dmLib.chip_ime_AppendCommand_LevelControl_StopWithOnOff.argtypes = [c_uint8, c_uint16]
        self._dmLib.chip_ime_AppendCommand_LevelControl_StopWithOnOff.restype = c_uint32
        # Cluster OnOff
        # Cluster OnOff Command Off
        self._dmLib.chip_ime_AppendCommand_OnOff_Off.argtypes = [c_uint8, c_uint16]
        self._dmLib.chip_ime_AppendCommand_OnOff_Off.restype = c_uint32
        # Cluster OnOff Command On
        self._dmLib.chip_ime_AppendCommand_OnOff_On.argtypes = [c_uint8, c_uint16]
        self._dmLib.chip_ime_AppendCommand_OnOff_On.restype = c_uint32
        # Cluster OnOff Command Toggle
        self._dmLib.chip_ime_AppendCommand_OnOff_Toggle.argtypes = [c_uint8, c_uint16]
        self._dmLib.chip_ime_AppendCommand_OnOff_Toggle.restype = c_uint32
        # Cluster Scenes
        # Cluster Scenes Command AddScene
        self._dmLib.chip_ime_AppendCommand_Scenes_AddScene.argtypes = [c_uint8, c_uint16, c_uint16, c_uint8, c_uint16, c_char_p, c_uint16, c_uint8, c_uint8]
        self._dmLib.chip_ime_AppendCommand_Scenes_AddScene.restype = c_uint32
        # Cluster Scenes Command GetSceneMembership
        self._dmLib.chip_ime_AppendCommand_Scenes_GetSceneMembership.argtypes = [c_uint8, c_uint16, c_uint16]
        self._dmLib.chip_ime_AppendCommand_Scenes_GetSceneMembership.restype = c_uint32
        # Cluster Scenes Command RecallScene
        self._dmLib.chip_ime_AppendCommand_Scenes_RecallScene.argtypes = [c_uint8, c_uint16, c_uint16, c_uint8, c_uint16]
        self._dmLib.chip_ime_AppendCommand_Scenes_RecallScene.restype = c_uint32
        # Cluster Scenes Command RemoveAllScenes
        self._dmLib.chip_ime_AppendCommand_Scenes_RemoveAllScenes.argtypes = [c_uint8, c_uint16, c_uint16]
        self._dmLib.chip_ime_AppendCommand_Scenes_RemoveAllScenes.restype = c_uint32
        # Cluster Scenes Command RemoveScene
        self._dmLib.chip_ime_AppendCommand_Scenes_RemoveScene.argtypes = [c_uint8, c_uint16, c_uint16, c_uint8]
        self._dmLib.chip_ime_AppendCommand_Scenes_RemoveScene.restype = c_uint32
        # Cluster Scenes Command StoreScene
        self._dmLib.chip_ime_AppendCommand_Scenes_StoreScene.argtypes = [c_uint8, c_uint16, c_uint16, c_uint8]
        self._dmLib.chip_ime_AppendCommand_Scenes_StoreScene.restype = c_uint32
        # Cluster Scenes Command ViewScene
        self._dmLib.chip_ime_AppendCommand_Scenes_ViewScene.argtypes = [c_uint8, c_uint16, c_uint16, c_uint8]
        self._dmLib.chip_ime_AppendCommand_Scenes_ViewScene.restype = c_uint32
        # Cluster TemperatureMeasurement
