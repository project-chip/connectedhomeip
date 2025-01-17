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

import static chip.devicecontroller.ChipTLVType.*;

public class ChipEventStructs {
public static class AccessControlClusterAccessControlEntryChangedEvent {
  public @Nullable Long adminNodeID;
  public @Nullable Integer adminPasscodeID;
  public Integer changeType;
  public @Nullable ChipStructs.AccessControlClusterAccessControlEntryStruct latestValue;
  public Integer fabricIndex;
  private static final long ADMIN_NODE_ID_ID = 1L;
  private static final long ADMIN_PASSCODE_ID_ID = 2L;
  private static final long CHANGE_TYPE_ID = 3L;
  private static final long LATEST_VALUE_ID = 4L;
  private static final long FABRIC_INDEX_ID = 254L;

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

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ADMIN_NODE_ID_ID, adminNodeID != null ? new UIntType(adminNodeID) : new NullType()));
    values.add(new StructElement(ADMIN_PASSCODE_ID_ID, adminPasscodeID != null ? new UIntType(adminPasscodeID) : new NullType()));
    values.add(new StructElement(CHANGE_TYPE_ID, new UIntType(changeType)));
    values.add(new StructElement(LATEST_VALUE_ID, latestValue != null ? latestValue.encodeTlv() : new NullType()));
    values.add(new StructElement(FABRIC_INDEX_ID, new UIntType(fabricIndex)));

    return new StructType(values);
  }

  public static AccessControlClusterAccessControlEntryChangedEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    @Nullable Long adminNodeID = null;
    @Nullable Integer adminPasscodeID = null;
    Integer changeType = null;
    @Nullable ChipStructs.AccessControlClusterAccessControlEntryStruct latestValue = null;
    Integer fabricIndex = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ADMIN_NODE_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          adminNodeID = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == ADMIN_PASSCODE_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          adminPasscodeID = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == CHANGE_TYPE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          changeType = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == LATEST_VALUE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          latestValue = ChipStructs.AccessControlClusterAccessControlEntryStruct.decodeTlv(castingValue);
        }
      } else if (element.contextTagNum() == FABRIC_INDEX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          fabricIndex = castingValue.value(Integer.class);
        }
      }
    }
    return new AccessControlClusterAccessControlEntryChangedEvent(
      adminNodeID,
      adminPasscodeID,
      changeType,
      latestValue,
      fabricIndex
    );
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
}
public static class AccessControlClusterAccessControlExtensionChangedEvent {
  public @Nullable Long adminNodeID;
  public @Nullable Integer adminPasscodeID;
  public Integer changeType;
  public @Nullable ChipStructs.AccessControlClusterAccessControlExtensionStruct latestValue;
  public Integer fabricIndex;
  private static final long ADMIN_NODE_ID_ID = 1L;
  private static final long ADMIN_PASSCODE_ID_ID = 2L;
  private static final long CHANGE_TYPE_ID = 3L;
  private static final long LATEST_VALUE_ID = 4L;
  private static final long FABRIC_INDEX_ID = 254L;

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

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ADMIN_NODE_ID_ID, adminNodeID != null ? new UIntType(adminNodeID) : new NullType()));
    values.add(new StructElement(ADMIN_PASSCODE_ID_ID, adminPasscodeID != null ? new UIntType(adminPasscodeID) : new NullType()));
    values.add(new StructElement(CHANGE_TYPE_ID, new UIntType(changeType)));
    values.add(new StructElement(LATEST_VALUE_ID, latestValue != null ? latestValue.encodeTlv() : new NullType()));
    values.add(new StructElement(FABRIC_INDEX_ID, new UIntType(fabricIndex)));

    return new StructType(values);
  }

  public static AccessControlClusterAccessControlExtensionChangedEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    @Nullable Long adminNodeID = null;
    @Nullable Integer adminPasscodeID = null;
    Integer changeType = null;
    @Nullable ChipStructs.AccessControlClusterAccessControlExtensionStruct latestValue = null;
    Integer fabricIndex = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ADMIN_NODE_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          adminNodeID = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == ADMIN_PASSCODE_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          adminPasscodeID = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == CHANGE_TYPE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          changeType = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == LATEST_VALUE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          latestValue = ChipStructs.AccessControlClusterAccessControlExtensionStruct.decodeTlv(castingValue);
        }
      } else if (element.contextTagNum() == FABRIC_INDEX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          fabricIndex = castingValue.value(Integer.class);
        }
      }
    }
    return new AccessControlClusterAccessControlExtensionChangedEvent(
      adminNodeID,
      adminPasscodeID,
      changeType,
      latestValue,
      fabricIndex
    );
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
}
public static class AccessControlClusterFabricRestrictionReviewUpdateEvent {
  public Long token;
  public Optional<String> instruction;
  public Optional<String> ARLRequestFlowUrl;
  public Integer fabricIndex;
  private static final long TOKEN_ID = 0L;
  private static final long INSTRUCTION_ID = 1L;
  private static final long ARL_REQUEST_FLOW_URL_ID = 2L;
  private static final long FABRIC_INDEX_ID = 254L;

  public AccessControlClusterFabricRestrictionReviewUpdateEvent(
    Long token,
    Optional<String> instruction,
    Optional<String> ARLRequestFlowUrl,
    Integer fabricIndex
  ) {
    this.token = token;
    this.instruction = instruction;
    this.ARLRequestFlowUrl = ARLRequestFlowUrl;
    this.fabricIndex = fabricIndex;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(TOKEN_ID, new UIntType(token)));
    values.add(new StructElement(INSTRUCTION_ID, instruction.<BaseTLVType>map((nonOptionalinstruction) -> new StringType(nonOptionalinstruction)).orElse(new EmptyType())));
    values.add(new StructElement(ARL_REQUEST_FLOW_URL_ID, ARLRequestFlowUrl.<BaseTLVType>map((nonOptionalARLRequestFlowUrl) -> new StringType(nonOptionalARLRequestFlowUrl)).orElse(new EmptyType())));
    values.add(new StructElement(FABRIC_INDEX_ID, new UIntType(fabricIndex)));

    return new StructType(values);
  }

  public static AccessControlClusterFabricRestrictionReviewUpdateEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long token = null;
    Optional<String> instruction = Optional.empty();
    Optional<String> ARLRequestFlowUrl = Optional.empty();
    Integer fabricIndex = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == TOKEN_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          token = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == INSTRUCTION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          instruction = Optional.of(castingValue.value(String.class));
        }
      } else if (element.contextTagNum() == ARL_REQUEST_FLOW_URL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          ARLRequestFlowUrl = Optional.of(castingValue.value(String.class));
        }
      } else if (element.contextTagNum() == FABRIC_INDEX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          fabricIndex = castingValue.value(Integer.class);
        }
      }
    }
    return new AccessControlClusterFabricRestrictionReviewUpdateEvent(
      token,
      instruction,
      ARLRequestFlowUrl,
      fabricIndex
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("AccessControlClusterFabricRestrictionReviewUpdateEvent {\n");
    output.append("\ttoken: ");
    output.append(token);
    output.append("\n");
    output.append("\tinstruction: ");
    output.append(instruction);
    output.append("\n");
    output.append("\tARLRequestFlowUrl: ");
    output.append(ARLRequestFlowUrl);
    output.append("\n");
    output.append("\tfabricIndex: ");
    output.append(fabricIndex);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ActionsClusterStateChangedEvent {
  public Integer actionID;
  public Long invokeID;
  public Integer newState;
  private static final long ACTION_ID_ID = 0L;
  private static final long INVOKE_ID_ID = 1L;
  private static final long NEW_STATE_ID = 2L;

  public ActionsClusterStateChangedEvent(
    Integer actionID,
    Long invokeID,
    Integer newState
  ) {
    this.actionID = actionID;
    this.invokeID = invokeID;
    this.newState = newState;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ACTION_ID_ID, new UIntType(actionID)));
    values.add(new StructElement(INVOKE_ID_ID, new UIntType(invokeID)));
    values.add(new StructElement(NEW_STATE_ID, new UIntType(newState)));

    return new StructType(values);
  }

  public static ActionsClusterStateChangedEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer actionID = null;
    Long invokeID = null;
    Integer newState = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ACTION_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          actionID = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == INVOKE_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          invokeID = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == NEW_STATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          newState = castingValue.value(Integer.class);
        }
      }
    }
    return new ActionsClusterStateChangedEvent(
      actionID,
      invokeID,
      newState
    );
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
}
public static class ActionsClusterActionFailedEvent {
  public Integer actionID;
  public Long invokeID;
  public Integer newState;
  public Integer error;
  private static final long ACTION_ID_ID = 0L;
  private static final long INVOKE_ID_ID = 1L;
  private static final long NEW_STATE_ID = 2L;
  private static final long ERROR_ID = 3L;

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

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ACTION_ID_ID, new UIntType(actionID)));
    values.add(new StructElement(INVOKE_ID_ID, new UIntType(invokeID)));
    values.add(new StructElement(NEW_STATE_ID, new UIntType(newState)));
    values.add(new StructElement(ERROR_ID, new UIntType(error)));

    return new StructType(values);
  }

  public static ActionsClusterActionFailedEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer actionID = null;
    Long invokeID = null;
    Integer newState = null;
    Integer error = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ACTION_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          actionID = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == INVOKE_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          invokeID = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == NEW_STATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          newState = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == ERROR_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          error = castingValue.value(Integer.class);
        }
      }
    }
    return new ActionsClusterActionFailedEvent(
      actionID,
      invokeID,
      newState,
      error
    );
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
}
public static class BasicInformationClusterStartUpEvent {
  public Long softwareVersion;
  private static final long SOFTWARE_VERSION_ID = 0L;

  public BasicInformationClusterStartUpEvent(
    Long softwareVersion
  ) {
    this.softwareVersion = softwareVersion;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(SOFTWARE_VERSION_ID, new UIntType(softwareVersion)));

    return new StructType(values);
  }

  public static BasicInformationClusterStartUpEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long softwareVersion = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == SOFTWARE_VERSION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          softwareVersion = castingValue.value(Long.class);
        }
      }
    }
    return new BasicInformationClusterStartUpEvent(
      softwareVersion
    );
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
}
public static class BasicInformationClusterShutDownEvent {

  public BasicInformationClusterShutDownEvent(
  ) {
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();

    return new StructType(values);
  }

  public static BasicInformationClusterShutDownEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    return new BasicInformationClusterShutDownEvent(
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("BasicInformationClusterShutDownEvent {\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class BasicInformationClusterLeaveEvent {
  public Integer fabricIndex;
  private static final long FABRIC_INDEX_ID = 0L;

  public BasicInformationClusterLeaveEvent(
    Integer fabricIndex
  ) {
    this.fabricIndex = fabricIndex;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(FABRIC_INDEX_ID, new UIntType(fabricIndex)));

    return new StructType(values);
  }

  public static BasicInformationClusterLeaveEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer fabricIndex = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == FABRIC_INDEX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          fabricIndex = castingValue.value(Integer.class);
        }
      }
    }
    return new BasicInformationClusterLeaveEvent(
      fabricIndex
    );
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
}
public static class BasicInformationClusterReachableChangedEvent {
  public Boolean reachableNewValue;
  private static final long REACHABLE_NEW_VALUE_ID = 0L;

  public BasicInformationClusterReachableChangedEvent(
    Boolean reachableNewValue
  ) {
    this.reachableNewValue = reachableNewValue;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(REACHABLE_NEW_VALUE_ID, new BooleanType(reachableNewValue)));

    return new StructType(values);
  }

  public static BasicInformationClusterReachableChangedEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Boolean reachableNewValue = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == REACHABLE_NEW_VALUE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          reachableNewValue = castingValue.value(Boolean.class);
        }
      }
    }
    return new BasicInformationClusterReachableChangedEvent(
      reachableNewValue
    );
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
}
public static class OtaSoftwareUpdateRequestorClusterStateTransitionEvent {
  public Integer previousState;
  public Integer newState;
  public Integer reason;
  public @Nullable Long targetSoftwareVersion;
  private static final long PREVIOUS_STATE_ID = 0L;
  private static final long NEW_STATE_ID = 1L;
  private static final long REASON_ID = 2L;
  private static final long TARGET_SOFTWARE_VERSION_ID = 3L;

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

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(PREVIOUS_STATE_ID, new UIntType(previousState)));
    values.add(new StructElement(NEW_STATE_ID, new UIntType(newState)));
    values.add(new StructElement(REASON_ID, new UIntType(reason)));
    values.add(new StructElement(TARGET_SOFTWARE_VERSION_ID, targetSoftwareVersion != null ? new UIntType(targetSoftwareVersion) : new NullType()));

    return new StructType(values);
  }

  public static OtaSoftwareUpdateRequestorClusterStateTransitionEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer previousState = null;
    Integer newState = null;
    Integer reason = null;
    @Nullable Long targetSoftwareVersion = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == PREVIOUS_STATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          previousState = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == NEW_STATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          newState = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == REASON_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          reason = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == TARGET_SOFTWARE_VERSION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          targetSoftwareVersion = castingValue.value(Long.class);
        }
      }
    }
    return new OtaSoftwareUpdateRequestorClusterStateTransitionEvent(
      previousState,
      newState,
      reason,
      targetSoftwareVersion
    );
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
}
public static class OtaSoftwareUpdateRequestorClusterVersionAppliedEvent {
  public Long softwareVersion;
  public Integer productID;
  private static final long SOFTWARE_VERSION_ID = 0L;
  private static final long PRODUCT_ID_ID = 1L;

  public OtaSoftwareUpdateRequestorClusterVersionAppliedEvent(
    Long softwareVersion,
    Integer productID
  ) {
    this.softwareVersion = softwareVersion;
    this.productID = productID;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(SOFTWARE_VERSION_ID, new UIntType(softwareVersion)));
    values.add(new StructElement(PRODUCT_ID_ID, new UIntType(productID)));

    return new StructType(values);
  }

  public static OtaSoftwareUpdateRequestorClusterVersionAppliedEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long softwareVersion = null;
    Integer productID = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == SOFTWARE_VERSION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          softwareVersion = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == PRODUCT_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          productID = castingValue.value(Integer.class);
        }
      }
    }
    return new OtaSoftwareUpdateRequestorClusterVersionAppliedEvent(
      softwareVersion,
      productID
    );
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
}
public static class OtaSoftwareUpdateRequestorClusterDownloadErrorEvent {
  public Long softwareVersion;
  public Long bytesDownloaded;
  public @Nullable Integer progressPercent;
  public @Nullable Long platformCode;
  private static final long SOFTWARE_VERSION_ID = 0L;
  private static final long BYTES_DOWNLOADED_ID = 1L;
  private static final long PROGRESS_PERCENT_ID = 2L;
  private static final long PLATFORM_CODE_ID = 3L;

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

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(SOFTWARE_VERSION_ID, new UIntType(softwareVersion)));
    values.add(new StructElement(BYTES_DOWNLOADED_ID, new UIntType(bytesDownloaded)));
    values.add(new StructElement(PROGRESS_PERCENT_ID, progressPercent != null ? new UIntType(progressPercent) : new NullType()));
    values.add(new StructElement(PLATFORM_CODE_ID, platformCode != null ? new IntType(platformCode) : new NullType()));

    return new StructType(values);
  }

  public static OtaSoftwareUpdateRequestorClusterDownloadErrorEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long softwareVersion = null;
    Long bytesDownloaded = null;
    @Nullable Integer progressPercent = null;
    @Nullable Long platformCode = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == SOFTWARE_VERSION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          softwareVersion = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == BYTES_DOWNLOADED_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          bytesDownloaded = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == PROGRESS_PERCENT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          progressPercent = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == PLATFORM_CODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          platformCode = castingValue.value(Long.class);
        }
      }
    }
    return new OtaSoftwareUpdateRequestorClusterDownloadErrorEvent(
      softwareVersion,
      bytesDownloaded,
      progressPercent,
      platformCode
    );
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
}
public static class PowerSourceClusterWiredFaultChangeEvent {
  public ArrayList<Integer> current;
  public ArrayList<Integer> previous;
  private static final long CURRENT_ID = 0L;
  private static final long PREVIOUS_ID = 1L;

  public PowerSourceClusterWiredFaultChangeEvent(
    ArrayList<Integer> current,
    ArrayList<Integer> previous
  ) {
    this.current = current;
    this.previous = previous;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(CURRENT_ID, ArrayType.generateArrayType(current, (elementcurrent) -> new UIntType(elementcurrent))));
    values.add(new StructElement(PREVIOUS_ID, ArrayType.generateArrayType(previous, (elementprevious) -> new UIntType(elementprevious))));

    return new StructType(values);
  }

  public static PowerSourceClusterWiredFaultChangeEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    ArrayList<Integer> current = null;
    ArrayList<Integer> previous = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == CURRENT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          current = castingValue.map((elementcastingValue) -> elementcastingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == PREVIOUS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          previous = castingValue.map((elementcastingValue) -> elementcastingValue.value(Integer.class));
        }
      }
    }
    return new PowerSourceClusterWiredFaultChangeEvent(
      current,
      previous
    );
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
}
public static class PowerSourceClusterBatFaultChangeEvent {
  public ArrayList<Integer> current;
  public ArrayList<Integer> previous;
  private static final long CURRENT_ID = 0L;
  private static final long PREVIOUS_ID = 1L;

  public PowerSourceClusterBatFaultChangeEvent(
    ArrayList<Integer> current,
    ArrayList<Integer> previous
  ) {
    this.current = current;
    this.previous = previous;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(CURRENT_ID, ArrayType.generateArrayType(current, (elementcurrent) -> new UIntType(elementcurrent))));
    values.add(new StructElement(PREVIOUS_ID, ArrayType.generateArrayType(previous, (elementprevious) -> new UIntType(elementprevious))));

    return new StructType(values);
  }

  public static PowerSourceClusterBatFaultChangeEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    ArrayList<Integer> current = null;
    ArrayList<Integer> previous = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == CURRENT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          current = castingValue.map((elementcastingValue) -> elementcastingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == PREVIOUS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          previous = castingValue.map((elementcastingValue) -> elementcastingValue.value(Integer.class));
        }
      }
    }
    return new PowerSourceClusterBatFaultChangeEvent(
      current,
      previous
    );
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
}
public static class PowerSourceClusterBatChargeFaultChangeEvent {
  public ArrayList<Integer> current;
  public ArrayList<Integer> previous;
  private static final long CURRENT_ID = 0L;
  private static final long PREVIOUS_ID = 1L;

  public PowerSourceClusterBatChargeFaultChangeEvent(
    ArrayList<Integer> current,
    ArrayList<Integer> previous
  ) {
    this.current = current;
    this.previous = previous;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(CURRENT_ID, ArrayType.generateArrayType(current, (elementcurrent) -> new UIntType(elementcurrent))));
    values.add(new StructElement(PREVIOUS_ID, ArrayType.generateArrayType(previous, (elementprevious) -> new UIntType(elementprevious))));

    return new StructType(values);
  }

  public static PowerSourceClusterBatChargeFaultChangeEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    ArrayList<Integer> current = null;
    ArrayList<Integer> previous = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == CURRENT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          current = castingValue.map((elementcastingValue) -> elementcastingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == PREVIOUS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          previous = castingValue.map((elementcastingValue) -> elementcastingValue.value(Integer.class));
        }
      }
    }
    return new PowerSourceClusterBatChargeFaultChangeEvent(
      current,
      previous
    );
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
}
public static class GeneralDiagnosticsClusterHardwareFaultChangeEvent {
  public ArrayList<Integer> current;
  public ArrayList<Integer> previous;
  private static final long CURRENT_ID = 0L;
  private static final long PREVIOUS_ID = 1L;

  public GeneralDiagnosticsClusterHardwareFaultChangeEvent(
    ArrayList<Integer> current,
    ArrayList<Integer> previous
  ) {
    this.current = current;
    this.previous = previous;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(CURRENT_ID, ArrayType.generateArrayType(current, (elementcurrent) -> new UIntType(elementcurrent))));
    values.add(new StructElement(PREVIOUS_ID, ArrayType.generateArrayType(previous, (elementprevious) -> new UIntType(elementprevious))));

    return new StructType(values);
  }

  public static GeneralDiagnosticsClusterHardwareFaultChangeEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    ArrayList<Integer> current = null;
    ArrayList<Integer> previous = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == CURRENT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          current = castingValue.map((elementcastingValue) -> elementcastingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == PREVIOUS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          previous = castingValue.map((elementcastingValue) -> elementcastingValue.value(Integer.class));
        }
      }
    }
    return new GeneralDiagnosticsClusterHardwareFaultChangeEvent(
      current,
      previous
    );
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
}
public static class GeneralDiagnosticsClusterRadioFaultChangeEvent {
  public ArrayList<Integer> current;
  public ArrayList<Integer> previous;
  private static final long CURRENT_ID = 0L;
  private static final long PREVIOUS_ID = 1L;

  public GeneralDiagnosticsClusterRadioFaultChangeEvent(
    ArrayList<Integer> current,
    ArrayList<Integer> previous
  ) {
    this.current = current;
    this.previous = previous;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(CURRENT_ID, ArrayType.generateArrayType(current, (elementcurrent) -> new UIntType(elementcurrent))));
    values.add(new StructElement(PREVIOUS_ID, ArrayType.generateArrayType(previous, (elementprevious) -> new UIntType(elementprevious))));

    return new StructType(values);
  }

  public static GeneralDiagnosticsClusterRadioFaultChangeEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    ArrayList<Integer> current = null;
    ArrayList<Integer> previous = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == CURRENT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          current = castingValue.map((elementcastingValue) -> elementcastingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == PREVIOUS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          previous = castingValue.map((elementcastingValue) -> elementcastingValue.value(Integer.class));
        }
      }
    }
    return new GeneralDiagnosticsClusterRadioFaultChangeEvent(
      current,
      previous
    );
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
}
public static class GeneralDiagnosticsClusterNetworkFaultChangeEvent {
  public ArrayList<Integer> current;
  public ArrayList<Integer> previous;
  private static final long CURRENT_ID = 0L;
  private static final long PREVIOUS_ID = 1L;

  public GeneralDiagnosticsClusterNetworkFaultChangeEvent(
    ArrayList<Integer> current,
    ArrayList<Integer> previous
  ) {
    this.current = current;
    this.previous = previous;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(CURRENT_ID, ArrayType.generateArrayType(current, (elementcurrent) -> new UIntType(elementcurrent))));
    values.add(new StructElement(PREVIOUS_ID, ArrayType.generateArrayType(previous, (elementprevious) -> new UIntType(elementprevious))));

    return new StructType(values);
  }

  public static GeneralDiagnosticsClusterNetworkFaultChangeEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    ArrayList<Integer> current = null;
    ArrayList<Integer> previous = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == CURRENT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          current = castingValue.map((elementcastingValue) -> elementcastingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == PREVIOUS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          previous = castingValue.map((elementcastingValue) -> elementcastingValue.value(Integer.class));
        }
      }
    }
    return new GeneralDiagnosticsClusterNetworkFaultChangeEvent(
      current,
      previous
    );
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
}
public static class GeneralDiagnosticsClusterBootReasonEvent {
  public Integer bootReason;
  private static final long BOOT_REASON_ID = 0L;

  public GeneralDiagnosticsClusterBootReasonEvent(
    Integer bootReason
  ) {
    this.bootReason = bootReason;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(BOOT_REASON_ID, new UIntType(bootReason)));

    return new StructType(values);
  }

  public static GeneralDiagnosticsClusterBootReasonEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer bootReason = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == BOOT_REASON_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          bootReason = castingValue.value(Integer.class);
        }
      }
    }
    return new GeneralDiagnosticsClusterBootReasonEvent(
      bootReason
    );
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
}
public static class SoftwareDiagnosticsClusterSoftwareFaultEvent {
  public Long id;
  public Optional<String> name;
  public Optional<byte[]> faultRecording;
  private static final long ID_ID = 0L;
  private static final long NAME_ID = 1L;
  private static final long FAULT_RECORDING_ID = 2L;

  public SoftwareDiagnosticsClusterSoftwareFaultEvent(
    Long id,
    Optional<String> name,
    Optional<byte[]> faultRecording
  ) {
    this.id = id;
    this.name = name;
    this.faultRecording = faultRecording;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ID_ID, new UIntType(id)));
    values.add(new StructElement(NAME_ID, name.<BaseTLVType>map((nonOptionalname) -> new StringType(nonOptionalname)).orElse(new EmptyType())));
    values.add(new StructElement(FAULT_RECORDING_ID, faultRecording.<BaseTLVType>map((nonOptionalfaultRecording) -> new ByteArrayType(nonOptionalfaultRecording)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static SoftwareDiagnosticsClusterSoftwareFaultEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long id = null;
    Optional<String> name = Optional.empty();
    Optional<byte[]> faultRecording = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          id = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == NAME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          name = Optional.of(castingValue.value(String.class));
        }
      } else if (element.contextTagNum() == FAULT_RECORDING_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.ByteArray) {
          ByteArrayType castingValue = element.value(ByteArrayType.class);
          faultRecording = Optional.of(castingValue.value(byte[].class));
        }
      }
    }
    return new SoftwareDiagnosticsClusterSoftwareFaultEvent(
      id,
      name,
      faultRecording
    );
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
}
public static class ThreadNetworkDiagnosticsClusterConnectionStatusEvent {
  public Integer connectionStatus;
  private static final long CONNECTION_STATUS_ID = 0L;

  public ThreadNetworkDiagnosticsClusterConnectionStatusEvent(
    Integer connectionStatus
  ) {
    this.connectionStatus = connectionStatus;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(CONNECTION_STATUS_ID, new UIntType(connectionStatus)));

    return new StructType(values);
  }

  public static ThreadNetworkDiagnosticsClusterConnectionStatusEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer connectionStatus = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == CONNECTION_STATUS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          connectionStatus = castingValue.value(Integer.class);
        }
      }
    }
    return new ThreadNetworkDiagnosticsClusterConnectionStatusEvent(
      connectionStatus
    );
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
}
public static class ThreadNetworkDiagnosticsClusterNetworkFaultChangeEvent {
  public ArrayList<Integer> current;
  public ArrayList<Integer> previous;
  private static final long CURRENT_ID = 0L;
  private static final long PREVIOUS_ID = 1L;

  public ThreadNetworkDiagnosticsClusterNetworkFaultChangeEvent(
    ArrayList<Integer> current,
    ArrayList<Integer> previous
  ) {
    this.current = current;
    this.previous = previous;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(CURRENT_ID, ArrayType.generateArrayType(current, (elementcurrent) -> new UIntType(elementcurrent))));
    values.add(new StructElement(PREVIOUS_ID, ArrayType.generateArrayType(previous, (elementprevious) -> new UIntType(elementprevious))));

    return new StructType(values);
  }

  public static ThreadNetworkDiagnosticsClusterNetworkFaultChangeEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    ArrayList<Integer> current = null;
    ArrayList<Integer> previous = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == CURRENT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          current = castingValue.map((elementcastingValue) -> elementcastingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == PREVIOUS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          previous = castingValue.map((elementcastingValue) -> elementcastingValue.value(Integer.class));
        }
      }
    }
    return new ThreadNetworkDiagnosticsClusterNetworkFaultChangeEvent(
      current,
      previous
    );
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
}
public static class WiFiNetworkDiagnosticsClusterDisconnectionEvent {
  public Integer reasonCode;
  private static final long REASON_CODE_ID = 0L;

  public WiFiNetworkDiagnosticsClusterDisconnectionEvent(
    Integer reasonCode
  ) {
    this.reasonCode = reasonCode;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(REASON_CODE_ID, new UIntType(reasonCode)));

    return new StructType(values);
  }

  public static WiFiNetworkDiagnosticsClusterDisconnectionEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer reasonCode = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == REASON_CODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          reasonCode = castingValue.value(Integer.class);
        }
      }
    }
    return new WiFiNetworkDiagnosticsClusterDisconnectionEvent(
      reasonCode
    );
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
}
public static class WiFiNetworkDiagnosticsClusterAssociationFailureEvent {
  public Integer associationFailureCause;
  public Integer status;
  private static final long ASSOCIATION_FAILURE_CAUSE_ID = 0L;
  private static final long STATUS_ID = 1L;

  public WiFiNetworkDiagnosticsClusterAssociationFailureEvent(
    Integer associationFailureCause,
    Integer status
  ) {
    this.associationFailureCause = associationFailureCause;
    this.status = status;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ASSOCIATION_FAILURE_CAUSE_ID, new UIntType(associationFailureCause)));
    values.add(new StructElement(STATUS_ID, new UIntType(status)));

    return new StructType(values);
  }

  public static WiFiNetworkDiagnosticsClusterAssociationFailureEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer associationFailureCause = null;
    Integer status = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ASSOCIATION_FAILURE_CAUSE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          associationFailureCause = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == STATUS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          status = castingValue.value(Integer.class);
        }
      }
    }
    return new WiFiNetworkDiagnosticsClusterAssociationFailureEvent(
      associationFailureCause,
      status
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("WiFiNetworkDiagnosticsClusterAssociationFailureEvent {\n");
    output.append("\tassociationFailureCause: ");
    output.append(associationFailureCause);
    output.append("\n");
    output.append("\tstatus: ");
    output.append(status);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class WiFiNetworkDiagnosticsClusterConnectionStatusEvent {
  public Integer connectionStatus;
  private static final long CONNECTION_STATUS_ID = 0L;

  public WiFiNetworkDiagnosticsClusterConnectionStatusEvent(
    Integer connectionStatus
  ) {
    this.connectionStatus = connectionStatus;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(CONNECTION_STATUS_ID, new UIntType(connectionStatus)));

    return new StructType(values);
  }

  public static WiFiNetworkDiagnosticsClusterConnectionStatusEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer connectionStatus = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == CONNECTION_STATUS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          connectionStatus = castingValue.value(Integer.class);
        }
      }
    }
    return new WiFiNetworkDiagnosticsClusterConnectionStatusEvent(
      connectionStatus
    );
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
}
public static class TimeSynchronizationClusterDSTTableEmptyEvent {

  public TimeSynchronizationClusterDSTTableEmptyEvent(
  ) {
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();

    return new StructType(values);
  }

  public static TimeSynchronizationClusterDSTTableEmptyEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    return new TimeSynchronizationClusterDSTTableEmptyEvent(
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("TimeSynchronizationClusterDSTTableEmptyEvent {\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class TimeSynchronizationClusterDSTStatusEvent {
  public Boolean DSTOffsetActive;
  private static final long DST_OFFSET_ACTIVE_ID = 0L;

  public TimeSynchronizationClusterDSTStatusEvent(
    Boolean DSTOffsetActive
  ) {
    this.DSTOffsetActive = DSTOffsetActive;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(DST_OFFSET_ACTIVE_ID, new BooleanType(DSTOffsetActive)));

    return new StructType(values);
  }

  public static TimeSynchronizationClusterDSTStatusEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Boolean DSTOffsetActive = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == DST_OFFSET_ACTIVE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          DSTOffsetActive = castingValue.value(Boolean.class);
        }
      }
    }
    return new TimeSynchronizationClusterDSTStatusEvent(
      DSTOffsetActive
    );
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
}
public static class TimeSynchronizationClusterTimeZoneStatusEvent {
  public Long offset;
  public Optional<String> name;
  private static final long OFFSET_ID = 0L;
  private static final long NAME_ID = 1L;

  public TimeSynchronizationClusterTimeZoneStatusEvent(
    Long offset,
    Optional<String> name
  ) {
    this.offset = offset;
    this.name = name;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(OFFSET_ID, new IntType(offset)));
    values.add(new StructElement(NAME_ID, name.<BaseTLVType>map((nonOptionalname) -> new StringType(nonOptionalname)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static TimeSynchronizationClusterTimeZoneStatusEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long offset = null;
    Optional<String> name = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == OFFSET_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          offset = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == NAME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          name = Optional.of(castingValue.value(String.class));
        }
      }
    }
    return new TimeSynchronizationClusterTimeZoneStatusEvent(
      offset,
      name
    );
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
}
public static class TimeSynchronizationClusterTimeFailureEvent {

  public TimeSynchronizationClusterTimeFailureEvent(
  ) {
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();

    return new StructType(values);
  }

  public static TimeSynchronizationClusterTimeFailureEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    return new TimeSynchronizationClusterTimeFailureEvent(
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("TimeSynchronizationClusterTimeFailureEvent {\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class TimeSynchronizationClusterMissingTrustedTimeSourceEvent {

  public TimeSynchronizationClusterMissingTrustedTimeSourceEvent(
  ) {
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();

    return new StructType(values);
  }

  public static TimeSynchronizationClusterMissingTrustedTimeSourceEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    return new TimeSynchronizationClusterMissingTrustedTimeSourceEvent(
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("TimeSynchronizationClusterMissingTrustedTimeSourceEvent {\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class BridgedDeviceBasicInformationClusterStartUpEvent {
  public Long softwareVersion;
  private static final long SOFTWARE_VERSION_ID = 0L;

  public BridgedDeviceBasicInformationClusterStartUpEvent(
    Long softwareVersion
  ) {
    this.softwareVersion = softwareVersion;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(SOFTWARE_VERSION_ID, new UIntType(softwareVersion)));

    return new StructType(values);
  }

  public static BridgedDeviceBasicInformationClusterStartUpEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long softwareVersion = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == SOFTWARE_VERSION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          softwareVersion = castingValue.value(Long.class);
        }
      }
    }
    return new BridgedDeviceBasicInformationClusterStartUpEvent(
      softwareVersion
    );
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
}
public static class BridgedDeviceBasicInformationClusterShutDownEvent {

  public BridgedDeviceBasicInformationClusterShutDownEvent(
  ) {
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();

    return new StructType(values);
  }

  public static BridgedDeviceBasicInformationClusterShutDownEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    return new BridgedDeviceBasicInformationClusterShutDownEvent(
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("BridgedDeviceBasicInformationClusterShutDownEvent {\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class BridgedDeviceBasicInformationClusterLeaveEvent {

  public BridgedDeviceBasicInformationClusterLeaveEvent(
  ) {
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();

    return new StructType(values);
  }

  public static BridgedDeviceBasicInformationClusterLeaveEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    return new BridgedDeviceBasicInformationClusterLeaveEvent(
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("BridgedDeviceBasicInformationClusterLeaveEvent {\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class BridgedDeviceBasicInformationClusterReachableChangedEvent {
  public Boolean reachableNewValue;
  private static final long REACHABLE_NEW_VALUE_ID = 0L;

  public BridgedDeviceBasicInformationClusterReachableChangedEvent(
    Boolean reachableNewValue
  ) {
    this.reachableNewValue = reachableNewValue;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(REACHABLE_NEW_VALUE_ID, new BooleanType(reachableNewValue)));

    return new StructType(values);
  }

  public static BridgedDeviceBasicInformationClusterReachableChangedEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Boolean reachableNewValue = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == REACHABLE_NEW_VALUE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          reachableNewValue = castingValue.value(Boolean.class);
        }
      }
    }
    return new BridgedDeviceBasicInformationClusterReachableChangedEvent(
      reachableNewValue
    );
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
}
public static class BridgedDeviceBasicInformationClusterActiveChangedEvent {
  public Long promisedActiveDuration;
  private static final long PROMISED_ACTIVE_DURATION_ID = 0L;

  public BridgedDeviceBasicInformationClusterActiveChangedEvent(
    Long promisedActiveDuration
  ) {
    this.promisedActiveDuration = promisedActiveDuration;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(PROMISED_ACTIVE_DURATION_ID, new UIntType(promisedActiveDuration)));

    return new StructType(values);
  }

  public static BridgedDeviceBasicInformationClusterActiveChangedEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long promisedActiveDuration = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == PROMISED_ACTIVE_DURATION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          promisedActiveDuration = castingValue.value(Long.class);
        }
      }
    }
    return new BridgedDeviceBasicInformationClusterActiveChangedEvent(
      promisedActiveDuration
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("BridgedDeviceBasicInformationClusterActiveChangedEvent {\n");
    output.append("\tpromisedActiveDuration: ");
    output.append(promisedActiveDuration);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class SwitchClusterSwitchLatchedEvent {
  public Integer newPosition;
  private static final long NEW_POSITION_ID = 0L;

  public SwitchClusterSwitchLatchedEvent(
    Integer newPosition
  ) {
    this.newPosition = newPosition;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(NEW_POSITION_ID, new UIntType(newPosition)));

    return new StructType(values);
  }

  public static SwitchClusterSwitchLatchedEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer newPosition = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == NEW_POSITION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          newPosition = castingValue.value(Integer.class);
        }
      }
    }
    return new SwitchClusterSwitchLatchedEvent(
      newPosition
    );
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
}
public static class SwitchClusterInitialPressEvent {
  public Integer newPosition;
  private static final long NEW_POSITION_ID = 0L;

  public SwitchClusterInitialPressEvent(
    Integer newPosition
  ) {
    this.newPosition = newPosition;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(NEW_POSITION_ID, new UIntType(newPosition)));

    return new StructType(values);
  }

  public static SwitchClusterInitialPressEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer newPosition = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == NEW_POSITION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          newPosition = castingValue.value(Integer.class);
        }
      }
    }
    return new SwitchClusterInitialPressEvent(
      newPosition
    );
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
}
public static class SwitchClusterLongPressEvent {
  public Integer newPosition;
  private static final long NEW_POSITION_ID = 0L;

  public SwitchClusterLongPressEvent(
    Integer newPosition
  ) {
    this.newPosition = newPosition;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(NEW_POSITION_ID, new UIntType(newPosition)));

    return new StructType(values);
  }

  public static SwitchClusterLongPressEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer newPosition = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == NEW_POSITION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          newPosition = castingValue.value(Integer.class);
        }
      }
    }
    return new SwitchClusterLongPressEvent(
      newPosition
    );
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
}
public static class SwitchClusterShortReleaseEvent {
  public Integer previousPosition;
  private static final long PREVIOUS_POSITION_ID = 0L;

  public SwitchClusterShortReleaseEvent(
    Integer previousPosition
  ) {
    this.previousPosition = previousPosition;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(PREVIOUS_POSITION_ID, new UIntType(previousPosition)));

    return new StructType(values);
  }

  public static SwitchClusterShortReleaseEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer previousPosition = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == PREVIOUS_POSITION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          previousPosition = castingValue.value(Integer.class);
        }
      }
    }
    return new SwitchClusterShortReleaseEvent(
      previousPosition
    );
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
}
public static class SwitchClusterLongReleaseEvent {
  public Integer previousPosition;
  private static final long PREVIOUS_POSITION_ID = 0L;

  public SwitchClusterLongReleaseEvent(
    Integer previousPosition
  ) {
    this.previousPosition = previousPosition;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(PREVIOUS_POSITION_ID, new UIntType(previousPosition)));

    return new StructType(values);
  }

  public static SwitchClusterLongReleaseEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer previousPosition = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == PREVIOUS_POSITION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          previousPosition = castingValue.value(Integer.class);
        }
      }
    }
    return new SwitchClusterLongReleaseEvent(
      previousPosition
    );
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
}
public static class SwitchClusterMultiPressOngoingEvent {
  public Integer newPosition;
  public Integer currentNumberOfPressesCounted;
  private static final long NEW_POSITION_ID = 0L;
  private static final long CURRENT_NUMBER_OF_PRESSES_COUNTED_ID = 1L;

  public SwitchClusterMultiPressOngoingEvent(
    Integer newPosition,
    Integer currentNumberOfPressesCounted
  ) {
    this.newPosition = newPosition;
    this.currentNumberOfPressesCounted = currentNumberOfPressesCounted;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(NEW_POSITION_ID, new UIntType(newPosition)));
    values.add(new StructElement(CURRENT_NUMBER_OF_PRESSES_COUNTED_ID, new UIntType(currentNumberOfPressesCounted)));

    return new StructType(values);
  }

  public static SwitchClusterMultiPressOngoingEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer newPosition = null;
    Integer currentNumberOfPressesCounted = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == NEW_POSITION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          newPosition = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == CURRENT_NUMBER_OF_PRESSES_COUNTED_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          currentNumberOfPressesCounted = castingValue.value(Integer.class);
        }
      }
    }
    return new SwitchClusterMultiPressOngoingEvent(
      newPosition,
      currentNumberOfPressesCounted
    );
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
}
public static class SwitchClusterMultiPressCompleteEvent {
  public Integer previousPosition;
  public Integer totalNumberOfPressesCounted;
  private static final long PREVIOUS_POSITION_ID = 0L;
  private static final long TOTAL_NUMBER_OF_PRESSES_COUNTED_ID = 1L;

  public SwitchClusterMultiPressCompleteEvent(
    Integer previousPosition,
    Integer totalNumberOfPressesCounted
  ) {
    this.previousPosition = previousPosition;
    this.totalNumberOfPressesCounted = totalNumberOfPressesCounted;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(PREVIOUS_POSITION_ID, new UIntType(previousPosition)));
    values.add(new StructElement(TOTAL_NUMBER_OF_PRESSES_COUNTED_ID, new UIntType(totalNumberOfPressesCounted)));

    return new StructType(values);
  }

  public static SwitchClusterMultiPressCompleteEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer previousPosition = null;
    Integer totalNumberOfPressesCounted = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == PREVIOUS_POSITION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          previousPosition = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == TOTAL_NUMBER_OF_PRESSES_COUNTED_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          totalNumberOfPressesCounted = castingValue.value(Integer.class);
        }
      }
    }
    return new SwitchClusterMultiPressCompleteEvent(
      previousPosition,
      totalNumberOfPressesCounted
    );
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
}
public static class BooleanStateClusterStateChangeEvent {
  public Boolean stateValue;
  private static final long STATE_VALUE_ID = 0L;

  public BooleanStateClusterStateChangeEvent(
    Boolean stateValue
  ) {
    this.stateValue = stateValue;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(STATE_VALUE_ID, new BooleanType(stateValue)));

    return new StructType(values);
  }

  public static BooleanStateClusterStateChangeEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Boolean stateValue = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == STATE_VALUE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          stateValue = castingValue.value(Boolean.class);
        }
      }
    }
    return new BooleanStateClusterStateChangeEvent(
      stateValue
    );
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
}
public static class OvenCavityOperationalStateClusterOperationalErrorEvent {
  public ChipStructs.OvenCavityOperationalStateClusterErrorStateStruct errorState;
  private static final long ERROR_STATE_ID = 0L;

  public OvenCavityOperationalStateClusterOperationalErrorEvent(
    ChipStructs.OvenCavityOperationalStateClusterErrorStateStruct errorState
  ) {
    this.errorState = errorState;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ERROR_STATE_ID, errorState.encodeTlv()));

    return new StructType(values);
  }

  public static OvenCavityOperationalStateClusterOperationalErrorEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    ChipStructs.OvenCavityOperationalStateClusterErrorStateStruct errorState = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ERROR_STATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          errorState = ChipStructs.OvenCavityOperationalStateClusterErrorStateStruct.decodeTlv(castingValue);
        }
      }
    }
    return new OvenCavityOperationalStateClusterOperationalErrorEvent(
      errorState
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("OvenCavityOperationalStateClusterOperationalErrorEvent {\n");
    output.append("\terrorState: ");
    output.append(errorState);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class OvenCavityOperationalStateClusterOperationCompletionEvent {
  public Integer completionErrorCode;
  public @Nullable Optional<Long> totalOperationalTime;
  public @Nullable Optional<Long> pausedTime;
  private static final long COMPLETION_ERROR_CODE_ID = 0L;
  private static final long TOTAL_OPERATIONAL_TIME_ID = 1L;
  private static final long PAUSED_TIME_ID = 2L;

  public OvenCavityOperationalStateClusterOperationCompletionEvent(
    Integer completionErrorCode,
    @Nullable Optional<Long> totalOperationalTime,
    @Nullable Optional<Long> pausedTime
  ) {
    this.completionErrorCode = completionErrorCode;
    this.totalOperationalTime = totalOperationalTime;
    this.pausedTime = pausedTime;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(COMPLETION_ERROR_CODE_ID, new UIntType(completionErrorCode)));
    values.add(new StructElement(TOTAL_OPERATIONAL_TIME_ID, totalOperationalTime != null ? totalOperationalTime.<BaseTLVType>map((nonOptionaltotalOperationalTime) -> new UIntType(nonOptionaltotalOperationalTime)).orElse(new EmptyType()) : new NullType()));
    values.add(new StructElement(PAUSED_TIME_ID, pausedTime != null ? pausedTime.<BaseTLVType>map((nonOptionalpausedTime) -> new UIntType(nonOptionalpausedTime)).orElse(new EmptyType()) : new NullType()));

    return new StructType(values);
  }

  public static OvenCavityOperationalStateClusterOperationCompletionEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer completionErrorCode = null;
    @Nullable Optional<Long> totalOperationalTime = null;
    @Nullable Optional<Long> pausedTime = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == COMPLETION_ERROR_CODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          completionErrorCode = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == TOTAL_OPERATIONAL_TIME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          totalOperationalTime = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == PAUSED_TIME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          pausedTime = Optional.of(castingValue.value(Long.class));
        }
      }
    }
    return new OvenCavityOperationalStateClusterOperationCompletionEvent(
      completionErrorCode,
      totalOperationalTime,
      pausedTime
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("OvenCavityOperationalStateClusterOperationCompletionEvent {\n");
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
}
public static class RefrigeratorAlarmClusterNotifyEvent {
  public Long active;
  public Long inactive;
  public Long state;
  public Long mask;
  private static final long ACTIVE_ID = 0L;
  private static final long INACTIVE_ID = 1L;
  private static final long STATE_ID = 2L;
  private static final long MASK_ID = 3L;

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

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ACTIVE_ID, new UIntType(active)));
    values.add(new StructElement(INACTIVE_ID, new UIntType(inactive)));
    values.add(new StructElement(STATE_ID, new UIntType(state)));
    values.add(new StructElement(MASK_ID, new UIntType(mask)));

    return new StructType(values);
  }

  public static RefrigeratorAlarmClusterNotifyEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long active = null;
    Long inactive = null;
    Long state = null;
    Long mask = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ACTIVE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          active = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == INACTIVE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          inactive = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == STATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          state = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == MASK_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          mask = castingValue.value(Long.class);
        }
      }
    }
    return new RefrigeratorAlarmClusterNotifyEvent(
      active,
      inactive,
      state,
      mask
    );
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
}
public static class SmokeCoAlarmClusterSmokeAlarmEvent {
  public Integer alarmSeverityLevel;
  private static final long ALARM_SEVERITY_LEVEL_ID = 0L;

  public SmokeCoAlarmClusterSmokeAlarmEvent(
    Integer alarmSeverityLevel
  ) {
    this.alarmSeverityLevel = alarmSeverityLevel;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ALARM_SEVERITY_LEVEL_ID, new UIntType(alarmSeverityLevel)));

    return new StructType(values);
  }

  public static SmokeCoAlarmClusterSmokeAlarmEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer alarmSeverityLevel = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ALARM_SEVERITY_LEVEL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          alarmSeverityLevel = castingValue.value(Integer.class);
        }
      }
    }
    return new SmokeCoAlarmClusterSmokeAlarmEvent(
      alarmSeverityLevel
    );
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
}
public static class SmokeCoAlarmClusterCOAlarmEvent {
  public Integer alarmSeverityLevel;
  private static final long ALARM_SEVERITY_LEVEL_ID = 0L;

  public SmokeCoAlarmClusterCOAlarmEvent(
    Integer alarmSeverityLevel
  ) {
    this.alarmSeverityLevel = alarmSeverityLevel;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ALARM_SEVERITY_LEVEL_ID, new UIntType(alarmSeverityLevel)));

    return new StructType(values);
  }

  public static SmokeCoAlarmClusterCOAlarmEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer alarmSeverityLevel = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ALARM_SEVERITY_LEVEL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          alarmSeverityLevel = castingValue.value(Integer.class);
        }
      }
    }
    return new SmokeCoAlarmClusterCOAlarmEvent(
      alarmSeverityLevel
    );
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
}
public static class SmokeCoAlarmClusterLowBatteryEvent {
  public Integer alarmSeverityLevel;
  private static final long ALARM_SEVERITY_LEVEL_ID = 0L;

  public SmokeCoAlarmClusterLowBatteryEvent(
    Integer alarmSeverityLevel
  ) {
    this.alarmSeverityLevel = alarmSeverityLevel;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ALARM_SEVERITY_LEVEL_ID, new UIntType(alarmSeverityLevel)));

    return new StructType(values);
  }

  public static SmokeCoAlarmClusterLowBatteryEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer alarmSeverityLevel = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ALARM_SEVERITY_LEVEL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          alarmSeverityLevel = castingValue.value(Integer.class);
        }
      }
    }
    return new SmokeCoAlarmClusterLowBatteryEvent(
      alarmSeverityLevel
    );
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
}
public static class SmokeCoAlarmClusterHardwareFaultEvent {

  public SmokeCoAlarmClusterHardwareFaultEvent(
  ) {
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();

    return new StructType(values);
  }

  public static SmokeCoAlarmClusterHardwareFaultEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    return new SmokeCoAlarmClusterHardwareFaultEvent(
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("SmokeCoAlarmClusterHardwareFaultEvent {\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class SmokeCoAlarmClusterEndOfServiceEvent {

  public SmokeCoAlarmClusterEndOfServiceEvent(
  ) {
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();

    return new StructType(values);
  }

  public static SmokeCoAlarmClusterEndOfServiceEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    return new SmokeCoAlarmClusterEndOfServiceEvent(
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("SmokeCoAlarmClusterEndOfServiceEvent {\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class SmokeCoAlarmClusterSelfTestCompleteEvent {

  public SmokeCoAlarmClusterSelfTestCompleteEvent(
  ) {
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();

    return new StructType(values);
  }

  public static SmokeCoAlarmClusterSelfTestCompleteEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    return new SmokeCoAlarmClusterSelfTestCompleteEvent(
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("SmokeCoAlarmClusterSelfTestCompleteEvent {\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class SmokeCoAlarmClusterAlarmMutedEvent {

  public SmokeCoAlarmClusterAlarmMutedEvent(
  ) {
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();

    return new StructType(values);
  }

  public static SmokeCoAlarmClusterAlarmMutedEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    return new SmokeCoAlarmClusterAlarmMutedEvent(
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("SmokeCoAlarmClusterAlarmMutedEvent {\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class SmokeCoAlarmClusterMuteEndedEvent {

  public SmokeCoAlarmClusterMuteEndedEvent(
  ) {
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();

    return new StructType(values);
  }

  public static SmokeCoAlarmClusterMuteEndedEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    return new SmokeCoAlarmClusterMuteEndedEvent(
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("SmokeCoAlarmClusterMuteEndedEvent {\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class SmokeCoAlarmClusterInterconnectSmokeAlarmEvent {
  public Integer alarmSeverityLevel;
  private static final long ALARM_SEVERITY_LEVEL_ID = 0L;

  public SmokeCoAlarmClusterInterconnectSmokeAlarmEvent(
    Integer alarmSeverityLevel
  ) {
    this.alarmSeverityLevel = alarmSeverityLevel;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ALARM_SEVERITY_LEVEL_ID, new UIntType(alarmSeverityLevel)));

    return new StructType(values);
  }

  public static SmokeCoAlarmClusterInterconnectSmokeAlarmEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer alarmSeverityLevel = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ALARM_SEVERITY_LEVEL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          alarmSeverityLevel = castingValue.value(Integer.class);
        }
      }
    }
    return new SmokeCoAlarmClusterInterconnectSmokeAlarmEvent(
      alarmSeverityLevel
    );
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
}
public static class SmokeCoAlarmClusterInterconnectCOAlarmEvent {
  public Integer alarmSeverityLevel;
  private static final long ALARM_SEVERITY_LEVEL_ID = 0L;

  public SmokeCoAlarmClusterInterconnectCOAlarmEvent(
    Integer alarmSeverityLevel
  ) {
    this.alarmSeverityLevel = alarmSeverityLevel;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ALARM_SEVERITY_LEVEL_ID, new UIntType(alarmSeverityLevel)));

    return new StructType(values);
  }

  public static SmokeCoAlarmClusterInterconnectCOAlarmEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer alarmSeverityLevel = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ALARM_SEVERITY_LEVEL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          alarmSeverityLevel = castingValue.value(Integer.class);
        }
      }
    }
    return new SmokeCoAlarmClusterInterconnectCOAlarmEvent(
      alarmSeverityLevel
    );
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
}
public static class SmokeCoAlarmClusterAllClearEvent {

  public SmokeCoAlarmClusterAllClearEvent(
  ) {
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();

    return new StructType(values);
  }

  public static SmokeCoAlarmClusterAllClearEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    return new SmokeCoAlarmClusterAllClearEvent(
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("SmokeCoAlarmClusterAllClearEvent {\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class DishwasherAlarmClusterNotifyEvent {
  public Long active;
  public Long inactive;
  public Long state;
  public Long mask;
  private static final long ACTIVE_ID = 0L;
  private static final long INACTIVE_ID = 1L;
  private static final long STATE_ID = 2L;
  private static final long MASK_ID = 3L;

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

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ACTIVE_ID, new UIntType(active)));
    values.add(new StructElement(INACTIVE_ID, new UIntType(inactive)));
    values.add(new StructElement(STATE_ID, new UIntType(state)));
    values.add(new StructElement(MASK_ID, new UIntType(mask)));

    return new StructType(values);
  }

  public static DishwasherAlarmClusterNotifyEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long active = null;
    Long inactive = null;
    Long state = null;
    Long mask = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ACTIVE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          active = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == INACTIVE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          inactive = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == STATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          state = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == MASK_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          mask = castingValue.value(Long.class);
        }
      }
    }
    return new DishwasherAlarmClusterNotifyEvent(
      active,
      inactive,
      state,
      mask
    );
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
}
public static class OperationalStateClusterOperationalErrorEvent {
  public ChipStructs.OperationalStateClusterErrorStateStruct errorState;
  private static final long ERROR_STATE_ID = 0L;

  public OperationalStateClusterOperationalErrorEvent(
    ChipStructs.OperationalStateClusterErrorStateStruct errorState
  ) {
    this.errorState = errorState;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ERROR_STATE_ID, errorState.encodeTlv()));

    return new StructType(values);
  }

  public static OperationalStateClusterOperationalErrorEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    ChipStructs.OperationalStateClusterErrorStateStruct errorState = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ERROR_STATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          errorState = ChipStructs.OperationalStateClusterErrorStateStruct.decodeTlv(castingValue);
        }
      }
    }
    return new OperationalStateClusterOperationalErrorEvent(
      errorState
    );
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
}
public static class OperationalStateClusterOperationCompletionEvent {
  public Integer completionErrorCode;
  public @Nullable Optional<Long> totalOperationalTime;
  public @Nullable Optional<Long> pausedTime;
  private static final long COMPLETION_ERROR_CODE_ID = 0L;
  private static final long TOTAL_OPERATIONAL_TIME_ID = 1L;
  private static final long PAUSED_TIME_ID = 2L;

  public OperationalStateClusterOperationCompletionEvent(
    Integer completionErrorCode,
    @Nullable Optional<Long> totalOperationalTime,
    @Nullable Optional<Long> pausedTime
  ) {
    this.completionErrorCode = completionErrorCode;
    this.totalOperationalTime = totalOperationalTime;
    this.pausedTime = pausedTime;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(COMPLETION_ERROR_CODE_ID, new UIntType(completionErrorCode)));
    values.add(new StructElement(TOTAL_OPERATIONAL_TIME_ID, totalOperationalTime != null ? totalOperationalTime.<BaseTLVType>map((nonOptionaltotalOperationalTime) -> new UIntType(nonOptionaltotalOperationalTime)).orElse(new EmptyType()) : new NullType()));
    values.add(new StructElement(PAUSED_TIME_ID, pausedTime != null ? pausedTime.<BaseTLVType>map((nonOptionalpausedTime) -> new UIntType(nonOptionalpausedTime)).orElse(new EmptyType()) : new NullType()));

    return new StructType(values);
  }

  public static OperationalStateClusterOperationCompletionEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer completionErrorCode = null;
    @Nullable Optional<Long> totalOperationalTime = null;
    @Nullable Optional<Long> pausedTime = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == COMPLETION_ERROR_CODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          completionErrorCode = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == TOTAL_OPERATIONAL_TIME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          totalOperationalTime = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == PAUSED_TIME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          pausedTime = Optional.of(castingValue.value(Long.class));
        }
      }
    }
    return new OperationalStateClusterOperationCompletionEvent(
      completionErrorCode,
      totalOperationalTime,
      pausedTime
    );
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
}
public static class RvcOperationalStateClusterOperationalErrorEvent {
  public ChipStructs.RvcOperationalStateClusterErrorStateStruct errorState;
  private static final long ERROR_STATE_ID = 0L;

  public RvcOperationalStateClusterOperationalErrorEvent(
    ChipStructs.RvcOperationalStateClusterErrorStateStruct errorState
  ) {
    this.errorState = errorState;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ERROR_STATE_ID, errorState.encodeTlv()));

    return new StructType(values);
  }

  public static RvcOperationalStateClusterOperationalErrorEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    ChipStructs.RvcOperationalStateClusterErrorStateStruct errorState = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ERROR_STATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          errorState = ChipStructs.RvcOperationalStateClusterErrorStateStruct.decodeTlv(castingValue);
        }
      }
    }
    return new RvcOperationalStateClusterOperationalErrorEvent(
      errorState
    );
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
}
public static class RvcOperationalStateClusterOperationCompletionEvent {
  public Integer completionErrorCode;
  public @Nullable Optional<Long> totalOperationalTime;
  public @Nullable Optional<Long> pausedTime;
  private static final long COMPLETION_ERROR_CODE_ID = 0L;
  private static final long TOTAL_OPERATIONAL_TIME_ID = 1L;
  private static final long PAUSED_TIME_ID = 2L;

  public RvcOperationalStateClusterOperationCompletionEvent(
    Integer completionErrorCode,
    @Nullable Optional<Long> totalOperationalTime,
    @Nullable Optional<Long> pausedTime
  ) {
    this.completionErrorCode = completionErrorCode;
    this.totalOperationalTime = totalOperationalTime;
    this.pausedTime = pausedTime;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(COMPLETION_ERROR_CODE_ID, new UIntType(completionErrorCode)));
    values.add(new StructElement(TOTAL_OPERATIONAL_TIME_ID, totalOperationalTime != null ? totalOperationalTime.<BaseTLVType>map((nonOptionaltotalOperationalTime) -> new UIntType(nonOptionaltotalOperationalTime)).orElse(new EmptyType()) : new NullType()));
    values.add(new StructElement(PAUSED_TIME_ID, pausedTime != null ? pausedTime.<BaseTLVType>map((nonOptionalpausedTime) -> new UIntType(nonOptionalpausedTime)).orElse(new EmptyType()) : new NullType()));

    return new StructType(values);
  }

  public static RvcOperationalStateClusterOperationCompletionEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer completionErrorCode = null;
    @Nullable Optional<Long> totalOperationalTime = null;
    @Nullable Optional<Long> pausedTime = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == COMPLETION_ERROR_CODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          completionErrorCode = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == TOTAL_OPERATIONAL_TIME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          totalOperationalTime = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == PAUSED_TIME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          pausedTime = Optional.of(castingValue.value(Long.class));
        }
      }
    }
    return new RvcOperationalStateClusterOperationCompletionEvent(
      completionErrorCode,
      totalOperationalTime,
      pausedTime
    );
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
}
public static class BooleanStateConfigurationClusterAlarmsStateChangedEvent {
  public Integer alarmsActive;
  public Optional<Integer> alarmsSuppressed;
  private static final long ALARMS_ACTIVE_ID = 0L;
  private static final long ALARMS_SUPPRESSED_ID = 1L;

  public BooleanStateConfigurationClusterAlarmsStateChangedEvent(
    Integer alarmsActive,
    Optional<Integer> alarmsSuppressed
  ) {
    this.alarmsActive = alarmsActive;
    this.alarmsSuppressed = alarmsSuppressed;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ALARMS_ACTIVE_ID, new UIntType(alarmsActive)));
    values.add(new StructElement(ALARMS_SUPPRESSED_ID, alarmsSuppressed.<BaseTLVType>map((nonOptionalalarmsSuppressed) -> new UIntType(nonOptionalalarmsSuppressed)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static BooleanStateConfigurationClusterAlarmsStateChangedEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer alarmsActive = null;
    Optional<Integer> alarmsSuppressed = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ALARMS_ACTIVE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          alarmsActive = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == ALARMS_SUPPRESSED_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          alarmsSuppressed = Optional.of(castingValue.value(Integer.class));
        }
      }
    }
    return new BooleanStateConfigurationClusterAlarmsStateChangedEvent(
      alarmsActive,
      alarmsSuppressed
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("BooleanStateConfigurationClusterAlarmsStateChangedEvent {\n");
    output.append("\talarmsActive: ");
    output.append(alarmsActive);
    output.append("\n");
    output.append("\talarmsSuppressed: ");
    output.append(alarmsSuppressed);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class BooleanStateConfigurationClusterSensorFaultEvent {
  public Integer sensorFault;
  private static final long SENSOR_FAULT_ID = 0L;

  public BooleanStateConfigurationClusterSensorFaultEvent(
    Integer sensorFault
  ) {
    this.sensorFault = sensorFault;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(SENSOR_FAULT_ID, new UIntType(sensorFault)));

    return new StructType(values);
  }

  public static BooleanStateConfigurationClusterSensorFaultEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer sensorFault = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == SENSOR_FAULT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          sensorFault = castingValue.value(Integer.class);
        }
      }
    }
    return new BooleanStateConfigurationClusterSensorFaultEvent(
      sensorFault
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("BooleanStateConfigurationClusterSensorFaultEvent {\n");
    output.append("\tsensorFault: ");
    output.append(sensorFault);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ValveConfigurationAndControlClusterValveStateChangedEvent {
  public Integer valveState;
  public Optional<Integer> valveLevel;
  private static final long VALVE_STATE_ID = 0L;
  private static final long VALVE_LEVEL_ID = 1L;

  public ValveConfigurationAndControlClusterValveStateChangedEvent(
    Integer valveState,
    Optional<Integer> valveLevel
  ) {
    this.valveState = valveState;
    this.valveLevel = valveLevel;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(VALVE_STATE_ID, new UIntType(valveState)));
    values.add(new StructElement(VALVE_LEVEL_ID, valveLevel.<BaseTLVType>map((nonOptionalvalveLevel) -> new UIntType(nonOptionalvalveLevel)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static ValveConfigurationAndControlClusterValveStateChangedEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer valveState = null;
    Optional<Integer> valveLevel = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == VALVE_STATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          valveState = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == VALVE_LEVEL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          valveLevel = Optional.of(castingValue.value(Integer.class));
        }
      }
    }
    return new ValveConfigurationAndControlClusterValveStateChangedEvent(
      valveState,
      valveLevel
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ValveConfigurationAndControlClusterValveStateChangedEvent {\n");
    output.append("\tvalveState: ");
    output.append(valveState);
    output.append("\n");
    output.append("\tvalveLevel: ");
    output.append(valveLevel);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ValveConfigurationAndControlClusterValveFaultEvent {
  public Integer valveFault;
  private static final long VALVE_FAULT_ID = 0L;

  public ValveConfigurationAndControlClusterValveFaultEvent(
    Integer valveFault
  ) {
    this.valveFault = valveFault;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(VALVE_FAULT_ID, new UIntType(valveFault)));

    return new StructType(values);
  }

  public static ValveConfigurationAndControlClusterValveFaultEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer valveFault = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == VALVE_FAULT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          valveFault = castingValue.value(Integer.class);
        }
      }
    }
    return new ValveConfigurationAndControlClusterValveFaultEvent(
      valveFault
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ValveConfigurationAndControlClusterValveFaultEvent {\n");
    output.append("\tvalveFault: ");
    output.append(valveFault);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ElectricalPowerMeasurementClusterMeasurementPeriodRangesEvent {
  public ArrayList<ChipStructs.ElectricalPowerMeasurementClusterMeasurementRangeStruct> ranges;
  private static final long RANGES_ID = 0L;

  public ElectricalPowerMeasurementClusterMeasurementPeriodRangesEvent(
    ArrayList<ChipStructs.ElectricalPowerMeasurementClusterMeasurementRangeStruct> ranges
  ) {
    this.ranges = ranges;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(RANGES_ID, ArrayType.generateArrayType(ranges, (elementranges) -> elementranges.encodeTlv())));

    return new StructType(values);
  }

  public static ElectricalPowerMeasurementClusterMeasurementPeriodRangesEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    ArrayList<ChipStructs.ElectricalPowerMeasurementClusterMeasurementRangeStruct> ranges = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == RANGES_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          ranges = castingValue.map((elementcastingValue) -> ChipStructs.ElectricalPowerMeasurementClusterMeasurementRangeStruct.decodeTlv(elementcastingValue));
        }
      }
    }
    return new ElectricalPowerMeasurementClusterMeasurementPeriodRangesEvent(
      ranges
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ElectricalPowerMeasurementClusterMeasurementPeriodRangesEvent {\n");
    output.append("\tranges: ");
    output.append(ranges);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ElectricalEnergyMeasurementClusterCumulativeEnergyMeasuredEvent {
  public Optional<ChipStructs.ElectricalEnergyMeasurementClusterEnergyMeasurementStruct> energyImported;
  public Optional<ChipStructs.ElectricalEnergyMeasurementClusterEnergyMeasurementStruct> energyExported;
  private static final long ENERGY_IMPORTED_ID = 0L;
  private static final long ENERGY_EXPORTED_ID = 1L;

  public ElectricalEnergyMeasurementClusterCumulativeEnergyMeasuredEvent(
    Optional<ChipStructs.ElectricalEnergyMeasurementClusterEnergyMeasurementStruct> energyImported,
    Optional<ChipStructs.ElectricalEnergyMeasurementClusterEnergyMeasurementStruct> energyExported
  ) {
    this.energyImported = energyImported;
    this.energyExported = energyExported;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ENERGY_IMPORTED_ID, energyImported.<BaseTLVType>map((nonOptionalenergyImported) -> nonOptionalenergyImported.encodeTlv()).orElse(new EmptyType())));
    values.add(new StructElement(ENERGY_EXPORTED_ID, energyExported.<BaseTLVType>map((nonOptionalenergyExported) -> nonOptionalenergyExported.encodeTlv()).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static ElectricalEnergyMeasurementClusterCumulativeEnergyMeasuredEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Optional<ChipStructs.ElectricalEnergyMeasurementClusterEnergyMeasurementStruct> energyImported = Optional.empty();
    Optional<ChipStructs.ElectricalEnergyMeasurementClusterEnergyMeasurementStruct> energyExported = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ENERGY_IMPORTED_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          energyImported = Optional.of(ChipStructs.ElectricalEnergyMeasurementClusterEnergyMeasurementStruct.decodeTlv(castingValue));
        }
      } else if (element.contextTagNum() == ENERGY_EXPORTED_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          energyExported = Optional.of(ChipStructs.ElectricalEnergyMeasurementClusterEnergyMeasurementStruct.decodeTlv(castingValue));
        }
      }
    }
    return new ElectricalEnergyMeasurementClusterCumulativeEnergyMeasuredEvent(
      energyImported,
      energyExported
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ElectricalEnergyMeasurementClusterCumulativeEnergyMeasuredEvent {\n");
    output.append("\tenergyImported: ");
    output.append(energyImported);
    output.append("\n");
    output.append("\tenergyExported: ");
    output.append(energyExported);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ElectricalEnergyMeasurementClusterPeriodicEnergyMeasuredEvent {
  public Optional<ChipStructs.ElectricalEnergyMeasurementClusterEnergyMeasurementStruct> energyImported;
  public Optional<ChipStructs.ElectricalEnergyMeasurementClusterEnergyMeasurementStruct> energyExported;
  private static final long ENERGY_IMPORTED_ID = 0L;
  private static final long ENERGY_EXPORTED_ID = 1L;

  public ElectricalEnergyMeasurementClusterPeriodicEnergyMeasuredEvent(
    Optional<ChipStructs.ElectricalEnergyMeasurementClusterEnergyMeasurementStruct> energyImported,
    Optional<ChipStructs.ElectricalEnergyMeasurementClusterEnergyMeasurementStruct> energyExported
  ) {
    this.energyImported = energyImported;
    this.energyExported = energyExported;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ENERGY_IMPORTED_ID, energyImported.<BaseTLVType>map((nonOptionalenergyImported) -> nonOptionalenergyImported.encodeTlv()).orElse(new EmptyType())));
    values.add(new StructElement(ENERGY_EXPORTED_ID, energyExported.<BaseTLVType>map((nonOptionalenergyExported) -> nonOptionalenergyExported.encodeTlv()).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static ElectricalEnergyMeasurementClusterPeriodicEnergyMeasuredEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Optional<ChipStructs.ElectricalEnergyMeasurementClusterEnergyMeasurementStruct> energyImported = Optional.empty();
    Optional<ChipStructs.ElectricalEnergyMeasurementClusterEnergyMeasurementStruct> energyExported = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ENERGY_IMPORTED_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          energyImported = Optional.of(ChipStructs.ElectricalEnergyMeasurementClusterEnergyMeasurementStruct.decodeTlv(castingValue));
        }
      } else if (element.contextTagNum() == ENERGY_EXPORTED_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          energyExported = Optional.of(ChipStructs.ElectricalEnergyMeasurementClusterEnergyMeasurementStruct.decodeTlv(castingValue));
        }
      }
    }
    return new ElectricalEnergyMeasurementClusterPeriodicEnergyMeasuredEvent(
      energyImported,
      energyExported
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ElectricalEnergyMeasurementClusterPeriodicEnergyMeasuredEvent {\n");
    output.append("\tenergyImported: ");
    output.append(energyImported);
    output.append("\n");
    output.append("\tenergyExported: ");
    output.append(energyExported);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class WaterHeaterManagementClusterBoostStartedEvent {
  public ChipStructs.WaterHeaterManagementClusterWaterHeaterBoostInfoStruct boostInfo;
  private static final long BOOST_INFO_ID = 0L;

  public WaterHeaterManagementClusterBoostStartedEvent(
    ChipStructs.WaterHeaterManagementClusterWaterHeaterBoostInfoStruct boostInfo
  ) {
    this.boostInfo = boostInfo;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(BOOST_INFO_ID, boostInfo.encodeTlv()));

    return new StructType(values);
  }

  public static WaterHeaterManagementClusterBoostStartedEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    ChipStructs.WaterHeaterManagementClusterWaterHeaterBoostInfoStruct boostInfo = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == BOOST_INFO_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          boostInfo = ChipStructs.WaterHeaterManagementClusterWaterHeaterBoostInfoStruct.decodeTlv(castingValue);
        }
      }
    }
    return new WaterHeaterManagementClusterBoostStartedEvent(
      boostInfo
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("WaterHeaterManagementClusterBoostStartedEvent {\n");
    output.append("\tboostInfo: ");
    output.append(boostInfo);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class WaterHeaterManagementClusterBoostEndedEvent {

  public WaterHeaterManagementClusterBoostEndedEvent(
  ) {
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();

    return new StructType(values);
  }

  public static WaterHeaterManagementClusterBoostEndedEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    return new WaterHeaterManagementClusterBoostEndedEvent(
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("WaterHeaterManagementClusterBoostEndedEvent {\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class DemandResponseLoadControlClusterLoadControlEventStatusChangeEvent {
  public byte[] eventID;
  public @Nullable Integer transitionIndex;
  public Integer status;
  public Integer criticality;
  public Integer control;
  public @Nullable Optional<ChipStructs.DemandResponseLoadControlClusterTemperatureControlStruct> temperatureControl;
  public @Nullable Optional<ChipStructs.DemandResponseLoadControlClusterAverageLoadControlStruct> averageLoadControl;
  public @Nullable Optional<ChipStructs.DemandResponseLoadControlClusterDutyCycleControlStruct> dutyCycleControl;
  public @Nullable Optional<ChipStructs.DemandResponseLoadControlClusterPowerSavingsControlStruct> powerSavingsControl;
  public @Nullable Optional<ChipStructs.DemandResponseLoadControlClusterHeatingSourceControlStruct> heatingSourceControl;
  private static final long EVENT_ID_ID = 0L;
  private static final long TRANSITION_INDEX_ID = 1L;
  private static final long STATUS_ID = 2L;
  private static final long CRITICALITY_ID = 3L;
  private static final long CONTROL_ID = 4L;
  private static final long TEMPERATURE_CONTROL_ID = 5L;
  private static final long AVERAGE_LOAD_CONTROL_ID = 6L;
  private static final long DUTY_CYCLE_CONTROL_ID = 7L;
  private static final long POWER_SAVINGS_CONTROL_ID = 8L;
  private static final long HEATING_SOURCE_CONTROL_ID = 9L;

  public DemandResponseLoadControlClusterLoadControlEventStatusChangeEvent(
    byte[] eventID,
    @Nullable Integer transitionIndex,
    Integer status,
    Integer criticality,
    Integer control,
    @Nullable Optional<ChipStructs.DemandResponseLoadControlClusterTemperatureControlStruct> temperatureControl,
    @Nullable Optional<ChipStructs.DemandResponseLoadControlClusterAverageLoadControlStruct> averageLoadControl,
    @Nullable Optional<ChipStructs.DemandResponseLoadControlClusterDutyCycleControlStruct> dutyCycleControl,
    @Nullable Optional<ChipStructs.DemandResponseLoadControlClusterPowerSavingsControlStruct> powerSavingsControl,
    @Nullable Optional<ChipStructs.DemandResponseLoadControlClusterHeatingSourceControlStruct> heatingSourceControl
  ) {
    this.eventID = eventID;
    this.transitionIndex = transitionIndex;
    this.status = status;
    this.criticality = criticality;
    this.control = control;
    this.temperatureControl = temperatureControl;
    this.averageLoadControl = averageLoadControl;
    this.dutyCycleControl = dutyCycleControl;
    this.powerSavingsControl = powerSavingsControl;
    this.heatingSourceControl = heatingSourceControl;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(EVENT_ID_ID, new ByteArrayType(eventID)));
    values.add(new StructElement(TRANSITION_INDEX_ID, transitionIndex != null ? new UIntType(transitionIndex) : new NullType()));
    values.add(new StructElement(STATUS_ID, new UIntType(status)));
    values.add(new StructElement(CRITICALITY_ID, new UIntType(criticality)));
    values.add(new StructElement(CONTROL_ID, new UIntType(control)));
    values.add(new StructElement(TEMPERATURE_CONTROL_ID, temperatureControl != null ? temperatureControl.<BaseTLVType>map((nonOptionaltemperatureControl) -> nonOptionaltemperatureControl.encodeTlv()).orElse(new EmptyType()) : new NullType()));
    values.add(new StructElement(AVERAGE_LOAD_CONTROL_ID, averageLoadControl != null ? averageLoadControl.<BaseTLVType>map((nonOptionalaverageLoadControl) -> nonOptionalaverageLoadControl.encodeTlv()).orElse(new EmptyType()) : new NullType()));
    values.add(new StructElement(DUTY_CYCLE_CONTROL_ID, dutyCycleControl != null ? dutyCycleControl.<BaseTLVType>map((nonOptionaldutyCycleControl) -> nonOptionaldutyCycleControl.encodeTlv()).orElse(new EmptyType()) : new NullType()));
    values.add(new StructElement(POWER_SAVINGS_CONTROL_ID, powerSavingsControl != null ? powerSavingsControl.<BaseTLVType>map((nonOptionalpowerSavingsControl) -> nonOptionalpowerSavingsControl.encodeTlv()).orElse(new EmptyType()) : new NullType()));
    values.add(new StructElement(HEATING_SOURCE_CONTROL_ID, heatingSourceControl != null ? heatingSourceControl.<BaseTLVType>map((nonOptionalheatingSourceControl) -> nonOptionalheatingSourceControl.encodeTlv()).orElse(new EmptyType()) : new NullType()));

    return new StructType(values);
  }

  public static DemandResponseLoadControlClusterLoadControlEventStatusChangeEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    byte[] eventID = null;
    @Nullable Integer transitionIndex = null;
    Integer status = null;
    Integer criticality = null;
    Integer control = null;
    @Nullable Optional<ChipStructs.DemandResponseLoadControlClusterTemperatureControlStruct> temperatureControl = null;
    @Nullable Optional<ChipStructs.DemandResponseLoadControlClusterAverageLoadControlStruct> averageLoadControl = null;
    @Nullable Optional<ChipStructs.DemandResponseLoadControlClusterDutyCycleControlStruct> dutyCycleControl = null;
    @Nullable Optional<ChipStructs.DemandResponseLoadControlClusterPowerSavingsControlStruct> powerSavingsControl = null;
    @Nullable Optional<ChipStructs.DemandResponseLoadControlClusterHeatingSourceControlStruct> heatingSourceControl = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == EVENT_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.ByteArray) {
          ByteArrayType castingValue = element.value(ByteArrayType.class);
          eventID = castingValue.value(byte[].class);
        }
      } else if (element.contextTagNum() == TRANSITION_INDEX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          transitionIndex = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == STATUS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          status = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == CRITICALITY_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          criticality = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == CONTROL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          control = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == TEMPERATURE_CONTROL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          temperatureControl = Optional.of(ChipStructs.DemandResponseLoadControlClusterTemperatureControlStruct.decodeTlv(castingValue));
        }
      } else if (element.contextTagNum() == AVERAGE_LOAD_CONTROL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          averageLoadControl = Optional.of(ChipStructs.DemandResponseLoadControlClusterAverageLoadControlStruct.decodeTlv(castingValue));
        }
      } else if (element.contextTagNum() == DUTY_CYCLE_CONTROL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          dutyCycleControl = Optional.of(ChipStructs.DemandResponseLoadControlClusterDutyCycleControlStruct.decodeTlv(castingValue));
        }
      } else if (element.contextTagNum() == POWER_SAVINGS_CONTROL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          powerSavingsControl = Optional.of(ChipStructs.DemandResponseLoadControlClusterPowerSavingsControlStruct.decodeTlv(castingValue));
        }
      } else if (element.contextTagNum() == HEATING_SOURCE_CONTROL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          heatingSourceControl = Optional.of(ChipStructs.DemandResponseLoadControlClusterHeatingSourceControlStruct.decodeTlv(castingValue));
        }
      }
    }
    return new DemandResponseLoadControlClusterLoadControlEventStatusChangeEvent(
      eventID,
      transitionIndex,
      status,
      criticality,
      control,
      temperatureControl,
      averageLoadControl,
      dutyCycleControl,
      powerSavingsControl,
      heatingSourceControl
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("DemandResponseLoadControlClusterLoadControlEventStatusChangeEvent {\n");
    output.append("\teventID: ");
    output.append(Arrays.toString(eventID));
    output.append("\n");
    output.append("\ttransitionIndex: ");
    output.append(transitionIndex);
    output.append("\n");
    output.append("\tstatus: ");
    output.append(status);
    output.append("\n");
    output.append("\tcriticality: ");
    output.append(criticality);
    output.append("\n");
    output.append("\tcontrol: ");
    output.append(control);
    output.append("\n");
    output.append("\ttemperatureControl: ");
    output.append(temperatureControl);
    output.append("\n");
    output.append("\taverageLoadControl: ");
    output.append(averageLoadControl);
    output.append("\n");
    output.append("\tdutyCycleControl: ");
    output.append(dutyCycleControl);
    output.append("\n");
    output.append("\tpowerSavingsControl: ");
    output.append(powerSavingsControl);
    output.append("\n");
    output.append("\theatingSourceControl: ");
    output.append(heatingSourceControl);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class MessagesClusterMessageQueuedEvent {
  public byte[] messageID;
  private static final long MESSAGE_ID_ID = 0L;

  public MessagesClusterMessageQueuedEvent(
    byte[] messageID
  ) {
    this.messageID = messageID;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(MESSAGE_ID_ID, new ByteArrayType(messageID)));

    return new StructType(values);
  }

  public static MessagesClusterMessageQueuedEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    byte[] messageID = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == MESSAGE_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.ByteArray) {
          ByteArrayType castingValue = element.value(ByteArrayType.class);
          messageID = castingValue.value(byte[].class);
        }
      }
    }
    return new MessagesClusterMessageQueuedEvent(
      messageID
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("MessagesClusterMessageQueuedEvent {\n");
    output.append("\tmessageID: ");
    output.append(Arrays.toString(messageID));
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class MessagesClusterMessagePresentedEvent {
  public byte[] messageID;
  private static final long MESSAGE_ID_ID = 0L;

  public MessagesClusterMessagePresentedEvent(
    byte[] messageID
  ) {
    this.messageID = messageID;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(MESSAGE_ID_ID, new ByteArrayType(messageID)));

    return new StructType(values);
  }

  public static MessagesClusterMessagePresentedEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    byte[] messageID = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == MESSAGE_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.ByteArray) {
          ByteArrayType castingValue = element.value(ByteArrayType.class);
          messageID = castingValue.value(byte[].class);
        }
      }
    }
    return new MessagesClusterMessagePresentedEvent(
      messageID
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("MessagesClusterMessagePresentedEvent {\n");
    output.append("\tmessageID: ");
    output.append(Arrays.toString(messageID));
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class MessagesClusterMessageCompleteEvent {
  public byte[] messageID;
  public @Nullable Optional<Long> responseID;
  public @Nullable Optional<String> reply;
  public @Nullable Integer futureMessagesPreference;
  private static final long MESSAGE_ID_ID = 0L;
  private static final long RESPONSE_ID_ID = 1L;
  private static final long REPLY_ID = 2L;
  private static final long FUTURE_MESSAGES_PREFERENCE_ID = 3L;

  public MessagesClusterMessageCompleteEvent(
    byte[] messageID,
    @Nullable Optional<Long> responseID,
    @Nullable Optional<String> reply,
    @Nullable Integer futureMessagesPreference
  ) {
    this.messageID = messageID;
    this.responseID = responseID;
    this.reply = reply;
    this.futureMessagesPreference = futureMessagesPreference;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(MESSAGE_ID_ID, new ByteArrayType(messageID)));
    values.add(new StructElement(RESPONSE_ID_ID, responseID != null ? responseID.<BaseTLVType>map((nonOptionalresponseID) -> new UIntType(nonOptionalresponseID)).orElse(new EmptyType()) : new NullType()));
    values.add(new StructElement(REPLY_ID, reply != null ? reply.<BaseTLVType>map((nonOptionalreply) -> new StringType(nonOptionalreply)).orElse(new EmptyType()) : new NullType()));
    values.add(new StructElement(FUTURE_MESSAGES_PREFERENCE_ID, futureMessagesPreference != null ? new UIntType(futureMessagesPreference) : new NullType()));

    return new StructType(values);
  }

  public static MessagesClusterMessageCompleteEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    byte[] messageID = null;
    @Nullable Optional<Long> responseID = null;
    @Nullable Optional<String> reply = null;
    @Nullable Integer futureMessagesPreference = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == MESSAGE_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.ByteArray) {
          ByteArrayType castingValue = element.value(ByteArrayType.class);
          messageID = castingValue.value(byte[].class);
        }
      } else if (element.contextTagNum() == RESPONSE_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          responseID = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == REPLY_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          reply = Optional.of(castingValue.value(String.class));
        }
      } else if (element.contextTagNum() == FUTURE_MESSAGES_PREFERENCE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          futureMessagesPreference = castingValue.value(Integer.class);
        }
      }
    }
    return new MessagesClusterMessageCompleteEvent(
      messageID,
      responseID,
      reply,
      futureMessagesPreference
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("MessagesClusterMessageCompleteEvent {\n");
    output.append("\tmessageID: ");
    output.append(Arrays.toString(messageID));
    output.append("\n");
    output.append("\tresponseID: ");
    output.append(responseID);
    output.append("\n");
    output.append("\treply: ");
    output.append(reply);
    output.append("\n");
    output.append("\tfutureMessagesPreference: ");
    output.append(futureMessagesPreference);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class DeviceEnergyManagementClusterPowerAdjustStartEvent {

  public DeviceEnergyManagementClusterPowerAdjustStartEvent(
  ) {
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();

    return new StructType(values);
  }

  public static DeviceEnergyManagementClusterPowerAdjustStartEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    return new DeviceEnergyManagementClusterPowerAdjustStartEvent(
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("DeviceEnergyManagementClusterPowerAdjustStartEvent {\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class DeviceEnergyManagementClusterPowerAdjustEndEvent {
  public Integer cause;
  public Long duration;
  public Long energyUse;
  private static final long CAUSE_ID = 0L;
  private static final long DURATION_ID = 1L;
  private static final long ENERGY_USE_ID = 2L;

  public DeviceEnergyManagementClusterPowerAdjustEndEvent(
    Integer cause,
    Long duration,
    Long energyUse
  ) {
    this.cause = cause;
    this.duration = duration;
    this.energyUse = energyUse;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(CAUSE_ID, new UIntType(cause)));
    values.add(new StructElement(DURATION_ID, new UIntType(duration)));
    values.add(new StructElement(ENERGY_USE_ID, new IntType(energyUse)));

    return new StructType(values);
  }

  public static DeviceEnergyManagementClusterPowerAdjustEndEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer cause = null;
    Long duration = null;
    Long energyUse = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == CAUSE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          cause = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == DURATION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          duration = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == ENERGY_USE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          energyUse = castingValue.value(Long.class);
        }
      }
    }
    return new DeviceEnergyManagementClusterPowerAdjustEndEvent(
      cause,
      duration,
      energyUse
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("DeviceEnergyManagementClusterPowerAdjustEndEvent {\n");
    output.append("\tcause: ");
    output.append(cause);
    output.append("\n");
    output.append("\tduration: ");
    output.append(duration);
    output.append("\n");
    output.append("\tenergyUse: ");
    output.append(energyUse);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class DeviceEnergyManagementClusterPausedEvent {

  public DeviceEnergyManagementClusterPausedEvent(
  ) {
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();

    return new StructType(values);
  }

  public static DeviceEnergyManagementClusterPausedEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    return new DeviceEnergyManagementClusterPausedEvent(
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("DeviceEnergyManagementClusterPausedEvent {\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class DeviceEnergyManagementClusterResumedEvent {
  public Integer cause;
  private static final long CAUSE_ID = 0L;

  public DeviceEnergyManagementClusterResumedEvent(
    Integer cause
  ) {
    this.cause = cause;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(CAUSE_ID, new UIntType(cause)));

    return new StructType(values);
  }

  public static DeviceEnergyManagementClusterResumedEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer cause = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == CAUSE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          cause = castingValue.value(Integer.class);
        }
      }
    }
    return new DeviceEnergyManagementClusterResumedEvent(
      cause
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("DeviceEnergyManagementClusterResumedEvent {\n");
    output.append("\tcause: ");
    output.append(cause);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class EnergyEvseClusterEVConnectedEvent {
  public Long sessionID;
  private static final long SESSION_ID_ID = 0L;

  public EnergyEvseClusterEVConnectedEvent(
    Long sessionID
  ) {
    this.sessionID = sessionID;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(SESSION_ID_ID, new UIntType(sessionID)));

    return new StructType(values);
  }

  public static EnergyEvseClusterEVConnectedEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long sessionID = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == SESSION_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          sessionID = castingValue.value(Long.class);
        }
      }
    }
    return new EnergyEvseClusterEVConnectedEvent(
      sessionID
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("EnergyEvseClusterEVConnectedEvent {\n");
    output.append("\tsessionID: ");
    output.append(sessionID);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class EnergyEvseClusterEVNotDetectedEvent {
  public Long sessionID;
  public Integer state;
  public Long sessionDuration;
  public Long sessionEnergyCharged;
  public Optional<Long> sessionEnergyDischarged;
  private static final long SESSION_ID_ID = 0L;
  private static final long STATE_ID = 1L;
  private static final long SESSION_DURATION_ID = 2L;
  private static final long SESSION_ENERGY_CHARGED_ID = 3L;
  private static final long SESSION_ENERGY_DISCHARGED_ID = 4L;

  public EnergyEvseClusterEVNotDetectedEvent(
    Long sessionID,
    Integer state,
    Long sessionDuration,
    Long sessionEnergyCharged,
    Optional<Long> sessionEnergyDischarged
  ) {
    this.sessionID = sessionID;
    this.state = state;
    this.sessionDuration = sessionDuration;
    this.sessionEnergyCharged = sessionEnergyCharged;
    this.sessionEnergyDischarged = sessionEnergyDischarged;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(SESSION_ID_ID, new UIntType(sessionID)));
    values.add(new StructElement(STATE_ID, new UIntType(state)));
    values.add(new StructElement(SESSION_DURATION_ID, new UIntType(sessionDuration)));
    values.add(new StructElement(SESSION_ENERGY_CHARGED_ID, new IntType(sessionEnergyCharged)));
    values.add(new StructElement(SESSION_ENERGY_DISCHARGED_ID, sessionEnergyDischarged.<BaseTLVType>map((nonOptionalsessionEnergyDischarged) -> new IntType(nonOptionalsessionEnergyDischarged)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static EnergyEvseClusterEVNotDetectedEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long sessionID = null;
    Integer state = null;
    Long sessionDuration = null;
    Long sessionEnergyCharged = null;
    Optional<Long> sessionEnergyDischarged = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == SESSION_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          sessionID = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == STATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          state = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == SESSION_DURATION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          sessionDuration = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == SESSION_ENERGY_CHARGED_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          sessionEnergyCharged = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == SESSION_ENERGY_DISCHARGED_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          sessionEnergyDischarged = Optional.of(castingValue.value(Long.class));
        }
      }
    }
    return new EnergyEvseClusterEVNotDetectedEvent(
      sessionID,
      state,
      sessionDuration,
      sessionEnergyCharged,
      sessionEnergyDischarged
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("EnergyEvseClusterEVNotDetectedEvent {\n");
    output.append("\tsessionID: ");
    output.append(sessionID);
    output.append("\n");
    output.append("\tstate: ");
    output.append(state);
    output.append("\n");
    output.append("\tsessionDuration: ");
    output.append(sessionDuration);
    output.append("\n");
    output.append("\tsessionEnergyCharged: ");
    output.append(sessionEnergyCharged);
    output.append("\n");
    output.append("\tsessionEnergyDischarged: ");
    output.append(sessionEnergyDischarged);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class EnergyEvseClusterEnergyTransferStartedEvent {
  public Long sessionID;
  public Integer state;
  public Long maximumCurrent;
  public Optional<Long> maximumDischargeCurrent;
  private static final long SESSION_ID_ID = 0L;
  private static final long STATE_ID = 1L;
  private static final long MAXIMUM_CURRENT_ID = 2L;
  private static final long MAXIMUM_DISCHARGE_CURRENT_ID = 3L;

  public EnergyEvseClusterEnergyTransferStartedEvent(
    Long sessionID,
    Integer state,
    Long maximumCurrent,
    Optional<Long> maximumDischargeCurrent
  ) {
    this.sessionID = sessionID;
    this.state = state;
    this.maximumCurrent = maximumCurrent;
    this.maximumDischargeCurrent = maximumDischargeCurrent;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(SESSION_ID_ID, new UIntType(sessionID)));
    values.add(new StructElement(STATE_ID, new UIntType(state)));
    values.add(new StructElement(MAXIMUM_CURRENT_ID, new IntType(maximumCurrent)));
    values.add(new StructElement(MAXIMUM_DISCHARGE_CURRENT_ID, maximumDischargeCurrent.<BaseTLVType>map((nonOptionalmaximumDischargeCurrent) -> new IntType(nonOptionalmaximumDischargeCurrent)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static EnergyEvseClusterEnergyTransferStartedEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long sessionID = null;
    Integer state = null;
    Long maximumCurrent = null;
    Optional<Long> maximumDischargeCurrent = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == SESSION_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          sessionID = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == STATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          state = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == MAXIMUM_CURRENT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          maximumCurrent = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == MAXIMUM_DISCHARGE_CURRENT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          maximumDischargeCurrent = Optional.of(castingValue.value(Long.class));
        }
      }
    }
    return new EnergyEvseClusterEnergyTransferStartedEvent(
      sessionID,
      state,
      maximumCurrent,
      maximumDischargeCurrent
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("EnergyEvseClusterEnergyTransferStartedEvent {\n");
    output.append("\tsessionID: ");
    output.append(sessionID);
    output.append("\n");
    output.append("\tstate: ");
    output.append(state);
    output.append("\n");
    output.append("\tmaximumCurrent: ");
    output.append(maximumCurrent);
    output.append("\n");
    output.append("\tmaximumDischargeCurrent: ");
    output.append(maximumDischargeCurrent);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class EnergyEvseClusterEnergyTransferStoppedEvent {
  public Long sessionID;
  public Integer state;
  public Integer reason;
  public Long energyTransferred;
  public Optional<Long> energyDischarged;
  private static final long SESSION_ID_ID = 0L;
  private static final long STATE_ID = 1L;
  private static final long REASON_ID = 2L;
  private static final long ENERGY_TRANSFERRED_ID = 4L;
  private static final long ENERGY_DISCHARGED_ID = 5L;

  public EnergyEvseClusterEnergyTransferStoppedEvent(
    Long sessionID,
    Integer state,
    Integer reason,
    Long energyTransferred,
    Optional<Long> energyDischarged
  ) {
    this.sessionID = sessionID;
    this.state = state;
    this.reason = reason;
    this.energyTransferred = energyTransferred;
    this.energyDischarged = energyDischarged;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(SESSION_ID_ID, new UIntType(sessionID)));
    values.add(new StructElement(STATE_ID, new UIntType(state)));
    values.add(new StructElement(REASON_ID, new UIntType(reason)));
    values.add(new StructElement(ENERGY_TRANSFERRED_ID, new IntType(energyTransferred)));
    values.add(new StructElement(ENERGY_DISCHARGED_ID, energyDischarged.<BaseTLVType>map((nonOptionalenergyDischarged) -> new IntType(nonOptionalenergyDischarged)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static EnergyEvseClusterEnergyTransferStoppedEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long sessionID = null;
    Integer state = null;
    Integer reason = null;
    Long energyTransferred = null;
    Optional<Long> energyDischarged = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == SESSION_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          sessionID = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == STATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          state = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == REASON_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          reason = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == ENERGY_TRANSFERRED_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          energyTransferred = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == ENERGY_DISCHARGED_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          energyDischarged = Optional.of(castingValue.value(Long.class));
        }
      }
    }
    return new EnergyEvseClusterEnergyTransferStoppedEvent(
      sessionID,
      state,
      reason,
      energyTransferred,
      energyDischarged
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("EnergyEvseClusterEnergyTransferStoppedEvent {\n");
    output.append("\tsessionID: ");
    output.append(sessionID);
    output.append("\n");
    output.append("\tstate: ");
    output.append(state);
    output.append("\n");
    output.append("\treason: ");
    output.append(reason);
    output.append("\n");
    output.append("\tenergyTransferred: ");
    output.append(energyTransferred);
    output.append("\n");
    output.append("\tenergyDischarged: ");
    output.append(energyDischarged);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class EnergyEvseClusterFaultEvent {
  public @Nullable Long sessionID;
  public Integer state;
  public Integer faultStatePreviousState;
  public Integer faultStateCurrentState;
  private static final long SESSION_ID_ID = 0L;
  private static final long STATE_ID = 1L;
  private static final long FAULT_STATE_PREVIOUS_STATE_ID = 2L;
  private static final long FAULT_STATE_CURRENT_STATE_ID = 4L;

  public EnergyEvseClusterFaultEvent(
    @Nullable Long sessionID,
    Integer state,
    Integer faultStatePreviousState,
    Integer faultStateCurrentState
  ) {
    this.sessionID = sessionID;
    this.state = state;
    this.faultStatePreviousState = faultStatePreviousState;
    this.faultStateCurrentState = faultStateCurrentState;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(SESSION_ID_ID, sessionID != null ? new UIntType(sessionID) : new NullType()));
    values.add(new StructElement(STATE_ID, new UIntType(state)));
    values.add(new StructElement(FAULT_STATE_PREVIOUS_STATE_ID, new UIntType(faultStatePreviousState)));
    values.add(new StructElement(FAULT_STATE_CURRENT_STATE_ID, new UIntType(faultStateCurrentState)));

    return new StructType(values);
  }

  public static EnergyEvseClusterFaultEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    @Nullable Long sessionID = null;
    Integer state = null;
    Integer faultStatePreviousState = null;
    Integer faultStateCurrentState = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == SESSION_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          sessionID = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == STATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          state = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == FAULT_STATE_PREVIOUS_STATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          faultStatePreviousState = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == FAULT_STATE_CURRENT_STATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          faultStateCurrentState = castingValue.value(Integer.class);
        }
      }
    }
    return new EnergyEvseClusterFaultEvent(
      sessionID,
      state,
      faultStatePreviousState,
      faultStateCurrentState
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("EnergyEvseClusterFaultEvent {\n");
    output.append("\tsessionID: ");
    output.append(sessionID);
    output.append("\n");
    output.append("\tstate: ");
    output.append(state);
    output.append("\n");
    output.append("\tfaultStatePreviousState: ");
    output.append(faultStatePreviousState);
    output.append("\n");
    output.append("\tfaultStateCurrentState: ");
    output.append(faultStateCurrentState);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class EnergyEvseClusterRFIDEvent {
  public byte[] uid;
  private static final long UID_ID = 0L;

  public EnergyEvseClusterRFIDEvent(
    byte[] uid
  ) {
    this.uid = uid;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(UID_ID, new ByteArrayType(uid)));

    return new StructType(values);
  }

  public static EnergyEvseClusterRFIDEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    byte[] uid = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == UID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.ByteArray) {
          ByteArrayType castingValue = element.value(ByteArrayType.class);
          uid = castingValue.value(byte[].class);
        }
      }
    }
    return new EnergyEvseClusterRFIDEvent(
      uid
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("EnergyEvseClusterRFIDEvent {\n");
    output.append("\tuid: ");
    output.append(Arrays.toString(uid));
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class DoorLockClusterDoorLockAlarmEvent {
  public Integer alarmCode;
  private static final long ALARM_CODE_ID = 0L;

  public DoorLockClusterDoorLockAlarmEvent(
    Integer alarmCode
  ) {
    this.alarmCode = alarmCode;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ALARM_CODE_ID, new UIntType(alarmCode)));

    return new StructType(values);
  }

  public static DoorLockClusterDoorLockAlarmEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer alarmCode = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ALARM_CODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          alarmCode = castingValue.value(Integer.class);
        }
      }
    }
    return new DoorLockClusterDoorLockAlarmEvent(
      alarmCode
    );
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
}
public static class DoorLockClusterDoorStateChangeEvent {
  public Integer doorState;
  private static final long DOOR_STATE_ID = 0L;

  public DoorLockClusterDoorStateChangeEvent(
    Integer doorState
  ) {
    this.doorState = doorState;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(DOOR_STATE_ID, new UIntType(doorState)));

    return new StructType(values);
  }

  public static DoorLockClusterDoorStateChangeEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer doorState = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == DOOR_STATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          doorState = castingValue.value(Integer.class);
        }
      }
    }
    return new DoorLockClusterDoorStateChangeEvent(
      doorState
    );
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
}
public static class DoorLockClusterLockOperationEvent {
  public Integer lockOperationType;
  public Integer operationSource;
  public @Nullable Integer userIndex;
  public @Nullable Integer fabricIndex;
  public @Nullable Long sourceNode;
  public @Nullable Optional<ArrayList<ChipStructs.DoorLockClusterCredentialStruct>> credentials;
  private static final long LOCK_OPERATION_TYPE_ID = 0L;
  private static final long OPERATION_SOURCE_ID = 1L;
  private static final long USER_INDEX_ID = 2L;
  private static final long FABRIC_INDEX_ID = 3L;
  private static final long SOURCE_NODE_ID = 4L;
  private static final long CREDENTIALS_ID = 5L;

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

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(LOCK_OPERATION_TYPE_ID, new UIntType(lockOperationType)));
    values.add(new StructElement(OPERATION_SOURCE_ID, new UIntType(operationSource)));
    values.add(new StructElement(USER_INDEX_ID, userIndex != null ? new UIntType(userIndex) : new NullType()));
    values.add(new StructElement(FABRIC_INDEX_ID, fabricIndex != null ? new UIntType(fabricIndex) : new NullType()));
    values.add(new StructElement(SOURCE_NODE_ID, sourceNode != null ? new UIntType(sourceNode) : new NullType()));
    values.add(new StructElement(CREDENTIALS_ID, credentials != null ? credentials.<BaseTLVType>map((nonOptionalcredentials) -> ArrayType.generateArrayType(nonOptionalcredentials, (elementnonOptionalcredentials) -> elementnonOptionalcredentials.encodeTlv())).orElse(new EmptyType()) : new NullType()));

    return new StructType(values);
  }

  public static DoorLockClusterLockOperationEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer lockOperationType = null;
    Integer operationSource = null;
    @Nullable Integer userIndex = null;
    @Nullable Integer fabricIndex = null;
    @Nullable Long sourceNode = null;
    @Nullable Optional<ArrayList<ChipStructs.DoorLockClusterCredentialStruct>> credentials = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == LOCK_OPERATION_TYPE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          lockOperationType = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == OPERATION_SOURCE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          operationSource = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == USER_INDEX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          userIndex = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == FABRIC_INDEX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          fabricIndex = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == SOURCE_NODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          sourceNode = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == CREDENTIALS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          credentials = Optional.of(castingValue.map((elementcastingValue) -> ChipStructs.DoorLockClusterCredentialStruct.decodeTlv(elementcastingValue)));
        }
      }
    }
    return new DoorLockClusterLockOperationEvent(
      lockOperationType,
      operationSource,
      userIndex,
      fabricIndex,
      sourceNode,
      credentials
    );
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
}
public static class DoorLockClusterLockOperationErrorEvent {
  public Integer lockOperationType;
  public Integer operationSource;
  public Integer operationError;
  public @Nullable Integer userIndex;
  public @Nullable Integer fabricIndex;
  public @Nullable Long sourceNode;
  public @Nullable Optional<ArrayList<ChipStructs.DoorLockClusterCredentialStruct>> credentials;
  private static final long LOCK_OPERATION_TYPE_ID = 0L;
  private static final long OPERATION_SOURCE_ID = 1L;
  private static final long OPERATION_ERROR_ID = 2L;
  private static final long USER_INDEX_ID = 3L;
  private static final long FABRIC_INDEX_ID = 4L;
  private static final long SOURCE_NODE_ID = 5L;
  private static final long CREDENTIALS_ID = 6L;

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

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(LOCK_OPERATION_TYPE_ID, new UIntType(lockOperationType)));
    values.add(new StructElement(OPERATION_SOURCE_ID, new UIntType(operationSource)));
    values.add(new StructElement(OPERATION_ERROR_ID, new UIntType(operationError)));
    values.add(new StructElement(USER_INDEX_ID, userIndex != null ? new UIntType(userIndex) : new NullType()));
    values.add(new StructElement(FABRIC_INDEX_ID, fabricIndex != null ? new UIntType(fabricIndex) : new NullType()));
    values.add(new StructElement(SOURCE_NODE_ID, sourceNode != null ? new UIntType(sourceNode) : new NullType()));
    values.add(new StructElement(CREDENTIALS_ID, credentials != null ? credentials.<BaseTLVType>map((nonOptionalcredentials) -> ArrayType.generateArrayType(nonOptionalcredentials, (elementnonOptionalcredentials) -> elementnonOptionalcredentials.encodeTlv())).orElse(new EmptyType()) : new NullType()));

    return new StructType(values);
  }

  public static DoorLockClusterLockOperationErrorEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer lockOperationType = null;
    Integer operationSource = null;
    Integer operationError = null;
    @Nullable Integer userIndex = null;
    @Nullable Integer fabricIndex = null;
    @Nullable Long sourceNode = null;
    @Nullable Optional<ArrayList<ChipStructs.DoorLockClusterCredentialStruct>> credentials = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == LOCK_OPERATION_TYPE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          lockOperationType = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == OPERATION_SOURCE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          operationSource = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == OPERATION_ERROR_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          operationError = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == USER_INDEX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          userIndex = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == FABRIC_INDEX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          fabricIndex = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == SOURCE_NODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          sourceNode = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == CREDENTIALS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          credentials = Optional.of(castingValue.map((elementcastingValue) -> ChipStructs.DoorLockClusterCredentialStruct.decodeTlv(elementcastingValue)));
        }
      }
    }
    return new DoorLockClusterLockOperationErrorEvent(
      lockOperationType,
      operationSource,
      operationError,
      userIndex,
      fabricIndex,
      sourceNode,
      credentials
    );
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
}
public static class DoorLockClusterLockUserChangeEvent {
  public Integer lockDataType;
  public Integer dataOperationType;
  public Integer operationSource;
  public @Nullable Integer userIndex;
  public @Nullable Integer fabricIndex;
  public @Nullable Long sourceNode;
  public @Nullable Integer dataIndex;
  private static final long LOCK_DATA_TYPE_ID = 0L;
  private static final long DATA_OPERATION_TYPE_ID = 1L;
  private static final long OPERATION_SOURCE_ID = 2L;
  private static final long USER_INDEX_ID = 3L;
  private static final long FABRIC_INDEX_ID = 4L;
  private static final long SOURCE_NODE_ID = 5L;
  private static final long DATA_INDEX_ID = 6L;

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

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(LOCK_DATA_TYPE_ID, new UIntType(lockDataType)));
    values.add(new StructElement(DATA_OPERATION_TYPE_ID, new UIntType(dataOperationType)));
    values.add(new StructElement(OPERATION_SOURCE_ID, new UIntType(operationSource)));
    values.add(new StructElement(USER_INDEX_ID, userIndex != null ? new UIntType(userIndex) : new NullType()));
    values.add(new StructElement(FABRIC_INDEX_ID, fabricIndex != null ? new UIntType(fabricIndex) : new NullType()));
    values.add(new StructElement(SOURCE_NODE_ID, sourceNode != null ? new UIntType(sourceNode) : new NullType()));
    values.add(new StructElement(DATA_INDEX_ID, dataIndex != null ? new UIntType(dataIndex) : new NullType()));

    return new StructType(values);
  }

  public static DoorLockClusterLockUserChangeEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer lockDataType = null;
    Integer dataOperationType = null;
    Integer operationSource = null;
    @Nullable Integer userIndex = null;
    @Nullable Integer fabricIndex = null;
    @Nullable Long sourceNode = null;
    @Nullable Integer dataIndex = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == LOCK_DATA_TYPE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          lockDataType = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == DATA_OPERATION_TYPE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          dataOperationType = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == OPERATION_SOURCE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          operationSource = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == USER_INDEX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          userIndex = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == FABRIC_INDEX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          fabricIndex = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == SOURCE_NODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          sourceNode = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == DATA_INDEX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          dataIndex = castingValue.value(Integer.class);
        }
      }
    }
    return new DoorLockClusterLockUserChangeEvent(
      lockDataType,
      dataOperationType,
      operationSource,
      userIndex,
      fabricIndex,
      sourceNode,
      dataIndex
    );
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
}
public static class PumpConfigurationAndControlClusterSupplyVoltageLowEvent {

  public PumpConfigurationAndControlClusterSupplyVoltageLowEvent(
  ) {
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();

    return new StructType(values);
  }

  public static PumpConfigurationAndControlClusterSupplyVoltageLowEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    return new PumpConfigurationAndControlClusterSupplyVoltageLowEvent(
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("PumpConfigurationAndControlClusterSupplyVoltageLowEvent {\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class PumpConfigurationAndControlClusterSupplyVoltageHighEvent {

  public PumpConfigurationAndControlClusterSupplyVoltageHighEvent(
  ) {
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();

    return new StructType(values);
  }

  public static PumpConfigurationAndControlClusterSupplyVoltageHighEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    return new PumpConfigurationAndControlClusterSupplyVoltageHighEvent(
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("PumpConfigurationAndControlClusterSupplyVoltageHighEvent {\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class PumpConfigurationAndControlClusterPowerMissingPhaseEvent {

  public PumpConfigurationAndControlClusterPowerMissingPhaseEvent(
  ) {
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();

    return new StructType(values);
  }

  public static PumpConfigurationAndControlClusterPowerMissingPhaseEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    return new PumpConfigurationAndControlClusterPowerMissingPhaseEvent(
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("PumpConfigurationAndControlClusterPowerMissingPhaseEvent {\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class PumpConfigurationAndControlClusterSystemPressureLowEvent {

  public PumpConfigurationAndControlClusterSystemPressureLowEvent(
  ) {
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();

    return new StructType(values);
  }

  public static PumpConfigurationAndControlClusterSystemPressureLowEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    return new PumpConfigurationAndControlClusterSystemPressureLowEvent(
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("PumpConfigurationAndControlClusterSystemPressureLowEvent {\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class PumpConfigurationAndControlClusterSystemPressureHighEvent {

  public PumpConfigurationAndControlClusterSystemPressureHighEvent(
  ) {
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();

    return new StructType(values);
  }

  public static PumpConfigurationAndControlClusterSystemPressureHighEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    return new PumpConfigurationAndControlClusterSystemPressureHighEvent(
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("PumpConfigurationAndControlClusterSystemPressureHighEvent {\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class PumpConfigurationAndControlClusterDryRunningEvent {

  public PumpConfigurationAndControlClusterDryRunningEvent(
  ) {
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();

    return new StructType(values);
  }

  public static PumpConfigurationAndControlClusterDryRunningEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    return new PumpConfigurationAndControlClusterDryRunningEvent(
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("PumpConfigurationAndControlClusterDryRunningEvent {\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class PumpConfigurationAndControlClusterMotorTemperatureHighEvent {

  public PumpConfigurationAndControlClusterMotorTemperatureHighEvent(
  ) {
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();

    return new StructType(values);
  }

  public static PumpConfigurationAndControlClusterMotorTemperatureHighEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    return new PumpConfigurationAndControlClusterMotorTemperatureHighEvent(
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("PumpConfigurationAndControlClusterMotorTemperatureHighEvent {\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class PumpConfigurationAndControlClusterPumpMotorFatalFailureEvent {

  public PumpConfigurationAndControlClusterPumpMotorFatalFailureEvent(
  ) {
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();

    return new StructType(values);
  }

  public static PumpConfigurationAndControlClusterPumpMotorFatalFailureEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    return new PumpConfigurationAndControlClusterPumpMotorFatalFailureEvent(
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("PumpConfigurationAndControlClusterPumpMotorFatalFailureEvent {\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class PumpConfigurationAndControlClusterElectronicTemperatureHighEvent {

  public PumpConfigurationAndControlClusterElectronicTemperatureHighEvent(
  ) {
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();

    return new StructType(values);
  }

  public static PumpConfigurationAndControlClusterElectronicTemperatureHighEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    return new PumpConfigurationAndControlClusterElectronicTemperatureHighEvent(
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("PumpConfigurationAndControlClusterElectronicTemperatureHighEvent {\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class PumpConfigurationAndControlClusterPumpBlockedEvent {

  public PumpConfigurationAndControlClusterPumpBlockedEvent(
  ) {
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();

    return new StructType(values);
  }

  public static PumpConfigurationAndControlClusterPumpBlockedEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    return new PumpConfigurationAndControlClusterPumpBlockedEvent(
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("PumpConfigurationAndControlClusterPumpBlockedEvent {\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class PumpConfigurationAndControlClusterSensorFailureEvent {

  public PumpConfigurationAndControlClusterSensorFailureEvent(
  ) {
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();

    return new StructType(values);
  }

  public static PumpConfigurationAndControlClusterSensorFailureEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    return new PumpConfigurationAndControlClusterSensorFailureEvent(
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("PumpConfigurationAndControlClusterSensorFailureEvent {\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class PumpConfigurationAndControlClusterElectronicNonFatalFailureEvent {

  public PumpConfigurationAndControlClusterElectronicNonFatalFailureEvent(
  ) {
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();

    return new StructType(values);
  }

  public static PumpConfigurationAndControlClusterElectronicNonFatalFailureEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    return new PumpConfigurationAndControlClusterElectronicNonFatalFailureEvent(
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("PumpConfigurationAndControlClusterElectronicNonFatalFailureEvent {\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class PumpConfigurationAndControlClusterElectronicFatalFailureEvent {

  public PumpConfigurationAndControlClusterElectronicFatalFailureEvent(
  ) {
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();

    return new StructType(values);
  }

  public static PumpConfigurationAndControlClusterElectronicFatalFailureEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    return new PumpConfigurationAndControlClusterElectronicFatalFailureEvent(
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("PumpConfigurationAndControlClusterElectronicFatalFailureEvent {\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class PumpConfigurationAndControlClusterGeneralFaultEvent {

  public PumpConfigurationAndControlClusterGeneralFaultEvent(
  ) {
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();

    return new StructType(values);
  }

  public static PumpConfigurationAndControlClusterGeneralFaultEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    return new PumpConfigurationAndControlClusterGeneralFaultEvent(
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("PumpConfigurationAndControlClusterGeneralFaultEvent {\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class PumpConfigurationAndControlClusterLeakageEvent {

  public PumpConfigurationAndControlClusterLeakageEvent(
  ) {
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();

    return new StructType(values);
  }

  public static PumpConfigurationAndControlClusterLeakageEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    return new PumpConfigurationAndControlClusterLeakageEvent(
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("PumpConfigurationAndControlClusterLeakageEvent {\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class PumpConfigurationAndControlClusterAirDetectionEvent {

  public PumpConfigurationAndControlClusterAirDetectionEvent(
  ) {
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();

    return new StructType(values);
  }

  public static PumpConfigurationAndControlClusterAirDetectionEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    return new PumpConfigurationAndControlClusterAirDetectionEvent(
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("PumpConfigurationAndControlClusterAirDetectionEvent {\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class PumpConfigurationAndControlClusterTurbineOperationEvent {

  public PumpConfigurationAndControlClusterTurbineOperationEvent(
  ) {
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();

    return new StructType(values);
  }

  public static PumpConfigurationAndControlClusterTurbineOperationEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    return new PumpConfigurationAndControlClusterTurbineOperationEvent(
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("PumpConfigurationAndControlClusterTurbineOperationEvent {\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class OccupancySensingClusterOccupancyChangedEvent {
  public Integer occupancy;
  private static final long OCCUPANCY_ID = 0L;

  public OccupancySensingClusterOccupancyChangedEvent(
    Integer occupancy
  ) {
    this.occupancy = occupancy;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(OCCUPANCY_ID, new UIntType(occupancy)));

    return new StructType(values);
  }

  public static OccupancySensingClusterOccupancyChangedEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer occupancy = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == OCCUPANCY_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          occupancy = castingValue.value(Integer.class);
        }
      }
    }
    return new OccupancySensingClusterOccupancyChangedEvent(
      occupancy
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("OccupancySensingClusterOccupancyChangedEvent {\n");
    output.append("\toccupancy: ");
    output.append(occupancy);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class TargetNavigatorClusterTargetUpdatedEvent {
  public ArrayList<ChipStructs.TargetNavigatorClusterTargetInfoStruct> targetList;
  public Integer currentTarget;
  public byte[] data;
  private static final long TARGET_LIST_ID = 0L;
  private static final long CURRENT_TARGET_ID = 1L;
  private static final long DATA_ID = 2L;

  public TargetNavigatorClusterTargetUpdatedEvent(
    ArrayList<ChipStructs.TargetNavigatorClusterTargetInfoStruct> targetList,
    Integer currentTarget,
    byte[] data
  ) {
    this.targetList = targetList;
    this.currentTarget = currentTarget;
    this.data = data;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(TARGET_LIST_ID, ArrayType.generateArrayType(targetList, (elementtargetList) -> elementtargetList.encodeTlv())));
    values.add(new StructElement(CURRENT_TARGET_ID, new UIntType(currentTarget)));
    values.add(new StructElement(DATA_ID, new ByteArrayType(data)));

    return new StructType(values);
  }

  public static TargetNavigatorClusterTargetUpdatedEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    ArrayList<ChipStructs.TargetNavigatorClusterTargetInfoStruct> targetList = null;
    Integer currentTarget = null;
    byte[] data = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == TARGET_LIST_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          targetList = castingValue.map((elementcastingValue) -> ChipStructs.TargetNavigatorClusterTargetInfoStruct.decodeTlv(elementcastingValue));
        }
      } else if (element.contextTagNum() == CURRENT_TARGET_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          currentTarget = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == DATA_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.ByteArray) {
          ByteArrayType castingValue = element.value(ByteArrayType.class);
          data = castingValue.value(byte[].class);
        }
      }
    }
    return new TargetNavigatorClusterTargetUpdatedEvent(
      targetList,
      currentTarget,
      data
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("TargetNavigatorClusterTargetUpdatedEvent {\n");
    output.append("\ttargetList: ");
    output.append(targetList);
    output.append("\n");
    output.append("\tcurrentTarget: ");
    output.append(currentTarget);
    output.append("\n");
    output.append("\tdata: ");
    output.append(Arrays.toString(data));
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class MediaPlaybackClusterStateChangedEvent {
  public Integer currentState;
  public Long startTime;
  public Long duration;
  public ChipStructs.MediaPlaybackClusterPlaybackPositionStruct sampledPosition;
  public Float playbackSpeed;
  public Long seekRangeEnd;
  public Long seekRangeStart;
  public Optional<byte[]> data;
  public Boolean audioAdvanceUnmuted;
  private static final long CURRENT_STATE_ID = 0L;
  private static final long START_TIME_ID = 1L;
  private static final long DURATION_ID = 2L;
  private static final long SAMPLED_POSITION_ID = 3L;
  private static final long PLAYBACK_SPEED_ID = 4L;
  private static final long SEEK_RANGE_END_ID = 5L;
  private static final long SEEK_RANGE_START_ID = 6L;
  private static final long DATA_ID = 7L;
  private static final long AUDIO_ADVANCE_UNMUTED_ID = 8L;

  public MediaPlaybackClusterStateChangedEvent(
    Integer currentState,
    Long startTime,
    Long duration,
    ChipStructs.MediaPlaybackClusterPlaybackPositionStruct sampledPosition,
    Float playbackSpeed,
    Long seekRangeEnd,
    Long seekRangeStart,
    Optional<byte[]> data,
    Boolean audioAdvanceUnmuted
  ) {
    this.currentState = currentState;
    this.startTime = startTime;
    this.duration = duration;
    this.sampledPosition = sampledPosition;
    this.playbackSpeed = playbackSpeed;
    this.seekRangeEnd = seekRangeEnd;
    this.seekRangeStart = seekRangeStart;
    this.data = data;
    this.audioAdvanceUnmuted = audioAdvanceUnmuted;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(CURRENT_STATE_ID, new UIntType(currentState)));
    values.add(new StructElement(START_TIME_ID, new UIntType(startTime)));
    values.add(new StructElement(DURATION_ID, new UIntType(duration)));
    values.add(new StructElement(SAMPLED_POSITION_ID, sampledPosition.encodeTlv()));
    values.add(new StructElement(PLAYBACK_SPEED_ID, new FloatType(playbackSpeed)));
    values.add(new StructElement(SEEK_RANGE_END_ID, new UIntType(seekRangeEnd)));
    values.add(new StructElement(SEEK_RANGE_START_ID, new UIntType(seekRangeStart)));
    values.add(new StructElement(DATA_ID, data.<BaseTLVType>map((nonOptionaldata) -> new ByteArrayType(nonOptionaldata)).orElse(new EmptyType())));
    values.add(new StructElement(AUDIO_ADVANCE_UNMUTED_ID, new BooleanType(audioAdvanceUnmuted)));

    return new StructType(values);
  }

  public static MediaPlaybackClusterStateChangedEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer currentState = null;
    Long startTime = null;
    Long duration = null;
    ChipStructs.MediaPlaybackClusterPlaybackPositionStruct sampledPosition = null;
    Float playbackSpeed = null;
    Long seekRangeEnd = null;
    Long seekRangeStart = null;
    Optional<byte[]> data = Optional.empty();
    Boolean audioAdvanceUnmuted = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == CURRENT_STATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          currentState = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == START_TIME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          startTime = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == DURATION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          duration = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == SAMPLED_POSITION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          sampledPosition = ChipStructs.MediaPlaybackClusterPlaybackPositionStruct.decodeTlv(castingValue);
        }
      } else if (element.contextTagNum() == PLAYBACK_SPEED_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Float) {
          FloatType castingValue = element.value(FloatType.class);
          playbackSpeed = castingValue.value(Float.class);
        }
      } else if (element.contextTagNum() == SEEK_RANGE_END_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          seekRangeEnd = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == SEEK_RANGE_START_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          seekRangeStart = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == DATA_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.ByteArray) {
          ByteArrayType castingValue = element.value(ByteArrayType.class);
          data = Optional.of(castingValue.value(byte[].class));
        }
      } else if (element.contextTagNum() == AUDIO_ADVANCE_UNMUTED_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          audioAdvanceUnmuted = castingValue.value(Boolean.class);
        }
      }
    }
    return new MediaPlaybackClusterStateChangedEvent(
      currentState,
      startTime,
      duration,
      sampledPosition,
      playbackSpeed,
      seekRangeEnd,
      seekRangeStart,
      data,
      audioAdvanceUnmuted
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("MediaPlaybackClusterStateChangedEvent {\n");
    output.append("\tcurrentState: ");
    output.append(currentState);
    output.append("\n");
    output.append("\tstartTime: ");
    output.append(startTime);
    output.append("\n");
    output.append("\tduration: ");
    output.append(duration);
    output.append("\n");
    output.append("\tsampledPosition: ");
    output.append(sampledPosition);
    output.append("\n");
    output.append("\tplaybackSpeed: ");
    output.append(playbackSpeed);
    output.append("\n");
    output.append("\tseekRangeEnd: ");
    output.append(seekRangeEnd);
    output.append("\n");
    output.append("\tseekRangeStart: ");
    output.append(seekRangeStart);
    output.append("\n");
    output.append("\tdata: ");
    output.append(data.isPresent() ? Arrays.toString(data.get()) : "");
    output.append("\n");
    output.append("\taudioAdvanceUnmuted: ");
    output.append(audioAdvanceUnmuted);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class AccountLoginClusterLoggedOutEvent {
  public Optional<Long> node;
  private static final long NODE_ID = 0L;

  public AccountLoginClusterLoggedOutEvent(
    Optional<Long> node
  ) {
    this.node = node;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(NODE_ID, node.<BaseTLVType>map((nonOptionalnode) -> new UIntType(nonOptionalnode)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static AccountLoginClusterLoggedOutEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Optional<Long> node = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == NODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          node = Optional.of(castingValue.value(Long.class));
        }
      }
    }
    return new AccountLoginClusterLoggedOutEvent(
      node
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("AccountLoginClusterLoggedOutEvent {\n");
    output.append("\tnode: ");
    output.append(node);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ContentControlClusterRemainingScreenTimeExpiredEvent {

  public ContentControlClusterRemainingScreenTimeExpiredEvent(
  ) {
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();

    return new StructType(values);
  }

  public static ContentControlClusterRemainingScreenTimeExpiredEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    return new ContentControlClusterRemainingScreenTimeExpiredEvent(
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ContentControlClusterRemainingScreenTimeExpiredEvent {\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ZoneManagementClusterZoneTriggeredEvent {
  public ArrayList<Integer> zones;
  public Integer reason;
  private static final long ZONES_ID = 0L;
  private static final long REASON_ID = 1L;

  public ZoneManagementClusterZoneTriggeredEvent(
    ArrayList<Integer> zones,
    Integer reason
  ) {
    this.zones = zones;
    this.reason = reason;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ZONES_ID, ArrayType.generateArrayType(zones, (elementzones) -> new UIntType(elementzones))));
    values.add(new StructElement(REASON_ID, new UIntType(reason)));

    return new StructType(values);
  }

  public static ZoneManagementClusterZoneTriggeredEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    ArrayList<Integer> zones = null;
    Integer reason = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ZONES_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          zones = castingValue.map((elementcastingValue) -> elementcastingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == REASON_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          reason = castingValue.value(Integer.class);
        }
      }
    }
    return new ZoneManagementClusterZoneTriggeredEvent(
      zones,
      reason
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ZoneManagementClusterZoneTriggeredEvent {\n");
    output.append("\tzones: ");
    output.append(zones);
    output.append("\n");
    output.append("\treason: ");
    output.append(reason);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ZoneManagementClusterZoneStoppedEvent {
  public ArrayList<Integer> zones;
  public Integer reason;
  private static final long ZONES_ID = 0L;
  private static final long REASON_ID = 1L;

  public ZoneManagementClusterZoneStoppedEvent(
    ArrayList<Integer> zones,
    Integer reason
  ) {
    this.zones = zones;
    this.reason = reason;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ZONES_ID, ArrayType.generateArrayType(zones, (elementzones) -> new UIntType(elementzones))));
    values.add(new StructElement(REASON_ID, new UIntType(reason)));

    return new StructType(values);
  }

  public static ZoneManagementClusterZoneStoppedEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    ArrayList<Integer> zones = null;
    Integer reason = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ZONES_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          zones = castingValue.map((elementcastingValue) -> elementcastingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == REASON_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          reason = castingValue.value(Integer.class);
        }
      }
    }
    return new ZoneManagementClusterZoneStoppedEvent(
      zones,
      reason
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ZoneManagementClusterZoneStoppedEvent {\n");
    output.append("\tzones: ");
    output.append(zones);
    output.append("\n");
    output.append("\treason: ");
    output.append(reason);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class CameraAvStreamManagementClusterVideoStreamChangedEvent {
  public Integer videoStreamID;
  public Optional<Integer> streamUsage;
  public Optional<Integer> videoCodec;
  public Optional<Integer> minFrameRate;
  public Optional<Integer> maxFrameRate;
  public Optional<ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct> minResolution;
  public Optional<ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct> maxResolution;
  public Optional<Long> minBitRate;
  public Optional<Long> maxBitRate;
  public Optional<Integer> minFragmentLen;
  public Optional<Integer> maxFragmentLen;
  private static final long VIDEO_STREAM_ID_ID = 0L;
  private static final long STREAM_USAGE_ID = 1L;
  private static final long VIDEO_CODEC_ID = 2L;
  private static final long MIN_FRAME_RATE_ID = 3L;
  private static final long MAX_FRAME_RATE_ID = 4L;
  private static final long MIN_RESOLUTION_ID = 5L;
  private static final long MAX_RESOLUTION_ID = 6L;
  private static final long MIN_BIT_RATE_ID = 7L;
  private static final long MAX_BIT_RATE_ID = 8L;
  private static final long MIN_FRAGMENT_LEN_ID = 9L;
  private static final long MAX_FRAGMENT_LEN_ID = 10L;

  public CameraAvStreamManagementClusterVideoStreamChangedEvent(
    Integer videoStreamID,
    Optional<Integer> streamUsage,
    Optional<Integer> videoCodec,
    Optional<Integer> minFrameRate,
    Optional<Integer> maxFrameRate,
    Optional<ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct> minResolution,
    Optional<ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct> maxResolution,
    Optional<Long> minBitRate,
    Optional<Long> maxBitRate,
    Optional<Integer> minFragmentLen,
    Optional<Integer> maxFragmentLen
  ) {
    this.videoStreamID = videoStreamID;
    this.streamUsage = streamUsage;
    this.videoCodec = videoCodec;
    this.minFrameRate = minFrameRate;
    this.maxFrameRate = maxFrameRate;
    this.minResolution = minResolution;
    this.maxResolution = maxResolution;
    this.minBitRate = minBitRate;
    this.maxBitRate = maxBitRate;
    this.minFragmentLen = minFragmentLen;
    this.maxFragmentLen = maxFragmentLen;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(VIDEO_STREAM_ID_ID, new UIntType(videoStreamID)));
    values.add(new StructElement(STREAM_USAGE_ID, streamUsage.<BaseTLVType>map((nonOptionalstreamUsage) -> new UIntType(nonOptionalstreamUsage)).orElse(new EmptyType())));
    values.add(new StructElement(VIDEO_CODEC_ID, videoCodec.<BaseTLVType>map((nonOptionalvideoCodec) -> new UIntType(nonOptionalvideoCodec)).orElse(new EmptyType())));
    values.add(new StructElement(MIN_FRAME_RATE_ID, minFrameRate.<BaseTLVType>map((nonOptionalminFrameRate) -> new UIntType(nonOptionalminFrameRate)).orElse(new EmptyType())));
    values.add(new StructElement(MAX_FRAME_RATE_ID, maxFrameRate.<BaseTLVType>map((nonOptionalmaxFrameRate) -> new UIntType(nonOptionalmaxFrameRate)).orElse(new EmptyType())));
    values.add(new StructElement(MIN_RESOLUTION_ID, minResolution.<BaseTLVType>map((nonOptionalminResolution) -> nonOptionalminResolution.encodeTlv()).orElse(new EmptyType())));
    values.add(new StructElement(MAX_RESOLUTION_ID, maxResolution.<BaseTLVType>map((nonOptionalmaxResolution) -> nonOptionalmaxResolution.encodeTlv()).orElse(new EmptyType())));
    values.add(new StructElement(MIN_BIT_RATE_ID, minBitRate.<BaseTLVType>map((nonOptionalminBitRate) -> new UIntType(nonOptionalminBitRate)).orElse(new EmptyType())));
    values.add(new StructElement(MAX_BIT_RATE_ID, maxBitRate.<BaseTLVType>map((nonOptionalmaxBitRate) -> new UIntType(nonOptionalmaxBitRate)).orElse(new EmptyType())));
    values.add(new StructElement(MIN_FRAGMENT_LEN_ID, minFragmentLen.<BaseTLVType>map((nonOptionalminFragmentLen) -> new UIntType(nonOptionalminFragmentLen)).orElse(new EmptyType())));
    values.add(new StructElement(MAX_FRAGMENT_LEN_ID, maxFragmentLen.<BaseTLVType>map((nonOptionalmaxFragmentLen) -> new UIntType(nonOptionalmaxFragmentLen)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static CameraAvStreamManagementClusterVideoStreamChangedEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer videoStreamID = null;
    Optional<Integer> streamUsage = Optional.empty();
    Optional<Integer> videoCodec = Optional.empty();
    Optional<Integer> minFrameRate = Optional.empty();
    Optional<Integer> maxFrameRate = Optional.empty();
    Optional<ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct> minResolution = Optional.empty();
    Optional<ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct> maxResolution = Optional.empty();
    Optional<Long> minBitRate = Optional.empty();
    Optional<Long> maxBitRate = Optional.empty();
    Optional<Integer> minFragmentLen = Optional.empty();
    Optional<Integer> maxFragmentLen = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == VIDEO_STREAM_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          videoStreamID = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == STREAM_USAGE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          streamUsage = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == VIDEO_CODEC_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          videoCodec = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == MIN_FRAME_RATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          minFrameRate = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == MAX_FRAME_RATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          maxFrameRate = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == MIN_RESOLUTION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          minResolution = Optional.of(ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct.decodeTlv(castingValue));
        }
      } else if (element.contextTagNum() == MAX_RESOLUTION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          maxResolution = Optional.of(ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct.decodeTlv(castingValue));
        }
      } else if (element.contextTagNum() == MIN_BIT_RATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          minBitRate = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == MAX_BIT_RATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          maxBitRate = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == MIN_FRAGMENT_LEN_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          minFragmentLen = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == MAX_FRAGMENT_LEN_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          maxFragmentLen = Optional.of(castingValue.value(Integer.class));
        }
      }
    }
    return new CameraAvStreamManagementClusterVideoStreamChangedEvent(
      videoStreamID,
      streamUsage,
      videoCodec,
      minFrameRate,
      maxFrameRate,
      minResolution,
      maxResolution,
      minBitRate,
      maxBitRate,
      minFragmentLen,
      maxFragmentLen
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("CameraAvStreamManagementClusterVideoStreamChangedEvent {\n");
    output.append("\tvideoStreamID: ");
    output.append(videoStreamID);
    output.append("\n");
    output.append("\tstreamUsage: ");
    output.append(streamUsage);
    output.append("\n");
    output.append("\tvideoCodec: ");
    output.append(videoCodec);
    output.append("\n");
    output.append("\tminFrameRate: ");
    output.append(minFrameRate);
    output.append("\n");
    output.append("\tmaxFrameRate: ");
    output.append(maxFrameRate);
    output.append("\n");
    output.append("\tminResolution: ");
    output.append(minResolution);
    output.append("\n");
    output.append("\tmaxResolution: ");
    output.append(maxResolution);
    output.append("\n");
    output.append("\tminBitRate: ");
    output.append(minBitRate);
    output.append("\n");
    output.append("\tmaxBitRate: ");
    output.append(maxBitRate);
    output.append("\n");
    output.append("\tminFragmentLen: ");
    output.append(minFragmentLen);
    output.append("\n");
    output.append("\tmaxFragmentLen: ");
    output.append(maxFragmentLen);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class CameraAvStreamManagementClusterAudioStreamChangedEvent {
  public Integer audioStreamID;
  public Optional<Integer> streamUsage;
  public Optional<Integer> audioCodec;
  public Optional<Integer> channelCount;
  public Optional<Long> sampleRate;
  public Optional<Long> bitRate;
  public Optional<Integer> bitDepth;
  private static final long AUDIO_STREAM_ID_ID = 0L;
  private static final long STREAM_USAGE_ID = 1L;
  private static final long AUDIO_CODEC_ID = 2L;
  private static final long CHANNEL_COUNT_ID = 3L;
  private static final long SAMPLE_RATE_ID = 4L;
  private static final long BIT_RATE_ID = 5L;
  private static final long BIT_DEPTH_ID = 6L;

  public CameraAvStreamManagementClusterAudioStreamChangedEvent(
    Integer audioStreamID,
    Optional<Integer> streamUsage,
    Optional<Integer> audioCodec,
    Optional<Integer> channelCount,
    Optional<Long> sampleRate,
    Optional<Long> bitRate,
    Optional<Integer> bitDepth
  ) {
    this.audioStreamID = audioStreamID;
    this.streamUsage = streamUsage;
    this.audioCodec = audioCodec;
    this.channelCount = channelCount;
    this.sampleRate = sampleRate;
    this.bitRate = bitRate;
    this.bitDepth = bitDepth;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(AUDIO_STREAM_ID_ID, new UIntType(audioStreamID)));
    values.add(new StructElement(STREAM_USAGE_ID, streamUsage.<BaseTLVType>map((nonOptionalstreamUsage) -> new UIntType(nonOptionalstreamUsage)).orElse(new EmptyType())));
    values.add(new StructElement(AUDIO_CODEC_ID, audioCodec.<BaseTLVType>map((nonOptionalaudioCodec) -> new UIntType(nonOptionalaudioCodec)).orElse(new EmptyType())));
    values.add(new StructElement(CHANNEL_COUNT_ID, channelCount.<BaseTLVType>map((nonOptionalchannelCount) -> new UIntType(nonOptionalchannelCount)).orElse(new EmptyType())));
    values.add(new StructElement(SAMPLE_RATE_ID, sampleRate.<BaseTLVType>map((nonOptionalsampleRate) -> new UIntType(nonOptionalsampleRate)).orElse(new EmptyType())));
    values.add(new StructElement(BIT_RATE_ID, bitRate.<BaseTLVType>map((nonOptionalbitRate) -> new UIntType(nonOptionalbitRate)).orElse(new EmptyType())));
    values.add(new StructElement(BIT_DEPTH_ID, bitDepth.<BaseTLVType>map((nonOptionalbitDepth) -> new UIntType(nonOptionalbitDepth)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static CameraAvStreamManagementClusterAudioStreamChangedEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer audioStreamID = null;
    Optional<Integer> streamUsage = Optional.empty();
    Optional<Integer> audioCodec = Optional.empty();
    Optional<Integer> channelCount = Optional.empty();
    Optional<Long> sampleRate = Optional.empty();
    Optional<Long> bitRate = Optional.empty();
    Optional<Integer> bitDepth = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == AUDIO_STREAM_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          audioStreamID = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == STREAM_USAGE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          streamUsage = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == AUDIO_CODEC_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          audioCodec = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == CHANNEL_COUNT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          channelCount = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == SAMPLE_RATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          sampleRate = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == BIT_RATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          bitRate = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == BIT_DEPTH_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          bitDepth = Optional.of(castingValue.value(Integer.class));
        }
      }
    }
    return new CameraAvStreamManagementClusterAudioStreamChangedEvent(
      audioStreamID,
      streamUsage,
      audioCodec,
      channelCount,
      sampleRate,
      bitRate,
      bitDepth
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("CameraAvStreamManagementClusterAudioStreamChangedEvent {\n");
    output.append("\taudioStreamID: ");
    output.append(audioStreamID);
    output.append("\n");
    output.append("\tstreamUsage: ");
    output.append(streamUsage);
    output.append("\n");
    output.append("\taudioCodec: ");
    output.append(audioCodec);
    output.append("\n");
    output.append("\tchannelCount: ");
    output.append(channelCount);
    output.append("\n");
    output.append("\tsampleRate: ");
    output.append(sampleRate);
    output.append("\n");
    output.append("\tbitRate: ");
    output.append(bitRate);
    output.append("\n");
    output.append("\tbitDepth: ");
    output.append(bitDepth);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class CameraAvStreamManagementClusterSnapshotStreamChangedEvent {
  public Integer snapshotStreamID;
  public Optional<Integer> imageCodec;
  public Optional<Integer> frameRate;
  public Optional<Long> bitRate;
  public Optional<ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct> minResolution;
  public Optional<ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct> maxResolution;
  public Optional<Integer> quality;
  private static final long SNAPSHOT_STREAM_ID_ID = 0L;
  private static final long IMAGE_CODEC_ID = 1L;
  private static final long FRAME_RATE_ID = 2L;
  private static final long BIT_RATE_ID = 3L;
  private static final long MIN_RESOLUTION_ID = 4L;
  private static final long MAX_RESOLUTION_ID = 5L;
  private static final long QUALITY_ID = 6L;

  public CameraAvStreamManagementClusterSnapshotStreamChangedEvent(
    Integer snapshotStreamID,
    Optional<Integer> imageCodec,
    Optional<Integer> frameRate,
    Optional<Long> bitRate,
    Optional<ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct> minResolution,
    Optional<ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct> maxResolution,
    Optional<Integer> quality
  ) {
    this.snapshotStreamID = snapshotStreamID;
    this.imageCodec = imageCodec;
    this.frameRate = frameRate;
    this.bitRate = bitRate;
    this.minResolution = minResolution;
    this.maxResolution = maxResolution;
    this.quality = quality;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(SNAPSHOT_STREAM_ID_ID, new UIntType(snapshotStreamID)));
    values.add(new StructElement(IMAGE_CODEC_ID, imageCodec.<BaseTLVType>map((nonOptionalimageCodec) -> new UIntType(nonOptionalimageCodec)).orElse(new EmptyType())));
    values.add(new StructElement(FRAME_RATE_ID, frameRate.<BaseTLVType>map((nonOptionalframeRate) -> new UIntType(nonOptionalframeRate)).orElse(new EmptyType())));
    values.add(new StructElement(BIT_RATE_ID, bitRate.<BaseTLVType>map((nonOptionalbitRate) -> new UIntType(nonOptionalbitRate)).orElse(new EmptyType())));
    values.add(new StructElement(MIN_RESOLUTION_ID, minResolution.<BaseTLVType>map((nonOptionalminResolution) -> nonOptionalminResolution.encodeTlv()).orElse(new EmptyType())));
    values.add(new StructElement(MAX_RESOLUTION_ID, maxResolution.<BaseTLVType>map((nonOptionalmaxResolution) -> nonOptionalmaxResolution.encodeTlv()).orElse(new EmptyType())));
    values.add(new StructElement(QUALITY_ID, quality.<BaseTLVType>map((nonOptionalquality) -> new UIntType(nonOptionalquality)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static CameraAvStreamManagementClusterSnapshotStreamChangedEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer snapshotStreamID = null;
    Optional<Integer> imageCodec = Optional.empty();
    Optional<Integer> frameRate = Optional.empty();
    Optional<Long> bitRate = Optional.empty();
    Optional<ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct> minResolution = Optional.empty();
    Optional<ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct> maxResolution = Optional.empty();
    Optional<Integer> quality = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == SNAPSHOT_STREAM_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          snapshotStreamID = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == IMAGE_CODEC_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          imageCodec = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == FRAME_RATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          frameRate = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == BIT_RATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          bitRate = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == MIN_RESOLUTION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          minResolution = Optional.of(ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct.decodeTlv(castingValue));
        }
      } else if (element.contextTagNum() == MAX_RESOLUTION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          maxResolution = Optional.of(ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct.decodeTlv(castingValue));
        }
      } else if (element.contextTagNum() == QUALITY_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          quality = Optional.of(castingValue.value(Integer.class));
        }
      }
    }
    return new CameraAvStreamManagementClusterSnapshotStreamChangedEvent(
      snapshotStreamID,
      imageCodec,
      frameRate,
      bitRate,
      minResolution,
      maxResolution,
      quality
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("CameraAvStreamManagementClusterSnapshotStreamChangedEvent {\n");
    output.append("\tsnapshotStreamID: ");
    output.append(snapshotStreamID);
    output.append("\n");
    output.append("\timageCodec: ");
    output.append(imageCodec);
    output.append("\n");
    output.append("\tframeRate: ");
    output.append(frameRate);
    output.append("\n");
    output.append("\tbitRate: ");
    output.append(bitRate);
    output.append("\n");
    output.append("\tminResolution: ");
    output.append(minResolution);
    output.append("\n");
    output.append("\tmaxResolution: ");
    output.append(maxResolution);
    output.append("\n");
    output.append("\tquality: ");
    output.append(quality);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class PushAvStreamTransportClusterPushTransportBeginEvent {
  public Integer connectionID;
  public Integer triggerType;
  public Optional<Integer> activationReason;
  private static final long CONNECTION_ID_ID = 0L;
  private static final long TRIGGER_TYPE_ID = 1L;
  private static final long ACTIVATION_REASON_ID = 2L;

  public PushAvStreamTransportClusterPushTransportBeginEvent(
    Integer connectionID,
    Integer triggerType,
    Optional<Integer> activationReason
  ) {
    this.connectionID = connectionID;
    this.triggerType = triggerType;
    this.activationReason = activationReason;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(CONNECTION_ID_ID, new UIntType(connectionID)));
    values.add(new StructElement(TRIGGER_TYPE_ID, new UIntType(triggerType)));
    values.add(new StructElement(ACTIVATION_REASON_ID, activationReason.<BaseTLVType>map((nonOptionalactivationReason) -> new UIntType(nonOptionalactivationReason)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static PushAvStreamTransportClusterPushTransportBeginEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer connectionID = null;
    Integer triggerType = null;
    Optional<Integer> activationReason = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == CONNECTION_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          connectionID = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == TRIGGER_TYPE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          triggerType = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == ACTIVATION_REASON_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          activationReason = Optional.of(castingValue.value(Integer.class));
        }
      }
    }
    return new PushAvStreamTransportClusterPushTransportBeginEvent(
      connectionID,
      triggerType,
      activationReason
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("PushAvStreamTransportClusterPushTransportBeginEvent {\n");
    output.append("\tconnectionID: ");
    output.append(connectionID);
    output.append("\n");
    output.append("\ttriggerType: ");
    output.append(triggerType);
    output.append("\n");
    output.append("\tactivationReason: ");
    output.append(activationReason);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class PushAvStreamTransportClusterPushTransportEndEvent {
  public Integer connectionID;
  public Integer triggerType;
  public Optional<Integer> activationReason;
  private static final long CONNECTION_ID_ID = 0L;
  private static final long TRIGGER_TYPE_ID = 1L;
  private static final long ACTIVATION_REASON_ID = 2L;

  public PushAvStreamTransportClusterPushTransportEndEvent(
    Integer connectionID,
    Integer triggerType,
    Optional<Integer> activationReason
  ) {
    this.connectionID = connectionID;
    this.triggerType = triggerType;
    this.activationReason = activationReason;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(CONNECTION_ID_ID, new UIntType(connectionID)));
    values.add(new StructElement(TRIGGER_TYPE_ID, new UIntType(triggerType)));
    values.add(new StructElement(ACTIVATION_REASON_ID, activationReason.<BaseTLVType>map((nonOptionalactivationReason) -> new UIntType(nonOptionalactivationReason)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static PushAvStreamTransportClusterPushTransportEndEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer connectionID = null;
    Integer triggerType = null;
    Optional<Integer> activationReason = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == CONNECTION_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          connectionID = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == TRIGGER_TYPE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          triggerType = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == ACTIVATION_REASON_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          activationReason = Optional.of(castingValue.value(Integer.class));
        }
      }
    }
    return new PushAvStreamTransportClusterPushTransportEndEvent(
      connectionID,
      triggerType,
      activationReason
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("PushAvStreamTransportClusterPushTransportEndEvent {\n");
    output.append("\tconnectionID: ");
    output.append(connectionID);
    output.append("\n");
    output.append("\ttriggerType: ");
    output.append(triggerType);
    output.append("\n");
    output.append("\tactivationReason: ");
    output.append(activationReason);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class CommissionerControlClusterCommissioningRequestResultEvent {
  public Long requestID;
  public Long clientNodeID;
  public Integer statusCode;
  public Integer fabricIndex;
  private static final long REQUEST_ID_ID = 0L;
  private static final long CLIENT_NODE_ID_ID = 1L;
  private static final long STATUS_CODE_ID = 2L;
  private static final long FABRIC_INDEX_ID = 254L;

  public CommissionerControlClusterCommissioningRequestResultEvent(
    Long requestID,
    Long clientNodeID,
    Integer statusCode,
    Integer fabricIndex
  ) {
    this.requestID = requestID;
    this.clientNodeID = clientNodeID;
    this.statusCode = statusCode;
    this.fabricIndex = fabricIndex;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(REQUEST_ID_ID, new UIntType(requestID)));
    values.add(new StructElement(CLIENT_NODE_ID_ID, new UIntType(clientNodeID)));
    values.add(new StructElement(STATUS_CODE_ID, new UIntType(statusCode)));
    values.add(new StructElement(FABRIC_INDEX_ID, new UIntType(fabricIndex)));

    return new StructType(values);
  }

  public static CommissionerControlClusterCommissioningRequestResultEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long requestID = null;
    Long clientNodeID = null;
    Integer statusCode = null;
    Integer fabricIndex = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == REQUEST_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          requestID = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == CLIENT_NODE_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          clientNodeID = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == STATUS_CODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          statusCode = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == FABRIC_INDEX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          fabricIndex = castingValue.value(Integer.class);
        }
      }
    }
    return new CommissionerControlClusterCommissioningRequestResultEvent(
      requestID,
      clientNodeID,
      statusCode,
      fabricIndex
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("CommissionerControlClusterCommissioningRequestResultEvent {\n");
    output.append("\trequestID: ");
    output.append(requestID);
    output.append("\n");
    output.append("\tclientNodeID: ");
    output.append(clientNodeID);
    output.append("\n");
    output.append("\tstatusCode: ");
    output.append(statusCode);
    output.append("\n");
    output.append("\tfabricIndex: ");
    output.append(fabricIndex);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class UnitTestingClusterTestEventEvent {
  public Integer arg1;
  public Integer arg2;
  public Boolean arg3;
  public ChipStructs.UnitTestingClusterSimpleStruct arg4;
  public ArrayList<ChipStructs.UnitTestingClusterSimpleStruct> arg5;
  public ArrayList<Integer> arg6;
  private static final long ARG1_ID = 1L;
  private static final long ARG2_ID = 2L;
  private static final long ARG3_ID = 3L;
  private static final long ARG4_ID = 4L;
  private static final long ARG5_ID = 5L;
  private static final long ARG6_ID = 6L;

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

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ARG1_ID, new UIntType(arg1)));
    values.add(new StructElement(ARG2_ID, new UIntType(arg2)));
    values.add(new StructElement(ARG3_ID, new BooleanType(arg3)));
    values.add(new StructElement(ARG4_ID, arg4.encodeTlv()));
    values.add(new StructElement(ARG5_ID, ArrayType.generateArrayType(arg5, (elementarg5) -> elementarg5.encodeTlv())));
    values.add(new StructElement(ARG6_ID, ArrayType.generateArrayType(arg6, (elementarg6) -> new UIntType(elementarg6))));

    return new StructType(values);
  }

  public static UnitTestingClusterTestEventEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer arg1 = null;
    Integer arg2 = null;
    Boolean arg3 = null;
    ChipStructs.UnitTestingClusterSimpleStruct arg4 = null;
    ArrayList<ChipStructs.UnitTestingClusterSimpleStruct> arg5 = null;
    ArrayList<Integer> arg6 = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ARG1_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          arg1 = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == ARG2_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          arg2 = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == ARG3_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          arg3 = castingValue.value(Boolean.class);
        }
      } else if (element.contextTagNum() == ARG4_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          arg4 = ChipStructs.UnitTestingClusterSimpleStruct.decodeTlv(castingValue);
        }
      } else if (element.contextTagNum() == ARG5_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          arg5 = castingValue.map((elementcastingValue) -> ChipStructs.UnitTestingClusterSimpleStruct.decodeTlv(elementcastingValue));
        }
      } else if (element.contextTagNum() == ARG6_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          arg6 = castingValue.map((elementcastingValue) -> elementcastingValue.value(Integer.class));
        }
      }
    }
    return new UnitTestingClusterTestEventEvent(
      arg1,
      arg2,
      arg3,
      arg4,
      arg5,
      arg6
    );
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
}
public static class UnitTestingClusterTestFabricScopedEventEvent {
  public Integer fabricIndex;
  private static final long FABRIC_INDEX_ID = 254L;

  public UnitTestingClusterTestFabricScopedEventEvent(
    Integer fabricIndex
  ) {
    this.fabricIndex = fabricIndex;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(FABRIC_INDEX_ID, new UIntType(fabricIndex)));

    return new StructType(values);
  }

  public static UnitTestingClusterTestFabricScopedEventEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer fabricIndex = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == FABRIC_INDEX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          fabricIndex = castingValue.value(Integer.class);
        }
      }
    }
    return new UnitTestingClusterTestFabricScopedEventEvent(
      fabricIndex
    );
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
}
public static class UnitTestingClusterTestDifferentVendorMeiEventEvent {
  public Integer arg1;
  private static final long ARG1_ID = 1L;

  public UnitTestingClusterTestDifferentVendorMeiEventEvent(
    Integer arg1
  ) {
    this.arg1 = arg1;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ARG1_ID, new UIntType(arg1)));

    return new StructType(values);
  }

  public static UnitTestingClusterTestDifferentVendorMeiEventEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer arg1 = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ARG1_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          arg1 = castingValue.value(Integer.class);
        }
      }
    }
    return new UnitTestingClusterTestDifferentVendorMeiEventEvent(
      arg1
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("UnitTestingClusterTestDifferentVendorMeiEventEvent {\n");
    output.append("\targ1: ");
    output.append(arg1);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class SampleMeiClusterPingCountEventEvent {
  public Long count;
  public Integer fabricIndex;
  private static final long COUNT_ID = 1L;
  private static final long FABRIC_INDEX_ID = 254L;

  public SampleMeiClusterPingCountEventEvent(
    Long count,
    Integer fabricIndex
  ) {
    this.count = count;
    this.fabricIndex = fabricIndex;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(COUNT_ID, new UIntType(count)));
    values.add(new StructElement(FABRIC_INDEX_ID, new UIntType(fabricIndex)));

    return new StructType(values);
  }

  public static SampleMeiClusterPingCountEventEvent decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long count = null;
    Integer fabricIndex = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == COUNT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          count = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == FABRIC_INDEX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          fabricIndex = castingValue.value(Integer.class);
        }
      }
    }
    return new SampleMeiClusterPingCountEventEvent(
      count,
      fabricIndex
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("SampleMeiClusterPingCountEventEvent {\n");
    output.append("\tcount: ");
    output.append(count);
    output.append("\n");
    output.append("\tfabricIndex: ");
    output.append(fabricIndex);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
}
