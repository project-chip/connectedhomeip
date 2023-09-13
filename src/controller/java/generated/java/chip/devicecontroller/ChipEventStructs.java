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

import javax.annotation.Nullable;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Optional;

public class ChipEventStructs {
public static class AccessControlClusterAccessControlEntryChangedEvent implements Cloneable {
  public @Nullable Long adminNodeID;
  public @Nullable Integer adminPasscodeID;
  public Integer changeType;
  public @Nullable ChipStructs.AccessControlClusterAccessControlEntryStruct latestValue;
  public Integer fabricIndex;
  public AccessControlClusterAccessControlEntryChangedEvent(
    @Nullable Long adminNodeID,
    @Nullable Integer adminPasscodeID,
    Integer changeType,
    @Nullable ChipStructs.AccessControlClusterAccessControlEntryStruct latestValue,
    Integer fabricIndex
  ) {
    this.adminNodeID = adminNodeID;
    this.adminPasscodeID = adminPasscodeID;
    this.changeType = changeType;
    this.latestValue = latestValue;
    this.fabricIndex = fabricIndex;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("AccessControlClusterAccessControlEntryChangedEvent {\n");
    output.append("\tadminNodeID: ");
    output.append(adminNodeID);
    output.append("\n");
    output.append("\tadminPasscodeID: ");
    output.append(adminPasscodeID);
    output.append("\n");
    output.append("\tchangeType: ");
    output.append(changeType);
    output.append("\n");
    output.append("\tlatestValue: ");
    output.append(latestValue);
    output.append("\n");
    output.append("\tfabricIndex: ");
    output.append(fabricIndex);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public AccessControlClusterAccessControlEntryChangedEvent clone() {
    AccessControlClusterAccessControlEntryChangedEvent ret;
    try {
      ret = (AccessControlClusterAccessControlEntryChangedEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.adminNodeID = (adminNodeID != null ? adminNodeID : null);
    ret.adminPasscodeID = (adminPasscodeID != null ? adminPasscodeID : null);
    ret.changeType = changeType;
    ret.latestValue = (latestValue != null ? latestValue.clone() : null);
    ret.fabricIndex = fabricIndex;
    return ret;
  }
}
public static class AccessControlClusterAccessControlExtensionChangedEvent implements Cloneable {
  public @Nullable Long adminNodeID;
  public @Nullable Integer adminPasscodeID;
  public Integer changeType;
  public @Nullable ChipStructs.AccessControlClusterAccessControlExtensionStruct latestValue;
  public Integer fabricIndex;
  public AccessControlClusterAccessControlExtensionChangedEvent(
    @Nullable Long adminNodeID,
    @Nullable Integer adminPasscodeID,
    Integer changeType,
    @Nullable ChipStructs.AccessControlClusterAccessControlExtensionStruct latestValue,
    Integer fabricIndex
  ) {
    this.adminNodeID = adminNodeID;
    this.adminPasscodeID = adminPasscodeID;
    this.changeType = changeType;
    this.latestValue = latestValue;
    this.fabricIndex = fabricIndex;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("AccessControlClusterAccessControlExtensionChangedEvent {\n");
    output.append("\tadminNodeID: ");
    output.append(adminNodeID);
    output.append("\n");
    output.append("\tadminPasscodeID: ");
    output.append(adminPasscodeID);
    output.append("\n");
    output.append("\tchangeType: ");
    output.append(changeType);
    output.append("\n");
    output.append("\tlatestValue: ");
    output.append(latestValue);
    output.append("\n");
    output.append("\tfabricIndex: ");
    output.append(fabricIndex);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public AccessControlClusterAccessControlExtensionChangedEvent clone() {
    AccessControlClusterAccessControlExtensionChangedEvent ret;
    try {
      ret = (AccessControlClusterAccessControlExtensionChangedEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.adminNodeID = (adminNodeID != null ? adminNodeID : null);
    ret.adminPasscodeID = (adminPasscodeID != null ? adminPasscodeID : null);
    ret.changeType = changeType;
    ret.latestValue = (latestValue != null ? latestValue.clone() : null);
    ret.fabricIndex = fabricIndex;
    return ret;
  }
}
public static class ActionsClusterStateChangedEvent implements Cloneable {
  public Integer actionID;
  public Long invokeID;
  public Integer newState;
  public ActionsClusterStateChangedEvent(
    Integer actionID,
    Long invokeID,
    Integer newState
  ) {
    this.actionID = actionID;
    this.invokeID = invokeID;
    this.newState = newState;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ActionsClusterStateChangedEvent {\n");
    output.append("\tactionID: ");
    output.append(actionID);
    output.append("\n");
    output.append("\tinvokeID: ");
    output.append(invokeID);
    output.append("\n");
    output.append("\tnewState: ");
    output.append(newState);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public ActionsClusterStateChangedEvent clone() {
    ActionsClusterStateChangedEvent ret;
    try {
      ret = (ActionsClusterStateChangedEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.actionID = actionID;
    ret.invokeID = invokeID;
    ret.newState = newState;
    return ret;
  }
}
public static class ActionsClusterActionFailedEvent implements Cloneable {
  public Integer actionID;
  public Long invokeID;
  public Integer newState;
  public Integer error;
  public ActionsClusterActionFailedEvent(
    Integer actionID,
    Long invokeID,
    Integer newState,
    Integer error
  ) {
    this.actionID = actionID;
    this.invokeID = invokeID;
    this.newState = newState;
    this.error = error;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ActionsClusterActionFailedEvent {\n");
    output.append("\tactionID: ");
    output.append(actionID);
    output.append("\n");
    output.append("\tinvokeID: ");
    output.append(invokeID);
    output.append("\n");
    output.append("\tnewState: ");
    output.append(newState);
    output.append("\n");
    output.append("\terror: ");
    output.append(error);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public ActionsClusterActionFailedEvent clone() {
    ActionsClusterActionFailedEvent ret;
    try {
      ret = (ActionsClusterActionFailedEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.actionID = actionID;
    ret.invokeID = invokeID;
    ret.newState = newState;
    ret.error = error;
    return ret;
  }
}
public static class BasicInformationClusterStartUpEvent implements Cloneable {
  public Long softwareVersion;
  public BasicInformationClusterStartUpEvent(
    Long softwareVersion
  ) {
    this.softwareVersion = softwareVersion;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("BasicInformationClusterStartUpEvent {\n");
    output.append("\tsoftwareVersion: ");
    output.append(softwareVersion);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public BasicInformationClusterStartUpEvent clone() {
    BasicInformationClusterStartUpEvent ret;
    try {
      ret = (BasicInformationClusterStartUpEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.softwareVersion = softwareVersion;
    return ret;
  }
}
public static class BasicInformationClusterLeaveEvent implements Cloneable {
  public Integer fabricIndex;
  public BasicInformationClusterLeaveEvent(
    Integer fabricIndex
  ) {
    this.fabricIndex = fabricIndex;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("BasicInformationClusterLeaveEvent {\n");
    output.append("\tfabricIndex: ");
    output.append(fabricIndex);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public BasicInformationClusterLeaveEvent clone() {
    BasicInformationClusterLeaveEvent ret;
    try {
      ret = (BasicInformationClusterLeaveEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.fabricIndex = fabricIndex;
    return ret;
  }
}
public static class BasicInformationClusterReachableChangedEvent implements Cloneable {
  public Boolean reachableNewValue;
  public BasicInformationClusterReachableChangedEvent(
    Boolean reachableNewValue
  ) {
    this.reachableNewValue = reachableNewValue;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("BasicInformationClusterReachableChangedEvent {\n");
    output.append("\treachableNewValue: ");
    output.append(reachableNewValue);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public BasicInformationClusterReachableChangedEvent clone() {
    BasicInformationClusterReachableChangedEvent ret;
    try {
      ret = (BasicInformationClusterReachableChangedEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.reachableNewValue = reachableNewValue;
    return ret;
  }
}
public static class OtaSoftwareUpdateRequestorClusterStateTransitionEvent implements Cloneable {
  public Integer previousState;
  public Integer newState;
  public Integer reason;
  public @Nullable Long targetSoftwareVersion;
  public OtaSoftwareUpdateRequestorClusterStateTransitionEvent(
    Integer previousState,
    Integer newState,
    Integer reason,
    @Nullable Long targetSoftwareVersion
  ) {
    this.previousState = previousState;
    this.newState = newState;
    this.reason = reason;
    this.targetSoftwareVersion = targetSoftwareVersion;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("OtaSoftwareUpdateRequestorClusterStateTransitionEvent {\n");
    output.append("\tpreviousState: ");
    output.append(previousState);
    output.append("\n");
    output.append("\tnewState: ");
    output.append(newState);
    output.append("\n");
    output.append("\treason: ");
    output.append(reason);
    output.append("\n");
    output.append("\ttargetSoftwareVersion: ");
    output.append(targetSoftwareVersion);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public OtaSoftwareUpdateRequestorClusterStateTransitionEvent clone() {
    OtaSoftwareUpdateRequestorClusterStateTransitionEvent ret;
    try {
      ret = (OtaSoftwareUpdateRequestorClusterStateTransitionEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.previousState = previousState;
    ret.newState = newState;
    ret.reason = reason;
    ret.targetSoftwareVersion = (targetSoftwareVersion != null ? targetSoftwareVersion : null);
    return ret;
  }
}
public static class OtaSoftwareUpdateRequestorClusterVersionAppliedEvent implements Cloneable {
  public Long softwareVersion;
  public Integer productID;
  public OtaSoftwareUpdateRequestorClusterVersionAppliedEvent(
    Long softwareVersion,
    Integer productID
  ) {
    this.softwareVersion = softwareVersion;
    this.productID = productID;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("OtaSoftwareUpdateRequestorClusterVersionAppliedEvent {\n");
    output.append("\tsoftwareVersion: ");
    output.append(softwareVersion);
    output.append("\n");
    output.append("\tproductID: ");
    output.append(productID);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public OtaSoftwareUpdateRequestorClusterVersionAppliedEvent clone() {
    OtaSoftwareUpdateRequestorClusterVersionAppliedEvent ret;
    try {
      ret = (OtaSoftwareUpdateRequestorClusterVersionAppliedEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.softwareVersion = softwareVersion;
    ret.productID = productID;
    return ret;
  }
}
public static class OtaSoftwareUpdateRequestorClusterDownloadErrorEvent implements Cloneable {
  public Long softwareVersion;
  public Long bytesDownloaded;
  public @Nullable Integer progressPercent;
  public @Nullable Long platformCode;
  public OtaSoftwareUpdateRequestorClusterDownloadErrorEvent(
    Long softwareVersion,
    Long bytesDownloaded,
    @Nullable Integer progressPercent,
    @Nullable Long platformCode
  ) {
    this.softwareVersion = softwareVersion;
    this.bytesDownloaded = bytesDownloaded;
    this.progressPercent = progressPercent;
    this.platformCode = platformCode;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("OtaSoftwareUpdateRequestorClusterDownloadErrorEvent {\n");
    output.append("\tsoftwareVersion: ");
    output.append(softwareVersion);
    output.append("\n");
    output.append("\tbytesDownloaded: ");
    output.append(bytesDownloaded);
    output.append("\n");
    output.append("\tprogressPercent: ");
    output.append(progressPercent);
    output.append("\n");
    output.append("\tplatformCode: ");
    output.append(platformCode);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public OtaSoftwareUpdateRequestorClusterDownloadErrorEvent clone() {
    OtaSoftwareUpdateRequestorClusterDownloadErrorEvent ret;
    try {
      ret = (OtaSoftwareUpdateRequestorClusterDownloadErrorEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.softwareVersion = softwareVersion;
    ret.bytesDownloaded = bytesDownloaded;
    ret.progressPercent = (progressPercent != null ? progressPercent : null);
    ret.platformCode = (platformCode != null ? platformCode : null);
    return ret;
  }
}
public static class PowerSourceClusterWiredFaultChangeEvent implements Cloneable {
  public ArrayList<Integer> current;
  public ArrayList<Integer> previous;
  public PowerSourceClusterWiredFaultChangeEvent(
    ArrayList<Integer> current,
    ArrayList<Integer> previous
  ) {
    this.current = current;
    this.previous = previous;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("PowerSourceClusterWiredFaultChangeEvent {\n");
    output.append("\tcurrent: ");
    output.append(current);
    output.append("\n");
    output.append("\tprevious: ");
    output.append(previous);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public PowerSourceClusterWiredFaultChangeEvent clone() {
    PowerSourceClusterWiredFaultChangeEvent ret;
    try {
      ret = (PowerSourceClusterWiredFaultChangeEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.current = arrayClone(current, current_it -> current_it);
    ret.previous = arrayClone(previous, previous_it -> previous_it);
    return ret;
  }
}
public static class PowerSourceClusterBatFaultChangeEvent implements Cloneable {
  public ArrayList<Integer> current;
  public ArrayList<Integer> previous;
  public PowerSourceClusterBatFaultChangeEvent(
    ArrayList<Integer> current,
    ArrayList<Integer> previous
  ) {
    this.current = current;
    this.previous = previous;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("PowerSourceClusterBatFaultChangeEvent {\n");
    output.append("\tcurrent: ");
    output.append(current);
    output.append("\n");
    output.append("\tprevious: ");
    output.append(previous);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public PowerSourceClusterBatFaultChangeEvent clone() {
    PowerSourceClusterBatFaultChangeEvent ret;
    try {
      ret = (PowerSourceClusterBatFaultChangeEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.current = arrayClone(current, current_it -> current_it);
    ret.previous = arrayClone(previous, previous_it -> previous_it);
    return ret;
  }
}
public static class PowerSourceClusterBatChargeFaultChangeEvent implements Cloneable {
  public ArrayList<Integer> current;
  public ArrayList<Integer> previous;
  public PowerSourceClusterBatChargeFaultChangeEvent(
    ArrayList<Integer> current,
    ArrayList<Integer> previous
  ) {
    this.current = current;
    this.previous = previous;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("PowerSourceClusterBatChargeFaultChangeEvent {\n");
    output.append("\tcurrent: ");
    output.append(current);
    output.append("\n");
    output.append("\tprevious: ");
    output.append(previous);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public PowerSourceClusterBatChargeFaultChangeEvent clone() {
    PowerSourceClusterBatChargeFaultChangeEvent ret;
    try {
      ret = (PowerSourceClusterBatChargeFaultChangeEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.current = arrayClone(current, current_it -> current_it);
    ret.previous = arrayClone(previous, previous_it -> previous_it);
    return ret;
  }
}
public static class GeneralDiagnosticsClusterHardwareFaultChangeEvent implements Cloneable {
  public ArrayList<Integer> current;
  public ArrayList<Integer> previous;
  public GeneralDiagnosticsClusterHardwareFaultChangeEvent(
    ArrayList<Integer> current,
    ArrayList<Integer> previous
  ) {
    this.current = current;
    this.previous = previous;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("GeneralDiagnosticsClusterHardwareFaultChangeEvent {\n");
    output.append("\tcurrent: ");
    output.append(current);
    output.append("\n");
    output.append("\tprevious: ");
    output.append(previous);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public GeneralDiagnosticsClusterHardwareFaultChangeEvent clone() {
    GeneralDiagnosticsClusterHardwareFaultChangeEvent ret;
    try {
      ret = (GeneralDiagnosticsClusterHardwareFaultChangeEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.current = arrayClone(current, current_it -> current_it);
    ret.previous = arrayClone(previous, previous_it -> previous_it);
    return ret;
  }
}
public static class GeneralDiagnosticsClusterRadioFaultChangeEvent implements Cloneable {
  public ArrayList<Integer> current;
  public ArrayList<Integer> previous;
  public GeneralDiagnosticsClusterRadioFaultChangeEvent(
    ArrayList<Integer> current,
    ArrayList<Integer> previous
  ) {
    this.current = current;
    this.previous = previous;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("GeneralDiagnosticsClusterRadioFaultChangeEvent {\n");
    output.append("\tcurrent: ");
    output.append(current);
    output.append("\n");
    output.append("\tprevious: ");
    output.append(previous);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public GeneralDiagnosticsClusterRadioFaultChangeEvent clone() {
    GeneralDiagnosticsClusterRadioFaultChangeEvent ret;
    try {
      ret = (GeneralDiagnosticsClusterRadioFaultChangeEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.current = arrayClone(current, current_it -> current_it);
    ret.previous = arrayClone(previous, previous_it -> previous_it);
    return ret;
  }
}
public static class GeneralDiagnosticsClusterNetworkFaultChangeEvent implements Cloneable {
  public ArrayList<Integer> current;
  public ArrayList<Integer> previous;
  public GeneralDiagnosticsClusterNetworkFaultChangeEvent(
    ArrayList<Integer> current,
    ArrayList<Integer> previous
  ) {
    this.current = current;
    this.previous = previous;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("GeneralDiagnosticsClusterNetworkFaultChangeEvent {\n");
    output.append("\tcurrent: ");
    output.append(current);
    output.append("\n");
    output.append("\tprevious: ");
    output.append(previous);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public GeneralDiagnosticsClusterNetworkFaultChangeEvent clone() {
    GeneralDiagnosticsClusterNetworkFaultChangeEvent ret;
    try {
      ret = (GeneralDiagnosticsClusterNetworkFaultChangeEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.current = arrayClone(current, current_it -> current_it);
    ret.previous = arrayClone(previous, previous_it -> previous_it);
    return ret;
  }
}
public static class GeneralDiagnosticsClusterBootReasonEvent implements Cloneable {
  public Integer bootReason;
  public GeneralDiagnosticsClusterBootReasonEvent(
    Integer bootReason
  ) {
    this.bootReason = bootReason;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("GeneralDiagnosticsClusterBootReasonEvent {\n");
    output.append("\tbootReason: ");
    output.append(bootReason);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public GeneralDiagnosticsClusterBootReasonEvent clone() {
    GeneralDiagnosticsClusterBootReasonEvent ret;
    try {
      ret = (GeneralDiagnosticsClusterBootReasonEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.bootReason = bootReason;
    return ret;
  }
}
public static class SoftwareDiagnosticsClusterSoftwareFaultEvent implements Cloneable {
  public Long id;
  public Optional<String> name;
  public Optional<byte[]> faultRecording;
  public SoftwareDiagnosticsClusterSoftwareFaultEvent(
    Long id,
    Optional<String> name,
    Optional<byte[]> faultRecording
  ) {
    this.id = id;
    this.name = name;
    this.faultRecording = faultRecording;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("SoftwareDiagnosticsClusterSoftwareFaultEvent {\n");
    output.append("\tid: ");
    output.append(id);
    output.append("\n");
    output.append("\tname: ");
    output.append(name);
    output.append("\n");
    output.append("\tfaultRecording: ");
    output.append(faultRecording.isPresent() ? Arrays.toString(faultRecording.get()) : "");
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public SoftwareDiagnosticsClusterSoftwareFaultEvent clone() {
    SoftwareDiagnosticsClusterSoftwareFaultEvent ret;
    try {
      ret = (SoftwareDiagnosticsClusterSoftwareFaultEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.id = id;
    ret.name = name.map(name_map -> name_map);
    ret.faultRecording = faultRecording.map(faultRecording_map -> faultRecording_map.clone());
    return ret;
  }
}
public static class ThreadNetworkDiagnosticsClusterConnectionStatusEvent implements Cloneable {
  public Integer connectionStatus;
  public ThreadNetworkDiagnosticsClusterConnectionStatusEvent(
    Integer connectionStatus
  ) {
    this.connectionStatus = connectionStatus;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ThreadNetworkDiagnosticsClusterConnectionStatusEvent {\n");
    output.append("\tconnectionStatus: ");
    output.append(connectionStatus);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public ThreadNetworkDiagnosticsClusterConnectionStatusEvent clone() {
    ThreadNetworkDiagnosticsClusterConnectionStatusEvent ret;
    try {
      ret = (ThreadNetworkDiagnosticsClusterConnectionStatusEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.connectionStatus = connectionStatus;
    return ret;
  }
}
public static class ThreadNetworkDiagnosticsClusterNetworkFaultChangeEvent implements Cloneable {
  public ArrayList<Integer> current;
  public ArrayList<Integer> previous;
  public ThreadNetworkDiagnosticsClusterNetworkFaultChangeEvent(
    ArrayList<Integer> current,
    ArrayList<Integer> previous
  ) {
    this.current = current;
    this.previous = previous;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ThreadNetworkDiagnosticsClusterNetworkFaultChangeEvent {\n");
    output.append("\tcurrent: ");
    output.append(current);
    output.append("\n");
    output.append("\tprevious: ");
    output.append(previous);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public ThreadNetworkDiagnosticsClusterNetworkFaultChangeEvent clone() {
    ThreadNetworkDiagnosticsClusterNetworkFaultChangeEvent ret;
    try {
      ret = (ThreadNetworkDiagnosticsClusterNetworkFaultChangeEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.current = arrayClone(current, current_it -> current_it);
    ret.previous = arrayClone(previous, previous_it -> previous_it);
    return ret;
  }
}
public static class WiFiNetworkDiagnosticsClusterDisconnectionEvent implements Cloneable {
  public Integer reasonCode;
  public WiFiNetworkDiagnosticsClusterDisconnectionEvent(
    Integer reasonCode
  ) {
    this.reasonCode = reasonCode;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("WiFiNetworkDiagnosticsClusterDisconnectionEvent {\n");
    output.append("\treasonCode: ");
    output.append(reasonCode);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public WiFiNetworkDiagnosticsClusterDisconnectionEvent clone() {
    WiFiNetworkDiagnosticsClusterDisconnectionEvent ret;
    try {
      ret = (WiFiNetworkDiagnosticsClusterDisconnectionEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.reasonCode = reasonCode;
    return ret;
  }
}
public static class WiFiNetworkDiagnosticsClusterAssociationFailureEvent implements Cloneable {
  public Integer associationFailure;
  public Integer status;
  public WiFiNetworkDiagnosticsClusterAssociationFailureEvent(
    Integer associationFailure,
    Integer status
  ) {
    this.associationFailure = associationFailure;
    this.status = status;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("WiFiNetworkDiagnosticsClusterAssociationFailureEvent {\n");
    output.append("\tassociationFailure: ");
    output.append(associationFailure);
    output.append("\n");
    output.append("\tstatus: ");
    output.append(status);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public WiFiNetworkDiagnosticsClusterAssociationFailureEvent clone() {
    WiFiNetworkDiagnosticsClusterAssociationFailureEvent ret;
    try {
      ret = (WiFiNetworkDiagnosticsClusterAssociationFailureEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.associationFailure = associationFailure;
    ret.status = status;
    return ret;
  }
}
public static class WiFiNetworkDiagnosticsClusterConnectionStatusEvent implements Cloneable {
  public Integer connectionStatus;
  public WiFiNetworkDiagnosticsClusterConnectionStatusEvent(
    Integer connectionStatus
  ) {
    this.connectionStatus = connectionStatus;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("WiFiNetworkDiagnosticsClusterConnectionStatusEvent {\n");
    output.append("\tconnectionStatus: ");
    output.append(connectionStatus);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public WiFiNetworkDiagnosticsClusterConnectionStatusEvent clone() {
    WiFiNetworkDiagnosticsClusterConnectionStatusEvent ret;
    try {
      ret = (WiFiNetworkDiagnosticsClusterConnectionStatusEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.connectionStatus = connectionStatus;
    return ret;
  }
}
public static class TimeSynchronizationClusterDSTStatusEvent implements Cloneable {
  public Boolean DSTOffsetActive;
  public TimeSynchronizationClusterDSTStatusEvent(
    Boolean DSTOffsetActive
  ) {
    this.DSTOffsetActive = DSTOffsetActive;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("TimeSynchronizationClusterDSTStatusEvent {\n");
    output.append("\tDSTOffsetActive: ");
    output.append(DSTOffsetActive);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public TimeSynchronizationClusterDSTStatusEvent clone() {
    TimeSynchronizationClusterDSTStatusEvent ret;
    try {
      ret = (TimeSynchronizationClusterDSTStatusEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.DSTOffsetActive = DSTOffsetActive;
    return ret;
  }
}
public static class TimeSynchronizationClusterTimeZoneStatusEvent implements Cloneable {
  public Long offset;
  public Optional<String> name;
  public TimeSynchronizationClusterTimeZoneStatusEvent(
    Long offset,
    Optional<String> name
  ) {
    this.offset = offset;
    this.name = name;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("TimeSynchronizationClusterTimeZoneStatusEvent {\n");
    output.append("\toffset: ");
    output.append(offset);
    output.append("\n");
    output.append("\tname: ");
    output.append(name);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public TimeSynchronizationClusterTimeZoneStatusEvent clone() {
    TimeSynchronizationClusterTimeZoneStatusEvent ret;
    try {
      ret = (TimeSynchronizationClusterTimeZoneStatusEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.offset = offset;
    ret.name = name.map(name_map -> name_map);
    return ret;
  }
}
public static class BridgedDeviceBasicInformationClusterStartUpEvent implements Cloneable {
  public Long softwareVersion;
  public BridgedDeviceBasicInformationClusterStartUpEvent(
    Long softwareVersion
  ) {
    this.softwareVersion = softwareVersion;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("BridgedDeviceBasicInformationClusterStartUpEvent {\n");
    output.append("\tsoftwareVersion: ");
    output.append(softwareVersion);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public BridgedDeviceBasicInformationClusterStartUpEvent clone() {
    BridgedDeviceBasicInformationClusterStartUpEvent ret;
    try {
      ret = (BridgedDeviceBasicInformationClusterStartUpEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.softwareVersion = softwareVersion;
    return ret;
  }
}
public static class BridgedDeviceBasicInformationClusterReachableChangedEvent implements Cloneable {
  public Boolean reachableNewValue;
  public BridgedDeviceBasicInformationClusterReachableChangedEvent(
    Boolean reachableNewValue
  ) {
    this.reachableNewValue = reachableNewValue;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("BridgedDeviceBasicInformationClusterReachableChangedEvent {\n");
    output.append("\treachableNewValue: ");
    output.append(reachableNewValue);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public BridgedDeviceBasicInformationClusterReachableChangedEvent clone() {
    BridgedDeviceBasicInformationClusterReachableChangedEvent ret;
    try {
      ret = (BridgedDeviceBasicInformationClusterReachableChangedEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.reachableNewValue = reachableNewValue;
    return ret;
  }
}
public static class SwitchClusterSwitchLatchedEvent implements Cloneable {
  public Integer newPosition;
  public SwitchClusterSwitchLatchedEvent(
    Integer newPosition
  ) {
    this.newPosition = newPosition;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("SwitchClusterSwitchLatchedEvent {\n");
    output.append("\tnewPosition: ");
    output.append(newPosition);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public SwitchClusterSwitchLatchedEvent clone() {
    SwitchClusterSwitchLatchedEvent ret;
    try {
      ret = (SwitchClusterSwitchLatchedEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.newPosition = newPosition;
    return ret;
  }
}
public static class SwitchClusterInitialPressEvent implements Cloneable {
  public Integer newPosition;
  public SwitchClusterInitialPressEvent(
    Integer newPosition
  ) {
    this.newPosition = newPosition;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("SwitchClusterInitialPressEvent {\n");
    output.append("\tnewPosition: ");
    output.append(newPosition);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public SwitchClusterInitialPressEvent clone() {
    SwitchClusterInitialPressEvent ret;
    try {
      ret = (SwitchClusterInitialPressEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.newPosition = newPosition;
    return ret;
  }
}
public static class SwitchClusterLongPressEvent implements Cloneable {
  public Integer newPosition;
  public SwitchClusterLongPressEvent(
    Integer newPosition
  ) {
    this.newPosition = newPosition;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("SwitchClusterLongPressEvent {\n");
    output.append("\tnewPosition: ");
    output.append(newPosition);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public SwitchClusterLongPressEvent clone() {
    SwitchClusterLongPressEvent ret;
    try {
      ret = (SwitchClusterLongPressEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.newPosition = newPosition;
    return ret;
  }
}
public static class SwitchClusterShortReleaseEvent implements Cloneable {
  public Integer previousPosition;
  public SwitchClusterShortReleaseEvent(
    Integer previousPosition
  ) {
    this.previousPosition = previousPosition;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("SwitchClusterShortReleaseEvent {\n");
    output.append("\tpreviousPosition: ");
    output.append(previousPosition);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public SwitchClusterShortReleaseEvent clone() {
    SwitchClusterShortReleaseEvent ret;
    try {
      ret = (SwitchClusterShortReleaseEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.previousPosition = previousPosition;
    return ret;
  }
}
public static class SwitchClusterLongReleaseEvent implements Cloneable {
  public Integer previousPosition;
  public SwitchClusterLongReleaseEvent(
    Integer previousPosition
  ) {
    this.previousPosition = previousPosition;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("SwitchClusterLongReleaseEvent {\n");
    output.append("\tpreviousPosition: ");
    output.append(previousPosition);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public SwitchClusterLongReleaseEvent clone() {
    SwitchClusterLongReleaseEvent ret;
    try {
      ret = (SwitchClusterLongReleaseEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.previousPosition = previousPosition;
    return ret;
  }
}
public static class SwitchClusterMultiPressOngoingEvent implements Cloneable {
  public Integer newPosition;
  public Integer currentNumberOfPressesCounted;
  public SwitchClusterMultiPressOngoingEvent(
    Integer newPosition,
    Integer currentNumberOfPressesCounted
  ) {
    this.newPosition = newPosition;
    this.currentNumberOfPressesCounted = currentNumberOfPressesCounted;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("SwitchClusterMultiPressOngoingEvent {\n");
    output.append("\tnewPosition: ");
    output.append(newPosition);
    output.append("\n");
    output.append("\tcurrentNumberOfPressesCounted: ");
    output.append(currentNumberOfPressesCounted);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public SwitchClusterMultiPressOngoingEvent clone() {
    SwitchClusterMultiPressOngoingEvent ret;
    try {
      ret = (SwitchClusterMultiPressOngoingEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.newPosition = newPosition;
    ret.currentNumberOfPressesCounted = currentNumberOfPressesCounted;
    return ret;
  }
}
public static class SwitchClusterMultiPressCompleteEvent implements Cloneable {
  public Integer previousPosition;
  public Integer totalNumberOfPressesCounted;
  public SwitchClusterMultiPressCompleteEvent(
    Integer previousPosition,
    Integer totalNumberOfPressesCounted
  ) {
    this.previousPosition = previousPosition;
    this.totalNumberOfPressesCounted = totalNumberOfPressesCounted;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("SwitchClusterMultiPressCompleteEvent {\n");
    output.append("\tpreviousPosition: ");
    output.append(previousPosition);
    output.append("\n");
    output.append("\ttotalNumberOfPressesCounted: ");
    output.append(totalNumberOfPressesCounted);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public SwitchClusterMultiPressCompleteEvent clone() {
    SwitchClusterMultiPressCompleteEvent ret;
    try {
      ret = (SwitchClusterMultiPressCompleteEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.previousPosition = previousPosition;
    ret.totalNumberOfPressesCounted = totalNumberOfPressesCounted;
    return ret;
  }
}
public static class BooleanStateClusterStateChangeEvent implements Cloneable {
  public Boolean stateValue;
  public BooleanStateClusterStateChangeEvent(
    Boolean stateValue
  ) {
    this.stateValue = stateValue;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("BooleanStateClusterStateChangeEvent {\n");
    output.append("\tstateValue: ");
    output.append(stateValue);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public BooleanStateClusterStateChangeEvent clone() {
    BooleanStateClusterStateChangeEvent ret;
    try {
      ret = (BooleanStateClusterStateChangeEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.stateValue = stateValue;
    return ret;
  }
}
public static class RefrigeratorAlarmClusterNotifyEvent implements Cloneable {
  public Long active;
  public Long inactive;
  public Long state;
  public Long mask;
  public RefrigeratorAlarmClusterNotifyEvent(
    Long active,
    Long inactive,
    Long state,
    Long mask
  ) {
    this.active = active;
    this.inactive = inactive;
    this.state = state;
    this.mask = mask;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("RefrigeratorAlarmClusterNotifyEvent {\n");
    output.append("\tactive: ");
    output.append(active);
    output.append("\n");
    output.append("\tinactive: ");
    output.append(inactive);
    output.append("\n");
    output.append("\tstate: ");
    output.append(state);
    output.append("\n");
    output.append("\tmask: ");
    output.append(mask);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public RefrigeratorAlarmClusterNotifyEvent clone() {
    RefrigeratorAlarmClusterNotifyEvent ret;
    try {
      ret = (RefrigeratorAlarmClusterNotifyEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.active = active;
    ret.inactive = inactive;
    ret.state = state;
    ret.mask = mask;
    return ret;
  }
}
public static class SmokeCoAlarmClusterSmokeAlarmEvent implements Cloneable {
  public Integer alarmSeverityLevel;
  public SmokeCoAlarmClusterSmokeAlarmEvent(
    Integer alarmSeverityLevel
  ) {
    this.alarmSeverityLevel = alarmSeverityLevel;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("SmokeCoAlarmClusterSmokeAlarmEvent {\n");
    output.append("\talarmSeverityLevel: ");
    output.append(alarmSeverityLevel);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public SmokeCoAlarmClusterSmokeAlarmEvent clone() {
    SmokeCoAlarmClusterSmokeAlarmEvent ret;
    try {
      ret = (SmokeCoAlarmClusterSmokeAlarmEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.alarmSeverityLevel = alarmSeverityLevel;
    return ret;
  }
}
public static class SmokeCoAlarmClusterCOAlarmEvent implements Cloneable {
  public Integer alarmSeverityLevel;
  public SmokeCoAlarmClusterCOAlarmEvent(
    Integer alarmSeverityLevel
  ) {
    this.alarmSeverityLevel = alarmSeverityLevel;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("SmokeCoAlarmClusterCOAlarmEvent {\n");
    output.append("\talarmSeverityLevel: ");
    output.append(alarmSeverityLevel);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public SmokeCoAlarmClusterCOAlarmEvent clone() {
    SmokeCoAlarmClusterCOAlarmEvent ret;
    try {
      ret = (SmokeCoAlarmClusterCOAlarmEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.alarmSeverityLevel = alarmSeverityLevel;
    return ret;
  }
}
public static class SmokeCoAlarmClusterLowBatteryEvent implements Cloneable {
  public Integer alarmSeverityLevel;
  public SmokeCoAlarmClusterLowBatteryEvent(
    Integer alarmSeverityLevel
  ) {
    this.alarmSeverityLevel = alarmSeverityLevel;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("SmokeCoAlarmClusterLowBatteryEvent {\n");
    output.append("\talarmSeverityLevel: ");
    output.append(alarmSeverityLevel);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public SmokeCoAlarmClusterLowBatteryEvent clone() {
    SmokeCoAlarmClusterLowBatteryEvent ret;
    try {
      ret = (SmokeCoAlarmClusterLowBatteryEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.alarmSeverityLevel = alarmSeverityLevel;
    return ret;
  }
}
public static class SmokeCoAlarmClusterInterconnectSmokeAlarmEvent implements Cloneable {
  public Integer alarmSeverityLevel;
  public SmokeCoAlarmClusterInterconnectSmokeAlarmEvent(
    Integer alarmSeverityLevel
  ) {
    this.alarmSeverityLevel = alarmSeverityLevel;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("SmokeCoAlarmClusterInterconnectSmokeAlarmEvent {\n");
    output.append("\talarmSeverityLevel: ");
    output.append(alarmSeverityLevel);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public SmokeCoAlarmClusterInterconnectSmokeAlarmEvent clone() {
    SmokeCoAlarmClusterInterconnectSmokeAlarmEvent ret;
    try {
      ret = (SmokeCoAlarmClusterInterconnectSmokeAlarmEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.alarmSeverityLevel = alarmSeverityLevel;
    return ret;
  }
}
public static class SmokeCoAlarmClusterInterconnectCOAlarmEvent implements Cloneable {
  public Integer alarmSeverityLevel;
  public SmokeCoAlarmClusterInterconnectCOAlarmEvent(
    Integer alarmSeverityLevel
  ) {
    this.alarmSeverityLevel = alarmSeverityLevel;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("SmokeCoAlarmClusterInterconnectCOAlarmEvent {\n");
    output.append("\talarmSeverityLevel: ");
    output.append(alarmSeverityLevel);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public SmokeCoAlarmClusterInterconnectCOAlarmEvent clone() {
    SmokeCoAlarmClusterInterconnectCOAlarmEvent ret;
    try {
      ret = (SmokeCoAlarmClusterInterconnectCOAlarmEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.alarmSeverityLevel = alarmSeverityLevel;
    return ret;
  }
}
public static class DishwasherAlarmClusterNotifyEvent implements Cloneable {
  public Long active;
  public Long inactive;
  public Long state;
  public Long mask;
  public DishwasherAlarmClusterNotifyEvent(
    Long active,
    Long inactive,
    Long state,
    Long mask
  ) {
    this.active = active;
    this.inactive = inactive;
    this.state = state;
    this.mask = mask;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("DishwasherAlarmClusterNotifyEvent {\n");
    output.append("\tactive: ");
    output.append(active);
    output.append("\n");
    output.append("\tinactive: ");
    output.append(inactive);
    output.append("\n");
    output.append("\tstate: ");
    output.append(state);
    output.append("\n");
    output.append("\tmask: ");
    output.append(mask);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public DishwasherAlarmClusterNotifyEvent clone() {
    DishwasherAlarmClusterNotifyEvent ret;
    try {
      ret = (DishwasherAlarmClusterNotifyEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.active = active;
    ret.inactive = inactive;
    ret.state = state;
    ret.mask = mask;
    return ret;
  }
}
public static class OperationalStateClusterOperationalErrorEvent implements Cloneable {
  public ChipStructs.OperationalStateClusterErrorStateStruct errorState;
  public OperationalStateClusterOperationalErrorEvent(
    ChipStructs.OperationalStateClusterErrorStateStruct errorState
  ) {
    this.errorState = errorState;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("OperationalStateClusterOperationalErrorEvent {\n");
    output.append("\terrorState: ");
    output.append(errorState);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public OperationalStateClusterOperationalErrorEvent clone() {
    OperationalStateClusterOperationalErrorEvent ret;
    try {
      ret = (OperationalStateClusterOperationalErrorEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.errorState = errorState.clone();
    return ret;
  }
}
public static class OperationalStateClusterOperationCompletionEvent implements Cloneable {
  public Integer completionErrorCode;
  public @Nullable Optional<Long> totalOperationalTime;
  public @Nullable Optional<Long> pausedTime;
  public OperationalStateClusterOperationCompletionEvent(
    Integer completionErrorCode,
    @Nullable Optional<Long> totalOperationalTime,
    @Nullable Optional<Long> pausedTime
  ) {
    this.completionErrorCode = completionErrorCode;
    this.totalOperationalTime = totalOperationalTime;
    this.pausedTime = pausedTime;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("OperationalStateClusterOperationCompletionEvent {\n");
    output.append("\tcompletionErrorCode: ");
    output.append(completionErrorCode);
    output.append("\n");
    output.append("\ttotalOperationalTime: ");
    output.append(totalOperationalTime);
    output.append("\n");
    output.append("\tpausedTime: ");
    output.append(pausedTime);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public OperationalStateClusterOperationCompletionEvent clone() {
    OperationalStateClusterOperationCompletionEvent ret;
    try {
      ret = (OperationalStateClusterOperationCompletionEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.completionErrorCode = completionErrorCode;
    ret.totalOperationalTime = (totalOperationalTime != null ? totalOperationalTime.map(totalOperationalTime_map -> totalOperationalTime_map) : null);
    ret.pausedTime = (pausedTime != null ? pausedTime.map(pausedTime_map -> pausedTime_map) : null);
    return ret;
  }
}
public static class RvcOperationalStateClusterOperationalErrorEvent implements Cloneable {
  public ChipStructs.RvcOperationalStateClusterErrorStateStruct errorState;
  public RvcOperationalStateClusterOperationalErrorEvent(
    ChipStructs.RvcOperationalStateClusterErrorStateStruct errorState
  ) {
    this.errorState = errorState;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("RvcOperationalStateClusterOperationalErrorEvent {\n");
    output.append("\terrorState: ");
    output.append(errorState);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public RvcOperationalStateClusterOperationalErrorEvent clone() {
    RvcOperationalStateClusterOperationalErrorEvent ret;
    try {
      ret = (RvcOperationalStateClusterOperationalErrorEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.errorState = errorState.clone();
    return ret;
  }
}
public static class RvcOperationalStateClusterOperationCompletionEvent implements Cloneable {
  public Integer completionErrorCode;
  public @Nullable Optional<Long> totalOperationalTime;
  public @Nullable Optional<Long> pausedTime;
  public RvcOperationalStateClusterOperationCompletionEvent(
    Integer completionErrorCode,
    @Nullable Optional<Long> totalOperationalTime,
    @Nullable Optional<Long> pausedTime
  ) {
    this.completionErrorCode = completionErrorCode;
    this.totalOperationalTime = totalOperationalTime;
    this.pausedTime = pausedTime;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("RvcOperationalStateClusterOperationCompletionEvent {\n");
    output.append("\tcompletionErrorCode: ");
    output.append(completionErrorCode);
    output.append("\n");
    output.append("\ttotalOperationalTime: ");
    output.append(totalOperationalTime);
    output.append("\n");
    output.append("\tpausedTime: ");
    output.append(pausedTime);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public RvcOperationalStateClusterOperationCompletionEvent clone() {
    RvcOperationalStateClusterOperationCompletionEvent ret;
    try {
      ret = (RvcOperationalStateClusterOperationCompletionEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.completionErrorCode = completionErrorCode;
    ret.totalOperationalTime = (totalOperationalTime != null ? totalOperationalTime.map(totalOperationalTime_map -> totalOperationalTime_map) : null);
    ret.pausedTime = (pausedTime != null ? pausedTime.map(pausedTime_map -> pausedTime_map) : null);
    return ret;
  }
}
public static class DoorLockClusterDoorLockAlarmEvent implements Cloneable {
  public Integer alarmCode;
  public DoorLockClusterDoorLockAlarmEvent(
    Integer alarmCode
  ) {
    this.alarmCode = alarmCode;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("DoorLockClusterDoorLockAlarmEvent {\n");
    output.append("\talarmCode: ");
    output.append(alarmCode);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public DoorLockClusterDoorLockAlarmEvent clone() {
    DoorLockClusterDoorLockAlarmEvent ret;
    try {
      ret = (DoorLockClusterDoorLockAlarmEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.alarmCode = alarmCode;
    return ret;
  }
}
public static class DoorLockClusterDoorStateChangeEvent implements Cloneable {
  public Integer doorState;
  public DoorLockClusterDoorStateChangeEvent(
    Integer doorState
  ) {
    this.doorState = doorState;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("DoorLockClusterDoorStateChangeEvent {\n");
    output.append("\tdoorState: ");
    output.append(doorState);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public DoorLockClusterDoorStateChangeEvent clone() {
    DoorLockClusterDoorStateChangeEvent ret;
    try {
      ret = (DoorLockClusterDoorStateChangeEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.doorState = doorState;
    return ret;
  }
}
public static class DoorLockClusterLockOperationEvent implements Cloneable {
  public Integer lockOperationType;
  public Integer operationSource;
  public @Nullable Integer userIndex;
  public @Nullable Integer fabricIndex;
  public @Nullable Long sourceNode;
  public @Nullable Optional<ArrayList<ChipStructs.DoorLockClusterCredentialStruct>> credentials;
  public DoorLockClusterLockOperationEvent(
    Integer lockOperationType,
    Integer operationSource,
    @Nullable Integer userIndex,
    @Nullable Integer fabricIndex,
    @Nullable Long sourceNode,
    @Nullable Optional<ArrayList<ChipStructs.DoorLockClusterCredentialStruct>> credentials
  ) {
    this.lockOperationType = lockOperationType;
    this.operationSource = operationSource;
    this.userIndex = userIndex;
    this.fabricIndex = fabricIndex;
    this.sourceNode = sourceNode;
    this.credentials = credentials;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("DoorLockClusterLockOperationEvent {\n");
    output.append("\tlockOperationType: ");
    output.append(lockOperationType);
    output.append("\n");
    output.append("\toperationSource: ");
    output.append(operationSource);
    output.append("\n");
    output.append("\tuserIndex: ");
    output.append(userIndex);
    output.append("\n");
    output.append("\tfabricIndex: ");
    output.append(fabricIndex);
    output.append("\n");
    output.append("\tsourceNode: ");
    output.append(sourceNode);
    output.append("\n");
    output.append("\tcredentials: ");
    output.append(credentials);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public DoorLockClusterLockOperationEvent clone() {
    DoorLockClusterLockOperationEvent ret;
    try {
      ret = (DoorLockClusterLockOperationEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.lockOperationType = lockOperationType;
    ret.operationSource = operationSource;
    ret.userIndex = (userIndex != null ? userIndex : null);
    ret.fabricIndex = (fabricIndex != null ? fabricIndex : null);
    ret.sourceNode = (sourceNode != null ? sourceNode : null);
    ret.credentials = (credentials != null ? credentials.map(credentials_map -> arrayClone(credentials_map, credentials_map_it -> credentials_map_it.clone())) : null);
    return ret;
  }
}
public static class DoorLockClusterLockOperationErrorEvent implements Cloneable {
  public Integer lockOperationType;
  public Integer operationSource;
  public Integer operationError;
  public @Nullable Integer userIndex;
  public @Nullable Integer fabricIndex;
  public @Nullable Long sourceNode;
  public @Nullable Optional<ArrayList<ChipStructs.DoorLockClusterCredentialStruct>> credentials;
  public DoorLockClusterLockOperationErrorEvent(
    Integer lockOperationType,
    Integer operationSource,
    Integer operationError,
    @Nullable Integer userIndex,
    @Nullable Integer fabricIndex,
    @Nullable Long sourceNode,
    @Nullable Optional<ArrayList<ChipStructs.DoorLockClusterCredentialStruct>> credentials
  ) {
    this.lockOperationType = lockOperationType;
    this.operationSource = operationSource;
    this.operationError = operationError;
    this.userIndex = userIndex;
    this.fabricIndex = fabricIndex;
    this.sourceNode = sourceNode;
    this.credentials = credentials;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("DoorLockClusterLockOperationErrorEvent {\n");
    output.append("\tlockOperationType: ");
    output.append(lockOperationType);
    output.append("\n");
    output.append("\toperationSource: ");
    output.append(operationSource);
    output.append("\n");
    output.append("\toperationError: ");
    output.append(operationError);
    output.append("\n");
    output.append("\tuserIndex: ");
    output.append(userIndex);
    output.append("\n");
    output.append("\tfabricIndex: ");
    output.append(fabricIndex);
    output.append("\n");
    output.append("\tsourceNode: ");
    output.append(sourceNode);
    output.append("\n");
    output.append("\tcredentials: ");
    output.append(credentials);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public DoorLockClusterLockOperationErrorEvent clone() {
    DoorLockClusterLockOperationErrorEvent ret;
    try {
      ret = (DoorLockClusterLockOperationErrorEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.lockOperationType = lockOperationType;
    ret.operationSource = operationSource;
    ret.operationError = operationError;
    ret.userIndex = (userIndex != null ? userIndex : null);
    ret.fabricIndex = (fabricIndex != null ? fabricIndex : null);
    ret.sourceNode = (sourceNode != null ? sourceNode : null);
    ret.credentials = (credentials != null ? credentials.map(credentials_map -> arrayClone(credentials_map, credentials_map_it -> credentials_map_it.clone())) : null);
    return ret;
  }
}
public static class DoorLockClusterLockUserChangeEvent implements Cloneable {
  public Integer lockDataType;
  public Integer dataOperationType;
  public Integer operationSource;
  public @Nullable Integer userIndex;
  public @Nullable Integer fabricIndex;
  public @Nullable Long sourceNode;
  public @Nullable Integer dataIndex;
  public DoorLockClusterLockUserChangeEvent(
    Integer lockDataType,
    Integer dataOperationType,
    Integer operationSource,
    @Nullable Integer userIndex,
    @Nullable Integer fabricIndex,
    @Nullable Long sourceNode,
    @Nullable Integer dataIndex
  ) {
    this.lockDataType = lockDataType;
    this.dataOperationType = dataOperationType;
    this.operationSource = operationSource;
    this.userIndex = userIndex;
    this.fabricIndex = fabricIndex;
    this.sourceNode = sourceNode;
    this.dataIndex = dataIndex;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("DoorLockClusterLockUserChangeEvent {\n");
    output.append("\tlockDataType: ");
    output.append(lockDataType);
    output.append("\n");
    output.append("\tdataOperationType: ");
    output.append(dataOperationType);
    output.append("\n");
    output.append("\toperationSource: ");
    output.append(operationSource);
    output.append("\n");
    output.append("\tuserIndex: ");
    output.append(userIndex);
    output.append("\n");
    output.append("\tfabricIndex: ");
    output.append(fabricIndex);
    output.append("\n");
    output.append("\tsourceNode: ");
    output.append(sourceNode);
    output.append("\n");
    output.append("\tdataIndex: ");
    output.append(dataIndex);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public DoorLockClusterLockUserChangeEvent clone() {
    DoorLockClusterLockUserChangeEvent ret;
    try {
      ret = (DoorLockClusterLockUserChangeEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.lockDataType = lockDataType;
    ret.dataOperationType = dataOperationType;
    ret.operationSource = operationSource;
    ret.userIndex = (userIndex != null ? userIndex : null);
    ret.fabricIndex = (fabricIndex != null ? fabricIndex : null);
    ret.sourceNode = (sourceNode != null ? sourceNode : null);
    ret.dataIndex = (dataIndex != null ? dataIndex : null);
    return ret;
  }
}
public static class UnitTestingClusterTestEventEvent implements Cloneable {
  public Integer arg1;
  public Integer arg2;
  public Boolean arg3;
  public ChipStructs.UnitTestingClusterSimpleStruct arg4;
  public ArrayList<ChipStructs.UnitTestingClusterSimpleStruct> arg5;
  public ArrayList<Integer> arg6;
  public UnitTestingClusterTestEventEvent(
    Integer arg1,
    Integer arg2,
    Boolean arg3,
    ChipStructs.UnitTestingClusterSimpleStruct arg4,
    ArrayList<ChipStructs.UnitTestingClusterSimpleStruct> arg5,
    ArrayList<Integer> arg6
  ) {
    this.arg1 = arg1;
    this.arg2 = arg2;
    this.arg3 = arg3;
    this.arg4 = arg4;
    this.arg5 = arg5;
    this.arg6 = arg6;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("UnitTestingClusterTestEventEvent {\n");
    output.append("\targ1: ");
    output.append(arg1);
    output.append("\n");
    output.append("\targ2: ");
    output.append(arg2);
    output.append("\n");
    output.append("\targ3: ");
    output.append(arg3);
    output.append("\n");
    output.append("\targ4: ");
    output.append(arg4);
    output.append("\n");
    output.append("\targ5: ");
    output.append(arg5);
    output.append("\n");
    output.append("\targ6: ");
    output.append(arg6);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public UnitTestingClusterTestEventEvent clone() {
    UnitTestingClusterTestEventEvent ret;
    try {
      ret = (UnitTestingClusterTestEventEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.arg1 = arg1;
    ret.arg2 = arg2;
    ret.arg3 = arg3;
    ret.arg4 = arg4.clone();
    ret.arg5 = arrayClone(arg5, arg5_it -> arg5_it.clone());
    ret.arg6 = arrayClone(arg6, arg6_it -> arg6_it);
    return ret;
  }
}
public static class UnitTestingClusterTestFabricScopedEventEvent implements Cloneable {
  public Integer fabricIndex;
  public UnitTestingClusterTestFabricScopedEventEvent(
    Integer fabricIndex
  ) {
    this.fabricIndex = fabricIndex;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("UnitTestingClusterTestFabricScopedEventEvent {\n");
    output.append("\tfabricIndex: ");
    output.append(fabricIndex);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }

  @Override
  public UnitTestingClusterTestFabricScopedEventEvent clone() {
    UnitTestingClusterTestFabricScopedEventEvent ret;
    try {
      ret = (UnitTestingClusterTestFabricScopedEventEvent)super.clone();
    } catch (CloneNotSupportedException e) {
      return null;
    }
    ret.fabricIndex = fabricIndex;
    return ret;
  }
}
  private interface ArrayCloneFunction<T> {
    T cloneFunction(T input) throws CloneNotSupportedException;
  }

  private static<T> ArrayList<T> arrayClone(ArrayList<T> inputArray, ArrayCloneFunction<T> func) {
    ArrayList<T> ret = new ArrayList<T>(inputArray.size());
    for (T it : inputArray) {
      try {
        ret.add(func.cloneFunction(it));
      } catch (CloneNotSupportedException e) {
        // Ignore
      }
    }
    return ret;
  }
}
