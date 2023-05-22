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
    if (clusterId == Scenes.ID) {
      return new Scenes();
    }
    if (clusterId == OnOff.ID) {
      return new OnOff();
    }
    if (clusterId == OnOffSwitchConfiguration.ID) {
      return new OnOffSwitchConfiguration();
    }
    if (clusterId == LevelControl.ID) {
      return new LevelControl();
    }
    if (clusterId == BinaryInputBasic.ID) {
      return new BinaryInputBasic();
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
    if (clusterId == ModeSelect.ID) {
      return new ModeSelect();
    }
    if (clusterId == TemperatureControl.ID) {
      return new TemperatureControl();
    }
    if (clusterId == RefrigeratorAlarm.ID) {
      return new RefrigeratorAlarm();
    }
    if (clusterId == AirQuality.ID) {
      return new AirQuality();
    }
    if (clusterId == SmokeCoAlarm.ID) {
      return new SmokeCoAlarm();
    }
    if (clusterId == HepaFilterMonitoring.ID) {
      return new HepaFilterMonitoring();
    }
    if (clusterId == ActivatedCarbonFilterMonitoring.ID) {
      return new ActivatedCarbonFilterMonitoring();
    }
    if (clusterId == CeramicFilterMonitoring.ID) {
      return new CeramicFilterMonitoring();
    }
    if (clusterId == ElectrostaticFilterMonitoring.ID) {
      return new ElectrostaticFilterMonitoring();
    }
    if (clusterId == UvFilterMonitoring.ID) {
      return new UvFilterMonitoring();
    }
    if (clusterId == IonizingFilterMonitoring.ID) {
      return new IonizingFilterMonitoring();
    }
    if (clusterId == ZeoliteFilterMonitoring.ID) {
      return new ZeoliteFilterMonitoring();
    }
    if (clusterId == OzoneFilterMonitoring.ID) {
      return new OzoneFilterMonitoring();
    }
    if (clusterId == WaterTankMonitoring.ID) {
      return new WaterTankMonitoring();
    }
    if (clusterId == FuelTankMonitoring.ID) {
      return new FuelTankMonitoring();
    }
    if (clusterId == InkCartridgeMonitoring.ID) {
      return new InkCartridgeMonitoring();
    }
    if (clusterId == TonerCartridgeMonitoring.ID) {
      return new TonerCartridgeMonitoring();
    }
    if (clusterId == DoorLock.ID) {
      return new DoorLock();
    }
    if (clusterId == WindowCovering.ID) {
      return new WindowCovering();
    }
    if (clusterId == BarrierControl.ID) {
      return new BarrierControl();
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
    if (clusterId == ElectricalMeasurement.ID) {
      return new ElectricalMeasurement();
    }
    if (clusterId == UnitTesting.ID) {
      return new UnitTesting();
    }
    if (clusterId == FaultInjection.ID) {
      return new FaultInjection();
    }
    return null;
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      TriggerEffect(64L),
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      AddGroupIfIdentifying(5L),
      ;
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
    }

    @Override
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

  public static class Scenes implements BaseCluster {
    public static final long ID = 5L;

    public long getID() {
      return ID;
    }

    public enum Attribute {
      SceneCount(0L),
      CurrentScene(1L),
      CurrentGroup(2L),
      SceneValid(3L),
      NameSupport(4L),
      LastConfiguredBy(5L),
      GeneratedCommandList(65528L),
      AcceptedCommandList(65529L),
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      EnhancedAddScene(64L),
      EnhancedViewScene(65L),
      CopyScene(66L),
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      OnWithTimedOff(66L),
      ;
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
    }

    @Override
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

  public static class OnOffSwitchConfiguration implements BaseCluster {
    public static final long ID = 7L;

    public long getID() {
      return ID;
    }

    public enum Attribute {
      SwitchType(0L),
      SwitchActions(16L),
      GeneratedCommandList(65528L),
      AcceptedCommandList(65529L),
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      MoveToClosestFrequency(8L),
      ;
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
    }

    @Override
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

  public static class BinaryInputBasic implements BaseCluster {
    public static final long ID = 15L;

    public long getID() {
      return ID;
    }

    public enum Attribute {
      ActiveText(4L),
      Description(28L),
      InactiveText(46L),
      OutOfService(81L),
      Polarity(84L),
      PresentValue(85L),
      Reliability(103L),
      StatusFlags(111L),
      ApplicationType(256L),
      GeneratedCommandList(65528L),
      AcceptedCommandList(65529L),
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ;
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
    }

    @Override
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
      GeneratedCommandList(65528L),
      AcceptedCommandList(65529L),
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ;
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
    }

    @Override
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
      GeneratedCommandList(65528L),
      AcceptedCommandList(65529L),
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ActionFailed(1L),
      ;
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
      DisableActionWithDuration(11L),
      ;
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
    }

    @Override
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
      GeneratedCommandList(65528L),
      AcceptedCommandList(65529L),
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      MfgSpecificPing(0L),
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      NotifyUpdateApplied(4L),
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      DownloadError(2L),
      ;
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
      AnnounceOTAProvider(0L),
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ;
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
    }

    @Override
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
      GeneratedCommandList(65528L),
      AcceptedCommandList(65529L),
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ;
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
    }

    @Override
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
      GeneratedCommandList(65528L),
      AcceptedCommandList(65529L),
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      BatChargeFaultChange(2L),
      ;
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
      ;
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
    }

    @Override
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
      GeneratedCommandList(65528L),
      AcceptedCommandList(65529L),
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ;
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
    }

    @Override
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
      GeneratedCommandList(65528L),
      AcceptedCommandList(65529L),
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      RetrieveLogsRequest(0L),
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      BootReason(3L),
      ;
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
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      SoftwareFault(0L),
      ;
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
      ResetWatermarks(0L),
      ;
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
    }

    @Override
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
      GeneratedCommandList(65528L),
      AcceptedCommandList(65529L),
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      NetworkFaultChange(1L),
      ;
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
      ResetCounts(0L),
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ConnectionStatus(2L),
      ;
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
      ResetCounts(0L),
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ResetCounts(0L),
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      MissingTrustedTimeSource(4L),
      ;
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
      SetDefaultNTP(5L),
      ;
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
    }

    @Override
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
      GeneratedCommandList(65528L),
      AcceptedCommandList(65529L),
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      MultiPressComplete(6L),
      ;
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
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      RevokeCommissioning(2L),
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      KeySetReadAllIndices(4L),
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      StateChange(0L),
      ;
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
      ;
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
    }

    @Override
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
      IdleModeInterval(0L),
      ActiveModeInterval(1L),
      ActiveModeThreshold(2L),
      RegisteredClients(3L),
      ICDCounter(4L),
      ClientsSupportedPerFabric(5L),
      GeneratedCommandList(65528L),
      AcceptedCommandList(65529L),
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      StayActiveRequest(3L),
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ChangeToMode(0L),
      ;
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
    }

    @Override
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
      CurrentTemperatureLevelIndex(4L),
      SupportedTemperatureLevels(5L),
      GeneratedCommandList(65528L),
      AcceptedCommandList(65529L),
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      SetTemperature(0L),
      ;
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
    }

    @Override
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
      Latch(1L),
      State(2L),
      GeneratedCommandList(65528L),
      AcceptedCommandList(65529L),
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      Notify(0L),
      ;
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
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ;
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
    }

    @Override
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
      SensitivityLevel(11L),
      GeneratedCommandList(65528L),
      AcceptedCommandList(65529L),
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      AllClear(10L),
      ;
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
      SelfTestRequest(0L),
      ;
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
    }

    @Override
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
      GeneratedCommandList(65528L),
      AcceptedCommandList(65529L),
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ResetCondition(0L),
      ;
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
    }

    @Override
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
      GeneratedCommandList(65528L),
      AcceptedCommandList(65529L),
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ResetCondition(0L),
      ;
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
    }

    @Override
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

  public static class CeramicFilterMonitoring implements BaseCluster {
    public static final long ID = 115L;

    public long getID() {
      return ID;
    }

    public enum Attribute {
      Condition(0L),
      DegradationDirection(1L),
      ChangeIndication(2L),
      InPlaceIndicator(3L),
      GeneratedCommandList(65528L),
      AcceptedCommandList(65529L),
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ResetCondition(0L),
      ;
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
    }

    @Override
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

  public static class ElectrostaticFilterMonitoring implements BaseCluster {
    public static final long ID = 116L;

    public long getID() {
      return ID;
    }

    public enum Attribute {
      Condition(0L),
      DegradationDirection(1L),
      ChangeIndication(2L),
      InPlaceIndicator(3L),
      GeneratedCommandList(65528L),
      AcceptedCommandList(65529L),
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ResetCondition(0L),
      ;
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
    }

    @Override
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

  public static class UvFilterMonitoring implements BaseCluster {
    public static final long ID = 117L;

    public long getID() {
      return ID;
    }

    public enum Attribute {
      Condition(0L),
      DegradationDirection(1L),
      ChangeIndication(2L),
      InPlaceIndicator(3L),
      GeneratedCommandList(65528L),
      AcceptedCommandList(65529L),
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ResetCondition(0L),
      ;
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
    }

    @Override
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

  public static class IonizingFilterMonitoring implements BaseCluster {
    public static final long ID = 118L;

    public long getID() {
      return ID;
    }

    public enum Attribute {
      Condition(0L),
      DegradationDirection(1L),
      ChangeIndication(2L),
      InPlaceIndicator(3L),
      GeneratedCommandList(65528L),
      AcceptedCommandList(65529L),
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ResetCondition(0L),
      ;
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
    }

    @Override
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

  public static class ZeoliteFilterMonitoring implements BaseCluster {
    public static final long ID = 119L;

    public long getID() {
      return ID;
    }

    public enum Attribute {
      Condition(0L),
      DegradationDirection(1L),
      ChangeIndication(2L),
      InPlaceIndicator(3L),
      GeneratedCommandList(65528L),
      AcceptedCommandList(65529L),
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ResetCondition(0L),
      ;
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
    }

    @Override
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

  public static class OzoneFilterMonitoring implements BaseCluster {
    public static final long ID = 120L;

    public long getID() {
      return ID;
    }

    public enum Attribute {
      Condition(0L),
      DegradationDirection(1L),
      ChangeIndication(2L),
      InPlaceIndicator(3L),
      GeneratedCommandList(65528L),
      AcceptedCommandList(65529L),
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ResetCondition(0L),
      ;
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
    }

    @Override
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

  public static class WaterTankMonitoring implements BaseCluster {
    public static final long ID = 121L;

    public long getID() {
      return ID;
    }

    public enum Attribute {
      Condition(0L),
      DegradationDirection(1L),
      ChangeIndication(2L),
      InPlaceIndicator(3L),
      GeneratedCommandList(65528L),
      AcceptedCommandList(65529L),
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ResetCondition(0L),
      ;
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
    }

    @Override
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

  public static class FuelTankMonitoring implements BaseCluster {
    public static final long ID = 122L;

    public long getID() {
      return ID;
    }

    public enum Attribute {
      Condition(0L),
      DegradationDirection(1L),
      ChangeIndication(2L),
      InPlaceIndicator(3L),
      GeneratedCommandList(65528L),
      AcceptedCommandList(65529L),
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ResetCondition(0L),
      ;
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
    }

    @Override
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

  public static class InkCartridgeMonitoring implements BaseCluster {
    public static final long ID = 123L;

    public long getID() {
      return ID;
    }

    public enum Attribute {
      Condition(0L),
      DegradationDirection(1L),
      ChangeIndication(2L),
      InPlaceIndicator(3L),
      GeneratedCommandList(65528L),
      AcceptedCommandList(65529L),
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ResetCondition(0L),
      ;
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
    }

    @Override
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

  public static class TonerCartridgeMonitoring implements BaseCluster {
    public static final long ID = 124L;

    public long getID() {
      return ID;
    }

    public enum Attribute {
      Condition(0L),
      DegradationDirection(1L),
      ChangeIndication(2L),
      InPlaceIndicator(3L),
      GeneratedCommandList(65528L),
      AcceptedCommandList(65529L),
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ResetCondition(0L),
      ;
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
    }

    @Override
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
      GeneratedCommandList(65528L),
      AcceptedCommandList(65529L),
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      LockUserChange(4L),
      ;
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
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      GoToTiltPercentage(8L),
      ;
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
    }

    @Override
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

  public static class BarrierControl implements BaseCluster {
    public static final long ID = 259L;

    public long getID() {
      return ID;
    }

    public enum Attribute {
      BarrierMovingState(1L),
      BarrierSafetyStatus(2L),
      BarrierCapabilities(3L),
      BarrierOpenEvents(4L),
      BarrierCloseEvents(5L),
      BarrierCommandOpenEvents(6L),
      BarrierCommandCloseEvents(7L),
      BarrierOpenPeriod(8L),
      BarrierClosePeriod(9L),
      BarrierPosition(10L),
      GeneratedCommandList(65528L),
      AcceptedCommandList(65529L),
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      BarrierControlGoToPercent(0L),
      BarrierControlStop(1L),
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      TurbineOperation(16L),
      ;
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
      ;
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
    }

    @Override
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
      GeneratedCommandList(65528L),
      AcceptedCommandList(65529L),
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ;
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
    }

    @Override
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
      GeneratedCommandList(65528L),
      AcceptedCommandList(65529L),
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      StepColorTemperature(76L),
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ;
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
    }

    @Override
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
      GeneratedCommandList(65528L),
      AcceptedCommandList(65529L),
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      NavigateTarget(0L),
      ;
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
    }

    @Override
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
      GeneratedCommandList(65528L),
      AcceptedCommandList(65529L),
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      RenameInput(3L),
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      Sleep(0L),
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      SendKey(0L),
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      LaunchURL(1L),
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      RenameOutput(1L),
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      HideApp(2L),
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      Logout(3L),
      ;
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
    }

    @Override
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

  public static class ElectricalMeasurement implements BaseCluster {
    public static final long ID = 2820L;

    public long getID() {
      return ID;
    }

    public enum Attribute {
      MeasurementType(0L),
      DcVoltage(256L),
      DcVoltageMin(257L),
      DcVoltageMax(258L),
      DcCurrent(259L),
      DcCurrentMin(260L),
      DcCurrentMax(261L),
      DcPower(262L),
      DcPowerMin(263L),
      DcPowerMax(264L),
      DcVoltageMultiplier(512L),
      DcVoltageDivisor(513L),
      DcCurrentMultiplier(514L),
      DcCurrentDivisor(515L),
      DcPowerMultiplier(516L),
      DcPowerDivisor(517L),
      AcFrequency(768L),
      AcFrequencyMin(769L),
      AcFrequencyMax(770L),
      NeutralCurrent(771L),
      TotalActivePower(772L),
      TotalReactivePower(773L),
      TotalApparentPower(774L),
      Measured1stHarmonicCurrent(775L),
      Measured3rdHarmonicCurrent(776L),
      Measured5thHarmonicCurrent(777L),
      Measured7thHarmonicCurrent(778L),
      Measured9thHarmonicCurrent(779L),
      Measured11thHarmonicCurrent(780L),
      MeasuredPhase1stHarmonicCurrent(781L),
      MeasuredPhase3rdHarmonicCurrent(782L),
      MeasuredPhase5thHarmonicCurrent(783L),
      MeasuredPhase7thHarmonicCurrent(784L),
      MeasuredPhase9thHarmonicCurrent(785L),
      MeasuredPhase11thHarmonicCurrent(786L),
      AcFrequencyMultiplier(1024L),
      AcFrequencyDivisor(1025L),
      PowerMultiplier(1026L),
      PowerDivisor(1027L),
      HarmonicCurrentMultiplier(1028L),
      PhaseHarmonicCurrentMultiplier(1029L),
      InstantaneousVoltage(1280L),
      InstantaneousLineCurrent(1281L),
      InstantaneousActiveCurrent(1282L),
      InstantaneousReactiveCurrent(1283L),
      InstantaneousPower(1284L),
      RmsVoltage(1285L),
      RmsVoltageMin(1286L),
      RmsVoltageMax(1287L),
      RmsCurrent(1288L),
      RmsCurrentMin(1289L),
      RmsCurrentMax(1290L),
      ActivePower(1291L),
      ActivePowerMin(1292L),
      ActivePowerMax(1293L),
      ReactivePower(1294L),
      ApparentPower(1295L),
      PowerFactor(1296L),
      AverageRmsVoltageMeasurementPeriod(1297L),
      AverageRmsUnderVoltageCounter(1299L),
      RmsExtremeOverVoltagePeriod(1300L),
      RmsExtremeUnderVoltagePeriod(1301L),
      RmsVoltageSagPeriod(1302L),
      RmsVoltageSwellPeriod(1303L),
      AcVoltageMultiplier(1536L),
      AcVoltageDivisor(1537L),
      AcCurrentMultiplier(1538L),
      AcCurrentDivisor(1539L),
      AcPowerMultiplier(1540L),
      AcPowerDivisor(1541L),
      OverloadAlarmsMask(1792L),
      VoltageOverload(1793L),
      CurrentOverload(1794L),
      AcOverloadAlarmsMask(2048L),
      AcVoltageOverload(2049L),
      AcCurrentOverload(2050L),
      AcActivePowerOverload(2051L),
      AcReactivePowerOverload(2052L),
      AverageRmsOverVoltage(2053L),
      AverageRmsUnderVoltage(2054L),
      RmsExtremeOverVoltage(2055L),
      RmsExtremeUnderVoltage(2056L),
      RmsVoltageSag(2057L),
      RmsVoltageSwell(2058L),
      LineCurrentPhaseB(2305L),
      ActiveCurrentPhaseB(2306L),
      ReactiveCurrentPhaseB(2307L),
      RmsVoltagePhaseB(2309L),
      RmsVoltageMinPhaseB(2310L),
      RmsVoltageMaxPhaseB(2311L),
      RmsCurrentPhaseB(2312L),
      RmsCurrentMinPhaseB(2313L),
      RmsCurrentMaxPhaseB(2314L),
      ActivePowerPhaseB(2315L),
      ActivePowerMinPhaseB(2316L),
      ActivePowerMaxPhaseB(2317L),
      ReactivePowerPhaseB(2318L),
      ApparentPowerPhaseB(2319L),
      PowerFactorPhaseB(2320L),
      AverageRmsVoltageMeasurementPeriodPhaseB(2321L),
      AverageRmsOverVoltageCounterPhaseB(2322L),
      AverageRmsUnderVoltageCounterPhaseB(2323L),
      RmsExtremeOverVoltagePeriodPhaseB(2324L),
      RmsExtremeUnderVoltagePeriodPhaseB(2325L),
      RmsVoltageSagPeriodPhaseB(2326L),
      RmsVoltageSwellPeriodPhaseB(2327L),
      LineCurrentPhaseC(2561L),
      ActiveCurrentPhaseC(2562L),
      ReactiveCurrentPhaseC(2563L),
      RmsVoltagePhaseC(2565L),
      RmsVoltageMinPhaseC(2566L),
      RmsVoltageMaxPhaseC(2567L),
      RmsCurrentPhaseC(2568L),
      RmsCurrentMinPhaseC(2569L),
      RmsCurrentMaxPhaseC(2570L),
      ActivePowerPhaseC(2571L),
      ActivePowerMinPhaseC(2572L),
      ActivePowerMaxPhaseC(2573L),
      ReactivePowerPhaseC(2574L),
      ApparentPowerPhaseC(2575L),
      PowerFactorPhaseC(2576L),
      AverageRmsVoltageMeasurementPeriodPhaseC(2577L),
      AverageRmsOverVoltageCounterPhaseC(2578L),
      AverageRmsUnderVoltageCounterPhaseC(2579L),
      RmsExtremeOverVoltagePeriodPhaseC(2580L),
      RmsExtremeUnderVoltagePeriodPhaseC(2581L),
      RmsVoltageSagPeriodPhaseC(2582L),
      RmsVoltageSwellPeriodPhaseC(2583L),
      GeneratedCommandList(65528L),
      AcceptedCommandList(65529L),
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      GetProfileInfoCommand(0L),
      GetMeasurementProfileCommand(1L),
      ;
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
    }

    @Override
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
      Unsupported(255L),
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
      GeneratedCommandList(65528L),
      AcceptedCommandList(65529L),
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      ;
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
    }

    @Override
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
      EventList(65530L),
      AttributeList(65531L),
      FeatureMap(65532L),
      ClusterRevision(65533L),
      ;
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
      ;
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
      FailRandomlyAtFault(1L),
      ;
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
    }

    @Override
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
}
