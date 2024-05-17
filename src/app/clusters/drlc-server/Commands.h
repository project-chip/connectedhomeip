/*----------------------------------------------------------------------------*\
| Cluster DemandResponseLoadControl                                   | 0x0096 |
|------------------------------------------------------------------------------|
| Commands:                                                           |        |
| * RegisterLoadControlProgramRequest                                 |   0x00 |
| * UnregisterLoadControlProgramRequest                               |   0x01 |
| * AddLoadControlEventRequest                                        |   0x02 |
| * RemoveLoadControlEventRequest                                     |   0x03 |
| * ClearLoadControlEventsRequest                                     |   0x04 |
|------------------------------------------------------------------------------|
| Attributes:                                                         |        |
| * LoadControlPrograms                                               | 0x0000 |
| * NumberOfLoadControlPrograms                                       | 0x0001 |
| * Events                                                            | 0x0002 |
| * ActiveEvents                                                      | 0x0003 |
| * NumberOfEventsPerProgram                                          | 0x0004 |
| * NumberOfTransitions                                               | 0x0005 |
| * DefaultRandomStart                                                | 0x0006 |
| * DefaultRandomDuration                                             | 0x0007 |
| * GeneratedCommandList                                              | 0xFFF8 |
| * AcceptedCommandList                                               | 0xFFF9 |
| * EventList                                                         | 0xFFFA |
| * AttributeList                                                     | 0xFFFB |
| * FeatureMap                                                        | 0xFFFC |
| * ClusterRevision                                                   | 0xFFFD |
|------------------------------------------------------------------------------|
| Events:                                                             |        |
| * LoadControlEventStatusChange                                      | 0x0000 |
\*----------------------------------------------------------------------------*/

#if MTR_ENABLE_PROVISIONAL
/*
 * Command RegisterLoadControlProgramRequest
 */
class DemandResponseLoadControlRegisterLoadControlProgramRequest : public ClusterCommand {
public:
    DemandResponseLoadControlRegisterLoadControlProgramRequest()
        : ClusterCommand("register-load-control-program-request")
        , mComplex_LoadControlProgram(&mRequest.loadControlProgram)
    {
#if MTR_ENABLE_PROVISIONAL
        AddArgument("LoadControlProgram", &mComplex_LoadControlProgram);
#endif // MTR_ENABLE_PROVISIONAL
        ClusterCommand::AddArguments();
    }

    CHIP_ERROR SendCommand(MTRBaseDevice * device, chip::EndpointId endpointId) override
    {
        constexpr chip::ClusterId clusterId = chip::app::Clusters::DemandResponseLoadControl::Id;
        constexpr chip::CommandId commandId = chip::app::Clusters::DemandResponseLoadControl::Commands::RegisterLoadControlProgramRequest::Id;

        ChipLogProgress(chipTool, "Sending cluster (0x%08" PRIX32 ") command (0x%08" PRIX32 ") on endpoint %u", clusterId, commandId, endpointId);

        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);
        __auto_type * cluster = [[MTRBaseClusterDemandResponseLoadControl alloc] initWithDevice:device endpointID:@(endpointId) queue:callbackQueue];
        __auto_type * params = [[MTRDemandResponseLoadControlClusterRegisterLoadControlProgramRequestParams alloc] init];
        params.timedInvokeTimeoutMs = mTimedInteractionTimeoutMs.HasValue() ? [NSNumber numberWithUnsignedShort:mTimedInteractionTimeoutMs.Value()] : nil;
#if MTR_ENABLE_PROVISIONAL
        params.loadControlProgram = [MTRDemandResponseLoadControlClusterLoadControlProgramStruct new];
        params.loadControlProgram.programID = [NSData dataWithBytes:mRequest.loadControlProgram.programID.data() length:mRequest.loadControlProgram.programID.size()];
        params.loadControlProgram.name = [[NSString alloc] initWithBytes:mRequest.loadControlProgram.name.data() length:mRequest.loadControlProgram.name.size() encoding:NSUTF8StringEncoding];
        if (mRequest.loadControlProgram.enrollmentGroup.IsNull()) {
            params.loadControlProgram.enrollmentGroup = nil;
        } else {
            params.loadControlProgram.enrollmentGroup = [NSNumber numberWithUnsignedChar:mRequest.loadControlProgram.enrollmentGroup.Value()];
        }
        if (mRequest.loadControlProgram.randomStartMinutes.IsNull()) {
            params.loadControlProgram.randomStartMinutes = nil;
        } else {
            params.loadControlProgram.randomStartMinutes = [NSNumber numberWithUnsignedChar:mRequest.loadControlProgram.randomStartMinutes.Value()];
        }
        if (mRequest.loadControlProgram.randomDurationMinutes.IsNull()) {
            params.loadControlProgram.randomDurationMinutes = nil;
        } else {
            params.loadControlProgram.randomDurationMinutes = [NSNumber numberWithUnsignedChar:mRequest.loadControlProgram.randomDurationMinutes.Value()];
        }
#endif // MTR_ENABLE_PROVISIONAL
        uint16_t repeatCount = mRepeatCount.ValueOr(1);
        uint16_t __block responsesNeeded = repeatCount;
        while (repeatCount--) {
            [cluster registerLoadControlProgramRequestWithParams:params completion:
                                                                            ^(NSError * _Nullable error) {
                                                                                responsesNeeded--;
                                                                                if (error != nil) {
                                                                                    mError = error;
                                                                                    LogNSError("Error", error);
                                                                                    RemoteDataModelLogger::LogCommandErrorAsJSON(@(endpointId), @(clusterId), @(commandId), error);
                                                                                }
                                                                                if (responsesNeeded == 0) {
                                                                                    SetCommandExitStatus(mError);
                                                                                }
                                                                            }];
        }
        return CHIP_NO_ERROR;
    }

private:
    chip::app::Clusters::DemandResponseLoadControl::Commands::RegisterLoadControlProgramRequest::Type mRequest;
    TypedComplexArgument<chip::app::Clusters::DemandResponseLoadControl::Structs::LoadControlProgramStruct::Type> mComplex_LoadControlProgram;
};

#endif // MTR_ENABLE_PROVISIONAL
#if MTR_ENABLE_PROVISIONAL
/*
 * Command UnregisterLoadControlProgramRequest
 */
class DemandResponseLoadControlUnregisterLoadControlProgramRequest : public ClusterCommand {
public:
    DemandResponseLoadControlUnregisterLoadControlProgramRequest()
        : ClusterCommand("unregister-load-control-program-request")
    {
#if MTR_ENABLE_PROVISIONAL
        AddArgument("LoadControlProgramID", &mRequest.loadControlProgramID);
#endif // MTR_ENABLE_PROVISIONAL
        ClusterCommand::AddArguments();
    }

    CHIP_ERROR SendCommand(MTRBaseDevice * device, chip::EndpointId endpointId) override
    {
        constexpr chip::ClusterId clusterId = chip::app::Clusters::DemandResponseLoadControl::Id;
        constexpr chip::CommandId commandId = chip::app::Clusters::DemandResponseLoadControl::Commands::UnregisterLoadControlProgramRequest::Id;

        ChipLogProgress(chipTool, "Sending cluster (0x%08" PRIX32 ") command (0x%08" PRIX32 ") on endpoint %u", clusterId, commandId, endpointId);

        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);
        __auto_type * cluster = [[MTRBaseClusterDemandResponseLoadControl alloc] initWithDevice:device endpointID:@(endpointId) queue:callbackQueue];
        __auto_type * params = [[MTRDemandResponseLoadControlClusterUnregisterLoadControlProgramRequestParams alloc] init];
        params.timedInvokeTimeoutMs = mTimedInteractionTimeoutMs.HasValue() ? [NSNumber numberWithUnsignedShort:mTimedInteractionTimeoutMs.Value()] : nil;
#if MTR_ENABLE_PROVISIONAL
        params.loadControlProgramID = [NSData dataWithBytes:mRequest.loadControlProgramID.data() length:mRequest.loadControlProgramID.size()];
#endif // MTR_ENABLE_PROVISIONAL
        uint16_t repeatCount = mRepeatCount.ValueOr(1);
        uint16_t __block responsesNeeded = repeatCount;
        while (repeatCount--) {
            [cluster unregisterLoadControlProgramRequestWithParams:params completion:
                                                                              ^(NSError * _Nullable error) {
                                                                                  responsesNeeded--;
                                                                                  if (error != nil) {
                                                                                      mError = error;
                                                                                      LogNSError("Error", error);
                                                                                      RemoteDataModelLogger::LogCommandErrorAsJSON(@(endpointId), @(clusterId), @(commandId), error);
                                                                                  }
                                                                                  if (responsesNeeded == 0) {
                                                                                      SetCommandExitStatus(mError);
                                                                                  }
                                                                              }];
        }
        return CHIP_NO_ERROR;
    }

private:
    chip::app::Clusters::DemandResponseLoadControl::Commands::UnregisterLoadControlProgramRequest::Type mRequest;
};

#endif // MTR_ENABLE_PROVISIONAL
#if MTR_ENABLE_PROVISIONAL
/*
 * Command AddLoadControlEventRequest
 */
class DemandResponseLoadControlAddLoadControlEventRequest : public ClusterCommand {
public:
    DemandResponseLoadControlAddLoadControlEventRequest()
        : ClusterCommand("add-load-control-event-request")
        , mComplex_Event(&mRequest.event)
    {
#if MTR_ENABLE_PROVISIONAL
        AddArgument("Event", &mComplex_Event);
#endif // MTR_ENABLE_PROVISIONAL
        ClusterCommand::AddArguments();
    }

    CHIP_ERROR SendCommand(MTRBaseDevice * device, chip::EndpointId endpointId) override
    {
        constexpr chip::ClusterId clusterId = chip::app::Clusters::DemandResponseLoadControl::Id;
        constexpr chip::CommandId commandId = chip::app::Clusters::DemandResponseLoadControl::Commands::AddLoadControlEventRequest::Id;

        ChipLogProgress(chipTool, "Sending cluster (0x%08" PRIX32 ") command (0x%08" PRIX32 ") on endpoint %u", clusterId, commandId, endpointId);

        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);
        __auto_type * cluster = [[MTRBaseClusterDemandResponseLoadControl alloc] initWithDevice:device endpointID:@(endpointId) queue:callbackQueue];
        __auto_type * params = [[MTRDemandResponseLoadControlClusterAddLoadControlEventRequestParams alloc] init];
        params.timedInvokeTimeoutMs = mTimedInteractionTimeoutMs.HasValue() ? [NSNumber numberWithUnsignedShort:mTimedInteractionTimeoutMs.Value()] : nil;
#if MTR_ENABLE_PROVISIONAL
        params.event = [MTRDemandResponseLoadControlClusterLoadControlEventStruct new];
        params.event.eventID = [NSData dataWithBytes:mRequest.event.eventID.data() length:mRequest.event.eventID.size()];
        if (mRequest.event.programID.IsNull()) {
            params.event.programID = nil;
        } else {
            params.event.programID = [NSData dataWithBytes:mRequest.event.programID.Value().data() length:mRequest.event.programID.Value().size()];
        }
        params.event.control = [NSNumber numberWithUnsignedShort:mRequest.event.control.Raw()];
        params.event.deviceClass = [NSNumber numberWithUnsignedInt:mRequest.event.deviceClass.Raw()];
        if (mRequest.event.enrollmentGroup.HasValue()) {
            params.event.enrollmentGroup = [NSNumber numberWithUnsignedChar:mRequest.event.enrollmentGroup.Value()];
        } else {
            params.event.enrollmentGroup = nil;
        }
        params.event.criticality = [NSNumber numberWithUnsignedChar:chip::to_underlying(mRequest.event.criticality)];
        if (mRequest.event.startTime.IsNull()) {
            params.event.startTime = nil;
        } else {
            params.event.startTime = [NSNumber numberWithUnsignedInt:mRequest.event.startTime.Value()];
        }
        { // Scope for our temporary variables
            auto * array_1 = [NSMutableArray new];
            for (auto & entry_1 : mRequest.event.transitions) {
                MTRDemandResponseLoadControlClusterLoadControlEventTransitionStruct * newElement_1;
                newElement_1 = [MTRDemandResponseLoadControlClusterLoadControlEventTransitionStruct new];
                newElement_1.duration = [NSNumber numberWithUnsignedShort:entry_1.duration];
                newElement_1.control = [NSNumber numberWithUnsignedShort:entry_1.control.Raw()];
                if (entry_1.temperatureControl.HasValue()) {
                    newElement_1.temperatureControl = [MTRDemandResponseLoadControlClusterTemperatureControlStruct new];
                    if (entry_1.temperatureControl.Value().coolingTempOffset.HasValue()) {
                        if (entry_1.temperatureControl.Value().coolingTempOffset.Value().IsNull()) {
                            newElement_1.temperatureControl.coolingTempOffset = nil;
                        } else {
                            newElement_1.temperatureControl.coolingTempOffset = [NSNumber numberWithUnsignedShort:entry_1.temperatureControl.Value().coolingTempOffset.Value().Value()];
                        }
                    } else {
                        newElement_1.temperatureControl.coolingTempOffset = nil;
                    }
                    if (entry_1.temperatureControl.Value().heatingtTempOffset.HasValue()) {
                        if (entry_1.temperatureControl.Value().heatingtTempOffset.Value().IsNull()) {
                            newElement_1.temperatureControl.heatingtTempOffset = nil;
                        } else {
                            newElement_1.temperatureControl.heatingtTempOffset = [NSNumber numberWithUnsignedShort:entry_1.temperatureControl.Value().heatingtTempOffset.Value().Value()];
                        }
                    } else {
                        newElement_1.temperatureControl.heatingtTempOffset = nil;
                    }
                    if (entry_1.temperatureControl.Value().coolingTempSetpoint.HasValue()) {
                        if (entry_1.temperatureControl.Value().coolingTempSetpoint.Value().IsNull()) {
                            newElement_1.temperatureControl.coolingTempSetpoint = nil;
                        } else {
                            newElement_1.temperatureControl.coolingTempSetpoint = [NSNumber numberWithShort:entry_1.temperatureControl.Value().coolingTempSetpoint.Value().Value()];
                        }
                    } else {
                        newElement_1.temperatureControl.coolingTempSetpoint = nil;
                    }
                    if (entry_1.temperatureControl.Value().heatingTempSetpoint.HasValue()) {
                        if (entry_1.temperatureControl.Value().heatingTempSetpoint.Value().IsNull()) {
                            newElement_1.temperatureControl.heatingTempSetpoint = nil;
                        } else {
                            newElement_1.temperatureControl.heatingTempSetpoint = [NSNumber numberWithShort:entry_1.temperatureControl.Value().heatingTempSetpoint.Value().Value()];
                        }
                    } else {
                        newElement_1.temperatureControl.heatingTempSetpoint = nil;
                    }
                } else {
                    newElement_1.temperatureControl = nil;
                }
                if (entry_1.averageLoadControl.HasValue()) {
                    newElement_1.averageLoadControl = [MTRDemandResponseLoadControlClusterAverageLoadControlStruct new];
                    newElement_1.averageLoadControl.loadAdjustment = [NSNumber numberWithChar:entry_1.averageLoadControl.Value().loadAdjustment];
                } else {
                    newElement_1.averageLoadControl = nil;
                }
                if (entry_1.dutyCycleControl.HasValue()) {
                    newElement_1.dutyCycleControl = [MTRDemandResponseLoadControlClusterDutyCycleControlStruct new];
                    newElement_1.dutyCycleControl.dutyCycle = [NSNumber numberWithUnsignedChar:entry_1.dutyCycleControl.Value().dutyCycle];
                } else {
                    newElement_1.dutyCycleControl = nil;
                }
                if (entry_1.powerSavingsControl.HasValue()) {
                    newElement_1.powerSavingsControl = [MTRDemandResponseLoadControlClusterPowerSavingsControlStruct new];
                    newElement_1.powerSavingsControl.powerSavings = [NSNumber numberWithUnsignedChar:entry_1.powerSavingsControl.Value().powerSavings];
                } else {
                    newElement_1.powerSavingsControl = nil;
                }
                if (entry_1.heatingSourceControl.HasValue()) {
                    newElement_1.heatingSourceControl = [MTRDemandResponseLoadControlClusterHeatingSourceControlStruct new];
                    newElement_1.heatingSourceControl.heatingSource = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_1.heatingSourceControl.Value().heatingSource)];
                } else {
                    newElement_1.heatingSourceControl = nil;
                }
                [array_1 addObject:newElement_1];
            }
            params.event.transitions = array_1;
        }
#endif // MTR_ENABLE_PROVISIONAL
        uint16_t repeatCount = mRepeatCount.ValueOr(1);
        uint16_t __block responsesNeeded = repeatCount;
        while (repeatCount--) {
            [cluster addLoadControlEventRequestWithParams:params completion:
                                                                     ^(NSError * _Nullable error) {
                                                                         responsesNeeded--;
                                                                         if (error != nil) {
                                                                             mError = error;
                                                                             LogNSError("Error", error);
                                                                             RemoteDataModelLogger::LogCommandErrorAsJSON(@(endpointId), @(clusterId), @(commandId), error);
                                                                         }
                                                                         if (responsesNeeded == 0) {
                                                                             SetCommandExitStatus(mError);
                                                                         }
                                                                     }];
        }
        return CHIP_NO_ERROR;
    }

private:
    chip::app::Clusters::DemandResponseLoadControl::Commands::AddLoadControlEventRequest::Type mRequest;
    TypedComplexArgument<chip::app::Clusters::DemandResponseLoadControl::Structs::LoadControlEventStruct::Type> mComplex_Event;
};

#endif // MTR_ENABLE_PROVISIONAL
#if MTR_ENABLE_PROVISIONAL
/*
 * Command RemoveLoadControlEventRequest
 */
class DemandResponseLoadControlRemoveLoadControlEventRequest : public ClusterCommand {
public:
    DemandResponseLoadControlRemoveLoadControlEventRequest()
        : ClusterCommand("remove-load-control-event-request")
    {
#if MTR_ENABLE_PROVISIONAL
        AddArgument("EventID", &mRequest.eventID);
#endif // MTR_ENABLE_PROVISIONAL
#if MTR_ENABLE_PROVISIONAL
        AddArgument("CancelControl", 0, UINT16_MAX, &mRequest.cancelControl);
#endif // MTR_ENABLE_PROVISIONAL
        ClusterCommand::AddArguments();
    }

    CHIP_ERROR SendCommand(MTRBaseDevice * device, chip::EndpointId endpointId) override
    {
        constexpr chip::ClusterId clusterId = chip::app::Clusters::DemandResponseLoadControl::Id;
        constexpr chip::CommandId commandId = chip::app::Clusters::DemandResponseLoadControl::Commands::RemoveLoadControlEventRequest::Id;

        ChipLogProgress(chipTool, "Sending cluster (0x%08" PRIX32 ") command (0x%08" PRIX32 ") on endpoint %u", clusterId, commandId, endpointId);

        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);
        __auto_type * cluster = [[MTRBaseClusterDemandResponseLoadControl alloc] initWithDevice:device endpointID:@(endpointId) queue:callbackQueue];
        __auto_type * params = [[MTRDemandResponseLoadControlClusterRemoveLoadControlEventRequestParams alloc] init];
        params.timedInvokeTimeoutMs = mTimedInteractionTimeoutMs.HasValue() ? [NSNumber numberWithUnsignedShort:mTimedInteractionTimeoutMs.Value()] : nil;
#if MTR_ENABLE_PROVISIONAL
        params.eventID = [NSData dataWithBytes:mRequest.eventID.data() length:mRequest.eventID.size()];
#endif // MTR_ENABLE_PROVISIONAL
#if MTR_ENABLE_PROVISIONAL
        params.cancelControl = [NSNumber numberWithUnsignedShort:mRequest.cancelControl.Raw()];
#endif // MTR_ENABLE_PROVISIONAL
        uint16_t repeatCount = mRepeatCount.ValueOr(1);
        uint16_t __block responsesNeeded = repeatCount;
        while (repeatCount--) {
            [cluster removeLoadControlEventRequestWithParams:params completion:
                                                                        ^(NSError * _Nullable error) {
                                                                            responsesNeeded--;
                                                                            if (error != nil) {
                                                                                mError = error;
                                                                                LogNSError("Error", error);
                                                                                RemoteDataModelLogger::LogCommandErrorAsJSON(@(endpointId), @(clusterId), @(commandId), error);
                                                                            }
                                                                            if (responsesNeeded == 0) {
                                                                                SetCommandExitStatus(mError);
                                                                            }
                                                                        }];
        }
        return CHIP_NO_ERROR;
    }

private:
    chip::app::Clusters::DemandResponseLoadControl::Commands::RemoveLoadControlEventRequest::Type mRequest;
};

#endif // MTR_ENABLE_PROVISIONAL
#if MTR_ENABLE_PROVISIONAL
/*
 * Command ClearLoadControlEventsRequest
 */
class DemandResponseLoadControlClearLoadControlEventsRequest : public ClusterCommand {
public:
    DemandResponseLoadControlClearLoadControlEventsRequest()
        : ClusterCommand("clear-load-control-events-request")
    {
        ClusterCommand::AddArguments();
    }

    CHIP_ERROR SendCommand(MTRBaseDevice * device, chip::EndpointId endpointId) override
    {
        constexpr chip::ClusterId clusterId = chip::app::Clusters::DemandResponseLoadControl::Id;
        constexpr chip::CommandId commandId = chip::app::Clusters::DemandResponseLoadControl::Commands::ClearLoadControlEventsRequest::Id;

        ChipLogProgress(chipTool, "Sending cluster (0x%08" PRIX32 ") command (0x%08" PRIX32 ") on endpoint %u", clusterId, commandId, endpointId);

        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);
        __auto_type * cluster = [[MTRBaseClusterDemandResponseLoadControl alloc] initWithDevice:device endpointID:@(endpointId) queue:callbackQueue];
        __auto_type * params = [[MTRDemandResponseLoadControlClusterClearLoadControlEventsRequestParams alloc] init];
        params.timedInvokeTimeoutMs = mTimedInteractionTimeoutMs.HasValue() ? [NSNumber numberWithUnsignedShort:mTimedInteractionTimeoutMs.Value()] : nil;
        uint16_t repeatCount = mRepeatCount.ValueOr(1);
        uint16_t __block responsesNeeded = repeatCount;
        while (repeatCount--) {
            [cluster clearLoadControlEventsRequestWithParams:params completion:
                                                                        ^(NSError * _Nullable error) {
                                                                            responsesNeeded--;
                                                                            if (error != nil) {
                                                                                mError = error;
                                                                                LogNSError("Error", error);
                                                                                RemoteDataModelLogger::LogCommandErrorAsJSON(@(endpointId), @(clusterId), @(commandId), error);
                                                                            }
                                                                            if (responsesNeeded == 0) {
                                                                                SetCommandExitStatus(mError);
                                                                            }
                                                                        }];
        }
        return CHIP_NO_ERROR;
    }

private:
};

#endif // MTR_ENABLE_PROVISIONAL

#if MTR_ENABLE_PROVISIONAL

/*
 * Attribute LoadControlPrograms
 */
class ReadDemandResponseLoadControlLoadControlPrograms : public ReadAttribute {
public:
    ReadDemandResponseLoadControlLoadControlPrograms()
        : ReadAttribute("load-control-programs")
    {
    }

    ~ReadDemandResponseLoadControlLoadControlPrograms()
    {
    }

    CHIP_ERROR SendCommand(MTRBaseDevice * device, chip::EndpointId endpointId) override
    {
        constexpr chip::ClusterId clusterId = chip::app::Clusters::DemandResponseLoadControl::Id;
        constexpr chip::AttributeId attributeId = chip::app::Clusters::DemandResponseLoadControl::Attributes::LoadControlPrograms::Id;

        ChipLogProgress(chipTool, "Sending cluster (0x%08" PRIX32 ") ReadAttribute (0x%08" PRIX32 ") on endpoint %u", endpointId, clusterId, attributeId);

        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);
        __auto_type * cluster = [[MTRBaseClusterDemandResponseLoadControl alloc] initWithDevice:device endpointID:@(endpointId) queue:callbackQueue];
        [cluster readAttributeLoadControlProgramsWithCompletion:^(NSArray * _Nullable value, NSError * _Nullable error) {
            NSLog(@"DemandResponseLoadControl.LoadControlPrograms response %@", [value description]);
            if (error == nil) {
                RemoteDataModelLogger::LogAttributeAsJSON(@(endpointId), @(clusterId), @(attributeId), value);
            } else {
                LogNSError("DemandResponseLoadControl LoadControlPrograms read Error", error);
                RemoteDataModelLogger::LogAttributeErrorAsJSON(@(endpointId), @(clusterId), @(attributeId), error);
            }
            SetCommandExitStatus(error);
        }];
        return CHIP_NO_ERROR;
    }
};

class SubscribeAttributeDemandResponseLoadControlLoadControlPrograms : public SubscribeAttribute {
public:
    SubscribeAttributeDemandResponseLoadControlLoadControlPrograms()
        : SubscribeAttribute("load-control-programs")
    {
    }

    ~SubscribeAttributeDemandResponseLoadControlLoadControlPrograms()
    {
    }

    CHIP_ERROR SendCommand(MTRBaseDevice * device, chip::EndpointId endpointId) override
    {
        constexpr chip::ClusterId clusterId = chip::app::Clusters::DemandResponseLoadControl::Id;
        constexpr chip::CommandId attributeId = chip::app::Clusters::DemandResponseLoadControl::Attributes::LoadControlPrograms::Id;

        ChipLogProgress(chipTool, "Sending cluster (0x%08" PRIX32 ") ReportAttribute (0x%08" PRIX32 ") on endpoint %u", clusterId, attributeId, endpointId);
        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);
        __auto_type * cluster = [[MTRBaseClusterDemandResponseLoadControl alloc] initWithDevice:device endpointID:@(endpointId) queue:callbackQueue];
        __auto_type * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(mMinInterval) maxInterval:@(mMaxInterval)];
        if (mKeepSubscriptions.HasValue()) {
            params.replaceExistingSubscriptions = !mKeepSubscriptions.Value();
        }
        if (mFabricFiltered.HasValue()) {
            params.filterByFabric = mFabricFiltered.Value();
        }
        if (mAutoResubscribe.HasValue()) {
            params.resubscribeAutomatically = mAutoResubscribe.Value();
        }
        [cluster subscribeAttributeLoadControlProgramsWithParams:params
            subscriptionEstablished:^() { mSubscriptionEstablished = YES; }
            reportHandler:^(NSArray * _Nullable value, NSError * _Nullable error) {
                NSLog(@"DemandResponseLoadControl.LoadControlPrograms response %@", [value description]);
                if (error == nil) {
                    RemoteDataModelLogger::LogAttributeAsJSON(@(endpointId), @(clusterId), @(attributeId), value);
                } else {
                    RemoteDataModelLogger::LogAttributeErrorAsJSON(@(endpointId), @(clusterId), @(attributeId), error);
                }
                SetCommandExitStatus(error);
            }];

        return CHIP_NO_ERROR;
    }
};

#endif // MTR_ENABLE_PROVISIONAL
#if MTR_ENABLE_PROVISIONAL

/*
 * Attribute NumberOfLoadControlPrograms
 */
class ReadDemandResponseLoadControlNumberOfLoadControlPrograms : public ReadAttribute {
public:
    ReadDemandResponseLoadControlNumberOfLoadControlPrograms()
        : ReadAttribute("number-of-load-control-programs")
    {
    }

    ~ReadDemandResponseLoadControlNumberOfLoadControlPrograms()
    {
    }

    CHIP_ERROR SendCommand(MTRBaseDevice * device, chip::EndpointId endpointId) override
    {
        constexpr chip::ClusterId clusterId = chip::app::Clusters::DemandResponseLoadControl::Id;
        constexpr chip::AttributeId attributeId = chip::app::Clusters::DemandResponseLoadControl::Attributes::NumberOfLoadControlPrograms::Id;

        ChipLogProgress(chipTool, "Sending cluster (0x%08" PRIX32 ") ReadAttribute (0x%08" PRIX32 ") on endpoint %u", endpointId, clusterId, attributeId);

        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);
        __auto_type * cluster = [[MTRBaseClusterDemandResponseLoadControl alloc] initWithDevice:device endpointID:@(endpointId) queue:callbackQueue];
        [cluster readAttributeNumberOfLoadControlProgramsWithCompletion:^(NSNumber * _Nullable value, NSError * _Nullable error) {
            NSLog(@"DemandResponseLoadControl.NumberOfLoadControlPrograms response %@", [value description]);
            if (error == nil) {
                RemoteDataModelLogger::LogAttributeAsJSON(@(endpointId), @(clusterId), @(attributeId), value);
            } else {
                LogNSError("DemandResponseLoadControl NumberOfLoadControlPrograms read Error", error);
                RemoteDataModelLogger::LogAttributeErrorAsJSON(@(endpointId), @(clusterId), @(attributeId), error);
            }
            SetCommandExitStatus(error);
        }];
        return CHIP_NO_ERROR;
    }
};

class SubscribeAttributeDemandResponseLoadControlNumberOfLoadControlPrograms : public SubscribeAttribute {
public:
    SubscribeAttributeDemandResponseLoadControlNumberOfLoadControlPrograms()
        : SubscribeAttribute("number-of-load-control-programs")
    {
    }

    ~SubscribeAttributeDemandResponseLoadControlNumberOfLoadControlPrograms()
    {
    }

    CHIP_ERROR SendCommand(MTRBaseDevice * device, chip::EndpointId endpointId) override
    {
        constexpr chip::ClusterId clusterId = chip::app::Clusters::DemandResponseLoadControl::Id;
        constexpr chip::CommandId attributeId = chip::app::Clusters::DemandResponseLoadControl::Attributes::NumberOfLoadControlPrograms::Id;

        ChipLogProgress(chipTool, "Sending cluster (0x%08" PRIX32 ") ReportAttribute (0x%08" PRIX32 ") on endpoint %u", clusterId, attributeId, endpointId);
        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);
        __auto_type * cluster = [[MTRBaseClusterDemandResponseLoadControl alloc] initWithDevice:device endpointID:@(endpointId) queue:callbackQueue];
        __auto_type * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(mMinInterval) maxInterval:@(mMaxInterval)];
        if (mKeepSubscriptions.HasValue()) {
            params.replaceExistingSubscriptions = !mKeepSubscriptions.Value();
        }
        if (mFabricFiltered.HasValue()) {
            params.filterByFabric = mFabricFiltered.Value();
        }
        if (mAutoResubscribe.HasValue()) {
            params.resubscribeAutomatically = mAutoResubscribe.Value();
        }
        [cluster subscribeAttributeNumberOfLoadControlProgramsWithParams:params
            subscriptionEstablished:^() { mSubscriptionEstablished = YES; }
            reportHandler:^(NSNumber * _Nullable value, NSError * _Nullable error) {
                NSLog(@"DemandResponseLoadControl.NumberOfLoadControlPrograms response %@", [value description]);
                if (error == nil) {
                    RemoteDataModelLogger::LogAttributeAsJSON(@(endpointId), @(clusterId), @(attributeId), value);
                } else {
                    RemoteDataModelLogger::LogAttributeErrorAsJSON(@(endpointId), @(clusterId), @(attributeId), error);
                }
                SetCommandExitStatus(error);
            }];

        return CHIP_NO_ERROR;
    }
};

#endif // MTR_ENABLE_PROVISIONAL
#if MTR_ENABLE_PROVISIONAL

/*
 * Attribute Events
 */
class ReadDemandResponseLoadControlEvents : public ReadAttribute {
public:
    ReadDemandResponseLoadControlEvents()
        : ReadAttribute("events")
    {
    }

    ~ReadDemandResponseLoadControlEvents()
    {
    }

    CHIP_ERROR SendCommand(MTRBaseDevice * device, chip::EndpointId endpointId) override
    {
        constexpr chip::ClusterId clusterId = chip::app::Clusters::DemandResponseLoadControl::Id;
        constexpr chip::AttributeId attributeId = chip::app::Clusters::DemandResponseLoadControl::Attributes::Events::Id;

        ChipLogProgress(chipTool, "Sending cluster (0x%08" PRIX32 ") ReadAttribute (0x%08" PRIX32 ") on endpoint %u", endpointId, clusterId, attributeId);

        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);
        __auto_type * cluster = [[MTRBaseClusterDemandResponseLoadControl alloc] initWithDevice:device endpointID:@(endpointId) queue:callbackQueue];
        [cluster readAttributeEventsWithCompletion:^(NSArray * _Nullable value, NSError * _Nullable error) {
            NSLog(@"DemandResponseLoadControl.Events response %@", [value description]);
            if (error == nil) {
                RemoteDataModelLogger::LogAttributeAsJSON(@(endpointId), @(clusterId), @(attributeId), value);
            } else {
                LogNSError("DemandResponseLoadControl Events read Error", error);
                RemoteDataModelLogger::LogAttributeErrorAsJSON(@(endpointId), @(clusterId), @(attributeId), error);
            }
            SetCommandExitStatus(error);
        }];
        return CHIP_NO_ERROR;
    }
};

class SubscribeAttributeDemandResponseLoadControlEvents : public SubscribeAttribute {
public:
    SubscribeAttributeDemandResponseLoadControlEvents()
        : SubscribeAttribute("events")
    {
    }

    ~SubscribeAttributeDemandResponseLoadControlEvents()
    {
    }

    CHIP_ERROR SendCommand(MTRBaseDevice * device, chip::EndpointId endpointId) override
    {
        constexpr chip::ClusterId clusterId = chip::app::Clusters::DemandResponseLoadControl::Id;
        constexpr chip::CommandId attributeId = chip::app::Clusters::DemandResponseLoadControl::Attributes::Events::Id;

        ChipLogProgress(chipTool, "Sending cluster (0x%08" PRIX32 ") ReportAttribute (0x%08" PRIX32 ") on endpoint %u", clusterId, attributeId, endpointId);
        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);
        __auto_type * cluster = [[MTRBaseClusterDemandResponseLoadControl alloc] initWithDevice:device endpointID:@(endpointId) queue:callbackQueue];
        __auto_type * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(mMinInterval) maxInterval:@(mMaxInterval)];
        if (mKeepSubscriptions.HasValue()) {
            params.replaceExistingSubscriptions = !mKeepSubscriptions.Value();
        }
        if (mFabricFiltered.HasValue()) {
            params.filterByFabric = mFabricFiltered.Value();
        }
        if (mAutoResubscribe.HasValue()) {
            params.resubscribeAutomatically = mAutoResubscribe.Value();
        }
        [cluster subscribeAttributeEventsWithParams:params
            subscriptionEstablished:^() { mSubscriptionEstablished = YES; }
            reportHandler:^(NSArray * _Nullable value, NSError * _Nullable error) {
                NSLog(@"DemandResponseLoadControl.Events response %@", [value description]);
                if (error == nil) {
                    RemoteDataModelLogger::LogAttributeAsJSON(@(endpointId), @(clusterId), @(attributeId), value);
                } else {
                    RemoteDataModelLogger::LogAttributeErrorAsJSON(@(endpointId), @(clusterId), @(attributeId), error);
                }
                SetCommandExitStatus(error);
            }];

        return CHIP_NO_ERROR;
    }
};

#endif // MTR_ENABLE_PROVISIONAL
#if MTR_ENABLE_PROVISIONAL

/*
 * Attribute ActiveEvents
 */
class ReadDemandResponseLoadControlActiveEvents : public ReadAttribute {
public:
    ReadDemandResponseLoadControlActiveEvents()
        : ReadAttribute("active-events")
    {
    }

    ~ReadDemandResponseLoadControlActiveEvents()
    {
    }

    CHIP_ERROR SendCommand(MTRBaseDevice * device, chip::EndpointId endpointId) override
    {
        constexpr chip::ClusterId clusterId = chip::app::Clusters::DemandResponseLoadControl::Id;
        constexpr chip::AttributeId attributeId = chip::app::Clusters::DemandResponseLoadControl::Attributes::ActiveEvents::Id;

        ChipLogProgress(chipTool, "Sending cluster (0x%08" PRIX32 ") ReadAttribute (0x%08" PRIX32 ") on endpoint %u", endpointId, clusterId, attributeId);

        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);
        __auto_type * cluster = [[MTRBaseClusterDemandResponseLoadControl alloc] initWithDevice:device endpointID:@(endpointId) queue:callbackQueue];
        [cluster readAttributeActiveEventsWithCompletion:^(NSArray * _Nullable value, NSError * _Nullable error) {
            NSLog(@"DemandResponseLoadControl.ActiveEvents response %@", [value description]);
            if (error == nil) {
                RemoteDataModelLogger::LogAttributeAsJSON(@(endpointId), @(clusterId), @(attributeId), value);
            } else {
                LogNSError("DemandResponseLoadControl ActiveEvents read Error", error);
                RemoteDataModelLogger::LogAttributeErrorAsJSON(@(endpointId), @(clusterId), @(attributeId), error);
            }
            SetCommandExitStatus(error);
        }];
        return CHIP_NO_ERROR;
    }
};

class SubscribeAttributeDemandResponseLoadControlActiveEvents : public SubscribeAttribute {
public:
    SubscribeAttributeDemandResponseLoadControlActiveEvents()
        : SubscribeAttribute("active-events")
    {
    }

    ~SubscribeAttributeDemandResponseLoadControlActiveEvents()
    {
    }

    CHIP_ERROR SendCommand(MTRBaseDevice * device, chip::EndpointId endpointId) override
    {
        constexpr chip::ClusterId clusterId = chip::app::Clusters::DemandResponseLoadControl::Id;
        constexpr chip::CommandId attributeId = chip::app::Clusters::DemandResponseLoadControl::Attributes::ActiveEvents::Id;

        ChipLogProgress(chipTool, "Sending cluster (0x%08" PRIX32 ") ReportAttribute (0x%08" PRIX32 ") on endpoint %u", clusterId, attributeId, endpointId);
        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);
        __auto_type * cluster = [[MTRBaseClusterDemandResponseLoadControl alloc] initWithDevice:device endpointID:@(endpointId) queue:callbackQueue];
        __auto_type * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(mMinInterval) maxInterval:@(mMaxInterval)];
        if (mKeepSubscriptions.HasValue()) {
            params.replaceExistingSubscriptions = !mKeepSubscriptions.Value();
        }
        if (mFabricFiltered.HasValue()) {
            params.filterByFabric = mFabricFiltered.Value();
        }
        if (mAutoResubscribe.HasValue()) {
            params.resubscribeAutomatically = mAutoResubscribe.Value();
        }
        [cluster subscribeAttributeActiveEventsWithParams:params
            subscriptionEstablished:^() { mSubscriptionEstablished = YES; }
            reportHandler:^(NSArray * _Nullable value, NSError * _Nullable error) {
                NSLog(@"DemandResponseLoadControl.ActiveEvents response %@", [value description]);
                if (error == nil) {
                    RemoteDataModelLogger::LogAttributeAsJSON(@(endpointId), @(clusterId), @(attributeId), value);
                } else {
                    RemoteDataModelLogger::LogAttributeErrorAsJSON(@(endpointId), @(clusterId), @(attributeId), error);
                }
                SetCommandExitStatus(error);
            }];

        return CHIP_NO_ERROR;
    }
};

#endif // MTR_ENABLE_PROVISIONAL
#if MTR_ENABLE_PROVISIONAL

/*
 * Attribute NumberOfEventsPerProgram
 */
class ReadDemandResponseLoadControlNumberOfEventsPerProgram : public ReadAttribute {
public:
    ReadDemandResponseLoadControlNumberOfEventsPerProgram()
        : ReadAttribute("number-of-events-per-program")
    {
    }

    ~ReadDemandResponseLoadControlNumberOfEventsPerProgram()
    {
    }

    CHIP_ERROR SendCommand(MTRBaseDevice * device, chip::EndpointId endpointId) override
    {
        constexpr chip::ClusterId clusterId = chip::app::Clusters::DemandResponseLoadControl::Id;
        constexpr chip::AttributeId attributeId = chip::app::Clusters::DemandResponseLoadControl::Attributes::NumberOfEventsPerProgram::Id;

        ChipLogProgress(chipTool, "Sending cluster (0x%08" PRIX32 ") ReadAttribute (0x%08" PRIX32 ") on endpoint %u", endpointId, clusterId, attributeId);

        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);
        __auto_type * cluster = [[MTRBaseClusterDemandResponseLoadControl alloc] initWithDevice:device endpointID:@(endpointId) queue:callbackQueue];
        [cluster readAttributeNumberOfEventsPerProgramWithCompletion:^(NSNumber * _Nullable value, NSError * _Nullable error) {
            NSLog(@"DemandResponseLoadControl.NumberOfEventsPerProgram response %@", [value description]);
            if (error == nil) {
                RemoteDataModelLogger::LogAttributeAsJSON(@(endpointId), @(clusterId), @(attributeId), value);
            } else {
                LogNSError("DemandResponseLoadControl NumberOfEventsPerProgram read Error", error);
                RemoteDataModelLogger::LogAttributeErrorAsJSON(@(endpointId), @(clusterId), @(attributeId), error);
            }
            SetCommandExitStatus(error);
        }];
        return CHIP_NO_ERROR;
    }
};

class SubscribeAttributeDemandResponseLoadControlNumberOfEventsPerProgram : public SubscribeAttribute {
public:
    SubscribeAttributeDemandResponseLoadControlNumberOfEventsPerProgram()
        : SubscribeAttribute("number-of-events-per-program")
    {
    }

    ~SubscribeAttributeDemandResponseLoadControlNumberOfEventsPerProgram()
    {
    }

    CHIP_ERROR SendCommand(MTRBaseDevice * device, chip::EndpointId endpointId) override
    {
        constexpr chip::ClusterId clusterId = chip::app::Clusters::DemandResponseLoadControl::Id;
        constexpr chip::CommandId attributeId = chip::app::Clusters::DemandResponseLoadControl::Attributes::NumberOfEventsPerProgram::Id;

        ChipLogProgress(chipTool, "Sending cluster (0x%08" PRIX32 ") ReportAttribute (0x%08" PRIX32 ") on endpoint %u", clusterId, attributeId, endpointId);
        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);
        __auto_type * cluster = [[MTRBaseClusterDemandResponseLoadControl alloc] initWithDevice:device endpointID:@(endpointId) queue:callbackQueue];
        __auto_type * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(mMinInterval) maxInterval:@(mMaxInterval)];
        if (mKeepSubscriptions.HasValue()) {
            params.replaceExistingSubscriptions = !mKeepSubscriptions.Value();
        }
        if (mFabricFiltered.HasValue()) {
            params.filterByFabric = mFabricFiltered.Value();
        }
        if (mAutoResubscribe.HasValue()) {
            params.resubscribeAutomatically = mAutoResubscribe.Value();
        }
        [cluster subscribeAttributeNumberOfEventsPerProgramWithParams:params
            subscriptionEstablished:^() { mSubscriptionEstablished = YES; }
            reportHandler:^(NSNumber * _Nullable value, NSError * _Nullable error) {
                NSLog(@"DemandResponseLoadControl.NumberOfEventsPerProgram response %@", [value description]);
                if (error == nil) {
                    RemoteDataModelLogger::LogAttributeAsJSON(@(endpointId), @(clusterId), @(attributeId), value);
                } else {
                    RemoteDataModelLogger::LogAttributeErrorAsJSON(@(endpointId), @(clusterId), @(attributeId), error);
                }
                SetCommandExitStatus(error);
            }];

        return CHIP_NO_ERROR;
    }
};

#endif // MTR_ENABLE_PROVISIONAL
#if MTR_ENABLE_PROVISIONAL

/*
 * Attribute NumberOfTransitions
 */
class ReadDemandResponseLoadControlNumberOfTransitions : public ReadAttribute {
public:
    ReadDemandResponseLoadControlNumberOfTransitions()
        : ReadAttribute("number-of-transitions")
    {
    }

    ~ReadDemandResponseLoadControlNumberOfTransitions()
    {
    }

    CHIP_ERROR SendCommand(MTRBaseDevice * device, chip::EndpointId endpointId) override
    {
        constexpr chip::ClusterId clusterId = chip::app::Clusters::DemandResponseLoadControl::Id;
        constexpr chip::AttributeId attributeId = chip::app::Clusters::DemandResponseLoadControl::Attributes::NumberOfTransitions::Id;

        ChipLogProgress(chipTool, "Sending cluster (0x%08" PRIX32 ") ReadAttribute (0x%08" PRIX32 ") on endpoint %u", endpointId, clusterId, attributeId);

        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);
        __auto_type * cluster = [[MTRBaseClusterDemandResponseLoadControl alloc] initWithDevice:device endpointID:@(endpointId) queue:callbackQueue];
        [cluster readAttributeNumberOfTransitionsWithCompletion:^(NSNumber * _Nullable value, NSError * _Nullable error) {
            NSLog(@"DemandResponseLoadControl.NumberOfTransitions response %@", [value description]);
            if (error == nil) {
                RemoteDataModelLogger::LogAttributeAsJSON(@(endpointId), @(clusterId), @(attributeId), value);
            } else {
                LogNSError("DemandResponseLoadControl NumberOfTransitions read Error", error);
                RemoteDataModelLogger::LogAttributeErrorAsJSON(@(endpointId), @(clusterId), @(attributeId), error);
            }
            SetCommandExitStatus(error);
        }];
        return CHIP_NO_ERROR;
    }
};

class SubscribeAttributeDemandResponseLoadControlNumberOfTransitions : public SubscribeAttribute {
public:
    SubscribeAttributeDemandResponseLoadControlNumberOfTransitions()
        : SubscribeAttribute("number-of-transitions")
    {
    }

    ~SubscribeAttributeDemandResponseLoadControlNumberOfTransitions()
    {
    }

    CHIP_ERROR SendCommand(MTRBaseDevice * device, chip::EndpointId endpointId) override
    {
        constexpr chip::ClusterId clusterId = chip::app::Clusters::DemandResponseLoadControl::Id;
        constexpr chip::CommandId attributeId = chip::app::Clusters::DemandResponseLoadControl::Attributes::NumberOfTransitions::Id;

        ChipLogProgress(chipTool, "Sending cluster (0x%08" PRIX32 ") ReportAttribute (0x%08" PRIX32 ") on endpoint %u", clusterId, attributeId, endpointId);
        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);
        __auto_type * cluster = [[MTRBaseClusterDemandResponseLoadControl alloc] initWithDevice:device endpointID:@(endpointId) queue:callbackQueue];
        __auto_type * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(mMinInterval) maxInterval:@(mMaxInterval)];
        if (mKeepSubscriptions.HasValue()) {
            params.replaceExistingSubscriptions = !mKeepSubscriptions.Value();
        }
        if (mFabricFiltered.HasValue()) {
            params.filterByFabric = mFabricFiltered.Value();
        }
        if (mAutoResubscribe.HasValue()) {
            params.resubscribeAutomatically = mAutoResubscribe.Value();
        }
        [cluster subscribeAttributeNumberOfTransitionsWithParams:params
            subscriptionEstablished:^() { mSubscriptionEstablished = YES; }
            reportHandler:^(NSNumber * _Nullable value, NSError * _Nullable error) {
                NSLog(@"DemandResponseLoadControl.NumberOfTransitions response %@", [value description]);
                if (error == nil) {
                    RemoteDataModelLogger::LogAttributeAsJSON(@(endpointId), @(clusterId), @(attributeId), value);
                } else {
                    RemoteDataModelLogger::LogAttributeErrorAsJSON(@(endpointId), @(clusterId), @(attributeId), error);
                }
                SetCommandExitStatus(error);
            }];

        return CHIP_NO_ERROR;
    }
};

#endif // MTR_ENABLE_PROVISIONAL
#if MTR_ENABLE_PROVISIONAL

/*
 * Attribute DefaultRandomStart
 */
class ReadDemandResponseLoadControlDefaultRandomStart : public ReadAttribute {
public:
    ReadDemandResponseLoadControlDefaultRandomStart()
        : ReadAttribute("default-random-start")
    {
    }

    ~ReadDemandResponseLoadControlDefaultRandomStart()
    {
    }

    CHIP_ERROR SendCommand(MTRBaseDevice * device, chip::EndpointId endpointId) override
    {
        constexpr chip::ClusterId clusterId = chip::app::Clusters::DemandResponseLoadControl::Id;
        constexpr chip::AttributeId attributeId = chip::app::Clusters::DemandResponseLoadControl::Attributes::DefaultRandomStart::Id;

        ChipLogProgress(chipTool, "Sending cluster (0x%08" PRIX32 ") ReadAttribute (0x%08" PRIX32 ") on endpoint %u", endpointId, clusterId, attributeId);

        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);
        __auto_type * cluster = [[MTRBaseClusterDemandResponseLoadControl alloc] initWithDevice:device endpointID:@(endpointId) queue:callbackQueue];
        [cluster readAttributeDefaultRandomStartWithCompletion:^(NSNumber * _Nullable value, NSError * _Nullable error) {
            NSLog(@"DemandResponseLoadControl.DefaultRandomStart response %@", [value description]);
            if (error == nil) {
                RemoteDataModelLogger::LogAttributeAsJSON(@(endpointId), @(clusterId), @(attributeId), value);
            } else {
                LogNSError("DemandResponseLoadControl DefaultRandomStart read Error", error);
                RemoteDataModelLogger::LogAttributeErrorAsJSON(@(endpointId), @(clusterId), @(attributeId), error);
            }
            SetCommandExitStatus(error);
        }];
        return CHIP_NO_ERROR;
    }
};

class WriteDemandResponseLoadControlDefaultRandomStart : public WriteAttribute {
public:
    WriteDemandResponseLoadControlDefaultRandomStart()
        : WriteAttribute("default-random-start")
    {
        AddArgument("attr-name", "default-random-start");
        AddArgument("attr-value", 0, UINT8_MAX, &mValue);
        WriteAttribute::AddArguments();
    }

    ~WriteDemandResponseLoadControlDefaultRandomStart()
    {
    }

    CHIP_ERROR SendCommand(MTRBaseDevice * device, chip::EndpointId endpointId) override
    {
        constexpr chip::ClusterId clusterId = chip::app::Clusters::DemandResponseLoadControl::Id;
        constexpr chip::AttributeId attributeId = chip::app::Clusters::DemandResponseLoadControl::Attributes::DefaultRandomStart::Id;

        ChipLogProgress(chipTool, "Sending cluster (0x%08" PRIX32 ") WriteAttribute (0x%08" PRIX32 ") on endpoint %u", clusterId, attributeId, endpointId);
        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);
        __auto_type * cluster = [[MTRBaseClusterDemandResponseLoadControl alloc] initWithDevice:device endpointID:@(endpointId) queue:callbackQueue];
        __auto_type * params = [[MTRWriteParams alloc] init];
        params.timedWriteTimeout = mTimedInteractionTimeoutMs.HasValue() ? [NSNumber numberWithUnsignedShort:mTimedInteractionTimeoutMs.Value()] : nil;
        params.dataVersion = mDataVersion.HasValue() ? [NSNumber numberWithUnsignedInt:mDataVersion.Value()] : nil;
        NSNumber * _Nonnull value = [NSNumber numberWithUnsignedChar:mValue];

        [cluster writeAttributeDefaultRandomStartWithValue:value params:params completion:^(NSError * _Nullable error) {
            if (error != nil) {
                LogNSError("DemandResponseLoadControl DefaultRandomStart write Error", error);
                RemoteDataModelLogger::LogAttributeErrorAsJSON(@(endpointId), @(clusterId), @(attributeId), error);
            }
            SetCommandExitStatus(error);
        }];
        return CHIP_NO_ERROR;
    }

private:
    uint8_t mValue;
};

class SubscribeAttributeDemandResponseLoadControlDefaultRandomStart : public SubscribeAttribute {
public:
    SubscribeAttributeDemandResponseLoadControlDefaultRandomStart()
        : SubscribeAttribute("default-random-start")
    {
    }

    ~SubscribeAttributeDemandResponseLoadControlDefaultRandomStart()
    {
    }

    CHIP_ERROR SendCommand(MTRBaseDevice * device, chip::EndpointId endpointId) override
    {
        constexpr chip::ClusterId clusterId = chip::app::Clusters::DemandResponseLoadControl::Id;
        constexpr chip::CommandId attributeId = chip::app::Clusters::DemandResponseLoadControl::Attributes::DefaultRandomStart::Id;

        ChipLogProgress(chipTool, "Sending cluster (0x%08" PRIX32 ") ReportAttribute (0x%08" PRIX32 ") on endpoint %u", clusterId, attributeId, endpointId);
        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);
        __auto_type * cluster = [[MTRBaseClusterDemandResponseLoadControl alloc] initWithDevice:device endpointID:@(endpointId) queue:callbackQueue];
        __auto_type * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(mMinInterval) maxInterval:@(mMaxInterval)];
        if (mKeepSubscriptions.HasValue()) {
            params.replaceExistingSubscriptions = !mKeepSubscriptions.Value();
        }
        if (mFabricFiltered.HasValue()) {
            params.filterByFabric = mFabricFiltered.Value();
        }
        if (mAutoResubscribe.HasValue()) {
            params.resubscribeAutomatically = mAutoResubscribe.Value();
        }
        [cluster subscribeAttributeDefaultRandomStartWithParams:params
            subscriptionEstablished:^() { mSubscriptionEstablished = YES; }
            reportHandler:^(NSNumber * _Nullable value, NSError * _Nullable error) {
                NSLog(@"DemandResponseLoadControl.DefaultRandomStart response %@", [value description]);
                if (error == nil) {
                    RemoteDataModelLogger::LogAttributeAsJSON(@(endpointId), @(clusterId), @(attributeId), value);
                } else {
                    RemoteDataModelLogger::LogAttributeErrorAsJSON(@(endpointId), @(clusterId), @(attributeId), error);
                }
                SetCommandExitStatus(error);
            }];

        return CHIP_NO_ERROR;
    }
};

#endif // MTR_ENABLE_PROVISIONAL
#if MTR_ENABLE_PROVISIONAL

/*
 * Attribute DefaultRandomDuration
 */
class ReadDemandResponseLoadControlDefaultRandomDuration : public ReadAttribute {
public:
    ReadDemandResponseLoadControlDefaultRandomDuration()
        : ReadAttribute("default-random-duration")
    {
    }

    ~ReadDemandResponseLoadControlDefaultRandomDuration()
    {
    }

    CHIP_ERROR SendCommand(MTRBaseDevice * device, chip::EndpointId endpointId) override
    {
        constexpr chip::ClusterId clusterId = chip::app::Clusters::DemandResponseLoadControl::Id;
        constexpr chip::AttributeId attributeId = chip::app::Clusters::DemandResponseLoadControl::Attributes::DefaultRandomDuration::Id;

        ChipLogProgress(chipTool, "Sending cluster (0x%08" PRIX32 ") ReadAttribute (0x%08" PRIX32 ") on endpoint %u", endpointId, clusterId, attributeId);

        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);
        __auto_type * cluster = [[MTRBaseClusterDemandResponseLoadControl alloc] initWithDevice:device endpointID:@(endpointId) queue:callbackQueue];
        [cluster readAttributeDefaultRandomDurationWithCompletion:^(NSNumber * _Nullable value, NSError * _Nullable error) {
            NSLog(@"DemandResponseLoadControl.DefaultRandomDuration response %@", [value description]);
            if (error == nil) {
                RemoteDataModelLogger::LogAttributeAsJSON(@(endpointId), @(clusterId), @(attributeId), value);
            } else {
                LogNSError("DemandResponseLoadControl DefaultRandomDuration read Error", error);
                RemoteDataModelLogger::LogAttributeErrorAsJSON(@(endpointId), @(clusterId), @(attributeId), error);
            }
            SetCommandExitStatus(error);
        }];
        return CHIP_NO_ERROR;
    }
};

class WriteDemandResponseLoadControlDefaultRandomDuration : public WriteAttribute {
public:
    WriteDemandResponseLoadControlDefaultRandomDuration()
        : WriteAttribute("default-random-duration")
    {
        AddArgument("attr-name", "default-random-duration");
        AddArgument("attr-value", 0, UINT8_MAX, &mValue);
        WriteAttribute::AddArguments();
    }

    ~WriteDemandResponseLoadControlDefaultRandomDuration()
    {
    }

    CHIP_ERROR SendCommand(MTRBaseDevice * device, chip::EndpointId endpointId) override
    {
        constexpr chip::ClusterId clusterId = chip::app::Clusters::DemandResponseLoadControl::Id;
        constexpr chip::AttributeId attributeId = chip::app::Clusters::DemandResponseLoadControl::Attributes::DefaultRandomDuration::Id;

        ChipLogProgress(chipTool, "Sending cluster (0x%08" PRIX32 ") WriteAttribute (0x%08" PRIX32 ") on endpoint %u", clusterId, attributeId, endpointId);
        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);
        __auto_type * cluster = [[MTRBaseClusterDemandResponseLoadControl alloc] initWithDevice:device endpointID:@(endpointId) queue:callbackQueue];
        __auto_type * params = [[MTRWriteParams alloc] init];
        params.timedWriteTimeout = mTimedInteractionTimeoutMs.HasValue() ? [NSNumber numberWithUnsignedShort:mTimedInteractionTimeoutMs.Value()] : nil;
        params.dataVersion = mDataVersion.HasValue() ? [NSNumber numberWithUnsignedInt:mDataVersion.Value()] : nil;
        NSNumber * _Nonnull value = [NSNumber numberWithUnsignedChar:mValue];

        [cluster writeAttributeDefaultRandomDurationWithValue:value params:params completion:^(NSError * _Nullable error) {
            if (error != nil) {
                LogNSError("DemandResponseLoadControl DefaultRandomDuration write Error", error);
                RemoteDataModelLogger::LogAttributeErrorAsJSON(@(endpointId), @(clusterId), @(attributeId), error);
            }
            SetCommandExitStatus(error);
        }];
        return CHIP_NO_ERROR;
    }

private:
    uint8_t mValue;
};

class SubscribeAttributeDemandResponseLoadControlDefaultRandomDuration : public SubscribeAttribute {
public:
    SubscribeAttributeDemandResponseLoadControlDefaultRandomDuration()
        : SubscribeAttribute("default-random-duration")
    {
    }

    ~SubscribeAttributeDemandResponseLoadControlDefaultRandomDuration()
    {
    }

    CHIP_ERROR SendCommand(MTRBaseDevice * device, chip::EndpointId endpointId) override
    {
        constexpr chip::ClusterId clusterId = chip::app::Clusters::DemandResponseLoadControl::Id;
        constexpr chip::CommandId attributeId = chip::app::Clusters::DemandResponseLoadControl::Attributes::DefaultRandomDuration::Id;

        ChipLogProgress(chipTool, "Sending cluster (0x%08" PRIX32 ") ReportAttribute (0x%08" PRIX32 ") on endpoint %u", clusterId, attributeId, endpointId);
        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);
        __auto_type * cluster = [[MTRBaseClusterDemandResponseLoadControl alloc] initWithDevice:device endpointID:@(endpointId) queue:callbackQueue];
        __auto_type * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(mMinInterval) maxInterval:@(mMaxInterval)];
        if (mKeepSubscriptions.HasValue()) {
            params.replaceExistingSubscriptions = !mKeepSubscriptions.Value();
        }
        if (mFabricFiltered.HasValue()) {
            params.filterByFabric = mFabricFiltered.Value();
        }
        if (mAutoResubscribe.HasValue()) {
            params.resubscribeAutomatically = mAutoResubscribe.Value();
        }
        [cluster subscribeAttributeDefaultRandomDurationWithParams:params
            subscriptionEstablished:^() { mSubscriptionEstablished = YES; }
            reportHandler:^(NSNumber * _Nullable value, NSError * _Nullable error) {
                NSLog(@"DemandResponseLoadControl.DefaultRandomDuration response %@", [value description]);
                if (error == nil) {
                    RemoteDataModelLogger::LogAttributeAsJSON(@(endpointId), @(clusterId), @(attributeId), value);
                } else {
                    RemoteDataModelLogger::LogAttributeErrorAsJSON(@(endpointId), @(clusterId), @(attributeId), error);
                }
                SetCommandExitStatus(error);
            }];

        return CHIP_NO_ERROR;
    }
};

#endif // MTR_ENABLE_PROVISIONAL
#if MTR_ENABLE_PROVISIONAL

/*
 * Attribute GeneratedCommandList
 */
class ReadDemandResponseLoadControlGeneratedCommandList : public ReadAttribute {
public:
    ReadDemandResponseLoadControlGeneratedCommandList()
        : ReadAttribute("generated-command-list")
    {
    }

    ~ReadDemandResponseLoadControlGeneratedCommandList()
    {
    }

    CHIP_ERROR SendCommand(MTRBaseDevice * device, chip::EndpointId endpointId) override
    {
        constexpr chip::ClusterId clusterId = chip::app::Clusters::DemandResponseLoadControl::Id;
        constexpr chip::AttributeId attributeId = chip::app::Clusters::DemandResponseLoadControl::Attributes::GeneratedCommandList::Id;

        ChipLogProgress(chipTool, "Sending cluster (0x%08" PRIX32 ") ReadAttribute (0x%08" PRIX32 ") on endpoint %u", endpointId, clusterId, attributeId);

        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);
        __auto_type * cluster = [[MTRBaseClusterDemandResponseLoadControl alloc] initWithDevice:device endpointID:@(endpointId) queue:callbackQueue];
        [cluster readAttributeGeneratedCommandListWithCompletion:^(NSArray * _Nullable value, NSError * _Nullable error) {
            NSLog(@"DemandResponseLoadControl.GeneratedCommandList response %@", [value description]);
            if (error == nil) {
                RemoteDataModelLogger::LogAttributeAsJSON(@(endpointId), @(clusterId), @(attributeId), value);
            } else {
                LogNSError("DemandResponseLoadControl GeneratedCommandList read Error", error);
                RemoteDataModelLogger::LogAttributeErrorAsJSON(@(endpointId), @(clusterId), @(attributeId), error);
            }
            SetCommandExitStatus(error);
        }];
        return CHIP_NO_ERROR;
    }
};

class SubscribeAttributeDemandResponseLoadControlGeneratedCommandList : public SubscribeAttribute {
public:
    SubscribeAttributeDemandResponseLoadControlGeneratedCommandList()
        : SubscribeAttribute("generated-command-list")
    {
    }

    ~SubscribeAttributeDemandResponseLoadControlGeneratedCommandList()
    {
    }

    CHIP_ERROR SendCommand(MTRBaseDevice * device, chip::EndpointId endpointId) override
    {
        constexpr chip::ClusterId clusterId = chip::app::Clusters::DemandResponseLoadControl::Id;
        constexpr chip::CommandId attributeId = chip::app::Clusters::DemandResponseLoadControl::Attributes::GeneratedCommandList::Id;

        ChipLogProgress(chipTool, "Sending cluster (0x%08" PRIX32 ") ReportAttribute (0x%08" PRIX32 ") on endpoint %u", clusterId, attributeId, endpointId);
        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);
        __auto_type * cluster = [[MTRBaseClusterDemandResponseLoadControl alloc] initWithDevice:device endpointID:@(endpointId) queue:callbackQueue];
        __auto_type * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(mMinInterval) maxInterval:@(mMaxInterval)];
        if (mKeepSubscriptions.HasValue()) {
            params.replaceExistingSubscriptions = !mKeepSubscriptions.Value();
        }
        if (mFabricFiltered.HasValue()) {
            params.filterByFabric = mFabricFiltered.Value();
        }
        if (mAutoResubscribe.HasValue()) {
            params.resubscribeAutomatically = mAutoResubscribe.Value();
        }
        [cluster subscribeAttributeGeneratedCommandListWithParams:params
            subscriptionEstablished:^() { mSubscriptionEstablished = YES; }
            reportHandler:^(NSArray * _Nullable value, NSError * _Nullable error) {
                NSLog(@"DemandResponseLoadControl.GeneratedCommandList response %@", [value description]);
                if (error == nil) {
                    RemoteDataModelLogger::LogAttributeAsJSON(@(endpointId), @(clusterId), @(attributeId), value);
                } else {
                    RemoteDataModelLogger::LogAttributeErrorAsJSON(@(endpointId), @(clusterId), @(attributeId), error);
                }
                SetCommandExitStatus(error);
            }];

        return CHIP_NO_ERROR;
    }
};

#endif // MTR_ENABLE_PROVISIONAL
#if MTR_ENABLE_PROVISIONAL

/*
 * Attribute AcceptedCommandList
 */
class ReadDemandResponseLoadControlAcceptedCommandList : public ReadAttribute {
public:
    ReadDemandResponseLoadControlAcceptedCommandList()
        : ReadAttribute("accepted-command-list")
    {
    }

    ~ReadDemandResponseLoadControlAcceptedCommandList()
    {
    }

    CHIP_ERROR SendCommand(MTRBaseDevice * device, chip::EndpointId endpointId) override
    {
        constexpr chip::ClusterId clusterId = chip::app::Clusters::DemandResponseLoadControl::Id;
        constexpr chip::AttributeId attributeId = chip::app::Clusters::DemandResponseLoadControl::Attributes::AcceptedCommandList::Id;

        ChipLogProgress(chipTool, "Sending cluster (0x%08" PRIX32 ") ReadAttribute (0x%08" PRIX32 ") on endpoint %u", endpointId, clusterId, attributeId);

        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);
        __auto_type * cluster = [[MTRBaseClusterDemandResponseLoadControl alloc] initWithDevice:device endpointID:@(endpointId) queue:callbackQueue];
        [cluster readAttributeAcceptedCommandListWithCompletion:^(NSArray * _Nullable value, NSError * _Nullable error) {
            NSLog(@"DemandResponseLoadControl.AcceptedCommandList response %@", [value description]);
            if (error == nil) {
                RemoteDataModelLogger::LogAttributeAsJSON(@(endpointId), @(clusterId), @(attributeId), value);
            } else {
                LogNSError("DemandResponseLoadControl AcceptedCommandList read Error", error);
                RemoteDataModelLogger::LogAttributeErrorAsJSON(@(endpointId), @(clusterId), @(attributeId), error);
            }
            SetCommandExitStatus(error);
        }];
        return CHIP_NO_ERROR;
    }
};

class SubscribeAttributeDemandResponseLoadControlAcceptedCommandList : public SubscribeAttribute {
public:
    SubscribeAttributeDemandResponseLoadControlAcceptedCommandList()
        : SubscribeAttribute("accepted-command-list")
    {
    }

    ~SubscribeAttributeDemandResponseLoadControlAcceptedCommandList()
    {
    }

    CHIP_ERROR SendCommand(MTRBaseDevice * device, chip::EndpointId endpointId) override
    {
        constexpr chip::ClusterId clusterId = chip::app::Clusters::DemandResponseLoadControl::Id;
        constexpr chip::CommandId attributeId = chip::app::Clusters::DemandResponseLoadControl::Attributes::AcceptedCommandList::Id;

        ChipLogProgress(chipTool, "Sending cluster (0x%08" PRIX32 ") ReportAttribute (0x%08" PRIX32 ") on endpoint %u", clusterId, attributeId, endpointId);
        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);
        __auto_type * cluster = [[MTRBaseClusterDemandResponseLoadControl alloc] initWithDevice:device endpointID:@(endpointId) queue:callbackQueue];
        __auto_type * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(mMinInterval) maxInterval:@(mMaxInterval)];
        if (mKeepSubscriptions.HasValue()) {
            params.replaceExistingSubscriptions = !mKeepSubscriptions.Value();
        }
        if (mFabricFiltered.HasValue()) {
            params.filterByFabric = mFabricFiltered.Value();
        }
        if (mAutoResubscribe.HasValue()) {
            params.resubscribeAutomatically = mAutoResubscribe.Value();
        }
        [cluster subscribeAttributeAcceptedCommandListWithParams:params
            subscriptionEstablished:^() { mSubscriptionEstablished = YES; }
            reportHandler:^(NSArray * _Nullable value, NSError * _Nullable error) {
                NSLog(@"DemandResponseLoadControl.AcceptedCommandList response %@", [value description]);
                if (error == nil) {
                    RemoteDataModelLogger::LogAttributeAsJSON(@(endpointId), @(clusterId), @(attributeId), value);
                } else {
                    RemoteDataModelLogger::LogAttributeErrorAsJSON(@(endpointId), @(clusterId), @(attributeId), error);
                }
                SetCommandExitStatus(error);
            }];

        return CHIP_NO_ERROR;
    }
};

#endif // MTR_ENABLE_PROVISIONAL
#if MTR_ENABLE_PROVISIONAL

/*
 * Attribute EventList
 */
class ReadDemandResponseLoadControlEventList : public ReadAttribute {
public:
    ReadDemandResponseLoadControlEventList()
        : ReadAttribute("event-list")
    {
    }

    ~ReadDemandResponseLoadControlEventList()
    {
    }

    CHIP_ERROR SendCommand(MTRBaseDevice * device, chip::EndpointId endpointId) override
    {
        constexpr chip::ClusterId clusterId = chip::app::Clusters::DemandResponseLoadControl::Id;
        constexpr chip::AttributeId attributeId = chip::app::Clusters::DemandResponseLoadControl::Attributes::EventList::Id;

        ChipLogProgress(chipTool, "Sending cluster (0x%08" PRIX32 ") ReadAttribute (0x%08" PRIX32 ") on endpoint %u", endpointId, clusterId, attributeId);

        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);
        __auto_type * cluster = [[MTRBaseClusterDemandResponseLoadControl alloc] initWithDevice:device endpointID:@(endpointId) queue:callbackQueue];
        [cluster readAttributeEventListWithCompletion:^(NSArray * _Nullable value, NSError * _Nullable error) {
            NSLog(@"DemandResponseLoadControl.EventList response %@", [value description]);
            if (error == nil) {
                RemoteDataModelLogger::LogAttributeAsJSON(@(endpointId), @(clusterId), @(attributeId), value);
            } else {
                LogNSError("DemandResponseLoadControl EventList read Error", error);
                RemoteDataModelLogger::LogAttributeErrorAsJSON(@(endpointId), @(clusterId), @(attributeId), error);
            }
            SetCommandExitStatus(error);
        }];
        return CHIP_NO_ERROR;
    }
};

class SubscribeAttributeDemandResponseLoadControlEventList : public SubscribeAttribute {
public:
    SubscribeAttributeDemandResponseLoadControlEventList()
        : SubscribeAttribute("event-list")
    {
    }

    ~SubscribeAttributeDemandResponseLoadControlEventList()
    {
    }

    CHIP_ERROR SendCommand(MTRBaseDevice * device, chip::EndpointId endpointId) override
    {
        constexpr chip::ClusterId clusterId = chip::app::Clusters::DemandResponseLoadControl::Id;
        constexpr chip::CommandId attributeId = chip::app::Clusters::DemandResponseLoadControl::Attributes::EventList::Id;

        ChipLogProgress(chipTool, "Sending cluster (0x%08" PRIX32 ") ReportAttribute (0x%08" PRIX32 ") on endpoint %u", clusterId, attributeId, endpointId);
        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);
        __auto_type * cluster = [[MTRBaseClusterDemandResponseLoadControl alloc] initWithDevice:device endpointID:@(endpointId) queue:callbackQueue];
        __auto_type * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(mMinInterval) maxInterval:@(mMaxInterval)];
        if (mKeepSubscriptions.HasValue()) {
            params.replaceExistingSubscriptions = !mKeepSubscriptions.Value();
        }
        if (mFabricFiltered.HasValue()) {
            params.filterByFabric = mFabricFiltered.Value();
        }
        if (mAutoResubscribe.HasValue()) {
            params.resubscribeAutomatically = mAutoResubscribe.Value();
        }
        [cluster subscribeAttributeEventListWithParams:params
            subscriptionEstablished:^() { mSubscriptionEstablished = YES; }
            reportHandler:^(NSArray * _Nullable value, NSError * _Nullable error) {
                NSLog(@"DemandResponseLoadControl.EventList response %@", [value description]);
                if (error == nil) {
                    RemoteDataModelLogger::LogAttributeAsJSON(@(endpointId), @(clusterId), @(attributeId), value);
                } else {
                    RemoteDataModelLogger::LogAttributeErrorAsJSON(@(endpointId), @(clusterId), @(attributeId), error);
                }
                SetCommandExitStatus(error);
            }];

        return CHIP_NO_ERROR;
    }
};

#endif // MTR_ENABLE_PROVISIONAL
#if MTR_ENABLE_PROVISIONAL

/*
 * Attribute AttributeList
 */
class ReadDemandResponseLoadControlAttributeList : public ReadAttribute {
public:
    ReadDemandResponseLoadControlAttributeList()
        : ReadAttribute("attribute-list")
    {
    }

    ~ReadDemandResponseLoadControlAttributeList()
    {
    }

    CHIP_ERROR SendCommand(MTRBaseDevice * device, chip::EndpointId endpointId) override
    {
        constexpr chip::ClusterId clusterId = chip::app::Clusters::DemandResponseLoadControl::Id;
        constexpr chip::AttributeId attributeId = chip::app::Clusters::DemandResponseLoadControl::Attributes::AttributeList::Id;

        ChipLogProgress(chipTool, "Sending cluster (0x%08" PRIX32 ") ReadAttribute (0x%08" PRIX32 ") on endpoint %u", endpointId, clusterId, attributeId);

        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);
        __auto_type * cluster = [[MTRBaseClusterDemandResponseLoadControl alloc] initWithDevice:device endpointID:@(endpointId) queue:callbackQueue];
        [cluster readAttributeAttributeListWithCompletion:^(NSArray * _Nullable value, NSError * _Nullable error) {
            NSLog(@"DemandResponseLoadControl.AttributeList response %@", [value description]);
            if (error == nil) {
                RemoteDataModelLogger::LogAttributeAsJSON(@(endpointId), @(clusterId), @(attributeId), value);
            } else {
                LogNSError("DemandResponseLoadControl AttributeList read Error", error);
                RemoteDataModelLogger::LogAttributeErrorAsJSON(@(endpointId), @(clusterId), @(attributeId), error);
            }
            SetCommandExitStatus(error);
        }];
        return CHIP_NO_ERROR;
    }
};

class SubscribeAttributeDemandResponseLoadControlAttributeList : public SubscribeAttribute {
public:
    SubscribeAttributeDemandResponseLoadControlAttributeList()
        : SubscribeAttribute("attribute-list")
    {
    }

    ~SubscribeAttributeDemandResponseLoadControlAttributeList()
    {
    }

    CHIP_ERROR SendCommand(MTRBaseDevice * device, chip::EndpointId endpointId) override
    {
        constexpr chip::ClusterId clusterId = chip::app::Clusters::DemandResponseLoadControl::Id;
        constexpr chip::CommandId attributeId = chip::app::Clusters::DemandResponseLoadControl::Attributes::AttributeList::Id;

        ChipLogProgress(chipTool, "Sending cluster (0x%08" PRIX32 ") ReportAttribute (0x%08" PRIX32 ") on endpoint %u", clusterId, attributeId, endpointId);
        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);
        __auto_type * cluster = [[MTRBaseClusterDemandResponseLoadControl alloc] initWithDevice:device endpointID:@(endpointId) queue:callbackQueue];
        __auto_type * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(mMinInterval) maxInterval:@(mMaxInterval)];
        if (mKeepSubscriptions.HasValue()) {
            params.replaceExistingSubscriptions = !mKeepSubscriptions.Value();
        }
        if (mFabricFiltered.HasValue()) {
            params.filterByFabric = mFabricFiltered.Value();
        }
        if (mAutoResubscribe.HasValue()) {
            params.resubscribeAutomatically = mAutoResubscribe.Value();
        }
        [cluster subscribeAttributeAttributeListWithParams:params
            subscriptionEstablished:^() { mSubscriptionEstablished = YES; }
            reportHandler:^(NSArray * _Nullable value, NSError * _Nullable error) {
                NSLog(@"DemandResponseLoadControl.AttributeList response %@", [value description]);
                if (error == nil) {
                    RemoteDataModelLogger::LogAttributeAsJSON(@(endpointId), @(clusterId), @(attributeId), value);
                } else {
                    RemoteDataModelLogger::LogAttributeErrorAsJSON(@(endpointId), @(clusterId), @(attributeId), error);
                }
                SetCommandExitStatus(error);
            }];

        return CHIP_NO_ERROR;
    }
};

#endif // MTR_ENABLE_PROVISIONAL
#if MTR_ENABLE_PROVISIONAL

/*
 * Attribute FeatureMap
 */
class ReadDemandResponseLoadControlFeatureMap : public ReadAttribute {
public:
    ReadDemandResponseLoadControlFeatureMap()
        : ReadAttribute("feature-map")
    {
    }

    ~ReadDemandResponseLoadControlFeatureMap()
    {
    }

    CHIP_ERROR SendCommand(MTRBaseDevice * device, chip::EndpointId endpointId) override
    {
        constexpr chip::ClusterId clusterId = chip::app::Clusters::DemandResponseLoadControl::Id;
        constexpr chip::AttributeId attributeId = chip::app::Clusters::DemandResponseLoadControl::Attributes::FeatureMap::Id;

        ChipLogProgress(chipTool, "Sending cluster (0x%08" PRIX32 ") ReadAttribute (0x%08" PRIX32 ") on endpoint %u", endpointId, clusterId, attributeId);

        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);
        __auto_type * cluster = [[MTRBaseClusterDemandResponseLoadControl alloc] initWithDevice:device endpointID:@(endpointId) queue:callbackQueue];
        [cluster readAttributeFeatureMapWithCompletion:^(NSNumber * _Nullable value, NSError * _Nullable error) {
            NSLog(@"DemandResponseLoadControl.FeatureMap response %@", [value description]);
            if (error == nil) {
                RemoteDataModelLogger::LogAttributeAsJSON(@(endpointId), @(clusterId), @(attributeId), value);
            } else {
                LogNSError("DemandResponseLoadControl FeatureMap read Error", error);
                RemoteDataModelLogger::LogAttributeErrorAsJSON(@(endpointId), @(clusterId), @(attributeId), error);
            }
            SetCommandExitStatus(error);
        }];
        return CHIP_NO_ERROR;
    }
};

class SubscribeAttributeDemandResponseLoadControlFeatureMap : public SubscribeAttribute {
public:
    SubscribeAttributeDemandResponseLoadControlFeatureMap()
        : SubscribeAttribute("feature-map")
    {
    }

    ~SubscribeAttributeDemandResponseLoadControlFeatureMap()
    {
    }

    CHIP_ERROR SendCommand(MTRBaseDevice * device, chip::EndpointId endpointId) override
    {
        constexpr chip::ClusterId clusterId = chip::app::Clusters::DemandResponseLoadControl::Id;
        constexpr chip::CommandId attributeId = chip::app::Clusters::DemandResponseLoadControl::Attributes::FeatureMap::Id;

        ChipLogProgress(chipTool, "Sending cluster (0x%08" PRIX32 ") ReportAttribute (0x%08" PRIX32 ") on endpoint %u", clusterId, attributeId, endpointId);
        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);
        __auto_type * cluster = [[MTRBaseClusterDemandResponseLoadControl alloc] initWithDevice:device endpointID:@(endpointId) queue:callbackQueue];
        __auto_type * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(mMinInterval) maxInterval:@(mMaxInterval)];
        if (mKeepSubscriptions.HasValue()) {
            params.replaceExistingSubscriptions = !mKeepSubscriptions.Value();
        }
        if (mFabricFiltered.HasValue()) {
            params.filterByFabric = mFabricFiltered.Value();
        }
        if (mAutoResubscribe.HasValue()) {
            params.resubscribeAutomatically = mAutoResubscribe.Value();
        }
        [cluster subscribeAttributeFeatureMapWithParams:params
            subscriptionEstablished:^() { mSubscriptionEstablished = YES; }
            reportHandler:^(NSNumber * _Nullable value, NSError * _Nullable error) {
                NSLog(@"DemandResponseLoadControl.FeatureMap response %@", [value description]);
                if (error == nil) {
                    RemoteDataModelLogger::LogAttributeAsJSON(@(endpointId), @(clusterId), @(attributeId), value);
                } else {
                    RemoteDataModelLogger::LogAttributeErrorAsJSON(@(endpointId), @(clusterId), @(attributeId), error);
                }
                SetCommandExitStatus(error);
            }];

        return CHIP_NO_ERROR;
    }
};

#endif // MTR_ENABLE_PROVISIONAL
#if MTR_ENABLE_PROVISIONAL

/*
 * Attribute ClusterRevision
 */
class ReadDemandResponseLoadControlClusterRevision : public ReadAttribute {
public:
    ReadDemandResponseLoadControlClusterRevision()
        : ReadAttribute("cluster-revision")
    {
    }

    ~ReadDemandResponseLoadControlClusterRevision()
    {
    }

    CHIP_ERROR SendCommand(MTRBaseDevice * device, chip::EndpointId endpointId) override
    {
        constexpr chip::ClusterId clusterId = chip::app::Clusters::DemandResponseLoadControl::Id;
        constexpr chip::AttributeId attributeId = chip::app::Clusters::DemandResponseLoadControl::Attributes::ClusterRevision::Id;

        ChipLogProgress(chipTool, "Sending cluster (0x%08" PRIX32 ") ReadAttribute (0x%08" PRIX32 ") on endpoint %u", endpointId, clusterId, attributeId);

        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);
        __auto_type * cluster = [[MTRBaseClusterDemandResponseLoadControl alloc] initWithDevice:device endpointID:@(endpointId) queue:callbackQueue];
        [cluster readAttributeClusterRevisionWithCompletion:^(NSNumber * _Nullable value, NSError * _Nullable error) {
            NSLog(@"DemandResponseLoadControl.ClusterRevision response %@", [value description]);
            if (error == nil) {
                RemoteDataModelLogger::LogAttributeAsJSON(@(endpointId), @(clusterId), @(attributeId), value);
            } else {
                LogNSError("DemandResponseLoadControl ClusterRevision read Error", error);
                RemoteDataModelLogger::LogAttributeErrorAsJSON(@(endpointId), @(clusterId), @(attributeId), error);
            }
            SetCommandExitStatus(error);
        }];
        return CHIP_NO_ERROR;
    }
};

class SubscribeAttributeDemandResponseLoadControlClusterRevision : public SubscribeAttribute {
public:
    SubscribeAttributeDemandResponseLoadControlClusterRevision()
        : SubscribeAttribute("cluster-revision")
    {
    }

    ~SubscribeAttributeDemandResponseLoadControlClusterRevision()
    {
    }

    CHIP_ERROR SendCommand(MTRBaseDevice * device, chip::EndpointId endpointId) override
    {
        constexpr chip::ClusterId clusterId = chip::app::Clusters::DemandResponseLoadControl::Id;
        constexpr chip::CommandId attributeId = chip::app::Clusters::DemandResponseLoadControl::Attributes::ClusterRevision::Id;

        ChipLogProgress(chipTool, "Sending cluster (0x%08" PRIX32 ") ReportAttribute (0x%08" PRIX32 ") on endpoint %u", clusterId, attributeId, endpointId);
        dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.command", DISPATCH_QUEUE_SERIAL);
        __auto_type * cluster = [[MTRBaseClusterDemandResponseLoadControl alloc] initWithDevice:device endpointID:@(endpointId) queue:callbackQueue];
        __auto_type * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(mMinInterval) maxInterval:@(mMaxInterval)];
        if (mKeepSubscriptions.HasValue()) {
            params.replaceExistingSubscriptions = !mKeepSubscriptions.Value();
        }
        if (mFabricFiltered.HasValue()) {
            params.filterByFabric = mFabricFiltered.Value();
        }
        if (mAutoResubscribe.HasValue()) {
            params.resubscribeAutomatically = mAutoResubscribe.Value();
        }
        [cluster subscribeAttributeClusterRevisionWithParams:params
            subscriptionEstablished:^() { mSubscriptionEstablished = YES; }
            reportHandler:^(NSNumber * _Nullable value, NSError * _Nullable error) {
                NSLog(@"DemandResponseLoadControl.ClusterRevision response %@", [value description]);
                if (error == nil) {
                    RemoteDataModelLogger::LogAttributeAsJSON(@(endpointId), @(clusterId), @(attributeId), value);
                } else {
                    RemoteDataModelLogger::LogAttributeErrorAsJSON(@(endpointId), @(clusterId), @(attributeId), error);
                }
                SetCommandExitStatus(error);
            }];

        return CHIP_NO_ERROR;
    }
};
