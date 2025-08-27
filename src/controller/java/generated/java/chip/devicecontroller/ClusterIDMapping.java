/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
package chip.devicecontroller;

public class ClusterIDMapping {
    public interface BaseCluster {
        long getID();
        String getAttributeName(long id) throws NoSuchFieldError;
        String getEventName(long id) throws NoSuchFieldError;
        String getCommandName(long id) throws NoSuchFieldError;
        long getAttributeID(String name) throws IllegalArgumentException;
        long getEventID(String name) throws IllegalArgumentException;
        long getCommandID(String name) throws IllegalArgumentException;
    }

    public static BaseCluster getCluster(long clusterId) {
        if (clusterId == Identify.ID) {
            return new Identify();
        }
        if (clusterId == Groups.ID) {
            return new Groups();
        }
        if (clusterId == OnOff.ID) {
            return new OnOff();
        }
        if (clusterId == LevelControl.ID) {
            return new LevelControl();
        }
        if (clusterId == PulseWidthModulation.ID) {
            return new PulseWidthModulation();
        }
        if (clusterId == Descriptor.ID) {
            return new Descriptor();
        }
        if (clusterId == Binding.ID) {
            return new Binding();
        }
        if (clusterId == AccessControl.ID) {
            return new AccessControl();
        }
        if (clusterId == Actions.ID) {
            return new Actions();
        }
        if (clusterId == BasicInformation.ID) {
            return new BasicInformation();
        }
        if (clusterId == OtaSoftwareUpdateProvider.ID) {
            return new OtaSoftwareUpdateProvider();
        }
        if (clusterId == OtaSoftwareUpdateRequestor.ID) {
            return new OtaSoftwareUpdateRequestor();
        }
        if (clusterId == LocalizationConfiguration.ID) {
            return new LocalizationConfiguration();
        }
        if (clusterId == TimeFormatLocalization.ID) {
            return new TimeFormatLocalization();
        }
        if (clusterId == UnitLocalization.ID) {
            return new UnitLocalization();
        }
        if (clusterId == PowerSourceConfiguration.ID) {
            return new PowerSourceConfiguration();
        }
        if (clusterId == PowerSource.ID) {
            return new PowerSource();
        }
        if (clusterId == GeneralCommissioning.ID) {
            return new GeneralCommissioning();
        }
        if (clusterId == NetworkCommissioning.ID) {
            return new NetworkCommissioning();
        }
        if (clusterId == DiagnosticLogs.ID) {
            return new DiagnosticLogs();
        }
        if (clusterId == GeneralDiagnostics.ID) {
            return new GeneralDiagnostics();
        }
        if (clusterId == SoftwareDiagnostics.ID) {
            return new SoftwareDiagnostics();
        }
        if (clusterId == ThreadNetworkDiagnostics.ID) {
            return new ThreadNetworkDiagnostics();
        }
        if (clusterId == WiFiNetworkDiagnostics.ID) {
            return new WiFiNetworkDiagnostics();
        }
        if (clusterId == EthernetNetworkDiagnostics.ID) {
            return new EthernetNetworkDiagnostics();
        }
        if (clusterId == TimeSynchronization.ID) {
            return new TimeSynchronization();
        }
        if (clusterId == BridgedDeviceBasicInformation.ID) {
            return new BridgedDeviceBasicInformation();
        }
        if (clusterId == Switch.ID) {
            return new Switch();
        }
        if (clusterId == AdministratorCommissioning.ID) {
            return new AdministratorCommissioning();
        }
        if (clusterId == OperationalCredentials.ID) {
            return new OperationalCredentials();
        }
        if (clusterId == GroupKeyManagement.ID) {
            return new GroupKeyManagement();
        }
        if (clusterId == FixedLabel.ID) {
            return new FixedLabel();
        }
        if (clusterId == UserLabel.ID) {
            return new UserLabel();
        }
        if (clusterId == ProxyConfiguration.ID) {
            return new ProxyConfiguration();
        }
        if (clusterId == ProxyDiscovery.ID) {
            return new ProxyDiscovery();
        }
        if (clusterId == ProxyValid.ID) {
            return new ProxyValid();
        }
        if (clusterId == BooleanState.ID) {
            return new BooleanState();
        }
        if (clusterId == IcdManagement.ID) {
            return new IcdManagement();
        }
        if (clusterId == Timer.ID) {
            return new Timer();
        }
        if (clusterId == OvenCavityOperationalState.ID) {
            return new OvenCavityOperationalState();
        }
        if (clusterId == OvenMode.ID) {
            return new OvenMode();
        }
        if (clusterId == LaundryDryerControls.ID) {
            return new LaundryDryerControls();
        }
        if (clusterId == ModeSelect.ID) {
            return new ModeSelect();
        }
        if (clusterId == LaundryWasherMode.ID) {
            return new LaundryWasherMode();
        }
        if (clusterId == RefrigeratorAndTemperatureControlledCabinetMode.ID) {
            return new RefrigeratorAndTemperatureControlledCabinetMode();
        }
        if (clusterId == LaundryWasherControls.ID) {
            return new LaundryWasherControls();
        }
        if (clusterId == RvcRunMode.ID) {
            return new RvcRunMode();
        }
        if (clusterId == RvcCleanMode.ID) {
            return new RvcCleanMode();
        }
        if (clusterId == TemperatureControl.ID) {
            return new TemperatureControl();
        }
        if (clusterId == RefrigeratorAlarm.ID) {
            return new RefrigeratorAlarm();
        }
        if (clusterId == DishwasherMode.ID) {
            return new DishwasherMode();
        }
        if (clusterId == AirQuality.ID) {
            return new AirQuality();
        }
        if (clusterId == SmokeCoAlarm.ID) {
            return new SmokeCoAlarm();
        }
        if (clusterId == DishwasherAlarm.ID) {
            return new DishwasherAlarm();
        }
        if (clusterId == MicrowaveOvenMode.ID) {
            return new MicrowaveOvenMode();
        }
        if (clusterId == MicrowaveOvenControl.ID) {
            return new MicrowaveOvenControl();
        }
        if (clusterId == OperationalState.ID) {
            return new OperationalState();
        }
        if (clusterId == RvcOperationalState.ID) {
            return new RvcOperationalState();
        }
        if (clusterId == ScenesManagement.ID) {
            return new ScenesManagement();
        }
        if (clusterId == HepaFilterMonitoring.ID) {
            return new HepaFilterMonitoring();
        }
        if (clusterId == ActivatedCarbonFilterMonitoring.ID) {
            return new ActivatedCarbonFilterMonitoring();
        }
        if (clusterId == BooleanStateConfiguration.ID) {
            return new BooleanStateConfiguration();
        }
        if (clusterId == ValveConfigurationAndControl.ID) {
            return new ValveConfigurationAndControl();
        }
        if (clusterId == ElectricalPowerMeasurement.ID) {
            return new ElectricalPowerMeasurement();
        }
        if (clusterId == ElectricalEnergyMeasurement.ID) {
            return new ElectricalEnergyMeasurement();
        }
        if (clusterId == WaterHeaterManagement.ID) {
            return new WaterHeaterManagement();
        }
        if (clusterId == CommodityPrice.ID) {
            return new CommodityPrice();
        }
        if (clusterId == Messages.ID) {
            return new Messages();
        }
        if (clusterId == DeviceEnergyManagement.ID) {
            return new DeviceEnergyManagement();
        }
        if (clusterId == EnergyEvse.ID) {
            return new EnergyEvse();
        }
        if (clusterId == EnergyPreference.ID) {
            return new EnergyPreference();
        }
        if (clusterId == PowerTopology.ID) {
            return new PowerTopology();
        }
        if (clusterId == EnergyEvseMode.ID) {
            return new EnergyEvseMode();
        }
        if (clusterId == WaterHeaterMode.ID) {
            return new WaterHeaterMode();
        }
        if (clusterId == DeviceEnergyManagementMode.ID) {
            return new DeviceEnergyManagementMode();
        }
        if (clusterId == ElectricalGridConditions.ID) {
            return new ElectricalGridConditions();
        }
        if (clusterId == DoorLock.ID) {
            return new DoorLock();
        }
        if (clusterId == WindowCovering.ID) {
            return new WindowCovering();
        }
        if (clusterId == ClosureControl.ID) {
            return new ClosureControl();
        }
        if (clusterId == ClosureDimension.ID) {
            return new ClosureDimension();
        }
        if (clusterId == ServiceArea.ID) {
            return new ServiceArea();
        }
        if (clusterId == PumpConfigurationAndControl.ID) {
            return new PumpConfigurationAndControl();
        }
        if (clusterId == Thermostat.ID) {
            return new Thermostat();
        }
        if (clusterId == FanControl.ID) {
            return new FanControl();
        }
        if (clusterId == ThermostatUserInterfaceConfiguration.ID) {
            return new ThermostatUserInterfaceConfiguration();
        }
        if (clusterId == ColorControl.ID) {
            return new ColorControl();
        }
        if (clusterId == BallastConfiguration.ID) {
            return new BallastConfiguration();
        }
        if (clusterId == IlluminanceMeasurement.ID) {
            return new IlluminanceMeasurement();
        }
        if (clusterId == TemperatureMeasurement.ID) {
            return new TemperatureMeasurement();
        }
        if (clusterId == PressureMeasurement.ID) {
            return new PressureMeasurement();
        }
        if (clusterId == FlowMeasurement.ID) {
            return new FlowMeasurement();
        }
        if (clusterId == RelativeHumidityMeasurement.ID) {
            return new RelativeHumidityMeasurement();
        }
        if (clusterId == OccupancySensing.ID) {
            return new OccupancySensing();
        }
        if (clusterId == CarbonMonoxideConcentrationMeasurement.ID) {
            return new CarbonMonoxideConcentrationMeasurement();
        }
        if (clusterId == CarbonDioxideConcentrationMeasurement.ID) {
            return new CarbonDioxideConcentrationMeasurement();
        }
        if (clusterId == NitrogenDioxideConcentrationMeasurement.ID) {
            return new NitrogenDioxideConcentrationMeasurement();
        }
        if (clusterId == OzoneConcentrationMeasurement.ID) {
            return new OzoneConcentrationMeasurement();
        }
        if (clusterId == Pm25ConcentrationMeasurement.ID) {
            return new Pm25ConcentrationMeasurement();
        }
        if (clusterId == FormaldehydeConcentrationMeasurement.ID) {
            return new FormaldehydeConcentrationMeasurement();
        }
        if (clusterId == Pm1ConcentrationMeasurement.ID) {
            return new Pm1ConcentrationMeasurement();
        }
        if (clusterId == Pm10ConcentrationMeasurement.ID) {
            return new Pm10ConcentrationMeasurement();
        }
        if (clusterId == TotalVolatileOrganicCompoundsConcentrationMeasurement.ID) {
            return new TotalVolatileOrganicCompoundsConcentrationMeasurement();
        }
        if (clusterId == RadonConcentrationMeasurement.ID) {
            return new RadonConcentrationMeasurement();
        }
        if (clusterId == SoilMeasurement.ID) {
            return new SoilMeasurement();
        }
        if (clusterId == WiFiNetworkManagement.ID) {
            return new WiFiNetworkManagement();
        }
        if (clusterId == ThreadBorderRouterManagement.ID) {
            return new ThreadBorderRouterManagement();
        }
        if (clusterId == ThreadNetworkDirectory.ID) {
            return new ThreadNetworkDirectory();
        }
        if (clusterId == WakeOnLan.ID) {
            return new WakeOnLan();
        }
        if (clusterId == Channel.ID) {
            return new Channel();
        }
        if (clusterId == TargetNavigator.ID) {
            return new TargetNavigator();
        }
        if (clusterId == MediaPlayback.ID) {
            return new MediaPlayback();
        }
        if (clusterId == MediaInput.ID) {
            return new MediaInput();
        }
        if (clusterId == LowPower.ID) {
            return new LowPower();
        }
        if (clusterId == KeypadInput.ID) {
            return new KeypadInput();
        }
        if (clusterId == ContentLauncher.ID) {
            return new ContentLauncher();
        }
        if (clusterId == AudioOutput.ID) {
            return new AudioOutput();
        }
        if (clusterId == ApplicationLauncher.ID) {
            return new ApplicationLauncher();
        }
        if (clusterId == ApplicationBasic.ID) {
            return new ApplicationBasic();
        }
        if (clusterId == AccountLogin.ID) {
            return new AccountLogin();
        }
        if (clusterId == ContentControl.ID) {
            return new ContentControl();
        }
        if (clusterId == ContentAppObserver.ID) {
            return new ContentAppObserver();
        }
        if (clusterId == ZoneManagement.ID) {
            return new ZoneManagement();
        }
        if (clusterId == CameraAvStreamManagement.ID) {
            return new CameraAvStreamManagement();
        }
        if (clusterId == CameraAvSettingsUserLevelManagement.ID) {
            return new CameraAvSettingsUserLevelManagement();
        }
        if (clusterId == WebRTCTransportProvider.ID) {
            return new WebRTCTransportProvider();
        }
        if (clusterId == WebRTCTransportRequestor.ID) {
            return new WebRTCTransportRequestor();
        }
        if (clusterId == PushAvStreamTransport.ID) {
            return new PushAvStreamTransport();
        }
        if (clusterId == Chime.ID) {
            return new Chime();
        }
        if (clusterId == CommodityTariff.ID) {
            return new CommodityTariff();
        }
        if (clusterId == EcosystemInformation.ID) {
            return new EcosystemInformation();
        }
        if (clusterId == CommissionerControl.ID) {
            return new CommissionerControl();
        }
        if (clusterId == JointFabricDatastore.ID) {
            return new JointFabricDatastore();
        }
        if (clusterId == JointFabricAdministrator.ID) {
            return new JointFabricAdministrator();
        }
        if (clusterId == TlsCertificateManagement.ID) {
            return new TlsCertificateManagement();
        }
        if (clusterId == TlsClientManagement.ID) {
            return new TlsClientManagement();
        }
        if (clusterId == MeterIdentification.ID) {
            return new MeterIdentification();
        }
        if (clusterId == CommodityMetering.ID) {
            return new CommodityMetering();
        }
        if (clusterId == UnitTesting.ID) {
            return new UnitTesting();
        }
        if (clusterId == FaultInjection.ID) {
            return new FaultInjection();
        }
        if (clusterId == SampleMei.ID) {
            return new SampleMei();
        }return null;
    }
    public static class Identify implements BaseCluster {
        public static final long ID = 3L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            IdentifyTime(0L),
            IdentifyType(1L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            Identify(0L),
            TriggerEffect(64L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum IdentifyCommandField {IdentifyTime(0),;
                    private final int id;
                    IdentifyCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static IdentifyCommandField value(int id) throws NoSuchFieldError {
                        for (IdentifyCommandField field : IdentifyCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum TriggerEffectCommandField {EffectIdentifier(0),EffectVariant(1),;
                    private final int id;
                    TriggerEffectCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static TriggerEffectCommandField value(int id) throws NoSuchFieldError {
                        for (TriggerEffectCommandField field : TriggerEffectCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class Groups implements BaseCluster {
        public static final long ID = 4L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            NameSupport(0L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            AddGroup(0L),
            ViewGroup(1L),
            GetGroupMembership(2L),
            RemoveGroup(3L),
            RemoveAllGroups(4L),
            AddGroupIfIdentifying(5L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum AddGroupCommandField {GroupID(0),GroupName(1),;
                    private final int id;
                    AddGroupCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static AddGroupCommandField value(int id) throws NoSuchFieldError {
                        for (AddGroupCommandField field : AddGroupCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum ViewGroupCommandField {GroupID(0),;
                    private final int id;
                    ViewGroupCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ViewGroupCommandField value(int id) throws NoSuchFieldError {
                        for (ViewGroupCommandField field : ViewGroupCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum GetGroupMembershipCommandField {GroupList(0),;
                    private final int id;
                    GetGroupMembershipCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static GetGroupMembershipCommandField value(int id) throws NoSuchFieldError {
                        for (GetGroupMembershipCommandField field : GetGroupMembershipCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum RemoveGroupCommandField {GroupID(0),;
                    private final int id;
                    RemoveGroupCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static RemoveGroupCommandField value(int id) throws NoSuchFieldError {
                        for (RemoveGroupCommandField field : RemoveGroupCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum AddGroupIfIdentifyingCommandField {GroupID(0),GroupName(1),;
                    private final int id;
                    AddGroupIfIdentifyingCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static AddGroupIfIdentifyingCommandField value(int id) throws NoSuchFieldError {
                        for (AddGroupIfIdentifyingCommandField field : AddGroupIfIdentifyingCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class OnOff implements BaseCluster {
        public static final long ID = 6L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            OnOff(0L),
            GlobalSceneControl(16384L),
            OnTime(16385L),
            OffWaitTime(16386L),
            StartUpOnOff(16387L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            Off(0L),
            On(1L),
            Toggle(2L),
            OffWithEffect(64L),
            OnWithRecallGlobalScene(65L),
            OnWithTimedOff(66L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum OffWithEffectCommandField {EffectIdentifier(0),EffectVariant(1),;
                    private final int id;
                    OffWithEffectCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static OffWithEffectCommandField value(int id) throws NoSuchFieldError {
                        for (OffWithEffectCommandField field : OffWithEffectCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum OnWithTimedOffCommandField {OnOffControl(0),OnTime(1),OffWaitTime(2),;
                    private final int id;
                    OnWithTimedOffCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static OnWithTimedOffCommandField value(int id) throws NoSuchFieldError {
                        for (OnWithTimedOffCommandField field : OnWithTimedOffCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class LevelControl implements BaseCluster {
        public static final long ID = 8L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            CurrentLevel(0L),
            RemainingTime(1L),
            MinLevel(2L),
            MaxLevel(3L),
            CurrentFrequency(4L),
            MinFrequency(5L),
            MaxFrequency(6L),
            Options(15L),
            OnOffTransitionTime(16L),
            OnLevel(17L),
            OnTransitionTime(18L),
            OffTransitionTime(19L),
            DefaultMoveRate(20L),
            StartUpCurrentLevel(16384L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            MoveToLevel(0L),
            Move(1L),
            Step(2L),
            Stop(3L),
            MoveToLevelWithOnOff(4L),
            MoveWithOnOff(5L),
            StepWithOnOff(6L),
            StopWithOnOff(7L),
            MoveToClosestFrequency(8L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum MoveToLevelCommandField {Level(0),TransitionTime(1),OptionsMask(2),OptionsOverride(3),;
                    private final int id;
                    MoveToLevelCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static MoveToLevelCommandField value(int id) throws NoSuchFieldError {
                        for (MoveToLevelCommandField field : MoveToLevelCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum MoveCommandField {MoveMode(0),Rate(1),OptionsMask(2),OptionsOverride(3),;
                    private final int id;
                    MoveCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static MoveCommandField value(int id) throws NoSuchFieldError {
                        for (MoveCommandField field : MoveCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum StepCommandField {StepMode(0),StepSize(1),TransitionTime(2),OptionsMask(3),OptionsOverride(4),;
                    private final int id;
                    StepCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static StepCommandField value(int id) throws NoSuchFieldError {
                        for (StepCommandField field : StepCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum StopCommandField {OptionsMask(0),OptionsOverride(1),;
                    private final int id;
                    StopCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static StopCommandField value(int id) throws NoSuchFieldError {
                        for (StopCommandField field : StopCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum MoveToLevelWithOnOffCommandField {Level(0),TransitionTime(1),OptionsMask(2),OptionsOverride(3),;
                    private final int id;
                    MoveToLevelWithOnOffCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static MoveToLevelWithOnOffCommandField value(int id) throws NoSuchFieldError {
                        for (MoveToLevelWithOnOffCommandField field : MoveToLevelWithOnOffCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum MoveWithOnOffCommandField {MoveMode(0),Rate(1),OptionsMask(2),OptionsOverride(3),;
                    private final int id;
                    MoveWithOnOffCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static MoveWithOnOffCommandField value(int id) throws NoSuchFieldError {
                        for (MoveWithOnOffCommandField field : MoveWithOnOffCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum StepWithOnOffCommandField {StepMode(0),StepSize(1),TransitionTime(2),OptionsMask(3),OptionsOverride(4),;
                    private final int id;
                    StepWithOnOffCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static StepWithOnOffCommandField value(int id) throws NoSuchFieldError {
                        for (StepWithOnOffCommandField field : StepWithOnOffCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum StopWithOnOffCommandField {OptionsMask(0),OptionsOverride(1),;
                    private final int id;
                    StopWithOnOffCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static StopWithOnOffCommandField value(int id) throws NoSuchFieldError {
                        for (StopWithOnOffCommandField field : StopWithOnOffCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum MoveToClosestFrequencyCommandField {Frequency(0),;
                    private final int id;
                    MoveToClosestFrequencyCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static MoveToClosestFrequencyCommandField value(int id) throws NoSuchFieldError {
                        for (MoveToClosestFrequencyCommandField field : MoveToClosestFrequencyCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class PulseWidthModulation implements BaseCluster {
        public static final long ID = 28L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class Descriptor implements BaseCluster {
        public static final long ID = 29L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            DeviceTypeList(0L),
            ServerList(1L),
            ClientList(2L),
            PartsList(3L),
            TagList(4L),
            EndpointUniqueID(5L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class Binding implements BaseCluster {
        public static final long ID = 30L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            Binding(0L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class AccessControl implements BaseCluster {
        public static final long ID = 31L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            Acl(0L),
            Extension(1L),
            SubjectsPerAccessControlEntry(2L),
            TargetsPerAccessControlEntry(3L),
            AccessControlEntriesPerFabric(4L),
            CommissioningARL(5L),
            Arl(6L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            AccessControlEntryChanged(0L),
            AccessControlExtensionChanged(1L),
            FabricRestrictionReviewUpdate(2L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            ReviewFabricRestrictions(0L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum ReviewFabricRestrictionsCommandField {Arl(0),;
                    private final int id;
                    ReviewFabricRestrictionsCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ReviewFabricRestrictionsCommandField value(int id) throws NoSuchFieldError {
                        for (ReviewFabricRestrictionsCommandField field : ReviewFabricRestrictionsCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class Actions implements BaseCluster {
        public static final long ID = 37L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            ActionList(0L),
            EndpointLists(1L),
            SetupURL(2L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            StateChanged(0L),
            ActionFailed(1L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            InstantAction(0L),
            InstantActionWithTransition(1L),
            StartAction(2L),
            StartActionWithDuration(3L),
            StopAction(4L),
            PauseAction(5L),
            PauseActionWithDuration(6L),
            ResumeAction(7L),
            EnableAction(8L),
            EnableActionWithDuration(9L),
            DisableAction(10L),
            DisableActionWithDuration(11L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum InstantActionCommandField {ActionID(0),InvokeID(1),;
                    private final int id;
                    InstantActionCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static InstantActionCommandField value(int id) throws NoSuchFieldError {
                        for (InstantActionCommandField field : InstantActionCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum InstantActionWithTransitionCommandField {ActionID(0),InvokeID(1),TransitionTime(2),;
                    private final int id;
                    InstantActionWithTransitionCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static InstantActionWithTransitionCommandField value(int id) throws NoSuchFieldError {
                        for (InstantActionWithTransitionCommandField field : InstantActionWithTransitionCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum StartActionCommandField {ActionID(0),InvokeID(1),;
                    private final int id;
                    StartActionCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static StartActionCommandField value(int id) throws NoSuchFieldError {
                        for (StartActionCommandField field : StartActionCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum StartActionWithDurationCommandField {ActionID(0),InvokeID(1),Duration(2),;
                    private final int id;
                    StartActionWithDurationCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static StartActionWithDurationCommandField value(int id) throws NoSuchFieldError {
                        for (StartActionWithDurationCommandField field : StartActionWithDurationCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum StopActionCommandField {ActionID(0),InvokeID(1),;
                    private final int id;
                    StopActionCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static StopActionCommandField value(int id) throws NoSuchFieldError {
                        for (StopActionCommandField field : StopActionCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum PauseActionCommandField {ActionID(0),InvokeID(1),;
                    private final int id;
                    PauseActionCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static PauseActionCommandField value(int id) throws NoSuchFieldError {
                        for (PauseActionCommandField field : PauseActionCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum PauseActionWithDurationCommandField {ActionID(0),InvokeID(1),Duration(2),;
                    private final int id;
                    PauseActionWithDurationCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static PauseActionWithDurationCommandField value(int id) throws NoSuchFieldError {
                        for (PauseActionWithDurationCommandField field : PauseActionWithDurationCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum ResumeActionCommandField {ActionID(0),InvokeID(1),;
                    private final int id;
                    ResumeActionCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ResumeActionCommandField value(int id) throws NoSuchFieldError {
                        for (ResumeActionCommandField field : ResumeActionCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum EnableActionCommandField {ActionID(0),InvokeID(1),;
                    private final int id;
                    EnableActionCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static EnableActionCommandField value(int id) throws NoSuchFieldError {
                        for (EnableActionCommandField field : EnableActionCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum EnableActionWithDurationCommandField {ActionID(0),InvokeID(1),Duration(2),;
                    private final int id;
                    EnableActionWithDurationCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static EnableActionWithDurationCommandField value(int id) throws NoSuchFieldError {
                        for (EnableActionWithDurationCommandField field : EnableActionWithDurationCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum DisableActionCommandField {ActionID(0),InvokeID(1),;
                    private final int id;
                    DisableActionCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static DisableActionCommandField value(int id) throws NoSuchFieldError {
                        for (DisableActionCommandField field : DisableActionCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum DisableActionWithDurationCommandField {ActionID(0),InvokeID(1),Duration(2),;
                    private final int id;
                    DisableActionWithDurationCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static DisableActionWithDurationCommandField value(int id) throws NoSuchFieldError {
                        for (DisableActionWithDurationCommandField field : DisableActionWithDurationCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class BasicInformation implements BaseCluster {
        public static final long ID = 40L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            DataModelRevision(0L),
            VendorName(1L),
            VendorID(2L),
            ProductName(3L),
            ProductID(4L),
            NodeLabel(5L),
            Location(6L),
            HardwareVersion(7L),
            HardwareVersionString(8L),
            SoftwareVersion(9L),
            SoftwareVersionString(10L),
            ManufacturingDate(11L),
            PartNumber(12L),
            ProductURL(13L),
            ProductLabel(14L),
            SerialNumber(15L),
            LocalConfigDisabled(16L),
            Reachable(17L),
            UniqueID(18L),
            CapabilityMinima(19L),
            ProductAppearance(20L),
            SpecificationVersion(21L),
            MaxPathsPerInvoke(22L),
            ConfigurationVersion(24L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            StartUp(0L),
            ShutDown(1L),
            Leave(2L),
            ReachableChanged(3L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            MfgSpecificPing(0L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class OtaSoftwareUpdateProvider implements BaseCluster {
        public static final long ID = 41L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            QueryImage(0L),
            ApplyUpdateRequest(2L),
            NotifyUpdateApplied(4L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum QueryImageCommandField {VendorID(0),ProductID(1),SoftwareVersion(2),ProtocolsSupported(3),HardwareVersion(4),Location(5),RequestorCanConsent(6),MetadataForProvider(7),;
                    private final int id;
                    QueryImageCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static QueryImageCommandField value(int id) throws NoSuchFieldError {
                        for (QueryImageCommandField field : QueryImageCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum ApplyUpdateRequestCommandField {UpdateToken(0),NewVersion(1),;
                    private final int id;
                    ApplyUpdateRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ApplyUpdateRequestCommandField value(int id) throws NoSuchFieldError {
                        for (ApplyUpdateRequestCommandField field : ApplyUpdateRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum NotifyUpdateAppliedCommandField {UpdateToken(0),SoftwareVersion(1),;
                    private final int id;
                    NotifyUpdateAppliedCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static NotifyUpdateAppliedCommandField value(int id) throws NoSuchFieldError {
                        for (NotifyUpdateAppliedCommandField field : NotifyUpdateAppliedCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class OtaSoftwareUpdateRequestor implements BaseCluster {
        public static final long ID = 42L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            DefaultOTAProviders(0L),
            UpdatePossible(1L),
            UpdateState(2L),
            UpdateStateProgress(3L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            StateTransition(0L),
            VersionApplied(1L),
            DownloadError(2L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            AnnounceOTAProvider(0L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum AnnounceOTAProviderCommandField {ProviderNodeID(0),VendorID(1),AnnouncementReason(2),MetadataForNode(3),Endpoint(4),;
                    private final int id;
                    AnnounceOTAProviderCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static AnnounceOTAProviderCommandField value(int id) throws NoSuchFieldError {
                        for (AnnounceOTAProviderCommandField field : AnnounceOTAProviderCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class LocalizationConfiguration implements BaseCluster {
        public static final long ID = 43L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            ActiveLocale(0L),
            SupportedLocales(1L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class TimeFormatLocalization implements BaseCluster {
        public static final long ID = 44L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            HourFormat(0L),
            ActiveCalendarType(1L),
            SupportedCalendarTypes(2L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class UnitLocalization implements BaseCluster {
        public static final long ID = 45L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            TemperatureUnit(0L),
            SupportedTemperatureUnits(1L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class PowerSourceConfiguration implements BaseCluster {
        public static final long ID = 46L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            Sources(0L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class PowerSource implements BaseCluster {
        public static final long ID = 47L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            Status(0L),
            Order(1L),
            Description(2L),
            WiredAssessedInputVoltage(3L),
            WiredAssessedInputFrequency(4L),
            WiredCurrentType(5L),
            WiredAssessedCurrent(6L),
            WiredNominalVoltage(7L),
            WiredMaximumCurrent(8L),
            WiredPresent(9L),
            ActiveWiredFaults(10L),
            BatVoltage(11L),
            BatPercentRemaining(12L),
            BatTimeRemaining(13L),
            BatChargeLevel(14L),
            BatReplacementNeeded(15L),
            BatReplaceability(16L),
            BatPresent(17L),
            ActiveBatFaults(18L),
            BatReplacementDescription(19L),
            BatCommonDesignation(20L),
            BatANSIDesignation(21L),
            BatIECDesignation(22L),
            BatApprovedChemistry(23L),
            BatCapacity(24L),
            BatQuantity(25L),
            BatChargeState(26L),
            BatTimeToFullCharge(27L),
            BatFunctionalWhileCharging(28L),
            BatChargingCurrent(29L),
            ActiveBatChargeFaults(30L),
            EndpointList(31L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            WiredFaultChange(0L),
            BatFaultChange(1L),
            BatChargeFaultChange(2L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class GeneralCommissioning implements BaseCluster {
        public static final long ID = 48L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            Breadcrumb(0L),
            BasicCommissioningInfo(1L),
            RegulatoryConfig(2L),
            LocationCapability(3L),
            SupportsConcurrentConnection(4L),
            TCAcceptedVersion(5L),
            TCMinRequiredVersion(6L),
            TCAcknowledgements(7L),
            TCAcknowledgementsRequired(8L),
            TCUpdateDeadline(9L),
            RecoveryIdentifier(10L),
            NetworkRecoveryReason(11L),
            IsCommissioningWithoutPower(12L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            ArmFailSafe(0L),
            SetRegulatoryConfig(2L),
            CommissioningComplete(4L),
            SetTCAcknowledgements(6L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum ArmFailSafeCommandField {ExpiryLengthSeconds(0),Breadcrumb(1),;
                    private final int id;
                    ArmFailSafeCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ArmFailSafeCommandField value(int id) throws NoSuchFieldError {
                        for (ArmFailSafeCommandField field : ArmFailSafeCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum SetRegulatoryConfigCommandField {NewRegulatoryConfig(0),CountryCode(1),Breadcrumb(2),;
                    private final int id;
                    SetRegulatoryConfigCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SetRegulatoryConfigCommandField value(int id) throws NoSuchFieldError {
                        for (SetRegulatoryConfigCommandField field : SetRegulatoryConfigCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum SetTCAcknowledgementsCommandField {TCVersion(0),TCUserResponse(1),;
                    private final int id;
                    SetTCAcknowledgementsCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SetTCAcknowledgementsCommandField value(int id) throws NoSuchFieldError {
                        for (SetTCAcknowledgementsCommandField field : SetTCAcknowledgementsCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class NetworkCommissioning implements BaseCluster {
        public static final long ID = 49L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            MaxNetworks(0L),
            Networks(1L),
            ScanMaxTimeSeconds(2L),
            ConnectMaxTimeSeconds(3L),
            InterfaceEnabled(4L),
            LastNetworkingStatus(5L),
            LastNetworkID(6L),
            LastConnectErrorValue(7L),
            SupportedWiFiBands(8L),
            SupportedThreadFeatures(9L),
            ThreadVersion(10L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            ScanNetworks(0L),
            AddOrUpdateWiFiNetwork(2L),
            AddOrUpdateThreadNetwork(3L),
            RemoveNetwork(4L),
            ConnectNetwork(6L),
            ReorderNetwork(8L),
            QueryIdentity(9L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum ScanNetworksCommandField {Ssid(0),Breadcrumb(1),;
                    private final int id;
                    ScanNetworksCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ScanNetworksCommandField value(int id) throws NoSuchFieldError {
                        for (ScanNetworksCommandField field : ScanNetworksCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum AddOrUpdateWiFiNetworkCommandField {Ssid(0),Credentials(1),Breadcrumb(2),NetworkIdentity(3),ClientIdentifier(4),PossessionNonce(5),;
                    private final int id;
                    AddOrUpdateWiFiNetworkCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static AddOrUpdateWiFiNetworkCommandField value(int id) throws NoSuchFieldError {
                        for (AddOrUpdateWiFiNetworkCommandField field : AddOrUpdateWiFiNetworkCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum AddOrUpdateThreadNetworkCommandField {OperationalDataset(0),Breadcrumb(1),;
                    private final int id;
                    AddOrUpdateThreadNetworkCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static AddOrUpdateThreadNetworkCommandField value(int id) throws NoSuchFieldError {
                        for (AddOrUpdateThreadNetworkCommandField field : AddOrUpdateThreadNetworkCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum RemoveNetworkCommandField {NetworkID(0),Breadcrumb(1),;
                    private final int id;
                    RemoveNetworkCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static RemoveNetworkCommandField value(int id) throws NoSuchFieldError {
                        for (RemoveNetworkCommandField field : RemoveNetworkCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum ConnectNetworkCommandField {NetworkID(0),Breadcrumb(1),;
                    private final int id;
                    ConnectNetworkCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ConnectNetworkCommandField value(int id) throws NoSuchFieldError {
                        for (ConnectNetworkCommandField field : ConnectNetworkCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum ReorderNetworkCommandField {NetworkID(0),NetworkIndex(1),Breadcrumb(2),;
                    private final int id;
                    ReorderNetworkCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ReorderNetworkCommandField value(int id) throws NoSuchFieldError {
                        for (ReorderNetworkCommandField field : ReorderNetworkCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum QueryIdentityCommandField {KeyIdentifier(0),PossessionNonce(1),;
                    private final int id;
                    QueryIdentityCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static QueryIdentityCommandField value(int id) throws NoSuchFieldError {
                        for (QueryIdentityCommandField field : QueryIdentityCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class DiagnosticLogs implements BaseCluster {
        public static final long ID = 50L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            RetrieveLogsRequest(0L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum RetrieveLogsRequestCommandField {Intent(0),RequestedProtocol(1),TransferFileDesignator(2),;
                    private final int id;
                    RetrieveLogsRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static RetrieveLogsRequestCommandField value(int id) throws NoSuchFieldError {
                        for (RetrieveLogsRequestCommandField field : RetrieveLogsRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class GeneralDiagnostics implements BaseCluster {
        public static final long ID = 51L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            NetworkInterfaces(0L),
            RebootCount(1L),
            UpTime(2L),
            TotalOperationalHours(3L),
            BootReason(4L),
            ActiveHardwareFaults(5L),
            ActiveRadioFaults(6L),
            ActiveNetworkFaults(7L),
            TestEventTriggersEnabled(8L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            HardwareFaultChange(0L),
            RadioFaultChange(1L),
            NetworkFaultChange(2L),
            BootReason(3L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            TestEventTrigger(0L),
            TimeSnapshot(1L),
            PayloadTestRequest(3L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum TestEventTriggerCommandField {EnableKey(0),EventTrigger(1),;
                    private final int id;
                    TestEventTriggerCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static TestEventTriggerCommandField value(int id) throws NoSuchFieldError {
                        for (TestEventTriggerCommandField field : TestEventTriggerCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum PayloadTestRequestCommandField {EnableKey(0),Value(1),Count(2),;
                    private final int id;
                    PayloadTestRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static PayloadTestRequestCommandField value(int id) throws NoSuchFieldError {
                        for (PayloadTestRequestCommandField field : PayloadTestRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class SoftwareDiagnostics implements BaseCluster {
        public static final long ID = 52L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            ThreadMetrics(0L),
            CurrentHeapFree(1L),
            CurrentHeapUsed(2L),
            CurrentHeapHighWatermark(3L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            SoftwareFault(0L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            ResetWatermarks(0L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class ThreadNetworkDiagnostics implements BaseCluster {
        public static final long ID = 53L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            Channel(0L),
            RoutingRole(1L),
            NetworkName(2L),
            PanId(3L),
            ExtendedPanId(4L),
            MeshLocalPrefix(5L),
            OverrunCount(6L),
            NeighborTable(7L),
            RouteTable(8L),
            PartitionId(9L),
            Weighting(10L),
            DataVersion(11L),
            StableDataVersion(12L),
            LeaderRouterId(13L),
            DetachedRoleCount(14L),
            ChildRoleCount(15L),
            RouterRoleCount(16L),
            LeaderRoleCount(17L),
            AttachAttemptCount(18L),
            PartitionIdChangeCount(19L),
            BetterPartitionAttachAttemptCount(20L),
            ParentChangeCount(21L),
            TxTotalCount(22L),
            TxUnicastCount(23L),
            TxBroadcastCount(24L),
            TxAckRequestedCount(25L),
            TxAckedCount(26L),
            TxNoAckRequestedCount(27L),
            TxDataCount(28L),
            TxDataPollCount(29L),
            TxBeaconCount(30L),
            TxBeaconRequestCount(31L),
            TxOtherCount(32L),
            TxRetryCount(33L),
            TxDirectMaxRetryExpiryCount(34L),
            TxIndirectMaxRetryExpiryCount(35L),
            TxErrCcaCount(36L),
            TxErrAbortCount(37L),
            TxErrBusyChannelCount(38L),
            RxTotalCount(39L),
            RxUnicastCount(40L),
            RxBroadcastCount(41L),
            RxDataCount(42L),
            RxDataPollCount(43L),
            RxBeaconCount(44L),
            RxBeaconRequestCount(45L),
            RxOtherCount(46L),
            RxAddressFilteredCount(47L),
            RxDestAddrFilteredCount(48L),
            RxDuplicatedCount(49L),
            RxErrNoFrameCount(50L),
            RxErrUnknownNeighborCount(51L),
            RxErrInvalidSrcAddrCount(52L),
            RxErrSecCount(53L),
            RxErrFcsCount(54L),
            RxErrOtherCount(55L),
            ActiveTimestamp(56L),
            PendingTimestamp(57L),
            Delay(58L),
            SecurityPolicy(59L),
            ChannelPage0Mask(60L),
            OperationalDatasetComponents(61L),
            ActiveNetworkFaultsList(62L),
            ExtAddress(63L),
            Rloc16(64L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            ConnectionStatus(0L),
            NetworkFaultChange(1L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            ResetCounts(0L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class WiFiNetworkDiagnostics implements BaseCluster {
        public static final long ID = 54L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            Bssid(0L),
            SecurityType(1L),
            WiFiVersion(2L),
            ChannelNumber(3L),
            Rssi(4L),
            BeaconLostCount(5L),
            BeaconRxCount(6L),
            PacketMulticastRxCount(7L),
            PacketMulticastTxCount(8L),
            PacketUnicastRxCount(9L),
            PacketUnicastTxCount(10L),
            CurrentMaxRate(11L),
            OverrunCount(12L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            Disconnection(0L),
            AssociationFailure(1L),
            ConnectionStatus(2L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            ResetCounts(0L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class EthernetNetworkDiagnostics implements BaseCluster {
        public static final long ID = 55L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            PHYRate(0L),
            FullDuplex(1L),
            PacketRxCount(2L),
            PacketTxCount(3L),
            TxErrCount(4L),
            CollisionCount(5L),
            OverrunCount(6L),
            CarrierDetect(7L),
            TimeSinceReset(8L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            ResetCounts(0L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class TimeSynchronization implements BaseCluster {
        public static final long ID = 56L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            UTCTime(0L),
            Granularity(1L),
            TimeSource(2L),
            TrustedTimeSource(3L),
            DefaultNTP(4L),
            TimeZone(5L),
            DSTOffset(6L),
            LocalTime(7L),
            TimeZoneDatabase(8L),
            NTPServerAvailable(9L),
            TimeZoneListMaxSize(10L),
            DSTOffsetListMaxSize(11L),
            SupportsDNSResolve(12L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            DSTTableEmpty(0L),
            DSTStatus(1L),
            TimeZoneStatus(2L),
            TimeFailure(3L),
            MissingTrustedTimeSource(4L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            SetUTCTime(0L),
            SetTrustedTimeSource(1L),
            SetTimeZone(2L),
            SetDSTOffset(4L),
            SetDefaultNTP(5L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum SetUTCTimeCommandField {UTCTime(0),Granularity(1),TimeSource(2),;
                    private final int id;
                    SetUTCTimeCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SetUTCTimeCommandField value(int id) throws NoSuchFieldError {
                        for (SetUTCTimeCommandField field : SetUTCTimeCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum SetTrustedTimeSourceCommandField {TrustedTimeSource(0),;
                    private final int id;
                    SetTrustedTimeSourceCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SetTrustedTimeSourceCommandField value(int id) throws NoSuchFieldError {
                        for (SetTrustedTimeSourceCommandField field : SetTrustedTimeSourceCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum SetTimeZoneCommandField {TimeZone(0),;
                    private final int id;
                    SetTimeZoneCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SetTimeZoneCommandField value(int id) throws NoSuchFieldError {
                        for (SetTimeZoneCommandField field : SetTimeZoneCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum SetDSTOffsetCommandField {DSTOffset(0),;
                    private final int id;
                    SetDSTOffsetCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SetDSTOffsetCommandField value(int id) throws NoSuchFieldError {
                        for (SetDSTOffsetCommandField field : SetDSTOffsetCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum SetDefaultNTPCommandField {DefaultNTP(0),;
                    private final int id;
                    SetDefaultNTPCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SetDefaultNTPCommandField value(int id) throws NoSuchFieldError {
                        for (SetDefaultNTPCommandField field : SetDefaultNTPCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class BridgedDeviceBasicInformation implements BaseCluster {
        public static final long ID = 57L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            VendorName(1L),
            VendorID(2L),
            ProductName(3L),
            ProductID(4L),
            NodeLabel(5L),
            HardwareVersion(7L),
            HardwareVersionString(8L),
            SoftwareVersion(9L),
            SoftwareVersionString(10L),
            ManufacturingDate(11L),
            PartNumber(12L),
            ProductURL(13L),
            ProductLabel(14L),
            SerialNumber(15L),
            Reachable(17L),
            UniqueID(18L),
            ProductAppearance(20L),
            ConfigurationVersion(24L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            StartUp(0L),
            ShutDown(1L),
            Leave(2L),
            ReachableChanged(3L),
            ActiveChanged(128L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            KeepActive(128L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum KeepActiveCommandField {StayActiveDuration(0),TimeoutMs(1),;
                    private final int id;
                    KeepActiveCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static KeepActiveCommandField value(int id) throws NoSuchFieldError {
                        for (KeepActiveCommandField field : KeepActiveCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class Switch implements BaseCluster {
        public static final long ID = 59L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            NumberOfPositions(0L),
            CurrentPosition(1L),
            MultiPressMax(2L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            SwitchLatched(0L),
            InitialPress(1L),
            LongPress(2L),
            ShortRelease(3L),
            LongRelease(4L),
            MultiPressOngoing(5L),
            MultiPressComplete(6L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class AdministratorCommissioning implements BaseCluster {
        public static final long ID = 60L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            WindowStatus(0L),
            AdminFabricIndex(1L),
            AdminVendorId(2L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            OpenCommissioningWindow(0L),
            OpenBasicCommissioningWindow(1L),
            RevokeCommissioning(2L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum OpenCommissioningWindowCommandField {CommissioningTimeout(0),PAKEPasscodeVerifier(1),Discriminator(2),Iterations(3),Salt(4),;
                    private final int id;
                    OpenCommissioningWindowCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static OpenCommissioningWindowCommandField value(int id) throws NoSuchFieldError {
                        for (OpenCommissioningWindowCommandField field : OpenCommissioningWindowCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum OpenBasicCommissioningWindowCommandField {CommissioningTimeout(0),;
                    private final int id;
                    OpenBasicCommissioningWindowCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static OpenBasicCommissioningWindowCommandField value(int id) throws NoSuchFieldError {
                        for (OpenBasicCommissioningWindowCommandField field : OpenBasicCommissioningWindowCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class OperationalCredentials implements BaseCluster {
        public static final long ID = 62L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            NOCs(0L),
            Fabrics(1L),
            SupportedFabrics(2L),
            CommissionedFabrics(3L),
            TrustedRootCertificates(4L),
            CurrentFabricIndex(5L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            AttestationRequest(0L),
            CertificateChainRequest(2L),
            CSRRequest(4L),
            AddNOC(6L),
            UpdateNOC(7L),
            UpdateFabricLabel(9L),
            RemoveFabric(10L),
            AddTrustedRootCertificate(11L),
            SetVIDVerificationStatement(12L),
            SignVIDVerificationRequest(13L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum AttestationRequestCommandField {AttestationNonce(0),;
                    private final int id;
                    AttestationRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static AttestationRequestCommandField value(int id) throws NoSuchFieldError {
                        for (AttestationRequestCommandField field : AttestationRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum CertificateChainRequestCommandField {CertificateType(0),;
                    private final int id;
                    CertificateChainRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static CertificateChainRequestCommandField value(int id) throws NoSuchFieldError {
                        for (CertificateChainRequestCommandField field : CertificateChainRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum CSRRequestCommandField {CSRNonce(0),IsForUpdateNOC(1),;
                    private final int id;
                    CSRRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static CSRRequestCommandField value(int id) throws NoSuchFieldError {
                        for (CSRRequestCommandField field : CSRRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum AddNOCCommandField {NOCValue(0),ICACValue(1),IPKValue(2),CaseAdminSubject(3),AdminVendorId(4),;
                    private final int id;
                    AddNOCCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static AddNOCCommandField value(int id) throws NoSuchFieldError {
                        for (AddNOCCommandField field : AddNOCCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum UpdateNOCCommandField {NOCValue(0),ICACValue(1),;
                    private final int id;
                    UpdateNOCCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static UpdateNOCCommandField value(int id) throws NoSuchFieldError {
                        for (UpdateNOCCommandField field : UpdateNOCCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum UpdateFabricLabelCommandField {Label(0),;
                    private final int id;
                    UpdateFabricLabelCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static UpdateFabricLabelCommandField value(int id) throws NoSuchFieldError {
                        for (UpdateFabricLabelCommandField field : UpdateFabricLabelCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum RemoveFabricCommandField {FabricIndex(0),;
                    private final int id;
                    RemoveFabricCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static RemoveFabricCommandField value(int id) throws NoSuchFieldError {
                        for (RemoveFabricCommandField field : RemoveFabricCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum AddTrustedRootCertificateCommandField {RootCACertificate(0),;
                    private final int id;
                    AddTrustedRootCertificateCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static AddTrustedRootCertificateCommandField value(int id) throws NoSuchFieldError {
                        for (AddTrustedRootCertificateCommandField field : AddTrustedRootCertificateCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum SetVIDVerificationStatementCommandField {VendorID(0),VIDVerificationStatement(1),Vvsc(2),;
                    private final int id;
                    SetVIDVerificationStatementCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SetVIDVerificationStatementCommandField value(int id) throws NoSuchFieldError {
                        for (SetVIDVerificationStatementCommandField field : SetVIDVerificationStatementCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum SignVIDVerificationRequestCommandField {FabricIndex(0),ClientChallenge(1),;
                    private final int id;
                    SignVIDVerificationRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SignVIDVerificationRequestCommandField value(int id) throws NoSuchFieldError {
                        for (SignVIDVerificationRequestCommandField field : SignVIDVerificationRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class GroupKeyManagement implements BaseCluster {
        public static final long ID = 63L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            GroupKeyMap(0L),
            GroupTable(1L),
            MaxGroupsPerFabric(2L),
            MaxGroupKeysPerFabric(3L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            KeySetWrite(0L),
            KeySetRead(1L),
            KeySetRemove(3L),
            KeySetReadAllIndices(4L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum KeySetWriteCommandField {GroupKeySet(0),;
                    private final int id;
                    KeySetWriteCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static KeySetWriteCommandField value(int id) throws NoSuchFieldError {
                        for (KeySetWriteCommandField field : KeySetWriteCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum KeySetReadCommandField {GroupKeySetID(0),;
                    private final int id;
                    KeySetReadCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static KeySetReadCommandField value(int id) throws NoSuchFieldError {
                        for (KeySetReadCommandField field : KeySetReadCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum KeySetRemoveCommandField {GroupKeySetID(0),;
                    private final int id;
                    KeySetRemoveCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static KeySetRemoveCommandField value(int id) throws NoSuchFieldError {
                        for (KeySetRemoveCommandField field : KeySetRemoveCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class FixedLabel implements BaseCluster {
        public static final long ID = 64L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            LabelList(0L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class UserLabel implements BaseCluster {
        public static final long ID = 65L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            LabelList(0L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class ProxyConfiguration implements BaseCluster {
        public static final long ID = 66L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class ProxyDiscovery implements BaseCluster {
        public static final long ID = 67L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class ProxyValid implements BaseCluster {
        public static final long ID = 68L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class BooleanState implements BaseCluster {
        public static final long ID = 69L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            StateValue(0L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            StateChange(0L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class IcdManagement implements BaseCluster {
        public static final long ID = 70L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            IdleModeDuration(0L),
            ActiveModeDuration(1L),
            ActiveModeThreshold(2L),
            RegisteredClients(3L),
            ICDCounter(4L),
            ClientsSupportedPerFabric(5L),
            UserActiveModeTriggerHint(6L),
            UserActiveModeTriggerInstruction(7L),
            OperatingMode(8L),
            MaximumCheckInBackOff(9L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            RegisterClient(0L),
            UnregisterClient(2L),
            StayActiveRequest(3L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum RegisterClientCommandField {CheckInNodeID(0),MonitoredSubject(1),Key(2),VerificationKey(3),ClientType(4),;
                    private final int id;
                    RegisterClientCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static RegisterClientCommandField value(int id) throws NoSuchFieldError {
                        for (RegisterClientCommandField field : RegisterClientCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum UnregisterClientCommandField {CheckInNodeID(0),VerificationKey(1),;
                    private final int id;
                    UnregisterClientCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static UnregisterClientCommandField value(int id) throws NoSuchFieldError {
                        for (UnregisterClientCommandField field : UnregisterClientCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum StayActiveRequestCommandField {StayActiveDuration(0),;
                    private final int id;
                    StayActiveRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static StayActiveRequestCommandField value(int id) throws NoSuchFieldError {
                        for (StayActiveRequestCommandField field : StayActiveRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class Timer implements BaseCluster {
        public static final long ID = 71L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            SetTime(0L),
            TimeRemaining(1L),
            TimerState(2L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            SetTimer(0L),
            ResetTimer(1L),
            AddTime(2L),
            ReduceTime(3L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum SetTimerCommandField {NewTime(0),;
                    private final int id;
                    SetTimerCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SetTimerCommandField value(int id) throws NoSuchFieldError {
                        for (SetTimerCommandField field : SetTimerCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum AddTimeCommandField {AdditionalTime(0),;
                    private final int id;
                    AddTimeCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static AddTimeCommandField value(int id) throws NoSuchFieldError {
                        for (AddTimeCommandField field : AddTimeCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum ReduceTimeCommandField {TimeReduction(0),;
                    private final int id;
                    ReduceTimeCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ReduceTimeCommandField value(int id) throws NoSuchFieldError {
                        for (ReduceTimeCommandField field : ReduceTimeCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class OvenCavityOperationalState implements BaseCluster {
        public static final long ID = 72L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            PhaseList(0L),
            CurrentPhase(1L),
            CountdownTime(2L),
            OperationalStateList(3L),
            OperationalState(4L),
            OperationalError(5L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            OperationalError(0L),
            OperationCompletion(1L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            Stop(1L),
            Start(2L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class OvenMode implements BaseCluster {
        public static final long ID = 73L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            SupportedModes(0L),
            CurrentMode(1L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            ChangeToMode(0L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum ChangeToModeCommandField {NewMode(0),;
                    private final int id;
                    ChangeToModeCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ChangeToModeCommandField value(int id) throws NoSuchFieldError {
                        for (ChangeToModeCommandField field : ChangeToModeCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class LaundryDryerControls implements BaseCluster {
        public static final long ID = 74L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            SupportedDrynessLevels(0L),
            SelectedDrynessLevel(1L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class ModeSelect implements BaseCluster {
        public static final long ID = 80L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            Description(0L),
            StandardNamespace(1L),
            SupportedModes(2L),
            CurrentMode(3L),
            StartUpMode(4L),
            OnMode(5L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            ChangeToMode(0L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum ChangeToModeCommandField {NewMode(0),;
                    private final int id;
                    ChangeToModeCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ChangeToModeCommandField value(int id) throws NoSuchFieldError {
                        for (ChangeToModeCommandField field : ChangeToModeCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class LaundryWasherMode implements BaseCluster {
        public static final long ID = 81L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            SupportedModes(0L),
            CurrentMode(1L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            ChangeToMode(0L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum ChangeToModeCommandField {NewMode(0),;
                    private final int id;
                    ChangeToModeCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ChangeToModeCommandField value(int id) throws NoSuchFieldError {
                        for (ChangeToModeCommandField field : ChangeToModeCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class RefrigeratorAndTemperatureControlledCabinetMode implements BaseCluster {
        public static final long ID = 82L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            SupportedModes(0L),
            CurrentMode(1L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            ChangeToMode(0L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum ChangeToModeCommandField {NewMode(0),;
                    private final int id;
                    ChangeToModeCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ChangeToModeCommandField value(int id) throws NoSuchFieldError {
                        for (ChangeToModeCommandField field : ChangeToModeCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class LaundryWasherControls implements BaseCluster {
        public static final long ID = 83L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            SpinSpeeds(0L),
            SpinSpeedCurrent(1L),
            NumberOfRinses(2L),
            SupportedRinses(3L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class RvcRunMode implements BaseCluster {
        public static final long ID = 84L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            SupportedModes(0L),
            CurrentMode(1L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            ChangeToMode(0L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum ChangeToModeCommandField {NewMode(0),;
                    private final int id;
                    ChangeToModeCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ChangeToModeCommandField value(int id) throws NoSuchFieldError {
                        for (ChangeToModeCommandField field : ChangeToModeCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class RvcCleanMode implements BaseCluster {
        public static final long ID = 85L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            SupportedModes(0L),
            CurrentMode(1L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            ChangeToMode(0L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum ChangeToModeCommandField {NewMode(0),;
                    private final int id;
                    ChangeToModeCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ChangeToModeCommandField value(int id) throws NoSuchFieldError {
                        for (ChangeToModeCommandField field : ChangeToModeCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class TemperatureControl implements BaseCluster {
        public static final long ID = 86L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            TemperatureSetpoint(0L),
            MinTemperature(1L),
            MaxTemperature(2L),
            Step(3L),
            SelectedTemperatureLevel(4L),
            SupportedTemperatureLevels(5L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            SetTemperature(0L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum SetTemperatureCommandField {TargetTemperature(0),TargetTemperatureLevel(1),;
                    private final int id;
                    SetTemperatureCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SetTemperatureCommandField value(int id) throws NoSuchFieldError {
                        for (SetTemperatureCommandField field : SetTemperatureCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class RefrigeratorAlarm implements BaseCluster {
        public static final long ID = 87L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            Mask(0L),
            State(2L),
            Supported(3L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            Notify(0L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class DishwasherMode implements BaseCluster {
        public static final long ID = 89L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            SupportedModes(0L),
            CurrentMode(1L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            ChangeToMode(0L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum ChangeToModeCommandField {NewMode(0),;
                    private final int id;
                    ChangeToModeCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ChangeToModeCommandField value(int id) throws NoSuchFieldError {
                        for (ChangeToModeCommandField field : ChangeToModeCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class AirQuality implements BaseCluster {
        public static final long ID = 91L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            AirQuality(0L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class SmokeCoAlarm implements BaseCluster {
        public static final long ID = 92L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            ExpressedState(0L),
            SmokeState(1L),
            COState(2L),
            BatteryAlert(3L),
            DeviceMuted(4L),
            TestInProgress(5L),
            HardwareFaultAlert(6L),
            EndOfServiceAlert(7L),
            InterconnectSmokeAlarm(8L),
            InterconnectCOAlarm(9L),
            ContaminationState(10L),
            SmokeSensitivityLevel(11L),
            ExpiryDate(12L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            SmokeAlarm(0L),
            COAlarm(1L),
            LowBattery(2L),
            HardwareFault(3L),
            EndOfService(4L),
            SelfTestComplete(5L),
            AlarmMuted(6L),
            MuteEnded(7L),
            InterconnectSmokeAlarm(8L),
            InterconnectCOAlarm(9L),
            AllClear(10L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            SelfTestRequest(0L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class DishwasherAlarm implements BaseCluster {
        public static final long ID = 93L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            Mask(0L),
            Latch(1L),
            State(2L),
            Supported(3L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            Notify(0L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            Reset(0L),
            ModifyEnabledAlarms(1L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum ResetCommandField {Alarms(0),;
                    private final int id;
                    ResetCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ResetCommandField value(int id) throws NoSuchFieldError {
                        for (ResetCommandField field : ResetCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum ModifyEnabledAlarmsCommandField {Mask(0),;
                    private final int id;
                    ModifyEnabledAlarmsCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ModifyEnabledAlarmsCommandField value(int id) throws NoSuchFieldError {
                        for (ModifyEnabledAlarmsCommandField field : ModifyEnabledAlarmsCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class MicrowaveOvenMode implements BaseCluster {
        public static final long ID = 94L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            SupportedModes(0L),
            CurrentMode(1L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class MicrowaveOvenControl implements BaseCluster {
        public static final long ID = 95L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            CookTime(0L),
            MaxCookTime(1L),
            PowerSetting(2L),
            MinPower(3L),
            MaxPower(4L),
            PowerStep(5L),
            SupportedWatts(6L),
            SelectedWattIndex(7L),
            WattRating(8L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            SetCookingParameters(0L),
            AddMoreTime(1L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum SetCookingParametersCommandField {CookMode(0),CookTime(1),PowerSetting(2),WattSettingIndex(3),StartAfterSetting(4),;
                    private final int id;
                    SetCookingParametersCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SetCookingParametersCommandField value(int id) throws NoSuchFieldError {
                        for (SetCookingParametersCommandField field : SetCookingParametersCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum AddMoreTimeCommandField {TimeToAdd(0),;
                    private final int id;
                    AddMoreTimeCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static AddMoreTimeCommandField value(int id) throws NoSuchFieldError {
                        for (AddMoreTimeCommandField field : AddMoreTimeCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class OperationalState implements BaseCluster {
        public static final long ID = 96L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            PhaseList(0L),
            CurrentPhase(1L),
            CountdownTime(2L),
            OperationalStateList(3L),
            OperationalState(4L),
            OperationalError(5L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            OperationalError(0L),
            OperationCompletion(1L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            Pause(0L),
            Stop(1L),
            Start(2L),
            Resume(3L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class RvcOperationalState implements BaseCluster {
        public static final long ID = 97L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            PhaseList(0L),
            CurrentPhase(1L),
            CountdownTime(2L),
            OperationalStateList(3L),
            OperationalState(4L),
            OperationalError(5L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            OperationalError(0L),
            OperationCompletion(1L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            Pause(0L),
            Resume(3L),
            GoHome(128L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class ScenesManagement implements BaseCluster {
        public static final long ID = 98L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            SceneTableSize(1L),
            FabricSceneInfo(2L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            AddScene(0L),
            ViewScene(1L),
            RemoveScene(2L),
            RemoveAllScenes(3L),
            StoreScene(4L),
            RecallScene(5L),
            GetSceneMembership(6L),
            CopyScene(64L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum AddSceneCommandField {GroupID(0),SceneID(1),TransitionTime(2),SceneName(3),ExtensionFieldSetStructs(4),;
                    private final int id;
                    AddSceneCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static AddSceneCommandField value(int id) throws NoSuchFieldError {
                        for (AddSceneCommandField field : AddSceneCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum ViewSceneCommandField {GroupID(0),SceneID(1),;
                    private final int id;
                    ViewSceneCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ViewSceneCommandField value(int id) throws NoSuchFieldError {
                        for (ViewSceneCommandField field : ViewSceneCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum RemoveSceneCommandField {GroupID(0),SceneID(1),;
                    private final int id;
                    RemoveSceneCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static RemoveSceneCommandField value(int id) throws NoSuchFieldError {
                        for (RemoveSceneCommandField field : RemoveSceneCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum RemoveAllScenesCommandField {GroupID(0),;
                    private final int id;
                    RemoveAllScenesCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static RemoveAllScenesCommandField value(int id) throws NoSuchFieldError {
                        for (RemoveAllScenesCommandField field : RemoveAllScenesCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum StoreSceneCommandField {GroupID(0),SceneID(1),;
                    private final int id;
                    StoreSceneCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static StoreSceneCommandField value(int id) throws NoSuchFieldError {
                        for (StoreSceneCommandField field : StoreSceneCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum RecallSceneCommandField {GroupID(0),SceneID(1),TransitionTime(2),;
                    private final int id;
                    RecallSceneCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static RecallSceneCommandField value(int id) throws NoSuchFieldError {
                        for (RecallSceneCommandField field : RecallSceneCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum GetSceneMembershipCommandField {GroupID(0),;
                    private final int id;
                    GetSceneMembershipCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static GetSceneMembershipCommandField value(int id) throws NoSuchFieldError {
                        for (GetSceneMembershipCommandField field : GetSceneMembershipCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum CopySceneCommandField {Mode(0),GroupIdentifierFrom(1),SceneIdentifierFrom(2),GroupIdentifierTo(3),SceneIdentifierTo(4),;
                    private final int id;
                    CopySceneCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static CopySceneCommandField value(int id) throws NoSuchFieldError {
                        for (CopySceneCommandField field : CopySceneCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class HepaFilterMonitoring implements BaseCluster {
        public static final long ID = 113L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            Condition(0L),
            DegradationDirection(1L),
            ChangeIndication(2L),
            InPlaceIndicator(3L),
            LastChangedTime(4L),
            ReplacementProductList(5L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            ResetCondition(0L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class ActivatedCarbonFilterMonitoring implements BaseCluster {
        public static final long ID = 114L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            Condition(0L),
            DegradationDirection(1L),
            ChangeIndication(2L),
            InPlaceIndicator(3L),
            LastChangedTime(4L),
            ReplacementProductList(5L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            ResetCondition(0L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class BooleanStateConfiguration implements BaseCluster {
        public static final long ID = 128L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            CurrentSensitivityLevel(0L),
            SupportedSensitivityLevels(1L),
            DefaultSensitivityLevel(2L),
            AlarmsActive(3L),
            AlarmsSuppressed(4L),
            AlarmsEnabled(5L),
            AlarmsSupported(6L),
            SensorFault(7L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            AlarmsStateChanged(0L),
            SensorFault(1L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            SuppressAlarm(0L),
            EnableDisableAlarm(1L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum SuppressAlarmCommandField {AlarmsToSuppress(0),;
                    private final int id;
                    SuppressAlarmCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SuppressAlarmCommandField value(int id) throws NoSuchFieldError {
                        for (SuppressAlarmCommandField field : SuppressAlarmCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum EnableDisableAlarmCommandField {AlarmsToEnableDisable(0),;
                    private final int id;
                    EnableDisableAlarmCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static EnableDisableAlarmCommandField value(int id) throws NoSuchFieldError {
                        for (EnableDisableAlarmCommandField field : EnableDisableAlarmCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class ValveConfigurationAndControl implements BaseCluster {
        public static final long ID = 129L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            OpenDuration(0L),
            DefaultOpenDuration(1L),
            AutoCloseTime(2L),
            RemainingDuration(3L),
            CurrentState(4L),
            TargetState(5L),
            CurrentLevel(6L),
            TargetLevel(7L),
            DefaultOpenLevel(8L),
            ValveFault(9L),
            LevelStep(10L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            ValveStateChanged(0L),
            ValveFault(1L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            Open(0L),
            Close(1L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum OpenCommandField {OpenDuration(0),TargetLevel(1),;
                    private final int id;
                    OpenCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static OpenCommandField value(int id) throws NoSuchFieldError {
                        for (OpenCommandField field : OpenCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class ElectricalPowerMeasurement implements BaseCluster {
        public static final long ID = 144L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            PowerMode(0L),
            NumberOfMeasurementTypes(1L),
            Accuracy(2L),
            Ranges(3L),
            Voltage(4L),
            ActiveCurrent(5L),
            ReactiveCurrent(6L),
            ApparentCurrent(7L),
            ActivePower(8L),
            ReactivePower(9L),
            ApparentPower(10L),
            RMSVoltage(11L),
            RMSCurrent(12L),
            RMSPower(13L),
            Frequency(14L),
            HarmonicCurrents(15L),
            HarmonicPhases(16L),
            PowerFactor(17L),
            NeutralCurrent(18L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            MeasurementPeriodRanges(0L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class ElectricalEnergyMeasurement implements BaseCluster {
        public static final long ID = 145L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            Accuracy(0L),
            CumulativeEnergyImported(1L),
            CumulativeEnergyExported(2L),
            PeriodicEnergyImported(3L),
            PeriodicEnergyExported(4L),
            CumulativeEnergyReset(5L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            CumulativeEnergyMeasured(0L),
            PeriodicEnergyMeasured(1L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class WaterHeaterManagement implements BaseCluster {
        public static final long ID = 148L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            HeaterTypes(0L),
            HeatDemand(1L),
            TankVolume(2L),
            EstimatedHeatRequired(3L),
            TankPercentage(4L),
            BoostState(5L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            BoostStarted(0L),
            BoostEnded(1L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            Boost(0L),
            CancelBoost(1L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum BoostCommandField {BoostInfo(0),;
                    private final int id;
                    BoostCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static BoostCommandField value(int id) throws NoSuchFieldError {
                        for (BoostCommandField field : BoostCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class CommodityPrice implements BaseCluster {
        public static final long ID = 149L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            TariffUnit(0L),
            Currency(1L),
            CurrentPrice(2L),
            PriceForecast(3L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            PriceChange(0L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            GetDetailedPriceRequest(0L),
            GetDetailedForecastRequest(2L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum GetDetailedPriceRequestCommandField {Details(0),;
                    private final int id;
                    GetDetailedPriceRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static GetDetailedPriceRequestCommandField value(int id) throws NoSuchFieldError {
                        for (GetDetailedPriceRequestCommandField field : GetDetailedPriceRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum GetDetailedForecastRequestCommandField {Details(0),;
                    private final int id;
                    GetDetailedForecastRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static GetDetailedForecastRequestCommandField value(int id) throws NoSuchFieldError {
                        for (GetDetailedForecastRequestCommandField field : GetDetailedForecastRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class Messages implements BaseCluster {
        public static final long ID = 151L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            Messages(0L),
            ActiveMessageIDs(1L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            MessageQueued(0L),
            MessagePresented(1L),
            MessageComplete(2L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            PresentMessagesRequest(0L),
            CancelMessagesRequest(1L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum PresentMessagesRequestCommandField {MessageID(0),Priority(1),MessageControl(2),StartTime(3),Duration(4),MessageText(5),Responses(6),;
                    private final int id;
                    PresentMessagesRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static PresentMessagesRequestCommandField value(int id) throws NoSuchFieldError {
                        for (PresentMessagesRequestCommandField field : PresentMessagesRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum CancelMessagesRequestCommandField {MessageIDs(0),;
                    private final int id;
                    CancelMessagesRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static CancelMessagesRequestCommandField value(int id) throws NoSuchFieldError {
                        for (CancelMessagesRequestCommandField field : CancelMessagesRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class DeviceEnergyManagement implements BaseCluster {
        public static final long ID = 152L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            ESAType(0L),
            ESACanGenerate(1L),
            ESAState(2L),
            AbsMinPower(3L),
            AbsMaxPower(4L),
            PowerAdjustmentCapability(5L),
            Forecast(6L),
            OptOutState(7L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            PowerAdjustStart(0L),
            PowerAdjustEnd(1L),
            Paused(2L),
            Resumed(3L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            PowerAdjustRequest(0L),
            CancelPowerAdjustRequest(1L),
            StartTimeAdjustRequest(2L),
            PauseRequest(3L),
            ResumeRequest(4L),
            ModifyForecastRequest(5L),
            RequestConstraintBasedForecast(6L),
            CancelRequest(7L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum PowerAdjustRequestCommandField {Power(0),Duration(1),Cause(2),;
                    private final int id;
                    PowerAdjustRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static PowerAdjustRequestCommandField value(int id) throws NoSuchFieldError {
                        for (PowerAdjustRequestCommandField field : PowerAdjustRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum StartTimeAdjustRequestCommandField {RequestedStartTime(0),Cause(1),;
                    private final int id;
                    StartTimeAdjustRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static StartTimeAdjustRequestCommandField value(int id) throws NoSuchFieldError {
                        for (StartTimeAdjustRequestCommandField field : StartTimeAdjustRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum PauseRequestCommandField {Duration(0),Cause(1),;
                    private final int id;
                    PauseRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static PauseRequestCommandField value(int id) throws NoSuchFieldError {
                        for (PauseRequestCommandField field : PauseRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum ModifyForecastRequestCommandField {ForecastID(0),SlotAdjustments(1),Cause(2),;
                    private final int id;
                    ModifyForecastRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ModifyForecastRequestCommandField value(int id) throws NoSuchFieldError {
                        for (ModifyForecastRequestCommandField field : ModifyForecastRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum RequestConstraintBasedForecastCommandField {Constraints(0),Cause(1),;
                    private final int id;
                    RequestConstraintBasedForecastCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static RequestConstraintBasedForecastCommandField value(int id) throws NoSuchFieldError {
                        for (RequestConstraintBasedForecastCommandField field : RequestConstraintBasedForecastCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class EnergyEvse implements BaseCluster {
        public static final long ID = 153L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            State(0L),
            SupplyState(1L),
            FaultState(2L),
            ChargingEnabledUntil(3L),
            DischargingEnabledUntil(4L),
            CircuitCapacity(5L),
            MinimumChargeCurrent(6L),
            MaximumChargeCurrent(7L),
            MaximumDischargeCurrent(8L),
            UserMaximumChargeCurrent(9L),
            RandomizationDelayWindow(10L),
            NextChargeStartTime(35L),
            NextChargeTargetTime(36L),
            NextChargeRequiredEnergy(37L),
            NextChargeTargetSoC(38L),
            ApproximateEVEfficiency(39L),
            StateOfCharge(48L),
            BatteryCapacity(49L),
            VehicleID(50L),
            SessionID(64L),
            SessionDuration(65L),
            SessionEnergyCharged(66L),
            SessionEnergyDischarged(67L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            EVConnected(0L),
            EVNotDetected(1L),
            EnergyTransferStarted(2L),
            EnergyTransferStopped(3L),
            Fault(4L),
            RFID(5L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            Disable(1L),
            EnableCharging(2L),
            EnableDischarging(3L),
            StartDiagnostics(4L),
            SetTargets(5L),
            GetTargets(6L),
            ClearTargets(7L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum EnableChargingCommandField {ChargingEnabledUntil(0),MinimumChargeCurrent(1),MaximumChargeCurrent(2),;
                    private final int id;
                    EnableChargingCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static EnableChargingCommandField value(int id) throws NoSuchFieldError {
                        for (EnableChargingCommandField field : EnableChargingCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum EnableDischargingCommandField {DischargingEnabledUntil(0),MaximumDischargeCurrent(1),;
                    private final int id;
                    EnableDischargingCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static EnableDischargingCommandField value(int id) throws NoSuchFieldError {
                        for (EnableDischargingCommandField field : EnableDischargingCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum SetTargetsCommandField {ChargingTargetSchedules(0),;
                    private final int id;
                    SetTargetsCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SetTargetsCommandField value(int id) throws NoSuchFieldError {
                        for (SetTargetsCommandField field : SetTargetsCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class EnergyPreference implements BaseCluster {
        public static final long ID = 155L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            EnergyBalances(0L),
            CurrentEnergyBalance(1L),
            EnergyPriorities(2L),
            LowPowerModeSensitivities(3L),
            CurrentLowPowerModeSensitivity(4L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class PowerTopology implements BaseCluster {
        public static final long ID = 156L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            AvailableEndpoints(0L),
            ActiveEndpoints(1L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class EnergyEvseMode implements BaseCluster {
        public static final long ID = 157L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            SupportedModes(0L),
            CurrentMode(1L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            ChangeToMode(0L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum ChangeToModeCommandField {NewMode(0),;
                    private final int id;
                    ChangeToModeCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ChangeToModeCommandField value(int id) throws NoSuchFieldError {
                        for (ChangeToModeCommandField field : ChangeToModeCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class WaterHeaterMode implements BaseCluster {
        public static final long ID = 158L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            SupportedModes(0L),
            CurrentMode(1L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            ChangeToMode(0L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum ChangeToModeCommandField {NewMode(0),;
                    private final int id;
                    ChangeToModeCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ChangeToModeCommandField value(int id) throws NoSuchFieldError {
                        for (ChangeToModeCommandField field : ChangeToModeCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class DeviceEnergyManagementMode implements BaseCluster {
        public static final long ID = 159L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            SupportedModes(0L),
            CurrentMode(1L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            ChangeToMode(0L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum ChangeToModeCommandField {NewMode(0),;
                    private final int id;
                    ChangeToModeCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ChangeToModeCommandField value(int id) throws NoSuchFieldError {
                        for (ChangeToModeCommandField field : ChangeToModeCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class ElectricalGridConditions implements BaseCluster {
        public static final long ID = 160L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            LocalGenerationAvailable(0L),
            CurrentConditions(1L),
            ForecastConditions(2L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            CurrentConditionsChanged(0L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class DoorLock implements BaseCluster {
        public static final long ID = 257L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            LockState(0L),
            LockType(1L),
            ActuatorEnabled(2L),
            DoorState(3L),
            DoorOpenEvents(4L),
            DoorClosedEvents(5L),
            OpenPeriod(6L),
            NumberOfTotalUsersSupported(17L),
            NumberOfPINUsersSupported(18L),
            NumberOfRFIDUsersSupported(19L),
            NumberOfWeekDaySchedulesSupportedPerUser(20L),
            NumberOfYearDaySchedulesSupportedPerUser(21L),
            NumberOfHolidaySchedulesSupported(22L),
            MaxPINCodeLength(23L),
            MinPINCodeLength(24L),
            MaxRFIDCodeLength(25L),
            MinRFIDCodeLength(26L),
            CredentialRulesSupport(27L),
            NumberOfCredentialsSupportedPerUser(28L),
            Language(33L),
            LEDSettings(34L),
            AutoRelockTime(35L),
            SoundVolume(36L),
            OperatingMode(37L),
            SupportedOperatingModes(38L),
            DefaultConfigurationRegister(39L),
            EnableLocalProgramming(40L),
            EnableOneTouchLocking(41L),
            EnableInsideStatusLED(42L),
            EnablePrivacyModeButton(43L),
            LocalProgrammingFeatures(44L),
            WrongCodeEntryLimit(48L),
            UserCodeTemporaryDisableTime(49L),
            SendPINOverTheAir(50L),
            RequirePINforRemoteOperation(51L),
            ExpiringUserTimeout(53L),
            AliroReaderVerificationKey(128L),
            AliroReaderGroupIdentifier(129L),
            AliroReaderGroupSubIdentifier(130L),
            AliroExpeditedTransactionSupportedProtocolVersions(131L),
            AliroGroupResolvingKey(132L),
            AliroSupportedBLEUWBProtocolVersions(133L),
            AliroBLEAdvertisingVersion(134L),
            NumberOfAliroCredentialIssuerKeysSupported(135L),
            NumberOfAliroEndpointKeysSupported(136L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            DoorLockAlarm(0L),
            DoorStateChange(1L),
            LockOperation(2L),
            LockOperationError(3L),
            LockUserChange(4L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            LockDoor(0L),
            UnlockDoor(1L),
            UnlockWithTimeout(3L),
            SetWeekDaySchedule(11L),
            GetWeekDaySchedule(12L),
            ClearWeekDaySchedule(13L),
            SetYearDaySchedule(14L),
            GetYearDaySchedule(15L),
            ClearYearDaySchedule(16L),
            SetHolidaySchedule(17L),
            GetHolidaySchedule(18L),
            ClearHolidaySchedule(19L),
            SetUser(26L),
            GetUser(27L),
            ClearUser(29L),
            SetCredential(34L),
            GetCredentialStatus(36L),
            ClearCredential(38L),
            UnboltDoor(39L),
            SetAliroReaderConfig(40L),
            ClearAliroReaderConfig(41L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum LockDoorCommandField {PINCode(0),;
                    private final int id;
                    LockDoorCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static LockDoorCommandField value(int id) throws NoSuchFieldError {
                        for (LockDoorCommandField field : LockDoorCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum UnlockDoorCommandField {PINCode(0),;
                    private final int id;
                    UnlockDoorCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static UnlockDoorCommandField value(int id) throws NoSuchFieldError {
                        for (UnlockDoorCommandField field : UnlockDoorCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum UnlockWithTimeoutCommandField {Timeout(0),PINCode(1),;
                    private final int id;
                    UnlockWithTimeoutCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static UnlockWithTimeoutCommandField value(int id) throws NoSuchFieldError {
                        for (UnlockWithTimeoutCommandField field : UnlockWithTimeoutCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum SetWeekDayScheduleCommandField {WeekDayIndex(0),UserIndex(1),DaysMask(2),StartHour(3),StartMinute(4),EndHour(5),EndMinute(6),;
                    private final int id;
                    SetWeekDayScheduleCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SetWeekDayScheduleCommandField value(int id) throws NoSuchFieldError {
                        for (SetWeekDayScheduleCommandField field : SetWeekDayScheduleCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum GetWeekDayScheduleCommandField {WeekDayIndex(0),UserIndex(1),;
                    private final int id;
                    GetWeekDayScheduleCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static GetWeekDayScheduleCommandField value(int id) throws NoSuchFieldError {
                        for (GetWeekDayScheduleCommandField field : GetWeekDayScheduleCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum ClearWeekDayScheduleCommandField {WeekDayIndex(0),UserIndex(1),;
                    private final int id;
                    ClearWeekDayScheduleCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ClearWeekDayScheduleCommandField value(int id) throws NoSuchFieldError {
                        for (ClearWeekDayScheduleCommandField field : ClearWeekDayScheduleCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum SetYearDayScheduleCommandField {YearDayIndex(0),UserIndex(1),LocalStartTime(2),LocalEndTime(3),;
                    private final int id;
                    SetYearDayScheduleCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SetYearDayScheduleCommandField value(int id) throws NoSuchFieldError {
                        for (SetYearDayScheduleCommandField field : SetYearDayScheduleCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum GetYearDayScheduleCommandField {YearDayIndex(0),UserIndex(1),;
                    private final int id;
                    GetYearDayScheduleCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static GetYearDayScheduleCommandField value(int id) throws NoSuchFieldError {
                        for (GetYearDayScheduleCommandField field : GetYearDayScheduleCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum ClearYearDayScheduleCommandField {YearDayIndex(0),UserIndex(1),;
                    private final int id;
                    ClearYearDayScheduleCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ClearYearDayScheduleCommandField value(int id) throws NoSuchFieldError {
                        for (ClearYearDayScheduleCommandField field : ClearYearDayScheduleCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum SetHolidayScheduleCommandField {HolidayIndex(0),LocalStartTime(1),LocalEndTime(2),OperatingMode(3),;
                    private final int id;
                    SetHolidayScheduleCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SetHolidayScheduleCommandField value(int id) throws NoSuchFieldError {
                        for (SetHolidayScheduleCommandField field : SetHolidayScheduleCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum GetHolidayScheduleCommandField {HolidayIndex(0),;
                    private final int id;
                    GetHolidayScheduleCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static GetHolidayScheduleCommandField value(int id) throws NoSuchFieldError {
                        for (GetHolidayScheduleCommandField field : GetHolidayScheduleCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum ClearHolidayScheduleCommandField {HolidayIndex(0),;
                    private final int id;
                    ClearHolidayScheduleCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ClearHolidayScheduleCommandField value(int id) throws NoSuchFieldError {
                        for (ClearHolidayScheduleCommandField field : ClearHolidayScheduleCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum SetUserCommandField {OperationType(0),UserIndex(1),UserName(2),UserUniqueID(3),UserStatus(4),UserType(5),CredentialRule(6),;
                    private final int id;
                    SetUserCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SetUserCommandField value(int id) throws NoSuchFieldError {
                        for (SetUserCommandField field : SetUserCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum GetUserCommandField {UserIndex(0),;
                    private final int id;
                    GetUserCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static GetUserCommandField value(int id) throws NoSuchFieldError {
                        for (GetUserCommandField field : GetUserCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum ClearUserCommandField {UserIndex(0),;
                    private final int id;
                    ClearUserCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ClearUserCommandField value(int id) throws NoSuchFieldError {
                        for (ClearUserCommandField field : ClearUserCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum SetCredentialCommandField {OperationType(0),Credential(1),CredentialData(2),UserIndex(3),UserStatus(4),UserType(5),;
                    private final int id;
                    SetCredentialCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SetCredentialCommandField value(int id) throws NoSuchFieldError {
                        for (SetCredentialCommandField field : SetCredentialCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum GetCredentialStatusCommandField {Credential(0),;
                    private final int id;
                    GetCredentialStatusCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static GetCredentialStatusCommandField value(int id) throws NoSuchFieldError {
                        for (GetCredentialStatusCommandField field : GetCredentialStatusCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum ClearCredentialCommandField {Credential(0),;
                    private final int id;
                    ClearCredentialCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ClearCredentialCommandField value(int id) throws NoSuchFieldError {
                        for (ClearCredentialCommandField field : ClearCredentialCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum UnboltDoorCommandField {PINCode(0),;
                    private final int id;
                    UnboltDoorCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static UnboltDoorCommandField value(int id) throws NoSuchFieldError {
                        for (UnboltDoorCommandField field : UnboltDoorCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum SetAliroReaderConfigCommandField {SigningKey(0),VerificationKey(1),GroupIdentifier(2),GroupResolvingKey(3),;
                    private final int id;
                    SetAliroReaderConfigCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SetAliroReaderConfigCommandField value(int id) throws NoSuchFieldError {
                        for (SetAliroReaderConfigCommandField field : SetAliroReaderConfigCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class WindowCovering implements BaseCluster {
        public static final long ID = 258L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            Type(0L),
            PhysicalClosedLimitLift(1L),
            PhysicalClosedLimitTilt(2L),
            CurrentPositionLift(3L),
            CurrentPositionTilt(4L),
            NumberOfActuationsLift(5L),
            NumberOfActuationsTilt(6L),
            ConfigStatus(7L),
            CurrentPositionLiftPercentage(8L),
            CurrentPositionTiltPercentage(9L),
            OperationalStatus(10L),
            TargetPositionLiftPercent100ths(11L),
            TargetPositionTiltPercent100ths(12L),
            EndProductType(13L),
            CurrentPositionLiftPercent100ths(14L),
            CurrentPositionTiltPercent100ths(15L),
            InstalledOpenLimitLift(16L),
            InstalledClosedLimitLift(17L),
            InstalledOpenLimitTilt(18L),
            InstalledClosedLimitTilt(19L),
            Mode(23L),
            SafetyStatus(26L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            UpOrOpen(0L),
            DownOrClose(1L),
            StopMotion(2L),
            GoToLiftValue(4L),
            GoToLiftPercentage(5L),
            GoToTiltValue(7L),
            GoToTiltPercentage(8L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum GoToLiftValueCommandField {LiftValue(0),;
                    private final int id;
                    GoToLiftValueCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static GoToLiftValueCommandField value(int id) throws NoSuchFieldError {
                        for (GoToLiftValueCommandField field : GoToLiftValueCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum GoToLiftPercentageCommandField {LiftPercent100thsValue(0),;
                    private final int id;
                    GoToLiftPercentageCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static GoToLiftPercentageCommandField value(int id) throws NoSuchFieldError {
                        for (GoToLiftPercentageCommandField field : GoToLiftPercentageCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum GoToTiltValueCommandField {TiltValue(0),;
                    private final int id;
                    GoToTiltValueCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static GoToTiltValueCommandField value(int id) throws NoSuchFieldError {
                        for (GoToTiltValueCommandField field : GoToTiltValueCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum GoToTiltPercentageCommandField {TiltPercent100thsValue(0),;
                    private final int id;
                    GoToTiltPercentageCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static GoToTiltPercentageCommandField value(int id) throws NoSuchFieldError {
                        for (GoToTiltPercentageCommandField field : GoToTiltPercentageCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class ClosureControl implements BaseCluster {
        public static final long ID = 260L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            CountdownTime(0L),
            MainState(1L),
            CurrentErrorList(2L),
            OverallCurrentState(3L),
            OverallTargetState(4L),
            LatchControlModes(5L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            OperationalError(0L),
            MovementCompleted(1L),
            EngageStateChanged(2L),
            SecureStateChanged(3L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            Stop(0L),
            MoveTo(1L),
            Calibrate(2L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum MoveToCommandField {Position(0),Latch(1),Speed(2),;
                    private final int id;
                    MoveToCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static MoveToCommandField value(int id) throws NoSuchFieldError {
                        for (MoveToCommandField field : MoveToCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class ClosureDimension implements BaseCluster {
        public static final long ID = 261L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            CurrentState(0L),
            TargetState(1L),
            Resolution(2L),
            StepValue(3L),
            Unit(4L),
            UnitRange(5L),
            LimitRange(6L),
            TranslationDirection(7L),
            RotationAxis(8L),
            Overflow(9L),
            ModulationType(10L),
            LatchControlModes(11L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            SetTarget(0L),
            Step(1L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum SetTargetCommandField {Position(0),Latch(1),Speed(2),;
                    private final int id;
                    SetTargetCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SetTargetCommandField value(int id) throws NoSuchFieldError {
                        for (SetTargetCommandField field : SetTargetCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum StepCommandField {Direction(0),NumberOfSteps(1),Speed(2),;
                    private final int id;
                    StepCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static StepCommandField value(int id) throws NoSuchFieldError {
                        for (StepCommandField field : StepCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class ServiceArea implements BaseCluster {
        public static final long ID = 336L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            SupportedAreas(0L),
            SupportedMaps(1L),
            SelectedAreas(2L),
            CurrentArea(3L),
            EstimatedEndTime(4L),
            Progress(5L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            SelectAreas(0L),
            SkipArea(2L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum SelectAreasCommandField {NewAreas(0),;
                    private final int id;
                    SelectAreasCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SelectAreasCommandField value(int id) throws NoSuchFieldError {
                        for (SelectAreasCommandField field : SelectAreasCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum SkipAreaCommandField {SkippedArea(0),;
                    private final int id;
                    SkipAreaCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SkipAreaCommandField value(int id) throws NoSuchFieldError {
                        for (SkipAreaCommandField field : SkipAreaCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class PumpConfigurationAndControl implements BaseCluster {
        public static final long ID = 512L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            MaxPressure(0L),
            MaxSpeed(1L),
            MaxFlow(2L),
            MinConstPressure(3L),
            MaxConstPressure(4L),
            MinCompPressure(5L),
            MaxCompPressure(6L),
            MinConstSpeed(7L),
            MaxConstSpeed(8L),
            MinConstFlow(9L),
            MaxConstFlow(10L),
            MinConstTemp(11L),
            MaxConstTemp(12L),
            PumpStatus(16L),
            EffectiveOperationMode(17L),
            EffectiveControlMode(18L),
            Capacity(19L),
            Speed(20L),
            LifetimeRunningHours(21L),
            Power(22L),
            LifetimeEnergyConsumed(23L),
            OperationMode(32L),
            ControlMode(33L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            SupplyVoltageLow(0L),
            SupplyVoltageHigh(1L),
            PowerMissingPhase(2L),
            SystemPressureLow(3L),
            SystemPressureHigh(4L),
            DryRunning(5L),
            MotorTemperatureHigh(6L),
            PumpMotorFatalFailure(7L),
            ElectronicTemperatureHigh(8L),
            PumpBlocked(9L),
            SensorFailure(10L),
            ElectronicNonFatalFailure(11L),
            ElectronicFatalFailure(12L),
            GeneralFault(13L),
            Leakage(14L),
            AirDetection(15L),
            TurbineOperation(16L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class Thermostat implements BaseCluster {
        public static final long ID = 513L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            LocalTemperature(0L),
            OutdoorTemperature(1L),
            Occupancy(2L),
            AbsMinHeatSetpointLimit(3L),
            AbsMaxHeatSetpointLimit(4L),
            AbsMinCoolSetpointLimit(5L),
            AbsMaxCoolSetpointLimit(6L),
            PICoolingDemand(7L),
            PIHeatingDemand(8L),
            HVACSystemTypeConfiguration(9L),
            LocalTemperatureCalibration(16L),
            OccupiedCoolingSetpoint(17L),
            OccupiedHeatingSetpoint(18L),
            UnoccupiedCoolingSetpoint(19L),
            UnoccupiedHeatingSetpoint(20L),
            MinHeatSetpointLimit(21L),
            MaxHeatSetpointLimit(22L),
            MinCoolSetpointLimit(23L),
            MaxCoolSetpointLimit(24L),
            MinSetpointDeadBand(25L),
            RemoteSensing(26L),
            ControlSequenceOfOperation(27L),
            SystemMode(28L),
            ThermostatRunningMode(30L),
            StartOfWeek(32L),
            NumberOfWeeklyTransitions(33L),
            NumberOfDailyTransitions(34L),
            TemperatureSetpointHold(35L),
            TemperatureSetpointHoldDuration(36L),
            ThermostatProgrammingOperationMode(37L),
            ThermostatRunningState(41L),
            SetpointChangeSource(48L),
            SetpointChangeAmount(49L),
            SetpointChangeSourceTimestamp(50L),
            OccupiedSetback(52L),
            OccupiedSetbackMin(53L),
            OccupiedSetbackMax(54L),
            UnoccupiedSetback(55L),
            UnoccupiedSetbackMin(56L),
            UnoccupiedSetbackMax(57L),
            EmergencyHeatDelta(58L),
            ACType(64L),
            ACCapacity(65L),
            ACRefrigerantType(66L),
            ACCompressorType(67L),
            ACErrorCode(68L),
            ACLouverPosition(69L),
            ACCoilTemperature(70L),
            ACCapacityformat(71L),
            PresetTypes(72L),
            ScheduleTypes(73L),
            NumberOfPresets(74L),
            NumberOfSchedules(75L),
            NumberOfScheduleTransitions(76L),
            NumberOfScheduleTransitionPerDay(77L),
            ActivePresetHandle(78L),
            ActiveScheduleHandle(79L),
            Presets(80L),
            Schedules(81L),
            SetpointHoldExpiryTimestamp(82L),
            MaxThermostatSuggestions(83L),
            ThermostatSuggestions(84L),
            CurrentThermostatSuggestion(85L),
            ThermostatSuggestionNotFollowingReason(86L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            SystemModeChange(0L),
            LocalTemperatureChange(1L),
            OccupancyChange(2L),
            SetpointChange(3L),
            RunningStateChange(4L),
            RunningModeChange(5L),
            ActiveScheduleChange(6L),
            ActivePresetChange(7L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            SetpointRaiseLower(0L),
            SetWeeklySchedule(1L),
            GetWeeklySchedule(2L),
            ClearWeeklySchedule(3L),
            SetActiveScheduleRequest(5L),
            SetActivePresetRequest(6L),
            AddThermostatSuggestion(7L),
            RemoveThermostatSuggestion(8L),
            AtomicRequest(254L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum SetpointRaiseLowerCommandField {Mode(0),Amount(1),;
                    private final int id;
                    SetpointRaiseLowerCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SetpointRaiseLowerCommandField value(int id) throws NoSuchFieldError {
                        for (SetpointRaiseLowerCommandField field : SetpointRaiseLowerCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum SetWeeklyScheduleCommandField {NumberOfTransitionsForSequence(0),DayOfWeekForSequence(1),ModeForSequence(2),Transitions(3),;
                    private final int id;
                    SetWeeklyScheduleCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SetWeeklyScheduleCommandField value(int id) throws NoSuchFieldError {
                        for (SetWeeklyScheduleCommandField field : SetWeeklyScheduleCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum GetWeeklyScheduleCommandField {DaysToReturn(0),ModeToReturn(1),;
                    private final int id;
                    GetWeeklyScheduleCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static GetWeeklyScheduleCommandField value(int id) throws NoSuchFieldError {
                        for (GetWeeklyScheduleCommandField field : GetWeeklyScheduleCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum SetActiveScheduleRequestCommandField {ScheduleHandle(0),;
                    private final int id;
                    SetActiveScheduleRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SetActiveScheduleRequestCommandField value(int id) throws NoSuchFieldError {
                        for (SetActiveScheduleRequestCommandField field : SetActiveScheduleRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum SetActivePresetRequestCommandField {PresetHandle(0),;
                    private final int id;
                    SetActivePresetRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SetActivePresetRequestCommandField value(int id) throws NoSuchFieldError {
                        for (SetActivePresetRequestCommandField field : SetActivePresetRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum AddThermostatSuggestionCommandField {PresetHandle(0),EffectiveTime(1),ExpirationInMinutes(2),;
                    private final int id;
                    AddThermostatSuggestionCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static AddThermostatSuggestionCommandField value(int id) throws NoSuchFieldError {
                        for (AddThermostatSuggestionCommandField field : AddThermostatSuggestionCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum RemoveThermostatSuggestionCommandField {UniqueID(0),;
                    private final int id;
                    RemoveThermostatSuggestionCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static RemoveThermostatSuggestionCommandField value(int id) throws NoSuchFieldError {
                        for (RemoveThermostatSuggestionCommandField field : RemoveThermostatSuggestionCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum AtomicRequestCommandField {RequestType(0),AttributeRequests(1),Timeout(2),;
                    private final int id;
                    AtomicRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static AtomicRequestCommandField value(int id) throws NoSuchFieldError {
                        for (AtomicRequestCommandField field : AtomicRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class FanControl implements BaseCluster {
        public static final long ID = 514L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            FanMode(0L),
            FanModeSequence(1L),
            PercentSetting(2L),
            PercentCurrent(3L),
            SpeedMax(4L),
            SpeedSetting(5L),
            SpeedCurrent(6L),
            RockSupport(7L),
            RockSetting(8L),
            WindSupport(9L),
            WindSetting(10L),
            AirflowDirection(11L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            Step(0L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum StepCommandField {Direction(0),Wrap(1),LowestOff(2),;
                    private final int id;
                    StepCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static StepCommandField value(int id) throws NoSuchFieldError {
                        for (StepCommandField field : StepCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class ThermostatUserInterfaceConfiguration implements BaseCluster {
        public static final long ID = 516L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            TemperatureDisplayMode(0L),
            KeypadLockout(1L),
            ScheduleProgrammingVisibility(2L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class ColorControl implements BaseCluster {
        public static final long ID = 768L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            CurrentHue(0L),
            CurrentSaturation(1L),
            RemainingTime(2L),
            CurrentX(3L),
            CurrentY(4L),
            DriftCompensation(5L),
            CompensationText(6L),
            ColorTemperatureMireds(7L),
            ColorMode(8L),
            Options(15L),
            NumberOfPrimaries(16L),
            Primary1X(17L),
            Primary1Y(18L),
            Primary1Intensity(19L),
            Primary2X(21L),
            Primary2Y(22L),
            Primary2Intensity(23L),
            Primary3X(25L),
            Primary3Y(26L),
            Primary3Intensity(27L),
            Primary4X(32L),
            Primary4Y(33L),
            Primary4Intensity(34L),
            Primary5X(36L),
            Primary5Y(37L),
            Primary5Intensity(38L),
            Primary6X(40L),
            Primary6Y(41L),
            Primary6Intensity(42L),
            WhitePointX(48L),
            WhitePointY(49L),
            ColorPointRX(50L),
            ColorPointRY(51L),
            ColorPointRIntensity(52L),
            ColorPointGX(54L),
            ColorPointGY(55L),
            ColorPointGIntensity(56L),
            ColorPointBX(58L),
            ColorPointBY(59L),
            ColorPointBIntensity(60L),
            EnhancedCurrentHue(16384L),
            EnhancedColorMode(16385L),
            ColorLoopActive(16386L),
            ColorLoopDirection(16387L),
            ColorLoopTime(16388L),
            ColorLoopStartEnhancedHue(16389L),
            ColorLoopStoredEnhancedHue(16390L),
            ColorCapabilities(16394L),
            ColorTempPhysicalMinMireds(16395L),
            ColorTempPhysicalMaxMireds(16396L),
            CoupleColorTempToLevelMinMireds(16397L),
            StartUpColorTemperatureMireds(16400L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            MoveToHue(0L),
            MoveHue(1L),
            StepHue(2L),
            MoveToSaturation(3L),
            MoveSaturation(4L),
            StepSaturation(5L),
            MoveToHueAndSaturation(6L),
            MoveToColor(7L),
            MoveColor(8L),
            StepColor(9L),
            MoveToColorTemperature(10L),
            EnhancedMoveToHue(64L),
            EnhancedMoveHue(65L),
            EnhancedStepHue(66L),
            EnhancedMoveToHueAndSaturation(67L),
            ColorLoopSet(68L),
            StopMoveStep(71L),
            MoveColorTemperature(75L),
            StepColorTemperature(76L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum MoveToHueCommandField {Hue(0),Direction(1),TransitionTime(2),OptionsMask(3),OptionsOverride(4),;
                    private final int id;
                    MoveToHueCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static MoveToHueCommandField value(int id) throws NoSuchFieldError {
                        for (MoveToHueCommandField field : MoveToHueCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum MoveHueCommandField {MoveMode(0),Rate(1),OptionsMask(2),OptionsOverride(3),;
                    private final int id;
                    MoveHueCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static MoveHueCommandField value(int id) throws NoSuchFieldError {
                        for (MoveHueCommandField field : MoveHueCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum StepHueCommandField {StepMode(0),StepSize(1),TransitionTime(2),OptionsMask(3),OptionsOverride(4),;
                    private final int id;
                    StepHueCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static StepHueCommandField value(int id) throws NoSuchFieldError {
                        for (StepHueCommandField field : StepHueCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum MoveToSaturationCommandField {Saturation(0),TransitionTime(1),OptionsMask(2),OptionsOverride(3),;
                    private final int id;
                    MoveToSaturationCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static MoveToSaturationCommandField value(int id) throws NoSuchFieldError {
                        for (MoveToSaturationCommandField field : MoveToSaturationCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum MoveSaturationCommandField {MoveMode(0),Rate(1),OptionsMask(2),OptionsOverride(3),;
                    private final int id;
                    MoveSaturationCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static MoveSaturationCommandField value(int id) throws NoSuchFieldError {
                        for (MoveSaturationCommandField field : MoveSaturationCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum StepSaturationCommandField {StepMode(0),StepSize(1),TransitionTime(2),OptionsMask(3),OptionsOverride(4),;
                    private final int id;
                    StepSaturationCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static StepSaturationCommandField value(int id) throws NoSuchFieldError {
                        for (StepSaturationCommandField field : StepSaturationCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum MoveToHueAndSaturationCommandField {Hue(0),Saturation(1),TransitionTime(2),OptionsMask(3),OptionsOverride(4),;
                    private final int id;
                    MoveToHueAndSaturationCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static MoveToHueAndSaturationCommandField value(int id) throws NoSuchFieldError {
                        for (MoveToHueAndSaturationCommandField field : MoveToHueAndSaturationCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum MoveToColorCommandField {ColorX(0),ColorY(1),TransitionTime(2),OptionsMask(3),OptionsOverride(4),;
                    private final int id;
                    MoveToColorCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static MoveToColorCommandField value(int id) throws NoSuchFieldError {
                        for (MoveToColorCommandField field : MoveToColorCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum MoveColorCommandField {RateX(0),RateY(1),OptionsMask(2),OptionsOverride(3),;
                    private final int id;
                    MoveColorCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static MoveColorCommandField value(int id) throws NoSuchFieldError {
                        for (MoveColorCommandField field : MoveColorCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum StepColorCommandField {StepX(0),StepY(1),TransitionTime(2),OptionsMask(3),OptionsOverride(4),;
                    private final int id;
                    StepColorCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static StepColorCommandField value(int id) throws NoSuchFieldError {
                        for (StepColorCommandField field : StepColorCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum MoveToColorTemperatureCommandField {ColorTemperatureMireds(0),TransitionTime(1),OptionsMask(2),OptionsOverride(3),;
                    private final int id;
                    MoveToColorTemperatureCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static MoveToColorTemperatureCommandField value(int id) throws NoSuchFieldError {
                        for (MoveToColorTemperatureCommandField field : MoveToColorTemperatureCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum EnhancedMoveToHueCommandField {EnhancedHue(0),Direction(1),TransitionTime(2),OptionsMask(3),OptionsOverride(4),;
                    private final int id;
                    EnhancedMoveToHueCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static EnhancedMoveToHueCommandField value(int id) throws NoSuchFieldError {
                        for (EnhancedMoveToHueCommandField field : EnhancedMoveToHueCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum EnhancedMoveHueCommandField {MoveMode(0),Rate(1),OptionsMask(2),OptionsOverride(3),;
                    private final int id;
                    EnhancedMoveHueCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static EnhancedMoveHueCommandField value(int id) throws NoSuchFieldError {
                        for (EnhancedMoveHueCommandField field : EnhancedMoveHueCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum EnhancedStepHueCommandField {StepMode(0),StepSize(1),TransitionTime(2),OptionsMask(3),OptionsOverride(4),;
                    private final int id;
                    EnhancedStepHueCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static EnhancedStepHueCommandField value(int id) throws NoSuchFieldError {
                        for (EnhancedStepHueCommandField field : EnhancedStepHueCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum EnhancedMoveToHueAndSaturationCommandField {EnhancedHue(0),Saturation(1),TransitionTime(2),OptionsMask(3),OptionsOverride(4),;
                    private final int id;
                    EnhancedMoveToHueAndSaturationCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static EnhancedMoveToHueAndSaturationCommandField value(int id) throws NoSuchFieldError {
                        for (EnhancedMoveToHueAndSaturationCommandField field : EnhancedMoveToHueAndSaturationCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum ColorLoopSetCommandField {UpdateFlags(0),Action(1),Direction(2),Time(3),StartHue(4),OptionsMask(5),OptionsOverride(6),;
                    private final int id;
                    ColorLoopSetCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ColorLoopSetCommandField value(int id) throws NoSuchFieldError {
                        for (ColorLoopSetCommandField field : ColorLoopSetCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum StopMoveStepCommandField {OptionsMask(0),OptionsOverride(1),;
                    private final int id;
                    StopMoveStepCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static StopMoveStepCommandField value(int id) throws NoSuchFieldError {
                        for (StopMoveStepCommandField field : StopMoveStepCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum MoveColorTemperatureCommandField {MoveMode(0),Rate(1),ColorTemperatureMinimumMireds(2),ColorTemperatureMaximumMireds(3),OptionsMask(4),OptionsOverride(5),;
                    private final int id;
                    MoveColorTemperatureCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static MoveColorTemperatureCommandField value(int id) throws NoSuchFieldError {
                        for (MoveColorTemperatureCommandField field : MoveColorTemperatureCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum StepColorTemperatureCommandField {StepMode(0),StepSize(1),TransitionTime(2),ColorTemperatureMinimumMireds(3),ColorTemperatureMaximumMireds(4),OptionsMask(5),OptionsOverride(6),;
                    private final int id;
                    StepColorTemperatureCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static StepColorTemperatureCommandField value(int id) throws NoSuchFieldError {
                        for (StepColorTemperatureCommandField field : StepColorTemperatureCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class BallastConfiguration implements BaseCluster {
        public static final long ID = 769L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            PhysicalMinLevel(0L),
            PhysicalMaxLevel(1L),
            BallastStatus(2L),
            MinLevel(16L),
            MaxLevel(17L),
            IntrinsicBallastFactor(20L),
            BallastFactorAdjustment(21L),
            LampQuantity(32L),
            LampType(48L),
            LampManufacturer(49L),
            LampRatedHours(50L),
            LampBurnHours(51L),
            LampAlarmMode(52L),
            LampBurnHoursTripPoint(53L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class IlluminanceMeasurement implements BaseCluster {
        public static final long ID = 1024L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            MeasuredValue(0L),
            MinMeasuredValue(1L),
            MaxMeasuredValue(2L),
            Tolerance(3L),
            LightSensorType(4L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class TemperatureMeasurement implements BaseCluster {
        public static final long ID = 1026L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            MeasuredValue(0L),
            MinMeasuredValue(1L),
            MaxMeasuredValue(2L),
            Tolerance(3L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class PressureMeasurement implements BaseCluster {
        public static final long ID = 1027L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            MeasuredValue(0L),
            MinMeasuredValue(1L),
            MaxMeasuredValue(2L),
            Tolerance(3L),
            ScaledValue(16L),
            MinScaledValue(17L),
            MaxScaledValue(18L),
            ScaledTolerance(19L),
            Scale(20L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class FlowMeasurement implements BaseCluster {
        public static final long ID = 1028L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            MeasuredValue(0L),
            MinMeasuredValue(1L),
            MaxMeasuredValue(2L),
            Tolerance(3L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class RelativeHumidityMeasurement implements BaseCluster {
        public static final long ID = 1029L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            MeasuredValue(0L),
            MinMeasuredValue(1L),
            MaxMeasuredValue(2L),
            Tolerance(3L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class OccupancySensing implements BaseCluster {
        public static final long ID = 1030L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            Occupancy(0L),
            OccupancySensorType(1L),
            OccupancySensorTypeBitmap(2L),
            HoldTime(3L),
            HoldTimeLimits(4L),
            PIROccupiedToUnoccupiedDelay(16L),
            PIRUnoccupiedToOccupiedDelay(17L),
            PIRUnoccupiedToOccupiedThreshold(18L),
            UltrasonicOccupiedToUnoccupiedDelay(32L),
            UltrasonicUnoccupiedToOccupiedDelay(33L),
            UltrasonicUnoccupiedToOccupiedThreshold(34L),
            PhysicalContactOccupiedToUnoccupiedDelay(48L),
            PhysicalContactUnoccupiedToOccupiedDelay(49L),
            PhysicalContactUnoccupiedToOccupiedThreshold(50L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            OccupancyChanged(0L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class CarbonMonoxideConcentrationMeasurement implements BaseCluster {
        public static final long ID = 1036L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            MeasuredValue(0L),
            MinMeasuredValue(1L),
            MaxMeasuredValue(2L),
            PeakMeasuredValue(3L),
            PeakMeasuredValueWindow(4L),
            AverageMeasuredValue(5L),
            AverageMeasuredValueWindow(6L),
            Uncertainty(7L),
            MeasurementUnit(8L),
            MeasurementMedium(9L),
            LevelValue(10L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class CarbonDioxideConcentrationMeasurement implements BaseCluster {
        public static final long ID = 1037L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            MeasuredValue(0L),
            MinMeasuredValue(1L),
            MaxMeasuredValue(2L),
            PeakMeasuredValue(3L),
            PeakMeasuredValueWindow(4L),
            AverageMeasuredValue(5L),
            AverageMeasuredValueWindow(6L),
            Uncertainty(7L),
            MeasurementUnit(8L),
            MeasurementMedium(9L),
            LevelValue(10L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class NitrogenDioxideConcentrationMeasurement implements BaseCluster {
        public static final long ID = 1043L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            MeasuredValue(0L),
            MinMeasuredValue(1L),
            MaxMeasuredValue(2L),
            PeakMeasuredValue(3L),
            PeakMeasuredValueWindow(4L),
            AverageMeasuredValue(5L),
            AverageMeasuredValueWindow(6L),
            Uncertainty(7L),
            MeasurementUnit(8L),
            MeasurementMedium(9L),
            LevelValue(10L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class OzoneConcentrationMeasurement implements BaseCluster {
        public static final long ID = 1045L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            MeasuredValue(0L),
            MinMeasuredValue(1L),
            MaxMeasuredValue(2L),
            PeakMeasuredValue(3L),
            PeakMeasuredValueWindow(4L),
            AverageMeasuredValue(5L),
            AverageMeasuredValueWindow(6L),
            Uncertainty(7L),
            MeasurementUnit(8L),
            MeasurementMedium(9L),
            LevelValue(10L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class Pm25ConcentrationMeasurement implements BaseCluster {
        public static final long ID = 1066L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            MeasuredValue(0L),
            MinMeasuredValue(1L),
            MaxMeasuredValue(2L),
            PeakMeasuredValue(3L),
            PeakMeasuredValueWindow(4L),
            AverageMeasuredValue(5L),
            AverageMeasuredValueWindow(6L),
            Uncertainty(7L),
            MeasurementUnit(8L),
            MeasurementMedium(9L),
            LevelValue(10L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class FormaldehydeConcentrationMeasurement implements BaseCluster {
        public static final long ID = 1067L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            MeasuredValue(0L),
            MinMeasuredValue(1L),
            MaxMeasuredValue(2L),
            PeakMeasuredValue(3L),
            PeakMeasuredValueWindow(4L),
            AverageMeasuredValue(5L),
            AverageMeasuredValueWindow(6L),
            Uncertainty(7L),
            MeasurementUnit(8L),
            MeasurementMedium(9L),
            LevelValue(10L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class Pm1ConcentrationMeasurement implements BaseCluster {
        public static final long ID = 1068L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            MeasuredValue(0L),
            MinMeasuredValue(1L),
            MaxMeasuredValue(2L),
            PeakMeasuredValue(3L),
            PeakMeasuredValueWindow(4L),
            AverageMeasuredValue(5L),
            AverageMeasuredValueWindow(6L),
            Uncertainty(7L),
            MeasurementUnit(8L),
            MeasurementMedium(9L),
            LevelValue(10L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class Pm10ConcentrationMeasurement implements BaseCluster {
        public static final long ID = 1069L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            MeasuredValue(0L),
            MinMeasuredValue(1L),
            MaxMeasuredValue(2L),
            PeakMeasuredValue(3L),
            PeakMeasuredValueWindow(4L),
            AverageMeasuredValue(5L),
            AverageMeasuredValueWindow(6L),
            Uncertainty(7L),
            MeasurementUnit(8L),
            MeasurementMedium(9L),
            LevelValue(10L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class TotalVolatileOrganicCompoundsConcentrationMeasurement implements BaseCluster {
        public static final long ID = 1070L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            MeasuredValue(0L),
            MinMeasuredValue(1L),
            MaxMeasuredValue(2L),
            PeakMeasuredValue(3L),
            PeakMeasuredValueWindow(4L),
            AverageMeasuredValue(5L),
            AverageMeasuredValueWindow(6L),
            Uncertainty(7L),
            MeasurementUnit(8L),
            MeasurementMedium(9L),
            LevelValue(10L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class RadonConcentrationMeasurement implements BaseCluster {
        public static final long ID = 1071L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            MeasuredValue(0L),
            MinMeasuredValue(1L),
            MaxMeasuredValue(2L),
            PeakMeasuredValue(3L),
            PeakMeasuredValueWindow(4L),
            AverageMeasuredValue(5L),
            AverageMeasuredValueWindow(6L),
            Uncertainty(7L),
            MeasurementUnit(8L),
            MeasurementMedium(9L),
            LevelValue(10L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class SoilMeasurement implements BaseCluster {
        public static final long ID = 1072L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            SoilMoistureMeasurementLimits(0L),
            SoilMoistureMeasuredValue(1L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class WiFiNetworkManagement implements BaseCluster {
        public static final long ID = 1105L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            Ssid(0L),
            PassphraseSurrogate(1L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            NetworkPassphraseRequest(0L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class ThreadBorderRouterManagement implements BaseCluster {
        public static final long ID = 1106L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            BorderRouterName(0L),
            BorderAgentID(1L),
            ThreadVersion(2L),
            InterfaceEnabled(3L),
            ActiveDatasetTimestamp(4L),
            PendingDatasetTimestamp(5L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            GetActiveDatasetRequest(0L),
            GetPendingDatasetRequest(1L),
            SetActiveDatasetRequest(3L),
            SetPendingDatasetRequest(4L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum SetActiveDatasetRequestCommandField {ActiveDataset(0),Breadcrumb(1),;
                    private final int id;
                    SetActiveDatasetRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SetActiveDatasetRequestCommandField value(int id) throws NoSuchFieldError {
                        for (SetActiveDatasetRequestCommandField field : SetActiveDatasetRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum SetPendingDatasetRequestCommandField {PendingDataset(0),;
                    private final int id;
                    SetPendingDatasetRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SetPendingDatasetRequestCommandField value(int id) throws NoSuchFieldError {
                        for (SetPendingDatasetRequestCommandField field : SetPendingDatasetRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class ThreadNetworkDirectory implements BaseCluster {
        public static final long ID = 1107L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            PreferredExtendedPanID(0L),
            ThreadNetworks(1L),
            ThreadNetworkTableSize(2L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            AddNetwork(0L),
            RemoveNetwork(1L),
            GetOperationalDataset(2L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum AddNetworkCommandField {OperationalDataset(0),;
                    private final int id;
                    AddNetworkCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static AddNetworkCommandField value(int id) throws NoSuchFieldError {
                        for (AddNetworkCommandField field : AddNetworkCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum RemoveNetworkCommandField {ExtendedPanID(0),;
                    private final int id;
                    RemoveNetworkCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static RemoveNetworkCommandField value(int id) throws NoSuchFieldError {
                        for (RemoveNetworkCommandField field : RemoveNetworkCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum GetOperationalDatasetCommandField {ExtendedPanID(0),;
                    private final int id;
                    GetOperationalDatasetCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static GetOperationalDatasetCommandField value(int id) throws NoSuchFieldError {
                        for (GetOperationalDatasetCommandField field : GetOperationalDatasetCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class WakeOnLan implements BaseCluster {
        public static final long ID = 1283L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            MACAddress(0L),
            LinkLocalAddress(1L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class Channel implements BaseCluster {
        public static final long ID = 1284L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            ChannelList(0L),
            Lineup(1L),
            CurrentChannel(2L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            ChangeChannel(0L),
            ChangeChannelByNumber(2L),
            SkipChannel(3L),
            GetProgramGuide(4L),
            RecordProgram(6L),
            CancelRecordProgram(7L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum ChangeChannelCommandField {Match(0),;
                    private final int id;
                    ChangeChannelCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ChangeChannelCommandField value(int id) throws NoSuchFieldError {
                        for (ChangeChannelCommandField field : ChangeChannelCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum ChangeChannelByNumberCommandField {MajorNumber(0),MinorNumber(1),;
                    private final int id;
                    ChangeChannelByNumberCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ChangeChannelByNumberCommandField value(int id) throws NoSuchFieldError {
                        for (ChangeChannelByNumberCommandField field : ChangeChannelByNumberCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum SkipChannelCommandField {Count(0),;
                    private final int id;
                    SkipChannelCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SkipChannelCommandField value(int id) throws NoSuchFieldError {
                        for (SkipChannelCommandField field : SkipChannelCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum GetProgramGuideCommandField {StartTime(0),EndTime(1),ChannelList(2),PageToken(3),RecordingFlag(4),ExternalIDList(5),Data(6),;
                    private final int id;
                    GetProgramGuideCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static GetProgramGuideCommandField value(int id) throws NoSuchFieldError {
                        for (GetProgramGuideCommandField field : GetProgramGuideCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum RecordProgramCommandField {ProgramIdentifier(0),ShouldRecordSeries(1),ExternalIDList(2),Data(3),;
                    private final int id;
                    RecordProgramCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static RecordProgramCommandField value(int id) throws NoSuchFieldError {
                        for (RecordProgramCommandField field : RecordProgramCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum CancelRecordProgramCommandField {ProgramIdentifier(0),ShouldRecordSeries(1),ExternalIDList(2),Data(3),;
                    private final int id;
                    CancelRecordProgramCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static CancelRecordProgramCommandField value(int id) throws NoSuchFieldError {
                        for (CancelRecordProgramCommandField field : CancelRecordProgramCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class TargetNavigator implements BaseCluster {
        public static final long ID = 1285L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            TargetList(0L),
            CurrentTarget(1L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            TargetUpdated(0L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            NavigateTarget(0L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum NavigateTargetCommandField {Target(0),Data(1),;
                    private final int id;
                    NavigateTargetCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static NavigateTargetCommandField value(int id) throws NoSuchFieldError {
                        for (NavigateTargetCommandField field : NavigateTargetCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class MediaPlayback implements BaseCluster {
        public static final long ID = 1286L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            CurrentState(0L),
            StartTime(1L),
            Duration(2L),
            SampledPosition(3L),
            PlaybackSpeed(4L),
            SeekRangeEnd(5L),
            SeekRangeStart(6L),
            ActiveAudioTrack(7L),
            AvailableAudioTracks(8L),
            ActiveTextTrack(9L),
            AvailableTextTracks(10L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            StateChanged(0L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            Play(0L),
            Pause(1L),
            Stop(2L),
            StartOver(3L),
            Previous(4L),
            Next(5L),
            Rewind(6L),
            FastForward(7L),
            SkipForward(8L),
            SkipBackward(9L),
            Seek(11L),
            ActivateAudioTrack(12L),
            ActivateTextTrack(13L),
            DeactivateTextTrack(14L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum RewindCommandField {AudioAdvanceUnmuted(0),;
                    private final int id;
                    RewindCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static RewindCommandField value(int id) throws NoSuchFieldError {
                        for (RewindCommandField field : RewindCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum FastForwardCommandField {AudioAdvanceUnmuted(0),;
                    private final int id;
                    FastForwardCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static FastForwardCommandField value(int id) throws NoSuchFieldError {
                        for (FastForwardCommandField field : FastForwardCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum SkipForwardCommandField {DeltaPositionMilliseconds(0),;
                    private final int id;
                    SkipForwardCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SkipForwardCommandField value(int id) throws NoSuchFieldError {
                        for (SkipForwardCommandField field : SkipForwardCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum SkipBackwardCommandField {DeltaPositionMilliseconds(0),;
                    private final int id;
                    SkipBackwardCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SkipBackwardCommandField value(int id) throws NoSuchFieldError {
                        for (SkipBackwardCommandField field : SkipBackwardCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum SeekCommandField {Position(0),;
                    private final int id;
                    SeekCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SeekCommandField value(int id) throws NoSuchFieldError {
                        for (SeekCommandField field : SeekCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum ActivateAudioTrackCommandField {TrackID(0),AudioOutputIndex(1),;
                    private final int id;
                    ActivateAudioTrackCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ActivateAudioTrackCommandField value(int id) throws NoSuchFieldError {
                        for (ActivateAudioTrackCommandField field : ActivateAudioTrackCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum ActivateTextTrackCommandField {TrackID(0),;
                    private final int id;
                    ActivateTextTrackCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ActivateTextTrackCommandField value(int id) throws NoSuchFieldError {
                        for (ActivateTextTrackCommandField field : ActivateTextTrackCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class MediaInput implements BaseCluster {
        public static final long ID = 1287L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            InputList(0L),
            CurrentInput(1L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            SelectInput(0L),
            ShowInputStatus(1L),
            HideInputStatus(2L),
            RenameInput(3L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum SelectInputCommandField {Index(0),;
                    private final int id;
                    SelectInputCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SelectInputCommandField value(int id) throws NoSuchFieldError {
                        for (SelectInputCommandField field : SelectInputCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum RenameInputCommandField {Index(0),Name(1),;
                    private final int id;
                    RenameInputCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static RenameInputCommandField value(int id) throws NoSuchFieldError {
                        for (RenameInputCommandField field : RenameInputCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class LowPower implements BaseCluster {
        public static final long ID = 1288L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            Sleep(0L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class KeypadInput implements BaseCluster {
        public static final long ID = 1289L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            SendKey(0L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum SendKeyCommandField {KeyCode(0),;
                    private final int id;
                    SendKeyCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SendKeyCommandField value(int id) throws NoSuchFieldError {
                        for (SendKeyCommandField field : SendKeyCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class ContentLauncher implements BaseCluster {
        public static final long ID = 1290L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            AcceptHeader(0L),
            SupportedStreamingProtocols(1L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            LaunchContent(0L),
            LaunchURL(1L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum LaunchContentCommandField {Search(0),AutoPlay(1),Data(2),PlaybackPreferences(3),UseCurrentContext(4),;
                    private final int id;
                    LaunchContentCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static LaunchContentCommandField value(int id) throws NoSuchFieldError {
                        for (LaunchContentCommandField field : LaunchContentCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum LaunchURLCommandField {ContentURL(0),DisplayString(1),BrandingInformation(2),;
                    private final int id;
                    LaunchURLCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static LaunchURLCommandField value(int id) throws NoSuchFieldError {
                        for (LaunchURLCommandField field : LaunchURLCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class AudioOutput implements BaseCluster {
        public static final long ID = 1291L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            OutputList(0L),
            CurrentOutput(1L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            SelectOutput(0L),
            RenameOutput(1L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum SelectOutputCommandField {Index(0),;
                    private final int id;
                    SelectOutputCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SelectOutputCommandField value(int id) throws NoSuchFieldError {
                        for (SelectOutputCommandField field : SelectOutputCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum RenameOutputCommandField {Index(0),Name(1),;
                    private final int id;
                    RenameOutputCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static RenameOutputCommandField value(int id) throws NoSuchFieldError {
                        for (RenameOutputCommandField field : RenameOutputCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class ApplicationLauncher implements BaseCluster {
        public static final long ID = 1292L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            CatalogList(0L),
            CurrentApp(1L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            LaunchApp(0L),
            StopApp(1L),
            HideApp(2L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum LaunchAppCommandField {Application(0),Data(1),;
                    private final int id;
                    LaunchAppCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static LaunchAppCommandField value(int id) throws NoSuchFieldError {
                        for (LaunchAppCommandField field : LaunchAppCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum StopAppCommandField {Application(0),;
                    private final int id;
                    StopAppCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static StopAppCommandField value(int id) throws NoSuchFieldError {
                        for (StopAppCommandField field : StopAppCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum HideAppCommandField {Application(0),;
                    private final int id;
                    HideAppCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static HideAppCommandField value(int id) throws NoSuchFieldError {
                        for (HideAppCommandField field : HideAppCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class ApplicationBasic implements BaseCluster {
        public static final long ID = 1293L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            VendorName(0L),
            VendorID(1L),
            ApplicationName(2L),
            ProductID(3L),
            Application(4L),
            Status(5L),
            ApplicationVersion(6L),
            AllowedVendorList(7L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class AccountLogin implements BaseCluster {
        public static final long ID = 1294L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            LoggedOut(0L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            GetSetupPIN(0L),
            Login(2L),
            Logout(3L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum GetSetupPINCommandField {TempAccountIdentifier(0),;
                    private final int id;
                    GetSetupPINCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static GetSetupPINCommandField value(int id) throws NoSuchFieldError {
                        for (GetSetupPINCommandField field : GetSetupPINCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum LoginCommandField {TempAccountIdentifier(0),SetupPIN(1),Node(2),;
                    private final int id;
                    LoginCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static LoginCommandField value(int id) throws NoSuchFieldError {
                        for (LoginCommandField field : LoginCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum LogoutCommandField {Node(0),;
                    private final int id;
                    LogoutCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static LogoutCommandField value(int id) throws NoSuchFieldError {
                        for (LogoutCommandField field : LogoutCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class ContentControl implements BaseCluster {
        public static final long ID = 1295L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            Enabled(0L),
            OnDemandRatings(1L),
            OnDemandRatingThreshold(2L),
            ScheduledContentRatings(3L),
            ScheduledContentRatingThreshold(4L),
            ScreenDailyTime(5L),
            RemainingScreenTime(6L),
            BlockUnrated(7L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            RemainingScreenTimeExpired(0L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            UpdatePIN(0L),
            ResetPIN(1L),
            Enable(3L),
            Disable(4L),
            AddBonusTime(5L),
            SetScreenDailyTime(6L),
            BlockUnratedContent(7L),
            UnblockUnratedContent(8L),
            SetOnDemandRatingThreshold(9L),
            SetScheduledContentRatingThreshold(10L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum UpdatePINCommandField {OldPIN(0),NewPIN(1),;
                    private final int id;
                    UpdatePINCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static UpdatePINCommandField value(int id) throws NoSuchFieldError {
                        for (UpdatePINCommandField field : UpdatePINCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum AddBonusTimeCommandField {PINCode(0),BonusTime(1),;
                    private final int id;
                    AddBonusTimeCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static AddBonusTimeCommandField value(int id) throws NoSuchFieldError {
                        for (AddBonusTimeCommandField field : AddBonusTimeCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum SetScreenDailyTimeCommandField {ScreenTime(0),;
                    private final int id;
                    SetScreenDailyTimeCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SetScreenDailyTimeCommandField value(int id) throws NoSuchFieldError {
                        for (SetScreenDailyTimeCommandField field : SetScreenDailyTimeCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum SetOnDemandRatingThresholdCommandField {Rating(0),;
                    private final int id;
                    SetOnDemandRatingThresholdCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SetOnDemandRatingThresholdCommandField value(int id) throws NoSuchFieldError {
                        for (SetOnDemandRatingThresholdCommandField field : SetOnDemandRatingThresholdCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum SetScheduledContentRatingThresholdCommandField {Rating(0),;
                    private final int id;
                    SetScheduledContentRatingThresholdCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SetScheduledContentRatingThresholdCommandField value(int id) throws NoSuchFieldError {
                        for (SetScheduledContentRatingThresholdCommandField field : SetScheduledContentRatingThresholdCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class ContentAppObserver implements BaseCluster {
        public static final long ID = 1296L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            ContentAppMessage(0L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum ContentAppMessageCommandField {Data(0),EncodingHint(1),;
                    private final int id;
                    ContentAppMessageCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ContentAppMessageCommandField value(int id) throws NoSuchFieldError {
                        for (ContentAppMessageCommandField field : ContentAppMessageCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class ZoneManagement implements BaseCluster {
        public static final long ID = 1360L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            MaxUserDefinedZones(0L),
            MaxZones(1L),
            Zones(2L),
            Triggers(3L),
            SensitivityMax(4L),
            Sensitivity(5L),
            TwoDCartesianMax(6L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            ZoneTriggered(0L),
            ZoneStopped(1L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            CreateTwoDCartesianZone(0L),
            UpdateTwoDCartesianZone(2L),
            RemoveZone(3L),
            CreateOrUpdateTrigger(4L),
            RemoveTrigger(5L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum CreateTwoDCartesianZoneCommandField {Zone(0),;
                    private final int id;
                    CreateTwoDCartesianZoneCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static CreateTwoDCartesianZoneCommandField value(int id) throws NoSuchFieldError {
                        for (CreateTwoDCartesianZoneCommandField field : CreateTwoDCartesianZoneCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum UpdateTwoDCartesianZoneCommandField {ZoneID(0),Zone(1),;
                    private final int id;
                    UpdateTwoDCartesianZoneCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static UpdateTwoDCartesianZoneCommandField value(int id) throws NoSuchFieldError {
                        for (UpdateTwoDCartesianZoneCommandField field : UpdateTwoDCartesianZoneCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum RemoveZoneCommandField {ZoneID(0),;
                    private final int id;
                    RemoveZoneCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static RemoveZoneCommandField value(int id) throws NoSuchFieldError {
                        for (RemoveZoneCommandField field : RemoveZoneCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum CreateOrUpdateTriggerCommandField {Trigger(0),;
                    private final int id;
                    CreateOrUpdateTriggerCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static CreateOrUpdateTriggerCommandField value(int id) throws NoSuchFieldError {
                        for (CreateOrUpdateTriggerCommandField field : CreateOrUpdateTriggerCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum RemoveTriggerCommandField {ZoneID(0),;
                    private final int id;
                    RemoveTriggerCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static RemoveTriggerCommandField value(int id) throws NoSuchFieldError {
                        for (RemoveTriggerCommandField field : RemoveTriggerCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class CameraAvStreamManagement implements BaseCluster {
        public static final long ID = 1361L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            MaxConcurrentEncoders(0L),
            MaxEncodedPixelRate(1L),
            VideoSensorParams(2L),
            NightVisionUsesInfrared(3L),
            MinViewportResolution(4L),
            RateDistortionTradeOffPoints(5L),
            MaxContentBufferSize(6L),
            MicrophoneCapabilities(7L),
            SpeakerCapabilities(8L),
            TwoWayTalkSupport(9L),
            SnapshotCapabilities(10L),
            MaxNetworkBandwidth(11L),
            CurrentFrameRate(12L),
            HDRModeEnabled(13L),
            SupportedStreamUsages(14L),
            AllocatedVideoStreams(15L),
            AllocatedAudioStreams(16L),
            AllocatedSnapshotStreams(17L),
            StreamUsagePriorities(18L),
            SoftRecordingPrivacyModeEnabled(19L),
            SoftLivestreamPrivacyModeEnabled(20L),
            HardPrivacyModeOn(21L),
            NightVision(22L),
            NightVisionIllum(23L),
            Viewport(24L),
            SpeakerMuted(25L),
            SpeakerVolumeLevel(26L),
            SpeakerMaxLevel(27L),
            SpeakerMinLevel(28L),
            MicrophoneMuted(29L),
            MicrophoneVolumeLevel(30L),
            MicrophoneMaxLevel(31L),
            MicrophoneMinLevel(32L),
            MicrophoneAGCEnabled(33L),
            ImageRotation(34L),
            ImageFlipHorizontal(35L),
            ImageFlipVertical(36L),
            LocalVideoRecordingEnabled(37L),
            LocalSnapshotRecordingEnabled(38L),
            StatusLightEnabled(39L),
            StatusLightBrightness(40L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            AudioStreamAllocate(0L),
            AudioStreamDeallocate(2L),
            VideoStreamAllocate(3L),
            VideoStreamModify(5L),
            VideoStreamDeallocate(6L),
            SnapshotStreamAllocate(7L),
            SnapshotStreamModify(9L),
            SnapshotStreamDeallocate(10L),
            SetStreamPriorities(11L),
            CaptureSnapshot(12L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum AudioStreamAllocateCommandField {StreamUsage(0),AudioCodec(1),ChannelCount(2),SampleRate(3),BitRate(4),BitDepth(5),;
                    private final int id;
                    AudioStreamAllocateCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static AudioStreamAllocateCommandField value(int id) throws NoSuchFieldError {
                        for (AudioStreamAllocateCommandField field : AudioStreamAllocateCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum AudioStreamDeallocateCommandField {AudioStreamID(0),;
                    private final int id;
                    AudioStreamDeallocateCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static AudioStreamDeallocateCommandField value(int id) throws NoSuchFieldError {
                        for (AudioStreamDeallocateCommandField field : AudioStreamDeallocateCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum VideoStreamAllocateCommandField {StreamUsage(0),VideoCodec(1),MinFrameRate(2),MaxFrameRate(3),MinResolution(4),MaxResolution(5),MinBitRate(6),MaxBitRate(7),KeyFrameInterval(8),WatermarkEnabled(9),OSDEnabled(10),;
                    private final int id;
                    VideoStreamAllocateCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static VideoStreamAllocateCommandField value(int id) throws NoSuchFieldError {
                        for (VideoStreamAllocateCommandField field : VideoStreamAllocateCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum VideoStreamModifyCommandField {VideoStreamID(0),WatermarkEnabled(1),OSDEnabled(2),;
                    private final int id;
                    VideoStreamModifyCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static VideoStreamModifyCommandField value(int id) throws NoSuchFieldError {
                        for (VideoStreamModifyCommandField field : VideoStreamModifyCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum VideoStreamDeallocateCommandField {VideoStreamID(0),;
                    private final int id;
                    VideoStreamDeallocateCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static VideoStreamDeallocateCommandField value(int id) throws NoSuchFieldError {
                        for (VideoStreamDeallocateCommandField field : VideoStreamDeallocateCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum SnapshotStreamAllocateCommandField {ImageCodec(0),MaxFrameRate(1),MinResolution(2),MaxResolution(3),Quality(4),WatermarkEnabled(5),OSDEnabled(6),;
                    private final int id;
                    SnapshotStreamAllocateCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SnapshotStreamAllocateCommandField value(int id) throws NoSuchFieldError {
                        for (SnapshotStreamAllocateCommandField field : SnapshotStreamAllocateCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum SnapshotStreamModifyCommandField {SnapshotStreamID(0),WatermarkEnabled(1),OSDEnabled(2),;
                    private final int id;
                    SnapshotStreamModifyCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SnapshotStreamModifyCommandField value(int id) throws NoSuchFieldError {
                        for (SnapshotStreamModifyCommandField field : SnapshotStreamModifyCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum SnapshotStreamDeallocateCommandField {SnapshotStreamID(0),;
                    private final int id;
                    SnapshotStreamDeallocateCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SnapshotStreamDeallocateCommandField value(int id) throws NoSuchFieldError {
                        for (SnapshotStreamDeallocateCommandField field : SnapshotStreamDeallocateCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum SetStreamPrioritiesCommandField {StreamPriorities(0),;
                    private final int id;
                    SetStreamPrioritiesCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SetStreamPrioritiesCommandField value(int id) throws NoSuchFieldError {
                        for (SetStreamPrioritiesCommandField field : SetStreamPrioritiesCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum CaptureSnapshotCommandField {SnapshotStreamID(0),RequestedResolution(1),;
                    private final int id;
                    CaptureSnapshotCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static CaptureSnapshotCommandField value(int id) throws NoSuchFieldError {
                        for (CaptureSnapshotCommandField field : CaptureSnapshotCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class CameraAvSettingsUserLevelManagement implements BaseCluster {
        public static final long ID = 1362L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            MPTZPosition(0L),
            MaxPresets(1L),
            MPTZPresets(2L),
            DPTZStreams(3L),
            ZoomMax(4L),
            TiltMin(5L),
            TiltMax(6L),
            PanMin(7L),
            PanMax(8L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            MPTZSetPosition(0L),
            MPTZRelativeMove(1L),
            MPTZMoveToPreset(2L),
            MPTZSavePreset(3L),
            MPTZRemovePreset(4L),
            DPTZSetViewport(5L),
            DPTZRelativeMove(6L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum MPTZSetPositionCommandField {Pan(0),Tilt(1),Zoom(2),;
                    private final int id;
                    MPTZSetPositionCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static MPTZSetPositionCommandField value(int id) throws NoSuchFieldError {
                        for (MPTZSetPositionCommandField field : MPTZSetPositionCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum MPTZRelativeMoveCommandField {PanDelta(0),TiltDelta(1),ZoomDelta(2),;
                    private final int id;
                    MPTZRelativeMoveCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static MPTZRelativeMoveCommandField value(int id) throws NoSuchFieldError {
                        for (MPTZRelativeMoveCommandField field : MPTZRelativeMoveCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum MPTZMoveToPresetCommandField {PresetID(0),;
                    private final int id;
                    MPTZMoveToPresetCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static MPTZMoveToPresetCommandField value(int id) throws NoSuchFieldError {
                        for (MPTZMoveToPresetCommandField field : MPTZMoveToPresetCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum MPTZSavePresetCommandField {PresetID(0),Name(1),;
                    private final int id;
                    MPTZSavePresetCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static MPTZSavePresetCommandField value(int id) throws NoSuchFieldError {
                        for (MPTZSavePresetCommandField field : MPTZSavePresetCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum MPTZRemovePresetCommandField {PresetID(0),;
                    private final int id;
                    MPTZRemovePresetCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static MPTZRemovePresetCommandField value(int id) throws NoSuchFieldError {
                        for (MPTZRemovePresetCommandField field : MPTZRemovePresetCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum DPTZSetViewportCommandField {VideoStreamID(0),Viewport(1),;
                    private final int id;
                    DPTZSetViewportCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static DPTZSetViewportCommandField value(int id) throws NoSuchFieldError {
                        for (DPTZSetViewportCommandField field : DPTZSetViewportCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum DPTZRelativeMoveCommandField {VideoStreamID(0),DeltaX(1),DeltaY(2),ZoomDelta(3),;
                    private final int id;
                    DPTZRelativeMoveCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static DPTZRelativeMoveCommandField value(int id) throws NoSuchFieldError {
                        for (DPTZRelativeMoveCommandField field : DPTZRelativeMoveCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class WebRTCTransportProvider implements BaseCluster {
        public static final long ID = 1363L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            CurrentSessions(0L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            SolicitOffer(0L),
            ProvideOffer(2L),
            ProvideAnswer(4L),
            ProvideICECandidates(5L),
            EndSession(6L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum SolicitOfferCommandField {StreamUsage(0),OriginatingEndpointID(1),VideoStreamID(2),AudioStreamID(3),ICEServers(4),ICETransportPolicy(5),MetadataEnabled(6),SFrameConfig(7),;
                    private final int id;
                    SolicitOfferCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SolicitOfferCommandField value(int id) throws NoSuchFieldError {
                        for (SolicitOfferCommandField field : SolicitOfferCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum ProvideOfferCommandField {WebRTCSessionID(0),Sdp(1),StreamUsage(2),OriginatingEndpointID(3),VideoStreamID(4),AudioStreamID(5),ICEServers(6),ICETransportPolicy(7),MetadataEnabled(8),SFrameConfig(9),;
                    private final int id;
                    ProvideOfferCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ProvideOfferCommandField value(int id) throws NoSuchFieldError {
                        for (ProvideOfferCommandField field : ProvideOfferCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum ProvideAnswerCommandField {WebRTCSessionID(0),Sdp(1),;
                    private final int id;
                    ProvideAnswerCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ProvideAnswerCommandField value(int id) throws NoSuchFieldError {
                        for (ProvideAnswerCommandField field : ProvideAnswerCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum ProvideICECandidatesCommandField {WebRTCSessionID(0),ICECandidates(1),;
                    private final int id;
                    ProvideICECandidatesCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ProvideICECandidatesCommandField value(int id) throws NoSuchFieldError {
                        for (ProvideICECandidatesCommandField field : ProvideICECandidatesCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum EndSessionCommandField {WebRTCSessionID(0),Reason(1),;
                    private final int id;
                    EndSessionCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static EndSessionCommandField value(int id) throws NoSuchFieldError {
                        for (EndSessionCommandField field : EndSessionCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class WebRTCTransportRequestor implements BaseCluster {
        public static final long ID = 1364L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            CurrentSessions(0L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            Offer(0L),
            Answer(1L),
            ICECandidates(2L),
            End(3L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum OfferCommandField {WebRTCSessionID(0),Sdp(1),ICEServers(2),ICETransportPolicy(3),;
                    private final int id;
                    OfferCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static OfferCommandField value(int id) throws NoSuchFieldError {
                        for (OfferCommandField field : OfferCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum AnswerCommandField {WebRTCSessionID(0),Sdp(1),;
                    private final int id;
                    AnswerCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static AnswerCommandField value(int id) throws NoSuchFieldError {
                        for (AnswerCommandField field : AnswerCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum ICECandidatesCommandField {WebRTCSessionID(0),ICECandidates(1),;
                    private final int id;
                    ICECandidatesCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ICECandidatesCommandField value(int id) throws NoSuchFieldError {
                        for (ICECandidatesCommandField field : ICECandidatesCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum EndCommandField {WebRTCSessionID(0),Reason(1),;
                    private final int id;
                    EndCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static EndCommandField value(int id) throws NoSuchFieldError {
                        for (EndCommandField field : EndCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class PushAvStreamTransport implements BaseCluster {
        public static final long ID = 1365L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            SupportedFormats(0L),
            CurrentConnections(1L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            PushTransportBegin(0L),
            PushTransportEnd(1L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            AllocatePushTransport(0L),
            DeallocatePushTransport(2L),
            ModifyPushTransport(3L),
            SetTransportStatus(4L),
            ManuallyTriggerTransport(5L),
            FindTransport(6L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum AllocatePushTransportCommandField {TransportOptions(0),;
                    private final int id;
                    AllocatePushTransportCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static AllocatePushTransportCommandField value(int id) throws NoSuchFieldError {
                        for (AllocatePushTransportCommandField field : AllocatePushTransportCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum DeallocatePushTransportCommandField {ConnectionID(0),;
                    private final int id;
                    DeallocatePushTransportCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static DeallocatePushTransportCommandField value(int id) throws NoSuchFieldError {
                        for (DeallocatePushTransportCommandField field : DeallocatePushTransportCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum ModifyPushTransportCommandField {ConnectionID(0),TransportOptions(1),;
                    private final int id;
                    ModifyPushTransportCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ModifyPushTransportCommandField value(int id) throws NoSuchFieldError {
                        for (ModifyPushTransportCommandField field : ModifyPushTransportCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum SetTransportStatusCommandField {ConnectionID(0),TransportStatus(1),;
                    private final int id;
                    SetTransportStatusCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SetTransportStatusCommandField value(int id) throws NoSuchFieldError {
                        for (SetTransportStatusCommandField field : SetTransportStatusCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum ManuallyTriggerTransportCommandField {ConnectionID(0),ActivationReason(1),TimeControl(2),UserDefined(3),;
                    private final int id;
                    ManuallyTriggerTransportCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ManuallyTriggerTransportCommandField value(int id) throws NoSuchFieldError {
                        for (ManuallyTriggerTransportCommandField field : ManuallyTriggerTransportCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum FindTransportCommandField {ConnectionID(0),;
                    private final int id;
                    FindTransportCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static FindTransportCommandField value(int id) throws NoSuchFieldError {
                        for (FindTransportCommandField field : FindTransportCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class Chime implements BaseCluster {
        public static final long ID = 1366L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            InstalledChimeSounds(0L),
            SelectedChime(1L),
            Enabled(2L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            PlayChimeSound(0L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class CommodityTariff implements BaseCluster {
        public static final long ID = 1792L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            TariffInfo(0L),
            TariffUnit(1L),
            StartDate(2L),
            DayEntries(3L),
            DayPatterns(4L),
            CalendarPeriods(5L),
            IndividualDays(6L),
            CurrentDay(7L),
            NextDay(8L),
            CurrentDayEntry(9L),
            CurrentDayEntryDate(10L),
            NextDayEntry(11L),
            NextDayEntryDate(12L),
            TariffComponents(13L),
            TariffPeriods(14L),
            CurrentTariffComponents(15L),
            NextTariffComponents(16L),
            DefaultRandomizationOffset(17L),
            DefaultRandomizationType(18L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            GetTariffComponent(0L),
            GetDayEntry(1L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum GetTariffComponentCommandField {TariffComponentID(0),;
                    private final int id;
                    GetTariffComponentCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static GetTariffComponentCommandField value(int id) throws NoSuchFieldError {
                        for (GetTariffComponentCommandField field : GetTariffComponentCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum GetDayEntryCommandField {DayEntryID(0),;
                    private final int id;
                    GetDayEntryCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static GetDayEntryCommandField value(int id) throws NoSuchFieldError {
                        for (GetDayEntryCommandField field : GetDayEntryCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class EcosystemInformation implements BaseCluster {
        public static final long ID = 1872L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            DeviceDirectory(0L),
            LocationDirectory(1L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class CommissionerControl implements BaseCluster {
        public static final long ID = 1873L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            SupportedDeviceCategories(0L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            CommissioningRequestResult(0L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            RequestCommissioningApproval(0L),
            CommissionNode(1L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum RequestCommissioningApprovalCommandField {RequestID(0),VendorID(1),ProductID(2),Label(3),;
                    private final int id;
                    RequestCommissioningApprovalCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static RequestCommissioningApprovalCommandField value(int id) throws NoSuchFieldError {
                        for (RequestCommissioningApprovalCommandField field : RequestCommissioningApprovalCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum CommissionNodeCommandField {RequestID(0),ResponseTimeoutSeconds(1),;
                    private final int id;
                    CommissionNodeCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static CommissionNodeCommandField value(int id) throws NoSuchFieldError {
                        for (CommissionNodeCommandField field : CommissionNodeCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class JointFabricDatastore implements BaseCluster {
        public static final long ID = 1874L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            AnchorRootCA(0L),
            AnchorNodeID(1L),
            AnchorVendorID(2L),
            FriendlyName(3L),
            GroupKeySetList(4L),
            GroupList(5L),
            NodeList(6L),
            AdminList(7L),
            Status(8L),
            EndpointGroupIDList(9L),
            EndpointBindingList(10L),
            NodeKeySetList(11L),
            NodeACLList(12L),
            NodeEndpointList(13L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            AddKeySet(0L),
            UpdateKeySet(1L),
            RemoveKeySet(2L),
            AddGroup(3L),
            UpdateGroup(4L),
            RemoveGroup(5L),
            AddAdmin(6L),
            UpdateAdmin(7L),
            RemoveAdmin(8L),
            AddPendingNode(9L),
            RefreshNode(10L),
            UpdateNode(11L),
            RemoveNode(12L),
            UpdateEndpointForNode(13L),
            AddGroupIDToEndpointForNode(14L),
            RemoveGroupIDFromEndpointForNode(15L),
            AddBindingToEndpointForNode(16L),
            RemoveBindingFromEndpointForNode(17L),
            AddACLToNode(18L),
            RemoveACLFromNode(19L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum AddKeySetCommandField {GroupKeySet(0),;
                    private final int id;
                    AddKeySetCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static AddKeySetCommandField value(int id) throws NoSuchFieldError {
                        for (AddKeySetCommandField field : AddKeySetCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum UpdateKeySetCommandField {GroupKeySet(0),;
                    private final int id;
                    UpdateKeySetCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static UpdateKeySetCommandField value(int id) throws NoSuchFieldError {
                        for (UpdateKeySetCommandField field : UpdateKeySetCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum RemoveKeySetCommandField {GroupKeySetID(0),;
                    private final int id;
                    RemoveKeySetCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static RemoveKeySetCommandField value(int id) throws NoSuchFieldError {
                        for (RemoveKeySetCommandField field : RemoveKeySetCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum AddGroupCommandField {GroupID(0),FriendlyName(1),GroupKeySetID(2),GroupCAT(3),GroupCATVersion(4),GroupPermission(5),;
                    private final int id;
                    AddGroupCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static AddGroupCommandField value(int id) throws NoSuchFieldError {
                        for (AddGroupCommandField field : AddGroupCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum UpdateGroupCommandField {GroupID(0),FriendlyName(1),GroupKeySetID(2),GroupCAT(3),GroupCATVersion(4),GroupPermission(5),;
                    private final int id;
                    UpdateGroupCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static UpdateGroupCommandField value(int id) throws NoSuchFieldError {
                        for (UpdateGroupCommandField field : UpdateGroupCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum RemoveGroupCommandField {GroupID(0),;
                    private final int id;
                    RemoveGroupCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static RemoveGroupCommandField value(int id) throws NoSuchFieldError {
                        for (RemoveGroupCommandField field : RemoveGroupCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum AddAdminCommandField {NodeID(0),FriendlyName(1),VendorID(2),Icac(3),;
                    private final int id;
                    AddAdminCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static AddAdminCommandField value(int id) throws NoSuchFieldError {
                        for (AddAdminCommandField field : AddAdminCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum UpdateAdminCommandField {NodeID(0),FriendlyName(1),Icac(2),;
                    private final int id;
                    UpdateAdminCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static UpdateAdminCommandField value(int id) throws NoSuchFieldError {
                        for (UpdateAdminCommandField field : UpdateAdminCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum RemoveAdminCommandField {NodeID(0),;
                    private final int id;
                    RemoveAdminCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static RemoveAdminCommandField value(int id) throws NoSuchFieldError {
                        for (RemoveAdminCommandField field : RemoveAdminCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum AddPendingNodeCommandField {NodeID(0),FriendlyName(1),;
                    private final int id;
                    AddPendingNodeCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static AddPendingNodeCommandField value(int id) throws NoSuchFieldError {
                        for (AddPendingNodeCommandField field : AddPendingNodeCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum RefreshNodeCommandField {NodeID(0),;
                    private final int id;
                    RefreshNodeCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static RefreshNodeCommandField value(int id) throws NoSuchFieldError {
                        for (RefreshNodeCommandField field : RefreshNodeCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum UpdateNodeCommandField {NodeID(0),FriendlyName(1),;
                    private final int id;
                    UpdateNodeCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static UpdateNodeCommandField value(int id) throws NoSuchFieldError {
                        for (UpdateNodeCommandField field : UpdateNodeCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum RemoveNodeCommandField {NodeID(0),;
                    private final int id;
                    RemoveNodeCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static RemoveNodeCommandField value(int id) throws NoSuchFieldError {
                        for (RemoveNodeCommandField field : RemoveNodeCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum UpdateEndpointForNodeCommandField {EndpointID(0),NodeID(1),FriendlyName(2),;
                    private final int id;
                    UpdateEndpointForNodeCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static UpdateEndpointForNodeCommandField value(int id) throws NoSuchFieldError {
                        for (UpdateEndpointForNodeCommandField field : UpdateEndpointForNodeCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum AddGroupIDToEndpointForNodeCommandField {NodeID(0),EndpointID(1),GroupID(2),;
                    private final int id;
                    AddGroupIDToEndpointForNodeCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static AddGroupIDToEndpointForNodeCommandField value(int id) throws NoSuchFieldError {
                        for (AddGroupIDToEndpointForNodeCommandField field : AddGroupIDToEndpointForNodeCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum RemoveGroupIDFromEndpointForNodeCommandField {NodeID(0),EndpointID(1),GroupID(2),;
                    private final int id;
                    RemoveGroupIDFromEndpointForNodeCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static RemoveGroupIDFromEndpointForNodeCommandField value(int id) throws NoSuchFieldError {
                        for (RemoveGroupIDFromEndpointForNodeCommandField field : RemoveGroupIDFromEndpointForNodeCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum AddBindingToEndpointForNodeCommandField {NodeID(0),EndpointID(1),Binding(2),;
                    private final int id;
                    AddBindingToEndpointForNodeCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static AddBindingToEndpointForNodeCommandField value(int id) throws NoSuchFieldError {
                        for (AddBindingToEndpointForNodeCommandField field : AddBindingToEndpointForNodeCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum RemoveBindingFromEndpointForNodeCommandField {ListID(0),EndpointID(1),NodeID(2),;
                    private final int id;
                    RemoveBindingFromEndpointForNodeCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static RemoveBindingFromEndpointForNodeCommandField value(int id) throws NoSuchFieldError {
                        for (RemoveBindingFromEndpointForNodeCommandField field : RemoveBindingFromEndpointForNodeCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum AddACLToNodeCommandField {NodeID(0),ACLEntry(1),;
                    private final int id;
                    AddACLToNodeCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static AddACLToNodeCommandField value(int id) throws NoSuchFieldError {
                        for (AddACLToNodeCommandField field : AddACLToNodeCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum RemoveACLFromNodeCommandField {ListID(0),NodeID(1),;
                    private final int id;
                    RemoveACLFromNodeCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static RemoveACLFromNodeCommandField value(int id) throws NoSuchFieldError {
                        for (RemoveACLFromNodeCommandField field : RemoveACLFromNodeCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class JointFabricAdministrator implements BaseCluster {
        public static final long ID = 1875L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            AdministratorFabricIndex(0L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            ICACCSRRequest(0L),
            AddICAC(2L),
            OpenJointCommissioningWindow(4L),
            TransferAnchorRequest(5L),
            TransferAnchorComplete(7L),
            AnnounceJointFabricAdministrator(8L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum AddICACCommandField {ICACValue(0),;
                    private final int id;
                    AddICACCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static AddICACCommandField value(int id) throws NoSuchFieldError {
                        for (AddICACCommandField field : AddICACCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum OpenJointCommissioningWindowCommandField {CommissioningTimeout(0),PAKEPasscodeVerifier(1),Discriminator(2),Iterations(3),Salt(4),;
                    private final int id;
                    OpenJointCommissioningWindowCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static OpenJointCommissioningWindowCommandField value(int id) throws NoSuchFieldError {
                        for (OpenJointCommissioningWindowCommandField field : OpenJointCommissioningWindowCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum AnnounceJointFabricAdministratorCommandField {EndpointID(0),;
                    private final int id;
                    AnnounceJointFabricAdministratorCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static AnnounceJointFabricAdministratorCommandField value(int id) throws NoSuchFieldError {
                        for (AnnounceJointFabricAdministratorCommandField field : AnnounceJointFabricAdministratorCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class TlsCertificateManagement implements BaseCluster {
        public static final long ID = 2049L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            MaxRootCertificates(0L),
            ProvisionedRootCertificates(1L),
            MaxClientCertificates(2L),
            ProvisionedClientCertificates(3L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            ProvisionRootCertificate(0L),
            FindRootCertificate(2L),
            LookupRootCertificate(4L),
            RemoveRootCertificate(6L),
            TLSClientCSR(7L),
            ProvisionClientCertificate(9L),
            FindClientCertificate(10L),
            LookupClientCertificate(12L),
            RemoveClientCertificate(14L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum ProvisionRootCertificateCommandField {Certificate(0),Caid(1),;
                    private final int id;
                    ProvisionRootCertificateCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ProvisionRootCertificateCommandField value(int id) throws NoSuchFieldError {
                        for (ProvisionRootCertificateCommandField field : ProvisionRootCertificateCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum FindRootCertificateCommandField {Caid(0),;
                    private final int id;
                    FindRootCertificateCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static FindRootCertificateCommandField value(int id) throws NoSuchFieldError {
                        for (FindRootCertificateCommandField field : FindRootCertificateCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum LookupRootCertificateCommandField {Fingerprint(0),;
                    private final int id;
                    LookupRootCertificateCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static LookupRootCertificateCommandField value(int id) throws NoSuchFieldError {
                        for (LookupRootCertificateCommandField field : LookupRootCertificateCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum RemoveRootCertificateCommandField {Caid(0),;
                    private final int id;
                    RemoveRootCertificateCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static RemoveRootCertificateCommandField value(int id) throws NoSuchFieldError {
                        for (RemoveRootCertificateCommandField field : RemoveRootCertificateCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum TLSClientCSRCommandField {Nonce(0),;
                    private final int id;
                    TLSClientCSRCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static TLSClientCSRCommandField value(int id) throws NoSuchFieldError {
                        for (TLSClientCSRCommandField field : TLSClientCSRCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum ProvisionClientCertificateCommandField {Ccdid(0),ClientCertificateDetails(1),;
                    private final int id;
                    ProvisionClientCertificateCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ProvisionClientCertificateCommandField value(int id) throws NoSuchFieldError {
                        for (ProvisionClientCertificateCommandField field : ProvisionClientCertificateCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum FindClientCertificateCommandField {Ccdid(0),;
                    private final int id;
                    FindClientCertificateCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static FindClientCertificateCommandField value(int id) throws NoSuchFieldError {
                        for (FindClientCertificateCommandField field : FindClientCertificateCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum LookupClientCertificateCommandField {Fingerprint(0),;
                    private final int id;
                    LookupClientCertificateCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static LookupClientCertificateCommandField value(int id) throws NoSuchFieldError {
                        for (LookupClientCertificateCommandField field : LookupClientCertificateCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum RemoveClientCertificateCommandField {Ccdid(0),;
                    private final int id;
                    RemoveClientCertificateCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static RemoveClientCertificateCommandField value(int id) throws NoSuchFieldError {
                        for (RemoveClientCertificateCommandField field : RemoveClientCertificateCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class TlsClientManagement implements BaseCluster {
        public static final long ID = 2050L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            MaxProvisioned(0L),
            ProvisionedEndpoints(1L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            ProvisionEndpoint(0L),
            FindEndpoint(2L),
            RemoveEndpoint(4L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum ProvisionEndpointCommandField {Hostname(0),Port(1),Caid(2),Ccdid(3),EndpointID(4),;
                    private final int id;
                    ProvisionEndpointCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static ProvisionEndpointCommandField value(int id) throws NoSuchFieldError {
                        for (ProvisionEndpointCommandField field : ProvisionEndpointCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum FindEndpointCommandField {EndpointID(0),;
                    private final int id;
                    FindEndpointCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static FindEndpointCommandField value(int id) throws NoSuchFieldError {
                        for (FindEndpointCommandField field : FindEndpointCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum RemoveEndpointCommandField {EndpointID(0),;
                    private final int id;
                    RemoveEndpointCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static RemoveEndpointCommandField value(int id) throws NoSuchFieldError {
                        for (RemoveEndpointCommandField field : RemoveEndpointCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class MeterIdentification implements BaseCluster {
        public static final long ID = 2822L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            MeterType(0L),
            PointOfDelivery(1L),
            MeterSerialNumber(2L),
            ProtocolVersion(3L),
            PowerThreshold(4L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class CommodityMetering implements BaseCluster {
        public static final long ID = 2823L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            MeteredQuantity(0L),
            MeteredQuantityTimestamp(1L),
            TariffUnit(2L),
            MaximumMeteredQuantities(3L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class UnitTesting implements BaseCluster {
        public static final long ID = 4294048773L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            Boolean(0L),
            Bitmap8(1L),
            Bitmap16(2L),
            Bitmap32(3L),
            Bitmap64(4L),
            Int8u(5L),
            Int16u(6L),
            Int24u(7L),
            Int32u(8L),
            Int40u(9L),
            Int48u(10L),
            Int56u(11L),
            Int64u(12L),
            Int8s(13L),
            Int16s(14L),
            Int24s(15L),
            Int32s(16L),
            Int40s(17L),
            Int48s(18L),
            Int56s(19L),
            Int64s(20L),
            Enum8(21L),
            Enum16(22L),
            FloatSingle(23L),
            FloatDouble(24L),
            OctetString(25L),
            ListInt8u(26L),
            ListOctetString(27L),
            ListStructOctetString(28L),
            LongOctetString(29L),
            CharString(30L),
            LongCharString(31L),
            EpochUs(32L),
            EpochS(33L),
            VendorId(34L),
            ListNullablesAndOptionalsStruct(35L),
            EnumAttr(36L),
            StructAttr(37L),
            RangeRestrictedInt8u(38L),
            RangeRestrictedInt8s(39L),
            RangeRestrictedInt16u(40L),
            RangeRestrictedInt16s(41L),
            ListLongOctetString(42L),
            ListFabricScoped(43L),
            TimedWriteBoolean(48L),
            GeneralErrorBoolean(49L),
            ClusterErrorBoolean(50L),
            GlobalEnum(51L),
            GlobalStruct(52L),
            UnsupportedAttributeRequiringAdminPrivilege(254L),
            Unsupported(255L),
            ReadFailureCode(12288L),
            FailureInt32U(12289L),
            NullableBoolean(16384L),
            NullableBitmap8(16385L),
            NullableBitmap16(16386L),
            NullableBitmap32(16387L),
            NullableBitmap64(16388L),
            NullableInt8u(16389L),
            NullableInt16u(16390L),
            NullableInt24u(16391L),
            NullableInt32u(16392L),
            NullableInt40u(16393L),
            NullableInt48u(16394L),
            NullableInt56u(16395L),
            NullableInt64u(16396L),
            NullableInt8s(16397L),
            NullableInt16s(16398L),
            NullableInt24s(16399L),
            NullableInt32s(16400L),
            NullableInt40s(16401L),
            NullableInt48s(16402L),
            NullableInt56s(16403L),
            NullableInt64s(16404L),
            NullableEnum8(16405L),
            NullableEnum16(16406L),
            NullableFloatSingle(16407L),
            NullableFloatDouble(16408L),
            NullableOctetString(16409L),
            NullableCharString(16414L),
            NullableEnumAttr(16420L),
            NullableStruct(16421L),
            NullableRangeRestrictedInt8u(16422L),
            NullableRangeRestrictedInt8s(16423L),
            NullableRangeRestrictedInt16u(16424L),
            NullableRangeRestrictedInt16s(16425L),
            WriteOnlyInt8u(16426L),
            NullableGlobalEnum(16435L),
            NullableGlobalStruct(16436L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),
            MeiInt8u(4294070017L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            TestEvent(1L),
            TestFabricScopedEvent(2L),
            TestDifferentVendorMeiEvent(4294050030L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            Test(0L),
            TestNotHandled(1L),
            TestSpecific(2L),
            TestUnknownCommand(3L),
            TestAddArguments(4L),
            TestSimpleArgumentRequest(5L),
            TestStructArrayArgumentRequest(6L),
            TestStructArgumentRequest(7L),
            TestNestedStructArgumentRequest(8L),
            TestListStructArgumentRequest(9L),
            TestListInt8UArgumentRequest(10L),
            TestNestedStructListArgumentRequest(11L),
            TestListNestedStructListArgumentRequest(12L),
            TestListInt8UReverseRequest(13L),
            TestEnumsRequest(14L),
            TestNullableOptionalRequest(15L),
            TestComplexNullableOptionalRequest(16L),
            SimpleStructEchoRequest(17L),
            TimedInvokeRequest(18L),
            TestSimpleOptionalArgumentRequest(19L),
            TestEmitTestEventRequest(20L),
            TestEmitTestFabricScopedEventRequest(21L),
            TestBatchHelperRequest(22L),
            TestSecondBatchHelperRequest(23L),
            StringEchoRequest(24L),
            GlobalEchoRequest(25L),
            TestCheckCommandFlags(26L),
            TestDifferentVendorMeiRequest(4294049962L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum TestAddArgumentsCommandField {Arg1(0),Arg2(1),;
                    private final int id;
                    TestAddArgumentsCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static TestAddArgumentsCommandField value(int id) throws NoSuchFieldError {
                        for (TestAddArgumentsCommandField field : TestAddArgumentsCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum TestSimpleArgumentRequestCommandField {Arg1(0),;
                    private final int id;
                    TestSimpleArgumentRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static TestSimpleArgumentRequestCommandField value(int id) throws NoSuchFieldError {
                        for (TestSimpleArgumentRequestCommandField field : TestSimpleArgumentRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum TestStructArrayArgumentRequestCommandField {Arg1(0),Arg2(1),Arg3(2),Arg4(3),Arg5(4),Arg6(5),;
                    private final int id;
                    TestStructArrayArgumentRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static TestStructArrayArgumentRequestCommandField value(int id) throws NoSuchFieldError {
                        for (TestStructArrayArgumentRequestCommandField field : TestStructArrayArgumentRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum TestStructArgumentRequestCommandField {Arg1(0),;
                    private final int id;
                    TestStructArgumentRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static TestStructArgumentRequestCommandField value(int id) throws NoSuchFieldError {
                        for (TestStructArgumentRequestCommandField field : TestStructArgumentRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum TestNestedStructArgumentRequestCommandField {Arg1(0),;
                    private final int id;
                    TestNestedStructArgumentRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static TestNestedStructArgumentRequestCommandField value(int id) throws NoSuchFieldError {
                        for (TestNestedStructArgumentRequestCommandField field : TestNestedStructArgumentRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum TestListStructArgumentRequestCommandField {Arg1(0),;
                    private final int id;
                    TestListStructArgumentRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static TestListStructArgumentRequestCommandField value(int id) throws NoSuchFieldError {
                        for (TestListStructArgumentRequestCommandField field : TestListStructArgumentRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum TestListInt8UArgumentRequestCommandField {Arg1(0),;
                    private final int id;
                    TestListInt8UArgumentRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static TestListInt8UArgumentRequestCommandField value(int id) throws NoSuchFieldError {
                        for (TestListInt8UArgumentRequestCommandField field : TestListInt8UArgumentRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum TestNestedStructListArgumentRequestCommandField {Arg1(0),;
                    private final int id;
                    TestNestedStructListArgumentRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static TestNestedStructListArgumentRequestCommandField value(int id) throws NoSuchFieldError {
                        for (TestNestedStructListArgumentRequestCommandField field : TestNestedStructListArgumentRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum TestListNestedStructListArgumentRequestCommandField {Arg1(0),;
                    private final int id;
                    TestListNestedStructListArgumentRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static TestListNestedStructListArgumentRequestCommandField value(int id) throws NoSuchFieldError {
                        for (TestListNestedStructListArgumentRequestCommandField field : TestListNestedStructListArgumentRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum TestListInt8UReverseRequestCommandField {Arg1(0),;
                    private final int id;
                    TestListInt8UReverseRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static TestListInt8UReverseRequestCommandField value(int id) throws NoSuchFieldError {
                        for (TestListInt8UReverseRequestCommandField field : TestListInt8UReverseRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum TestEnumsRequestCommandField {Arg1(0),Arg2(1),;
                    private final int id;
                    TestEnumsRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static TestEnumsRequestCommandField value(int id) throws NoSuchFieldError {
                        for (TestEnumsRequestCommandField field : TestEnumsRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum TestNullableOptionalRequestCommandField {Arg1(0),;
                    private final int id;
                    TestNullableOptionalRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static TestNullableOptionalRequestCommandField value(int id) throws NoSuchFieldError {
                        for (TestNullableOptionalRequestCommandField field : TestNullableOptionalRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum TestComplexNullableOptionalRequestCommandField {NullableInt(0),OptionalInt(1),NullableOptionalInt(2),NullableString(3),OptionalString(4),NullableOptionalString(5),NullableStruct(6),OptionalStruct(7),NullableOptionalStruct(8),NullableList(9),OptionalList(10),NullableOptionalList(11),;
                    private final int id;
                    TestComplexNullableOptionalRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static TestComplexNullableOptionalRequestCommandField value(int id) throws NoSuchFieldError {
                        for (TestComplexNullableOptionalRequestCommandField field : TestComplexNullableOptionalRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum SimpleStructEchoRequestCommandField {Arg1(0),;
                    private final int id;
                    SimpleStructEchoRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static SimpleStructEchoRequestCommandField value(int id) throws NoSuchFieldError {
                        for (SimpleStructEchoRequestCommandField field : SimpleStructEchoRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum TestSimpleOptionalArgumentRequestCommandField {Arg1(0),;
                    private final int id;
                    TestSimpleOptionalArgumentRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static TestSimpleOptionalArgumentRequestCommandField value(int id) throws NoSuchFieldError {
                        for (TestSimpleOptionalArgumentRequestCommandField field : TestSimpleOptionalArgumentRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum TestEmitTestEventRequestCommandField {Arg1(0),Arg2(1),Arg3(2),;
                    private final int id;
                    TestEmitTestEventRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static TestEmitTestEventRequestCommandField value(int id) throws NoSuchFieldError {
                        for (TestEmitTestEventRequestCommandField field : TestEmitTestEventRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum TestEmitTestFabricScopedEventRequestCommandField {Arg1(0),;
                    private final int id;
                    TestEmitTestFabricScopedEventRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static TestEmitTestFabricScopedEventRequestCommandField value(int id) throws NoSuchFieldError {
                        for (TestEmitTestFabricScopedEventRequestCommandField field : TestEmitTestFabricScopedEventRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum TestBatchHelperRequestCommandField {SleepBeforeResponseTimeMs(0),SizeOfResponseBuffer(1),FillCharacter(2),;
                    private final int id;
                    TestBatchHelperRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static TestBatchHelperRequestCommandField value(int id) throws NoSuchFieldError {
                        for (TestBatchHelperRequestCommandField field : TestBatchHelperRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum TestSecondBatchHelperRequestCommandField {SleepBeforeResponseTimeMs(0),SizeOfResponseBuffer(1),FillCharacter(2),;
                    private final int id;
                    TestSecondBatchHelperRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static TestSecondBatchHelperRequestCommandField value(int id) throws NoSuchFieldError {
                        for (TestSecondBatchHelperRequestCommandField field : TestSecondBatchHelperRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum StringEchoRequestCommandField {Payload(0),;
                    private final int id;
                    StringEchoRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static StringEchoRequestCommandField value(int id) throws NoSuchFieldError {
                        for (StringEchoRequestCommandField field : StringEchoRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum GlobalEchoRequestCommandField {Field1(0),Field2(1),;
                    private final int id;
                    GlobalEchoRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static GlobalEchoRequestCommandField value(int id) throws NoSuchFieldError {
                        for (GlobalEchoRequestCommandField field : GlobalEchoRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum TestDifferentVendorMeiRequestCommandField {Arg1(0),;
                    private final int id;
                    TestDifferentVendorMeiRequestCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static TestDifferentVendorMeiRequestCommandField value(int id) throws NoSuchFieldError {
                        for (TestDifferentVendorMeiRequestCommandField field : TestDifferentVendorMeiRequestCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class FaultInjection implements BaseCluster {
        public static final long ID = 4294048774L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            FailAtFault(0L),
            FailRandomlyAtFault(1L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum FailAtFaultCommandField {Type(0),Id(1),NumCallsToSkip(2),NumCallsToFail(3),TakeMutex(4),;
                    private final int id;
                    FailAtFaultCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static FailAtFaultCommandField value(int id) throws NoSuchFieldError {
                        for (FailAtFaultCommandField field : FailAtFaultCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }public enum FailRandomlyAtFaultCommandField {Type(0),Id(1),Percentage(2),;
                    private final int id;
                    FailRandomlyAtFaultCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static FailRandomlyAtFaultCommandField value(int id) throws NoSuchFieldError {
                        for (FailRandomlyAtFaultCommandField field : FailRandomlyAtFaultCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }
    public static class SampleMei implements BaseCluster {
        public static final long ID = 4294048800L;
        public long getID() {
            return ID;
        }

        public enum Attribute {
            FlipFlop(0L),
            GeneratedCommandList(65528L),
            AcceptedCommandList(65529L),
            AttributeList(65531L),
            FeatureMap(65532L),
            ClusterRevision(65533L),;
            private final long id;
            Attribute(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Attribute value(long id) throws NoSuchFieldError {
                for (Attribute attribute : Attribute.values()) {
                    if (attribute.getID() == id) {
                        return attribute;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Event {
            PingCountEvent(0L),;
            private final long id;
            Event(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Event value(long id) throws NoSuchFieldError {
                for (Event event : Event.values()) {
                    if (event.getID() == id) {
                        return event;
                    }
                }
                throw new NoSuchFieldError();
            }
        }

        public enum Command {
            Ping(0L),
            AddArguments(2L),;
            private final long id;
            Command(long id) {
                this.id = id;
            }

            public long getID() {
                return id;
            }

            public static Command value(long id) throws NoSuchFieldError {
                for (Command command : Command.values()) {
                    if (command.getID() == id) {
                        return command;
                    }
                }
                throw new NoSuchFieldError();
            }
        }public enum AddArgumentsCommandField {Arg1(0),Arg2(1),;
                    private final int id;
                    AddArgumentsCommandField(int id) {
                        this.id = id;
                    }

                    public int getID() {
                        return id;
                    }
                    public static AddArgumentsCommandField value(int id) throws NoSuchFieldError {
                        for (AddArgumentsCommandField field : AddArgumentsCommandField.values()) {
                        if (field.getID() == id) {
                            return field;
                        }
                        }
                        throw new NoSuchFieldError();
                    }
                }@Override
        public String getAttributeName(long id) throws NoSuchFieldError {
            return Attribute.value(id).toString();
        }

        @Override
        public String getEventName(long id) throws NoSuchFieldError {
            return Event.value(id).toString();
        }

        @Override
        public String getCommandName(long id) throws NoSuchFieldError {
            return Command.value(id).toString();
        }

        @Override
        public long getAttributeID(String name) throws IllegalArgumentException {
            return Attribute.valueOf(name).getID();
        }

        @Override
        public long getEventID(String name) throws IllegalArgumentException {
            return Event.valueOf(name).getID();
        }

        @Override
        public long getCommandID(String name) throws IllegalArgumentException {
            return Command.valueOf(name).getID();
        }
    }}
