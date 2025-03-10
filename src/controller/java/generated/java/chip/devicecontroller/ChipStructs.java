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

public class ChipStructs {
public static class DescriptorClusterDeviceTypeStruct {
  public Long deviceType;
  public Integer revision;
  private static final long DEVICE_TYPE_ID = 0L;
  private static final long REVISION_ID = 1L;

  public DescriptorClusterDeviceTypeStruct(
    Long deviceType,
    Integer revision
  ) {
    this.deviceType = deviceType;
    this.revision = revision;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(DEVICE_TYPE_ID, new UIntType(deviceType)));
    values.add(new StructElement(REVISION_ID, new UIntType(revision)));

    return new StructType(values);
  }

  public static DescriptorClusterDeviceTypeStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long deviceType = null;
    Integer revision = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == DEVICE_TYPE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          deviceType = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == REVISION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          revision = castingValue.value(Integer.class);
        }
      }
    }
    return new DescriptorClusterDeviceTypeStruct(
      deviceType,
      revision
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("DescriptorClusterDeviceTypeStruct {\n");
    output.append("\tdeviceType: ");
    output.append(deviceType);
    output.append("\n");
    output.append("\trevision: ");
    output.append(revision);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class DescriptorClusterSemanticTagStruct {
  public @Nullable Integer mfgCode;
  public Integer namespaceID;
  public Integer tag;
  public @Nullable Optional<String> label;
  private static final long MFG_CODE_ID = 0L;
  private static final long NAMESPACE_ID_ID = 1L;
  private static final long TAG_ID = 2L;
  private static final long LABEL_ID = 3L;

  public DescriptorClusterSemanticTagStruct(
    @Nullable Integer mfgCode,
    Integer namespaceID,
    Integer tag,
    @Nullable Optional<String> label
  ) {
    this.mfgCode = mfgCode;
    this.namespaceID = namespaceID;
    this.tag = tag;
    this.label = label;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(MFG_CODE_ID, mfgCode != null ? new UIntType(mfgCode) : new NullType()));
    values.add(new StructElement(NAMESPACE_ID_ID, new UIntType(namespaceID)));
    values.add(new StructElement(TAG_ID, new UIntType(tag)));
    values.add(new StructElement(LABEL_ID, label != null ? label.<BaseTLVType>map((nonOptionallabel) -> new StringType(nonOptionallabel)).orElse(new EmptyType()) : new NullType()));

    return new StructType(values);
  }

  public static DescriptorClusterSemanticTagStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    @Nullable Integer mfgCode = null;
    Integer namespaceID = null;
    Integer tag = null;
    @Nullable Optional<String> label = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == MFG_CODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          mfgCode = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == NAMESPACE_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          namespaceID = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == TAG_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          tag = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == LABEL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          label = Optional.of(castingValue.value(String.class));
        }
      }
    }
    return new DescriptorClusterSemanticTagStruct(
      mfgCode,
      namespaceID,
      tag,
      label
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("DescriptorClusterSemanticTagStruct {\n");
    output.append("\tmfgCode: ");
    output.append(mfgCode);
    output.append("\n");
    output.append("\tnamespaceID: ");
    output.append(namespaceID);
    output.append("\n");
    output.append("\ttag: ");
    output.append(tag);
    output.append("\n");
    output.append("\tlabel: ");
    output.append(label);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class BindingClusterTargetStruct {
  public Optional<Long> node;
  public Optional<Integer> group;
  public Optional<Integer> endpoint;
  public Optional<Long> cluster;
  public Integer fabricIndex;
  private static final long NODE_ID = 1L;
  private static final long GROUP_ID = 2L;
  private static final long ENDPOINT_ID = 3L;
  private static final long CLUSTER_ID = 4L;
  private static final long FABRIC_INDEX_ID = 254L;

  public BindingClusterTargetStruct(
    Optional<Long> node,
    Optional<Integer> group,
    Optional<Integer> endpoint,
    Optional<Long> cluster,
    Integer fabricIndex
  ) {
    this.node = node;
    this.group = group;
    this.endpoint = endpoint;
    this.cluster = cluster;
    this.fabricIndex = fabricIndex;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(NODE_ID, node.<BaseTLVType>map((nonOptionalnode) -> new UIntType(nonOptionalnode)).orElse(new EmptyType())));
    values.add(new StructElement(GROUP_ID, group.<BaseTLVType>map((nonOptionalgroup) -> new UIntType(nonOptionalgroup)).orElse(new EmptyType())));
    values.add(new StructElement(ENDPOINT_ID, endpoint.<BaseTLVType>map((nonOptionalendpoint) -> new UIntType(nonOptionalendpoint)).orElse(new EmptyType())));
    values.add(new StructElement(CLUSTER_ID, cluster.<BaseTLVType>map((nonOptionalcluster) -> new UIntType(nonOptionalcluster)).orElse(new EmptyType())));
    values.add(new StructElement(FABRIC_INDEX_ID, new UIntType(fabricIndex)));

    return new StructType(values);
  }

  public static BindingClusterTargetStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Optional<Long> node = Optional.empty();
    Optional<Integer> group = Optional.empty();
    Optional<Integer> endpoint = Optional.empty();
    Optional<Long> cluster = Optional.empty();
    Integer fabricIndex = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == NODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          node = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == GROUP_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          group = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == ENDPOINT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          endpoint = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == CLUSTER_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          cluster = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == FABRIC_INDEX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          fabricIndex = castingValue.value(Integer.class);
        }
      }
    }
    return new BindingClusterTargetStruct(
      node,
      group,
      endpoint,
      cluster,
      fabricIndex
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("BindingClusterTargetStruct {\n");
    output.append("\tnode: ");
    output.append(node);
    output.append("\n");
    output.append("\tgroup: ");
    output.append(group);
    output.append("\n");
    output.append("\tendpoint: ");
    output.append(endpoint);
    output.append("\n");
    output.append("\tcluster: ");
    output.append(cluster);
    output.append("\n");
    output.append("\tfabricIndex: ");
    output.append(fabricIndex);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class AccessControlClusterAccessRestrictionStruct {
  public Integer type;
  public @Nullable Long id;
  private static final long TYPE_ID = 0L;
  private static final long ID_ID = 1L;

  public AccessControlClusterAccessRestrictionStruct(
    Integer type,
    @Nullable Long id
  ) {
    this.type = type;
    this.id = id;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(TYPE_ID, new UIntType(type)));
    values.add(new StructElement(ID_ID, id != null ? new UIntType(id) : new NullType()));

    return new StructType(values);
  }

  public static AccessControlClusterAccessRestrictionStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer type = null;
    @Nullable Long id = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == TYPE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          type = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          id = castingValue.value(Long.class);
        }
      }
    }
    return new AccessControlClusterAccessRestrictionStruct(
      type,
      id
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("AccessControlClusterAccessRestrictionStruct {\n");
    output.append("\ttype: ");
    output.append(type);
    output.append("\n");
    output.append("\tid: ");
    output.append(id);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class AccessControlClusterCommissioningAccessRestrictionEntryStruct {
  public Integer endpoint;
  public Long cluster;
  public ArrayList<ChipStructs.AccessControlClusterAccessRestrictionStruct> restrictions;
  private static final long ENDPOINT_ID = 0L;
  private static final long CLUSTER_ID = 1L;
  private static final long RESTRICTIONS_ID = 2L;

  public AccessControlClusterCommissioningAccessRestrictionEntryStruct(
    Integer endpoint,
    Long cluster,
    ArrayList<ChipStructs.AccessControlClusterAccessRestrictionStruct> restrictions
  ) {
    this.endpoint = endpoint;
    this.cluster = cluster;
    this.restrictions = restrictions;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ENDPOINT_ID, new UIntType(endpoint)));
    values.add(new StructElement(CLUSTER_ID, new UIntType(cluster)));
    values.add(new StructElement(RESTRICTIONS_ID, ArrayType.generateArrayType(restrictions, (elementrestrictions) -> elementrestrictions.encodeTlv())));

    return new StructType(values);
  }

  public static AccessControlClusterCommissioningAccessRestrictionEntryStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer endpoint = null;
    Long cluster = null;
    ArrayList<ChipStructs.AccessControlClusterAccessRestrictionStruct> restrictions = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ENDPOINT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          endpoint = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == CLUSTER_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          cluster = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == RESTRICTIONS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          restrictions = castingValue.map((elementcastingValue) -> ChipStructs.AccessControlClusterAccessRestrictionStruct.decodeTlv(elementcastingValue));
        }
      }
    }
    return new AccessControlClusterCommissioningAccessRestrictionEntryStruct(
      endpoint,
      cluster,
      restrictions
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("AccessControlClusterCommissioningAccessRestrictionEntryStruct {\n");
    output.append("\tendpoint: ");
    output.append(endpoint);
    output.append("\n");
    output.append("\tcluster: ");
    output.append(cluster);
    output.append("\n");
    output.append("\trestrictions: ");
    output.append(restrictions);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class AccessControlClusterAccessRestrictionEntryStruct {
  public Integer endpoint;
  public Long cluster;
  public ArrayList<ChipStructs.AccessControlClusterAccessRestrictionStruct> restrictions;
  public Integer fabricIndex;
  private static final long ENDPOINT_ID = 0L;
  private static final long CLUSTER_ID = 1L;
  private static final long RESTRICTIONS_ID = 2L;
  private static final long FABRIC_INDEX_ID = 254L;

  public AccessControlClusterAccessRestrictionEntryStruct(
    Integer endpoint,
    Long cluster,
    ArrayList<ChipStructs.AccessControlClusterAccessRestrictionStruct> restrictions,
    Integer fabricIndex
  ) {
    this.endpoint = endpoint;
    this.cluster = cluster;
    this.restrictions = restrictions;
    this.fabricIndex = fabricIndex;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ENDPOINT_ID, new UIntType(endpoint)));
    values.add(new StructElement(CLUSTER_ID, new UIntType(cluster)));
    values.add(new StructElement(RESTRICTIONS_ID, ArrayType.generateArrayType(restrictions, (elementrestrictions) -> elementrestrictions.encodeTlv())));
    values.add(new StructElement(FABRIC_INDEX_ID, new UIntType(fabricIndex)));

    return new StructType(values);
  }

  public static AccessControlClusterAccessRestrictionEntryStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer endpoint = null;
    Long cluster = null;
    ArrayList<ChipStructs.AccessControlClusterAccessRestrictionStruct> restrictions = null;
    Integer fabricIndex = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ENDPOINT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          endpoint = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == CLUSTER_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          cluster = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == RESTRICTIONS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          restrictions = castingValue.map((elementcastingValue) -> ChipStructs.AccessControlClusterAccessRestrictionStruct.decodeTlv(elementcastingValue));
        }
      } else if (element.contextTagNum() == FABRIC_INDEX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          fabricIndex = castingValue.value(Integer.class);
        }
      }
    }
    return new AccessControlClusterAccessRestrictionEntryStruct(
      endpoint,
      cluster,
      restrictions,
      fabricIndex
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("AccessControlClusterAccessRestrictionEntryStruct {\n");
    output.append("\tendpoint: ");
    output.append(endpoint);
    output.append("\n");
    output.append("\tcluster: ");
    output.append(cluster);
    output.append("\n");
    output.append("\trestrictions: ");
    output.append(restrictions);
    output.append("\n");
    output.append("\tfabricIndex: ");
    output.append(fabricIndex);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class AccessControlClusterAccessControlTargetStruct {
  public @Nullable Long cluster;
  public @Nullable Integer endpoint;
  public @Nullable Long deviceType;
  private static final long CLUSTER_ID = 0L;
  private static final long ENDPOINT_ID = 1L;
  private static final long DEVICE_TYPE_ID = 2L;

  public AccessControlClusterAccessControlTargetStruct(
    @Nullable Long cluster,
    @Nullable Integer endpoint,
    @Nullable Long deviceType
  ) {
    this.cluster = cluster;
    this.endpoint = endpoint;
    this.deviceType = deviceType;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(CLUSTER_ID, cluster != null ? new UIntType(cluster) : new NullType()));
    values.add(new StructElement(ENDPOINT_ID, endpoint != null ? new UIntType(endpoint) : new NullType()));
    values.add(new StructElement(DEVICE_TYPE_ID, deviceType != null ? new UIntType(deviceType) : new NullType()));

    return new StructType(values);
  }

  public static AccessControlClusterAccessControlTargetStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    @Nullable Long cluster = null;
    @Nullable Integer endpoint = null;
    @Nullable Long deviceType = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == CLUSTER_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          cluster = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == ENDPOINT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          endpoint = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == DEVICE_TYPE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          deviceType = castingValue.value(Long.class);
        }
      }
    }
    return new AccessControlClusterAccessControlTargetStruct(
      cluster,
      endpoint,
      deviceType
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("AccessControlClusterAccessControlTargetStruct {\n");
    output.append("\tcluster: ");
    output.append(cluster);
    output.append("\n");
    output.append("\tendpoint: ");
    output.append(endpoint);
    output.append("\n");
    output.append("\tdeviceType: ");
    output.append(deviceType);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class AccessControlClusterAccessControlEntryStruct {
  public Integer privilege;
  public Integer authMode;
  public @Nullable ArrayList<Long> subjects;
  public @Nullable ArrayList<ChipStructs.AccessControlClusterAccessControlTargetStruct> targets;
  public Integer fabricIndex;
  private static final long PRIVILEGE_ID = 1L;
  private static final long AUTH_MODE_ID = 2L;
  private static final long SUBJECTS_ID = 3L;
  private static final long TARGETS_ID = 4L;
  private static final long FABRIC_INDEX_ID = 254L;

  public AccessControlClusterAccessControlEntryStruct(
    Integer privilege,
    Integer authMode,
    @Nullable ArrayList<Long> subjects,
    @Nullable ArrayList<ChipStructs.AccessControlClusterAccessControlTargetStruct> targets,
    Integer fabricIndex
  ) {
    this.privilege = privilege;
    this.authMode = authMode;
    this.subjects = subjects;
    this.targets = targets;
    this.fabricIndex = fabricIndex;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(PRIVILEGE_ID, new UIntType(privilege)));
    values.add(new StructElement(AUTH_MODE_ID, new UIntType(authMode)));
    values.add(new StructElement(SUBJECTS_ID, subjects != null ? ArrayType.generateArrayType(subjects, (elementsubjects) -> new UIntType(elementsubjects)) : new NullType()));
    values.add(new StructElement(TARGETS_ID, targets != null ? ArrayType.generateArrayType(targets, (elementtargets) -> elementtargets.encodeTlv()) : new NullType()));
    values.add(new StructElement(FABRIC_INDEX_ID, new UIntType(fabricIndex)));

    return new StructType(values);
  }

  public static AccessControlClusterAccessControlEntryStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer privilege = null;
    Integer authMode = null;
    @Nullable ArrayList<Long> subjects = null;
    @Nullable ArrayList<ChipStructs.AccessControlClusterAccessControlTargetStruct> targets = null;
    Integer fabricIndex = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == PRIVILEGE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          privilege = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == AUTH_MODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          authMode = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == SUBJECTS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          subjects = castingValue.map((elementcastingValue) -> elementcastingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == TARGETS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          targets = castingValue.map((elementcastingValue) -> ChipStructs.AccessControlClusterAccessControlTargetStruct.decodeTlv(elementcastingValue));
        }
      } else if (element.contextTagNum() == FABRIC_INDEX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          fabricIndex = castingValue.value(Integer.class);
        }
      }
    }
    return new AccessControlClusterAccessControlEntryStruct(
      privilege,
      authMode,
      subjects,
      targets,
      fabricIndex
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("AccessControlClusterAccessControlEntryStruct {\n");
    output.append("\tprivilege: ");
    output.append(privilege);
    output.append("\n");
    output.append("\tauthMode: ");
    output.append(authMode);
    output.append("\n");
    output.append("\tsubjects: ");
    output.append(subjects);
    output.append("\n");
    output.append("\ttargets: ");
    output.append(targets);
    output.append("\n");
    output.append("\tfabricIndex: ");
    output.append(fabricIndex);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class AccessControlClusterAccessControlExtensionStruct {
  public byte[] data;
  public Integer fabricIndex;
  private static final long DATA_ID = 1L;
  private static final long FABRIC_INDEX_ID = 254L;

  public AccessControlClusterAccessControlExtensionStruct(
    byte[] data,
    Integer fabricIndex
  ) {
    this.data = data;
    this.fabricIndex = fabricIndex;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(DATA_ID, new ByteArrayType(data)));
    values.add(new StructElement(FABRIC_INDEX_ID, new UIntType(fabricIndex)));

    return new StructType(values);
  }

  public static AccessControlClusterAccessControlExtensionStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    byte[] data = null;
    Integer fabricIndex = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == DATA_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.ByteArray) {
          ByteArrayType castingValue = element.value(ByteArrayType.class);
          data = castingValue.value(byte[].class);
        }
      } else if (element.contextTagNum() == FABRIC_INDEX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          fabricIndex = castingValue.value(Integer.class);
        }
      }
    }
    return new AccessControlClusterAccessControlExtensionStruct(
      data,
      fabricIndex
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("AccessControlClusterAccessControlExtensionStruct {\n");
    output.append("\tdata: ");
    output.append(Arrays.toString(data));
    output.append("\n");
    output.append("\tfabricIndex: ");
    output.append(fabricIndex);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ActionsClusterActionStruct {
  public Integer actionID;
  public String name;
  public Integer type;
  public Integer endpointListID;
  public Integer supportedCommands;
  public Integer state;
  private static final long ACTION_ID_ID = 0L;
  private static final long NAME_ID = 1L;
  private static final long TYPE_ID = 2L;
  private static final long ENDPOINT_LIST_ID_ID = 3L;
  private static final long SUPPORTED_COMMANDS_ID = 4L;
  private static final long STATE_ID = 5L;

  public ActionsClusterActionStruct(
    Integer actionID,
    String name,
    Integer type,
    Integer endpointListID,
    Integer supportedCommands,
    Integer state
  ) {
    this.actionID = actionID;
    this.name = name;
    this.type = type;
    this.endpointListID = endpointListID;
    this.supportedCommands = supportedCommands;
    this.state = state;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ACTION_ID_ID, new UIntType(actionID)));
    values.add(new StructElement(NAME_ID, new StringType(name)));
    values.add(new StructElement(TYPE_ID, new UIntType(type)));
    values.add(new StructElement(ENDPOINT_LIST_ID_ID, new UIntType(endpointListID)));
    values.add(new StructElement(SUPPORTED_COMMANDS_ID, new UIntType(supportedCommands)));
    values.add(new StructElement(STATE_ID, new UIntType(state)));

    return new StructType(values);
  }

  public static ActionsClusterActionStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer actionID = null;
    String name = null;
    Integer type = null;
    Integer endpointListID = null;
    Integer supportedCommands = null;
    Integer state = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ACTION_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          actionID = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == NAME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          name = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == TYPE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          type = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == ENDPOINT_LIST_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          endpointListID = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == SUPPORTED_COMMANDS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          supportedCommands = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == STATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          state = castingValue.value(Integer.class);
        }
      }
    }
    return new ActionsClusterActionStruct(
      actionID,
      name,
      type,
      endpointListID,
      supportedCommands,
      state
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ActionsClusterActionStruct {\n");
    output.append("\tactionID: ");
    output.append(actionID);
    output.append("\n");
    output.append("\tname: ");
    output.append(name);
    output.append("\n");
    output.append("\ttype: ");
    output.append(type);
    output.append("\n");
    output.append("\tendpointListID: ");
    output.append(endpointListID);
    output.append("\n");
    output.append("\tsupportedCommands: ");
    output.append(supportedCommands);
    output.append("\n");
    output.append("\tstate: ");
    output.append(state);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ActionsClusterEndpointListStruct {
  public Integer endpointListID;
  public String name;
  public Integer type;
  public ArrayList<Integer> endpoints;
  private static final long ENDPOINT_LIST_ID_ID = 0L;
  private static final long NAME_ID = 1L;
  private static final long TYPE_ID = 2L;
  private static final long ENDPOINTS_ID = 3L;

  public ActionsClusterEndpointListStruct(
    Integer endpointListID,
    String name,
    Integer type,
    ArrayList<Integer> endpoints
  ) {
    this.endpointListID = endpointListID;
    this.name = name;
    this.type = type;
    this.endpoints = endpoints;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ENDPOINT_LIST_ID_ID, new UIntType(endpointListID)));
    values.add(new StructElement(NAME_ID, new StringType(name)));
    values.add(new StructElement(TYPE_ID, new UIntType(type)));
    values.add(new StructElement(ENDPOINTS_ID, ArrayType.generateArrayType(endpoints, (elementendpoints) -> new UIntType(elementendpoints))));

    return new StructType(values);
  }

  public static ActionsClusterEndpointListStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer endpointListID = null;
    String name = null;
    Integer type = null;
    ArrayList<Integer> endpoints = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ENDPOINT_LIST_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          endpointListID = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == NAME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          name = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == TYPE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          type = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == ENDPOINTS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          endpoints = castingValue.map((elementcastingValue) -> elementcastingValue.value(Integer.class));
        }
      }
    }
    return new ActionsClusterEndpointListStruct(
      endpointListID,
      name,
      type,
      endpoints
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ActionsClusterEndpointListStruct {\n");
    output.append("\tendpointListID: ");
    output.append(endpointListID);
    output.append("\n");
    output.append("\tname: ");
    output.append(name);
    output.append("\n");
    output.append("\ttype: ");
    output.append(type);
    output.append("\n");
    output.append("\tendpoints: ");
    output.append(endpoints);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class BasicInformationClusterCapabilityMinimaStruct {
  public Integer caseSessionsPerFabric;
  public Integer subscriptionsPerFabric;
  private static final long CASE_SESSIONS_PER_FABRIC_ID = 0L;
  private static final long SUBSCRIPTIONS_PER_FABRIC_ID = 1L;

  public BasicInformationClusterCapabilityMinimaStruct(
    Integer caseSessionsPerFabric,
    Integer subscriptionsPerFabric
  ) {
    this.caseSessionsPerFabric = caseSessionsPerFabric;
    this.subscriptionsPerFabric = subscriptionsPerFabric;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(CASE_SESSIONS_PER_FABRIC_ID, new UIntType(caseSessionsPerFabric)));
    values.add(new StructElement(SUBSCRIPTIONS_PER_FABRIC_ID, new UIntType(subscriptionsPerFabric)));

    return new StructType(values);
  }

  public static BasicInformationClusterCapabilityMinimaStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer caseSessionsPerFabric = null;
    Integer subscriptionsPerFabric = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == CASE_SESSIONS_PER_FABRIC_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          caseSessionsPerFabric = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == SUBSCRIPTIONS_PER_FABRIC_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          subscriptionsPerFabric = castingValue.value(Integer.class);
        }
      }
    }
    return new BasicInformationClusterCapabilityMinimaStruct(
      caseSessionsPerFabric,
      subscriptionsPerFabric
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("BasicInformationClusterCapabilityMinimaStruct {\n");
    output.append("\tcaseSessionsPerFabric: ");
    output.append(caseSessionsPerFabric);
    output.append("\n");
    output.append("\tsubscriptionsPerFabric: ");
    output.append(subscriptionsPerFabric);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class BasicInformationClusterProductAppearanceStruct {
  public Integer finish;
  public @Nullable Integer primaryColor;
  private static final long FINISH_ID = 0L;
  private static final long PRIMARY_COLOR_ID = 1L;

  public BasicInformationClusterProductAppearanceStruct(
    Integer finish,
    @Nullable Integer primaryColor
  ) {
    this.finish = finish;
    this.primaryColor = primaryColor;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(FINISH_ID, new UIntType(finish)));
    values.add(new StructElement(PRIMARY_COLOR_ID, primaryColor != null ? new UIntType(primaryColor) : new NullType()));

    return new StructType(values);
  }

  public static BasicInformationClusterProductAppearanceStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer finish = null;
    @Nullable Integer primaryColor = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == FINISH_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          finish = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == PRIMARY_COLOR_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          primaryColor = castingValue.value(Integer.class);
        }
      }
    }
    return new BasicInformationClusterProductAppearanceStruct(
      finish,
      primaryColor
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("BasicInformationClusterProductAppearanceStruct {\n");
    output.append("\tfinish: ");
    output.append(finish);
    output.append("\n");
    output.append("\tprimaryColor: ");
    output.append(primaryColor);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class OtaSoftwareUpdateRequestorClusterProviderLocation {
  public Long providerNodeID;
  public Integer endpoint;
  public Integer fabricIndex;
  private static final long PROVIDER_NODE_ID_ID = 1L;
  private static final long ENDPOINT_ID = 2L;
  private static final long FABRIC_INDEX_ID = 254L;

  public OtaSoftwareUpdateRequestorClusterProviderLocation(
    Long providerNodeID,
    Integer endpoint,
    Integer fabricIndex
  ) {
    this.providerNodeID = providerNodeID;
    this.endpoint = endpoint;
    this.fabricIndex = fabricIndex;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(PROVIDER_NODE_ID_ID, new UIntType(providerNodeID)));
    values.add(new StructElement(ENDPOINT_ID, new UIntType(endpoint)));
    values.add(new StructElement(FABRIC_INDEX_ID, new UIntType(fabricIndex)));

    return new StructType(values);
  }

  public static OtaSoftwareUpdateRequestorClusterProviderLocation decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long providerNodeID = null;
    Integer endpoint = null;
    Integer fabricIndex = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == PROVIDER_NODE_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          providerNodeID = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == ENDPOINT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          endpoint = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == FABRIC_INDEX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          fabricIndex = castingValue.value(Integer.class);
        }
      }
    }
    return new OtaSoftwareUpdateRequestorClusterProviderLocation(
      providerNodeID,
      endpoint,
      fabricIndex
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("OtaSoftwareUpdateRequestorClusterProviderLocation {\n");
    output.append("\tproviderNodeID: ");
    output.append(providerNodeID);
    output.append("\n");
    output.append("\tendpoint: ");
    output.append(endpoint);
    output.append("\n");
    output.append("\tfabricIndex: ");
    output.append(fabricIndex);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class PowerSourceClusterBatChargeFaultChangeType {
  public ArrayList<Integer> current;
  public ArrayList<Integer> previous;
  private static final long CURRENT_ID = 0L;
  private static final long PREVIOUS_ID = 1L;

  public PowerSourceClusterBatChargeFaultChangeType(
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

  public static PowerSourceClusterBatChargeFaultChangeType decodeTlv(BaseTLVType tlvValue) {
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
    return new PowerSourceClusterBatChargeFaultChangeType(
      current,
      previous
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("PowerSourceClusterBatChargeFaultChangeType {\n");
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
public static class PowerSourceClusterBatFaultChangeType {
  public ArrayList<Integer> current;
  public ArrayList<Integer> previous;
  private static final long CURRENT_ID = 0L;
  private static final long PREVIOUS_ID = 1L;

  public PowerSourceClusterBatFaultChangeType(
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

  public static PowerSourceClusterBatFaultChangeType decodeTlv(BaseTLVType tlvValue) {
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
    return new PowerSourceClusterBatFaultChangeType(
      current,
      previous
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("PowerSourceClusterBatFaultChangeType {\n");
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
public static class PowerSourceClusterWiredFaultChangeType {
  public ArrayList<Integer> current;
  public ArrayList<Integer> previous;
  private static final long CURRENT_ID = 0L;
  private static final long PREVIOUS_ID = 1L;

  public PowerSourceClusterWiredFaultChangeType(
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

  public static PowerSourceClusterWiredFaultChangeType decodeTlv(BaseTLVType tlvValue) {
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
    return new PowerSourceClusterWiredFaultChangeType(
      current,
      previous
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("PowerSourceClusterWiredFaultChangeType {\n");
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
public static class GeneralCommissioningClusterBasicCommissioningInfo {
  public Integer failSafeExpiryLengthSeconds;
  public Integer maxCumulativeFailsafeSeconds;
  private static final long FAIL_SAFE_EXPIRY_LENGTH_SECONDS_ID = 0L;
  private static final long MAX_CUMULATIVE_FAILSAFE_SECONDS_ID = 1L;

  public GeneralCommissioningClusterBasicCommissioningInfo(
    Integer failSafeExpiryLengthSeconds,
    Integer maxCumulativeFailsafeSeconds
  ) {
    this.failSafeExpiryLengthSeconds = failSafeExpiryLengthSeconds;
    this.maxCumulativeFailsafeSeconds = maxCumulativeFailsafeSeconds;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(FAIL_SAFE_EXPIRY_LENGTH_SECONDS_ID, new UIntType(failSafeExpiryLengthSeconds)));
    values.add(new StructElement(MAX_CUMULATIVE_FAILSAFE_SECONDS_ID, new UIntType(maxCumulativeFailsafeSeconds)));

    return new StructType(values);
  }

  public static GeneralCommissioningClusterBasicCommissioningInfo decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer failSafeExpiryLengthSeconds = null;
    Integer maxCumulativeFailsafeSeconds = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == FAIL_SAFE_EXPIRY_LENGTH_SECONDS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          failSafeExpiryLengthSeconds = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == MAX_CUMULATIVE_FAILSAFE_SECONDS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          maxCumulativeFailsafeSeconds = castingValue.value(Integer.class);
        }
      }
    }
    return new GeneralCommissioningClusterBasicCommissioningInfo(
      failSafeExpiryLengthSeconds,
      maxCumulativeFailsafeSeconds
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("GeneralCommissioningClusterBasicCommissioningInfo {\n");
    output.append("\tfailSafeExpiryLengthSeconds: ");
    output.append(failSafeExpiryLengthSeconds);
    output.append("\n");
    output.append("\tmaxCumulativeFailsafeSeconds: ");
    output.append(maxCumulativeFailsafeSeconds);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class NetworkCommissioningClusterNetworkInfoStruct {
  public byte[] networkID;
  public Boolean connected;
  public @Nullable Optional<byte[]> networkIdentifier;
  public @Nullable Optional<byte[]> clientIdentifier;
  private static final long NETWORK_ID_ID = 0L;
  private static final long CONNECTED_ID = 1L;
  private static final long NETWORK_IDENTIFIER_ID = 2L;
  private static final long CLIENT_IDENTIFIER_ID = 3L;

  public NetworkCommissioningClusterNetworkInfoStruct(
    byte[] networkID,
    Boolean connected,
    @Nullable Optional<byte[]> networkIdentifier,
    @Nullable Optional<byte[]> clientIdentifier
  ) {
    this.networkID = networkID;
    this.connected = connected;
    this.networkIdentifier = networkIdentifier;
    this.clientIdentifier = clientIdentifier;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(NETWORK_ID_ID, new ByteArrayType(networkID)));
    values.add(new StructElement(CONNECTED_ID, new BooleanType(connected)));
    values.add(new StructElement(NETWORK_IDENTIFIER_ID, networkIdentifier != null ? networkIdentifier.<BaseTLVType>map((nonOptionalnetworkIdentifier) -> new ByteArrayType(nonOptionalnetworkIdentifier)).orElse(new EmptyType()) : new NullType()));
    values.add(new StructElement(CLIENT_IDENTIFIER_ID, clientIdentifier != null ? clientIdentifier.<BaseTLVType>map((nonOptionalclientIdentifier) -> new ByteArrayType(nonOptionalclientIdentifier)).orElse(new EmptyType()) : new NullType()));

    return new StructType(values);
  }

  public static NetworkCommissioningClusterNetworkInfoStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    byte[] networkID = null;
    Boolean connected = null;
    @Nullable Optional<byte[]> networkIdentifier = null;
    @Nullable Optional<byte[]> clientIdentifier = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == NETWORK_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.ByteArray) {
          ByteArrayType castingValue = element.value(ByteArrayType.class);
          networkID = castingValue.value(byte[].class);
        }
      } else if (element.contextTagNum() == CONNECTED_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          connected = castingValue.value(Boolean.class);
        }
      } else if (element.contextTagNum() == NETWORK_IDENTIFIER_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.ByteArray) {
          ByteArrayType castingValue = element.value(ByteArrayType.class);
          networkIdentifier = Optional.of(castingValue.value(byte[].class));
        }
      } else if (element.contextTagNum() == CLIENT_IDENTIFIER_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.ByteArray) {
          ByteArrayType castingValue = element.value(ByteArrayType.class);
          clientIdentifier = Optional.of(castingValue.value(byte[].class));
        }
      }
    }
    return new NetworkCommissioningClusterNetworkInfoStruct(
      networkID,
      connected,
      networkIdentifier,
      clientIdentifier
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("NetworkCommissioningClusterNetworkInfoStruct {\n");
    output.append("\tnetworkID: ");
    output.append(Arrays.toString(networkID));
    output.append("\n");
    output.append("\tconnected: ");
    output.append(connected);
    output.append("\n");
    output.append("\tnetworkIdentifier: ");
    output.append(networkIdentifier.isPresent() ? Arrays.toString(networkIdentifier.get()) : "");
    output.append("\n");
    output.append("\tclientIdentifier: ");
    output.append(clientIdentifier.isPresent() ? Arrays.toString(clientIdentifier.get()) : "");
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class NetworkCommissioningClusterThreadInterfaceScanResultStruct {
  public Integer panId;
  public Long extendedPanId;
  public String networkName;
  public Integer channel;
  public Integer version;
  public byte[] extendedAddress;
  public Integer rssi;
  public Integer lqi;
  private static final long PAN_ID_ID = 0L;
  private static final long EXTENDED_PAN_ID_ID = 1L;
  private static final long NETWORK_NAME_ID = 2L;
  private static final long CHANNEL_ID = 3L;
  private static final long VERSION_ID = 4L;
  private static final long EXTENDED_ADDRESS_ID = 5L;
  private static final long RSSI_ID = 6L;
  private static final long LQI_ID = 7L;

  public NetworkCommissioningClusterThreadInterfaceScanResultStruct(
    Integer panId,
    Long extendedPanId,
    String networkName,
    Integer channel,
    Integer version,
    byte[] extendedAddress,
    Integer rssi,
    Integer lqi
  ) {
    this.panId = panId;
    this.extendedPanId = extendedPanId;
    this.networkName = networkName;
    this.channel = channel;
    this.version = version;
    this.extendedAddress = extendedAddress;
    this.rssi = rssi;
    this.lqi = lqi;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(PAN_ID_ID, new UIntType(panId)));
    values.add(new StructElement(EXTENDED_PAN_ID_ID, new UIntType(extendedPanId)));
    values.add(new StructElement(NETWORK_NAME_ID, new StringType(networkName)));
    values.add(new StructElement(CHANNEL_ID, new UIntType(channel)));
    values.add(new StructElement(VERSION_ID, new UIntType(version)));
    values.add(new StructElement(EXTENDED_ADDRESS_ID, new ByteArrayType(extendedAddress)));
    values.add(new StructElement(RSSI_ID, new IntType(rssi)));
    values.add(new StructElement(LQI_ID, new UIntType(lqi)));

    return new StructType(values);
  }

  public static NetworkCommissioningClusterThreadInterfaceScanResultStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer panId = null;
    Long extendedPanId = null;
    String networkName = null;
    Integer channel = null;
    Integer version = null;
    byte[] extendedAddress = null;
    Integer rssi = null;
    Integer lqi = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == PAN_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          panId = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == EXTENDED_PAN_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          extendedPanId = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == NETWORK_NAME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          networkName = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == CHANNEL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          channel = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == VERSION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          version = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == EXTENDED_ADDRESS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.ByteArray) {
          ByteArrayType castingValue = element.value(ByteArrayType.class);
          extendedAddress = castingValue.value(byte[].class);
        }
      } else if (element.contextTagNum() == RSSI_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          rssi = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == LQI_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          lqi = castingValue.value(Integer.class);
        }
      }
    }
    return new NetworkCommissioningClusterThreadInterfaceScanResultStruct(
      panId,
      extendedPanId,
      networkName,
      channel,
      version,
      extendedAddress,
      rssi,
      lqi
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("NetworkCommissioningClusterThreadInterfaceScanResultStruct {\n");
    output.append("\tpanId: ");
    output.append(panId);
    output.append("\n");
    output.append("\textendedPanId: ");
    output.append(extendedPanId);
    output.append("\n");
    output.append("\tnetworkName: ");
    output.append(networkName);
    output.append("\n");
    output.append("\tchannel: ");
    output.append(channel);
    output.append("\n");
    output.append("\tversion: ");
    output.append(version);
    output.append("\n");
    output.append("\textendedAddress: ");
    output.append(Arrays.toString(extendedAddress));
    output.append("\n");
    output.append("\trssi: ");
    output.append(rssi);
    output.append("\n");
    output.append("\tlqi: ");
    output.append(lqi);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class NetworkCommissioningClusterWiFiInterfaceScanResultStruct {
  public Integer security;
  public byte[] ssid;
  public byte[] bssid;
  public Integer channel;
  public Integer wiFiBand;
  public Integer rssi;
  private static final long SECURITY_ID = 0L;
  private static final long SSID_ID = 1L;
  private static final long BSSID_ID = 2L;
  private static final long CHANNEL_ID = 3L;
  private static final long WI_FI_BAND_ID = 4L;
  private static final long RSSI_ID = 5L;

  public NetworkCommissioningClusterWiFiInterfaceScanResultStruct(
    Integer security,
    byte[] ssid,
    byte[] bssid,
    Integer channel,
    Integer wiFiBand,
    Integer rssi
  ) {
    this.security = security;
    this.ssid = ssid;
    this.bssid = bssid;
    this.channel = channel;
    this.wiFiBand = wiFiBand;
    this.rssi = rssi;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(SECURITY_ID, new UIntType(security)));
    values.add(new StructElement(SSID_ID, new ByteArrayType(ssid)));
    values.add(new StructElement(BSSID_ID, new ByteArrayType(bssid)));
    values.add(new StructElement(CHANNEL_ID, new UIntType(channel)));
    values.add(new StructElement(WI_FI_BAND_ID, new UIntType(wiFiBand)));
    values.add(new StructElement(RSSI_ID, new IntType(rssi)));

    return new StructType(values);
  }

  public static NetworkCommissioningClusterWiFiInterfaceScanResultStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer security = null;
    byte[] ssid = null;
    byte[] bssid = null;
    Integer channel = null;
    Integer wiFiBand = null;
    Integer rssi = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == SECURITY_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          security = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == SSID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.ByteArray) {
          ByteArrayType castingValue = element.value(ByteArrayType.class);
          ssid = castingValue.value(byte[].class);
        }
      } else if (element.contextTagNum() == BSSID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.ByteArray) {
          ByteArrayType castingValue = element.value(ByteArrayType.class);
          bssid = castingValue.value(byte[].class);
        }
      } else if (element.contextTagNum() == CHANNEL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          channel = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == WI_FI_BAND_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          wiFiBand = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == RSSI_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          rssi = castingValue.value(Integer.class);
        }
      }
    }
    return new NetworkCommissioningClusterWiFiInterfaceScanResultStruct(
      security,
      ssid,
      bssid,
      channel,
      wiFiBand,
      rssi
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("NetworkCommissioningClusterWiFiInterfaceScanResultStruct {\n");
    output.append("\tsecurity: ");
    output.append(security);
    output.append("\n");
    output.append("\tssid: ");
    output.append(Arrays.toString(ssid));
    output.append("\n");
    output.append("\tbssid: ");
    output.append(Arrays.toString(bssid));
    output.append("\n");
    output.append("\tchannel: ");
    output.append(channel);
    output.append("\n");
    output.append("\twiFiBand: ");
    output.append(wiFiBand);
    output.append("\n");
    output.append("\trssi: ");
    output.append(rssi);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class GeneralDiagnosticsClusterNetworkInterface {
  public String name;
  public Boolean isOperational;
  public @Nullable Boolean offPremiseServicesReachableIPv4;
  public @Nullable Boolean offPremiseServicesReachableIPv6;
  public byte[] hardwareAddress;
  public ArrayList<byte[]> IPv4Addresses;
  public ArrayList<byte[]> IPv6Addresses;
  public Integer type;
  private static final long NAME_ID = 0L;
  private static final long IS_OPERATIONAL_ID = 1L;
  private static final long OFF_PREMISE_SERVICES_REACHABLE_I_PV4_ID = 2L;
  private static final long OFF_PREMISE_SERVICES_REACHABLE_I_PV6_ID = 3L;
  private static final long HARDWARE_ADDRESS_ID = 4L;
  private static final long I_PV4_ADDRESSES_ID = 5L;
  private static final long I_PV6_ADDRESSES_ID = 6L;
  private static final long TYPE_ID = 7L;

  public GeneralDiagnosticsClusterNetworkInterface(
    String name,
    Boolean isOperational,
    @Nullable Boolean offPremiseServicesReachableIPv4,
    @Nullable Boolean offPremiseServicesReachableIPv6,
    byte[] hardwareAddress,
    ArrayList<byte[]> IPv4Addresses,
    ArrayList<byte[]> IPv6Addresses,
    Integer type
  ) {
    this.name = name;
    this.isOperational = isOperational;
    this.offPremiseServicesReachableIPv4 = offPremiseServicesReachableIPv4;
    this.offPremiseServicesReachableIPv6 = offPremiseServicesReachableIPv6;
    this.hardwareAddress = hardwareAddress;
    this.IPv4Addresses = IPv4Addresses;
    this.IPv6Addresses = IPv6Addresses;
    this.type = type;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(NAME_ID, new StringType(name)));
    values.add(new StructElement(IS_OPERATIONAL_ID, new BooleanType(isOperational)));
    values.add(new StructElement(OFF_PREMISE_SERVICES_REACHABLE_I_PV4_ID, offPremiseServicesReachableIPv4 != null ? new BooleanType(offPremiseServicesReachableIPv4) : new NullType()));
    values.add(new StructElement(OFF_PREMISE_SERVICES_REACHABLE_I_PV6_ID, offPremiseServicesReachableIPv6 != null ? new BooleanType(offPremiseServicesReachableIPv6) : new NullType()));
    values.add(new StructElement(HARDWARE_ADDRESS_ID, new ByteArrayType(hardwareAddress)));
    values.add(new StructElement(I_PV4_ADDRESSES_ID, ArrayType.generateArrayType(IPv4Addresses, (elementIPv4Addresses) -> new ByteArrayType(elementIPv4Addresses))));
    values.add(new StructElement(I_PV6_ADDRESSES_ID, ArrayType.generateArrayType(IPv6Addresses, (elementIPv6Addresses) -> new ByteArrayType(elementIPv6Addresses))));
    values.add(new StructElement(TYPE_ID, new UIntType(type)));

    return new StructType(values);
  }

  public static GeneralDiagnosticsClusterNetworkInterface decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    String name = null;
    Boolean isOperational = null;
    @Nullable Boolean offPremiseServicesReachableIPv4 = null;
    @Nullable Boolean offPremiseServicesReachableIPv6 = null;
    byte[] hardwareAddress = null;
    ArrayList<byte[]> IPv4Addresses = null;
    ArrayList<byte[]> IPv6Addresses = null;
    Integer type = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == NAME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          name = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == IS_OPERATIONAL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          isOperational = castingValue.value(Boolean.class);
        }
      } else if (element.contextTagNum() == OFF_PREMISE_SERVICES_REACHABLE_I_PV4_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          offPremiseServicesReachableIPv4 = castingValue.value(Boolean.class);
        }
      } else if (element.contextTagNum() == OFF_PREMISE_SERVICES_REACHABLE_I_PV6_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          offPremiseServicesReachableIPv6 = castingValue.value(Boolean.class);
        }
      } else if (element.contextTagNum() == HARDWARE_ADDRESS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.ByteArray) {
          ByteArrayType castingValue = element.value(ByteArrayType.class);
          hardwareAddress = castingValue.value(byte[].class);
        }
      } else if (element.contextTagNum() == I_PV4_ADDRESSES_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          IPv4Addresses = castingValue.map((elementcastingValue) -> elementcastingValue.value(byte[].class));
        }
      } else if (element.contextTagNum() == I_PV6_ADDRESSES_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          IPv6Addresses = castingValue.map((elementcastingValue) -> elementcastingValue.value(byte[].class));
        }
      } else if (element.contextTagNum() == TYPE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          type = castingValue.value(Integer.class);
        }
      }
    }
    return new GeneralDiagnosticsClusterNetworkInterface(
      name,
      isOperational,
      offPremiseServicesReachableIPv4,
      offPremiseServicesReachableIPv6,
      hardwareAddress,
      IPv4Addresses,
      IPv6Addresses,
      type
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("GeneralDiagnosticsClusterNetworkInterface {\n");
    output.append("\tname: ");
    output.append(name);
    output.append("\n");
    output.append("\tisOperational: ");
    output.append(isOperational);
    output.append("\n");
    output.append("\toffPremiseServicesReachableIPv4: ");
    output.append(offPremiseServicesReachableIPv4);
    output.append("\n");
    output.append("\toffPremiseServicesReachableIPv6: ");
    output.append(offPremiseServicesReachableIPv6);
    output.append("\n");
    output.append("\thardwareAddress: ");
    output.append(Arrays.toString(hardwareAddress));
    output.append("\n");
    output.append("\tIPv4Addresses: ");
    output.append(IPv4Addresses);
    output.append("\n");
    output.append("\tIPv6Addresses: ");
    output.append(IPv6Addresses);
    output.append("\n");
    output.append("\ttype: ");
    output.append(type);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class SoftwareDiagnosticsClusterThreadMetricsStruct {
  public Long id;
  public Optional<String> name;
  public Optional<Long> stackFreeCurrent;
  public Optional<Long> stackFreeMinimum;
  public Optional<Long> stackSize;
  private static final long ID_ID = 0L;
  private static final long NAME_ID = 1L;
  private static final long STACK_FREE_CURRENT_ID = 2L;
  private static final long STACK_FREE_MINIMUM_ID = 3L;
  private static final long STACK_SIZE_ID = 4L;

  public SoftwareDiagnosticsClusterThreadMetricsStruct(
    Long id,
    Optional<String> name,
    Optional<Long> stackFreeCurrent,
    Optional<Long> stackFreeMinimum,
    Optional<Long> stackSize
  ) {
    this.id = id;
    this.name = name;
    this.stackFreeCurrent = stackFreeCurrent;
    this.stackFreeMinimum = stackFreeMinimum;
    this.stackSize = stackSize;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ID_ID, new UIntType(id)));
    values.add(new StructElement(NAME_ID, name.<BaseTLVType>map((nonOptionalname) -> new StringType(nonOptionalname)).orElse(new EmptyType())));
    values.add(new StructElement(STACK_FREE_CURRENT_ID, stackFreeCurrent.<BaseTLVType>map((nonOptionalstackFreeCurrent) -> new UIntType(nonOptionalstackFreeCurrent)).orElse(new EmptyType())));
    values.add(new StructElement(STACK_FREE_MINIMUM_ID, stackFreeMinimum.<BaseTLVType>map((nonOptionalstackFreeMinimum) -> new UIntType(nonOptionalstackFreeMinimum)).orElse(new EmptyType())));
    values.add(new StructElement(STACK_SIZE_ID, stackSize.<BaseTLVType>map((nonOptionalstackSize) -> new UIntType(nonOptionalstackSize)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static SoftwareDiagnosticsClusterThreadMetricsStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long id = null;
    Optional<String> name = Optional.empty();
    Optional<Long> stackFreeCurrent = Optional.empty();
    Optional<Long> stackFreeMinimum = Optional.empty();
    Optional<Long> stackSize = Optional.empty();
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
      } else if (element.contextTagNum() == STACK_FREE_CURRENT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          stackFreeCurrent = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == STACK_FREE_MINIMUM_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          stackFreeMinimum = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == STACK_SIZE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          stackSize = Optional.of(castingValue.value(Long.class));
        }
      }
    }
    return new SoftwareDiagnosticsClusterThreadMetricsStruct(
      id,
      name,
      stackFreeCurrent,
      stackFreeMinimum,
      stackSize
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("SoftwareDiagnosticsClusterThreadMetricsStruct {\n");
    output.append("\tid: ");
    output.append(id);
    output.append("\n");
    output.append("\tname: ");
    output.append(name);
    output.append("\n");
    output.append("\tstackFreeCurrent: ");
    output.append(stackFreeCurrent);
    output.append("\n");
    output.append("\tstackFreeMinimum: ");
    output.append(stackFreeMinimum);
    output.append("\n");
    output.append("\tstackSize: ");
    output.append(stackSize);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ThreadNetworkDiagnosticsClusterNeighborTableStruct {
  public Long extAddress;
  public Long age;
  public Integer rloc16;
  public Long linkFrameCounter;
  public Long mleFrameCounter;
  public Integer lqi;
  public @Nullable Integer averageRssi;
  public @Nullable Integer lastRssi;
  public Integer frameErrorRate;
  public Integer messageErrorRate;
  public Boolean rxOnWhenIdle;
  public Boolean fullThreadDevice;
  public Boolean fullNetworkData;
  public Boolean isChild;
  private static final long EXT_ADDRESS_ID = 0L;
  private static final long AGE_ID = 1L;
  private static final long RLOC16_ID = 2L;
  private static final long LINK_FRAME_COUNTER_ID = 3L;
  private static final long MLE_FRAME_COUNTER_ID = 4L;
  private static final long LQI_ID = 5L;
  private static final long AVERAGE_RSSI_ID = 6L;
  private static final long LAST_RSSI_ID = 7L;
  private static final long FRAME_ERROR_RATE_ID = 8L;
  private static final long MESSAGE_ERROR_RATE_ID = 9L;
  private static final long RX_ON_WHEN_IDLE_ID = 10L;
  private static final long FULL_THREAD_DEVICE_ID = 11L;
  private static final long FULL_NETWORK_DATA_ID = 12L;
  private static final long IS_CHILD_ID = 13L;

  public ThreadNetworkDiagnosticsClusterNeighborTableStruct(
    Long extAddress,
    Long age,
    Integer rloc16,
    Long linkFrameCounter,
    Long mleFrameCounter,
    Integer lqi,
    @Nullable Integer averageRssi,
    @Nullable Integer lastRssi,
    Integer frameErrorRate,
    Integer messageErrorRate,
    Boolean rxOnWhenIdle,
    Boolean fullThreadDevice,
    Boolean fullNetworkData,
    Boolean isChild
  ) {
    this.extAddress = extAddress;
    this.age = age;
    this.rloc16 = rloc16;
    this.linkFrameCounter = linkFrameCounter;
    this.mleFrameCounter = mleFrameCounter;
    this.lqi = lqi;
    this.averageRssi = averageRssi;
    this.lastRssi = lastRssi;
    this.frameErrorRate = frameErrorRate;
    this.messageErrorRate = messageErrorRate;
    this.rxOnWhenIdle = rxOnWhenIdle;
    this.fullThreadDevice = fullThreadDevice;
    this.fullNetworkData = fullNetworkData;
    this.isChild = isChild;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(EXT_ADDRESS_ID, new UIntType(extAddress)));
    values.add(new StructElement(AGE_ID, new UIntType(age)));
    values.add(new StructElement(RLOC16_ID, new UIntType(rloc16)));
    values.add(new StructElement(LINK_FRAME_COUNTER_ID, new UIntType(linkFrameCounter)));
    values.add(new StructElement(MLE_FRAME_COUNTER_ID, new UIntType(mleFrameCounter)));
    values.add(new StructElement(LQI_ID, new UIntType(lqi)));
    values.add(new StructElement(AVERAGE_RSSI_ID, averageRssi != null ? new IntType(averageRssi) : new NullType()));
    values.add(new StructElement(LAST_RSSI_ID, lastRssi != null ? new IntType(lastRssi) : new NullType()));
    values.add(new StructElement(FRAME_ERROR_RATE_ID, new UIntType(frameErrorRate)));
    values.add(new StructElement(MESSAGE_ERROR_RATE_ID, new UIntType(messageErrorRate)));
    values.add(new StructElement(RX_ON_WHEN_IDLE_ID, new BooleanType(rxOnWhenIdle)));
    values.add(new StructElement(FULL_THREAD_DEVICE_ID, new BooleanType(fullThreadDevice)));
    values.add(new StructElement(FULL_NETWORK_DATA_ID, new BooleanType(fullNetworkData)));
    values.add(new StructElement(IS_CHILD_ID, new BooleanType(isChild)));

    return new StructType(values);
  }

  public static ThreadNetworkDiagnosticsClusterNeighborTableStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long extAddress = null;
    Long age = null;
    Integer rloc16 = null;
    Long linkFrameCounter = null;
    Long mleFrameCounter = null;
    Integer lqi = null;
    @Nullable Integer averageRssi = null;
    @Nullable Integer lastRssi = null;
    Integer frameErrorRate = null;
    Integer messageErrorRate = null;
    Boolean rxOnWhenIdle = null;
    Boolean fullThreadDevice = null;
    Boolean fullNetworkData = null;
    Boolean isChild = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == EXT_ADDRESS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          extAddress = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == AGE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          age = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == RLOC16_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          rloc16 = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == LINK_FRAME_COUNTER_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          linkFrameCounter = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == MLE_FRAME_COUNTER_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          mleFrameCounter = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == LQI_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          lqi = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == AVERAGE_RSSI_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          averageRssi = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == LAST_RSSI_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          lastRssi = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == FRAME_ERROR_RATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          frameErrorRate = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == MESSAGE_ERROR_RATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          messageErrorRate = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == RX_ON_WHEN_IDLE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          rxOnWhenIdle = castingValue.value(Boolean.class);
        }
      } else if (element.contextTagNum() == FULL_THREAD_DEVICE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          fullThreadDevice = castingValue.value(Boolean.class);
        }
      } else if (element.contextTagNum() == FULL_NETWORK_DATA_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          fullNetworkData = castingValue.value(Boolean.class);
        }
      } else if (element.contextTagNum() == IS_CHILD_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          isChild = castingValue.value(Boolean.class);
        }
      }
    }
    return new ThreadNetworkDiagnosticsClusterNeighborTableStruct(
      extAddress,
      age,
      rloc16,
      linkFrameCounter,
      mleFrameCounter,
      lqi,
      averageRssi,
      lastRssi,
      frameErrorRate,
      messageErrorRate,
      rxOnWhenIdle,
      fullThreadDevice,
      fullNetworkData,
      isChild
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ThreadNetworkDiagnosticsClusterNeighborTableStruct {\n");
    output.append("\textAddress: ");
    output.append(extAddress);
    output.append("\n");
    output.append("\tage: ");
    output.append(age);
    output.append("\n");
    output.append("\trloc16: ");
    output.append(rloc16);
    output.append("\n");
    output.append("\tlinkFrameCounter: ");
    output.append(linkFrameCounter);
    output.append("\n");
    output.append("\tmleFrameCounter: ");
    output.append(mleFrameCounter);
    output.append("\n");
    output.append("\tlqi: ");
    output.append(lqi);
    output.append("\n");
    output.append("\taverageRssi: ");
    output.append(averageRssi);
    output.append("\n");
    output.append("\tlastRssi: ");
    output.append(lastRssi);
    output.append("\n");
    output.append("\tframeErrorRate: ");
    output.append(frameErrorRate);
    output.append("\n");
    output.append("\tmessageErrorRate: ");
    output.append(messageErrorRate);
    output.append("\n");
    output.append("\trxOnWhenIdle: ");
    output.append(rxOnWhenIdle);
    output.append("\n");
    output.append("\tfullThreadDevice: ");
    output.append(fullThreadDevice);
    output.append("\n");
    output.append("\tfullNetworkData: ");
    output.append(fullNetworkData);
    output.append("\n");
    output.append("\tisChild: ");
    output.append(isChild);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ThreadNetworkDiagnosticsClusterOperationalDatasetComponents {
  public Boolean activeTimestampPresent;
  public Boolean pendingTimestampPresent;
  public Boolean masterKeyPresent;
  public Boolean networkNamePresent;
  public Boolean extendedPanIdPresent;
  public Boolean meshLocalPrefixPresent;
  public Boolean delayPresent;
  public Boolean panIdPresent;
  public Boolean channelPresent;
  public Boolean pskcPresent;
  public Boolean securityPolicyPresent;
  public Boolean channelMaskPresent;
  private static final long ACTIVE_TIMESTAMP_PRESENT_ID = 0L;
  private static final long PENDING_TIMESTAMP_PRESENT_ID = 1L;
  private static final long MASTER_KEY_PRESENT_ID = 2L;
  private static final long NETWORK_NAME_PRESENT_ID = 3L;
  private static final long EXTENDED_PAN_ID_PRESENT_ID = 4L;
  private static final long MESH_LOCAL_PREFIX_PRESENT_ID = 5L;
  private static final long DELAY_PRESENT_ID = 6L;
  private static final long PAN_ID_PRESENT_ID = 7L;
  private static final long CHANNEL_PRESENT_ID = 8L;
  private static final long PSKC_PRESENT_ID = 9L;
  private static final long SECURITY_POLICY_PRESENT_ID = 10L;
  private static final long CHANNEL_MASK_PRESENT_ID = 11L;

  public ThreadNetworkDiagnosticsClusterOperationalDatasetComponents(
    Boolean activeTimestampPresent,
    Boolean pendingTimestampPresent,
    Boolean masterKeyPresent,
    Boolean networkNamePresent,
    Boolean extendedPanIdPresent,
    Boolean meshLocalPrefixPresent,
    Boolean delayPresent,
    Boolean panIdPresent,
    Boolean channelPresent,
    Boolean pskcPresent,
    Boolean securityPolicyPresent,
    Boolean channelMaskPresent
  ) {
    this.activeTimestampPresent = activeTimestampPresent;
    this.pendingTimestampPresent = pendingTimestampPresent;
    this.masterKeyPresent = masterKeyPresent;
    this.networkNamePresent = networkNamePresent;
    this.extendedPanIdPresent = extendedPanIdPresent;
    this.meshLocalPrefixPresent = meshLocalPrefixPresent;
    this.delayPresent = delayPresent;
    this.panIdPresent = panIdPresent;
    this.channelPresent = channelPresent;
    this.pskcPresent = pskcPresent;
    this.securityPolicyPresent = securityPolicyPresent;
    this.channelMaskPresent = channelMaskPresent;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ACTIVE_TIMESTAMP_PRESENT_ID, new BooleanType(activeTimestampPresent)));
    values.add(new StructElement(PENDING_TIMESTAMP_PRESENT_ID, new BooleanType(pendingTimestampPresent)));
    values.add(new StructElement(MASTER_KEY_PRESENT_ID, new BooleanType(masterKeyPresent)));
    values.add(new StructElement(NETWORK_NAME_PRESENT_ID, new BooleanType(networkNamePresent)));
    values.add(new StructElement(EXTENDED_PAN_ID_PRESENT_ID, new BooleanType(extendedPanIdPresent)));
    values.add(new StructElement(MESH_LOCAL_PREFIX_PRESENT_ID, new BooleanType(meshLocalPrefixPresent)));
    values.add(new StructElement(DELAY_PRESENT_ID, new BooleanType(delayPresent)));
    values.add(new StructElement(PAN_ID_PRESENT_ID, new BooleanType(panIdPresent)));
    values.add(new StructElement(CHANNEL_PRESENT_ID, new BooleanType(channelPresent)));
    values.add(new StructElement(PSKC_PRESENT_ID, new BooleanType(pskcPresent)));
    values.add(new StructElement(SECURITY_POLICY_PRESENT_ID, new BooleanType(securityPolicyPresent)));
    values.add(new StructElement(CHANNEL_MASK_PRESENT_ID, new BooleanType(channelMaskPresent)));

    return new StructType(values);
  }

  public static ThreadNetworkDiagnosticsClusterOperationalDatasetComponents decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Boolean activeTimestampPresent = null;
    Boolean pendingTimestampPresent = null;
    Boolean masterKeyPresent = null;
    Boolean networkNamePresent = null;
    Boolean extendedPanIdPresent = null;
    Boolean meshLocalPrefixPresent = null;
    Boolean delayPresent = null;
    Boolean panIdPresent = null;
    Boolean channelPresent = null;
    Boolean pskcPresent = null;
    Boolean securityPolicyPresent = null;
    Boolean channelMaskPresent = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ACTIVE_TIMESTAMP_PRESENT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          activeTimestampPresent = castingValue.value(Boolean.class);
        }
      } else if (element.contextTagNum() == PENDING_TIMESTAMP_PRESENT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          pendingTimestampPresent = castingValue.value(Boolean.class);
        }
      } else if (element.contextTagNum() == MASTER_KEY_PRESENT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          masterKeyPresent = castingValue.value(Boolean.class);
        }
      } else if (element.contextTagNum() == NETWORK_NAME_PRESENT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          networkNamePresent = castingValue.value(Boolean.class);
        }
      } else if (element.contextTagNum() == EXTENDED_PAN_ID_PRESENT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          extendedPanIdPresent = castingValue.value(Boolean.class);
        }
      } else if (element.contextTagNum() == MESH_LOCAL_PREFIX_PRESENT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          meshLocalPrefixPresent = castingValue.value(Boolean.class);
        }
      } else if (element.contextTagNum() == DELAY_PRESENT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          delayPresent = castingValue.value(Boolean.class);
        }
      } else if (element.contextTagNum() == PAN_ID_PRESENT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          panIdPresent = castingValue.value(Boolean.class);
        }
      } else if (element.contextTagNum() == CHANNEL_PRESENT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          channelPresent = castingValue.value(Boolean.class);
        }
      } else if (element.contextTagNum() == PSKC_PRESENT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          pskcPresent = castingValue.value(Boolean.class);
        }
      } else if (element.contextTagNum() == SECURITY_POLICY_PRESENT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          securityPolicyPresent = castingValue.value(Boolean.class);
        }
      } else if (element.contextTagNum() == CHANNEL_MASK_PRESENT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          channelMaskPresent = castingValue.value(Boolean.class);
        }
      }
    }
    return new ThreadNetworkDiagnosticsClusterOperationalDatasetComponents(
      activeTimestampPresent,
      pendingTimestampPresent,
      masterKeyPresent,
      networkNamePresent,
      extendedPanIdPresent,
      meshLocalPrefixPresent,
      delayPresent,
      panIdPresent,
      channelPresent,
      pskcPresent,
      securityPolicyPresent,
      channelMaskPresent
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ThreadNetworkDiagnosticsClusterOperationalDatasetComponents {\n");
    output.append("\tactiveTimestampPresent: ");
    output.append(activeTimestampPresent);
    output.append("\n");
    output.append("\tpendingTimestampPresent: ");
    output.append(pendingTimestampPresent);
    output.append("\n");
    output.append("\tmasterKeyPresent: ");
    output.append(masterKeyPresent);
    output.append("\n");
    output.append("\tnetworkNamePresent: ");
    output.append(networkNamePresent);
    output.append("\n");
    output.append("\textendedPanIdPresent: ");
    output.append(extendedPanIdPresent);
    output.append("\n");
    output.append("\tmeshLocalPrefixPresent: ");
    output.append(meshLocalPrefixPresent);
    output.append("\n");
    output.append("\tdelayPresent: ");
    output.append(delayPresent);
    output.append("\n");
    output.append("\tpanIdPresent: ");
    output.append(panIdPresent);
    output.append("\n");
    output.append("\tchannelPresent: ");
    output.append(channelPresent);
    output.append("\n");
    output.append("\tpskcPresent: ");
    output.append(pskcPresent);
    output.append("\n");
    output.append("\tsecurityPolicyPresent: ");
    output.append(securityPolicyPresent);
    output.append("\n");
    output.append("\tchannelMaskPresent: ");
    output.append(channelMaskPresent);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ThreadNetworkDiagnosticsClusterRouteTableStruct {
  public Long extAddress;
  public Integer rloc16;
  public Integer routerId;
  public Integer nextHop;
  public Integer pathCost;
  public Integer LQIIn;
  public Integer LQIOut;
  public Integer age;
  public Boolean allocated;
  public Boolean linkEstablished;
  private static final long EXT_ADDRESS_ID = 0L;
  private static final long RLOC16_ID = 1L;
  private static final long ROUTER_ID_ID = 2L;
  private static final long NEXT_HOP_ID = 3L;
  private static final long PATH_COST_ID = 4L;
  private static final long LQI_IN_ID = 5L;
  private static final long LQI_OUT_ID = 6L;
  private static final long AGE_ID = 7L;
  private static final long ALLOCATED_ID = 8L;
  private static final long LINK_ESTABLISHED_ID = 9L;

  public ThreadNetworkDiagnosticsClusterRouteTableStruct(
    Long extAddress,
    Integer rloc16,
    Integer routerId,
    Integer nextHop,
    Integer pathCost,
    Integer LQIIn,
    Integer LQIOut,
    Integer age,
    Boolean allocated,
    Boolean linkEstablished
  ) {
    this.extAddress = extAddress;
    this.rloc16 = rloc16;
    this.routerId = routerId;
    this.nextHop = nextHop;
    this.pathCost = pathCost;
    this.LQIIn = LQIIn;
    this.LQIOut = LQIOut;
    this.age = age;
    this.allocated = allocated;
    this.linkEstablished = linkEstablished;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(EXT_ADDRESS_ID, new UIntType(extAddress)));
    values.add(new StructElement(RLOC16_ID, new UIntType(rloc16)));
    values.add(new StructElement(ROUTER_ID_ID, new UIntType(routerId)));
    values.add(new StructElement(NEXT_HOP_ID, new UIntType(nextHop)));
    values.add(new StructElement(PATH_COST_ID, new UIntType(pathCost)));
    values.add(new StructElement(LQI_IN_ID, new UIntType(LQIIn)));
    values.add(new StructElement(LQI_OUT_ID, new UIntType(LQIOut)));
    values.add(new StructElement(AGE_ID, new UIntType(age)));
    values.add(new StructElement(ALLOCATED_ID, new BooleanType(allocated)));
    values.add(new StructElement(LINK_ESTABLISHED_ID, new BooleanType(linkEstablished)));

    return new StructType(values);
  }

  public static ThreadNetworkDiagnosticsClusterRouteTableStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long extAddress = null;
    Integer rloc16 = null;
    Integer routerId = null;
    Integer nextHop = null;
    Integer pathCost = null;
    Integer LQIIn = null;
    Integer LQIOut = null;
    Integer age = null;
    Boolean allocated = null;
    Boolean linkEstablished = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == EXT_ADDRESS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          extAddress = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == RLOC16_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          rloc16 = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == ROUTER_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          routerId = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == NEXT_HOP_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          nextHop = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == PATH_COST_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          pathCost = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == LQI_IN_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          LQIIn = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == LQI_OUT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          LQIOut = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == AGE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          age = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == ALLOCATED_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          allocated = castingValue.value(Boolean.class);
        }
      } else if (element.contextTagNum() == LINK_ESTABLISHED_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          linkEstablished = castingValue.value(Boolean.class);
        }
      }
    }
    return new ThreadNetworkDiagnosticsClusterRouteTableStruct(
      extAddress,
      rloc16,
      routerId,
      nextHop,
      pathCost,
      LQIIn,
      LQIOut,
      age,
      allocated,
      linkEstablished
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ThreadNetworkDiagnosticsClusterRouteTableStruct {\n");
    output.append("\textAddress: ");
    output.append(extAddress);
    output.append("\n");
    output.append("\trloc16: ");
    output.append(rloc16);
    output.append("\n");
    output.append("\trouterId: ");
    output.append(routerId);
    output.append("\n");
    output.append("\tnextHop: ");
    output.append(nextHop);
    output.append("\n");
    output.append("\tpathCost: ");
    output.append(pathCost);
    output.append("\n");
    output.append("\tLQIIn: ");
    output.append(LQIIn);
    output.append("\n");
    output.append("\tLQIOut: ");
    output.append(LQIOut);
    output.append("\n");
    output.append("\tage: ");
    output.append(age);
    output.append("\n");
    output.append("\tallocated: ");
    output.append(allocated);
    output.append("\n");
    output.append("\tlinkEstablished: ");
    output.append(linkEstablished);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ThreadNetworkDiagnosticsClusterSecurityPolicy {
  public Integer rotationTime;
  public Integer flags;
  private static final long ROTATION_TIME_ID = 0L;
  private static final long FLAGS_ID = 1L;

  public ThreadNetworkDiagnosticsClusterSecurityPolicy(
    Integer rotationTime,
    Integer flags
  ) {
    this.rotationTime = rotationTime;
    this.flags = flags;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ROTATION_TIME_ID, new UIntType(rotationTime)));
    values.add(new StructElement(FLAGS_ID, new UIntType(flags)));

    return new StructType(values);
  }

  public static ThreadNetworkDiagnosticsClusterSecurityPolicy decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer rotationTime = null;
    Integer flags = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ROTATION_TIME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          rotationTime = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == FLAGS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          flags = castingValue.value(Integer.class);
        }
      }
    }
    return new ThreadNetworkDiagnosticsClusterSecurityPolicy(
      rotationTime,
      flags
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ThreadNetworkDiagnosticsClusterSecurityPolicy {\n");
    output.append("\trotationTime: ");
    output.append(rotationTime);
    output.append("\n");
    output.append("\tflags: ");
    output.append(flags);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class TimeSynchronizationClusterDSTOffsetStruct {
  public Long offset;
  public Long validStarting;
  public @Nullable Long validUntil;
  private static final long OFFSET_ID = 0L;
  private static final long VALID_STARTING_ID = 1L;
  private static final long VALID_UNTIL_ID = 2L;

  public TimeSynchronizationClusterDSTOffsetStruct(
    Long offset,
    Long validStarting,
    @Nullable Long validUntil
  ) {
    this.offset = offset;
    this.validStarting = validStarting;
    this.validUntil = validUntil;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(OFFSET_ID, new IntType(offset)));
    values.add(new StructElement(VALID_STARTING_ID, new UIntType(validStarting)));
    values.add(new StructElement(VALID_UNTIL_ID, validUntil != null ? new UIntType(validUntil) : new NullType()));

    return new StructType(values);
  }

  public static TimeSynchronizationClusterDSTOffsetStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long offset = null;
    Long validStarting = null;
    @Nullable Long validUntil = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == OFFSET_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          offset = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == VALID_STARTING_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          validStarting = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == VALID_UNTIL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          validUntil = castingValue.value(Long.class);
        }
      }
    }
    return new TimeSynchronizationClusterDSTOffsetStruct(
      offset,
      validStarting,
      validUntil
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("TimeSynchronizationClusterDSTOffsetStruct {\n");
    output.append("\toffset: ");
    output.append(offset);
    output.append("\n");
    output.append("\tvalidStarting: ");
    output.append(validStarting);
    output.append("\n");
    output.append("\tvalidUntil: ");
    output.append(validUntil);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class TimeSynchronizationClusterFabricScopedTrustedTimeSourceStruct {
  public Long nodeID;
  public Integer endpoint;
  private static final long NODE_ID_ID = 0L;
  private static final long ENDPOINT_ID = 1L;

  public TimeSynchronizationClusterFabricScopedTrustedTimeSourceStruct(
    Long nodeID,
    Integer endpoint
  ) {
    this.nodeID = nodeID;
    this.endpoint = endpoint;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(NODE_ID_ID, new UIntType(nodeID)));
    values.add(new StructElement(ENDPOINT_ID, new UIntType(endpoint)));

    return new StructType(values);
  }

  public static TimeSynchronizationClusterFabricScopedTrustedTimeSourceStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long nodeID = null;
    Integer endpoint = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == NODE_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          nodeID = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == ENDPOINT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          endpoint = castingValue.value(Integer.class);
        }
      }
    }
    return new TimeSynchronizationClusterFabricScopedTrustedTimeSourceStruct(
      nodeID,
      endpoint
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("TimeSynchronizationClusterFabricScopedTrustedTimeSourceStruct {\n");
    output.append("\tnodeID: ");
    output.append(nodeID);
    output.append("\n");
    output.append("\tendpoint: ");
    output.append(endpoint);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class TimeSynchronizationClusterTimeZoneStruct {
  public Long offset;
  public Long validAt;
  public Optional<String> name;
  private static final long OFFSET_ID = 0L;
  private static final long VALID_AT_ID = 1L;
  private static final long NAME_ID = 2L;

  public TimeSynchronizationClusterTimeZoneStruct(
    Long offset,
    Long validAt,
    Optional<String> name
  ) {
    this.offset = offset;
    this.validAt = validAt;
    this.name = name;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(OFFSET_ID, new IntType(offset)));
    values.add(new StructElement(VALID_AT_ID, new UIntType(validAt)));
    values.add(new StructElement(NAME_ID, name.<BaseTLVType>map((nonOptionalname) -> new StringType(nonOptionalname)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static TimeSynchronizationClusterTimeZoneStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long offset = null;
    Long validAt = null;
    Optional<String> name = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == OFFSET_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          offset = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == VALID_AT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          validAt = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == NAME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          name = Optional.of(castingValue.value(String.class));
        }
      }
    }
    return new TimeSynchronizationClusterTimeZoneStruct(
      offset,
      validAt,
      name
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("TimeSynchronizationClusterTimeZoneStruct {\n");
    output.append("\toffset: ");
    output.append(offset);
    output.append("\n");
    output.append("\tvalidAt: ");
    output.append(validAt);
    output.append("\n");
    output.append("\tname: ");
    output.append(name);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class TimeSynchronizationClusterTrustedTimeSourceStruct {
  public Integer fabricIndex;
  public Long nodeID;
  public Integer endpoint;
  private static final long FABRIC_INDEX_ID = 0L;
  private static final long NODE_ID_ID = 1L;
  private static final long ENDPOINT_ID = 2L;

  public TimeSynchronizationClusterTrustedTimeSourceStruct(
    Integer fabricIndex,
    Long nodeID,
    Integer endpoint
  ) {
    this.fabricIndex = fabricIndex;
    this.nodeID = nodeID;
    this.endpoint = endpoint;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(FABRIC_INDEX_ID, new UIntType(fabricIndex)));
    values.add(new StructElement(NODE_ID_ID, new UIntType(nodeID)));
    values.add(new StructElement(ENDPOINT_ID, new UIntType(endpoint)));

    return new StructType(values);
  }

  public static TimeSynchronizationClusterTrustedTimeSourceStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer fabricIndex = null;
    Long nodeID = null;
    Integer endpoint = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == FABRIC_INDEX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          fabricIndex = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == NODE_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          nodeID = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == ENDPOINT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          endpoint = castingValue.value(Integer.class);
        }
      }
    }
    return new TimeSynchronizationClusterTrustedTimeSourceStruct(
      fabricIndex,
      nodeID,
      endpoint
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("TimeSynchronizationClusterTrustedTimeSourceStruct {\n");
    output.append("\tfabricIndex: ");
    output.append(fabricIndex);
    output.append("\n");
    output.append("\tnodeID: ");
    output.append(nodeID);
    output.append("\n");
    output.append("\tendpoint: ");
    output.append(endpoint);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class BridgedDeviceBasicInformationClusterProductAppearanceStruct {
  public Integer finish;
  public @Nullable Integer primaryColor;
  private static final long FINISH_ID = 0L;
  private static final long PRIMARY_COLOR_ID = 1L;

  public BridgedDeviceBasicInformationClusterProductAppearanceStruct(
    Integer finish,
    @Nullable Integer primaryColor
  ) {
    this.finish = finish;
    this.primaryColor = primaryColor;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(FINISH_ID, new UIntType(finish)));
    values.add(new StructElement(PRIMARY_COLOR_ID, primaryColor != null ? new UIntType(primaryColor) : new NullType()));

    return new StructType(values);
  }

  public static BridgedDeviceBasicInformationClusterProductAppearanceStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer finish = null;
    @Nullable Integer primaryColor = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == FINISH_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          finish = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == PRIMARY_COLOR_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          primaryColor = castingValue.value(Integer.class);
        }
      }
    }
    return new BridgedDeviceBasicInformationClusterProductAppearanceStruct(
      finish,
      primaryColor
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("BridgedDeviceBasicInformationClusterProductAppearanceStruct {\n");
    output.append("\tfinish: ");
    output.append(finish);
    output.append("\n");
    output.append("\tprimaryColor: ");
    output.append(primaryColor);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class OperationalCredentialsClusterFabricDescriptorStruct {
  public byte[] rootPublicKey;
  public Integer vendorID;
  public Long fabricID;
  public Long nodeID;
  public String label;
  public Optional<byte[]> vidVerificationStatement;
  public Integer fabricIndex;
  private static final long ROOT_PUBLIC_KEY_ID = 1L;
  private static final long VENDOR_ID_ID = 2L;
  private static final long FABRIC_ID_ID = 3L;
  private static final long NODE_ID_ID = 4L;
  private static final long LABEL_ID = 5L;
  private static final long VID_VERIFICATION_STATEMENT_ID = 6L;
  private static final long FABRIC_INDEX_ID = 254L;

  public OperationalCredentialsClusterFabricDescriptorStruct(
    byte[] rootPublicKey,
    Integer vendorID,
    Long fabricID,
    Long nodeID,
    String label,
    Optional<byte[]> vidVerificationStatement,
    Integer fabricIndex
  ) {
    this.rootPublicKey = rootPublicKey;
    this.vendorID = vendorID;
    this.fabricID = fabricID;
    this.nodeID = nodeID;
    this.label = label;
    this.vidVerificationStatement = vidVerificationStatement;
    this.fabricIndex = fabricIndex;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ROOT_PUBLIC_KEY_ID, new ByteArrayType(rootPublicKey)));
    values.add(new StructElement(VENDOR_ID_ID, new UIntType(vendorID)));
    values.add(new StructElement(FABRIC_ID_ID, new UIntType(fabricID)));
    values.add(new StructElement(NODE_ID_ID, new UIntType(nodeID)));
    values.add(new StructElement(LABEL_ID, new StringType(label)));
    values.add(new StructElement(VID_VERIFICATION_STATEMENT_ID, vidVerificationStatement.<BaseTLVType>map((nonOptionalvidVerificationStatement) -> new ByteArrayType(nonOptionalvidVerificationStatement)).orElse(new EmptyType())));
    values.add(new StructElement(FABRIC_INDEX_ID, new UIntType(fabricIndex)));

    return new StructType(values);
  }

  public static OperationalCredentialsClusterFabricDescriptorStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    byte[] rootPublicKey = null;
    Integer vendorID = null;
    Long fabricID = null;
    Long nodeID = null;
    String label = null;
    Optional<byte[]> vidVerificationStatement = Optional.empty();
    Integer fabricIndex = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ROOT_PUBLIC_KEY_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.ByteArray) {
          ByteArrayType castingValue = element.value(ByteArrayType.class);
          rootPublicKey = castingValue.value(byte[].class);
        }
      } else if (element.contextTagNum() == VENDOR_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          vendorID = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == FABRIC_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          fabricID = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == NODE_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          nodeID = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == LABEL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          label = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == VID_VERIFICATION_STATEMENT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.ByteArray) {
          ByteArrayType castingValue = element.value(ByteArrayType.class);
          vidVerificationStatement = Optional.of(castingValue.value(byte[].class));
        }
      } else if (element.contextTagNum() == FABRIC_INDEX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          fabricIndex = castingValue.value(Integer.class);
        }
      }
    }
    return new OperationalCredentialsClusterFabricDescriptorStruct(
      rootPublicKey,
      vendorID,
      fabricID,
      nodeID,
      label,
      vidVerificationStatement,
      fabricIndex
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("OperationalCredentialsClusterFabricDescriptorStruct {\n");
    output.append("\trootPublicKey: ");
    output.append(Arrays.toString(rootPublicKey));
    output.append("\n");
    output.append("\tvendorID: ");
    output.append(vendorID);
    output.append("\n");
    output.append("\tfabricID: ");
    output.append(fabricID);
    output.append("\n");
    output.append("\tnodeID: ");
    output.append(nodeID);
    output.append("\n");
    output.append("\tlabel: ");
    output.append(label);
    output.append("\n");
    output.append("\tvidVerificationStatement: ");
    output.append(vidVerificationStatement.isPresent() ? Arrays.toString(vidVerificationStatement.get()) : "");
    output.append("\n");
    output.append("\tfabricIndex: ");
    output.append(fabricIndex);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class OperationalCredentialsClusterNOCStruct {
  public byte[] noc;
  public @Nullable byte[] icac;
  public Optional<byte[]> vvsc;
  public Integer fabricIndex;
  private static final long NOC_ID = 1L;
  private static final long ICAC_ID = 2L;
  private static final long VVSC_ID = 3L;
  private static final long FABRIC_INDEX_ID = 254L;

  public OperationalCredentialsClusterNOCStruct(
    byte[] noc,
    @Nullable byte[] icac,
    Optional<byte[]> vvsc,
    Integer fabricIndex
  ) {
    this.noc = noc;
    this.icac = icac;
    this.vvsc = vvsc;
    this.fabricIndex = fabricIndex;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(NOC_ID, new ByteArrayType(noc)));
    values.add(new StructElement(ICAC_ID, icac != null ? new ByteArrayType(icac) : new NullType()));
    values.add(new StructElement(VVSC_ID, vvsc.<BaseTLVType>map((nonOptionalvvsc) -> new ByteArrayType(nonOptionalvvsc)).orElse(new EmptyType())));
    values.add(new StructElement(FABRIC_INDEX_ID, new UIntType(fabricIndex)));

    return new StructType(values);
  }

  public static OperationalCredentialsClusterNOCStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    byte[] noc = null;
    @Nullable byte[] icac = null;
    Optional<byte[]> vvsc = Optional.empty();
    Integer fabricIndex = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == NOC_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.ByteArray) {
          ByteArrayType castingValue = element.value(ByteArrayType.class);
          noc = castingValue.value(byte[].class);
        }
      } else if (element.contextTagNum() == ICAC_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.ByteArray) {
          ByteArrayType castingValue = element.value(ByteArrayType.class);
          icac = castingValue.value(byte[].class);
        }
      } else if (element.contextTagNum() == VVSC_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.ByteArray) {
          ByteArrayType castingValue = element.value(ByteArrayType.class);
          vvsc = Optional.of(castingValue.value(byte[].class));
        }
      } else if (element.contextTagNum() == FABRIC_INDEX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          fabricIndex = castingValue.value(Integer.class);
        }
      }
    }
    return new OperationalCredentialsClusterNOCStruct(
      noc,
      icac,
      vvsc,
      fabricIndex
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("OperationalCredentialsClusterNOCStruct {\n");
    output.append("\tnoc: ");
    output.append(Arrays.toString(noc));
    output.append("\n");
    output.append("\ticac: ");
    output.append(Arrays.toString(icac));
    output.append("\n");
    output.append("\tvvsc: ");
    output.append(vvsc.isPresent() ? Arrays.toString(vvsc.get()) : "");
    output.append("\n");
    output.append("\tfabricIndex: ");
    output.append(fabricIndex);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class GroupKeyManagementClusterGroupInfoMapStruct {
  public Integer groupId;
  public ArrayList<Integer> endpoints;
  public Optional<String> groupName;
  public Integer fabricIndex;
  private static final long GROUP_ID_ID = 1L;
  private static final long ENDPOINTS_ID = 2L;
  private static final long GROUP_NAME_ID = 3L;
  private static final long FABRIC_INDEX_ID = 254L;

  public GroupKeyManagementClusterGroupInfoMapStruct(
    Integer groupId,
    ArrayList<Integer> endpoints,
    Optional<String> groupName,
    Integer fabricIndex
  ) {
    this.groupId = groupId;
    this.endpoints = endpoints;
    this.groupName = groupName;
    this.fabricIndex = fabricIndex;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(GROUP_ID_ID, new UIntType(groupId)));
    values.add(new StructElement(ENDPOINTS_ID, ArrayType.generateArrayType(endpoints, (elementendpoints) -> new UIntType(elementendpoints))));
    values.add(new StructElement(GROUP_NAME_ID, groupName.<BaseTLVType>map((nonOptionalgroupName) -> new StringType(nonOptionalgroupName)).orElse(new EmptyType())));
    values.add(new StructElement(FABRIC_INDEX_ID, new UIntType(fabricIndex)));

    return new StructType(values);
  }

  public static GroupKeyManagementClusterGroupInfoMapStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer groupId = null;
    ArrayList<Integer> endpoints = null;
    Optional<String> groupName = Optional.empty();
    Integer fabricIndex = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == GROUP_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          groupId = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == ENDPOINTS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          endpoints = castingValue.map((elementcastingValue) -> elementcastingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == GROUP_NAME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          groupName = Optional.of(castingValue.value(String.class));
        }
      } else if (element.contextTagNum() == FABRIC_INDEX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          fabricIndex = castingValue.value(Integer.class);
        }
      }
    }
    return new GroupKeyManagementClusterGroupInfoMapStruct(
      groupId,
      endpoints,
      groupName,
      fabricIndex
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("GroupKeyManagementClusterGroupInfoMapStruct {\n");
    output.append("\tgroupId: ");
    output.append(groupId);
    output.append("\n");
    output.append("\tendpoints: ");
    output.append(endpoints);
    output.append("\n");
    output.append("\tgroupName: ");
    output.append(groupName);
    output.append("\n");
    output.append("\tfabricIndex: ");
    output.append(fabricIndex);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class GroupKeyManagementClusterGroupKeyMapStruct {
  public Integer groupId;
  public Integer groupKeySetID;
  public Integer fabricIndex;
  private static final long GROUP_ID_ID = 1L;
  private static final long GROUP_KEY_SET_ID_ID = 2L;
  private static final long FABRIC_INDEX_ID = 254L;

  public GroupKeyManagementClusterGroupKeyMapStruct(
    Integer groupId,
    Integer groupKeySetID,
    Integer fabricIndex
  ) {
    this.groupId = groupId;
    this.groupKeySetID = groupKeySetID;
    this.fabricIndex = fabricIndex;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(GROUP_ID_ID, new UIntType(groupId)));
    values.add(new StructElement(GROUP_KEY_SET_ID_ID, new UIntType(groupKeySetID)));
    values.add(new StructElement(FABRIC_INDEX_ID, new UIntType(fabricIndex)));

    return new StructType(values);
  }

  public static GroupKeyManagementClusterGroupKeyMapStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer groupId = null;
    Integer groupKeySetID = null;
    Integer fabricIndex = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == GROUP_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          groupId = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == GROUP_KEY_SET_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          groupKeySetID = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == FABRIC_INDEX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          fabricIndex = castingValue.value(Integer.class);
        }
      }
    }
    return new GroupKeyManagementClusterGroupKeyMapStruct(
      groupId,
      groupKeySetID,
      fabricIndex
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("GroupKeyManagementClusterGroupKeyMapStruct {\n");
    output.append("\tgroupId: ");
    output.append(groupId);
    output.append("\n");
    output.append("\tgroupKeySetID: ");
    output.append(groupKeySetID);
    output.append("\n");
    output.append("\tfabricIndex: ");
    output.append(fabricIndex);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class GroupKeyManagementClusterGroupKeySetStruct {
  public Integer groupKeySetID;
  public Integer groupKeySecurityPolicy;
  public @Nullable byte[] epochKey0;
  public @Nullable Long epochStartTime0;
  public @Nullable byte[] epochKey1;
  public @Nullable Long epochStartTime1;
  public @Nullable byte[] epochKey2;
  public @Nullable Long epochStartTime2;
  private static final long GROUP_KEY_SET_ID_ID = 0L;
  private static final long GROUP_KEY_SECURITY_POLICY_ID = 1L;
  private static final long EPOCH_KEY0_ID = 2L;
  private static final long EPOCH_START_TIME0_ID = 3L;
  private static final long EPOCH_KEY1_ID = 4L;
  private static final long EPOCH_START_TIME1_ID = 5L;
  private static final long EPOCH_KEY2_ID = 6L;
  private static final long EPOCH_START_TIME2_ID = 7L;

  public GroupKeyManagementClusterGroupKeySetStruct(
    Integer groupKeySetID,
    Integer groupKeySecurityPolicy,
    @Nullable byte[] epochKey0,
    @Nullable Long epochStartTime0,
    @Nullable byte[] epochKey1,
    @Nullable Long epochStartTime1,
    @Nullable byte[] epochKey2,
    @Nullable Long epochStartTime2
  ) {
    this.groupKeySetID = groupKeySetID;
    this.groupKeySecurityPolicy = groupKeySecurityPolicy;
    this.epochKey0 = epochKey0;
    this.epochStartTime0 = epochStartTime0;
    this.epochKey1 = epochKey1;
    this.epochStartTime1 = epochStartTime1;
    this.epochKey2 = epochKey2;
    this.epochStartTime2 = epochStartTime2;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(GROUP_KEY_SET_ID_ID, new UIntType(groupKeySetID)));
    values.add(new StructElement(GROUP_KEY_SECURITY_POLICY_ID, new UIntType(groupKeySecurityPolicy)));
    values.add(new StructElement(EPOCH_KEY0_ID, epochKey0 != null ? new ByteArrayType(epochKey0) : new NullType()));
    values.add(new StructElement(EPOCH_START_TIME0_ID, epochStartTime0 != null ? new UIntType(epochStartTime0) : new NullType()));
    values.add(new StructElement(EPOCH_KEY1_ID, epochKey1 != null ? new ByteArrayType(epochKey1) : new NullType()));
    values.add(new StructElement(EPOCH_START_TIME1_ID, epochStartTime1 != null ? new UIntType(epochStartTime1) : new NullType()));
    values.add(new StructElement(EPOCH_KEY2_ID, epochKey2 != null ? new ByteArrayType(epochKey2) : new NullType()));
    values.add(new StructElement(EPOCH_START_TIME2_ID, epochStartTime2 != null ? new UIntType(epochStartTime2) : new NullType()));

    return new StructType(values);
  }

  public static GroupKeyManagementClusterGroupKeySetStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer groupKeySetID = null;
    Integer groupKeySecurityPolicy = null;
    @Nullable byte[] epochKey0 = null;
    @Nullable Long epochStartTime0 = null;
    @Nullable byte[] epochKey1 = null;
    @Nullable Long epochStartTime1 = null;
    @Nullable byte[] epochKey2 = null;
    @Nullable Long epochStartTime2 = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == GROUP_KEY_SET_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          groupKeySetID = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == GROUP_KEY_SECURITY_POLICY_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          groupKeySecurityPolicy = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == EPOCH_KEY0_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.ByteArray) {
          ByteArrayType castingValue = element.value(ByteArrayType.class);
          epochKey0 = castingValue.value(byte[].class);
        }
      } else if (element.contextTagNum() == EPOCH_START_TIME0_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          epochStartTime0 = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == EPOCH_KEY1_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.ByteArray) {
          ByteArrayType castingValue = element.value(ByteArrayType.class);
          epochKey1 = castingValue.value(byte[].class);
        }
      } else if (element.contextTagNum() == EPOCH_START_TIME1_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          epochStartTime1 = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == EPOCH_KEY2_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.ByteArray) {
          ByteArrayType castingValue = element.value(ByteArrayType.class);
          epochKey2 = castingValue.value(byte[].class);
        }
      } else if (element.contextTagNum() == EPOCH_START_TIME2_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          epochStartTime2 = castingValue.value(Long.class);
        }
      }
    }
    return new GroupKeyManagementClusterGroupKeySetStruct(
      groupKeySetID,
      groupKeySecurityPolicy,
      epochKey0,
      epochStartTime0,
      epochKey1,
      epochStartTime1,
      epochKey2,
      epochStartTime2
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("GroupKeyManagementClusterGroupKeySetStruct {\n");
    output.append("\tgroupKeySetID: ");
    output.append(groupKeySetID);
    output.append("\n");
    output.append("\tgroupKeySecurityPolicy: ");
    output.append(groupKeySecurityPolicy);
    output.append("\n");
    output.append("\tepochKey0: ");
    output.append(Arrays.toString(epochKey0));
    output.append("\n");
    output.append("\tepochStartTime0: ");
    output.append(epochStartTime0);
    output.append("\n");
    output.append("\tepochKey1: ");
    output.append(Arrays.toString(epochKey1));
    output.append("\n");
    output.append("\tepochStartTime1: ");
    output.append(epochStartTime1);
    output.append("\n");
    output.append("\tepochKey2: ");
    output.append(Arrays.toString(epochKey2));
    output.append("\n");
    output.append("\tepochStartTime2: ");
    output.append(epochStartTime2);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class FixedLabelClusterLabelStruct {
  public String label;
  public String value;
  private static final long LABEL_ID = 0L;
  private static final long VALUE_ID = 1L;

  public FixedLabelClusterLabelStruct(
    String label,
    String value
  ) {
    this.label = label;
    this.value = value;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(LABEL_ID, new StringType(label)));
    values.add(new StructElement(VALUE_ID, new StringType(value)));

    return new StructType(values);
  }

  public static FixedLabelClusterLabelStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    String label = null;
    String value = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == LABEL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          label = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == VALUE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          value = castingValue.value(String.class);
        }
      }
    }
    return new FixedLabelClusterLabelStruct(
      label,
      value
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("FixedLabelClusterLabelStruct {\n");
    output.append("\tlabel: ");
    output.append(label);
    output.append("\n");
    output.append("\tvalue: ");
    output.append(value);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class UserLabelClusterLabelStruct {
  public String label;
  public String value;
  private static final long LABEL_ID = 0L;
  private static final long VALUE_ID = 1L;

  public UserLabelClusterLabelStruct(
    String label,
    String value
  ) {
    this.label = label;
    this.value = value;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(LABEL_ID, new StringType(label)));
    values.add(new StructElement(VALUE_ID, new StringType(value)));

    return new StructType(values);
  }

  public static UserLabelClusterLabelStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    String label = null;
    String value = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == LABEL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          label = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == VALUE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          value = castingValue.value(String.class);
        }
      }
    }
    return new UserLabelClusterLabelStruct(
      label,
      value
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("UserLabelClusterLabelStruct {\n");
    output.append("\tlabel: ");
    output.append(label);
    output.append("\n");
    output.append("\tvalue: ");
    output.append(value);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class IcdManagementClusterMonitoringRegistrationStruct {
  public Long checkInNodeID;
  public Long monitoredSubject;
  public Integer clientType;
  public Integer fabricIndex;
  private static final long CHECK_IN_NODE_ID_ID = 1L;
  private static final long MONITORED_SUBJECT_ID = 2L;
  private static final long CLIENT_TYPE_ID = 4L;
  private static final long FABRIC_INDEX_ID = 254L;

  public IcdManagementClusterMonitoringRegistrationStruct(
    Long checkInNodeID,
    Long monitoredSubject,
    Integer clientType,
    Integer fabricIndex
  ) {
    this.checkInNodeID = checkInNodeID;
    this.monitoredSubject = monitoredSubject;
    this.clientType = clientType;
    this.fabricIndex = fabricIndex;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(CHECK_IN_NODE_ID_ID, new UIntType(checkInNodeID)));
    values.add(new StructElement(MONITORED_SUBJECT_ID, new UIntType(monitoredSubject)));
    values.add(new StructElement(CLIENT_TYPE_ID, new UIntType(clientType)));
    values.add(new StructElement(FABRIC_INDEX_ID, new UIntType(fabricIndex)));

    return new StructType(values);
  }

  public static IcdManagementClusterMonitoringRegistrationStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long checkInNodeID = null;
    Long monitoredSubject = null;
    Integer clientType = null;
    Integer fabricIndex = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == CHECK_IN_NODE_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          checkInNodeID = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == MONITORED_SUBJECT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          monitoredSubject = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == CLIENT_TYPE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          clientType = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == FABRIC_INDEX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          fabricIndex = castingValue.value(Integer.class);
        }
      }
    }
    return new IcdManagementClusterMonitoringRegistrationStruct(
      checkInNodeID,
      monitoredSubject,
      clientType,
      fabricIndex
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("IcdManagementClusterMonitoringRegistrationStruct {\n");
    output.append("\tcheckInNodeID: ");
    output.append(checkInNodeID);
    output.append("\n");
    output.append("\tmonitoredSubject: ");
    output.append(monitoredSubject);
    output.append("\n");
    output.append("\tclientType: ");
    output.append(clientType);
    output.append("\n");
    output.append("\tfabricIndex: ");
    output.append(fabricIndex);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class OvenCavityOperationalStateClusterErrorStateStruct {
  public Integer errorStateID;
  public Optional<String> errorStateLabel;
  public Optional<String> errorStateDetails;
  private static final long ERROR_STATE_ID_ID = 0L;
  private static final long ERROR_STATE_LABEL_ID = 1L;
  private static final long ERROR_STATE_DETAILS_ID = 2L;

  public OvenCavityOperationalStateClusterErrorStateStruct(
    Integer errorStateID,
    Optional<String> errorStateLabel,
    Optional<String> errorStateDetails
  ) {
    this.errorStateID = errorStateID;
    this.errorStateLabel = errorStateLabel;
    this.errorStateDetails = errorStateDetails;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ERROR_STATE_ID_ID, new UIntType(errorStateID)));
    values.add(new StructElement(ERROR_STATE_LABEL_ID, errorStateLabel.<BaseTLVType>map((nonOptionalerrorStateLabel) -> new StringType(nonOptionalerrorStateLabel)).orElse(new EmptyType())));
    values.add(new StructElement(ERROR_STATE_DETAILS_ID, errorStateDetails.<BaseTLVType>map((nonOptionalerrorStateDetails) -> new StringType(nonOptionalerrorStateDetails)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static OvenCavityOperationalStateClusterErrorStateStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer errorStateID = null;
    Optional<String> errorStateLabel = Optional.empty();
    Optional<String> errorStateDetails = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ERROR_STATE_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          errorStateID = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == ERROR_STATE_LABEL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          errorStateLabel = Optional.of(castingValue.value(String.class));
        }
      } else if (element.contextTagNum() == ERROR_STATE_DETAILS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          errorStateDetails = Optional.of(castingValue.value(String.class));
        }
      }
    }
    return new OvenCavityOperationalStateClusterErrorStateStruct(
      errorStateID,
      errorStateLabel,
      errorStateDetails
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("OvenCavityOperationalStateClusterErrorStateStruct {\n");
    output.append("\terrorStateID: ");
    output.append(errorStateID);
    output.append("\n");
    output.append("\terrorStateLabel: ");
    output.append(errorStateLabel);
    output.append("\n");
    output.append("\terrorStateDetails: ");
    output.append(errorStateDetails);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class OvenCavityOperationalStateClusterOperationalStateStruct {
  public Integer operationalStateID;
  public Optional<String> operationalStateLabel;
  private static final long OPERATIONAL_STATE_ID_ID = 0L;
  private static final long OPERATIONAL_STATE_LABEL_ID = 1L;

  public OvenCavityOperationalStateClusterOperationalStateStruct(
    Integer operationalStateID,
    Optional<String> operationalStateLabel
  ) {
    this.operationalStateID = operationalStateID;
    this.operationalStateLabel = operationalStateLabel;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(OPERATIONAL_STATE_ID_ID, new UIntType(operationalStateID)));
    values.add(new StructElement(OPERATIONAL_STATE_LABEL_ID, operationalStateLabel.<BaseTLVType>map((nonOptionaloperationalStateLabel) -> new StringType(nonOptionaloperationalStateLabel)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static OvenCavityOperationalStateClusterOperationalStateStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer operationalStateID = null;
    Optional<String> operationalStateLabel = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == OPERATIONAL_STATE_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          operationalStateID = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == OPERATIONAL_STATE_LABEL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          operationalStateLabel = Optional.of(castingValue.value(String.class));
        }
      }
    }
    return new OvenCavityOperationalStateClusterOperationalStateStruct(
      operationalStateID,
      operationalStateLabel
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("OvenCavityOperationalStateClusterOperationalStateStruct {\n");
    output.append("\toperationalStateID: ");
    output.append(operationalStateID);
    output.append("\n");
    output.append("\toperationalStateLabel: ");
    output.append(operationalStateLabel);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class OvenModeClusterModeTagStruct {
  public Optional<Integer> mfgCode;
  public Integer value;
  private static final long MFG_CODE_ID = 0L;
  private static final long VALUE_ID = 1L;

  public OvenModeClusterModeTagStruct(
    Optional<Integer> mfgCode,
    Integer value
  ) {
    this.mfgCode = mfgCode;
    this.value = value;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(MFG_CODE_ID, mfgCode.<BaseTLVType>map((nonOptionalmfgCode) -> new UIntType(nonOptionalmfgCode)).orElse(new EmptyType())));
    values.add(new StructElement(VALUE_ID, new UIntType(value)));

    return new StructType(values);
  }

  public static OvenModeClusterModeTagStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Optional<Integer> mfgCode = Optional.empty();
    Integer value = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == MFG_CODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          mfgCode = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == VALUE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          value = castingValue.value(Integer.class);
        }
      }
    }
    return new OvenModeClusterModeTagStruct(
      mfgCode,
      value
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("OvenModeClusterModeTagStruct {\n");
    output.append("\tmfgCode: ");
    output.append(mfgCode);
    output.append("\n");
    output.append("\tvalue: ");
    output.append(value);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class OvenModeClusterModeOptionStruct {
  public String label;
  public Integer mode;
  public ArrayList<ChipStructs.OvenModeClusterModeTagStruct> modeTags;
  private static final long LABEL_ID = 0L;
  private static final long MODE_ID = 1L;
  private static final long MODE_TAGS_ID = 2L;

  public OvenModeClusterModeOptionStruct(
    String label,
    Integer mode,
    ArrayList<ChipStructs.OvenModeClusterModeTagStruct> modeTags
  ) {
    this.label = label;
    this.mode = mode;
    this.modeTags = modeTags;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(LABEL_ID, new StringType(label)));
    values.add(new StructElement(MODE_ID, new UIntType(mode)));
    values.add(new StructElement(MODE_TAGS_ID, ArrayType.generateArrayType(modeTags, (elementmodeTags) -> elementmodeTags.encodeTlv())));

    return new StructType(values);
  }

  public static OvenModeClusterModeOptionStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    String label = null;
    Integer mode = null;
    ArrayList<ChipStructs.OvenModeClusterModeTagStruct> modeTags = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == LABEL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          label = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == MODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          mode = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == MODE_TAGS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          modeTags = castingValue.map((elementcastingValue) -> ChipStructs.OvenModeClusterModeTagStruct.decodeTlv(elementcastingValue));
        }
      }
    }
    return new OvenModeClusterModeOptionStruct(
      label,
      mode,
      modeTags
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("OvenModeClusterModeOptionStruct {\n");
    output.append("\tlabel: ");
    output.append(label);
    output.append("\n");
    output.append("\tmode: ");
    output.append(mode);
    output.append("\n");
    output.append("\tmodeTags: ");
    output.append(modeTags);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ModeSelectClusterSemanticTagStruct {
  public Integer mfgCode;
  public Integer value;
  private static final long MFG_CODE_ID = 0L;
  private static final long VALUE_ID = 1L;

  public ModeSelectClusterSemanticTagStruct(
    Integer mfgCode,
    Integer value
  ) {
    this.mfgCode = mfgCode;
    this.value = value;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(MFG_CODE_ID, new UIntType(mfgCode)));
    values.add(new StructElement(VALUE_ID, new UIntType(value)));

    return new StructType(values);
  }

  public static ModeSelectClusterSemanticTagStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer mfgCode = null;
    Integer value = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == MFG_CODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          mfgCode = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == VALUE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          value = castingValue.value(Integer.class);
        }
      }
    }
    return new ModeSelectClusterSemanticTagStruct(
      mfgCode,
      value
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ModeSelectClusterSemanticTagStruct {\n");
    output.append("\tmfgCode: ");
    output.append(mfgCode);
    output.append("\n");
    output.append("\tvalue: ");
    output.append(value);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ModeSelectClusterModeOptionStruct {
  public String label;
  public Integer mode;
  public ArrayList<ChipStructs.ModeSelectClusterSemanticTagStruct> semanticTags;
  private static final long LABEL_ID = 0L;
  private static final long MODE_ID = 1L;
  private static final long SEMANTIC_TAGS_ID = 2L;

  public ModeSelectClusterModeOptionStruct(
    String label,
    Integer mode,
    ArrayList<ChipStructs.ModeSelectClusterSemanticTagStruct> semanticTags
  ) {
    this.label = label;
    this.mode = mode;
    this.semanticTags = semanticTags;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(LABEL_ID, new StringType(label)));
    values.add(new StructElement(MODE_ID, new UIntType(mode)));
    values.add(new StructElement(SEMANTIC_TAGS_ID, ArrayType.generateArrayType(semanticTags, (elementsemanticTags) -> elementsemanticTags.encodeTlv())));

    return new StructType(values);
  }

  public static ModeSelectClusterModeOptionStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    String label = null;
    Integer mode = null;
    ArrayList<ChipStructs.ModeSelectClusterSemanticTagStruct> semanticTags = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == LABEL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          label = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == MODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          mode = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == SEMANTIC_TAGS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          semanticTags = castingValue.map((elementcastingValue) -> ChipStructs.ModeSelectClusterSemanticTagStruct.decodeTlv(elementcastingValue));
        }
      }
    }
    return new ModeSelectClusterModeOptionStruct(
      label,
      mode,
      semanticTags
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ModeSelectClusterModeOptionStruct {\n");
    output.append("\tlabel: ");
    output.append(label);
    output.append("\n");
    output.append("\tmode: ");
    output.append(mode);
    output.append("\n");
    output.append("\tsemanticTags: ");
    output.append(semanticTags);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class LaundryWasherModeClusterModeTagStruct {
  public Optional<Integer> mfgCode;
  public Integer value;
  private static final long MFG_CODE_ID = 0L;
  private static final long VALUE_ID = 1L;

  public LaundryWasherModeClusterModeTagStruct(
    Optional<Integer> mfgCode,
    Integer value
  ) {
    this.mfgCode = mfgCode;
    this.value = value;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(MFG_CODE_ID, mfgCode.<BaseTLVType>map((nonOptionalmfgCode) -> new UIntType(nonOptionalmfgCode)).orElse(new EmptyType())));
    values.add(new StructElement(VALUE_ID, new UIntType(value)));

    return new StructType(values);
  }

  public static LaundryWasherModeClusterModeTagStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Optional<Integer> mfgCode = Optional.empty();
    Integer value = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == MFG_CODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          mfgCode = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == VALUE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          value = castingValue.value(Integer.class);
        }
      }
    }
    return new LaundryWasherModeClusterModeTagStruct(
      mfgCode,
      value
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("LaundryWasherModeClusterModeTagStruct {\n");
    output.append("\tmfgCode: ");
    output.append(mfgCode);
    output.append("\n");
    output.append("\tvalue: ");
    output.append(value);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class LaundryWasherModeClusterModeOptionStruct {
  public String label;
  public Integer mode;
  public ArrayList<ChipStructs.LaundryWasherModeClusterModeTagStruct> modeTags;
  private static final long LABEL_ID = 0L;
  private static final long MODE_ID = 1L;
  private static final long MODE_TAGS_ID = 2L;

  public LaundryWasherModeClusterModeOptionStruct(
    String label,
    Integer mode,
    ArrayList<ChipStructs.LaundryWasherModeClusterModeTagStruct> modeTags
  ) {
    this.label = label;
    this.mode = mode;
    this.modeTags = modeTags;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(LABEL_ID, new StringType(label)));
    values.add(new StructElement(MODE_ID, new UIntType(mode)));
    values.add(new StructElement(MODE_TAGS_ID, ArrayType.generateArrayType(modeTags, (elementmodeTags) -> elementmodeTags.encodeTlv())));

    return new StructType(values);
  }

  public static LaundryWasherModeClusterModeOptionStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    String label = null;
    Integer mode = null;
    ArrayList<ChipStructs.LaundryWasherModeClusterModeTagStruct> modeTags = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == LABEL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          label = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == MODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          mode = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == MODE_TAGS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          modeTags = castingValue.map((elementcastingValue) -> ChipStructs.LaundryWasherModeClusterModeTagStruct.decodeTlv(elementcastingValue));
        }
      }
    }
    return new LaundryWasherModeClusterModeOptionStruct(
      label,
      mode,
      modeTags
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("LaundryWasherModeClusterModeOptionStruct {\n");
    output.append("\tlabel: ");
    output.append(label);
    output.append("\n");
    output.append("\tmode: ");
    output.append(mode);
    output.append("\n");
    output.append("\tmodeTags: ");
    output.append(modeTags);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class RefrigeratorAndTemperatureControlledCabinetModeClusterModeTagStruct {
  public Optional<Integer> mfgCode;
  public Integer value;
  private static final long MFG_CODE_ID = 0L;
  private static final long VALUE_ID = 1L;

  public RefrigeratorAndTemperatureControlledCabinetModeClusterModeTagStruct(
    Optional<Integer> mfgCode,
    Integer value
  ) {
    this.mfgCode = mfgCode;
    this.value = value;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(MFG_CODE_ID, mfgCode.<BaseTLVType>map((nonOptionalmfgCode) -> new UIntType(nonOptionalmfgCode)).orElse(new EmptyType())));
    values.add(new StructElement(VALUE_ID, new UIntType(value)));

    return new StructType(values);
  }

  public static RefrigeratorAndTemperatureControlledCabinetModeClusterModeTagStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Optional<Integer> mfgCode = Optional.empty();
    Integer value = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == MFG_CODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          mfgCode = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == VALUE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          value = castingValue.value(Integer.class);
        }
      }
    }
    return new RefrigeratorAndTemperatureControlledCabinetModeClusterModeTagStruct(
      mfgCode,
      value
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("RefrigeratorAndTemperatureControlledCabinetModeClusterModeTagStruct {\n");
    output.append("\tmfgCode: ");
    output.append(mfgCode);
    output.append("\n");
    output.append("\tvalue: ");
    output.append(value);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class RefrigeratorAndTemperatureControlledCabinetModeClusterModeOptionStruct {
  public String label;
  public Integer mode;
  public ArrayList<ChipStructs.RefrigeratorAndTemperatureControlledCabinetModeClusterModeTagStruct> modeTags;
  private static final long LABEL_ID = 0L;
  private static final long MODE_ID = 1L;
  private static final long MODE_TAGS_ID = 2L;

  public RefrigeratorAndTemperatureControlledCabinetModeClusterModeOptionStruct(
    String label,
    Integer mode,
    ArrayList<ChipStructs.RefrigeratorAndTemperatureControlledCabinetModeClusterModeTagStruct> modeTags
  ) {
    this.label = label;
    this.mode = mode;
    this.modeTags = modeTags;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(LABEL_ID, new StringType(label)));
    values.add(new StructElement(MODE_ID, new UIntType(mode)));
    values.add(new StructElement(MODE_TAGS_ID, ArrayType.generateArrayType(modeTags, (elementmodeTags) -> elementmodeTags.encodeTlv())));

    return new StructType(values);
  }

  public static RefrigeratorAndTemperatureControlledCabinetModeClusterModeOptionStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    String label = null;
    Integer mode = null;
    ArrayList<ChipStructs.RefrigeratorAndTemperatureControlledCabinetModeClusterModeTagStruct> modeTags = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == LABEL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          label = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == MODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          mode = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == MODE_TAGS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          modeTags = castingValue.map((elementcastingValue) -> ChipStructs.RefrigeratorAndTemperatureControlledCabinetModeClusterModeTagStruct.decodeTlv(elementcastingValue));
        }
      }
    }
    return new RefrigeratorAndTemperatureControlledCabinetModeClusterModeOptionStruct(
      label,
      mode,
      modeTags
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("RefrigeratorAndTemperatureControlledCabinetModeClusterModeOptionStruct {\n");
    output.append("\tlabel: ");
    output.append(label);
    output.append("\n");
    output.append("\tmode: ");
    output.append(mode);
    output.append("\n");
    output.append("\tmodeTags: ");
    output.append(modeTags);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class RvcRunModeClusterModeTagStruct {
  public Optional<Integer> mfgCode;
  public Integer value;
  private static final long MFG_CODE_ID = 0L;
  private static final long VALUE_ID = 1L;

  public RvcRunModeClusterModeTagStruct(
    Optional<Integer> mfgCode,
    Integer value
  ) {
    this.mfgCode = mfgCode;
    this.value = value;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(MFG_CODE_ID, mfgCode.<BaseTLVType>map((nonOptionalmfgCode) -> new UIntType(nonOptionalmfgCode)).orElse(new EmptyType())));
    values.add(new StructElement(VALUE_ID, new UIntType(value)));

    return new StructType(values);
  }

  public static RvcRunModeClusterModeTagStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Optional<Integer> mfgCode = Optional.empty();
    Integer value = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == MFG_CODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          mfgCode = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == VALUE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          value = castingValue.value(Integer.class);
        }
      }
    }
    return new RvcRunModeClusterModeTagStruct(
      mfgCode,
      value
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("RvcRunModeClusterModeTagStruct {\n");
    output.append("\tmfgCode: ");
    output.append(mfgCode);
    output.append("\n");
    output.append("\tvalue: ");
    output.append(value);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class RvcRunModeClusterModeOptionStruct {
  public String label;
  public Integer mode;
  public ArrayList<ChipStructs.RvcRunModeClusterModeTagStruct> modeTags;
  private static final long LABEL_ID = 0L;
  private static final long MODE_ID = 1L;
  private static final long MODE_TAGS_ID = 2L;

  public RvcRunModeClusterModeOptionStruct(
    String label,
    Integer mode,
    ArrayList<ChipStructs.RvcRunModeClusterModeTagStruct> modeTags
  ) {
    this.label = label;
    this.mode = mode;
    this.modeTags = modeTags;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(LABEL_ID, new StringType(label)));
    values.add(new StructElement(MODE_ID, new UIntType(mode)));
    values.add(new StructElement(MODE_TAGS_ID, ArrayType.generateArrayType(modeTags, (elementmodeTags) -> elementmodeTags.encodeTlv())));

    return new StructType(values);
  }

  public static RvcRunModeClusterModeOptionStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    String label = null;
    Integer mode = null;
    ArrayList<ChipStructs.RvcRunModeClusterModeTagStruct> modeTags = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == LABEL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          label = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == MODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          mode = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == MODE_TAGS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          modeTags = castingValue.map((elementcastingValue) -> ChipStructs.RvcRunModeClusterModeTagStruct.decodeTlv(elementcastingValue));
        }
      }
    }
    return new RvcRunModeClusterModeOptionStruct(
      label,
      mode,
      modeTags
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("RvcRunModeClusterModeOptionStruct {\n");
    output.append("\tlabel: ");
    output.append(label);
    output.append("\n");
    output.append("\tmode: ");
    output.append(mode);
    output.append("\n");
    output.append("\tmodeTags: ");
    output.append(modeTags);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class RvcCleanModeClusterModeTagStruct {
  public Optional<Integer> mfgCode;
  public Integer value;
  private static final long MFG_CODE_ID = 0L;
  private static final long VALUE_ID = 1L;

  public RvcCleanModeClusterModeTagStruct(
    Optional<Integer> mfgCode,
    Integer value
  ) {
    this.mfgCode = mfgCode;
    this.value = value;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(MFG_CODE_ID, mfgCode.<BaseTLVType>map((nonOptionalmfgCode) -> new UIntType(nonOptionalmfgCode)).orElse(new EmptyType())));
    values.add(new StructElement(VALUE_ID, new UIntType(value)));

    return new StructType(values);
  }

  public static RvcCleanModeClusterModeTagStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Optional<Integer> mfgCode = Optional.empty();
    Integer value = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == MFG_CODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          mfgCode = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == VALUE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          value = castingValue.value(Integer.class);
        }
      }
    }
    return new RvcCleanModeClusterModeTagStruct(
      mfgCode,
      value
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("RvcCleanModeClusterModeTagStruct {\n");
    output.append("\tmfgCode: ");
    output.append(mfgCode);
    output.append("\n");
    output.append("\tvalue: ");
    output.append(value);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class RvcCleanModeClusterModeOptionStruct {
  public String label;
  public Integer mode;
  public ArrayList<ChipStructs.RvcCleanModeClusterModeTagStruct> modeTags;
  private static final long LABEL_ID = 0L;
  private static final long MODE_ID = 1L;
  private static final long MODE_TAGS_ID = 2L;

  public RvcCleanModeClusterModeOptionStruct(
    String label,
    Integer mode,
    ArrayList<ChipStructs.RvcCleanModeClusterModeTagStruct> modeTags
  ) {
    this.label = label;
    this.mode = mode;
    this.modeTags = modeTags;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(LABEL_ID, new StringType(label)));
    values.add(new StructElement(MODE_ID, new UIntType(mode)));
    values.add(new StructElement(MODE_TAGS_ID, ArrayType.generateArrayType(modeTags, (elementmodeTags) -> elementmodeTags.encodeTlv())));

    return new StructType(values);
  }

  public static RvcCleanModeClusterModeOptionStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    String label = null;
    Integer mode = null;
    ArrayList<ChipStructs.RvcCleanModeClusterModeTagStruct> modeTags = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == LABEL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          label = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == MODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          mode = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == MODE_TAGS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          modeTags = castingValue.map((elementcastingValue) -> ChipStructs.RvcCleanModeClusterModeTagStruct.decodeTlv(elementcastingValue));
        }
      }
    }
    return new RvcCleanModeClusterModeOptionStruct(
      label,
      mode,
      modeTags
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("RvcCleanModeClusterModeOptionStruct {\n");
    output.append("\tlabel: ");
    output.append(label);
    output.append("\n");
    output.append("\tmode: ");
    output.append(mode);
    output.append("\n");
    output.append("\tmodeTags: ");
    output.append(modeTags);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class DishwasherModeClusterModeTagStruct {
  public Optional<Integer> mfgCode;
  public Integer value;
  private static final long MFG_CODE_ID = 0L;
  private static final long VALUE_ID = 1L;

  public DishwasherModeClusterModeTagStruct(
    Optional<Integer> mfgCode,
    Integer value
  ) {
    this.mfgCode = mfgCode;
    this.value = value;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(MFG_CODE_ID, mfgCode.<BaseTLVType>map((nonOptionalmfgCode) -> new UIntType(nonOptionalmfgCode)).orElse(new EmptyType())));
    values.add(new StructElement(VALUE_ID, new UIntType(value)));

    return new StructType(values);
  }

  public static DishwasherModeClusterModeTagStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Optional<Integer> mfgCode = Optional.empty();
    Integer value = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == MFG_CODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          mfgCode = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == VALUE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          value = castingValue.value(Integer.class);
        }
      }
    }
    return new DishwasherModeClusterModeTagStruct(
      mfgCode,
      value
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("DishwasherModeClusterModeTagStruct {\n");
    output.append("\tmfgCode: ");
    output.append(mfgCode);
    output.append("\n");
    output.append("\tvalue: ");
    output.append(value);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class DishwasherModeClusterModeOptionStruct {
  public String label;
  public Integer mode;
  public ArrayList<ChipStructs.DishwasherModeClusterModeTagStruct> modeTags;
  private static final long LABEL_ID = 0L;
  private static final long MODE_ID = 1L;
  private static final long MODE_TAGS_ID = 2L;

  public DishwasherModeClusterModeOptionStruct(
    String label,
    Integer mode,
    ArrayList<ChipStructs.DishwasherModeClusterModeTagStruct> modeTags
  ) {
    this.label = label;
    this.mode = mode;
    this.modeTags = modeTags;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(LABEL_ID, new StringType(label)));
    values.add(new StructElement(MODE_ID, new UIntType(mode)));
    values.add(new StructElement(MODE_TAGS_ID, ArrayType.generateArrayType(modeTags, (elementmodeTags) -> elementmodeTags.encodeTlv())));

    return new StructType(values);
  }

  public static DishwasherModeClusterModeOptionStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    String label = null;
    Integer mode = null;
    ArrayList<ChipStructs.DishwasherModeClusterModeTagStruct> modeTags = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == LABEL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          label = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == MODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          mode = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == MODE_TAGS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          modeTags = castingValue.map((elementcastingValue) -> ChipStructs.DishwasherModeClusterModeTagStruct.decodeTlv(elementcastingValue));
        }
      }
    }
    return new DishwasherModeClusterModeOptionStruct(
      label,
      mode,
      modeTags
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("DishwasherModeClusterModeOptionStruct {\n");
    output.append("\tlabel: ");
    output.append(label);
    output.append("\n");
    output.append("\tmode: ");
    output.append(mode);
    output.append("\n");
    output.append("\tmodeTags: ");
    output.append(modeTags);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class MicrowaveOvenModeClusterModeTagStruct {
  public Optional<Integer> mfgCode;
  public Integer value;
  private static final long MFG_CODE_ID = 0L;
  private static final long VALUE_ID = 1L;

  public MicrowaveOvenModeClusterModeTagStruct(
    Optional<Integer> mfgCode,
    Integer value
  ) {
    this.mfgCode = mfgCode;
    this.value = value;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(MFG_CODE_ID, mfgCode.<BaseTLVType>map((nonOptionalmfgCode) -> new UIntType(nonOptionalmfgCode)).orElse(new EmptyType())));
    values.add(new StructElement(VALUE_ID, new UIntType(value)));

    return new StructType(values);
  }

  public static MicrowaveOvenModeClusterModeTagStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Optional<Integer> mfgCode = Optional.empty();
    Integer value = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == MFG_CODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          mfgCode = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == VALUE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          value = castingValue.value(Integer.class);
        }
      }
    }
    return new MicrowaveOvenModeClusterModeTagStruct(
      mfgCode,
      value
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("MicrowaveOvenModeClusterModeTagStruct {\n");
    output.append("\tmfgCode: ");
    output.append(mfgCode);
    output.append("\n");
    output.append("\tvalue: ");
    output.append(value);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class MicrowaveOvenModeClusterModeOptionStruct {
  public String label;
  public Integer mode;
  public ArrayList<ChipStructs.MicrowaveOvenModeClusterModeTagStruct> modeTags;
  private static final long LABEL_ID = 0L;
  private static final long MODE_ID = 1L;
  private static final long MODE_TAGS_ID = 2L;

  public MicrowaveOvenModeClusterModeOptionStruct(
    String label,
    Integer mode,
    ArrayList<ChipStructs.MicrowaveOvenModeClusterModeTagStruct> modeTags
  ) {
    this.label = label;
    this.mode = mode;
    this.modeTags = modeTags;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(LABEL_ID, new StringType(label)));
    values.add(new StructElement(MODE_ID, new UIntType(mode)));
    values.add(new StructElement(MODE_TAGS_ID, ArrayType.generateArrayType(modeTags, (elementmodeTags) -> elementmodeTags.encodeTlv())));

    return new StructType(values);
  }

  public static MicrowaveOvenModeClusterModeOptionStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    String label = null;
    Integer mode = null;
    ArrayList<ChipStructs.MicrowaveOvenModeClusterModeTagStruct> modeTags = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == LABEL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          label = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == MODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          mode = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == MODE_TAGS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          modeTags = castingValue.map((elementcastingValue) -> ChipStructs.MicrowaveOvenModeClusterModeTagStruct.decodeTlv(elementcastingValue));
        }
      }
    }
    return new MicrowaveOvenModeClusterModeOptionStruct(
      label,
      mode,
      modeTags
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("MicrowaveOvenModeClusterModeOptionStruct {\n");
    output.append("\tlabel: ");
    output.append(label);
    output.append("\n");
    output.append("\tmode: ");
    output.append(mode);
    output.append("\n");
    output.append("\tmodeTags: ");
    output.append(modeTags);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class OperationalStateClusterErrorStateStruct {
  public Integer errorStateID;
  public Optional<String> errorStateLabel;
  public Optional<String> errorStateDetails;
  private static final long ERROR_STATE_ID_ID = 0L;
  private static final long ERROR_STATE_LABEL_ID = 1L;
  private static final long ERROR_STATE_DETAILS_ID = 2L;

  public OperationalStateClusterErrorStateStruct(
    Integer errorStateID,
    Optional<String> errorStateLabel,
    Optional<String> errorStateDetails
  ) {
    this.errorStateID = errorStateID;
    this.errorStateLabel = errorStateLabel;
    this.errorStateDetails = errorStateDetails;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ERROR_STATE_ID_ID, new UIntType(errorStateID)));
    values.add(new StructElement(ERROR_STATE_LABEL_ID, errorStateLabel.<BaseTLVType>map((nonOptionalerrorStateLabel) -> new StringType(nonOptionalerrorStateLabel)).orElse(new EmptyType())));
    values.add(new StructElement(ERROR_STATE_DETAILS_ID, errorStateDetails.<BaseTLVType>map((nonOptionalerrorStateDetails) -> new StringType(nonOptionalerrorStateDetails)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static OperationalStateClusterErrorStateStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer errorStateID = null;
    Optional<String> errorStateLabel = Optional.empty();
    Optional<String> errorStateDetails = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ERROR_STATE_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          errorStateID = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == ERROR_STATE_LABEL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          errorStateLabel = Optional.of(castingValue.value(String.class));
        }
      } else if (element.contextTagNum() == ERROR_STATE_DETAILS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          errorStateDetails = Optional.of(castingValue.value(String.class));
        }
      }
    }
    return new OperationalStateClusterErrorStateStruct(
      errorStateID,
      errorStateLabel,
      errorStateDetails
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("OperationalStateClusterErrorStateStruct {\n");
    output.append("\terrorStateID: ");
    output.append(errorStateID);
    output.append("\n");
    output.append("\terrorStateLabel: ");
    output.append(errorStateLabel);
    output.append("\n");
    output.append("\terrorStateDetails: ");
    output.append(errorStateDetails);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class OperationalStateClusterOperationalStateStruct {
  public Integer operationalStateID;
  public Optional<String> operationalStateLabel;
  private static final long OPERATIONAL_STATE_ID_ID = 0L;
  private static final long OPERATIONAL_STATE_LABEL_ID = 1L;

  public OperationalStateClusterOperationalStateStruct(
    Integer operationalStateID,
    Optional<String> operationalStateLabel
  ) {
    this.operationalStateID = operationalStateID;
    this.operationalStateLabel = operationalStateLabel;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(OPERATIONAL_STATE_ID_ID, new UIntType(operationalStateID)));
    values.add(new StructElement(OPERATIONAL_STATE_LABEL_ID, operationalStateLabel.<BaseTLVType>map((nonOptionaloperationalStateLabel) -> new StringType(nonOptionaloperationalStateLabel)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static OperationalStateClusterOperationalStateStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer operationalStateID = null;
    Optional<String> operationalStateLabel = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == OPERATIONAL_STATE_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          operationalStateID = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == OPERATIONAL_STATE_LABEL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          operationalStateLabel = Optional.of(castingValue.value(String.class));
        }
      }
    }
    return new OperationalStateClusterOperationalStateStruct(
      operationalStateID,
      operationalStateLabel
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("OperationalStateClusterOperationalStateStruct {\n");
    output.append("\toperationalStateID: ");
    output.append(operationalStateID);
    output.append("\n");
    output.append("\toperationalStateLabel: ");
    output.append(operationalStateLabel);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class RvcOperationalStateClusterErrorStateStruct {
  public Integer errorStateID;
  public Optional<String> errorStateLabel;
  public Optional<String> errorStateDetails;
  private static final long ERROR_STATE_ID_ID = 0L;
  private static final long ERROR_STATE_LABEL_ID = 1L;
  private static final long ERROR_STATE_DETAILS_ID = 2L;

  public RvcOperationalStateClusterErrorStateStruct(
    Integer errorStateID,
    Optional<String> errorStateLabel,
    Optional<String> errorStateDetails
  ) {
    this.errorStateID = errorStateID;
    this.errorStateLabel = errorStateLabel;
    this.errorStateDetails = errorStateDetails;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ERROR_STATE_ID_ID, new UIntType(errorStateID)));
    values.add(new StructElement(ERROR_STATE_LABEL_ID, errorStateLabel.<BaseTLVType>map((nonOptionalerrorStateLabel) -> new StringType(nonOptionalerrorStateLabel)).orElse(new EmptyType())));
    values.add(new StructElement(ERROR_STATE_DETAILS_ID, errorStateDetails.<BaseTLVType>map((nonOptionalerrorStateDetails) -> new StringType(nonOptionalerrorStateDetails)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static RvcOperationalStateClusterErrorStateStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer errorStateID = null;
    Optional<String> errorStateLabel = Optional.empty();
    Optional<String> errorStateDetails = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ERROR_STATE_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          errorStateID = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == ERROR_STATE_LABEL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          errorStateLabel = Optional.of(castingValue.value(String.class));
        }
      } else if (element.contextTagNum() == ERROR_STATE_DETAILS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          errorStateDetails = Optional.of(castingValue.value(String.class));
        }
      }
    }
    return new RvcOperationalStateClusterErrorStateStruct(
      errorStateID,
      errorStateLabel,
      errorStateDetails
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("RvcOperationalStateClusterErrorStateStruct {\n");
    output.append("\terrorStateID: ");
    output.append(errorStateID);
    output.append("\n");
    output.append("\terrorStateLabel: ");
    output.append(errorStateLabel);
    output.append("\n");
    output.append("\terrorStateDetails: ");
    output.append(errorStateDetails);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class RvcOperationalStateClusterOperationalStateStruct {
  public Integer operationalStateID;
  public Optional<String> operationalStateLabel;
  private static final long OPERATIONAL_STATE_ID_ID = 0L;
  private static final long OPERATIONAL_STATE_LABEL_ID = 1L;

  public RvcOperationalStateClusterOperationalStateStruct(
    Integer operationalStateID,
    Optional<String> operationalStateLabel
  ) {
    this.operationalStateID = operationalStateID;
    this.operationalStateLabel = operationalStateLabel;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(OPERATIONAL_STATE_ID_ID, new UIntType(operationalStateID)));
    values.add(new StructElement(OPERATIONAL_STATE_LABEL_ID, operationalStateLabel.<BaseTLVType>map((nonOptionaloperationalStateLabel) -> new StringType(nonOptionaloperationalStateLabel)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static RvcOperationalStateClusterOperationalStateStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer operationalStateID = null;
    Optional<String> operationalStateLabel = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == OPERATIONAL_STATE_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          operationalStateID = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == OPERATIONAL_STATE_LABEL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          operationalStateLabel = Optional.of(castingValue.value(String.class));
        }
      }
    }
    return new RvcOperationalStateClusterOperationalStateStruct(
      operationalStateID,
      operationalStateLabel
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("RvcOperationalStateClusterOperationalStateStruct {\n");
    output.append("\toperationalStateID: ");
    output.append(operationalStateID);
    output.append("\n");
    output.append("\toperationalStateLabel: ");
    output.append(operationalStateLabel);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ScenesManagementClusterAttributeValuePairStruct {
  public Long attributeID;
  public Optional<Integer> valueUnsigned8;
  public Optional<Integer> valueSigned8;
  public Optional<Integer> valueUnsigned16;
  public Optional<Integer> valueSigned16;
  public Optional<Long> valueUnsigned32;
  public Optional<Long> valueSigned32;
  public Optional<Long> valueUnsigned64;
  public Optional<Long> valueSigned64;
  private static final long ATTRIBUTE_ID_ID = 0L;
  private static final long VALUE_UNSIGNED8_ID = 1L;
  private static final long VALUE_SIGNED8_ID = 2L;
  private static final long VALUE_UNSIGNED16_ID = 3L;
  private static final long VALUE_SIGNED16_ID = 4L;
  private static final long VALUE_UNSIGNED32_ID = 5L;
  private static final long VALUE_SIGNED32_ID = 6L;
  private static final long VALUE_UNSIGNED64_ID = 7L;
  private static final long VALUE_SIGNED64_ID = 8L;

  public ScenesManagementClusterAttributeValuePairStruct(
    Long attributeID,
    Optional<Integer> valueUnsigned8,
    Optional<Integer> valueSigned8,
    Optional<Integer> valueUnsigned16,
    Optional<Integer> valueSigned16,
    Optional<Long> valueUnsigned32,
    Optional<Long> valueSigned32,
    Optional<Long> valueUnsigned64,
    Optional<Long> valueSigned64
  ) {
    this.attributeID = attributeID;
    this.valueUnsigned8 = valueUnsigned8;
    this.valueSigned8 = valueSigned8;
    this.valueUnsigned16 = valueUnsigned16;
    this.valueSigned16 = valueSigned16;
    this.valueUnsigned32 = valueUnsigned32;
    this.valueSigned32 = valueSigned32;
    this.valueUnsigned64 = valueUnsigned64;
    this.valueSigned64 = valueSigned64;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ATTRIBUTE_ID_ID, new UIntType(attributeID)));
    values.add(new StructElement(VALUE_UNSIGNED8_ID, valueUnsigned8.<BaseTLVType>map((nonOptionalvalueUnsigned8) -> new UIntType(nonOptionalvalueUnsigned8)).orElse(new EmptyType())));
    values.add(new StructElement(VALUE_SIGNED8_ID, valueSigned8.<BaseTLVType>map((nonOptionalvalueSigned8) -> new IntType(nonOptionalvalueSigned8)).orElse(new EmptyType())));
    values.add(new StructElement(VALUE_UNSIGNED16_ID, valueUnsigned16.<BaseTLVType>map((nonOptionalvalueUnsigned16) -> new UIntType(nonOptionalvalueUnsigned16)).orElse(new EmptyType())));
    values.add(new StructElement(VALUE_SIGNED16_ID, valueSigned16.<BaseTLVType>map((nonOptionalvalueSigned16) -> new IntType(nonOptionalvalueSigned16)).orElse(new EmptyType())));
    values.add(new StructElement(VALUE_UNSIGNED32_ID, valueUnsigned32.<BaseTLVType>map((nonOptionalvalueUnsigned32) -> new UIntType(nonOptionalvalueUnsigned32)).orElse(new EmptyType())));
    values.add(new StructElement(VALUE_SIGNED32_ID, valueSigned32.<BaseTLVType>map((nonOptionalvalueSigned32) -> new IntType(nonOptionalvalueSigned32)).orElse(new EmptyType())));
    values.add(new StructElement(VALUE_UNSIGNED64_ID, valueUnsigned64.<BaseTLVType>map((nonOptionalvalueUnsigned64) -> new UIntType(nonOptionalvalueUnsigned64)).orElse(new EmptyType())));
    values.add(new StructElement(VALUE_SIGNED64_ID, valueSigned64.<BaseTLVType>map((nonOptionalvalueSigned64) -> new IntType(nonOptionalvalueSigned64)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static ScenesManagementClusterAttributeValuePairStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long attributeID = null;
    Optional<Integer> valueUnsigned8 = Optional.empty();
    Optional<Integer> valueSigned8 = Optional.empty();
    Optional<Integer> valueUnsigned16 = Optional.empty();
    Optional<Integer> valueSigned16 = Optional.empty();
    Optional<Long> valueUnsigned32 = Optional.empty();
    Optional<Long> valueSigned32 = Optional.empty();
    Optional<Long> valueUnsigned64 = Optional.empty();
    Optional<Long> valueSigned64 = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ATTRIBUTE_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          attributeID = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == VALUE_UNSIGNED8_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          valueUnsigned8 = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == VALUE_SIGNED8_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          valueSigned8 = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == VALUE_UNSIGNED16_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          valueUnsigned16 = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == VALUE_SIGNED16_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          valueSigned16 = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == VALUE_UNSIGNED32_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          valueUnsigned32 = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == VALUE_SIGNED32_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          valueSigned32 = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == VALUE_UNSIGNED64_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          valueUnsigned64 = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == VALUE_SIGNED64_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          valueSigned64 = Optional.of(castingValue.value(Long.class));
        }
      }
    }
    return new ScenesManagementClusterAttributeValuePairStruct(
      attributeID,
      valueUnsigned8,
      valueSigned8,
      valueUnsigned16,
      valueSigned16,
      valueUnsigned32,
      valueSigned32,
      valueUnsigned64,
      valueSigned64
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ScenesManagementClusterAttributeValuePairStruct {\n");
    output.append("\tattributeID: ");
    output.append(attributeID);
    output.append("\n");
    output.append("\tvalueUnsigned8: ");
    output.append(valueUnsigned8);
    output.append("\n");
    output.append("\tvalueSigned8: ");
    output.append(valueSigned8);
    output.append("\n");
    output.append("\tvalueUnsigned16: ");
    output.append(valueUnsigned16);
    output.append("\n");
    output.append("\tvalueSigned16: ");
    output.append(valueSigned16);
    output.append("\n");
    output.append("\tvalueUnsigned32: ");
    output.append(valueUnsigned32);
    output.append("\n");
    output.append("\tvalueSigned32: ");
    output.append(valueSigned32);
    output.append("\n");
    output.append("\tvalueUnsigned64: ");
    output.append(valueUnsigned64);
    output.append("\n");
    output.append("\tvalueSigned64: ");
    output.append(valueSigned64);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ScenesManagementClusterExtensionFieldSet {
  public Long clusterID;
  public ArrayList<ChipStructs.ScenesManagementClusterAttributeValuePairStruct> attributeValueList;
  private static final long CLUSTER_ID_ID = 0L;
  private static final long ATTRIBUTE_VALUE_LIST_ID = 1L;

  public ScenesManagementClusterExtensionFieldSet(
    Long clusterID,
    ArrayList<ChipStructs.ScenesManagementClusterAttributeValuePairStruct> attributeValueList
  ) {
    this.clusterID = clusterID;
    this.attributeValueList = attributeValueList;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(CLUSTER_ID_ID, new UIntType(clusterID)));
    values.add(new StructElement(ATTRIBUTE_VALUE_LIST_ID, ArrayType.generateArrayType(attributeValueList, (elementattributeValueList) -> elementattributeValueList.encodeTlv())));

    return new StructType(values);
  }

  public static ScenesManagementClusterExtensionFieldSet decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long clusterID = null;
    ArrayList<ChipStructs.ScenesManagementClusterAttributeValuePairStruct> attributeValueList = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == CLUSTER_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          clusterID = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == ATTRIBUTE_VALUE_LIST_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          attributeValueList = castingValue.map((elementcastingValue) -> ChipStructs.ScenesManagementClusterAttributeValuePairStruct.decodeTlv(elementcastingValue));
        }
      }
    }
    return new ScenesManagementClusterExtensionFieldSet(
      clusterID,
      attributeValueList
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ScenesManagementClusterExtensionFieldSet {\n");
    output.append("\tclusterID: ");
    output.append(clusterID);
    output.append("\n");
    output.append("\tattributeValueList: ");
    output.append(attributeValueList);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ScenesManagementClusterSceneInfoStruct {
  public Integer sceneCount;
  public Integer currentScene;
  public Integer currentGroup;
  public Boolean sceneValid;
  public Integer remainingCapacity;
  public Integer fabricIndex;
  private static final long SCENE_COUNT_ID = 0L;
  private static final long CURRENT_SCENE_ID = 1L;
  private static final long CURRENT_GROUP_ID = 2L;
  private static final long SCENE_VALID_ID = 3L;
  private static final long REMAINING_CAPACITY_ID = 4L;
  private static final long FABRIC_INDEX_ID = 254L;

  public ScenesManagementClusterSceneInfoStruct(
    Integer sceneCount,
    Integer currentScene,
    Integer currentGroup,
    Boolean sceneValid,
    Integer remainingCapacity,
    Integer fabricIndex
  ) {
    this.sceneCount = sceneCount;
    this.currentScene = currentScene;
    this.currentGroup = currentGroup;
    this.sceneValid = sceneValid;
    this.remainingCapacity = remainingCapacity;
    this.fabricIndex = fabricIndex;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(SCENE_COUNT_ID, new UIntType(sceneCount)));
    values.add(new StructElement(CURRENT_SCENE_ID, new UIntType(currentScene)));
    values.add(new StructElement(CURRENT_GROUP_ID, new UIntType(currentGroup)));
    values.add(new StructElement(SCENE_VALID_ID, new BooleanType(sceneValid)));
    values.add(new StructElement(REMAINING_CAPACITY_ID, new UIntType(remainingCapacity)));
    values.add(new StructElement(FABRIC_INDEX_ID, new UIntType(fabricIndex)));

    return new StructType(values);
  }

  public static ScenesManagementClusterSceneInfoStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer sceneCount = null;
    Integer currentScene = null;
    Integer currentGroup = null;
    Boolean sceneValid = null;
    Integer remainingCapacity = null;
    Integer fabricIndex = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == SCENE_COUNT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          sceneCount = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == CURRENT_SCENE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          currentScene = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == CURRENT_GROUP_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          currentGroup = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == SCENE_VALID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          sceneValid = castingValue.value(Boolean.class);
        }
      } else if (element.contextTagNum() == REMAINING_CAPACITY_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          remainingCapacity = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == FABRIC_INDEX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          fabricIndex = castingValue.value(Integer.class);
        }
      }
    }
    return new ScenesManagementClusterSceneInfoStruct(
      sceneCount,
      currentScene,
      currentGroup,
      sceneValid,
      remainingCapacity,
      fabricIndex
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ScenesManagementClusterSceneInfoStruct {\n");
    output.append("\tsceneCount: ");
    output.append(sceneCount);
    output.append("\n");
    output.append("\tcurrentScene: ");
    output.append(currentScene);
    output.append("\n");
    output.append("\tcurrentGroup: ");
    output.append(currentGroup);
    output.append("\n");
    output.append("\tsceneValid: ");
    output.append(sceneValid);
    output.append("\n");
    output.append("\tremainingCapacity: ");
    output.append(remainingCapacity);
    output.append("\n");
    output.append("\tfabricIndex: ");
    output.append(fabricIndex);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class HepaFilterMonitoringClusterReplacementProductStruct {
  public Integer productIdentifierType;
  public String productIdentifierValue;
  private static final long PRODUCT_IDENTIFIER_TYPE_ID = 0L;
  private static final long PRODUCT_IDENTIFIER_VALUE_ID = 1L;

  public HepaFilterMonitoringClusterReplacementProductStruct(
    Integer productIdentifierType,
    String productIdentifierValue
  ) {
    this.productIdentifierType = productIdentifierType;
    this.productIdentifierValue = productIdentifierValue;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(PRODUCT_IDENTIFIER_TYPE_ID, new UIntType(productIdentifierType)));
    values.add(new StructElement(PRODUCT_IDENTIFIER_VALUE_ID, new StringType(productIdentifierValue)));

    return new StructType(values);
  }

  public static HepaFilterMonitoringClusterReplacementProductStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer productIdentifierType = null;
    String productIdentifierValue = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == PRODUCT_IDENTIFIER_TYPE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          productIdentifierType = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == PRODUCT_IDENTIFIER_VALUE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          productIdentifierValue = castingValue.value(String.class);
        }
      }
    }
    return new HepaFilterMonitoringClusterReplacementProductStruct(
      productIdentifierType,
      productIdentifierValue
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("HepaFilterMonitoringClusterReplacementProductStruct {\n");
    output.append("\tproductIdentifierType: ");
    output.append(productIdentifierType);
    output.append("\n");
    output.append("\tproductIdentifierValue: ");
    output.append(productIdentifierValue);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ActivatedCarbonFilterMonitoringClusterReplacementProductStruct {
  public Integer productIdentifierType;
  public String productIdentifierValue;
  private static final long PRODUCT_IDENTIFIER_TYPE_ID = 0L;
  private static final long PRODUCT_IDENTIFIER_VALUE_ID = 1L;

  public ActivatedCarbonFilterMonitoringClusterReplacementProductStruct(
    Integer productIdentifierType,
    String productIdentifierValue
  ) {
    this.productIdentifierType = productIdentifierType;
    this.productIdentifierValue = productIdentifierValue;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(PRODUCT_IDENTIFIER_TYPE_ID, new UIntType(productIdentifierType)));
    values.add(new StructElement(PRODUCT_IDENTIFIER_VALUE_ID, new StringType(productIdentifierValue)));

    return new StructType(values);
  }

  public static ActivatedCarbonFilterMonitoringClusterReplacementProductStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer productIdentifierType = null;
    String productIdentifierValue = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == PRODUCT_IDENTIFIER_TYPE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          productIdentifierType = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == PRODUCT_IDENTIFIER_VALUE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          productIdentifierValue = castingValue.value(String.class);
        }
      }
    }
    return new ActivatedCarbonFilterMonitoringClusterReplacementProductStruct(
      productIdentifierType,
      productIdentifierValue
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ActivatedCarbonFilterMonitoringClusterReplacementProductStruct {\n");
    output.append("\tproductIdentifierType: ");
    output.append(productIdentifierType);
    output.append("\n");
    output.append("\tproductIdentifierValue: ");
    output.append(productIdentifierValue);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ElectricalPowerMeasurementClusterMeasurementAccuracyRangeStruct {
  public Long rangeMin;
  public Long rangeMax;
  public Optional<Integer> percentMax;
  public Optional<Integer> percentMin;
  public Optional<Integer> percentTypical;
  public Optional<Long> fixedMax;
  public Optional<Long> fixedMin;
  public Optional<Long> fixedTypical;
  private static final long RANGE_MIN_ID = 0L;
  private static final long RANGE_MAX_ID = 1L;
  private static final long PERCENT_MAX_ID = 2L;
  private static final long PERCENT_MIN_ID = 3L;
  private static final long PERCENT_TYPICAL_ID = 4L;
  private static final long FIXED_MAX_ID = 5L;
  private static final long FIXED_MIN_ID = 6L;
  private static final long FIXED_TYPICAL_ID = 7L;

  public ElectricalPowerMeasurementClusterMeasurementAccuracyRangeStruct(
    Long rangeMin,
    Long rangeMax,
    Optional<Integer> percentMax,
    Optional<Integer> percentMin,
    Optional<Integer> percentTypical,
    Optional<Long> fixedMax,
    Optional<Long> fixedMin,
    Optional<Long> fixedTypical
  ) {
    this.rangeMin = rangeMin;
    this.rangeMax = rangeMax;
    this.percentMax = percentMax;
    this.percentMin = percentMin;
    this.percentTypical = percentTypical;
    this.fixedMax = fixedMax;
    this.fixedMin = fixedMin;
    this.fixedTypical = fixedTypical;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(RANGE_MIN_ID, new IntType(rangeMin)));
    values.add(new StructElement(RANGE_MAX_ID, new IntType(rangeMax)));
    values.add(new StructElement(PERCENT_MAX_ID, percentMax.<BaseTLVType>map((nonOptionalpercentMax) -> new UIntType(nonOptionalpercentMax)).orElse(new EmptyType())));
    values.add(new StructElement(PERCENT_MIN_ID, percentMin.<BaseTLVType>map((nonOptionalpercentMin) -> new UIntType(nonOptionalpercentMin)).orElse(new EmptyType())));
    values.add(new StructElement(PERCENT_TYPICAL_ID, percentTypical.<BaseTLVType>map((nonOptionalpercentTypical) -> new UIntType(nonOptionalpercentTypical)).orElse(new EmptyType())));
    values.add(new StructElement(FIXED_MAX_ID, fixedMax.<BaseTLVType>map((nonOptionalfixedMax) -> new UIntType(nonOptionalfixedMax)).orElse(new EmptyType())));
    values.add(new StructElement(FIXED_MIN_ID, fixedMin.<BaseTLVType>map((nonOptionalfixedMin) -> new UIntType(nonOptionalfixedMin)).orElse(new EmptyType())));
    values.add(new StructElement(FIXED_TYPICAL_ID, fixedTypical.<BaseTLVType>map((nonOptionalfixedTypical) -> new UIntType(nonOptionalfixedTypical)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static ElectricalPowerMeasurementClusterMeasurementAccuracyRangeStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long rangeMin = null;
    Long rangeMax = null;
    Optional<Integer> percentMax = Optional.empty();
    Optional<Integer> percentMin = Optional.empty();
    Optional<Integer> percentTypical = Optional.empty();
    Optional<Long> fixedMax = Optional.empty();
    Optional<Long> fixedMin = Optional.empty();
    Optional<Long> fixedTypical = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == RANGE_MIN_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          rangeMin = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == RANGE_MAX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          rangeMax = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == PERCENT_MAX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          percentMax = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == PERCENT_MIN_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          percentMin = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == PERCENT_TYPICAL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          percentTypical = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == FIXED_MAX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          fixedMax = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == FIXED_MIN_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          fixedMin = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == FIXED_TYPICAL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          fixedTypical = Optional.of(castingValue.value(Long.class));
        }
      }
    }
    return new ElectricalPowerMeasurementClusterMeasurementAccuracyRangeStruct(
      rangeMin,
      rangeMax,
      percentMax,
      percentMin,
      percentTypical,
      fixedMax,
      fixedMin,
      fixedTypical
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ElectricalPowerMeasurementClusterMeasurementAccuracyRangeStruct {\n");
    output.append("\trangeMin: ");
    output.append(rangeMin);
    output.append("\n");
    output.append("\trangeMax: ");
    output.append(rangeMax);
    output.append("\n");
    output.append("\tpercentMax: ");
    output.append(percentMax);
    output.append("\n");
    output.append("\tpercentMin: ");
    output.append(percentMin);
    output.append("\n");
    output.append("\tpercentTypical: ");
    output.append(percentTypical);
    output.append("\n");
    output.append("\tfixedMax: ");
    output.append(fixedMax);
    output.append("\n");
    output.append("\tfixedMin: ");
    output.append(fixedMin);
    output.append("\n");
    output.append("\tfixedTypical: ");
    output.append(fixedTypical);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ElectricalPowerMeasurementClusterMeasurementAccuracyStruct {
  public Integer measurementType;
  public Boolean measured;
  public Long minMeasuredValue;
  public Long maxMeasuredValue;
  public ArrayList<ChipStructs.ElectricalPowerMeasurementClusterMeasurementAccuracyRangeStruct> accuracyRanges;
  private static final long MEASUREMENT_TYPE_ID = 0L;
  private static final long MEASURED_ID = 1L;
  private static final long MIN_MEASURED_VALUE_ID = 2L;
  private static final long MAX_MEASURED_VALUE_ID = 3L;
  private static final long ACCURACY_RANGES_ID = 4L;

  public ElectricalPowerMeasurementClusterMeasurementAccuracyStruct(
    Integer measurementType,
    Boolean measured,
    Long minMeasuredValue,
    Long maxMeasuredValue,
    ArrayList<ChipStructs.ElectricalPowerMeasurementClusterMeasurementAccuracyRangeStruct> accuracyRanges
  ) {
    this.measurementType = measurementType;
    this.measured = measured;
    this.minMeasuredValue = minMeasuredValue;
    this.maxMeasuredValue = maxMeasuredValue;
    this.accuracyRanges = accuracyRanges;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(MEASUREMENT_TYPE_ID, new UIntType(measurementType)));
    values.add(new StructElement(MEASURED_ID, new BooleanType(measured)));
    values.add(new StructElement(MIN_MEASURED_VALUE_ID, new IntType(minMeasuredValue)));
    values.add(new StructElement(MAX_MEASURED_VALUE_ID, new IntType(maxMeasuredValue)));
    values.add(new StructElement(ACCURACY_RANGES_ID, ArrayType.generateArrayType(accuracyRanges, (elementaccuracyRanges) -> elementaccuracyRanges.encodeTlv())));

    return new StructType(values);
  }

  public static ElectricalPowerMeasurementClusterMeasurementAccuracyStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer measurementType = null;
    Boolean measured = null;
    Long minMeasuredValue = null;
    Long maxMeasuredValue = null;
    ArrayList<ChipStructs.ElectricalPowerMeasurementClusterMeasurementAccuracyRangeStruct> accuracyRanges = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == MEASUREMENT_TYPE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          measurementType = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == MEASURED_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          measured = castingValue.value(Boolean.class);
        }
      } else if (element.contextTagNum() == MIN_MEASURED_VALUE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          minMeasuredValue = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == MAX_MEASURED_VALUE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          maxMeasuredValue = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == ACCURACY_RANGES_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          accuracyRanges = castingValue.map((elementcastingValue) -> ChipStructs.ElectricalPowerMeasurementClusterMeasurementAccuracyRangeStruct.decodeTlv(elementcastingValue));
        }
      }
    }
    return new ElectricalPowerMeasurementClusterMeasurementAccuracyStruct(
      measurementType,
      measured,
      minMeasuredValue,
      maxMeasuredValue,
      accuracyRanges
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ElectricalPowerMeasurementClusterMeasurementAccuracyStruct {\n");
    output.append("\tmeasurementType: ");
    output.append(measurementType);
    output.append("\n");
    output.append("\tmeasured: ");
    output.append(measured);
    output.append("\n");
    output.append("\tminMeasuredValue: ");
    output.append(minMeasuredValue);
    output.append("\n");
    output.append("\tmaxMeasuredValue: ");
    output.append(maxMeasuredValue);
    output.append("\n");
    output.append("\taccuracyRanges: ");
    output.append(accuracyRanges);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ElectricalPowerMeasurementClusterHarmonicMeasurementStruct {
  public Integer order;
  public @Nullable Long measurement;
  private static final long ORDER_ID = 0L;
  private static final long MEASUREMENT_ID = 1L;

  public ElectricalPowerMeasurementClusterHarmonicMeasurementStruct(
    Integer order,
    @Nullable Long measurement
  ) {
    this.order = order;
    this.measurement = measurement;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ORDER_ID, new UIntType(order)));
    values.add(new StructElement(MEASUREMENT_ID, measurement != null ? new IntType(measurement) : new NullType()));

    return new StructType(values);
  }

  public static ElectricalPowerMeasurementClusterHarmonicMeasurementStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer order = null;
    @Nullable Long measurement = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ORDER_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          order = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == MEASUREMENT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          measurement = castingValue.value(Long.class);
        }
      }
    }
    return new ElectricalPowerMeasurementClusterHarmonicMeasurementStruct(
      order,
      measurement
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ElectricalPowerMeasurementClusterHarmonicMeasurementStruct {\n");
    output.append("\torder: ");
    output.append(order);
    output.append("\n");
    output.append("\tmeasurement: ");
    output.append(measurement);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ElectricalPowerMeasurementClusterMeasurementRangeStruct {
  public Integer measurementType;
  public Long min;
  public Long max;
  public Optional<Long> startTimestamp;
  public Optional<Long> endTimestamp;
  public Optional<Long> minTimestamp;
  public Optional<Long> maxTimestamp;
  public Optional<Long> startSystime;
  public Optional<Long> endSystime;
  public Optional<Long> minSystime;
  public Optional<Long> maxSystime;
  private static final long MEASUREMENT_TYPE_ID = 0L;
  private static final long MIN_ID = 1L;
  private static final long MAX_ID = 2L;
  private static final long START_TIMESTAMP_ID = 3L;
  private static final long END_TIMESTAMP_ID = 4L;
  private static final long MIN_TIMESTAMP_ID = 5L;
  private static final long MAX_TIMESTAMP_ID = 6L;
  private static final long START_SYSTIME_ID = 7L;
  private static final long END_SYSTIME_ID = 8L;
  private static final long MIN_SYSTIME_ID = 9L;
  private static final long MAX_SYSTIME_ID = 10L;

  public ElectricalPowerMeasurementClusterMeasurementRangeStruct(
    Integer measurementType,
    Long min,
    Long max,
    Optional<Long> startTimestamp,
    Optional<Long> endTimestamp,
    Optional<Long> minTimestamp,
    Optional<Long> maxTimestamp,
    Optional<Long> startSystime,
    Optional<Long> endSystime,
    Optional<Long> minSystime,
    Optional<Long> maxSystime
  ) {
    this.measurementType = measurementType;
    this.min = min;
    this.max = max;
    this.startTimestamp = startTimestamp;
    this.endTimestamp = endTimestamp;
    this.minTimestamp = minTimestamp;
    this.maxTimestamp = maxTimestamp;
    this.startSystime = startSystime;
    this.endSystime = endSystime;
    this.minSystime = minSystime;
    this.maxSystime = maxSystime;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(MEASUREMENT_TYPE_ID, new UIntType(measurementType)));
    values.add(new StructElement(MIN_ID, new IntType(min)));
    values.add(new StructElement(MAX_ID, new IntType(max)));
    values.add(new StructElement(START_TIMESTAMP_ID, startTimestamp.<BaseTLVType>map((nonOptionalstartTimestamp) -> new UIntType(nonOptionalstartTimestamp)).orElse(new EmptyType())));
    values.add(new StructElement(END_TIMESTAMP_ID, endTimestamp.<BaseTLVType>map((nonOptionalendTimestamp) -> new UIntType(nonOptionalendTimestamp)).orElse(new EmptyType())));
    values.add(new StructElement(MIN_TIMESTAMP_ID, minTimestamp.<BaseTLVType>map((nonOptionalminTimestamp) -> new UIntType(nonOptionalminTimestamp)).orElse(new EmptyType())));
    values.add(new StructElement(MAX_TIMESTAMP_ID, maxTimestamp.<BaseTLVType>map((nonOptionalmaxTimestamp) -> new UIntType(nonOptionalmaxTimestamp)).orElse(new EmptyType())));
    values.add(new StructElement(START_SYSTIME_ID, startSystime.<BaseTLVType>map((nonOptionalstartSystime) -> new UIntType(nonOptionalstartSystime)).orElse(new EmptyType())));
    values.add(new StructElement(END_SYSTIME_ID, endSystime.<BaseTLVType>map((nonOptionalendSystime) -> new UIntType(nonOptionalendSystime)).orElse(new EmptyType())));
    values.add(new StructElement(MIN_SYSTIME_ID, minSystime.<BaseTLVType>map((nonOptionalminSystime) -> new UIntType(nonOptionalminSystime)).orElse(new EmptyType())));
    values.add(new StructElement(MAX_SYSTIME_ID, maxSystime.<BaseTLVType>map((nonOptionalmaxSystime) -> new UIntType(nonOptionalmaxSystime)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static ElectricalPowerMeasurementClusterMeasurementRangeStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer measurementType = null;
    Long min = null;
    Long max = null;
    Optional<Long> startTimestamp = Optional.empty();
    Optional<Long> endTimestamp = Optional.empty();
    Optional<Long> minTimestamp = Optional.empty();
    Optional<Long> maxTimestamp = Optional.empty();
    Optional<Long> startSystime = Optional.empty();
    Optional<Long> endSystime = Optional.empty();
    Optional<Long> minSystime = Optional.empty();
    Optional<Long> maxSystime = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == MEASUREMENT_TYPE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          measurementType = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == MIN_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          min = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == MAX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          max = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == START_TIMESTAMP_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          startTimestamp = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == END_TIMESTAMP_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          endTimestamp = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == MIN_TIMESTAMP_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          minTimestamp = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == MAX_TIMESTAMP_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          maxTimestamp = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == START_SYSTIME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          startSystime = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == END_SYSTIME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          endSystime = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == MIN_SYSTIME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          minSystime = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == MAX_SYSTIME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          maxSystime = Optional.of(castingValue.value(Long.class));
        }
      }
    }
    return new ElectricalPowerMeasurementClusterMeasurementRangeStruct(
      measurementType,
      min,
      max,
      startTimestamp,
      endTimestamp,
      minTimestamp,
      maxTimestamp,
      startSystime,
      endSystime,
      minSystime,
      maxSystime
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ElectricalPowerMeasurementClusterMeasurementRangeStruct {\n");
    output.append("\tmeasurementType: ");
    output.append(measurementType);
    output.append("\n");
    output.append("\tmin: ");
    output.append(min);
    output.append("\n");
    output.append("\tmax: ");
    output.append(max);
    output.append("\n");
    output.append("\tstartTimestamp: ");
    output.append(startTimestamp);
    output.append("\n");
    output.append("\tendTimestamp: ");
    output.append(endTimestamp);
    output.append("\n");
    output.append("\tminTimestamp: ");
    output.append(minTimestamp);
    output.append("\n");
    output.append("\tmaxTimestamp: ");
    output.append(maxTimestamp);
    output.append("\n");
    output.append("\tstartSystime: ");
    output.append(startSystime);
    output.append("\n");
    output.append("\tendSystime: ");
    output.append(endSystime);
    output.append("\n");
    output.append("\tminSystime: ");
    output.append(minSystime);
    output.append("\n");
    output.append("\tmaxSystime: ");
    output.append(maxSystime);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ElectricalEnergyMeasurementClusterMeasurementAccuracyRangeStruct {
  public Long rangeMin;
  public Long rangeMax;
  public Optional<Integer> percentMax;
  public Optional<Integer> percentMin;
  public Optional<Integer> percentTypical;
  public Optional<Long> fixedMax;
  public Optional<Long> fixedMin;
  public Optional<Long> fixedTypical;
  private static final long RANGE_MIN_ID = 0L;
  private static final long RANGE_MAX_ID = 1L;
  private static final long PERCENT_MAX_ID = 2L;
  private static final long PERCENT_MIN_ID = 3L;
  private static final long PERCENT_TYPICAL_ID = 4L;
  private static final long FIXED_MAX_ID = 5L;
  private static final long FIXED_MIN_ID = 6L;
  private static final long FIXED_TYPICAL_ID = 7L;

  public ElectricalEnergyMeasurementClusterMeasurementAccuracyRangeStruct(
    Long rangeMin,
    Long rangeMax,
    Optional<Integer> percentMax,
    Optional<Integer> percentMin,
    Optional<Integer> percentTypical,
    Optional<Long> fixedMax,
    Optional<Long> fixedMin,
    Optional<Long> fixedTypical
  ) {
    this.rangeMin = rangeMin;
    this.rangeMax = rangeMax;
    this.percentMax = percentMax;
    this.percentMin = percentMin;
    this.percentTypical = percentTypical;
    this.fixedMax = fixedMax;
    this.fixedMin = fixedMin;
    this.fixedTypical = fixedTypical;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(RANGE_MIN_ID, new IntType(rangeMin)));
    values.add(new StructElement(RANGE_MAX_ID, new IntType(rangeMax)));
    values.add(new StructElement(PERCENT_MAX_ID, percentMax.<BaseTLVType>map((nonOptionalpercentMax) -> new UIntType(nonOptionalpercentMax)).orElse(new EmptyType())));
    values.add(new StructElement(PERCENT_MIN_ID, percentMin.<BaseTLVType>map((nonOptionalpercentMin) -> new UIntType(nonOptionalpercentMin)).orElse(new EmptyType())));
    values.add(new StructElement(PERCENT_TYPICAL_ID, percentTypical.<BaseTLVType>map((nonOptionalpercentTypical) -> new UIntType(nonOptionalpercentTypical)).orElse(new EmptyType())));
    values.add(new StructElement(FIXED_MAX_ID, fixedMax.<BaseTLVType>map((nonOptionalfixedMax) -> new UIntType(nonOptionalfixedMax)).orElse(new EmptyType())));
    values.add(new StructElement(FIXED_MIN_ID, fixedMin.<BaseTLVType>map((nonOptionalfixedMin) -> new UIntType(nonOptionalfixedMin)).orElse(new EmptyType())));
    values.add(new StructElement(FIXED_TYPICAL_ID, fixedTypical.<BaseTLVType>map((nonOptionalfixedTypical) -> new UIntType(nonOptionalfixedTypical)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static ElectricalEnergyMeasurementClusterMeasurementAccuracyRangeStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long rangeMin = null;
    Long rangeMax = null;
    Optional<Integer> percentMax = Optional.empty();
    Optional<Integer> percentMin = Optional.empty();
    Optional<Integer> percentTypical = Optional.empty();
    Optional<Long> fixedMax = Optional.empty();
    Optional<Long> fixedMin = Optional.empty();
    Optional<Long> fixedTypical = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == RANGE_MIN_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          rangeMin = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == RANGE_MAX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          rangeMax = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == PERCENT_MAX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          percentMax = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == PERCENT_MIN_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          percentMin = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == PERCENT_TYPICAL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          percentTypical = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == FIXED_MAX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          fixedMax = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == FIXED_MIN_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          fixedMin = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == FIXED_TYPICAL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          fixedTypical = Optional.of(castingValue.value(Long.class));
        }
      }
    }
    return new ElectricalEnergyMeasurementClusterMeasurementAccuracyRangeStruct(
      rangeMin,
      rangeMax,
      percentMax,
      percentMin,
      percentTypical,
      fixedMax,
      fixedMin,
      fixedTypical
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ElectricalEnergyMeasurementClusterMeasurementAccuracyRangeStruct {\n");
    output.append("\trangeMin: ");
    output.append(rangeMin);
    output.append("\n");
    output.append("\trangeMax: ");
    output.append(rangeMax);
    output.append("\n");
    output.append("\tpercentMax: ");
    output.append(percentMax);
    output.append("\n");
    output.append("\tpercentMin: ");
    output.append(percentMin);
    output.append("\n");
    output.append("\tpercentTypical: ");
    output.append(percentTypical);
    output.append("\n");
    output.append("\tfixedMax: ");
    output.append(fixedMax);
    output.append("\n");
    output.append("\tfixedMin: ");
    output.append(fixedMin);
    output.append("\n");
    output.append("\tfixedTypical: ");
    output.append(fixedTypical);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ElectricalEnergyMeasurementClusterMeasurementAccuracyStruct {
  public Integer measurementType;
  public Boolean measured;
  public Long minMeasuredValue;
  public Long maxMeasuredValue;
  public ArrayList<ChipStructs.ElectricalEnergyMeasurementClusterMeasurementAccuracyRangeStruct> accuracyRanges;
  private static final long MEASUREMENT_TYPE_ID = 0L;
  private static final long MEASURED_ID = 1L;
  private static final long MIN_MEASURED_VALUE_ID = 2L;
  private static final long MAX_MEASURED_VALUE_ID = 3L;
  private static final long ACCURACY_RANGES_ID = 4L;

  public ElectricalEnergyMeasurementClusterMeasurementAccuracyStruct(
    Integer measurementType,
    Boolean measured,
    Long minMeasuredValue,
    Long maxMeasuredValue,
    ArrayList<ChipStructs.ElectricalEnergyMeasurementClusterMeasurementAccuracyRangeStruct> accuracyRanges
  ) {
    this.measurementType = measurementType;
    this.measured = measured;
    this.minMeasuredValue = minMeasuredValue;
    this.maxMeasuredValue = maxMeasuredValue;
    this.accuracyRanges = accuracyRanges;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(MEASUREMENT_TYPE_ID, new UIntType(measurementType)));
    values.add(new StructElement(MEASURED_ID, new BooleanType(measured)));
    values.add(new StructElement(MIN_MEASURED_VALUE_ID, new IntType(minMeasuredValue)));
    values.add(new StructElement(MAX_MEASURED_VALUE_ID, new IntType(maxMeasuredValue)));
    values.add(new StructElement(ACCURACY_RANGES_ID, ArrayType.generateArrayType(accuracyRanges, (elementaccuracyRanges) -> elementaccuracyRanges.encodeTlv())));

    return new StructType(values);
  }

  public static ElectricalEnergyMeasurementClusterMeasurementAccuracyStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer measurementType = null;
    Boolean measured = null;
    Long minMeasuredValue = null;
    Long maxMeasuredValue = null;
    ArrayList<ChipStructs.ElectricalEnergyMeasurementClusterMeasurementAccuracyRangeStruct> accuracyRanges = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == MEASUREMENT_TYPE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          measurementType = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == MEASURED_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          measured = castingValue.value(Boolean.class);
        }
      } else if (element.contextTagNum() == MIN_MEASURED_VALUE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          minMeasuredValue = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == MAX_MEASURED_VALUE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          maxMeasuredValue = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == ACCURACY_RANGES_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          accuracyRanges = castingValue.map((elementcastingValue) -> ChipStructs.ElectricalEnergyMeasurementClusterMeasurementAccuracyRangeStruct.decodeTlv(elementcastingValue));
        }
      }
    }
    return new ElectricalEnergyMeasurementClusterMeasurementAccuracyStruct(
      measurementType,
      measured,
      minMeasuredValue,
      maxMeasuredValue,
      accuracyRanges
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ElectricalEnergyMeasurementClusterMeasurementAccuracyStruct {\n");
    output.append("\tmeasurementType: ");
    output.append(measurementType);
    output.append("\n");
    output.append("\tmeasured: ");
    output.append(measured);
    output.append("\n");
    output.append("\tminMeasuredValue: ");
    output.append(minMeasuredValue);
    output.append("\n");
    output.append("\tmaxMeasuredValue: ");
    output.append(maxMeasuredValue);
    output.append("\n");
    output.append("\taccuracyRanges: ");
    output.append(accuracyRanges);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ElectricalEnergyMeasurementClusterCumulativeEnergyResetStruct {
  public @Nullable Optional<Long> importedResetTimestamp;
  public @Nullable Optional<Long> exportedResetTimestamp;
  public @Nullable Optional<Long> importedResetSystime;
  public @Nullable Optional<Long> exportedResetSystime;
  private static final long IMPORTED_RESET_TIMESTAMP_ID = 0L;
  private static final long EXPORTED_RESET_TIMESTAMP_ID = 1L;
  private static final long IMPORTED_RESET_SYSTIME_ID = 2L;
  private static final long EXPORTED_RESET_SYSTIME_ID = 3L;

  public ElectricalEnergyMeasurementClusterCumulativeEnergyResetStruct(
    @Nullable Optional<Long> importedResetTimestamp,
    @Nullable Optional<Long> exportedResetTimestamp,
    @Nullable Optional<Long> importedResetSystime,
    @Nullable Optional<Long> exportedResetSystime
  ) {
    this.importedResetTimestamp = importedResetTimestamp;
    this.exportedResetTimestamp = exportedResetTimestamp;
    this.importedResetSystime = importedResetSystime;
    this.exportedResetSystime = exportedResetSystime;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(IMPORTED_RESET_TIMESTAMP_ID, importedResetTimestamp != null ? importedResetTimestamp.<BaseTLVType>map((nonOptionalimportedResetTimestamp) -> new UIntType(nonOptionalimportedResetTimestamp)).orElse(new EmptyType()) : new NullType()));
    values.add(new StructElement(EXPORTED_RESET_TIMESTAMP_ID, exportedResetTimestamp != null ? exportedResetTimestamp.<BaseTLVType>map((nonOptionalexportedResetTimestamp) -> new UIntType(nonOptionalexportedResetTimestamp)).orElse(new EmptyType()) : new NullType()));
    values.add(new StructElement(IMPORTED_RESET_SYSTIME_ID, importedResetSystime != null ? importedResetSystime.<BaseTLVType>map((nonOptionalimportedResetSystime) -> new UIntType(nonOptionalimportedResetSystime)).orElse(new EmptyType()) : new NullType()));
    values.add(new StructElement(EXPORTED_RESET_SYSTIME_ID, exportedResetSystime != null ? exportedResetSystime.<BaseTLVType>map((nonOptionalexportedResetSystime) -> new UIntType(nonOptionalexportedResetSystime)).orElse(new EmptyType()) : new NullType()));

    return new StructType(values);
  }

  public static ElectricalEnergyMeasurementClusterCumulativeEnergyResetStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    @Nullable Optional<Long> importedResetTimestamp = null;
    @Nullable Optional<Long> exportedResetTimestamp = null;
    @Nullable Optional<Long> importedResetSystime = null;
    @Nullable Optional<Long> exportedResetSystime = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == IMPORTED_RESET_TIMESTAMP_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          importedResetTimestamp = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == EXPORTED_RESET_TIMESTAMP_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          exportedResetTimestamp = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == IMPORTED_RESET_SYSTIME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          importedResetSystime = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == EXPORTED_RESET_SYSTIME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          exportedResetSystime = Optional.of(castingValue.value(Long.class));
        }
      }
    }
    return new ElectricalEnergyMeasurementClusterCumulativeEnergyResetStruct(
      importedResetTimestamp,
      exportedResetTimestamp,
      importedResetSystime,
      exportedResetSystime
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ElectricalEnergyMeasurementClusterCumulativeEnergyResetStruct {\n");
    output.append("\timportedResetTimestamp: ");
    output.append(importedResetTimestamp);
    output.append("\n");
    output.append("\texportedResetTimestamp: ");
    output.append(exportedResetTimestamp);
    output.append("\n");
    output.append("\timportedResetSystime: ");
    output.append(importedResetSystime);
    output.append("\n");
    output.append("\texportedResetSystime: ");
    output.append(exportedResetSystime);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ElectricalEnergyMeasurementClusterEnergyMeasurementStruct {
  public Long energy;
  public Optional<Long> startTimestamp;
  public Optional<Long> endTimestamp;
  public Optional<Long> startSystime;
  public Optional<Long> endSystime;
  private static final long ENERGY_ID = 0L;
  private static final long START_TIMESTAMP_ID = 1L;
  private static final long END_TIMESTAMP_ID = 2L;
  private static final long START_SYSTIME_ID = 3L;
  private static final long END_SYSTIME_ID = 4L;

  public ElectricalEnergyMeasurementClusterEnergyMeasurementStruct(
    Long energy,
    Optional<Long> startTimestamp,
    Optional<Long> endTimestamp,
    Optional<Long> startSystime,
    Optional<Long> endSystime
  ) {
    this.energy = energy;
    this.startTimestamp = startTimestamp;
    this.endTimestamp = endTimestamp;
    this.startSystime = startSystime;
    this.endSystime = endSystime;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ENERGY_ID, new IntType(energy)));
    values.add(new StructElement(START_TIMESTAMP_ID, startTimestamp.<BaseTLVType>map((nonOptionalstartTimestamp) -> new UIntType(nonOptionalstartTimestamp)).orElse(new EmptyType())));
    values.add(new StructElement(END_TIMESTAMP_ID, endTimestamp.<BaseTLVType>map((nonOptionalendTimestamp) -> new UIntType(nonOptionalendTimestamp)).orElse(new EmptyType())));
    values.add(new StructElement(START_SYSTIME_ID, startSystime.<BaseTLVType>map((nonOptionalstartSystime) -> new UIntType(nonOptionalstartSystime)).orElse(new EmptyType())));
    values.add(new StructElement(END_SYSTIME_ID, endSystime.<BaseTLVType>map((nonOptionalendSystime) -> new UIntType(nonOptionalendSystime)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static ElectricalEnergyMeasurementClusterEnergyMeasurementStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long energy = null;
    Optional<Long> startTimestamp = Optional.empty();
    Optional<Long> endTimestamp = Optional.empty();
    Optional<Long> startSystime = Optional.empty();
    Optional<Long> endSystime = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ENERGY_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          energy = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == START_TIMESTAMP_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          startTimestamp = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == END_TIMESTAMP_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          endTimestamp = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == START_SYSTIME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          startSystime = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == END_SYSTIME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          endSystime = Optional.of(castingValue.value(Long.class));
        }
      }
    }
    return new ElectricalEnergyMeasurementClusterEnergyMeasurementStruct(
      energy,
      startTimestamp,
      endTimestamp,
      startSystime,
      endSystime
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ElectricalEnergyMeasurementClusterEnergyMeasurementStruct {\n");
    output.append("\tenergy: ");
    output.append(energy);
    output.append("\n");
    output.append("\tstartTimestamp: ");
    output.append(startTimestamp);
    output.append("\n");
    output.append("\tendTimestamp: ");
    output.append(endTimestamp);
    output.append("\n");
    output.append("\tstartSystime: ");
    output.append(startSystime);
    output.append("\n");
    output.append("\tendSystime: ");
    output.append(endSystime);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class WaterHeaterManagementClusterWaterHeaterBoostInfoStruct {
  public Long duration;
  public Optional<Boolean> oneShot;
  public Optional<Boolean> emergencyBoost;
  public Optional<Integer> temporarySetpoint;
  public Optional<Integer> targetPercentage;
  public Optional<Integer> targetReheat;
  private static final long DURATION_ID = 0L;
  private static final long ONE_SHOT_ID = 1L;
  private static final long EMERGENCY_BOOST_ID = 2L;
  private static final long TEMPORARY_SETPOINT_ID = 3L;
  private static final long TARGET_PERCENTAGE_ID = 4L;
  private static final long TARGET_REHEAT_ID = 5L;

  public WaterHeaterManagementClusterWaterHeaterBoostInfoStruct(
    Long duration,
    Optional<Boolean> oneShot,
    Optional<Boolean> emergencyBoost,
    Optional<Integer> temporarySetpoint,
    Optional<Integer> targetPercentage,
    Optional<Integer> targetReheat
  ) {
    this.duration = duration;
    this.oneShot = oneShot;
    this.emergencyBoost = emergencyBoost;
    this.temporarySetpoint = temporarySetpoint;
    this.targetPercentage = targetPercentage;
    this.targetReheat = targetReheat;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(DURATION_ID, new UIntType(duration)));
    values.add(new StructElement(ONE_SHOT_ID, oneShot.<BaseTLVType>map((nonOptionaloneShot) -> new BooleanType(nonOptionaloneShot)).orElse(new EmptyType())));
    values.add(new StructElement(EMERGENCY_BOOST_ID, emergencyBoost.<BaseTLVType>map((nonOptionalemergencyBoost) -> new BooleanType(nonOptionalemergencyBoost)).orElse(new EmptyType())));
    values.add(new StructElement(TEMPORARY_SETPOINT_ID, temporarySetpoint.<BaseTLVType>map((nonOptionaltemporarySetpoint) -> new IntType(nonOptionaltemporarySetpoint)).orElse(new EmptyType())));
    values.add(new StructElement(TARGET_PERCENTAGE_ID, targetPercentage.<BaseTLVType>map((nonOptionaltargetPercentage) -> new UIntType(nonOptionaltargetPercentage)).orElse(new EmptyType())));
    values.add(new StructElement(TARGET_REHEAT_ID, targetReheat.<BaseTLVType>map((nonOptionaltargetReheat) -> new UIntType(nonOptionaltargetReheat)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static WaterHeaterManagementClusterWaterHeaterBoostInfoStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long duration = null;
    Optional<Boolean> oneShot = Optional.empty();
    Optional<Boolean> emergencyBoost = Optional.empty();
    Optional<Integer> temporarySetpoint = Optional.empty();
    Optional<Integer> targetPercentage = Optional.empty();
    Optional<Integer> targetReheat = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == DURATION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          duration = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == ONE_SHOT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          oneShot = Optional.of(castingValue.value(Boolean.class));
        }
      } else if (element.contextTagNum() == EMERGENCY_BOOST_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          emergencyBoost = Optional.of(castingValue.value(Boolean.class));
        }
      } else if (element.contextTagNum() == TEMPORARY_SETPOINT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          temporarySetpoint = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == TARGET_PERCENTAGE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          targetPercentage = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == TARGET_REHEAT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          targetReheat = Optional.of(castingValue.value(Integer.class));
        }
      }
    }
    return new WaterHeaterManagementClusterWaterHeaterBoostInfoStruct(
      duration,
      oneShot,
      emergencyBoost,
      temporarySetpoint,
      targetPercentage,
      targetReheat
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("WaterHeaterManagementClusterWaterHeaterBoostInfoStruct {\n");
    output.append("\tduration: ");
    output.append(duration);
    output.append("\n");
    output.append("\toneShot: ");
    output.append(oneShot);
    output.append("\n");
    output.append("\temergencyBoost: ");
    output.append(emergencyBoost);
    output.append("\n");
    output.append("\ttemporarySetpoint: ");
    output.append(temporarySetpoint);
    output.append("\n");
    output.append("\ttargetPercentage: ");
    output.append(targetPercentage);
    output.append("\n");
    output.append("\ttargetReheat: ");
    output.append(targetReheat);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class DemandResponseLoadControlClusterHeatingSourceControlStruct {
  public Integer heatingSource;
  private static final long HEATING_SOURCE_ID = 0L;

  public DemandResponseLoadControlClusterHeatingSourceControlStruct(
    Integer heatingSource
  ) {
    this.heatingSource = heatingSource;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(HEATING_SOURCE_ID, new UIntType(heatingSource)));

    return new StructType(values);
  }

  public static DemandResponseLoadControlClusterHeatingSourceControlStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer heatingSource = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == HEATING_SOURCE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          heatingSource = castingValue.value(Integer.class);
        }
      }
    }
    return new DemandResponseLoadControlClusterHeatingSourceControlStruct(
      heatingSource
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("DemandResponseLoadControlClusterHeatingSourceControlStruct {\n");
    output.append("\theatingSource: ");
    output.append(heatingSource);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class DemandResponseLoadControlClusterPowerSavingsControlStruct {
  public Integer powerSavings;
  private static final long POWER_SAVINGS_ID = 0L;

  public DemandResponseLoadControlClusterPowerSavingsControlStruct(
    Integer powerSavings
  ) {
    this.powerSavings = powerSavings;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(POWER_SAVINGS_ID, new UIntType(powerSavings)));

    return new StructType(values);
  }

  public static DemandResponseLoadControlClusterPowerSavingsControlStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer powerSavings = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == POWER_SAVINGS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          powerSavings = castingValue.value(Integer.class);
        }
      }
    }
    return new DemandResponseLoadControlClusterPowerSavingsControlStruct(
      powerSavings
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("DemandResponseLoadControlClusterPowerSavingsControlStruct {\n");
    output.append("\tpowerSavings: ");
    output.append(powerSavings);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class DemandResponseLoadControlClusterDutyCycleControlStruct {
  public Integer dutyCycle;
  private static final long DUTY_CYCLE_ID = 0L;

  public DemandResponseLoadControlClusterDutyCycleControlStruct(
    Integer dutyCycle
  ) {
    this.dutyCycle = dutyCycle;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(DUTY_CYCLE_ID, new UIntType(dutyCycle)));

    return new StructType(values);
  }

  public static DemandResponseLoadControlClusterDutyCycleControlStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer dutyCycle = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == DUTY_CYCLE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          dutyCycle = castingValue.value(Integer.class);
        }
      }
    }
    return new DemandResponseLoadControlClusterDutyCycleControlStruct(
      dutyCycle
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("DemandResponseLoadControlClusterDutyCycleControlStruct {\n");
    output.append("\tdutyCycle: ");
    output.append(dutyCycle);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class DemandResponseLoadControlClusterAverageLoadControlStruct {
  public Integer loadAdjustment;
  private static final long LOAD_ADJUSTMENT_ID = 0L;

  public DemandResponseLoadControlClusterAverageLoadControlStruct(
    Integer loadAdjustment
  ) {
    this.loadAdjustment = loadAdjustment;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(LOAD_ADJUSTMENT_ID, new IntType(loadAdjustment)));

    return new StructType(values);
  }

  public static DemandResponseLoadControlClusterAverageLoadControlStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer loadAdjustment = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == LOAD_ADJUSTMENT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          loadAdjustment = castingValue.value(Integer.class);
        }
      }
    }
    return new DemandResponseLoadControlClusterAverageLoadControlStruct(
      loadAdjustment
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("DemandResponseLoadControlClusterAverageLoadControlStruct {\n");
    output.append("\tloadAdjustment: ");
    output.append(loadAdjustment);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class DemandResponseLoadControlClusterTemperatureControlStruct {
  public @Nullable Optional<Integer> coolingTempOffset;
  public @Nullable Optional<Integer> heatingtTempOffset;
  public @Nullable Optional<Integer> coolingTempSetpoint;
  public @Nullable Optional<Integer> heatingTempSetpoint;
  private static final long COOLING_TEMP_OFFSET_ID = 0L;
  private static final long HEATINGT_TEMP_OFFSET_ID = 1L;
  private static final long COOLING_TEMP_SETPOINT_ID = 2L;
  private static final long HEATING_TEMP_SETPOINT_ID = 3L;

  public DemandResponseLoadControlClusterTemperatureControlStruct(
    @Nullable Optional<Integer> coolingTempOffset,
    @Nullable Optional<Integer> heatingtTempOffset,
    @Nullable Optional<Integer> coolingTempSetpoint,
    @Nullable Optional<Integer> heatingTempSetpoint
  ) {
    this.coolingTempOffset = coolingTempOffset;
    this.heatingtTempOffset = heatingtTempOffset;
    this.coolingTempSetpoint = coolingTempSetpoint;
    this.heatingTempSetpoint = heatingTempSetpoint;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(COOLING_TEMP_OFFSET_ID, coolingTempOffset != null ? coolingTempOffset.<BaseTLVType>map((nonOptionalcoolingTempOffset) -> new UIntType(nonOptionalcoolingTempOffset)).orElse(new EmptyType()) : new NullType()));
    values.add(new StructElement(HEATINGT_TEMP_OFFSET_ID, heatingtTempOffset != null ? heatingtTempOffset.<BaseTLVType>map((nonOptionalheatingtTempOffset) -> new UIntType(nonOptionalheatingtTempOffset)).orElse(new EmptyType()) : new NullType()));
    values.add(new StructElement(COOLING_TEMP_SETPOINT_ID, coolingTempSetpoint != null ? coolingTempSetpoint.<BaseTLVType>map((nonOptionalcoolingTempSetpoint) -> new IntType(nonOptionalcoolingTempSetpoint)).orElse(new EmptyType()) : new NullType()));
    values.add(new StructElement(HEATING_TEMP_SETPOINT_ID, heatingTempSetpoint != null ? heatingTempSetpoint.<BaseTLVType>map((nonOptionalheatingTempSetpoint) -> new IntType(nonOptionalheatingTempSetpoint)).orElse(new EmptyType()) : new NullType()));

    return new StructType(values);
  }

  public static DemandResponseLoadControlClusterTemperatureControlStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    @Nullable Optional<Integer> coolingTempOffset = null;
    @Nullable Optional<Integer> heatingtTempOffset = null;
    @Nullable Optional<Integer> coolingTempSetpoint = null;
    @Nullable Optional<Integer> heatingTempSetpoint = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == COOLING_TEMP_OFFSET_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          coolingTempOffset = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == HEATINGT_TEMP_OFFSET_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          heatingtTempOffset = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == COOLING_TEMP_SETPOINT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          coolingTempSetpoint = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == HEATING_TEMP_SETPOINT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          heatingTempSetpoint = Optional.of(castingValue.value(Integer.class));
        }
      }
    }
    return new DemandResponseLoadControlClusterTemperatureControlStruct(
      coolingTempOffset,
      heatingtTempOffset,
      coolingTempSetpoint,
      heatingTempSetpoint
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("DemandResponseLoadControlClusterTemperatureControlStruct {\n");
    output.append("\tcoolingTempOffset: ");
    output.append(coolingTempOffset);
    output.append("\n");
    output.append("\theatingtTempOffset: ");
    output.append(heatingtTempOffset);
    output.append("\n");
    output.append("\tcoolingTempSetpoint: ");
    output.append(coolingTempSetpoint);
    output.append("\n");
    output.append("\theatingTempSetpoint: ");
    output.append(heatingTempSetpoint);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class DemandResponseLoadControlClusterLoadControlEventTransitionStruct {
  public Integer duration;
  public Integer control;
  public Optional<ChipStructs.DemandResponseLoadControlClusterTemperatureControlStruct> temperatureControl;
  public Optional<ChipStructs.DemandResponseLoadControlClusterAverageLoadControlStruct> averageLoadControl;
  public Optional<ChipStructs.DemandResponseLoadControlClusterDutyCycleControlStruct> dutyCycleControl;
  public Optional<ChipStructs.DemandResponseLoadControlClusterPowerSavingsControlStruct> powerSavingsControl;
  public Optional<ChipStructs.DemandResponseLoadControlClusterHeatingSourceControlStruct> heatingSourceControl;
  private static final long DURATION_ID = 0L;
  private static final long CONTROL_ID = 1L;
  private static final long TEMPERATURE_CONTROL_ID = 2L;
  private static final long AVERAGE_LOAD_CONTROL_ID = 3L;
  private static final long DUTY_CYCLE_CONTROL_ID = 4L;
  private static final long POWER_SAVINGS_CONTROL_ID = 5L;
  private static final long HEATING_SOURCE_CONTROL_ID = 6L;

  public DemandResponseLoadControlClusterLoadControlEventTransitionStruct(
    Integer duration,
    Integer control,
    Optional<ChipStructs.DemandResponseLoadControlClusterTemperatureControlStruct> temperatureControl,
    Optional<ChipStructs.DemandResponseLoadControlClusterAverageLoadControlStruct> averageLoadControl,
    Optional<ChipStructs.DemandResponseLoadControlClusterDutyCycleControlStruct> dutyCycleControl,
    Optional<ChipStructs.DemandResponseLoadControlClusterPowerSavingsControlStruct> powerSavingsControl,
    Optional<ChipStructs.DemandResponseLoadControlClusterHeatingSourceControlStruct> heatingSourceControl
  ) {
    this.duration = duration;
    this.control = control;
    this.temperatureControl = temperatureControl;
    this.averageLoadControl = averageLoadControl;
    this.dutyCycleControl = dutyCycleControl;
    this.powerSavingsControl = powerSavingsControl;
    this.heatingSourceControl = heatingSourceControl;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(DURATION_ID, new UIntType(duration)));
    values.add(new StructElement(CONTROL_ID, new UIntType(control)));
    values.add(new StructElement(TEMPERATURE_CONTROL_ID, temperatureControl.<BaseTLVType>map((nonOptionaltemperatureControl) -> nonOptionaltemperatureControl.encodeTlv()).orElse(new EmptyType())));
    values.add(new StructElement(AVERAGE_LOAD_CONTROL_ID, averageLoadControl.<BaseTLVType>map((nonOptionalaverageLoadControl) -> nonOptionalaverageLoadControl.encodeTlv()).orElse(new EmptyType())));
    values.add(new StructElement(DUTY_CYCLE_CONTROL_ID, dutyCycleControl.<BaseTLVType>map((nonOptionaldutyCycleControl) -> nonOptionaldutyCycleControl.encodeTlv()).orElse(new EmptyType())));
    values.add(new StructElement(POWER_SAVINGS_CONTROL_ID, powerSavingsControl.<BaseTLVType>map((nonOptionalpowerSavingsControl) -> nonOptionalpowerSavingsControl.encodeTlv()).orElse(new EmptyType())));
    values.add(new StructElement(HEATING_SOURCE_CONTROL_ID, heatingSourceControl.<BaseTLVType>map((nonOptionalheatingSourceControl) -> nonOptionalheatingSourceControl.encodeTlv()).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static DemandResponseLoadControlClusterLoadControlEventTransitionStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer duration = null;
    Integer control = null;
    Optional<ChipStructs.DemandResponseLoadControlClusterTemperatureControlStruct> temperatureControl = Optional.empty();
    Optional<ChipStructs.DemandResponseLoadControlClusterAverageLoadControlStruct> averageLoadControl = Optional.empty();
    Optional<ChipStructs.DemandResponseLoadControlClusterDutyCycleControlStruct> dutyCycleControl = Optional.empty();
    Optional<ChipStructs.DemandResponseLoadControlClusterPowerSavingsControlStruct> powerSavingsControl = Optional.empty();
    Optional<ChipStructs.DemandResponseLoadControlClusterHeatingSourceControlStruct> heatingSourceControl = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == DURATION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          duration = castingValue.value(Integer.class);
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
    return new DemandResponseLoadControlClusterLoadControlEventTransitionStruct(
      duration,
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
    output.append("DemandResponseLoadControlClusterLoadControlEventTransitionStruct {\n");
    output.append("\tduration: ");
    output.append(duration);
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
public static class DemandResponseLoadControlClusterLoadControlEventStruct {
  public byte[] eventID;
  public @Nullable byte[] programID;
  public Integer control;
  public Long deviceClass;
  public Optional<Integer> enrollmentGroup;
  public Integer criticality;
  public @Nullable Long startTime;
  public ArrayList<ChipStructs.DemandResponseLoadControlClusterLoadControlEventTransitionStruct> transitions;
  private static final long EVENT_ID_ID = 0L;
  private static final long PROGRAM_ID_ID = 1L;
  private static final long CONTROL_ID = 2L;
  private static final long DEVICE_CLASS_ID = 3L;
  private static final long ENROLLMENT_GROUP_ID = 4L;
  private static final long CRITICALITY_ID = 5L;
  private static final long START_TIME_ID = 6L;
  private static final long TRANSITIONS_ID = 7L;

  public DemandResponseLoadControlClusterLoadControlEventStruct(
    byte[] eventID,
    @Nullable byte[] programID,
    Integer control,
    Long deviceClass,
    Optional<Integer> enrollmentGroup,
    Integer criticality,
    @Nullable Long startTime,
    ArrayList<ChipStructs.DemandResponseLoadControlClusterLoadControlEventTransitionStruct> transitions
  ) {
    this.eventID = eventID;
    this.programID = programID;
    this.control = control;
    this.deviceClass = deviceClass;
    this.enrollmentGroup = enrollmentGroup;
    this.criticality = criticality;
    this.startTime = startTime;
    this.transitions = transitions;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(EVENT_ID_ID, new ByteArrayType(eventID)));
    values.add(new StructElement(PROGRAM_ID_ID, programID != null ? new ByteArrayType(programID) : new NullType()));
    values.add(new StructElement(CONTROL_ID, new UIntType(control)));
    values.add(new StructElement(DEVICE_CLASS_ID, new UIntType(deviceClass)));
    values.add(new StructElement(ENROLLMENT_GROUP_ID, enrollmentGroup.<BaseTLVType>map((nonOptionalenrollmentGroup) -> new UIntType(nonOptionalenrollmentGroup)).orElse(new EmptyType())));
    values.add(new StructElement(CRITICALITY_ID, new UIntType(criticality)));
    values.add(new StructElement(START_TIME_ID, startTime != null ? new UIntType(startTime) : new NullType()));
    values.add(new StructElement(TRANSITIONS_ID, ArrayType.generateArrayType(transitions, (elementtransitions) -> elementtransitions.encodeTlv())));

    return new StructType(values);
  }

  public static DemandResponseLoadControlClusterLoadControlEventStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    byte[] eventID = null;
    @Nullable byte[] programID = null;
    Integer control = null;
    Long deviceClass = null;
    Optional<Integer> enrollmentGroup = Optional.empty();
    Integer criticality = null;
    @Nullable Long startTime = null;
    ArrayList<ChipStructs.DemandResponseLoadControlClusterLoadControlEventTransitionStruct> transitions = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == EVENT_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.ByteArray) {
          ByteArrayType castingValue = element.value(ByteArrayType.class);
          eventID = castingValue.value(byte[].class);
        }
      } else if (element.contextTagNum() == PROGRAM_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.ByteArray) {
          ByteArrayType castingValue = element.value(ByteArrayType.class);
          programID = castingValue.value(byte[].class);
        }
      } else if (element.contextTagNum() == CONTROL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          control = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == DEVICE_CLASS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          deviceClass = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == ENROLLMENT_GROUP_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          enrollmentGroup = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == CRITICALITY_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          criticality = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == START_TIME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          startTime = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == TRANSITIONS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          transitions = castingValue.map((elementcastingValue) -> ChipStructs.DemandResponseLoadControlClusterLoadControlEventTransitionStruct.decodeTlv(elementcastingValue));
        }
      }
    }
    return new DemandResponseLoadControlClusterLoadControlEventStruct(
      eventID,
      programID,
      control,
      deviceClass,
      enrollmentGroup,
      criticality,
      startTime,
      transitions
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("DemandResponseLoadControlClusterLoadControlEventStruct {\n");
    output.append("\teventID: ");
    output.append(Arrays.toString(eventID));
    output.append("\n");
    output.append("\tprogramID: ");
    output.append(Arrays.toString(programID));
    output.append("\n");
    output.append("\tcontrol: ");
    output.append(control);
    output.append("\n");
    output.append("\tdeviceClass: ");
    output.append(deviceClass);
    output.append("\n");
    output.append("\tenrollmentGroup: ");
    output.append(enrollmentGroup);
    output.append("\n");
    output.append("\tcriticality: ");
    output.append(criticality);
    output.append("\n");
    output.append("\tstartTime: ");
    output.append(startTime);
    output.append("\n");
    output.append("\ttransitions: ");
    output.append(transitions);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class DemandResponseLoadControlClusterLoadControlProgramStruct {
  public byte[] programID;
  public String name;
  public @Nullable Integer enrollmentGroup;
  public @Nullable Integer randomStartMinutes;
  public @Nullable Integer randomDurationMinutes;
  private static final long PROGRAM_ID_ID = 0L;
  private static final long NAME_ID = 1L;
  private static final long ENROLLMENT_GROUP_ID = 2L;
  private static final long RANDOM_START_MINUTES_ID = 3L;
  private static final long RANDOM_DURATION_MINUTES_ID = 4L;

  public DemandResponseLoadControlClusterLoadControlProgramStruct(
    byte[] programID,
    String name,
    @Nullable Integer enrollmentGroup,
    @Nullable Integer randomStartMinutes,
    @Nullable Integer randomDurationMinutes
  ) {
    this.programID = programID;
    this.name = name;
    this.enrollmentGroup = enrollmentGroup;
    this.randomStartMinutes = randomStartMinutes;
    this.randomDurationMinutes = randomDurationMinutes;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(PROGRAM_ID_ID, new ByteArrayType(programID)));
    values.add(new StructElement(NAME_ID, new StringType(name)));
    values.add(new StructElement(ENROLLMENT_GROUP_ID, enrollmentGroup != null ? new UIntType(enrollmentGroup) : new NullType()));
    values.add(new StructElement(RANDOM_START_MINUTES_ID, randomStartMinutes != null ? new UIntType(randomStartMinutes) : new NullType()));
    values.add(new StructElement(RANDOM_DURATION_MINUTES_ID, randomDurationMinutes != null ? new UIntType(randomDurationMinutes) : new NullType()));

    return new StructType(values);
  }

  public static DemandResponseLoadControlClusterLoadControlProgramStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    byte[] programID = null;
    String name = null;
    @Nullable Integer enrollmentGroup = null;
    @Nullable Integer randomStartMinutes = null;
    @Nullable Integer randomDurationMinutes = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == PROGRAM_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.ByteArray) {
          ByteArrayType castingValue = element.value(ByteArrayType.class);
          programID = castingValue.value(byte[].class);
        }
      } else if (element.contextTagNum() == NAME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          name = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == ENROLLMENT_GROUP_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          enrollmentGroup = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == RANDOM_START_MINUTES_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          randomStartMinutes = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == RANDOM_DURATION_MINUTES_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          randomDurationMinutes = castingValue.value(Integer.class);
        }
      }
    }
    return new DemandResponseLoadControlClusterLoadControlProgramStruct(
      programID,
      name,
      enrollmentGroup,
      randomStartMinutes,
      randomDurationMinutes
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("DemandResponseLoadControlClusterLoadControlProgramStruct {\n");
    output.append("\tprogramID: ");
    output.append(Arrays.toString(programID));
    output.append("\n");
    output.append("\tname: ");
    output.append(name);
    output.append("\n");
    output.append("\tenrollmentGroup: ");
    output.append(enrollmentGroup);
    output.append("\n");
    output.append("\trandomStartMinutes: ");
    output.append(randomStartMinutes);
    output.append("\n");
    output.append("\trandomDurationMinutes: ");
    output.append(randomDurationMinutes);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class MessagesClusterMessageResponseOptionStruct {
  public Optional<Long> messageResponseID;
  public Optional<String> label;
  private static final long MESSAGE_RESPONSE_ID_ID = 0L;
  private static final long LABEL_ID = 1L;

  public MessagesClusterMessageResponseOptionStruct(
    Optional<Long> messageResponseID,
    Optional<String> label
  ) {
    this.messageResponseID = messageResponseID;
    this.label = label;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(MESSAGE_RESPONSE_ID_ID, messageResponseID.<BaseTLVType>map((nonOptionalmessageResponseID) -> new UIntType(nonOptionalmessageResponseID)).orElse(new EmptyType())));
    values.add(new StructElement(LABEL_ID, label.<BaseTLVType>map((nonOptionallabel) -> new StringType(nonOptionallabel)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static MessagesClusterMessageResponseOptionStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Optional<Long> messageResponseID = Optional.empty();
    Optional<String> label = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == MESSAGE_RESPONSE_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          messageResponseID = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == LABEL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          label = Optional.of(castingValue.value(String.class));
        }
      }
    }
    return new MessagesClusterMessageResponseOptionStruct(
      messageResponseID,
      label
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("MessagesClusterMessageResponseOptionStruct {\n");
    output.append("\tmessageResponseID: ");
    output.append(messageResponseID);
    output.append("\n");
    output.append("\tlabel: ");
    output.append(label);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class MessagesClusterMessageStruct {
  public byte[] messageID;
  public Integer priority;
  public Integer messageControl;
  public @Nullable Long startTime;
  public @Nullable Long duration;
  public String messageText;
  public Optional<ArrayList<ChipStructs.MessagesClusterMessageResponseOptionStruct>> responses;
  private static final long MESSAGE_ID_ID = 0L;
  private static final long PRIORITY_ID = 1L;
  private static final long MESSAGE_CONTROL_ID = 2L;
  private static final long START_TIME_ID = 3L;
  private static final long DURATION_ID = 4L;
  private static final long MESSAGE_TEXT_ID = 5L;
  private static final long RESPONSES_ID = 6L;

  public MessagesClusterMessageStruct(
    byte[] messageID,
    Integer priority,
    Integer messageControl,
    @Nullable Long startTime,
    @Nullable Long duration,
    String messageText,
    Optional<ArrayList<ChipStructs.MessagesClusterMessageResponseOptionStruct>> responses
  ) {
    this.messageID = messageID;
    this.priority = priority;
    this.messageControl = messageControl;
    this.startTime = startTime;
    this.duration = duration;
    this.messageText = messageText;
    this.responses = responses;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(MESSAGE_ID_ID, new ByteArrayType(messageID)));
    values.add(new StructElement(PRIORITY_ID, new UIntType(priority)));
    values.add(new StructElement(MESSAGE_CONTROL_ID, new UIntType(messageControl)));
    values.add(new StructElement(START_TIME_ID, startTime != null ? new UIntType(startTime) : new NullType()));
    values.add(new StructElement(DURATION_ID, duration != null ? new UIntType(duration) : new NullType()));
    values.add(new StructElement(MESSAGE_TEXT_ID, new StringType(messageText)));
    values.add(new StructElement(RESPONSES_ID, responses.<BaseTLVType>map((nonOptionalresponses) -> ArrayType.generateArrayType(nonOptionalresponses, (elementnonOptionalresponses) -> elementnonOptionalresponses.encodeTlv())).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static MessagesClusterMessageStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    byte[] messageID = null;
    Integer priority = null;
    Integer messageControl = null;
    @Nullable Long startTime = null;
    @Nullable Long duration = null;
    String messageText = null;
    Optional<ArrayList<ChipStructs.MessagesClusterMessageResponseOptionStruct>> responses = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == MESSAGE_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.ByteArray) {
          ByteArrayType castingValue = element.value(ByteArrayType.class);
          messageID = castingValue.value(byte[].class);
        }
      } else if (element.contextTagNum() == PRIORITY_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          priority = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == MESSAGE_CONTROL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          messageControl = castingValue.value(Integer.class);
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
      } else if (element.contextTagNum() == MESSAGE_TEXT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          messageText = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == RESPONSES_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          responses = Optional.of(castingValue.map((elementcastingValue) -> ChipStructs.MessagesClusterMessageResponseOptionStruct.decodeTlv(elementcastingValue)));
        }
      }
    }
    return new MessagesClusterMessageStruct(
      messageID,
      priority,
      messageControl,
      startTime,
      duration,
      messageText,
      responses
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("MessagesClusterMessageStruct {\n");
    output.append("\tmessageID: ");
    output.append(Arrays.toString(messageID));
    output.append("\n");
    output.append("\tpriority: ");
    output.append(priority);
    output.append("\n");
    output.append("\tmessageControl: ");
    output.append(messageControl);
    output.append("\n");
    output.append("\tstartTime: ");
    output.append(startTime);
    output.append("\n");
    output.append("\tduration: ");
    output.append(duration);
    output.append("\n");
    output.append("\tmessageText: ");
    output.append(messageText);
    output.append("\n");
    output.append("\tresponses: ");
    output.append(responses);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class DeviceEnergyManagementClusterCostStruct {
  public Integer costType;
  public Long value;
  public Integer decimalPoints;
  public Optional<Integer> currency;
  private static final long COST_TYPE_ID = 0L;
  private static final long VALUE_ID = 1L;
  private static final long DECIMAL_POINTS_ID = 2L;
  private static final long CURRENCY_ID = 3L;

  public DeviceEnergyManagementClusterCostStruct(
    Integer costType,
    Long value,
    Integer decimalPoints,
    Optional<Integer> currency
  ) {
    this.costType = costType;
    this.value = value;
    this.decimalPoints = decimalPoints;
    this.currency = currency;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(COST_TYPE_ID, new UIntType(costType)));
    values.add(new StructElement(VALUE_ID, new IntType(value)));
    values.add(new StructElement(DECIMAL_POINTS_ID, new UIntType(decimalPoints)));
    values.add(new StructElement(CURRENCY_ID, currency.<BaseTLVType>map((nonOptionalcurrency) -> new UIntType(nonOptionalcurrency)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static DeviceEnergyManagementClusterCostStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer costType = null;
    Long value = null;
    Integer decimalPoints = null;
    Optional<Integer> currency = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == COST_TYPE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          costType = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == VALUE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          value = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == DECIMAL_POINTS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          decimalPoints = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == CURRENCY_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          currency = Optional.of(castingValue.value(Integer.class));
        }
      }
    }
    return new DeviceEnergyManagementClusterCostStruct(
      costType,
      value,
      decimalPoints,
      currency
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("DeviceEnergyManagementClusterCostStruct {\n");
    output.append("\tcostType: ");
    output.append(costType);
    output.append("\n");
    output.append("\tvalue: ");
    output.append(value);
    output.append("\n");
    output.append("\tdecimalPoints: ");
    output.append(decimalPoints);
    output.append("\n");
    output.append("\tcurrency: ");
    output.append(currency);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class DeviceEnergyManagementClusterPowerAdjustStruct {
  public Long minPower;
  public Long maxPower;
  public Long minDuration;
  public Long maxDuration;
  private static final long MIN_POWER_ID = 0L;
  private static final long MAX_POWER_ID = 1L;
  private static final long MIN_DURATION_ID = 2L;
  private static final long MAX_DURATION_ID = 3L;

  public DeviceEnergyManagementClusterPowerAdjustStruct(
    Long minPower,
    Long maxPower,
    Long minDuration,
    Long maxDuration
  ) {
    this.minPower = minPower;
    this.maxPower = maxPower;
    this.minDuration = minDuration;
    this.maxDuration = maxDuration;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(MIN_POWER_ID, new IntType(minPower)));
    values.add(new StructElement(MAX_POWER_ID, new IntType(maxPower)));
    values.add(new StructElement(MIN_DURATION_ID, new UIntType(minDuration)));
    values.add(new StructElement(MAX_DURATION_ID, new UIntType(maxDuration)));

    return new StructType(values);
  }

  public static DeviceEnergyManagementClusterPowerAdjustStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long minPower = null;
    Long maxPower = null;
    Long minDuration = null;
    Long maxDuration = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == MIN_POWER_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          minPower = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == MAX_POWER_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          maxPower = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == MIN_DURATION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          minDuration = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == MAX_DURATION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          maxDuration = castingValue.value(Long.class);
        }
      }
    }
    return new DeviceEnergyManagementClusterPowerAdjustStruct(
      minPower,
      maxPower,
      minDuration,
      maxDuration
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("DeviceEnergyManagementClusterPowerAdjustStruct {\n");
    output.append("\tminPower: ");
    output.append(minPower);
    output.append("\n");
    output.append("\tmaxPower: ");
    output.append(maxPower);
    output.append("\n");
    output.append("\tminDuration: ");
    output.append(minDuration);
    output.append("\n");
    output.append("\tmaxDuration: ");
    output.append(maxDuration);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class DeviceEnergyManagementClusterPowerAdjustCapabilityStruct {
  public @Nullable ArrayList<ChipStructs.DeviceEnergyManagementClusterPowerAdjustStruct> powerAdjustCapability;
  public Integer cause;
  private static final long POWER_ADJUST_CAPABILITY_ID = 0L;
  private static final long CAUSE_ID = 1L;

  public DeviceEnergyManagementClusterPowerAdjustCapabilityStruct(
    @Nullable ArrayList<ChipStructs.DeviceEnergyManagementClusterPowerAdjustStruct> powerAdjustCapability,
    Integer cause
  ) {
    this.powerAdjustCapability = powerAdjustCapability;
    this.cause = cause;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(POWER_ADJUST_CAPABILITY_ID, powerAdjustCapability != null ? ArrayType.generateArrayType(powerAdjustCapability, (elementpowerAdjustCapability) -> elementpowerAdjustCapability.encodeTlv()) : new NullType()));
    values.add(new StructElement(CAUSE_ID, new UIntType(cause)));

    return new StructType(values);
  }

  public static DeviceEnergyManagementClusterPowerAdjustCapabilityStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    @Nullable ArrayList<ChipStructs.DeviceEnergyManagementClusterPowerAdjustStruct> powerAdjustCapability = null;
    Integer cause = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == POWER_ADJUST_CAPABILITY_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          powerAdjustCapability = castingValue.map((elementcastingValue) -> ChipStructs.DeviceEnergyManagementClusterPowerAdjustStruct.decodeTlv(elementcastingValue));
        }
      } else if (element.contextTagNum() == CAUSE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          cause = castingValue.value(Integer.class);
        }
      }
    }
    return new DeviceEnergyManagementClusterPowerAdjustCapabilityStruct(
      powerAdjustCapability,
      cause
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("DeviceEnergyManagementClusterPowerAdjustCapabilityStruct {\n");
    output.append("\tpowerAdjustCapability: ");
    output.append(powerAdjustCapability);
    output.append("\n");
    output.append("\tcause: ");
    output.append(cause);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class DeviceEnergyManagementClusterSlotStruct {
  public Long minDuration;
  public Long maxDuration;
  public Long defaultDuration;
  public Long elapsedSlotTime;
  public Long remainingSlotTime;
  public Optional<Boolean> slotIsPausable;
  public Optional<Long> minPauseDuration;
  public Optional<Long> maxPauseDuration;
  public Optional<Integer> manufacturerESAState;
  public Optional<Long> nominalPower;
  public Optional<Long> minPower;
  public Optional<Long> maxPower;
  public Optional<Long> nominalEnergy;
  public Optional<ArrayList<ChipStructs.DeviceEnergyManagementClusterCostStruct>> costs;
  public Optional<Long> minPowerAdjustment;
  public Optional<Long> maxPowerAdjustment;
  public Optional<Long> minDurationAdjustment;
  public Optional<Long> maxDurationAdjustment;
  private static final long MIN_DURATION_ID = 0L;
  private static final long MAX_DURATION_ID = 1L;
  private static final long DEFAULT_DURATION_ID = 2L;
  private static final long ELAPSED_SLOT_TIME_ID = 3L;
  private static final long REMAINING_SLOT_TIME_ID = 4L;
  private static final long SLOT_IS_PAUSABLE_ID = 5L;
  private static final long MIN_PAUSE_DURATION_ID = 6L;
  private static final long MAX_PAUSE_DURATION_ID = 7L;
  private static final long MANUFACTURER_ESA_STATE_ID = 8L;
  private static final long NOMINAL_POWER_ID = 9L;
  private static final long MIN_POWER_ID = 10L;
  private static final long MAX_POWER_ID = 11L;
  private static final long NOMINAL_ENERGY_ID = 12L;
  private static final long COSTS_ID = 13L;
  private static final long MIN_POWER_ADJUSTMENT_ID = 14L;
  private static final long MAX_POWER_ADJUSTMENT_ID = 15L;
  private static final long MIN_DURATION_ADJUSTMENT_ID = 16L;
  private static final long MAX_DURATION_ADJUSTMENT_ID = 17L;

  public DeviceEnergyManagementClusterSlotStruct(
    Long minDuration,
    Long maxDuration,
    Long defaultDuration,
    Long elapsedSlotTime,
    Long remainingSlotTime,
    Optional<Boolean> slotIsPausable,
    Optional<Long> minPauseDuration,
    Optional<Long> maxPauseDuration,
    Optional<Integer> manufacturerESAState,
    Optional<Long> nominalPower,
    Optional<Long> minPower,
    Optional<Long> maxPower,
    Optional<Long> nominalEnergy,
    Optional<ArrayList<ChipStructs.DeviceEnergyManagementClusterCostStruct>> costs,
    Optional<Long> minPowerAdjustment,
    Optional<Long> maxPowerAdjustment,
    Optional<Long> minDurationAdjustment,
    Optional<Long> maxDurationAdjustment
  ) {
    this.minDuration = minDuration;
    this.maxDuration = maxDuration;
    this.defaultDuration = defaultDuration;
    this.elapsedSlotTime = elapsedSlotTime;
    this.remainingSlotTime = remainingSlotTime;
    this.slotIsPausable = slotIsPausable;
    this.minPauseDuration = minPauseDuration;
    this.maxPauseDuration = maxPauseDuration;
    this.manufacturerESAState = manufacturerESAState;
    this.nominalPower = nominalPower;
    this.minPower = minPower;
    this.maxPower = maxPower;
    this.nominalEnergy = nominalEnergy;
    this.costs = costs;
    this.minPowerAdjustment = minPowerAdjustment;
    this.maxPowerAdjustment = maxPowerAdjustment;
    this.minDurationAdjustment = minDurationAdjustment;
    this.maxDurationAdjustment = maxDurationAdjustment;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(MIN_DURATION_ID, new UIntType(minDuration)));
    values.add(new StructElement(MAX_DURATION_ID, new UIntType(maxDuration)));
    values.add(new StructElement(DEFAULT_DURATION_ID, new UIntType(defaultDuration)));
    values.add(new StructElement(ELAPSED_SLOT_TIME_ID, new UIntType(elapsedSlotTime)));
    values.add(new StructElement(REMAINING_SLOT_TIME_ID, new UIntType(remainingSlotTime)));
    values.add(new StructElement(SLOT_IS_PAUSABLE_ID, slotIsPausable.<BaseTLVType>map((nonOptionalslotIsPausable) -> new BooleanType(nonOptionalslotIsPausable)).orElse(new EmptyType())));
    values.add(new StructElement(MIN_PAUSE_DURATION_ID, minPauseDuration.<BaseTLVType>map((nonOptionalminPauseDuration) -> new UIntType(nonOptionalminPauseDuration)).orElse(new EmptyType())));
    values.add(new StructElement(MAX_PAUSE_DURATION_ID, maxPauseDuration.<BaseTLVType>map((nonOptionalmaxPauseDuration) -> new UIntType(nonOptionalmaxPauseDuration)).orElse(new EmptyType())));
    values.add(new StructElement(MANUFACTURER_ESA_STATE_ID, manufacturerESAState.<BaseTLVType>map((nonOptionalmanufacturerESAState) -> new UIntType(nonOptionalmanufacturerESAState)).orElse(new EmptyType())));
    values.add(new StructElement(NOMINAL_POWER_ID, nominalPower.<BaseTLVType>map((nonOptionalnominalPower) -> new IntType(nonOptionalnominalPower)).orElse(new EmptyType())));
    values.add(new StructElement(MIN_POWER_ID, minPower.<BaseTLVType>map((nonOptionalminPower) -> new IntType(nonOptionalminPower)).orElse(new EmptyType())));
    values.add(new StructElement(MAX_POWER_ID, maxPower.<BaseTLVType>map((nonOptionalmaxPower) -> new IntType(nonOptionalmaxPower)).orElse(new EmptyType())));
    values.add(new StructElement(NOMINAL_ENERGY_ID, nominalEnergy.<BaseTLVType>map((nonOptionalnominalEnergy) -> new IntType(nonOptionalnominalEnergy)).orElse(new EmptyType())));
    values.add(new StructElement(COSTS_ID, costs.<BaseTLVType>map((nonOptionalcosts) -> ArrayType.generateArrayType(nonOptionalcosts, (elementnonOptionalcosts) -> elementnonOptionalcosts.encodeTlv())).orElse(new EmptyType())));
    values.add(new StructElement(MIN_POWER_ADJUSTMENT_ID, minPowerAdjustment.<BaseTLVType>map((nonOptionalminPowerAdjustment) -> new IntType(nonOptionalminPowerAdjustment)).orElse(new EmptyType())));
    values.add(new StructElement(MAX_POWER_ADJUSTMENT_ID, maxPowerAdjustment.<BaseTLVType>map((nonOptionalmaxPowerAdjustment) -> new IntType(nonOptionalmaxPowerAdjustment)).orElse(new EmptyType())));
    values.add(new StructElement(MIN_DURATION_ADJUSTMENT_ID, minDurationAdjustment.<BaseTLVType>map((nonOptionalminDurationAdjustment) -> new UIntType(nonOptionalminDurationAdjustment)).orElse(new EmptyType())));
    values.add(new StructElement(MAX_DURATION_ADJUSTMENT_ID, maxDurationAdjustment.<BaseTLVType>map((nonOptionalmaxDurationAdjustment) -> new UIntType(nonOptionalmaxDurationAdjustment)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static DeviceEnergyManagementClusterSlotStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long minDuration = null;
    Long maxDuration = null;
    Long defaultDuration = null;
    Long elapsedSlotTime = null;
    Long remainingSlotTime = null;
    Optional<Boolean> slotIsPausable = Optional.empty();
    Optional<Long> minPauseDuration = Optional.empty();
    Optional<Long> maxPauseDuration = Optional.empty();
    Optional<Integer> manufacturerESAState = Optional.empty();
    Optional<Long> nominalPower = Optional.empty();
    Optional<Long> minPower = Optional.empty();
    Optional<Long> maxPower = Optional.empty();
    Optional<Long> nominalEnergy = Optional.empty();
    Optional<ArrayList<ChipStructs.DeviceEnergyManagementClusterCostStruct>> costs = Optional.empty();
    Optional<Long> minPowerAdjustment = Optional.empty();
    Optional<Long> maxPowerAdjustment = Optional.empty();
    Optional<Long> minDurationAdjustment = Optional.empty();
    Optional<Long> maxDurationAdjustment = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == MIN_DURATION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          minDuration = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == MAX_DURATION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          maxDuration = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == DEFAULT_DURATION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          defaultDuration = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == ELAPSED_SLOT_TIME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          elapsedSlotTime = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == REMAINING_SLOT_TIME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          remainingSlotTime = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == SLOT_IS_PAUSABLE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          slotIsPausable = Optional.of(castingValue.value(Boolean.class));
        }
      } else if (element.contextTagNum() == MIN_PAUSE_DURATION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          minPauseDuration = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == MAX_PAUSE_DURATION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          maxPauseDuration = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == MANUFACTURER_ESA_STATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          manufacturerESAState = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == NOMINAL_POWER_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          nominalPower = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == MIN_POWER_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          minPower = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == MAX_POWER_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          maxPower = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == NOMINAL_ENERGY_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          nominalEnergy = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == COSTS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          costs = Optional.of(castingValue.map((elementcastingValue) -> ChipStructs.DeviceEnergyManagementClusterCostStruct.decodeTlv(elementcastingValue)));
        }
      } else if (element.contextTagNum() == MIN_POWER_ADJUSTMENT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          minPowerAdjustment = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == MAX_POWER_ADJUSTMENT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          maxPowerAdjustment = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == MIN_DURATION_ADJUSTMENT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          minDurationAdjustment = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == MAX_DURATION_ADJUSTMENT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          maxDurationAdjustment = Optional.of(castingValue.value(Long.class));
        }
      }
    }
    return new DeviceEnergyManagementClusterSlotStruct(
      minDuration,
      maxDuration,
      defaultDuration,
      elapsedSlotTime,
      remainingSlotTime,
      slotIsPausable,
      minPauseDuration,
      maxPauseDuration,
      manufacturerESAState,
      nominalPower,
      minPower,
      maxPower,
      nominalEnergy,
      costs,
      minPowerAdjustment,
      maxPowerAdjustment,
      minDurationAdjustment,
      maxDurationAdjustment
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("DeviceEnergyManagementClusterSlotStruct {\n");
    output.append("\tminDuration: ");
    output.append(minDuration);
    output.append("\n");
    output.append("\tmaxDuration: ");
    output.append(maxDuration);
    output.append("\n");
    output.append("\tdefaultDuration: ");
    output.append(defaultDuration);
    output.append("\n");
    output.append("\telapsedSlotTime: ");
    output.append(elapsedSlotTime);
    output.append("\n");
    output.append("\tremainingSlotTime: ");
    output.append(remainingSlotTime);
    output.append("\n");
    output.append("\tslotIsPausable: ");
    output.append(slotIsPausable);
    output.append("\n");
    output.append("\tminPauseDuration: ");
    output.append(minPauseDuration);
    output.append("\n");
    output.append("\tmaxPauseDuration: ");
    output.append(maxPauseDuration);
    output.append("\n");
    output.append("\tmanufacturerESAState: ");
    output.append(manufacturerESAState);
    output.append("\n");
    output.append("\tnominalPower: ");
    output.append(nominalPower);
    output.append("\n");
    output.append("\tminPower: ");
    output.append(minPower);
    output.append("\n");
    output.append("\tmaxPower: ");
    output.append(maxPower);
    output.append("\n");
    output.append("\tnominalEnergy: ");
    output.append(nominalEnergy);
    output.append("\n");
    output.append("\tcosts: ");
    output.append(costs);
    output.append("\n");
    output.append("\tminPowerAdjustment: ");
    output.append(minPowerAdjustment);
    output.append("\n");
    output.append("\tmaxPowerAdjustment: ");
    output.append(maxPowerAdjustment);
    output.append("\n");
    output.append("\tminDurationAdjustment: ");
    output.append(minDurationAdjustment);
    output.append("\n");
    output.append("\tmaxDurationAdjustment: ");
    output.append(maxDurationAdjustment);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class DeviceEnergyManagementClusterForecastStruct {
  public Long forecastID;
  public @Nullable Integer activeSlotNumber;
  public Long startTime;
  public Long endTime;
  public @Nullable Optional<Long> earliestStartTime;
  public Optional<Long> latestEndTime;
  public Boolean isPausable;
  public ArrayList<ChipStructs.DeviceEnergyManagementClusterSlotStruct> slots;
  public Integer forecastUpdateReason;
  private static final long FORECAST_ID_ID = 0L;
  private static final long ACTIVE_SLOT_NUMBER_ID = 1L;
  private static final long START_TIME_ID = 2L;
  private static final long END_TIME_ID = 3L;
  private static final long EARLIEST_START_TIME_ID = 4L;
  private static final long LATEST_END_TIME_ID = 5L;
  private static final long IS_PAUSABLE_ID = 6L;
  private static final long SLOTS_ID = 7L;
  private static final long FORECAST_UPDATE_REASON_ID = 8L;

  public DeviceEnergyManagementClusterForecastStruct(
    Long forecastID,
    @Nullable Integer activeSlotNumber,
    Long startTime,
    Long endTime,
    @Nullable Optional<Long> earliestStartTime,
    Optional<Long> latestEndTime,
    Boolean isPausable,
    ArrayList<ChipStructs.DeviceEnergyManagementClusterSlotStruct> slots,
    Integer forecastUpdateReason
  ) {
    this.forecastID = forecastID;
    this.activeSlotNumber = activeSlotNumber;
    this.startTime = startTime;
    this.endTime = endTime;
    this.earliestStartTime = earliestStartTime;
    this.latestEndTime = latestEndTime;
    this.isPausable = isPausable;
    this.slots = slots;
    this.forecastUpdateReason = forecastUpdateReason;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(FORECAST_ID_ID, new UIntType(forecastID)));
    values.add(new StructElement(ACTIVE_SLOT_NUMBER_ID, activeSlotNumber != null ? new UIntType(activeSlotNumber) : new NullType()));
    values.add(new StructElement(START_TIME_ID, new UIntType(startTime)));
    values.add(new StructElement(END_TIME_ID, new UIntType(endTime)));
    values.add(new StructElement(EARLIEST_START_TIME_ID, earliestStartTime != null ? earliestStartTime.<BaseTLVType>map((nonOptionalearliestStartTime) -> new UIntType(nonOptionalearliestStartTime)).orElse(new EmptyType()) : new NullType()));
    values.add(new StructElement(LATEST_END_TIME_ID, latestEndTime.<BaseTLVType>map((nonOptionallatestEndTime) -> new UIntType(nonOptionallatestEndTime)).orElse(new EmptyType())));
    values.add(new StructElement(IS_PAUSABLE_ID, new BooleanType(isPausable)));
    values.add(new StructElement(SLOTS_ID, ArrayType.generateArrayType(slots, (elementslots) -> elementslots.encodeTlv())));
    values.add(new StructElement(FORECAST_UPDATE_REASON_ID, new UIntType(forecastUpdateReason)));

    return new StructType(values);
  }

  public static DeviceEnergyManagementClusterForecastStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long forecastID = null;
    @Nullable Integer activeSlotNumber = null;
    Long startTime = null;
    Long endTime = null;
    @Nullable Optional<Long> earliestStartTime = null;
    Optional<Long> latestEndTime = Optional.empty();
    Boolean isPausable = null;
    ArrayList<ChipStructs.DeviceEnergyManagementClusterSlotStruct> slots = null;
    Integer forecastUpdateReason = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == FORECAST_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          forecastID = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == ACTIVE_SLOT_NUMBER_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          activeSlotNumber = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == START_TIME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          startTime = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == END_TIME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          endTime = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == EARLIEST_START_TIME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          earliestStartTime = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == LATEST_END_TIME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          latestEndTime = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == IS_PAUSABLE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          isPausable = castingValue.value(Boolean.class);
        }
      } else if (element.contextTagNum() == SLOTS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          slots = castingValue.map((elementcastingValue) -> ChipStructs.DeviceEnergyManagementClusterSlotStruct.decodeTlv(elementcastingValue));
        }
      } else if (element.contextTagNum() == FORECAST_UPDATE_REASON_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          forecastUpdateReason = castingValue.value(Integer.class);
        }
      }
    }
    return new DeviceEnergyManagementClusterForecastStruct(
      forecastID,
      activeSlotNumber,
      startTime,
      endTime,
      earliestStartTime,
      latestEndTime,
      isPausable,
      slots,
      forecastUpdateReason
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("DeviceEnergyManagementClusterForecastStruct {\n");
    output.append("\tforecastID: ");
    output.append(forecastID);
    output.append("\n");
    output.append("\tactiveSlotNumber: ");
    output.append(activeSlotNumber);
    output.append("\n");
    output.append("\tstartTime: ");
    output.append(startTime);
    output.append("\n");
    output.append("\tendTime: ");
    output.append(endTime);
    output.append("\n");
    output.append("\tearliestStartTime: ");
    output.append(earliestStartTime);
    output.append("\n");
    output.append("\tlatestEndTime: ");
    output.append(latestEndTime);
    output.append("\n");
    output.append("\tisPausable: ");
    output.append(isPausable);
    output.append("\n");
    output.append("\tslots: ");
    output.append(slots);
    output.append("\n");
    output.append("\tforecastUpdateReason: ");
    output.append(forecastUpdateReason);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class DeviceEnergyManagementClusterConstraintsStruct {
  public Long startTime;
  public Long duration;
  public Optional<Long> nominalPower;
  public Optional<Long> maximumEnergy;
  public Optional<Integer> loadControl;
  private static final long START_TIME_ID = 0L;
  private static final long DURATION_ID = 1L;
  private static final long NOMINAL_POWER_ID = 2L;
  private static final long MAXIMUM_ENERGY_ID = 3L;
  private static final long LOAD_CONTROL_ID = 4L;

  public DeviceEnergyManagementClusterConstraintsStruct(
    Long startTime,
    Long duration,
    Optional<Long> nominalPower,
    Optional<Long> maximumEnergy,
    Optional<Integer> loadControl
  ) {
    this.startTime = startTime;
    this.duration = duration;
    this.nominalPower = nominalPower;
    this.maximumEnergy = maximumEnergy;
    this.loadControl = loadControl;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(START_TIME_ID, new UIntType(startTime)));
    values.add(new StructElement(DURATION_ID, new UIntType(duration)));
    values.add(new StructElement(NOMINAL_POWER_ID, nominalPower.<BaseTLVType>map((nonOptionalnominalPower) -> new IntType(nonOptionalnominalPower)).orElse(new EmptyType())));
    values.add(new StructElement(MAXIMUM_ENERGY_ID, maximumEnergy.<BaseTLVType>map((nonOptionalmaximumEnergy) -> new IntType(nonOptionalmaximumEnergy)).orElse(new EmptyType())));
    values.add(new StructElement(LOAD_CONTROL_ID, loadControl.<BaseTLVType>map((nonOptionalloadControl) -> new IntType(nonOptionalloadControl)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static DeviceEnergyManagementClusterConstraintsStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long startTime = null;
    Long duration = null;
    Optional<Long> nominalPower = Optional.empty();
    Optional<Long> maximumEnergy = Optional.empty();
    Optional<Integer> loadControl = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == START_TIME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          startTime = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == DURATION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          duration = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == NOMINAL_POWER_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          nominalPower = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == MAXIMUM_ENERGY_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          maximumEnergy = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == LOAD_CONTROL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          loadControl = Optional.of(castingValue.value(Integer.class));
        }
      }
    }
    return new DeviceEnergyManagementClusterConstraintsStruct(
      startTime,
      duration,
      nominalPower,
      maximumEnergy,
      loadControl
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("DeviceEnergyManagementClusterConstraintsStruct {\n");
    output.append("\tstartTime: ");
    output.append(startTime);
    output.append("\n");
    output.append("\tduration: ");
    output.append(duration);
    output.append("\n");
    output.append("\tnominalPower: ");
    output.append(nominalPower);
    output.append("\n");
    output.append("\tmaximumEnergy: ");
    output.append(maximumEnergy);
    output.append("\n");
    output.append("\tloadControl: ");
    output.append(loadControl);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class DeviceEnergyManagementClusterSlotAdjustmentStruct {
  public Integer slotIndex;
  public Optional<Long> nominalPower;
  public Long duration;
  private static final long SLOT_INDEX_ID = 0L;
  private static final long NOMINAL_POWER_ID = 1L;
  private static final long DURATION_ID = 2L;

  public DeviceEnergyManagementClusterSlotAdjustmentStruct(
    Integer slotIndex,
    Optional<Long> nominalPower,
    Long duration
  ) {
    this.slotIndex = slotIndex;
    this.nominalPower = nominalPower;
    this.duration = duration;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(SLOT_INDEX_ID, new UIntType(slotIndex)));
    values.add(new StructElement(NOMINAL_POWER_ID, nominalPower.<BaseTLVType>map((nonOptionalnominalPower) -> new IntType(nonOptionalnominalPower)).orElse(new EmptyType())));
    values.add(new StructElement(DURATION_ID, new UIntType(duration)));

    return new StructType(values);
  }

  public static DeviceEnergyManagementClusterSlotAdjustmentStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer slotIndex = null;
    Optional<Long> nominalPower = Optional.empty();
    Long duration = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == SLOT_INDEX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          slotIndex = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == NOMINAL_POWER_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          nominalPower = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == DURATION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          duration = castingValue.value(Long.class);
        }
      }
    }
    return new DeviceEnergyManagementClusterSlotAdjustmentStruct(
      slotIndex,
      nominalPower,
      duration
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("DeviceEnergyManagementClusterSlotAdjustmentStruct {\n");
    output.append("\tslotIndex: ");
    output.append(slotIndex);
    output.append("\n");
    output.append("\tnominalPower: ");
    output.append(nominalPower);
    output.append("\n");
    output.append("\tduration: ");
    output.append(duration);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class EnergyEvseClusterChargingTargetStruct {
  public Integer targetTimeMinutesPastMidnight;
  public Optional<Integer> targetSoC;
  public Optional<Long> addedEnergy;
  private static final long TARGET_TIME_MINUTES_PAST_MIDNIGHT_ID = 0L;
  private static final long TARGET_SO_C_ID = 1L;
  private static final long ADDED_ENERGY_ID = 2L;

  public EnergyEvseClusterChargingTargetStruct(
    Integer targetTimeMinutesPastMidnight,
    Optional<Integer> targetSoC,
    Optional<Long> addedEnergy
  ) {
    this.targetTimeMinutesPastMidnight = targetTimeMinutesPastMidnight;
    this.targetSoC = targetSoC;
    this.addedEnergy = addedEnergy;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(TARGET_TIME_MINUTES_PAST_MIDNIGHT_ID, new UIntType(targetTimeMinutesPastMidnight)));
    values.add(new StructElement(TARGET_SO_C_ID, targetSoC.<BaseTLVType>map((nonOptionaltargetSoC) -> new UIntType(nonOptionaltargetSoC)).orElse(new EmptyType())));
    values.add(new StructElement(ADDED_ENERGY_ID, addedEnergy.<BaseTLVType>map((nonOptionaladdedEnergy) -> new IntType(nonOptionaladdedEnergy)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static EnergyEvseClusterChargingTargetStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer targetTimeMinutesPastMidnight = null;
    Optional<Integer> targetSoC = Optional.empty();
    Optional<Long> addedEnergy = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == TARGET_TIME_MINUTES_PAST_MIDNIGHT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          targetTimeMinutesPastMidnight = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == TARGET_SO_C_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          targetSoC = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == ADDED_ENERGY_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          addedEnergy = Optional.of(castingValue.value(Long.class));
        }
      }
    }
    return new EnergyEvseClusterChargingTargetStruct(
      targetTimeMinutesPastMidnight,
      targetSoC,
      addedEnergy
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("EnergyEvseClusterChargingTargetStruct {\n");
    output.append("\ttargetTimeMinutesPastMidnight: ");
    output.append(targetTimeMinutesPastMidnight);
    output.append("\n");
    output.append("\ttargetSoC: ");
    output.append(targetSoC);
    output.append("\n");
    output.append("\taddedEnergy: ");
    output.append(addedEnergy);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class EnergyEvseClusterChargingTargetScheduleStruct {
  public Integer dayOfWeekForSequence;
  public ArrayList<ChipStructs.EnergyEvseClusterChargingTargetStruct> chargingTargets;
  private static final long DAY_OF_WEEK_FOR_SEQUENCE_ID = 0L;
  private static final long CHARGING_TARGETS_ID = 1L;

  public EnergyEvseClusterChargingTargetScheduleStruct(
    Integer dayOfWeekForSequence,
    ArrayList<ChipStructs.EnergyEvseClusterChargingTargetStruct> chargingTargets
  ) {
    this.dayOfWeekForSequence = dayOfWeekForSequence;
    this.chargingTargets = chargingTargets;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(DAY_OF_WEEK_FOR_SEQUENCE_ID, new UIntType(dayOfWeekForSequence)));
    values.add(new StructElement(CHARGING_TARGETS_ID, ArrayType.generateArrayType(chargingTargets, (elementchargingTargets) -> elementchargingTargets.encodeTlv())));

    return new StructType(values);
  }

  public static EnergyEvseClusterChargingTargetScheduleStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer dayOfWeekForSequence = null;
    ArrayList<ChipStructs.EnergyEvseClusterChargingTargetStruct> chargingTargets = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == DAY_OF_WEEK_FOR_SEQUENCE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          dayOfWeekForSequence = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == CHARGING_TARGETS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          chargingTargets = castingValue.map((elementcastingValue) -> ChipStructs.EnergyEvseClusterChargingTargetStruct.decodeTlv(elementcastingValue));
        }
      }
    }
    return new EnergyEvseClusterChargingTargetScheduleStruct(
      dayOfWeekForSequence,
      chargingTargets
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("EnergyEvseClusterChargingTargetScheduleStruct {\n");
    output.append("\tdayOfWeekForSequence: ");
    output.append(dayOfWeekForSequence);
    output.append("\n");
    output.append("\tchargingTargets: ");
    output.append(chargingTargets);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class EnergyPreferenceClusterBalanceStruct {
  public Integer step;
  public Optional<String> label;
  private static final long STEP_ID = 0L;
  private static final long LABEL_ID = 1L;

  public EnergyPreferenceClusterBalanceStruct(
    Integer step,
    Optional<String> label
  ) {
    this.step = step;
    this.label = label;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(STEP_ID, new UIntType(step)));
    values.add(new StructElement(LABEL_ID, label.<BaseTLVType>map((nonOptionallabel) -> new StringType(nonOptionallabel)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static EnergyPreferenceClusterBalanceStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer step = null;
    Optional<String> label = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == STEP_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          step = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == LABEL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          label = Optional.of(castingValue.value(String.class));
        }
      }
    }
    return new EnergyPreferenceClusterBalanceStruct(
      step,
      label
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("EnergyPreferenceClusterBalanceStruct {\n");
    output.append("\tstep: ");
    output.append(step);
    output.append("\n");
    output.append("\tlabel: ");
    output.append(label);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class EnergyEvseModeClusterModeTagStruct {
  public Optional<Integer> mfgCode;
  public Integer value;
  private static final long MFG_CODE_ID = 0L;
  private static final long VALUE_ID = 1L;

  public EnergyEvseModeClusterModeTagStruct(
    Optional<Integer> mfgCode,
    Integer value
  ) {
    this.mfgCode = mfgCode;
    this.value = value;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(MFG_CODE_ID, mfgCode.<BaseTLVType>map((nonOptionalmfgCode) -> new UIntType(nonOptionalmfgCode)).orElse(new EmptyType())));
    values.add(new StructElement(VALUE_ID, new UIntType(value)));

    return new StructType(values);
  }

  public static EnergyEvseModeClusterModeTagStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Optional<Integer> mfgCode = Optional.empty();
    Integer value = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == MFG_CODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          mfgCode = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == VALUE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          value = castingValue.value(Integer.class);
        }
      }
    }
    return new EnergyEvseModeClusterModeTagStruct(
      mfgCode,
      value
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("EnergyEvseModeClusterModeTagStruct {\n");
    output.append("\tmfgCode: ");
    output.append(mfgCode);
    output.append("\n");
    output.append("\tvalue: ");
    output.append(value);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class EnergyEvseModeClusterModeOptionStruct {
  public String label;
  public Integer mode;
  public ArrayList<ChipStructs.EnergyEvseModeClusterModeTagStruct> modeTags;
  private static final long LABEL_ID = 0L;
  private static final long MODE_ID = 1L;
  private static final long MODE_TAGS_ID = 2L;

  public EnergyEvseModeClusterModeOptionStruct(
    String label,
    Integer mode,
    ArrayList<ChipStructs.EnergyEvseModeClusterModeTagStruct> modeTags
  ) {
    this.label = label;
    this.mode = mode;
    this.modeTags = modeTags;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(LABEL_ID, new StringType(label)));
    values.add(new StructElement(MODE_ID, new UIntType(mode)));
    values.add(new StructElement(MODE_TAGS_ID, ArrayType.generateArrayType(modeTags, (elementmodeTags) -> elementmodeTags.encodeTlv())));

    return new StructType(values);
  }

  public static EnergyEvseModeClusterModeOptionStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    String label = null;
    Integer mode = null;
    ArrayList<ChipStructs.EnergyEvseModeClusterModeTagStruct> modeTags = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == LABEL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          label = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == MODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          mode = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == MODE_TAGS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          modeTags = castingValue.map((elementcastingValue) -> ChipStructs.EnergyEvseModeClusterModeTagStruct.decodeTlv(elementcastingValue));
        }
      }
    }
    return new EnergyEvseModeClusterModeOptionStruct(
      label,
      mode,
      modeTags
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("EnergyEvseModeClusterModeOptionStruct {\n");
    output.append("\tlabel: ");
    output.append(label);
    output.append("\n");
    output.append("\tmode: ");
    output.append(mode);
    output.append("\n");
    output.append("\tmodeTags: ");
    output.append(modeTags);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class WaterHeaterModeClusterModeTagStruct {
  public Optional<Integer> mfgCode;
  public Integer value;
  private static final long MFG_CODE_ID = 0L;
  private static final long VALUE_ID = 1L;

  public WaterHeaterModeClusterModeTagStruct(
    Optional<Integer> mfgCode,
    Integer value
  ) {
    this.mfgCode = mfgCode;
    this.value = value;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(MFG_CODE_ID, mfgCode.<BaseTLVType>map((nonOptionalmfgCode) -> new UIntType(nonOptionalmfgCode)).orElse(new EmptyType())));
    values.add(new StructElement(VALUE_ID, new UIntType(value)));

    return new StructType(values);
  }

  public static WaterHeaterModeClusterModeTagStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Optional<Integer> mfgCode = Optional.empty();
    Integer value = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == MFG_CODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          mfgCode = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == VALUE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          value = castingValue.value(Integer.class);
        }
      }
    }
    return new WaterHeaterModeClusterModeTagStruct(
      mfgCode,
      value
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("WaterHeaterModeClusterModeTagStruct {\n");
    output.append("\tmfgCode: ");
    output.append(mfgCode);
    output.append("\n");
    output.append("\tvalue: ");
    output.append(value);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class WaterHeaterModeClusterModeOptionStruct {
  public String label;
  public Integer mode;
  public ArrayList<ChipStructs.WaterHeaterModeClusterModeTagStruct> modeTags;
  private static final long LABEL_ID = 0L;
  private static final long MODE_ID = 1L;
  private static final long MODE_TAGS_ID = 2L;

  public WaterHeaterModeClusterModeOptionStruct(
    String label,
    Integer mode,
    ArrayList<ChipStructs.WaterHeaterModeClusterModeTagStruct> modeTags
  ) {
    this.label = label;
    this.mode = mode;
    this.modeTags = modeTags;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(LABEL_ID, new StringType(label)));
    values.add(new StructElement(MODE_ID, new UIntType(mode)));
    values.add(new StructElement(MODE_TAGS_ID, ArrayType.generateArrayType(modeTags, (elementmodeTags) -> elementmodeTags.encodeTlv())));

    return new StructType(values);
  }

  public static WaterHeaterModeClusterModeOptionStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    String label = null;
    Integer mode = null;
    ArrayList<ChipStructs.WaterHeaterModeClusterModeTagStruct> modeTags = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == LABEL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          label = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == MODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          mode = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == MODE_TAGS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          modeTags = castingValue.map((elementcastingValue) -> ChipStructs.WaterHeaterModeClusterModeTagStruct.decodeTlv(elementcastingValue));
        }
      }
    }
    return new WaterHeaterModeClusterModeOptionStruct(
      label,
      mode,
      modeTags
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("WaterHeaterModeClusterModeOptionStruct {\n");
    output.append("\tlabel: ");
    output.append(label);
    output.append("\n");
    output.append("\tmode: ");
    output.append(mode);
    output.append("\n");
    output.append("\tmodeTags: ");
    output.append(modeTags);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class DeviceEnergyManagementModeClusterModeTagStruct {
  public Optional<Integer> mfgCode;
  public Integer value;
  private static final long MFG_CODE_ID = 0L;
  private static final long VALUE_ID = 1L;

  public DeviceEnergyManagementModeClusterModeTagStruct(
    Optional<Integer> mfgCode,
    Integer value
  ) {
    this.mfgCode = mfgCode;
    this.value = value;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(MFG_CODE_ID, mfgCode.<BaseTLVType>map((nonOptionalmfgCode) -> new UIntType(nonOptionalmfgCode)).orElse(new EmptyType())));
    values.add(new StructElement(VALUE_ID, new UIntType(value)));

    return new StructType(values);
  }

  public static DeviceEnergyManagementModeClusterModeTagStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Optional<Integer> mfgCode = Optional.empty();
    Integer value = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == MFG_CODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          mfgCode = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == VALUE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          value = castingValue.value(Integer.class);
        }
      }
    }
    return new DeviceEnergyManagementModeClusterModeTagStruct(
      mfgCode,
      value
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("DeviceEnergyManagementModeClusterModeTagStruct {\n");
    output.append("\tmfgCode: ");
    output.append(mfgCode);
    output.append("\n");
    output.append("\tvalue: ");
    output.append(value);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class DeviceEnergyManagementModeClusterModeOptionStruct {
  public String label;
  public Integer mode;
  public ArrayList<ChipStructs.DeviceEnergyManagementModeClusterModeTagStruct> modeTags;
  private static final long LABEL_ID = 0L;
  private static final long MODE_ID = 1L;
  private static final long MODE_TAGS_ID = 2L;

  public DeviceEnergyManagementModeClusterModeOptionStruct(
    String label,
    Integer mode,
    ArrayList<ChipStructs.DeviceEnergyManagementModeClusterModeTagStruct> modeTags
  ) {
    this.label = label;
    this.mode = mode;
    this.modeTags = modeTags;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(LABEL_ID, new StringType(label)));
    values.add(new StructElement(MODE_ID, new UIntType(mode)));
    values.add(new StructElement(MODE_TAGS_ID, ArrayType.generateArrayType(modeTags, (elementmodeTags) -> elementmodeTags.encodeTlv())));

    return new StructType(values);
  }

  public static DeviceEnergyManagementModeClusterModeOptionStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    String label = null;
    Integer mode = null;
    ArrayList<ChipStructs.DeviceEnergyManagementModeClusterModeTagStruct> modeTags = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == LABEL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          label = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == MODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          mode = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == MODE_TAGS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          modeTags = castingValue.map((elementcastingValue) -> ChipStructs.DeviceEnergyManagementModeClusterModeTagStruct.decodeTlv(elementcastingValue));
        }
      }
    }
    return new DeviceEnergyManagementModeClusterModeOptionStruct(
      label,
      mode,
      modeTags
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("DeviceEnergyManagementModeClusterModeOptionStruct {\n");
    output.append("\tlabel: ");
    output.append(label);
    output.append("\n");
    output.append("\tmode: ");
    output.append(mode);
    output.append("\n");
    output.append("\tmodeTags: ");
    output.append(modeTags);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class DoorLockClusterCredentialStruct {
  public Integer credentialType;
  public Integer credentialIndex;
  private static final long CREDENTIAL_TYPE_ID = 0L;
  private static final long CREDENTIAL_INDEX_ID = 1L;

  public DoorLockClusterCredentialStruct(
    Integer credentialType,
    Integer credentialIndex
  ) {
    this.credentialType = credentialType;
    this.credentialIndex = credentialIndex;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(CREDENTIAL_TYPE_ID, new UIntType(credentialType)));
    values.add(new StructElement(CREDENTIAL_INDEX_ID, new UIntType(credentialIndex)));

    return new StructType(values);
  }

  public static DoorLockClusterCredentialStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer credentialType = null;
    Integer credentialIndex = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == CREDENTIAL_TYPE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          credentialType = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == CREDENTIAL_INDEX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          credentialIndex = castingValue.value(Integer.class);
        }
      }
    }
    return new DoorLockClusterCredentialStruct(
      credentialType,
      credentialIndex
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("DoorLockClusterCredentialStruct {\n");
    output.append("\tcredentialType: ");
    output.append(credentialType);
    output.append("\n");
    output.append("\tcredentialIndex: ");
    output.append(credentialIndex);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ClosureControlClusterOverallStateStruct {
  public Optional<Integer> positioning;
  public Optional<Integer> latching;
  public Optional<Integer> speed;
  public Optional<Long> extraInfo;
  private static final long POSITIONING_ID = 0L;
  private static final long LATCHING_ID = 1L;
  private static final long SPEED_ID = 2L;
  private static final long EXTRA_INFO_ID = 3L;

  public ClosureControlClusterOverallStateStruct(
    Optional<Integer> positioning,
    Optional<Integer> latching,
    Optional<Integer> speed,
    Optional<Long> extraInfo
  ) {
    this.positioning = positioning;
    this.latching = latching;
    this.speed = speed;
    this.extraInfo = extraInfo;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(POSITIONING_ID, positioning.<BaseTLVType>map((nonOptionalpositioning) -> new UIntType(nonOptionalpositioning)).orElse(new EmptyType())));
    values.add(new StructElement(LATCHING_ID, latching.<BaseTLVType>map((nonOptionallatching) -> new UIntType(nonOptionallatching)).orElse(new EmptyType())));
    values.add(new StructElement(SPEED_ID, speed.<BaseTLVType>map((nonOptionalspeed) -> new UIntType(nonOptionalspeed)).orElse(new EmptyType())));
    values.add(new StructElement(EXTRA_INFO_ID, extraInfo.<BaseTLVType>map((nonOptionalextraInfo) -> new UIntType(nonOptionalextraInfo)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static ClosureControlClusterOverallStateStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Optional<Integer> positioning = Optional.empty();
    Optional<Integer> latching = Optional.empty();
    Optional<Integer> speed = Optional.empty();
    Optional<Long> extraInfo = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == POSITIONING_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          positioning = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == LATCHING_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          latching = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == SPEED_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          speed = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == EXTRA_INFO_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          extraInfo = Optional.of(castingValue.value(Long.class));
        }
      }
    }
    return new ClosureControlClusterOverallStateStruct(
      positioning,
      latching,
      speed,
      extraInfo
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ClosureControlClusterOverallStateStruct {\n");
    output.append("\tpositioning: ");
    output.append(positioning);
    output.append("\n");
    output.append("\tlatching: ");
    output.append(latching);
    output.append("\n");
    output.append("\tspeed: ");
    output.append(speed);
    output.append("\n");
    output.append("\textraInfo: ");
    output.append(extraInfo);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ClosureControlClusterOverallTargetStruct {
  public Optional<Integer> tagPosition;
  public Optional<Integer> tagLatch;
  public Optional<Integer> speed;
  private static final long TAG_POSITION_ID = 0L;
  private static final long TAG_LATCH_ID = 1L;
  private static final long SPEED_ID = 2L;

  public ClosureControlClusterOverallTargetStruct(
    Optional<Integer> tagPosition,
    Optional<Integer> tagLatch,
    Optional<Integer> speed
  ) {
    this.tagPosition = tagPosition;
    this.tagLatch = tagLatch;
    this.speed = speed;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(TAG_POSITION_ID, tagPosition.<BaseTLVType>map((nonOptionaltagPosition) -> new UIntType(nonOptionaltagPosition)).orElse(new EmptyType())));
    values.add(new StructElement(TAG_LATCH_ID, tagLatch.<BaseTLVType>map((nonOptionaltagLatch) -> new UIntType(nonOptionaltagLatch)).orElse(new EmptyType())));
    values.add(new StructElement(SPEED_ID, speed.<BaseTLVType>map((nonOptionalspeed) -> new UIntType(nonOptionalspeed)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static ClosureControlClusterOverallTargetStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Optional<Integer> tagPosition = Optional.empty();
    Optional<Integer> tagLatch = Optional.empty();
    Optional<Integer> speed = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == TAG_POSITION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          tagPosition = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == TAG_LATCH_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          tagLatch = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == SPEED_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          speed = Optional.of(castingValue.value(Integer.class));
        }
      }
    }
    return new ClosureControlClusterOverallTargetStruct(
      tagPosition,
      tagLatch,
      speed
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ClosureControlClusterOverallTargetStruct {\n");
    output.append("\ttagPosition: ");
    output.append(tagPosition);
    output.append("\n");
    output.append("\ttagLatch: ");
    output.append(tagLatch);
    output.append("\n");
    output.append("\tspeed: ");
    output.append(speed);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ServiceAreaClusterLandmarkInfoStruct {
  public Integer landmarkTag;
  public @Nullable Integer relativePositionTag;
  private static final long LANDMARK_TAG_ID = 0L;
  private static final long RELATIVE_POSITION_TAG_ID = 1L;

  public ServiceAreaClusterLandmarkInfoStruct(
    Integer landmarkTag,
    @Nullable Integer relativePositionTag
  ) {
    this.landmarkTag = landmarkTag;
    this.relativePositionTag = relativePositionTag;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(LANDMARK_TAG_ID, new UIntType(landmarkTag)));
    values.add(new StructElement(RELATIVE_POSITION_TAG_ID, relativePositionTag != null ? new UIntType(relativePositionTag) : new NullType()));

    return new StructType(values);
  }

  public static ServiceAreaClusterLandmarkInfoStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer landmarkTag = null;
    @Nullable Integer relativePositionTag = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == LANDMARK_TAG_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          landmarkTag = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == RELATIVE_POSITION_TAG_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          relativePositionTag = castingValue.value(Integer.class);
        }
      }
    }
    return new ServiceAreaClusterLandmarkInfoStruct(
      landmarkTag,
      relativePositionTag
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ServiceAreaClusterLandmarkInfoStruct {\n");
    output.append("\tlandmarkTag: ");
    output.append(landmarkTag);
    output.append("\n");
    output.append("\trelativePositionTag: ");
    output.append(relativePositionTag);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ServiceAreaClusterAreaInfoStruct {
  public @Nullable ChipStructs.ServiceAreaClusterLocationDescriptorStruct locationInfo;
  public @Nullable ChipStructs.ServiceAreaClusterLandmarkInfoStruct landmarkInfo;
  private static final long LOCATION_INFO_ID = 0L;
  private static final long LANDMARK_INFO_ID = 1L;

  public ServiceAreaClusterAreaInfoStruct(
    @Nullable ChipStructs.ServiceAreaClusterLocationDescriptorStruct locationInfo,
    @Nullable ChipStructs.ServiceAreaClusterLandmarkInfoStruct landmarkInfo
  ) {
    this.locationInfo = locationInfo;
    this.landmarkInfo = landmarkInfo;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(LOCATION_INFO_ID, locationInfo != null ? locationInfo.encodeTlv() : new NullType()));
    values.add(new StructElement(LANDMARK_INFO_ID, landmarkInfo != null ? landmarkInfo.encodeTlv() : new NullType()));

    return new StructType(values);
  }

  public static ServiceAreaClusterAreaInfoStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    @Nullable ChipStructs.ServiceAreaClusterLocationDescriptorStruct locationInfo = null;
    @Nullable ChipStructs.ServiceAreaClusterLandmarkInfoStruct landmarkInfo = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == LOCATION_INFO_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          locationInfo = ChipStructs.ServiceAreaClusterLocationDescriptorStruct.decodeTlv(castingValue);
        }
      } else if (element.contextTagNum() == LANDMARK_INFO_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          landmarkInfo = ChipStructs.ServiceAreaClusterLandmarkInfoStruct.decodeTlv(castingValue);
        }
      }
    }
    return new ServiceAreaClusterAreaInfoStruct(
      locationInfo,
      landmarkInfo
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ServiceAreaClusterAreaInfoStruct {\n");
    output.append("\tlocationInfo: ");
    output.append(locationInfo);
    output.append("\n");
    output.append("\tlandmarkInfo: ");
    output.append(landmarkInfo);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ServiceAreaClusterAreaStruct {
  public Long areaID;
  public @Nullable Long mapID;
  public ChipStructs.ServiceAreaClusterAreaInfoStruct areaInfo;
  private static final long AREA_ID_ID = 0L;
  private static final long MAP_ID_ID = 1L;
  private static final long AREA_INFO_ID = 2L;

  public ServiceAreaClusterAreaStruct(
    Long areaID,
    @Nullable Long mapID,
    ChipStructs.ServiceAreaClusterAreaInfoStruct areaInfo
  ) {
    this.areaID = areaID;
    this.mapID = mapID;
    this.areaInfo = areaInfo;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(AREA_ID_ID, new UIntType(areaID)));
    values.add(new StructElement(MAP_ID_ID, mapID != null ? new UIntType(mapID) : new NullType()));
    values.add(new StructElement(AREA_INFO_ID, areaInfo.encodeTlv()));

    return new StructType(values);
  }

  public static ServiceAreaClusterAreaStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long areaID = null;
    @Nullable Long mapID = null;
    ChipStructs.ServiceAreaClusterAreaInfoStruct areaInfo = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == AREA_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          areaID = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == MAP_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          mapID = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == AREA_INFO_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          areaInfo = ChipStructs.ServiceAreaClusterAreaInfoStruct.decodeTlv(castingValue);
        }
      }
    }
    return new ServiceAreaClusterAreaStruct(
      areaID,
      mapID,
      areaInfo
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ServiceAreaClusterAreaStruct {\n");
    output.append("\tareaID: ");
    output.append(areaID);
    output.append("\n");
    output.append("\tmapID: ");
    output.append(mapID);
    output.append("\n");
    output.append("\tareaInfo: ");
    output.append(areaInfo);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ServiceAreaClusterMapStruct {
  public Long mapID;
  public String name;
  private static final long MAP_ID_ID = 0L;
  private static final long NAME_ID = 1L;

  public ServiceAreaClusterMapStruct(
    Long mapID,
    String name
  ) {
    this.mapID = mapID;
    this.name = name;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(MAP_ID_ID, new UIntType(mapID)));
    values.add(new StructElement(NAME_ID, new StringType(name)));

    return new StructType(values);
  }

  public static ServiceAreaClusterMapStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long mapID = null;
    String name = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == MAP_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          mapID = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == NAME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          name = castingValue.value(String.class);
        }
      }
    }
    return new ServiceAreaClusterMapStruct(
      mapID,
      name
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ServiceAreaClusterMapStruct {\n");
    output.append("\tmapID: ");
    output.append(mapID);
    output.append("\n");
    output.append("\tname: ");
    output.append(name);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ServiceAreaClusterProgressStruct {
  public Long areaID;
  public Integer status;
  public @Nullable Optional<Long> totalOperationalTime;
  public @Nullable Optional<Long> estimatedTime;
  private static final long AREA_ID_ID = 0L;
  private static final long STATUS_ID = 1L;
  private static final long TOTAL_OPERATIONAL_TIME_ID = 2L;
  private static final long ESTIMATED_TIME_ID = 3L;

  public ServiceAreaClusterProgressStruct(
    Long areaID,
    Integer status,
    @Nullable Optional<Long> totalOperationalTime,
    @Nullable Optional<Long> estimatedTime
  ) {
    this.areaID = areaID;
    this.status = status;
    this.totalOperationalTime = totalOperationalTime;
    this.estimatedTime = estimatedTime;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(AREA_ID_ID, new UIntType(areaID)));
    values.add(new StructElement(STATUS_ID, new UIntType(status)));
    values.add(new StructElement(TOTAL_OPERATIONAL_TIME_ID, totalOperationalTime != null ? totalOperationalTime.<BaseTLVType>map((nonOptionaltotalOperationalTime) -> new UIntType(nonOptionaltotalOperationalTime)).orElse(new EmptyType()) : new NullType()));
    values.add(new StructElement(ESTIMATED_TIME_ID, estimatedTime != null ? estimatedTime.<BaseTLVType>map((nonOptionalestimatedTime) -> new UIntType(nonOptionalestimatedTime)).orElse(new EmptyType()) : new NullType()));

    return new StructType(values);
  }

  public static ServiceAreaClusterProgressStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long areaID = null;
    Integer status = null;
    @Nullable Optional<Long> totalOperationalTime = null;
    @Nullable Optional<Long> estimatedTime = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == AREA_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          areaID = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == STATUS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          status = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == TOTAL_OPERATIONAL_TIME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          totalOperationalTime = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == ESTIMATED_TIME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          estimatedTime = Optional.of(castingValue.value(Long.class));
        }
      }
    }
    return new ServiceAreaClusterProgressStruct(
      areaID,
      status,
      totalOperationalTime,
      estimatedTime
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ServiceAreaClusterProgressStruct {\n");
    output.append("\tareaID: ");
    output.append(areaID);
    output.append("\n");
    output.append("\tstatus: ");
    output.append(status);
    output.append("\n");
    output.append("\ttotalOperationalTime: ");
    output.append(totalOperationalTime);
    output.append("\n");
    output.append("\testimatedTime: ");
    output.append(estimatedTime);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ServiceAreaClusterLocationDescriptorStruct {
  public String locationName;
  public @Nullable Integer floorNumber;
  public @Nullable Integer areaType;
  private static final long LOCATION_NAME_ID = 0L;
  private static final long FLOOR_NUMBER_ID = 1L;
  private static final long AREA_TYPE_ID = 2L;

  public ServiceAreaClusterLocationDescriptorStruct(
    String locationName,
    @Nullable Integer floorNumber,
    @Nullable Integer areaType
  ) {
    this.locationName = locationName;
    this.floorNumber = floorNumber;
    this.areaType = areaType;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(LOCATION_NAME_ID, new StringType(locationName)));
    values.add(new StructElement(FLOOR_NUMBER_ID, floorNumber != null ? new IntType(floorNumber) : new NullType()));
    values.add(new StructElement(AREA_TYPE_ID, areaType != null ? new UIntType(areaType) : new NullType()));

    return new StructType(values);
  }

  public static ServiceAreaClusterLocationDescriptorStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    String locationName = null;
    @Nullable Integer floorNumber = null;
    @Nullable Integer areaType = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == LOCATION_NAME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          locationName = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == FLOOR_NUMBER_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          floorNumber = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == AREA_TYPE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          areaType = castingValue.value(Integer.class);
        }
      }
    }
    return new ServiceAreaClusterLocationDescriptorStruct(
      locationName,
      floorNumber,
      areaType
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ServiceAreaClusterLocationDescriptorStruct {\n");
    output.append("\tlocationName: ");
    output.append(locationName);
    output.append("\n");
    output.append("\tfloorNumber: ");
    output.append(floorNumber);
    output.append("\n");
    output.append("\tareaType: ");
    output.append(areaType);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ThermostatClusterScheduleTransitionStruct {
  public Integer dayOfWeek;
  public Integer transitionTime;
  public Optional<byte[]> presetHandle;
  public Optional<Integer> systemMode;
  public Optional<Integer> coolingSetpoint;
  public Optional<Integer> heatingSetpoint;
  private static final long DAY_OF_WEEK_ID = 0L;
  private static final long TRANSITION_TIME_ID = 1L;
  private static final long PRESET_HANDLE_ID = 2L;
  private static final long SYSTEM_MODE_ID = 3L;
  private static final long COOLING_SETPOINT_ID = 4L;
  private static final long HEATING_SETPOINT_ID = 5L;

  public ThermostatClusterScheduleTransitionStruct(
    Integer dayOfWeek,
    Integer transitionTime,
    Optional<byte[]> presetHandle,
    Optional<Integer> systemMode,
    Optional<Integer> coolingSetpoint,
    Optional<Integer> heatingSetpoint
  ) {
    this.dayOfWeek = dayOfWeek;
    this.transitionTime = transitionTime;
    this.presetHandle = presetHandle;
    this.systemMode = systemMode;
    this.coolingSetpoint = coolingSetpoint;
    this.heatingSetpoint = heatingSetpoint;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(DAY_OF_WEEK_ID, new UIntType(dayOfWeek)));
    values.add(new StructElement(TRANSITION_TIME_ID, new UIntType(transitionTime)));
    values.add(new StructElement(PRESET_HANDLE_ID, presetHandle.<BaseTLVType>map((nonOptionalpresetHandle) -> new ByteArrayType(nonOptionalpresetHandle)).orElse(new EmptyType())));
    values.add(new StructElement(SYSTEM_MODE_ID, systemMode.<BaseTLVType>map((nonOptionalsystemMode) -> new UIntType(nonOptionalsystemMode)).orElse(new EmptyType())));
    values.add(new StructElement(COOLING_SETPOINT_ID, coolingSetpoint.<BaseTLVType>map((nonOptionalcoolingSetpoint) -> new IntType(nonOptionalcoolingSetpoint)).orElse(new EmptyType())));
    values.add(new StructElement(HEATING_SETPOINT_ID, heatingSetpoint.<BaseTLVType>map((nonOptionalheatingSetpoint) -> new IntType(nonOptionalheatingSetpoint)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static ThermostatClusterScheduleTransitionStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer dayOfWeek = null;
    Integer transitionTime = null;
    Optional<byte[]> presetHandle = Optional.empty();
    Optional<Integer> systemMode = Optional.empty();
    Optional<Integer> coolingSetpoint = Optional.empty();
    Optional<Integer> heatingSetpoint = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == DAY_OF_WEEK_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          dayOfWeek = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == TRANSITION_TIME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          transitionTime = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == PRESET_HANDLE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.ByteArray) {
          ByteArrayType castingValue = element.value(ByteArrayType.class);
          presetHandle = Optional.of(castingValue.value(byte[].class));
        }
      } else if (element.contextTagNum() == SYSTEM_MODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          systemMode = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == COOLING_SETPOINT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          coolingSetpoint = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == HEATING_SETPOINT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          heatingSetpoint = Optional.of(castingValue.value(Integer.class));
        }
      }
    }
    return new ThermostatClusterScheduleTransitionStruct(
      dayOfWeek,
      transitionTime,
      presetHandle,
      systemMode,
      coolingSetpoint,
      heatingSetpoint
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ThermostatClusterScheduleTransitionStruct {\n");
    output.append("\tdayOfWeek: ");
    output.append(dayOfWeek);
    output.append("\n");
    output.append("\ttransitionTime: ");
    output.append(transitionTime);
    output.append("\n");
    output.append("\tpresetHandle: ");
    output.append(presetHandle.isPresent() ? Arrays.toString(presetHandle.get()) : "");
    output.append("\n");
    output.append("\tsystemMode: ");
    output.append(systemMode);
    output.append("\n");
    output.append("\tcoolingSetpoint: ");
    output.append(coolingSetpoint);
    output.append("\n");
    output.append("\theatingSetpoint: ");
    output.append(heatingSetpoint);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ThermostatClusterScheduleStruct {
  public @Nullable byte[] scheduleHandle;
  public Integer systemMode;
  public Optional<String> name;
  public Optional<byte[]> presetHandle;
  public ArrayList<ChipStructs.ThermostatClusterScheduleTransitionStruct> transitions;
  public @Nullable Boolean builtIn;
  private static final long SCHEDULE_HANDLE_ID = 0L;
  private static final long SYSTEM_MODE_ID = 1L;
  private static final long NAME_ID = 2L;
  private static final long PRESET_HANDLE_ID = 3L;
  private static final long TRANSITIONS_ID = 4L;
  private static final long BUILT_IN_ID = 5L;

  public ThermostatClusterScheduleStruct(
    @Nullable byte[] scheduleHandle,
    Integer systemMode,
    Optional<String> name,
    Optional<byte[]> presetHandle,
    ArrayList<ChipStructs.ThermostatClusterScheduleTransitionStruct> transitions,
    @Nullable Boolean builtIn
  ) {
    this.scheduleHandle = scheduleHandle;
    this.systemMode = systemMode;
    this.name = name;
    this.presetHandle = presetHandle;
    this.transitions = transitions;
    this.builtIn = builtIn;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(SCHEDULE_HANDLE_ID, scheduleHandle != null ? new ByteArrayType(scheduleHandle) : new NullType()));
    values.add(new StructElement(SYSTEM_MODE_ID, new UIntType(systemMode)));
    values.add(new StructElement(NAME_ID, name.<BaseTLVType>map((nonOptionalname) -> new StringType(nonOptionalname)).orElse(new EmptyType())));
    values.add(new StructElement(PRESET_HANDLE_ID, presetHandle.<BaseTLVType>map((nonOptionalpresetHandle) -> new ByteArrayType(nonOptionalpresetHandle)).orElse(new EmptyType())));
    values.add(new StructElement(TRANSITIONS_ID, ArrayType.generateArrayType(transitions, (elementtransitions) -> elementtransitions.encodeTlv())));
    values.add(new StructElement(BUILT_IN_ID, builtIn != null ? new BooleanType(builtIn) : new NullType()));

    return new StructType(values);
  }

  public static ThermostatClusterScheduleStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    @Nullable byte[] scheduleHandle = null;
    Integer systemMode = null;
    Optional<String> name = Optional.empty();
    Optional<byte[]> presetHandle = Optional.empty();
    ArrayList<ChipStructs.ThermostatClusterScheduleTransitionStruct> transitions = null;
    @Nullable Boolean builtIn = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == SCHEDULE_HANDLE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.ByteArray) {
          ByteArrayType castingValue = element.value(ByteArrayType.class);
          scheduleHandle = castingValue.value(byte[].class);
        }
      } else if (element.contextTagNum() == SYSTEM_MODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          systemMode = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == NAME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          name = Optional.of(castingValue.value(String.class));
        }
      } else if (element.contextTagNum() == PRESET_HANDLE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.ByteArray) {
          ByteArrayType castingValue = element.value(ByteArrayType.class);
          presetHandle = Optional.of(castingValue.value(byte[].class));
        }
      } else if (element.contextTagNum() == TRANSITIONS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          transitions = castingValue.map((elementcastingValue) -> ChipStructs.ThermostatClusterScheduleTransitionStruct.decodeTlv(elementcastingValue));
        }
      } else if (element.contextTagNum() == BUILT_IN_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          builtIn = castingValue.value(Boolean.class);
        }
      }
    }
    return new ThermostatClusterScheduleStruct(
      scheduleHandle,
      systemMode,
      name,
      presetHandle,
      transitions,
      builtIn
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ThermostatClusterScheduleStruct {\n");
    output.append("\tscheduleHandle: ");
    output.append(Arrays.toString(scheduleHandle));
    output.append("\n");
    output.append("\tsystemMode: ");
    output.append(systemMode);
    output.append("\n");
    output.append("\tname: ");
    output.append(name);
    output.append("\n");
    output.append("\tpresetHandle: ");
    output.append(presetHandle.isPresent() ? Arrays.toString(presetHandle.get()) : "");
    output.append("\n");
    output.append("\ttransitions: ");
    output.append(transitions);
    output.append("\n");
    output.append("\tbuiltIn: ");
    output.append(builtIn);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ThermostatClusterPresetStruct {
  public @Nullable byte[] presetHandle;
  public Integer presetScenario;
  public @Nullable Optional<String> name;
  public Optional<Integer> coolingSetpoint;
  public Optional<Integer> heatingSetpoint;
  public @Nullable Boolean builtIn;
  private static final long PRESET_HANDLE_ID = 0L;
  private static final long PRESET_SCENARIO_ID = 1L;
  private static final long NAME_ID = 2L;
  private static final long COOLING_SETPOINT_ID = 3L;
  private static final long HEATING_SETPOINT_ID = 4L;
  private static final long BUILT_IN_ID = 5L;

  public ThermostatClusterPresetStruct(
    @Nullable byte[] presetHandle,
    Integer presetScenario,
    @Nullable Optional<String> name,
    Optional<Integer> coolingSetpoint,
    Optional<Integer> heatingSetpoint,
    @Nullable Boolean builtIn
  ) {
    this.presetHandle = presetHandle;
    this.presetScenario = presetScenario;
    this.name = name;
    this.coolingSetpoint = coolingSetpoint;
    this.heatingSetpoint = heatingSetpoint;
    this.builtIn = builtIn;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(PRESET_HANDLE_ID, presetHandle != null ? new ByteArrayType(presetHandle) : new NullType()));
    values.add(new StructElement(PRESET_SCENARIO_ID, new UIntType(presetScenario)));
    values.add(new StructElement(NAME_ID, name != null ? name.<BaseTLVType>map((nonOptionalname) -> new StringType(nonOptionalname)).orElse(new EmptyType()) : new NullType()));
    values.add(new StructElement(COOLING_SETPOINT_ID, coolingSetpoint.<BaseTLVType>map((nonOptionalcoolingSetpoint) -> new IntType(nonOptionalcoolingSetpoint)).orElse(new EmptyType())));
    values.add(new StructElement(HEATING_SETPOINT_ID, heatingSetpoint.<BaseTLVType>map((nonOptionalheatingSetpoint) -> new IntType(nonOptionalheatingSetpoint)).orElse(new EmptyType())));
    values.add(new StructElement(BUILT_IN_ID, builtIn != null ? new BooleanType(builtIn) : new NullType()));

    return new StructType(values);
  }

  public static ThermostatClusterPresetStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    @Nullable byte[] presetHandle = null;
    Integer presetScenario = null;
    @Nullable Optional<String> name = null;
    Optional<Integer> coolingSetpoint = Optional.empty();
    Optional<Integer> heatingSetpoint = Optional.empty();
    @Nullable Boolean builtIn = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == PRESET_HANDLE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.ByteArray) {
          ByteArrayType castingValue = element.value(ByteArrayType.class);
          presetHandle = castingValue.value(byte[].class);
        }
      } else if (element.contextTagNum() == PRESET_SCENARIO_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          presetScenario = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == NAME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          name = Optional.of(castingValue.value(String.class));
        }
      } else if (element.contextTagNum() == COOLING_SETPOINT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          coolingSetpoint = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == HEATING_SETPOINT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          heatingSetpoint = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == BUILT_IN_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          builtIn = castingValue.value(Boolean.class);
        }
      }
    }
    return new ThermostatClusterPresetStruct(
      presetHandle,
      presetScenario,
      name,
      coolingSetpoint,
      heatingSetpoint,
      builtIn
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ThermostatClusterPresetStruct {\n");
    output.append("\tpresetHandle: ");
    output.append(Arrays.toString(presetHandle));
    output.append("\n");
    output.append("\tpresetScenario: ");
    output.append(presetScenario);
    output.append("\n");
    output.append("\tname: ");
    output.append(name);
    output.append("\n");
    output.append("\tcoolingSetpoint: ");
    output.append(coolingSetpoint);
    output.append("\n");
    output.append("\theatingSetpoint: ");
    output.append(heatingSetpoint);
    output.append("\n");
    output.append("\tbuiltIn: ");
    output.append(builtIn);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ThermostatClusterPresetTypeStruct {
  public Integer presetScenario;
  public Integer numberOfPresets;
  public Integer presetTypeFeatures;
  private static final long PRESET_SCENARIO_ID = 0L;
  private static final long NUMBER_OF_PRESETS_ID = 1L;
  private static final long PRESET_TYPE_FEATURES_ID = 2L;

  public ThermostatClusterPresetTypeStruct(
    Integer presetScenario,
    Integer numberOfPresets,
    Integer presetTypeFeatures
  ) {
    this.presetScenario = presetScenario;
    this.numberOfPresets = numberOfPresets;
    this.presetTypeFeatures = presetTypeFeatures;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(PRESET_SCENARIO_ID, new UIntType(presetScenario)));
    values.add(new StructElement(NUMBER_OF_PRESETS_ID, new UIntType(numberOfPresets)));
    values.add(new StructElement(PRESET_TYPE_FEATURES_ID, new UIntType(presetTypeFeatures)));

    return new StructType(values);
  }

  public static ThermostatClusterPresetTypeStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer presetScenario = null;
    Integer numberOfPresets = null;
    Integer presetTypeFeatures = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == PRESET_SCENARIO_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          presetScenario = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == NUMBER_OF_PRESETS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          numberOfPresets = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == PRESET_TYPE_FEATURES_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          presetTypeFeatures = castingValue.value(Integer.class);
        }
      }
    }
    return new ThermostatClusterPresetTypeStruct(
      presetScenario,
      numberOfPresets,
      presetTypeFeatures
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ThermostatClusterPresetTypeStruct {\n");
    output.append("\tpresetScenario: ");
    output.append(presetScenario);
    output.append("\n");
    output.append("\tnumberOfPresets: ");
    output.append(numberOfPresets);
    output.append("\n");
    output.append("\tpresetTypeFeatures: ");
    output.append(presetTypeFeatures);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ThermostatClusterScheduleTypeStruct {
  public Integer systemMode;
  public Integer numberOfSchedules;
  public Integer scheduleTypeFeatures;
  private static final long SYSTEM_MODE_ID = 0L;
  private static final long NUMBER_OF_SCHEDULES_ID = 1L;
  private static final long SCHEDULE_TYPE_FEATURES_ID = 2L;

  public ThermostatClusterScheduleTypeStruct(
    Integer systemMode,
    Integer numberOfSchedules,
    Integer scheduleTypeFeatures
  ) {
    this.systemMode = systemMode;
    this.numberOfSchedules = numberOfSchedules;
    this.scheduleTypeFeatures = scheduleTypeFeatures;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(SYSTEM_MODE_ID, new UIntType(systemMode)));
    values.add(new StructElement(NUMBER_OF_SCHEDULES_ID, new UIntType(numberOfSchedules)));
    values.add(new StructElement(SCHEDULE_TYPE_FEATURES_ID, new UIntType(scheduleTypeFeatures)));

    return new StructType(values);
  }

  public static ThermostatClusterScheduleTypeStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer systemMode = null;
    Integer numberOfSchedules = null;
    Integer scheduleTypeFeatures = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == SYSTEM_MODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          systemMode = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == NUMBER_OF_SCHEDULES_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          numberOfSchedules = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == SCHEDULE_TYPE_FEATURES_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          scheduleTypeFeatures = castingValue.value(Integer.class);
        }
      }
    }
    return new ThermostatClusterScheduleTypeStruct(
      systemMode,
      numberOfSchedules,
      scheduleTypeFeatures
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ThermostatClusterScheduleTypeStruct {\n");
    output.append("\tsystemMode: ");
    output.append(systemMode);
    output.append("\n");
    output.append("\tnumberOfSchedules: ");
    output.append(numberOfSchedules);
    output.append("\n");
    output.append("\tscheduleTypeFeatures: ");
    output.append(scheduleTypeFeatures);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ThermostatClusterWeeklyScheduleTransitionStruct {
  public Integer transitionTime;
  public @Nullable Integer heatSetpoint;
  public @Nullable Integer coolSetpoint;
  private static final long TRANSITION_TIME_ID = 0L;
  private static final long HEAT_SETPOINT_ID = 1L;
  private static final long COOL_SETPOINT_ID = 2L;

  public ThermostatClusterWeeklyScheduleTransitionStruct(
    Integer transitionTime,
    @Nullable Integer heatSetpoint,
    @Nullable Integer coolSetpoint
  ) {
    this.transitionTime = transitionTime;
    this.heatSetpoint = heatSetpoint;
    this.coolSetpoint = coolSetpoint;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(TRANSITION_TIME_ID, new UIntType(transitionTime)));
    values.add(new StructElement(HEAT_SETPOINT_ID, heatSetpoint != null ? new IntType(heatSetpoint) : new NullType()));
    values.add(new StructElement(COOL_SETPOINT_ID, coolSetpoint != null ? new IntType(coolSetpoint) : new NullType()));

    return new StructType(values);
  }

  public static ThermostatClusterWeeklyScheduleTransitionStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer transitionTime = null;
    @Nullable Integer heatSetpoint = null;
    @Nullable Integer coolSetpoint = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == TRANSITION_TIME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          transitionTime = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == HEAT_SETPOINT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          heatSetpoint = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == COOL_SETPOINT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          coolSetpoint = castingValue.value(Integer.class);
        }
      }
    }
    return new ThermostatClusterWeeklyScheduleTransitionStruct(
      transitionTime,
      heatSetpoint,
      coolSetpoint
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ThermostatClusterWeeklyScheduleTransitionStruct {\n");
    output.append("\ttransitionTime: ");
    output.append(transitionTime);
    output.append("\n");
    output.append("\theatSetpoint: ");
    output.append(heatSetpoint);
    output.append("\n");
    output.append("\tcoolSetpoint: ");
    output.append(coolSetpoint);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ThermostatClusterAtomicAttributeStatusStruct {
  public Long attributeID;
  public Integer statusCode;
  private static final long ATTRIBUTE_ID_ID = 0L;
  private static final long STATUS_CODE_ID = 1L;

  public ThermostatClusterAtomicAttributeStatusStruct(
    Long attributeID,
    Integer statusCode
  ) {
    this.attributeID = attributeID;
    this.statusCode = statusCode;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ATTRIBUTE_ID_ID, new UIntType(attributeID)));
    values.add(new StructElement(STATUS_CODE_ID, new UIntType(statusCode)));

    return new StructType(values);
  }

  public static ThermostatClusterAtomicAttributeStatusStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long attributeID = null;
    Integer statusCode = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ATTRIBUTE_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          attributeID = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == STATUS_CODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          statusCode = castingValue.value(Integer.class);
        }
      }
    }
    return new ThermostatClusterAtomicAttributeStatusStruct(
      attributeID,
      statusCode
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ThermostatClusterAtomicAttributeStatusStruct {\n");
    output.append("\tattributeID: ");
    output.append(attributeID);
    output.append("\n");
    output.append("\tstatusCode: ");
    output.append(statusCode);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class OccupancySensingClusterHoldTimeLimitsStruct {
  public Integer holdTimeMin;
  public Integer holdTimeMax;
  public Integer holdTimeDefault;
  private static final long HOLD_TIME_MIN_ID = 0L;
  private static final long HOLD_TIME_MAX_ID = 1L;
  private static final long HOLD_TIME_DEFAULT_ID = 2L;

  public OccupancySensingClusterHoldTimeLimitsStruct(
    Integer holdTimeMin,
    Integer holdTimeMax,
    Integer holdTimeDefault
  ) {
    this.holdTimeMin = holdTimeMin;
    this.holdTimeMax = holdTimeMax;
    this.holdTimeDefault = holdTimeDefault;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(HOLD_TIME_MIN_ID, new UIntType(holdTimeMin)));
    values.add(new StructElement(HOLD_TIME_MAX_ID, new UIntType(holdTimeMax)));
    values.add(new StructElement(HOLD_TIME_DEFAULT_ID, new UIntType(holdTimeDefault)));

    return new StructType(values);
  }

  public static OccupancySensingClusterHoldTimeLimitsStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer holdTimeMin = null;
    Integer holdTimeMax = null;
    Integer holdTimeDefault = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == HOLD_TIME_MIN_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          holdTimeMin = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == HOLD_TIME_MAX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          holdTimeMax = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == HOLD_TIME_DEFAULT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          holdTimeDefault = castingValue.value(Integer.class);
        }
      }
    }
    return new OccupancySensingClusterHoldTimeLimitsStruct(
      holdTimeMin,
      holdTimeMax,
      holdTimeDefault
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("OccupancySensingClusterHoldTimeLimitsStruct {\n");
    output.append("\tholdTimeMin: ");
    output.append(holdTimeMin);
    output.append("\n");
    output.append("\tholdTimeMax: ");
    output.append(holdTimeMax);
    output.append("\n");
    output.append("\tholdTimeDefault: ");
    output.append(holdTimeDefault);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ThreadNetworkDirectoryClusterThreadNetworkStruct {
  public byte[] extendedPanID;
  public String networkName;
  public Integer channel;
  public Long activeTimestamp;
  private static final long EXTENDED_PAN_ID_ID = 0L;
  private static final long NETWORK_NAME_ID = 1L;
  private static final long CHANNEL_ID = 2L;
  private static final long ACTIVE_TIMESTAMP_ID = 3L;

  public ThreadNetworkDirectoryClusterThreadNetworkStruct(
    byte[] extendedPanID,
    String networkName,
    Integer channel,
    Long activeTimestamp
  ) {
    this.extendedPanID = extendedPanID;
    this.networkName = networkName;
    this.channel = channel;
    this.activeTimestamp = activeTimestamp;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(EXTENDED_PAN_ID_ID, new ByteArrayType(extendedPanID)));
    values.add(new StructElement(NETWORK_NAME_ID, new StringType(networkName)));
    values.add(new StructElement(CHANNEL_ID, new UIntType(channel)));
    values.add(new StructElement(ACTIVE_TIMESTAMP_ID, new UIntType(activeTimestamp)));

    return new StructType(values);
  }

  public static ThreadNetworkDirectoryClusterThreadNetworkStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    byte[] extendedPanID = null;
    String networkName = null;
    Integer channel = null;
    Long activeTimestamp = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == EXTENDED_PAN_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.ByteArray) {
          ByteArrayType castingValue = element.value(ByteArrayType.class);
          extendedPanID = castingValue.value(byte[].class);
        }
      } else if (element.contextTagNum() == NETWORK_NAME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          networkName = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == CHANNEL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          channel = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == ACTIVE_TIMESTAMP_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          activeTimestamp = castingValue.value(Long.class);
        }
      }
    }
    return new ThreadNetworkDirectoryClusterThreadNetworkStruct(
      extendedPanID,
      networkName,
      channel,
      activeTimestamp
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ThreadNetworkDirectoryClusterThreadNetworkStruct {\n");
    output.append("\textendedPanID: ");
    output.append(Arrays.toString(extendedPanID));
    output.append("\n");
    output.append("\tnetworkName: ");
    output.append(networkName);
    output.append("\n");
    output.append("\tchannel: ");
    output.append(channel);
    output.append("\n");
    output.append("\tactiveTimestamp: ");
    output.append(activeTimestamp);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ChannelClusterProgramCastStruct {
  public String name;
  public String role;
  private static final long NAME_ID = 0L;
  private static final long ROLE_ID = 1L;

  public ChannelClusterProgramCastStruct(
    String name,
    String role
  ) {
    this.name = name;
    this.role = role;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(NAME_ID, new StringType(name)));
    values.add(new StructElement(ROLE_ID, new StringType(role)));

    return new StructType(values);
  }

  public static ChannelClusterProgramCastStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    String name = null;
    String role = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == NAME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          name = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == ROLE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          role = castingValue.value(String.class);
        }
      }
    }
    return new ChannelClusterProgramCastStruct(
      name,
      role
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ChannelClusterProgramCastStruct {\n");
    output.append("\tname: ");
    output.append(name);
    output.append("\n");
    output.append("\trole: ");
    output.append(role);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ChannelClusterProgramCategoryStruct {
  public String category;
  public Optional<String> subCategory;
  private static final long CATEGORY_ID = 0L;
  private static final long SUB_CATEGORY_ID = 1L;

  public ChannelClusterProgramCategoryStruct(
    String category,
    Optional<String> subCategory
  ) {
    this.category = category;
    this.subCategory = subCategory;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(CATEGORY_ID, new StringType(category)));
    values.add(new StructElement(SUB_CATEGORY_ID, subCategory.<BaseTLVType>map((nonOptionalsubCategory) -> new StringType(nonOptionalsubCategory)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static ChannelClusterProgramCategoryStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    String category = null;
    Optional<String> subCategory = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == CATEGORY_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          category = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == SUB_CATEGORY_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          subCategory = Optional.of(castingValue.value(String.class));
        }
      }
    }
    return new ChannelClusterProgramCategoryStruct(
      category,
      subCategory
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ChannelClusterProgramCategoryStruct {\n");
    output.append("\tcategory: ");
    output.append(category);
    output.append("\n");
    output.append("\tsubCategory: ");
    output.append(subCategory);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ChannelClusterSeriesInfoStruct {
  public String season;
  public String episode;
  private static final long SEASON_ID = 0L;
  private static final long EPISODE_ID = 1L;

  public ChannelClusterSeriesInfoStruct(
    String season,
    String episode
  ) {
    this.season = season;
    this.episode = episode;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(SEASON_ID, new StringType(season)));
    values.add(new StructElement(EPISODE_ID, new StringType(episode)));

    return new StructType(values);
  }

  public static ChannelClusterSeriesInfoStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    String season = null;
    String episode = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == SEASON_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          season = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == EPISODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          episode = castingValue.value(String.class);
        }
      }
    }
    return new ChannelClusterSeriesInfoStruct(
      season,
      episode
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ChannelClusterSeriesInfoStruct {\n");
    output.append("\tseason: ");
    output.append(season);
    output.append("\n");
    output.append("\tepisode: ");
    output.append(episode);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ChannelClusterChannelInfoStruct {
  public Integer majorNumber;
  public Integer minorNumber;
  public Optional<String> name;
  public Optional<String> callSign;
  public Optional<String> affiliateCallSign;
  public Optional<String> identifier;
  public Optional<Integer> type;
  private static final long MAJOR_NUMBER_ID = 0L;
  private static final long MINOR_NUMBER_ID = 1L;
  private static final long NAME_ID = 2L;
  private static final long CALL_SIGN_ID = 3L;
  private static final long AFFILIATE_CALL_SIGN_ID = 4L;
  private static final long IDENTIFIER_ID = 5L;
  private static final long TYPE_ID = 6L;

  public ChannelClusterChannelInfoStruct(
    Integer majorNumber,
    Integer minorNumber,
    Optional<String> name,
    Optional<String> callSign,
    Optional<String> affiliateCallSign,
    Optional<String> identifier,
    Optional<Integer> type
  ) {
    this.majorNumber = majorNumber;
    this.minorNumber = minorNumber;
    this.name = name;
    this.callSign = callSign;
    this.affiliateCallSign = affiliateCallSign;
    this.identifier = identifier;
    this.type = type;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(MAJOR_NUMBER_ID, new UIntType(majorNumber)));
    values.add(new StructElement(MINOR_NUMBER_ID, new UIntType(minorNumber)));
    values.add(new StructElement(NAME_ID, name.<BaseTLVType>map((nonOptionalname) -> new StringType(nonOptionalname)).orElse(new EmptyType())));
    values.add(new StructElement(CALL_SIGN_ID, callSign.<BaseTLVType>map((nonOptionalcallSign) -> new StringType(nonOptionalcallSign)).orElse(new EmptyType())));
    values.add(new StructElement(AFFILIATE_CALL_SIGN_ID, affiliateCallSign.<BaseTLVType>map((nonOptionalaffiliateCallSign) -> new StringType(nonOptionalaffiliateCallSign)).orElse(new EmptyType())));
    values.add(new StructElement(IDENTIFIER_ID, identifier.<BaseTLVType>map((nonOptionalidentifier) -> new StringType(nonOptionalidentifier)).orElse(new EmptyType())));
    values.add(new StructElement(TYPE_ID, type.<BaseTLVType>map((nonOptionaltype) -> new UIntType(nonOptionaltype)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static ChannelClusterChannelInfoStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer majorNumber = null;
    Integer minorNumber = null;
    Optional<String> name = Optional.empty();
    Optional<String> callSign = Optional.empty();
    Optional<String> affiliateCallSign = Optional.empty();
    Optional<String> identifier = Optional.empty();
    Optional<Integer> type = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == MAJOR_NUMBER_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          majorNumber = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == MINOR_NUMBER_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          minorNumber = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == NAME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          name = Optional.of(castingValue.value(String.class));
        }
      } else if (element.contextTagNum() == CALL_SIGN_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          callSign = Optional.of(castingValue.value(String.class));
        }
      } else if (element.contextTagNum() == AFFILIATE_CALL_SIGN_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          affiliateCallSign = Optional.of(castingValue.value(String.class));
        }
      } else if (element.contextTagNum() == IDENTIFIER_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          identifier = Optional.of(castingValue.value(String.class));
        }
      } else if (element.contextTagNum() == TYPE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          type = Optional.of(castingValue.value(Integer.class));
        }
      }
    }
    return new ChannelClusterChannelInfoStruct(
      majorNumber,
      minorNumber,
      name,
      callSign,
      affiliateCallSign,
      identifier,
      type
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ChannelClusterChannelInfoStruct {\n");
    output.append("\tmajorNumber: ");
    output.append(majorNumber);
    output.append("\n");
    output.append("\tminorNumber: ");
    output.append(minorNumber);
    output.append("\n");
    output.append("\tname: ");
    output.append(name);
    output.append("\n");
    output.append("\tcallSign: ");
    output.append(callSign);
    output.append("\n");
    output.append("\taffiliateCallSign: ");
    output.append(affiliateCallSign);
    output.append("\n");
    output.append("\tidentifier: ");
    output.append(identifier);
    output.append("\n");
    output.append("\ttype: ");
    output.append(type);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ChannelClusterProgramStruct {
  public String identifier;
  public ChipStructs.ChannelClusterChannelInfoStruct channel;
  public Long startTime;
  public Long endTime;
  public String title;
  public Optional<String> subtitle;
  public Optional<String> description;
  public Optional<ArrayList<String>> audioLanguages;
  public Optional<ArrayList<String>> ratings;
  public Optional<String> thumbnailUrl;
  public Optional<String> posterArtUrl;
  public Optional<String> dvbiUrl;
  public Optional<String> releaseDate;
  public Optional<String> parentalGuidanceText;
  public Optional<Long> recordingFlag;
  public @Nullable Optional<ChipStructs.ChannelClusterSeriesInfoStruct> seriesInfo;
  public Optional<ArrayList<ChipStructs.ChannelClusterProgramCategoryStruct>> categoryList;
  public Optional<ArrayList<ChipStructs.ChannelClusterProgramCastStruct>> castList;
  public Optional<ArrayList<ChipStructs.ChannelClusterProgramCastStruct>> externalIDList;
  private static final long IDENTIFIER_ID = 0L;
  private static final long CHANNEL_ID = 1L;
  private static final long START_TIME_ID = 2L;
  private static final long END_TIME_ID = 3L;
  private static final long TITLE_ID = 4L;
  private static final long SUBTITLE_ID = 5L;
  private static final long DESCRIPTION_ID = 6L;
  private static final long AUDIO_LANGUAGES_ID = 7L;
  private static final long RATINGS_ID = 8L;
  private static final long THUMBNAIL_URL_ID = 9L;
  private static final long POSTER_ART_URL_ID = 10L;
  private static final long DVBI_URL_ID = 11L;
  private static final long RELEASE_DATE_ID = 12L;
  private static final long PARENTAL_GUIDANCE_TEXT_ID = 13L;
  private static final long RECORDING_FLAG_ID = 14L;
  private static final long SERIES_INFO_ID = 15L;
  private static final long CATEGORY_LIST_ID = 16L;
  private static final long CAST_LIST_ID = 17L;
  private static final long EXTERNAL_ID_LIST_ID = 18L;

  public ChannelClusterProgramStruct(
    String identifier,
    ChipStructs.ChannelClusterChannelInfoStruct channel,
    Long startTime,
    Long endTime,
    String title,
    Optional<String> subtitle,
    Optional<String> description,
    Optional<ArrayList<String>> audioLanguages,
    Optional<ArrayList<String>> ratings,
    Optional<String> thumbnailUrl,
    Optional<String> posterArtUrl,
    Optional<String> dvbiUrl,
    Optional<String> releaseDate,
    Optional<String> parentalGuidanceText,
    Optional<Long> recordingFlag,
    @Nullable Optional<ChipStructs.ChannelClusterSeriesInfoStruct> seriesInfo,
    Optional<ArrayList<ChipStructs.ChannelClusterProgramCategoryStruct>> categoryList,
    Optional<ArrayList<ChipStructs.ChannelClusterProgramCastStruct>> castList,
    Optional<ArrayList<ChipStructs.ChannelClusterProgramCastStruct>> externalIDList
  ) {
    this.identifier = identifier;
    this.channel = channel;
    this.startTime = startTime;
    this.endTime = endTime;
    this.title = title;
    this.subtitle = subtitle;
    this.description = description;
    this.audioLanguages = audioLanguages;
    this.ratings = ratings;
    this.thumbnailUrl = thumbnailUrl;
    this.posterArtUrl = posterArtUrl;
    this.dvbiUrl = dvbiUrl;
    this.releaseDate = releaseDate;
    this.parentalGuidanceText = parentalGuidanceText;
    this.recordingFlag = recordingFlag;
    this.seriesInfo = seriesInfo;
    this.categoryList = categoryList;
    this.castList = castList;
    this.externalIDList = externalIDList;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(IDENTIFIER_ID, new StringType(identifier)));
    values.add(new StructElement(CHANNEL_ID, channel.encodeTlv()));
    values.add(new StructElement(START_TIME_ID, new UIntType(startTime)));
    values.add(new StructElement(END_TIME_ID, new UIntType(endTime)));
    values.add(new StructElement(TITLE_ID, new StringType(title)));
    values.add(new StructElement(SUBTITLE_ID, subtitle.<BaseTLVType>map((nonOptionalsubtitle) -> new StringType(nonOptionalsubtitle)).orElse(new EmptyType())));
    values.add(new StructElement(DESCRIPTION_ID, description.<BaseTLVType>map((nonOptionaldescription) -> new StringType(nonOptionaldescription)).orElse(new EmptyType())));
    values.add(new StructElement(AUDIO_LANGUAGES_ID, audioLanguages.<BaseTLVType>map((nonOptionalaudioLanguages) -> ArrayType.generateArrayType(nonOptionalaudioLanguages, (elementnonOptionalaudioLanguages) -> new StringType(elementnonOptionalaudioLanguages))).orElse(new EmptyType())));
    values.add(new StructElement(RATINGS_ID, ratings.<BaseTLVType>map((nonOptionalratings) -> ArrayType.generateArrayType(nonOptionalratings, (elementnonOptionalratings) -> new StringType(elementnonOptionalratings))).orElse(new EmptyType())));
    values.add(new StructElement(THUMBNAIL_URL_ID, thumbnailUrl.<BaseTLVType>map((nonOptionalthumbnailUrl) -> new StringType(nonOptionalthumbnailUrl)).orElse(new EmptyType())));
    values.add(new StructElement(POSTER_ART_URL_ID, posterArtUrl.<BaseTLVType>map((nonOptionalposterArtUrl) -> new StringType(nonOptionalposterArtUrl)).orElse(new EmptyType())));
    values.add(new StructElement(DVBI_URL_ID, dvbiUrl.<BaseTLVType>map((nonOptionaldvbiUrl) -> new StringType(nonOptionaldvbiUrl)).orElse(new EmptyType())));
    values.add(new StructElement(RELEASE_DATE_ID, releaseDate.<BaseTLVType>map((nonOptionalreleaseDate) -> new StringType(nonOptionalreleaseDate)).orElse(new EmptyType())));
    values.add(new StructElement(PARENTAL_GUIDANCE_TEXT_ID, parentalGuidanceText.<BaseTLVType>map((nonOptionalparentalGuidanceText) -> new StringType(nonOptionalparentalGuidanceText)).orElse(new EmptyType())));
    values.add(new StructElement(RECORDING_FLAG_ID, recordingFlag.<BaseTLVType>map((nonOptionalrecordingFlag) -> new UIntType(nonOptionalrecordingFlag)).orElse(new EmptyType())));
    values.add(new StructElement(SERIES_INFO_ID, seriesInfo != null ? seriesInfo.<BaseTLVType>map((nonOptionalseriesInfo) -> nonOptionalseriesInfo.encodeTlv()).orElse(new EmptyType()) : new NullType()));
    values.add(new StructElement(CATEGORY_LIST_ID, categoryList.<BaseTLVType>map((nonOptionalcategoryList) -> ArrayType.generateArrayType(nonOptionalcategoryList, (elementnonOptionalcategoryList) -> elementnonOptionalcategoryList.encodeTlv())).orElse(new EmptyType())));
    values.add(new StructElement(CAST_LIST_ID, castList.<BaseTLVType>map((nonOptionalcastList) -> ArrayType.generateArrayType(nonOptionalcastList, (elementnonOptionalcastList) -> elementnonOptionalcastList.encodeTlv())).orElse(new EmptyType())));
    values.add(new StructElement(EXTERNAL_ID_LIST_ID, externalIDList.<BaseTLVType>map((nonOptionalexternalIDList) -> ArrayType.generateArrayType(nonOptionalexternalIDList, (elementnonOptionalexternalIDList) -> elementnonOptionalexternalIDList.encodeTlv())).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static ChannelClusterProgramStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    String identifier = null;
    ChipStructs.ChannelClusterChannelInfoStruct channel = null;
    Long startTime = null;
    Long endTime = null;
    String title = null;
    Optional<String> subtitle = Optional.empty();
    Optional<String> description = Optional.empty();
    Optional<ArrayList<String>> audioLanguages = Optional.empty();
    Optional<ArrayList<String>> ratings = Optional.empty();
    Optional<String> thumbnailUrl = Optional.empty();
    Optional<String> posterArtUrl = Optional.empty();
    Optional<String> dvbiUrl = Optional.empty();
    Optional<String> releaseDate = Optional.empty();
    Optional<String> parentalGuidanceText = Optional.empty();
    Optional<Long> recordingFlag = Optional.empty();
    @Nullable Optional<ChipStructs.ChannelClusterSeriesInfoStruct> seriesInfo = null;
    Optional<ArrayList<ChipStructs.ChannelClusterProgramCategoryStruct>> categoryList = Optional.empty();
    Optional<ArrayList<ChipStructs.ChannelClusterProgramCastStruct>> castList = Optional.empty();
    Optional<ArrayList<ChipStructs.ChannelClusterProgramCastStruct>> externalIDList = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == IDENTIFIER_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          identifier = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == CHANNEL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          channel = ChipStructs.ChannelClusterChannelInfoStruct.decodeTlv(castingValue);
        }
      } else if (element.contextTagNum() == START_TIME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          startTime = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == END_TIME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          endTime = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == TITLE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          title = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == SUBTITLE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          subtitle = Optional.of(castingValue.value(String.class));
        }
      } else if (element.contextTagNum() == DESCRIPTION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          description = Optional.of(castingValue.value(String.class));
        }
      } else if (element.contextTagNum() == AUDIO_LANGUAGES_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          audioLanguages = Optional.of(castingValue.map((elementcastingValue) -> elementcastingValue.value(String.class)));
        }
      } else if (element.contextTagNum() == RATINGS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          ratings = Optional.of(castingValue.map((elementcastingValue) -> elementcastingValue.value(String.class)));
        }
      } else if (element.contextTagNum() == THUMBNAIL_URL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          thumbnailUrl = Optional.of(castingValue.value(String.class));
        }
      } else if (element.contextTagNum() == POSTER_ART_URL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          posterArtUrl = Optional.of(castingValue.value(String.class));
        }
      } else if (element.contextTagNum() == DVBI_URL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          dvbiUrl = Optional.of(castingValue.value(String.class));
        }
      } else if (element.contextTagNum() == RELEASE_DATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          releaseDate = Optional.of(castingValue.value(String.class));
        }
      } else if (element.contextTagNum() == PARENTAL_GUIDANCE_TEXT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          parentalGuidanceText = Optional.of(castingValue.value(String.class));
        }
      } else if (element.contextTagNum() == RECORDING_FLAG_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          recordingFlag = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == SERIES_INFO_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          seriesInfo = Optional.of(ChipStructs.ChannelClusterSeriesInfoStruct.decodeTlv(castingValue));
        }
      } else if (element.contextTagNum() == CATEGORY_LIST_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          categoryList = Optional.of(castingValue.map((elementcastingValue) -> ChipStructs.ChannelClusterProgramCategoryStruct.decodeTlv(elementcastingValue)));
        }
      } else if (element.contextTagNum() == CAST_LIST_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          castList = Optional.of(castingValue.map((elementcastingValue) -> ChipStructs.ChannelClusterProgramCastStruct.decodeTlv(elementcastingValue)));
        }
      } else if (element.contextTagNum() == EXTERNAL_ID_LIST_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          externalIDList = Optional.of(castingValue.map((elementcastingValue) -> ChipStructs.ChannelClusterProgramCastStruct.decodeTlv(elementcastingValue)));
        }
      }
    }
    return new ChannelClusterProgramStruct(
      identifier,
      channel,
      startTime,
      endTime,
      title,
      subtitle,
      description,
      audioLanguages,
      ratings,
      thumbnailUrl,
      posterArtUrl,
      dvbiUrl,
      releaseDate,
      parentalGuidanceText,
      recordingFlag,
      seriesInfo,
      categoryList,
      castList,
      externalIDList
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ChannelClusterProgramStruct {\n");
    output.append("\tidentifier: ");
    output.append(identifier);
    output.append("\n");
    output.append("\tchannel: ");
    output.append(channel);
    output.append("\n");
    output.append("\tstartTime: ");
    output.append(startTime);
    output.append("\n");
    output.append("\tendTime: ");
    output.append(endTime);
    output.append("\n");
    output.append("\ttitle: ");
    output.append(title);
    output.append("\n");
    output.append("\tsubtitle: ");
    output.append(subtitle);
    output.append("\n");
    output.append("\tdescription: ");
    output.append(description);
    output.append("\n");
    output.append("\taudioLanguages: ");
    output.append(audioLanguages);
    output.append("\n");
    output.append("\tratings: ");
    output.append(ratings);
    output.append("\n");
    output.append("\tthumbnailUrl: ");
    output.append(thumbnailUrl);
    output.append("\n");
    output.append("\tposterArtUrl: ");
    output.append(posterArtUrl);
    output.append("\n");
    output.append("\tdvbiUrl: ");
    output.append(dvbiUrl);
    output.append("\n");
    output.append("\treleaseDate: ");
    output.append(releaseDate);
    output.append("\n");
    output.append("\tparentalGuidanceText: ");
    output.append(parentalGuidanceText);
    output.append("\n");
    output.append("\trecordingFlag: ");
    output.append(recordingFlag);
    output.append("\n");
    output.append("\tseriesInfo: ");
    output.append(seriesInfo);
    output.append("\n");
    output.append("\tcategoryList: ");
    output.append(categoryList);
    output.append("\n");
    output.append("\tcastList: ");
    output.append(castList);
    output.append("\n");
    output.append("\texternalIDList: ");
    output.append(externalIDList);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ChannelClusterPageTokenStruct {
  public Optional<Integer> limit;
  public Optional<String> after;
  public Optional<String> before;
  private static final long LIMIT_ID = 0L;
  private static final long AFTER_ID = 1L;
  private static final long BEFORE_ID = 2L;

  public ChannelClusterPageTokenStruct(
    Optional<Integer> limit,
    Optional<String> after,
    Optional<String> before
  ) {
    this.limit = limit;
    this.after = after;
    this.before = before;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(LIMIT_ID, limit.<BaseTLVType>map((nonOptionallimit) -> new UIntType(nonOptionallimit)).orElse(new EmptyType())));
    values.add(new StructElement(AFTER_ID, after.<BaseTLVType>map((nonOptionalafter) -> new StringType(nonOptionalafter)).orElse(new EmptyType())));
    values.add(new StructElement(BEFORE_ID, before.<BaseTLVType>map((nonOptionalbefore) -> new StringType(nonOptionalbefore)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static ChannelClusterPageTokenStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Optional<Integer> limit = Optional.empty();
    Optional<String> after = Optional.empty();
    Optional<String> before = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == LIMIT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          limit = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == AFTER_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          after = Optional.of(castingValue.value(String.class));
        }
      } else if (element.contextTagNum() == BEFORE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          before = Optional.of(castingValue.value(String.class));
        }
      }
    }
    return new ChannelClusterPageTokenStruct(
      limit,
      after,
      before
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ChannelClusterPageTokenStruct {\n");
    output.append("\tlimit: ");
    output.append(limit);
    output.append("\n");
    output.append("\tafter: ");
    output.append(after);
    output.append("\n");
    output.append("\tbefore: ");
    output.append(before);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ChannelClusterChannelPagingStruct {
  public @Nullable Optional<ChipStructs.ChannelClusterPageTokenStruct> previousToken;
  public @Nullable Optional<ChipStructs.ChannelClusterPageTokenStruct> nextToken;
  private static final long PREVIOUS_TOKEN_ID = 0L;
  private static final long NEXT_TOKEN_ID = 1L;

  public ChannelClusterChannelPagingStruct(
    @Nullable Optional<ChipStructs.ChannelClusterPageTokenStruct> previousToken,
    @Nullable Optional<ChipStructs.ChannelClusterPageTokenStruct> nextToken
  ) {
    this.previousToken = previousToken;
    this.nextToken = nextToken;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(PREVIOUS_TOKEN_ID, previousToken != null ? previousToken.<BaseTLVType>map((nonOptionalpreviousToken) -> nonOptionalpreviousToken.encodeTlv()).orElse(new EmptyType()) : new NullType()));
    values.add(new StructElement(NEXT_TOKEN_ID, nextToken != null ? nextToken.<BaseTLVType>map((nonOptionalnextToken) -> nonOptionalnextToken.encodeTlv()).orElse(new EmptyType()) : new NullType()));

    return new StructType(values);
  }

  public static ChannelClusterChannelPagingStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    @Nullable Optional<ChipStructs.ChannelClusterPageTokenStruct> previousToken = null;
    @Nullable Optional<ChipStructs.ChannelClusterPageTokenStruct> nextToken = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == PREVIOUS_TOKEN_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          previousToken = Optional.of(ChipStructs.ChannelClusterPageTokenStruct.decodeTlv(castingValue));
        }
      } else if (element.contextTagNum() == NEXT_TOKEN_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          nextToken = Optional.of(ChipStructs.ChannelClusterPageTokenStruct.decodeTlv(castingValue));
        }
      }
    }
    return new ChannelClusterChannelPagingStruct(
      previousToken,
      nextToken
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ChannelClusterChannelPagingStruct {\n");
    output.append("\tpreviousToken: ");
    output.append(previousToken);
    output.append("\n");
    output.append("\tnextToken: ");
    output.append(nextToken);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ChannelClusterAdditionalInfoStruct {
  public String name;
  public String value;
  private static final long NAME_ID = 0L;
  private static final long VALUE_ID = 1L;

  public ChannelClusterAdditionalInfoStruct(
    String name,
    String value
  ) {
    this.name = name;
    this.value = value;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(NAME_ID, new StringType(name)));
    values.add(new StructElement(VALUE_ID, new StringType(value)));

    return new StructType(values);
  }

  public static ChannelClusterAdditionalInfoStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    String name = null;
    String value = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == NAME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          name = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == VALUE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          value = castingValue.value(String.class);
        }
      }
    }
    return new ChannelClusterAdditionalInfoStruct(
      name,
      value
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ChannelClusterAdditionalInfoStruct {\n");
    output.append("\tname: ");
    output.append(name);
    output.append("\n");
    output.append("\tvalue: ");
    output.append(value);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ChannelClusterLineupInfoStruct {
  public String operatorName;
  public Optional<String> lineupName;
  public Optional<String> postalCode;
  public Integer lineupInfoType;
  private static final long OPERATOR_NAME_ID = 0L;
  private static final long LINEUP_NAME_ID = 1L;
  private static final long POSTAL_CODE_ID = 2L;
  private static final long LINEUP_INFO_TYPE_ID = 3L;

  public ChannelClusterLineupInfoStruct(
    String operatorName,
    Optional<String> lineupName,
    Optional<String> postalCode,
    Integer lineupInfoType
  ) {
    this.operatorName = operatorName;
    this.lineupName = lineupName;
    this.postalCode = postalCode;
    this.lineupInfoType = lineupInfoType;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(OPERATOR_NAME_ID, new StringType(operatorName)));
    values.add(new StructElement(LINEUP_NAME_ID, lineupName.<BaseTLVType>map((nonOptionallineupName) -> new StringType(nonOptionallineupName)).orElse(new EmptyType())));
    values.add(new StructElement(POSTAL_CODE_ID, postalCode.<BaseTLVType>map((nonOptionalpostalCode) -> new StringType(nonOptionalpostalCode)).orElse(new EmptyType())));
    values.add(new StructElement(LINEUP_INFO_TYPE_ID, new UIntType(lineupInfoType)));

    return new StructType(values);
  }

  public static ChannelClusterLineupInfoStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    String operatorName = null;
    Optional<String> lineupName = Optional.empty();
    Optional<String> postalCode = Optional.empty();
    Integer lineupInfoType = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == OPERATOR_NAME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          operatorName = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == LINEUP_NAME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          lineupName = Optional.of(castingValue.value(String.class));
        }
      } else if (element.contextTagNum() == POSTAL_CODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          postalCode = Optional.of(castingValue.value(String.class));
        }
      } else if (element.contextTagNum() == LINEUP_INFO_TYPE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          lineupInfoType = castingValue.value(Integer.class);
        }
      }
    }
    return new ChannelClusterLineupInfoStruct(
      operatorName,
      lineupName,
      postalCode,
      lineupInfoType
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ChannelClusterLineupInfoStruct {\n");
    output.append("\toperatorName: ");
    output.append(operatorName);
    output.append("\n");
    output.append("\tlineupName: ");
    output.append(lineupName);
    output.append("\n");
    output.append("\tpostalCode: ");
    output.append(postalCode);
    output.append("\n");
    output.append("\tlineupInfoType: ");
    output.append(lineupInfoType);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class TargetNavigatorClusterTargetInfoStruct {
  public Integer identifier;
  public String name;
  private static final long IDENTIFIER_ID = 0L;
  private static final long NAME_ID = 1L;

  public TargetNavigatorClusterTargetInfoStruct(
    Integer identifier,
    String name
  ) {
    this.identifier = identifier;
    this.name = name;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(IDENTIFIER_ID, new UIntType(identifier)));
    values.add(new StructElement(NAME_ID, new StringType(name)));

    return new StructType(values);
  }

  public static TargetNavigatorClusterTargetInfoStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer identifier = null;
    String name = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == IDENTIFIER_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          identifier = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == NAME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          name = castingValue.value(String.class);
        }
      }
    }
    return new TargetNavigatorClusterTargetInfoStruct(
      identifier,
      name
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("TargetNavigatorClusterTargetInfoStruct {\n");
    output.append("\tidentifier: ");
    output.append(identifier);
    output.append("\n");
    output.append("\tname: ");
    output.append(name);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class MediaPlaybackClusterTrackAttributesStruct {
  public String languageCode;
  public @Nullable Optional<String> displayName;
  private static final long LANGUAGE_CODE_ID = 0L;
  private static final long DISPLAY_NAME_ID = 1L;

  public MediaPlaybackClusterTrackAttributesStruct(
    String languageCode,
    @Nullable Optional<String> displayName
  ) {
    this.languageCode = languageCode;
    this.displayName = displayName;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(LANGUAGE_CODE_ID, new StringType(languageCode)));
    values.add(new StructElement(DISPLAY_NAME_ID, displayName != null ? displayName.<BaseTLVType>map((nonOptionaldisplayName) -> new StringType(nonOptionaldisplayName)).orElse(new EmptyType()) : new NullType()));

    return new StructType(values);
  }

  public static MediaPlaybackClusterTrackAttributesStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    String languageCode = null;
    @Nullable Optional<String> displayName = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == LANGUAGE_CODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          languageCode = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == DISPLAY_NAME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          displayName = Optional.of(castingValue.value(String.class));
        }
      }
    }
    return new MediaPlaybackClusterTrackAttributesStruct(
      languageCode,
      displayName
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("MediaPlaybackClusterTrackAttributesStruct {\n");
    output.append("\tlanguageCode: ");
    output.append(languageCode);
    output.append("\n");
    output.append("\tdisplayName: ");
    output.append(displayName);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class MediaPlaybackClusterTrackStruct {
  public String id;
  public @Nullable ChipStructs.MediaPlaybackClusterTrackAttributesStruct trackAttributes;
  private static final long ID_ID = 0L;
  private static final long TRACK_ATTRIBUTES_ID = 1L;

  public MediaPlaybackClusterTrackStruct(
    String id,
    @Nullable ChipStructs.MediaPlaybackClusterTrackAttributesStruct trackAttributes
  ) {
    this.id = id;
    this.trackAttributes = trackAttributes;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ID_ID, new StringType(id)));
    values.add(new StructElement(TRACK_ATTRIBUTES_ID, trackAttributes != null ? trackAttributes.encodeTlv() : new NullType()));

    return new StructType(values);
  }

  public static MediaPlaybackClusterTrackStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    String id = null;
    @Nullable ChipStructs.MediaPlaybackClusterTrackAttributesStruct trackAttributes = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          id = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == TRACK_ATTRIBUTES_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          trackAttributes = ChipStructs.MediaPlaybackClusterTrackAttributesStruct.decodeTlv(castingValue);
        }
      }
    }
    return new MediaPlaybackClusterTrackStruct(
      id,
      trackAttributes
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("MediaPlaybackClusterTrackStruct {\n");
    output.append("\tid: ");
    output.append(id);
    output.append("\n");
    output.append("\ttrackAttributes: ");
    output.append(trackAttributes);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class MediaPlaybackClusterPlaybackPositionStruct {
  public Long updatedAt;
  public @Nullable Long position;
  private static final long UPDATED_AT_ID = 0L;
  private static final long POSITION_ID = 1L;

  public MediaPlaybackClusterPlaybackPositionStruct(
    Long updatedAt,
    @Nullable Long position
  ) {
    this.updatedAt = updatedAt;
    this.position = position;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(UPDATED_AT_ID, new UIntType(updatedAt)));
    values.add(new StructElement(POSITION_ID, position != null ? new UIntType(position) : new NullType()));

    return new StructType(values);
  }

  public static MediaPlaybackClusterPlaybackPositionStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long updatedAt = null;
    @Nullable Long position = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == UPDATED_AT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          updatedAt = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == POSITION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          position = castingValue.value(Long.class);
        }
      }
    }
    return new MediaPlaybackClusterPlaybackPositionStruct(
      updatedAt,
      position
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("MediaPlaybackClusterPlaybackPositionStruct {\n");
    output.append("\tupdatedAt: ");
    output.append(updatedAt);
    output.append("\n");
    output.append("\tposition: ");
    output.append(position);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class MediaInputClusterInputInfoStruct {
  public Integer index;
  public Integer inputType;
  public String name;
  public String description;
  private static final long INDEX_ID = 0L;
  private static final long INPUT_TYPE_ID = 1L;
  private static final long NAME_ID = 2L;
  private static final long DESCRIPTION_ID = 3L;

  public MediaInputClusterInputInfoStruct(
    Integer index,
    Integer inputType,
    String name,
    String description
  ) {
    this.index = index;
    this.inputType = inputType;
    this.name = name;
    this.description = description;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(INDEX_ID, new UIntType(index)));
    values.add(new StructElement(INPUT_TYPE_ID, new UIntType(inputType)));
    values.add(new StructElement(NAME_ID, new StringType(name)));
    values.add(new StructElement(DESCRIPTION_ID, new StringType(description)));

    return new StructType(values);
  }

  public static MediaInputClusterInputInfoStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer index = null;
    Integer inputType = null;
    String name = null;
    String description = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == INDEX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          index = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == INPUT_TYPE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          inputType = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == NAME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          name = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == DESCRIPTION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          description = castingValue.value(String.class);
        }
      }
    }
    return new MediaInputClusterInputInfoStruct(
      index,
      inputType,
      name,
      description
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("MediaInputClusterInputInfoStruct {\n");
    output.append("\tindex: ");
    output.append(index);
    output.append("\n");
    output.append("\tinputType: ");
    output.append(inputType);
    output.append("\n");
    output.append("\tname: ");
    output.append(name);
    output.append("\n");
    output.append("\tdescription: ");
    output.append(description);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ContentLauncherClusterDimensionStruct {
  public Double width;
  public Double height;
  public Integer metric;
  private static final long WIDTH_ID = 0L;
  private static final long HEIGHT_ID = 1L;
  private static final long METRIC_ID = 2L;

  public ContentLauncherClusterDimensionStruct(
    Double width,
    Double height,
    Integer metric
  ) {
    this.width = width;
    this.height = height;
    this.metric = metric;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(WIDTH_ID, new DoubleType(width)));
    values.add(new StructElement(HEIGHT_ID, new DoubleType(height)));
    values.add(new StructElement(METRIC_ID, new UIntType(metric)));

    return new StructType(values);
  }

  public static ContentLauncherClusterDimensionStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Double width = null;
    Double height = null;
    Integer metric = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == WIDTH_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Double) {
          DoubleType castingValue = element.value(DoubleType.class);
          width = castingValue.value(Double.class);
        }
      } else if (element.contextTagNum() == HEIGHT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Double) {
          DoubleType castingValue = element.value(DoubleType.class);
          height = castingValue.value(Double.class);
        }
      } else if (element.contextTagNum() == METRIC_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          metric = castingValue.value(Integer.class);
        }
      }
    }
    return new ContentLauncherClusterDimensionStruct(
      width,
      height,
      metric
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ContentLauncherClusterDimensionStruct {\n");
    output.append("\twidth: ");
    output.append(width);
    output.append("\n");
    output.append("\theight: ");
    output.append(height);
    output.append("\n");
    output.append("\tmetric: ");
    output.append(metric);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ContentLauncherClusterTrackPreferenceStruct {
  public String languageCode;
  public Optional<ArrayList<Integer>> characteristics;
  public Integer audioOutputIndex;
  private static final long LANGUAGE_CODE_ID = 0L;
  private static final long CHARACTERISTICS_ID = 1L;
  private static final long AUDIO_OUTPUT_INDEX_ID = 2L;

  public ContentLauncherClusterTrackPreferenceStruct(
    String languageCode,
    Optional<ArrayList<Integer>> characteristics,
    Integer audioOutputIndex
  ) {
    this.languageCode = languageCode;
    this.characteristics = characteristics;
    this.audioOutputIndex = audioOutputIndex;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(LANGUAGE_CODE_ID, new StringType(languageCode)));
    values.add(new StructElement(CHARACTERISTICS_ID, characteristics.<BaseTLVType>map((nonOptionalcharacteristics) -> ArrayType.generateArrayType(nonOptionalcharacteristics, (elementnonOptionalcharacteristics) -> new UIntType(elementnonOptionalcharacteristics))).orElse(new EmptyType())));
    values.add(new StructElement(AUDIO_OUTPUT_INDEX_ID, new UIntType(audioOutputIndex)));

    return new StructType(values);
  }

  public static ContentLauncherClusterTrackPreferenceStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    String languageCode = null;
    Optional<ArrayList<Integer>> characteristics = Optional.empty();
    Integer audioOutputIndex = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == LANGUAGE_CODE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          languageCode = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == CHARACTERISTICS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          characteristics = Optional.of(castingValue.map((elementcastingValue) -> elementcastingValue.value(Integer.class)));
        }
      } else if (element.contextTagNum() == AUDIO_OUTPUT_INDEX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          audioOutputIndex = castingValue.value(Integer.class);
        }
      }
    }
    return new ContentLauncherClusterTrackPreferenceStruct(
      languageCode,
      characteristics,
      audioOutputIndex
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ContentLauncherClusterTrackPreferenceStruct {\n");
    output.append("\tlanguageCode: ");
    output.append(languageCode);
    output.append("\n");
    output.append("\tcharacteristics: ");
    output.append(characteristics);
    output.append("\n");
    output.append("\taudioOutputIndex: ");
    output.append(audioOutputIndex);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ContentLauncherClusterPlaybackPreferencesStruct {
  public Long playbackPosition;
  public ChipStructs.ContentLauncherClusterTrackPreferenceStruct textTrack;
  public Optional<ArrayList<ChipStructs.ContentLauncherClusterTrackPreferenceStruct>> audioTracks;
  private static final long PLAYBACK_POSITION_ID = 0L;
  private static final long TEXT_TRACK_ID = 1L;
  private static final long AUDIO_TRACKS_ID = 2L;

  public ContentLauncherClusterPlaybackPreferencesStruct(
    Long playbackPosition,
    ChipStructs.ContentLauncherClusterTrackPreferenceStruct textTrack,
    Optional<ArrayList<ChipStructs.ContentLauncherClusterTrackPreferenceStruct>> audioTracks
  ) {
    this.playbackPosition = playbackPosition;
    this.textTrack = textTrack;
    this.audioTracks = audioTracks;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(PLAYBACK_POSITION_ID, new UIntType(playbackPosition)));
    values.add(new StructElement(TEXT_TRACK_ID, textTrack.encodeTlv()));
    values.add(new StructElement(AUDIO_TRACKS_ID, audioTracks.<BaseTLVType>map((nonOptionalaudioTracks) -> ArrayType.generateArrayType(nonOptionalaudioTracks, (elementnonOptionalaudioTracks) -> elementnonOptionalaudioTracks.encodeTlv())).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static ContentLauncherClusterPlaybackPreferencesStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long playbackPosition = null;
    ChipStructs.ContentLauncherClusterTrackPreferenceStruct textTrack = null;
    Optional<ArrayList<ChipStructs.ContentLauncherClusterTrackPreferenceStruct>> audioTracks = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == PLAYBACK_POSITION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          playbackPosition = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == TEXT_TRACK_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          textTrack = ChipStructs.ContentLauncherClusterTrackPreferenceStruct.decodeTlv(castingValue);
        }
      } else if (element.contextTagNum() == AUDIO_TRACKS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          audioTracks = Optional.of(castingValue.map((elementcastingValue) -> ChipStructs.ContentLauncherClusterTrackPreferenceStruct.decodeTlv(elementcastingValue)));
        }
      }
    }
    return new ContentLauncherClusterPlaybackPreferencesStruct(
      playbackPosition,
      textTrack,
      audioTracks
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ContentLauncherClusterPlaybackPreferencesStruct {\n");
    output.append("\tplaybackPosition: ");
    output.append(playbackPosition);
    output.append("\n");
    output.append("\ttextTrack: ");
    output.append(textTrack);
    output.append("\n");
    output.append("\taudioTracks: ");
    output.append(audioTracks);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ContentLauncherClusterAdditionalInfoStruct {
  public String name;
  public String value;
  private static final long NAME_ID = 0L;
  private static final long VALUE_ID = 1L;

  public ContentLauncherClusterAdditionalInfoStruct(
    String name,
    String value
  ) {
    this.name = name;
    this.value = value;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(NAME_ID, new StringType(name)));
    values.add(new StructElement(VALUE_ID, new StringType(value)));

    return new StructType(values);
  }

  public static ContentLauncherClusterAdditionalInfoStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    String name = null;
    String value = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == NAME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          name = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == VALUE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          value = castingValue.value(String.class);
        }
      }
    }
    return new ContentLauncherClusterAdditionalInfoStruct(
      name,
      value
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ContentLauncherClusterAdditionalInfoStruct {\n");
    output.append("\tname: ");
    output.append(name);
    output.append("\n");
    output.append("\tvalue: ");
    output.append(value);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ContentLauncherClusterParameterStruct {
  public Integer type;
  public String value;
  public Optional<ArrayList<ChipStructs.ContentLauncherClusterAdditionalInfoStruct>> externalIDList;
  private static final long TYPE_ID = 0L;
  private static final long VALUE_ID = 1L;
  private static final long EXTERNAL_ID_LIST_ID = 2L;

  public ContentLauncherClusterParameterStruct(
    Integer type,
    String value,
    Optional<ArrayList<ChipStructs.ContentLauncherClusterAdditionalInfoStruct>> externalIDList
  ) {
    this.type = type;
    this.value = value;
    this.externalIDList = externalIDList;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(TYPE_ID, new UIntType(type)));
    values.add(new StructElement(VALUE_ID, new StringType(value)));
    values.add(new StructElement(EXTERNAL_ID_LIST_ID, externalIDList.<BaseTLVType>map((nonOptionalexternalIDList) -> ArrayType.generateArrayType(nonOptionalexternalIDList, (elementnonOptionalexternalIDList) -> elementnonOptionalexternalIDList.encodeTlv())).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static ContentLauncherClusterParameterStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer type = null;
    String value = null;
    Optional<ArrayList<ChipStructs.ContentLauncherClusterAdditionalInfoStruct>> externalIDList = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == TYPE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          type = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == VALUE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          value = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == EXTERNAL_ID_LIST_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          externalIDList = Optional.of(castingValue.map((elementcastingValue) -> ChipStructs.ContentLauncherClusterAdditionalInfoStruct.decodeTlv(elementcastingValue)));
        }
      }
    }
    return new ContentLauncherClusterParameterStruct(
      type,
      value,
      externalIDList
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ContentLauncherClusterParameterStruct {\n");
    output.append("\ttype: ");
    output.append(type);
    output.append("\n");
    output.append("\tvalue: ");
    output.append(value);
    output.append("\n");
    output.append("\texternalIDList: ");
    output.append(externalIDList);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ContentLauncherClusterContentSearchStruct {
  public ArrayList<ChipStructs.ContentLauncherClusterParameterStruct> parameterList;
  private static final long PARAMETER_LIST_ID = 0L;

  public ContentLauncherClusterContentSearchStruct(
    ArrayList<ChipStructs.ContentLauncherClusterParameterStruct> parameterList
  ) {
    this.parameterList = parameterList;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(PARAMETER_LIST_ID, ArrayType.generateArrayType(parameterList, (elementparameterList) -> elementparameterList.encodeTlv())));

    return new StructType(values);
  }

  public static ContentLauncherClusterContentSearchStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    ArrayList<ChipStructs.ContentLauncherClusterParameterStruct> parameterList = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == PARAMETER_LIST_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          parameterList = castingValue.map((elementcastingValue) -> ChipStructs.ContentLauncherClusterParameterStruct.decodeTlv(elementcastingValue));
        }
      }
    }
    return new ContentLauncherClusterContentSearchStruct(
      parameterList
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ContentLauncherClusterContentSearchStruct {\n");
    output.append("\tparameterList: ");
    output.append(parameterList);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ContentLauncherClusterStyleInformationStruct {
  public Optional<String> imageURL;
  public Optional<String> color;
  public Optional<ChipStructs.ContentLauncherClusterDimensionStruct> size;
  private static final long IMAGE_URL_ID = 0L;
  private static final long COLOR_ID = 1L;
  private static final long SIZE_ID = 2L;

  public ContentLauncherClusterStyleInformationStruct(
    Optional<String> imageURL,
    Optional<String> color,
    Optional<ChipStructs.ContentLauncherClusterDimensionStruct> size
  ) {
    this.imageURL = imageURL;
    this.color = color;
    this.size = size;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(IMAGE_URL_ID, imageURL.<BaseTLVType>map((nonOptionalimageURL) -> new StringType(nonOptionalimageURL)).orElse(new EmptyType())));
    values.add(new StructElement(COLOR_ID, color.<BaseTLVType>map((nonOptionalcolor) -> new StringType(nonOptionalcolor)).orElse(new EmptyType())));
    values.add(new StructElement(SIZE_ID, size.<BaseTLVType>map((nonOptionalsize) -> nonOptionalsize.encodeTlv()).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static ContentLauncherClusterStyleInformationStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Optional<String> imageURL = Optional.empty();
    Optional<String> color = Optional.empty();
    Optional<ChipStructs.ContentLauncherClusterDimensionStruct> size = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == IMAGE_URL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          imageURL = Optional.of(castingValue.value(String.class));
        }
      } else if (element.contextTagNum() == COLOR_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          color = Optional.of(castingValue.value(String.class));
        }
      } else if (element.contextTagNum() == SIZE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          size = Optional.of(ChipStructs.ContentLauncherClusterDimensionStruct.decodeTlv(castingValue));
        }
      }
    }
    return new ContentLauncherClusterStyleInformationStruct(
      imageURL,
      color,
      size
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ContentLauncherClusterStyleInformationStruct {\n");
    output.append("\timageURL: ");
    output.append(imageURL);
    output.append("\n");
    output.append("\tcolor: ");
    output.append(color);
    output.append("\n");
    output.append("\tsize: ");
    output.append(size);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ContentLauncherClusterBrandingInformationStruct {
  public String providerName;
  public Optional<ChipStructs.ContentLauncherClusterStyleInformationStruct> background;
  public Optional<ChipStructs.ContentLauncherClusterStyleInformationStruct> logo;
  public Optional<ChipStructs.ContentLauncherClusterStyleInformationStruct> progressBar;
  public Optional<ChipStructs.ContentLauncherClusterStyleInformationStruct> splash;
  public Optional<ChipStructs.ContentLauncherClusterStyleInformationStruct> waterMark;
  private static final long PROVIDER_NAME_ID = 0L;
  private static final long BACKGROUND_ID = 1L;
  private static final long LOGO_ID = 2L;
  private static final long PROGRESS_BAR_ID = 3L;
  private static final long SPLASH_ID = 4L;
  private static final long WATER_MARK_ID = 5L;

  public ContentLauncherClusterBrandingInformationStruct(
    String providerName,
    Optional<ChipStructs.ContentLauncherClusterStyleInformationStruct> background,
    Optional<ChipStructs.ContentLauncherClusterStyleInformationStruct> logo,
    Optional<ChipStructs.ContentLauncherClusterStyleInformationStruct> progressBar,
    Optional<ChipStructs.ContentLauncherClusterStyleInformationStruct> splash,
    Optional<ChipStructs.ContentLauncherClusterStyleInformationStruct> waterMark
  ) {
    this.providerName = providerName;
    this.background = background;
    this.logo = logo;
    this.progressBar = progressBar;
    this.splash = splash;
    this.waterMark = waterMark;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(PROVIDER_NAME_ID, new StringType(providerName)));
    values.add(new StructElement(BACKGROUND_ID, background.<BaseTLVType>map((nonOptionalbackground) -> nonOptionalbackground.encodeTlv()).orElse(new EmptyType())));
    values.add(new StructElement(LOGO_ID, logo.<BaseTLVType>map((nonOptionallogo) -> nonOptionallogo.encodeTlv()).orElse(new EmptyType())));
    values.add(new StructElement(PROGRESS_BAR_ID, progressBar.<BaseTLVType>map((nonOptionalprogressBar) -> nonOptionalprogressBar.encodeTlv()).orElse(new EmptyType())));
    values.add(new StructElement(SPLASH_ID, splash.<BaseTLVType>map((nonOptionalsplash) -> nonOptionalsplash.encodeTlv()).orElse(new EmptyType())));
    values.add(new StructElement(WATER_MARK_ID, waterMark.<BaseTLVType>map((nonOptionalwaterMark) -> nonOptionalwaterMark.encodeTlv()).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static ContentLauncherClusterBrandingInformationStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    String providerName = null;
    Optional<ChipStructs.ContentLauncherClusterStyleInformationStruct> background = Optional.empty();
    Optional<ChipStructs.ContentLauncherClusterStyleInformationStruct> logo = Optional.empty();
    Optional<ChipStructs.ContentLauncherClusterStyleInformationStruct> progressBar = Optional.empty();
    Optional<ChipStructs.ContentLauncherClusterStyleInformationStruct> splash = Optional.empty();
    Optional<ChipStructs.ContentLauncherClusterStyleInformationStruct> waterMark = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == PROVIDER_NAME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          providerName = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == BACKGROUND_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          background = Optional.of(ChipStructs.ContentLauncherClusterStyleInformationStruct.decodeTlv(castingValue));
        }
      } else if (element.contextTagNum() == LOGO_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          logo = Optional.of(ChipStructs.ContentLauncherClusterStyleInformationStruct.decodeTlv(castingValue));
        }
      } else if (element.contextTagNum() == PROGRESS_BAR_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          progressBar = Optional.of(ChipStructs.ContentLauncherClusterStyleInformationStruct.decodeTlv(castingValue));
        }
      } else if (element.contextTagNum() == SPLASH_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          splash = Optional.of(ChipStructs.ContentLauncherClusterStyleInformationStruct.decodeTlv(castingValue));
        }
      } else if (element.contextTagNum() == WATER_MARK_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          waterMark = Optional.of(ChipStructs.ContentLauncherClusterStyleInformationStruct.decodeTlv(castingValue));
        }
      }
    }
    return new ContentLauncherClusterBrandingInformationStruct(
      providerName,
      background,
      logo,
      progressBar,
      splash,
      waterMark
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ContentLauncherClusterBrandingInformationStruct {\n");
    output.append("\tproviderName: ");
    output.append(providerName);
    output.append("\n");
    output.append("\tbackground: ");
    output.append(background);
    output.append("\n");
    output.append("\tlogo: ");
    output.append(logo);
    output.append("\n");
    output.append("\tprogressBar: ");
    output.append(progressBar);
    output.append("\n");
    output.append("\tsplash: ");
    output.append(splash);
    output.append("\n");
    output.append("\twaterMark: ");
    output.append(waterMark);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class AudioOutputClusterOutputInfoStruct {
  public Integer index;
  public Integer outputType;
  public String name;
  private static final long INDEX_ID = 0L;
  private static final long OUTPUT_TYPE_ID = 1L;
  private static final long NAME_ID = 2L;

  public AudioOutputClusterOutputInfoStruct(
    Integer index,
    Integer outputType,
    String name
  ) {
    this.index = index;
    this.outputType = outputType;
    this.name = name;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(INDEX_ID, new UIntType(index)));
    values.add(new StructElement(OUTPUT_TYPE_ID, new UIntType(outputType)));
    values.add(new StructElement(NAME_ID, new StringType(name)));

    return new StructType(values);
  }

  public static AudioOutputClusterOutputInfoStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer index = null;
    Integer outputType = null;
    String name = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == INDEX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          index = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == OUTPUT_TYPE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          outputType = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == NAME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          name = castingValue.value(String.class);
        }
      }
    }
    return new AudioOutputClusterOutputInfoStruct(
      index,
      outputType,
      name
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("AudioOutputClusterOutputInfoStruct {\n");
    output.append("\tindex: ");
    output.append(index);
    output.append("\n");
    output.append("\toutputType: ");
    output.append(outputType);
    output.append("\n");
    output.append("\tname: ");
    output.append(name);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ApplicationLauncherClusterApplicationStruct {
  public Integer catalogVendorID;
  public String applicationID;
  private static final long CATALOG_VENDOR_ID_ID = 0L;
  private static final long APPLICATION_ID_ID = 1L;

  public ApplicationLauncherClusterApplicationStruct(
    Integer catalogVendorID,
    String applicationID
  ) {
    this.catalogVendorID = catalogVendorID;
    this.applicationID = applicationID;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(CATALOG_VENDOR_ID_ID, new UIntType(catalogVendorID)));
    values.add(new StructElement(APPLICATION_ID_ID, new StringType(applicationID)));

    return new StructType(values);
  }

  public static ApplicationLauncherClusterApplicationStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer catalogVendorID = null;
    String applicationID = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == CATALOG_VENDOR_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          catalogVendorID = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == APPLICATION_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          applicationID = castingValue.value(String.class);
        }
      }
    }
    return new ApplicationLauncherClusterApplicationStruct(
      catalogVendorID,
      applicationID
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ApplicationLauncherClusterApplicationStruct {\n");
    output.append("\tcatalogVendorID: ");
    output.append(catalogVendorID);
    output.append("\n");
    output.append("\tapplicationID: ");
    output.append(applicationID);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ApplicationLauncherClusterApplicationEPStruct {
  public ChipStructs.ApplicationLauncherClusterApplicationStruct application;
  public Optional<Integer> endpoint;
  private static final long APPLICATION_ID = 0L;
  private static final long ENDPOINT_ID = 1L;

  public ApplicationLauncherClusterApplicationEPStruct(
    ChipStructs.ApplicationLauncherClusterApplicationStruct application,
    Optional<Integer> endpoint
  ) {
    this.application = application;
    this.endpoint = endpoint;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(APPLICATION_ID, application.encodeTlv()));
    values.add(new StructElement(ENDPOINT_ID, endpoint.<BaseTLVType>map((nonOptionalendpoint) -> new UIntType(nonOptionalendpoint)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static ApplicationLauncherClusterApplicationEPStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    ChipStructs.ApplicationLauncherClusterApplicationStruct application = null;
    Optional<Integer> endpoint = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == APPLICATION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          application = ChipStructs.ApplicationLauncherClusterApplicationStruct.decodeTlv(castingValue);
        }
      } else if (element.contextTagNum() == ENDPOINT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          endpoint = Optional.of(castingValue.value(Integer.class));
        }
      }
    }
    return new ApplicationLauncherClusterApplicationEPStruct(
      application,
      endpoint
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ApplicationLauncherClusterApplicationEPStruct {\n");
    output.append("\tapplication: ");
    output.append(application);
    output.append("\n");
    output.append("\tendpoint: ");
    output.append(endpoint);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ApplicationBasicClusterApplicationStruct {
  public Integer catalogVendorID;
  public String applicationID;
  private static final long CATALOG_VENDOR_ID_ID = 0L;
  private static final long APPLICATION_ID_ID = 1L;

  public ApplicationBasicClusterApplicationStruct(
    Integer catalogVendorID,
    String applicationID
  ) {
    this.catalogVendorID = catalogVendorID;
    this.applicationID = applicationID;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(CATALOG_VENDOR_ID_ID, new UIntType(catalogVendorID)));
    values.add(new StructElement(APPLICATION_ID_ID, new StringType(applicationID)));

    return new StructType(values);
  }

  public static ApplicationBasicClusterApplicationStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer catalogVendorID = null;
    String applicationID = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == CATALOG_VENDOR_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          catalogVendorID = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == APPLICATION_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          applicationID = castingValue.value(String.class);
        }
      }
    }
    return new ApplicationBasicClusterApplicationStruct(
      catalogVendorID,
      applicationID
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ApplicationBasicClusterApplicationStruct {\n");
    output.append("\tcatalogVendorID: ");
    output.append(catalogVendorID);
    output.append("\n");
    output.append("\tapplicationID: ");
    output.append(applicationID);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ContentControlClusterRatingNameStruct {
  public String ratingName;
  public Optional<String> ratingNameDesc;
  private static final long RATING_NAME_ID = 0L;
  private static final long RATING_NAME_DESC_ID = 1L;

  public ContentControlClusterRatingNameStruct(
    String ratingName,
    Optional<String> ratingNameDesc
  ) {
    this.ratingName = ratingName;
    this.ratingNameDesc = ratingNameDesc;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(RATING_NAME_ID, new StringType(ratingName)));
    values.add(new StructElement(RATING_NAME_DESC_ID, ratingNameDesc.<BaseTLVType>map((nonOptionalratingNameDesc) -> new StringType(nonOptionalratingNameDesc)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static ContentControlClusterRatingNameStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    String ratingName = null;
    Optional<String> ratingNameDesc = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == RATING_NAME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          ratingName = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == RATING_NAME_DESC_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          ratingNameDesc = Optional.of(castingValue.value(String.class));
        }
      }
    }
    return new ContentControlClusterRatingNameStruct(
      ratingName,
      ratingNameDesc
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ContentControlClusterRatingNameStruct {\n");
    output.append("\tratingName: ");
    output.append(ratingName);
    output.append("\n");
    output.append("\tratingNameDesc: ");
    output.append(ratingNameDesc);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ZoneManagementClusterTwoDCartesianVertexStruct {
  public Integer x;
  public Integer y;
  private static final long X_ID = 0L;
  private static final long Y_ID = 1L;

  public ZoneManagementClusterTwoDCartesianVertexStruct(
    Integer x,
    Integer y
  ) {
    this.x = x;
    this.y = y;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(X_ID, new UIntType(x)));
    values.add(new StructElement(Y_ID, new UIntType(y)));

    return new StructType(values);
  }

  public static ZoneManagementClusterTwoDCartesianVertexStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer x = null;
    Integer y = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == X_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          x = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == Y_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          y = castingValue.value(Integer.class);
        }
      }
    }
    return new ZoneManagementClusterTwoDCartesianVertexStruct(
      x,
      y
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ZoneManagementClusterTwoDCartesianVertexStruct {\n");
    output.append("\tx: ");
    output.append(x);
    output.append("\n");
    output.append("\ty: ");
    output.append(y);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ZoneManagementClusterTwoDCartesianZoneStruct {
  public String name;
  public Integer use;
  public ArrayList<ChipStructs.ZoneManagementClusterTwoDCartesianVertexStruct> vertices;
  public Optional<String> color;
  private static final long NAME_ID = 0L;
  private static final long USE_ID = 1L;
  private static final long VERTICES_ID = 2L;
  private static final long COLOR_ID = 3L;

  public ZoneManagementClusterTwoDCartesianZoneStruct(
    String name,
    Integer use,
    ArrayList<ChipStructs.ZoneManagementClusterTwoDCartesianVertexStruct> vertices,
    Optional<String> color
  ) {
    this.name = name;
    this.use = use;
    this.vertices = vertices;
    this.color = color;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(NAME_ID, new StringType(name)));
    values.add(new StructElement(USE_ID, new UIntType(use)));
    values.add(new StructElement(VERTICES_ID, ArrayType.generateArrayType(vertices, (elementvertices) -> elementvertices.encodeTlv())));
    values.add(new StructElement(COLOR_ID, color.<BaseTLVType>map((nonOptionalcolor) -> new StringType(nonOptionalcolor)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static ZoneManagementClusterTwoDCartesianZoneStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    String name = null;
    Integer use = null;
    ArrayList<ChipStructs.ZoneManagementClusterTwoDCartesianVertexStruct> vertices = null;
    Optional<String> color = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == NAME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          name = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == USE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          use = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == VERTICES_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          vertices = castingValue.map((elementcastingValue) -> ChipStructs.ZoneManagementClusterTwoDCartesianVertexStruct.decodeTlv(elementcastingValue));
        }
      } else if (element.contextTagNum() == COLOR_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          color = Optional.of(castingValue.value(String.class));
        }
      }
    }
    return new ZoneManagementClusterTwoDCartesianZoneStruct(
      name,
      use,
      vertices,
      color
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ZoneManagementClusterTwoDCartesianZoneStruct {\n");
    output.append("\tname: ");
    output.append(name);
    output.append("\n");
    output.append("\tuse: ");
    output.append(use);
    output.append("\n");
    output.append("\tvertices: ");
    output.append(vertices);
    output.append("\n");
    output.append("\tcolor: ");
    output.append(color);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ZoneManagementClusterZoneInformationStruct {
  public Integer zoneID;
  public Integer zoneType;
  public Integer zoneSource;
  private static final long ZONE_ID_ID = 0L;
  private static final long ZONE_TYPE_ID = 1L;
  private static final long ZONE_SOURCE_ID = 2L;

  public ZoneManagementClusterZoneInformationStruct(
    Integer zoneID,
    Integer zoneType,
    Integer zoneSource
  ) {
    this.zoneID = zoneID;
    this.zoneType = zoneType;
    this.zoneSource = zoneSource;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ZONE_ID_ID, new UIntType(zoneID)));
    values.add(new StructElement(ZONE_TYPE_ID, new UIntType(zoneType)));
    values.add(new StructElement(ZONE_SOURCE_ID, new UIntType(zoneSource)));

    return new StructType(values);
  }

  public static ZoneManagementClusterZoneInformationStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer zoneID = null;
    Integer zoneType = null;
    Integer zoneSource = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ZONE_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          zoneID = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == ZONE_TYPE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          zoneType = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == ZONE_SOURCE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          zoneSource = castingValue.value(Integer.class);
        }
      }
    }
    return new ZoneManagementClusterZoneInformationStruct(
      zoneID,
      zoneType,
      zoneSource
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ZoneManagementClusterZoneInformationStruct {\n");
    output.append("\tzoneID: ");
    output.append(zoneID);
    output.append("\n");
    output.append("\tzoneType: ");
    output.append(zoneType);
    output.append("\n");
    output.append("\tzoneSource: ");
    output.append(zoneSource);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ZoneManagementClusterZoneTriggerControlStruct {
  public Integer initialDuration;
  public Integer augmentationDuration;
  public Long maxDuration;
  public Integer blindDuration;
  public Optional<Integer> sensitivity;
  private static final long INITIAL_DURATION_ID = 0L;
  private static final long AUGMENTATION_DURATION_ID = 1L;
  private static final long MAX_DURATION_ID = 2L;
  private static final long BLIND_DURATION_ID = 3L;
  private static final long SENSITIVITY_ID = 4L;

  public ZoneManagementClusterZoneTriggerControlStruct(
    Integer initialDuration,
    Integer augmentationDuration,
    Long maxDuration,
    Integer blindDuration,
    Optional<Integer> sensitivity
  ) {
    this.initialDuration = initialDuration;
    this.augmentationDuration = augmentationDuration;
    this.maxDuration = maxDuration;
    this.blindDuration = blindDuration;
    this.sensitivity = sensitivity;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(INITIAL_DURATION_ID, new UIntType(initialDuration)));
    values.add(new StructElement(AUGMENTATION_DURATION_ID, new UIntType(augmentationDuration)));
    values.add(new StructElement(MAX_DURATION_ID, new UIntType(maxDuration)));
    values.add(new StructElement(BLIND_DURATION_ID, new UIntType(blindDuration)));
    values.add(new StructElement(SENSITIVITY_ID, sensitivity.<BaseTLVType>map((nonOptionalsensitivity) -> new UIntType(nonOptionalsensitivity)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static ZoneManagementClusterZoneTriggerControlStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer initialDuration = null;
    Integer augmentationDuration = null;
    Long maxDuration = null;
    Integer blindDuration = null;
    Optional<Integer> sensitivity = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == INITIAL_DURATION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          initialDuration = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == AUGMENTATION_DURATION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          augmentationDuration = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == MAX_DURATION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          maxDuration = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == BLIND_DURATION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          blindDuration = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == SENSITIVITY_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          sensitivity = Optional.of(castingValue.value(Integer.class));
        }
      }
    }
    return new ZoneManagementClusterZoneTriggerControlStruct(
      initialDuration,
      augmentationDuration,
      maxDuration,
      blindDuration,
      sensitivity
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ZoneManagementClusterZoneTriggerControlStruct {\n");
    output.append("\tinitialDuration: ");
    output.append(initialDuration);
    output.append("\n");
    output.append("\taugmentationDuration: ");
    output.append(augmentationDuration);
    output.append("\n");
    output.append("\tmaxDuration: ");
    output.append(maxDuration);
    output.append("\n");
    output.append("\tblindDuration: ");
    output.append(blindDuration);
    output.append("\n");
    output.append("\tsensitivity: ");
    output.append(sensitivity);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class CameraAvStreamManagementClusterVideoResolutionStruct {
  public Integer width;
  public Integer height;
  private static final long WIDTH_ID = 0L;
  private static final long HEIGHT_ID = 1L;

  public CameraAvStreamManagementClusterVideoResolutionStruct(
    Integer width,
    Integer height
  ) {
    this.width = width;
    this.height = height;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(WIDTH_ID, new UIntType(width)));
    values.add(new StructElement(HEIGHT_ID, new UIntType(height)));

    return new StructType(values);
  }

  public static CameraAvStreamManagementClusterVideoResolutionStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer width = null;
    Integer height = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == WIDTH_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          width = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == HEIGHT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          height = castingValue.value(Integer.class);
        }
      }
    }
    return new CameraAvStreamManagementClusterVideoResolutionStruct(
      width,
      height
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("CameraAvStreamManagementClusterVideoResolutionStruct {\n");
    output.append("\twidth: ");
    output.append(width);
    output.append("\n");
    output.append("\theight: ");
    output.append(height);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class CameraAvStreamManagementClusterVideoStreamStruct {
  public Integer videoStreamID;
  public Integer streamUsage;
  public Integer videoCodec;
  public Integer minFrameRate;
  public Integer maxFrameRate;
  public ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct minResolution;
  public ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct maxResolution;
  public Long minBitRate;
  public Long maxBitRate;
  public Integer minFragmentLen;
  public Integer maxFragmentLen;
  public Optional<Boolean> watermarkEnabled;
  public Optional<Boolean> OSDEnabled;
  public Integer referenceCount;
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
  private static final long WATERMARK_ENABLED_ID = 11L;
  private static final long OSD_ENABLED_ID = 12L;
  private static final long REFERENCE_COUNT_ID = 13L;

  public CameraAvStreamManagementClusterVideoStreamStruct(
    Integer videoStreamID,
    Integer streamUsage,
    Integer videoCodec,
    Integer minFrameRate,
    Integer maxFrameRate,
    ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct minResolution,
    ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct maxResolution,
    Long minBitRate,
    Long maxBitRate,
    Integer minFragmentLen,
    Integer maxFragmentLen,
    Optional<Boolean> watermarkEnabled,
    Optional<Boolean> OSDEnabled,
    Integer referenceCount
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
    this.watermarkEnabled = watermarkEnabled;
    this.OSDEnabled = OSDEnabled;
    this.referenceCount = referenceCount;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(VIDEO_STREAM_ID_ID, new UIntType(videoStreamID)));
    values.add(new StructElement(STREAM_USAGE_ID, new UIntType(streamUsage)));
    values.add(new StructElement(VIDEO_CODEC_ID, new UIntType(videoCodec)));
    values.add(new StructElement(MIN_FRAME_RATE_ID, new UIntType(minFrameRate)));
    values.add(new StructElement(MAX_FRAME_RATE_ID, new UIntType(maxFrameRate)));
    values.add(new StructElement(MIN_RESOLUTION_ID, minResolution.encodeTlv()));
    values.add(new StructElement(MAX_RESOLUTION_ID, maxResolution.encodeTlv()));
    values.add(new StructElement(MIN_BIT_RATE_ID, new UIntType(minBitRate)));
    values.add(new StructElement(MAX_BIT_RATE_ID, new UIntType(maxBitRate)));
    values.add(new StructElement(MIN_FRAGMENT_LEN_ID, new UIntType(minFragmentLen)));
    values.add(new StructElement(MAX_FRAGMENT_LEN_ID, new UIntType(maxFragmentLen)));
    values.add(new StructElement(WATERMARK_ENABLED_ID, watermarkEnabled.<BaseTLVType>map((nonOptionalwatermarkEnabled) -> new BooleanType(nonOptionalwatermarkEnabled)).orElse(new EmptyType())));
    values.add(new StructElement(OSD_ENABLED_ID, OSDEnabled.<BaseTLVType>map((nonOptionalOSDEnabled) -> new BooleanType(nonOptionalOSDEnabled)).orElse(new EmptyType())));
    values.add(new StructElement(REFERENCE_COUNT_ID, new UIntType(referenceCount)));

    return new StructType(values);
  }

  public static CameraAvStreamManagementClusterVideoStreamStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer videoStreamID = null;
    Integer streamUsage = null;
    Integer videoCodec = null;
    Integer minFrameRate = null;
    Integer maxFrameRate = null;
    ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct minResolution = null;
    ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct maxResolution = null;
    Long minBitRate = null;
    Long maxBitRate = null;
    Integer minFragmentLen = null;
    Integer maxFragmentLen = null;
    Optional<Boolean> watermarkEnabled = Optional.empty();
    Optional<Boolean> OSDEnabled = Optional.empty();
    Integer referenceCount = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == VIDEO_STREAM_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          videoStreamID = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == STREAM_USAGE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          streamUsage = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == VIDEO_CODEC_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          videoCodec = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == MIN_FRAME_RATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          minFrameRate = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == MAX_FRAME_RATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          maxFrameRate = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == MIN_RESOLUTION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          minResolution = ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct.decodeTlv(castingValue);
        }
      } else if (element.contextTagNum() == MAX_RESOLUTION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          maxResolution = ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct.decodeTlv(castingValue);
        }
      } else if (element.contextTagNum() == MIN_BIT_RATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          minBitRate = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == MAX_BIT_RATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          maxBitRate = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == MIN_FRAGMENT_LEN_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          minFragmentLen = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == MAX_FRAGMENT_LEN_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          maxFragmentLen = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == WATERMARK_ENABLED_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          watermarkEnabled = Optional.of(castingValue.value(Boolean.class));
        }
      } else if (element.contextTagNum() == OSD_ENABLED_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          OSDEnabled = Optional.of(castingValue.value(Boolean.class));
        }
      } else if (element.contextTagNum() == REFERENCE_COUNT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          referenceCount = castingValue.value(Integer.class);
        }
      }
    }
    return new CameraAvStreamManagementClusterVideoStreamStruct(
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
      maxFragmentLen,
      watermarkEnabled,
      OSDEnabled,
      referenceCount
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("CameraAvStreamManagementClusterVideoStreamStruct {\n");
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
    output.append("\twatermarkEnabled: ");
    output.append(watermarkEnabled);
    output.append("\n");
    output.append("\tOSDEnabled: ");
    output.append(OSDEnabled);
    output.append("\n");
    output.append("\treferenceCount: ");
    output.append(referenceCount);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class CameraAvStreamManagementClusterSnapshotStreamStruct {
  public Integer snapshotStreamID;
  public Integer imageCodec;
  public Integer frameRate;
  public Long bitRate;
  public ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct minResolution;
  public ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct maxResolution;
  public Integer quality;
  public Integer referenceCount;
  private static final long SNAPSHOT_STREAM_ID_ID = 0L;
  private static final long IMAGE_CODEC_ID = 1L;
  private static final long FRAME_RATE_ID = 2L;
  private static final long BIT_RATE_ID = 3L;
  private static final long MIN_RESOLUTION_ID = 4L;
  private static final long MAX_RESOLUTION_ID = 5L;
  private static final long QUALITY_ID = 6L;
  private static final long REFERENCE_COUNT_ID = 7L;

  public CameraAvStreamManagementClusterSnapshotStreamStruct(
    Integer snapshotStreamID,
    Integer imageCodec,
    Integer frameRate,
    Long bitRate,
    ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct minResolution,
    ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct maxResolution,
    Integer quality,
    Integer referenceCount
  ) {
    this.snapshotStreamID = snapshotStreamID;
    this.imageCodec = imageCodec;
    this.frameRate = frameRate;
    this.bitRate = bitRate;
    this.minResolution = minResolution;
    this.maxResolution = maxResolution;
    this.quality = quality;
    this.referenceCount = referenceCount;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(SNAPSHOT_STREAM_ID_ID, new UIntType(snapshotStreamID)));
    values.add(new StructElement(IMAGE_CODEC_ID, new UIntType(imageCodec)));
    values.add(new StructElement(FRAME_RATE_ID, new UIntType(frameRate)));
    values.add(new StructElement(BIT_RATE_ID, new UIntType(bitRate)));
    values.add(new StructElement(MIN_RESOLUTION_ID, minResolution.encodeTlv()));
    values.add(new StructElement(MAX_RESOLUTION_ID, maxResolution.encodeTlv()));
    values.add(new StructElement(QUALITY_ID, new UIntType(quality)));
    values.add(new StructElement(REFERENCE_COUNT_ID, new UIntType(referenceCount)));

    return new StructType(values);
  }

  public static CameraAvStreamManagementClusterSnapshotStreamStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer snapshotStreamID = null;
    Integer imageCodec = null;
    Integer frameRate = null;
    Long bitRate = null;
    ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct minResolution = null;
    ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct maxResolution = null;
    Integer quality = null;
    Integer referenceCount = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == SNAPSHOT_STREAM_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          snapshotStreamID = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == IMAGE_CODEC_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          imageCodec = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == FRAME_RATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          frameRate = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == BIT_RATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          bitRate = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == MIN_RESOLUTION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          minResolution = ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct.decodeTlv(castingValue);
        }
      } else if (element.contextTagNum() == MAX_RESOLUTION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          maxResolution = ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct.decodeTlv(castingValue);
        }
      } else if (element.contextTagNum() == QUALITY_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          quality = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == REFERENCE_COUNT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          referenceCount = castingValue.value(Integer.class);
        }
      }
    }
    return new CameraAvStreamManagementClusterSnapshotStreamStruct(
      snapshotStreamID,
      imageCodec,
      frameRate,
      bitRate,
      minResolution,
      maxResolution,
      quality,
      referenceCount
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("CameraAvStreamManagementClusterSnapshotStreamStruct {\n");
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
    output.append("\treferenceCount: ");
    output.append(referenceCount);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class CameraAvStreamManagementClusterSnapshotParamsStruct {
  public ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct resolution;
  public Integer maxFrameRate;
  public Integer imageCodec;
  private static final long RESOLUTION_ID = 0L;
  private static final long MAX_FRAME_RATE_ID = 1L;
  private static final long IMAGE_CODEC_ID = 2L;

  public CameraAvStreamManagementClusterSnapshotParamsStruct(
    ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct resolution,
    Integer maxFrameRate,
    Integer imageCodec
  ) {
    this.resolution = resolution;
    this.maxFrameRate = maxFrameRate;
    this.imageCodec = imageCodec;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(RESOLUTION_ID, resolution.encodeTlv()));
    values.add(new StructElement(MAX_FRAME_RATE_ID, new UIntType(maxFrameRate)));
    values.add(new StructElement(IMAGE_CODEC_ID, new UIntType(imageCodec)));

    return new StructType(values);
  }

  public static CameraAvStreamManagementClusterSnapshotParamsStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct resolution = null;
    Integer maxFrameRate = null;
    Integer imageCodec = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == RESOLUTION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          resolution = ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct.decodeTlv(castingValue);
        }
      } else if (element.contextTagNum() == MAX_FRAME_RATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          maxFrameRate = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == IMAGE_CODEC_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          imageCodec = castingValue.value(Integer.class);
        }
      }
    }
    return new CameraAvStreamManagementClusterSnapshotParamsStruct(
      resolution,
      maxFrameRate,
      imageCodec
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("CameraAvStreamManagementClusterSnapshotParamsStruct {\n");
    output.append("\tresolution: ");
    output.append(resolution);
    output.append("\n");
    output.append("\tmaxFrameRate: ");
    output.append(maxFrameRate);
    output.append("\n");
    output.append("\timageCodec: ");
    output.append(imageCodec);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class CameraAvStreamManagementClusterRateDistortionTradeOffPointsStruct {
  public Integer codec;
  public ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct resolution;
  public Long minBitRate;
  private static final long CODEC_ID = 0L;
  private static final long RESOLUTION_ID = 1L;
  private static final long MIN_BIT_RATE_ID = 2L;

  public CameraAvStreamManagementClusterRateDistortionTradeOffPointsStruct(
    Integer codec,
    ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct resolution,
    Long minBitRate
  ) {
    this.codec = codec;
    this.resolution = resolution;
    this.minBitRate = minBitRate;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(CODEC_ID, new UIntType(codec)));
    values.add(new StructElement(RESOLUTION_ID, resolution.encodeTlv()));
    values.add(new StructElement(MIN_BIT_RATE_ID, new UIntType(minBitRate)));

    return new StructType(values);
  }

  public static CameraAvStreamManagementClusterRateDistortionTradeOffPointsStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer codec = null;
    ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct resolution = null;
    Long minBitRate = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == CODEC_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          codec = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == RESOLUTION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          resolution = ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct.decodeTlv(castingValue);
        }
      } else if (element.contextTagNum() == MIN_BIT_RATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          minBitRate = castingValue.value(Long.class);
        }
      }
    }
    return new CameraAvStreamManagementClusterRateDistortionTradeOffPointsStruct(
      codec,
      resolution,
      minBitRate
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("CameraAvStreamManagementClusterRateDistortionTradeOffPointsStruct {\n");
    output.append("\tcodec: ");
    output.append(codec);
    output.append("\n");
    output.append("\tresolution: ");
    output.append(resolution);
    output.append("\n");
    output.append("\tminBitRate: ");
    output.append(minBitRate);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class CameraAvStreamManagementClusterAudioCapabilitiesStruct {
  public Integer maxNumberOfChannels;
  public ArrayList<Integer> supportedCodecs;
  public ArrayList<Long> supportedSampleRates;
  public ArrayList<Integer> supportedBitDepths;
  private static final long MAX_NUMBER_OF_CHANNELS_ID = 0L;
  private static final long SUPPORTED_CODECS_ID = 1L;
  private static final long SUPPORTED_SAMPLE_RATES_ID = 2L;
  private static final long SUPPORTED_BIT_DEPTHS_ID = 3L;

  public CameraAvStreamManagementClusterAudioCapabilitiesStruct(
    Integer maxNumberOfChannels,
    ArrayList<Integer> supportedCodecs,
    ArrayList<Long> supportedSampleRates,
    ArrayList<Integer> supportedBitDepths
  ) {
    this.maxNumberOfChannels = maxNumberOfChannels;
    this.supportedCodecs = supportedCodecs;
    this.supportedSampleRates = supportedSampleRates;
    this.supportedBitDepths = supportedBitDepths;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(MAX_NUMBER_OF_CHANNELS_ID, new UIntType(maxNumberOfChannels)));
    values.add(new StructElement(SUPPORTED_CODECS_ID, ArrayType.generateArrayType(supportedCodecs, (elementsupportedCodecs) -> new UIntType(elementsupportedCodecs))));
    values.add(new StructElement(SUPPORTED_SAMPLE_RATES_ID, ArrayType.generateArrayType(supportedSampleRates, (elementsupportedSampleRates) -> new UIntType(elementsupportedSampleRates))));
    values.add(new StructElement(SUPPORTED_BIT_DEPTHS_ID, ArrayType.generateArrayType(supportedBitDepths, (elementsupportedBitDepths) -> new UIntType(elementsupportedBitDepths))));

    return new StructType(values);
  }

  public static CameraAvStreamManagementClusterAudioCapabilitiesStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer maxNumberOfChannels = null;
    ArrayList<Integer> supportedCodecs = null;
    ArrayList<Long> supportedSampleRates = null;
    ArrayList<Integer> supportedBitDepths = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == MAX_NUMBER_OF_CHANNELS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          maxNumberOfChannels = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == SUPPORTED_CODECS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          supportedCodecs = castingValue.map((elementcastingValue) -> elementcastingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == SUPPORTED_SAMPLE_RATES_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          supportedSampleRates = castingValue.map((elementcastingValue) -> elementcastingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == SUPPORTED_BIT_DEPTHS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          supportedBitDepths = castingValue.map((elementcastingValue) -> elementcastingValue.value(Integer.class));
        }
      }
    }
    return new CameraAvStreamManagementClusterAudioCapabilitiesStruct(
      maxNumberOfChannels,
      supportedCodecs,
      supportedSampleRates,
      supportedBitDepths
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("CameraAvStreamManagementClusterAudioCapabilitiesStruct {\n");
    output.append("\tmaxNumberOfChannels: ");
    output.append(maxNumberOfChannels);
    output.append("\n");
    output.append("\tsupportedCodecs: ");
    output.append(supportedCodecs);
    output.append("\n");
    output.append("\tsupportedSampleRates: ");
    output.append(supportedSampleRates);
    output.append("\n");
    output.append("\tsupportedBitDepths: ");
    output.append(supportedBitDepths);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class CameraAvStreamManagementClusterAudioStreamStruct {
  public Integer audioStreamID;
  public Integer streamUsage;
  public Integer audioCodec;
  public Integer channelCount;
  public Long sampleRate;
  public Long bitRate;
  public Integer bitDepth;
  public Integer referenceCount;
  private static final long AUDIO_STREAM_ID_ID = 0L;
  private static final long STREAM_USAGE_ID = 1L;
  private static final long AUDIO_CODEC_ID = 2L;
  private static final long CHANNEL_COUNT_ID = 3L;
  private static final long SAMPLE_RATE_ID = 4L;
  private static final long BIT_RATE_ID = 5L;
  private static final long BIT_DEPTH_ID = 6L;
  private static final long REFERENCE_COUNT_ID = 7L;

  public CameraAvStreamManagementClusterAudioStreamStruct(
    Integer audioStreamID,
    Integer streamUsage,
    Integer audioCodec,
    Integer channelCount,
    Long sampleRate,
    Long bitRate,
    Integer bitDepth,
    Integer referenceCount
  ) {
    this.audioStreamID = audioStreamID;
    this.streamUsage = streamUsage;
    this.audioCodec = audioCodec;
    this.channelCount = channelCount;
    this.sampleRate = sampleRate;
    this.bitRate = bitRate;
    this.bitDepth = bitDepth;
    this.referenceCount = referenceCount;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(AUDIO_STREAM_ID_ID, new UIntType(audioStreamID)));
    values.add(new StructElement(STREAM_USAGE_ID, new UIntType(streamUsage)));
    values.add(new StructElement(AUDIO_CODEC_ID, new UIntType(audioCodec)));
    values.add(new StructElement(CHANNEL_COUNT_ID, new UIntType(channelCount)));
    values.add(new StructElement(SAMPLE_RATE_ID, new UIntType(sampleRate)));
    values.add(new StructElement(BIT_RATE_ID, new UIntType(bitRate)));
    values.add(new StructElement(BIT_DEPTH_ID, new UIntType(bitDepth)));
    values.add(new StructElement(REFERENCE_COUNT_ID, new UIntType(referenceCount)));

    return new StructType(values);
  }

  public static CameraAvStreamManagementClusterAudioStreamStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer audioStreamID = null;
    Integer streamUsage = null;
    Integer audioCodec = null;
    Integer channelCount = null;
    Long sampleRate = null;
    Long bitRate = null;
    Integer bitDepth = null;
    Integer referenceCount = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == AUDIO_STREAM_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          audioStreamID = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == STREAM_USAGE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          streamUsage = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == AUDIO_CODEC_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          audioCodec = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == CHANNEL_COUNT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          channelCount = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == SAMPLE_RATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          sampleRate = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == BIT_RATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          bitRate = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == BIT_DEPTH_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          bitDepth = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == REFERENCE_COUNT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          referenceCount = castingValue.value(Integer.class);
        }
      }
    }
    return new CameraAvStreamManagementClusterAudioStreamStruct(
      audioStreamID,
      streamUsage,
      audioCodec,
      channelCount,
      sampleRate,
      bitRate,
      bitDepth,
      referenceCount
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("CameraAvStreamManagementClusterAudioStreamStruct {\n");
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
    output.append("\treferenceCount: ");
    output.append(referenceCount);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class CameraAvStreamManagementClusterVideoSensorParamsStruct {
  public Integer sensorWidth;
  public Integer sensorHeight;
  public Integer maxFPS;
  public Optional<Integer> maxHDRFPS;
  private static final long SENSOR_WIDTH_ID = 0L;
  private static final long SENSOR_HEIGHT_ID = 1L;
  private static final long MAX_FPS_ID = 2L;
  private static final long MAX_HDRFPS_ID = 3L;

  public CameraAvStreamManagementClusterVideoSensorParamsStruct(
    Integer sensorWidth,
    Integer sensorHeight,
    Integer maxFPS,
    Optional<Integer> maxHDRFPS
  ) {
    this.sensorWidth = sensorWidth;
    this.sensorHeight = sensorHeight;
    this.maxFPS = maxFPS;
    this.maxHDRFPS = maxHDRFPS;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(SENSOR_WIDTH_ID, new UIntType(sensorWidth)));
    values.add(new StructElement(SENSOR_HEIGHT_ID, new UIntType(sensorHeight)));
    values.add(new StructElement(MAX_FPS_ID, new UIntType(maxFPS)));
    values.add(new StructElement(MAX_HDRFPS_ID, maxHDRFPS.<BaseTLVType>map((nonOptionalmaxHDRFPS) -> new UIntType(nonOptionalmaxHDRFPS)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static CameraAvStreamManagementClusterVideoSensorParamsStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer sensorWidth = null;
    Integer sensorHeight = null;
    Integer maxFPS = null;
    Optional<Integer> maxHDRFPS = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == SENSOR_WIDTH_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          sensorWidth = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == SENSOR_HEIGHT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          sensorHeight = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == MAX_FPS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          maxFPS = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == MAX_HDRFPS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          maxHDRFPS = Optional.of(castingValue.value(Integer.class));
        }
      }
    }
    return new CameraAvStreamManagementClusterVideoSensorParamsStruct(
      sensorWidth,
      sensorHeight,
      maxFPS,
      maxHDRFPS
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("CameraAvStreamManagementClusterVideoSensorParamsStruct {\n");
    output.append("\tsensorWidth: ");
    output.append(sensorWidth);
    output.append("\n");
    output.append("\tsensorHeight: ");
    output.append(sensorHeight);
    output.append("\n");
    output.append("\tmaxFPS: ");
    output.append(maxFPS);
    output.append("\n");
    output.append("\tmaxHDRFPS: ");
    output.append(maxHDRFPS);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class CameraAvStreamManagementClusterViewportStruct {
  public Integer x1;
  public Integer y1;
  public Integer x2;
  public Integer y2;
  private static final long X1_ID = 0L;
  private static final long Y1_ID = 1L;
  private static final long X2_ID = 2L;
  private static final long Y2_ID = 3L;

  public CameraAvStreamManagementClusterViewportStruct(
    Integer x1,
    Integer y1,
    Integer x2,
    Integer y2
  ) {
    this.x1 = x1;
    this.y1 = y1;
    this.x2 = x2;
    this.y2 = y2;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(X1_ID, new UIntType(x1)));
    values.add(new StructElement(Y1_ID, new UIntType(y1)));
    values.add(new StructElement(X2_ID, new UIntType(x2)));
    values.add(new StructElement(Y2_ID, new UIntType(y2)));

    return new StructType(values);
  }

  public static CameraAvStreamManagementClusterViewportStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer x1 = null;
    Integer y1 = null;
    Integer x2 = null;
    Integer y2 = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == X1_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          x1 = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == Y1_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          y1 = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == X2_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          x2 = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == Y2_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          y2 = castingValue.value(Integer.class);
        }
      }
    }
    return new CameraAvStreamManagementClusterViewportStruct(
      x1,
      y1,
      x2,
      y2
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("CameraAvStreamManagementClusterViewportStruct {\n");
    output.append("\tx1: ");
    output.append(x1);
    output.append("\n");
    output.append("\ty1: ");
    output.append(y1);
    output.append("\n");
    output.append("\tx2: ");
    output.append(x2);
    output.append("\n");
    output.append("\ty2: ");
    output.append(y2);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class CameraAvSettingsUserLevelManagementClusterMPTZStruct {
  public Optional<Integer> pan;
  public Optional<Integer> tilt;
  public Optional<Integer> zoom;
  private static final long PAN_ID = 0L;
  private static final long TILT_ID = 1L;
  private static final long ZOOM_ID = 2L;

  public CameraAvSettingsUserLevelManagementClusterMPTZStruct(
    Optional<Integer> pan,
    Optional<Integer> tilt,
    Optional<Integer> zoom
  ) {
    this.pan = pan;
    this.tilt = tilt;
    this.zoom = zoom;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(PAN_ID, pan.<BaseTLVType>map((nonOptionalpan) -> new IntType(nonOptionalpan)).orElse(new EmptyType())));
    values.add(new StructElement(TILT_ID, tilt.<BaseTLVType>map((nonOptionaltilt) -> new IntType(nonOptionaltilt)).orElse(new EmptyType())));
    values.add(new StructElement(ZOOM_ID, zoom.<BaseTLVType>map((nonOptionalzoom) -> new UIntType(nonOptionalzoom)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static CameraAvSettingsUserLevelManagementClusterMPTZStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Optional<Integer> pan = Optional.empty();
    Optional<Integer> tilt = Optional.empty();
    Optional<Integer> zoom = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == PAN_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          pan = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == TILT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          tilt = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == ZOOM_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          zoom = Optional.of(castingValue.value(Integer.class));
        }
      }
    }
    return new CameraAvSettingsUserLevelManagementClusterMPTZStruct(
      pan,
      tilt,
      zoom
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("CameraAvSettingsUserLevelManagementClusterMPTZStruct {\n");
    output.append("\tpan: ");
    output.append(pan);
    output.append("\n");
    output.append("\ttilt: ");
    output.append(tilt);
    output.append("\n");
    output.append("\tzoom: ");
    output.append(zoom);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class CameraAvSettingsUserLevelManagementClusterMPTZPresetStruct {
  public Integer presetID;
  public String name;
  public ChipStructs.CameraAvSettingsUserLevelManagementClusterMPTZStruct settings;
  private static final long PRESET_ID_ID = 0L;
  private static final long NAME_ID = 1L;
  private static final long SETTINGS_ID = 2L;

  public CameraAvSettingsUserLevelManagementClusterMPTZPresetStruct(
    Integer presetID,
    String name,
    ChipStructs.CameraAvSettingsUserLevelManagementClusterMPTZStruct settings
  ) {
    this.presetID = presetID;
    this.name = name;
    this.settings = settings;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(PRESET_ID_ID, new UIntType(presetID)));
    values.add(new StructElement(NAME_ID, new StringType(name)));
    values.add(new StructElement(SETTINGS_ID, settings.encodeTlv()));

    return new StructType(values);
  }

  public static CameraAvSettingsUserLevelManagementClusterMPTZPresetStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer presetID = null;
    String name = null;
    ChipStructs.CameraAvSettingsUserLevelManagementClusterMPTZStruct settings = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == PRESET_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          presetID = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == NAME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          name = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == SETTINGS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          settings = ChipStructs.CameraAvSettingsUserLevelManagementClusterMPTZStruct.decodeTlv(castingValue);
        }
      }
    }
    return new CameraAvSettingsUserLevelManagementClusterMPTZPresetStruct(
      presetID,
      name,
      settings
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("CameraAvSettingsUserLevelManagementClusterMPTZPresetStruct {\n");
    output.append("\tpresetID: ");
    output.append(presetID);
    output.append("\n");
    output.append("\tname: ");
    output.append(name);
    output.append("\n");
    output.append("\tsettings: ");
    output.append(settings);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class CameraAvSettingsUserLevelManagementClusterViewportStruct {
  public Integer x1;
  public Integer y1;
  public Integer x2;
  public Integer y2;
  private static final long X1_ID = 0L;
  private static final long Y1_ID = 1L;
  private static final long X2_ID = 2L;
  private static final long Y2_ID = 3L;

  public CameraAvSettingsUserLevelManagementClusterViewportStruct(
    Integer x1,
    Integer y1,
    Integer x2,
    Integer y2
  ) {
    this.x1 = x1;
    this.y1 = y1;
    this.x2 = x2;
    this.y2 = y2;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(X1_ID, new UIntType(x1)));
    values.add(new StructElement(Y1_ID, new UIntType(y1)));
    values.add(new StructElement(X2_ID, new UIntType(x2)));
    values.add(new StructElement(Y2_ID, new UIntType(y2)));

    return new StructType(values);
  }

  public static CameraAvSettingsUserLevelManagementClusterViewportStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer x1 = null;
    Integer y1 = null;
    Integer x2 = null;
    Integer y2 = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == X1_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          x1 = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == Y1_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          y1 = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == X2_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          x2 = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == Y2_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          y2 = castingValue.value(Integer.class);
        }
      }
    }
    return new CameraAvSettingsUserLevelManagementClusterViewportStruct(
      x1,
      y1,
      x2,
      y2
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("CameraAvSettingsUserLevelManagementClusterViewportStruct {\n");
    output.append("\tx1: ");
    output.append(x1);
    output.append("\n");
    output.append("\ty1: ");
    output.append(y1);
    output.append("\n");
    output.append("\tx2: ");
    output.append(x2);
    output.append("\n");
    output.append("\ty2: ");
    output.append(y2);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class WebRTCTransportProviderClusterICEServerStruct {
  public ArrayList<String> urls;
  public Optional<String> username;
  public Optional<String> credential;
  public Optional<Integer> caid;
  private static final long URLS_ID = 1L;
  private static final long USERNAME_ID = 2L;
  private static final long CREDENTIAL_ID = 3L;
  private static final long CAID_ID = 4L;

  public WebRTCTransportProviderClusterICEServerStruct(
    ArrayList<String> urls,
    Optional<String> username,
    Optional<String> credential,
    Optional<Integer> caid
  ) {
    this.urls = urls;
    this.username = username;
    this.credential = credential;
    this.caid = caid;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(URLS_ID, ArrayType.generateArrayType(urls, (elementurls) -> new StringType(elementurls))));
    values.add(new StructElement(USERNAME_ID, username.<BaseTLVType>map((nonOptionalusername) -> new StringType(nonOptionalusername)).orElse(new EmptyType())));
    values.add(new StructElement(CREDENTIAL_ID, credential.<BaseTLVType>map((nonOptionalcredential) -> new StringType(nonOptionalcredential)).orElse(new EmptyType())));
    values.add(new StructElement(CAID_ID, caid.<BaseTLVType>map((nonOptionalcaid) -> new UIntType(nonOptionalcaid)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static WebRTCTransportProviderClusterICEServerStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    ArrayList<String> urls = null;
    Optional<String> username = Optional.empty();
    Optional<String> credential = Optional.empty();
    Optional<Integer> caid = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == URLS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          urls = castingValue.map((elementcastingValue) -> elementcastingValue.value(String.class));
        }
      } else if (element.contextTagNum() == USERNAME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          username = Optional.of(castingValue.value(String.class));
        }
      } else if (element.contextTagNum() == CREDENTIAL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          credential = Optional.of(castingValue.value(String.class));
        }
      } else if (element.contextTagNum() == CAID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          caid = Optional.of(castingValue.value(Integer.class));
        }
      }
    }
    return new WebRTCTransportProviderClusterICEServerStruct(
      urls,
      username,
      credential,
      caid
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("WebRTCTransportProviderClusterICEServerStruct {\n");
    output.append("\turls: ");
    output.append(urls);
    output.append("\n");
    output.append("\tusername: ");
    output.append(username);
    output.append("\n");
    output.append("\tcredential: ");
    output.append(credential);
    output.append("\n");
    output.append("\tcaid: ");
    output.append(caid);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class WebRTCTransportProviderClusterWebRTCSessionStruct {
  public Integer id;
  public Long peerNodeID;
  public Integer streamUsage;
  public @Nullable Integer videoStreamID;
  public @Nullable Integer audioStreamID;
  public Integer metadataOptions;
  public Integer fabricIndex;
  private static final long ID_ID = 1L;
  private static final long PEER_NODE_ID_ID = 2L;
  private static final long STREAM_USAGE_ID = 3L;
  private static final long VIDEO_STREAM_ID_ID = 4L;
  private static final long AUDIO_STREAM_ID_ID = 5L;
  private static final long METADATA_OPTIONS_ID = 6L;
  private static final long FABRIC_INDEX_ID = 254L;

  public WebRTCTransportProviderClusterWebRTCSessionStruct(
    Integer id,
    Long peerNodeID,
    Integer streamUsage,
    @Nullable Integer videoStreamID,
    @Nullable Integer audioStreamID,
    Integer metadataOptions,
    Integer fabricIndex
  ) {
    this.id = id;
    this.peerNodeID = peerNodeID;
    this.streamUsage = streamUsage;
    this.videoStreamID = videoStreamID;
    this.audioStreamID = audioStreamID;
    this.metadataOptions = metadataOptions;
    this.fabricIndex = fabricIndex;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ID_ID, new UIntType(id)));
    values.add(new StructElement(PEER_NODE_ID_ID, new UIntType(peerNodeID)));
    values.add(new StructElement(STREAM_USAGE_ID, new UIntType(streamUsage)));
    values.add(new StructElement(VIDEO_STREAM_ID_ID, videoStreamID != null ? new UIntType(videoStreamID) : new NullType()));
    values.add(new StructElement(AUDIO_STREAM_ID_ID, audioStreamID != null ? new UIntType(audioStreamID) : new NullType()));
    values.add(new StructElement(METADATA_OPTIONS_ID, new UIntType(metadataOptions)));
    values.add(new StructElement(FABRIC_INDEX_ID, new UIntType(fabricIndex)));

    return new StructType(values);
  }

  public static WebRTCTransportProviderClusterWebRTCSessionStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer id = null;
    Long peerNodeID = null;
    Integer streamUsage = null;
    @Nullable Integer videoStreamID = null;
    @Nullable Integer audioStreamID = null;
    Integer metadataOptions = null;
    Integer fabricIndex = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          id = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == PEER_NODE_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          peerNodeID = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == STREAM_USAGE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          streamUsage = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == VIDEO_STREAM_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          videoStreamID = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == AUDIO_STREAM_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          audioStreamID = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == METADATA_OPTIONS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          metadataOptions = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == FABRIC_INDEX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          fabricIndex = castingValue.value(Integer.class);
        }
      }
    }
    return new WebRTCTransportProviderClusterWebRTCSessionStruct(
      id,
      peerNodeID,
      streamUsage,
      videoStreamID,
      audioStreamID,
      metadataOptions,
      fabricIndex
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("WebRTCTransportProviderClusterWebRTCSessionStruct {\n");
    output.append("\tid: ");
    output.append(id);
    output.append("\n");
    output.append("\tpeerNodeID: ");
    output.append(peerNodeID);
    output.append("\n");
    output.append("\tstreamUsage: ");
    output.append(streamUsage);
    output.append("\n");
    output.append("\tvideoStreamID: ");
    output.append(videoStreamID);
    output.append("\n");
    output.append("\taudioStreamID: ");
    output.append(audioStreamID);
    output.append("\n");
    output.append("\tmetadataOptions: ");
    output.append(metadataOptions);
    output.append("\n");
    output.append("\tfabricIndex: ");
    output.append(fabricIndex);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class WebRTCTransportRequestorClusterICEServerStruct {
  public ArrayList<String> urls;
  public Optional<String> username;
  public Optional<String> credential;
  public Optional<Integer> caid;
  private static final long URLS_ID = 1L;
  private static final long USERNAME_ID = 2L;
  private static final long CREDENTIAL_ID = 3L;
  private static final long CAID_ID = 4L;

  public WebRTCTransportRequestorClusterICEServerStruct(
    ArrayList<String> urls,
    Optional<String> username,
    Optional<String> credential,
    Optional<Integer> caid
  ) {
    this.urls = urls;
    this.username = username;
    this.credential = credential;
    this.caid = caid;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(URLS_ID, ArrayType.generateArrayType(urls, (elementurls) -> new StringType(elementurls))));
    values.add(new StructElement(USERNAME_ID, username.<BaseTLVType>map((nonOptionalusername) -> new StringType(nonOptionalusername)).orElse(new EmptyType())));
    values.add(new StructElement(CREDENTIAL_ID, credential.<BaseTLVType>map((nonOptionalcredential) -> new StringType(nonOptionalcredential)).orElse(new EmptyType())));
    values.add(new StructElement(CAID_ID, caid.<BaseTLVType>map((nonOptionalcaid) -> new UIntType(nonOptionalcaid)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static WebRTCTransportRequestorClusterICEServerStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    ArrayList<String> urls = null;
    Optional<String> username = Optional.empty();
    Optional<String> credential = Optional.empty();
    Optional<Integer> caid = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == URLS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          urls = castingValue.map((elementcastingValue) -> elementcastingValue.value(String.class));
        }
      } else if (element.contextTagNum() == USERNAME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          username = Optional.of(castingValue.value(String.class));
        }
      } else if (element.contextTagNum() == CREDENTIAL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          credential = Optional.of(castingValue.value(String.class));
        }
      } else if (element.contextTagNum() == CAID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          caid = Optional.of(castingValue.value(Integer.class));
        }
      }
    }
    return new WebRTCTransportRequestorClusterICEServerStruct(
      urls,
      username,
      credential,
      caid
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("WebRTCTransportRequestorClusterICEServerStruct {\n");
    output.append("\turls: ");
    output.append(urls);
    output.append("\n");
    output.append("\tusername: ");
    output.append(username);
    output.append("\n");
    output.append("\tcredential: ");
    output.append(credential);
    output.append("\n");
    output.append("\tcaid: ");
    output.append(caid);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class WebRTCTransportRequestorClusterWebRTCSessionStruct {
  public Integer id;
  public Long peerNodeID;
  public Integer streamUsage;
  public @Nullable Integer videoStreamID;
  public @Nullable Integer audioStreamID;
  public Integer metadataOptions;
  public Integer fabricIndex;
  private static final long ID_ID = 1L;
  private static final long PEER_NODE_ID_ID = 2L;
  private static final long STREAM_USAGE_ID = 3L;
  private static final long VIDEO_STREAM_ID_ID = 4L;
  private static final long AUDIO_STREAM_ID_ID = 5L;
  private static final long METADATA_OPTIONS_ID = 6L;
  private static final long FABRIC_INDEX_ID = 254L;

  public WebRTCTransportRequestorClusterWebRTCSessionStruct(
    Integer id,
    Long peerNodeID,
    Integer streamUsage,
    @Nullable Integer videoStreamID,
    @Nullable Integer audioStreamID,
    Integer metadataOptions,
    Integer fabricIndex
  ) {
    this.id = id;
    this.peerNodeID = peerNodeID;
    this.streamUsage = streamUsage;
    this.videoStreamID = videoStreamID;
    this.audioStreamID = audioStreamID;
    this.metadataOptions = metadataOptions;
    this.fabricIndex = fabricIndex;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ID_ID, new UIntType(id)));
    values.add(new StructElement(PEER_NODE_ID_ID, new UIntType(peerNodeID)));
    values.add(new StructElement(STREAM_USAGE_ID, new UIntType(streamUsage)));
    values.add(new StructElement(VIDEO_STREAM_ID_ID, videoStreamID != null ? new UIntType(videoStreamID) : new NullType()));
    values.add(new StructElement(AUDIO_STREAM_ID_ID, audioStreamID != null ? new UIntType(audioStreamID) : new NullType()));
    values.add(new StructElement(METADATA_OPTIONS_ID, new UIntType(metadataOptions)));
    values.add(new StructElement(FABRIC_INDEX_ID, new UIntType(fabricIndex)));

    return new StructType(values);
  }

  public static WebRTCTransportRequestorClusterWebRTCSessionStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer id = null;
    Long peerNodeID = null;
    Integer streamUsage = null;
    @Nullable Integer videoStreamID = null;
    @Nullable Integer audioStreamID = null;
    Integer metadataOptions = null;
    Integer fabricIndex = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          id = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == PEER_NODE_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          peerNodeID = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == STREAM_USAGE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          streamUsage = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == VIDEO_STREAM_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          videoStreamID = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == AUDIO_STREAM_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          audioStreamID = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == METADATA_OPTIONS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          metadataOptions = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == FABRIC_INDEX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          fabricIndex = castingValue.value(Integer.class);
        }
      }
    }
    return new WebRTCTransportRequestorClusterWebRTCSessionStruct(
      id,
      peerNodeID,
      streamUsage,
      videoStreamID,
      audioStreamID,
      metadataOptions,
      fabricIndex
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("WebRTCTransportRequestorClusterWebRTCSessionStruct {\n");
    output.append("\tid: ");
    output.append(id);
    output.append("\n");
    output.append("\tpeerNodeID: ");
    output.append(peerNodeID);
    output.append("\n");
    output.append("\tstreamUsage: ");
    output.append(streamUsage);
    output.append("\n");
    output.append("\tvideoStreamID: ");
    output.append(videoStreamID);
    output.append("\n");
    output.append("\taudioStreamID: ");
    output.append(audioStreamID);
    output.append("\n");
    output.append("\tmetadataOptions: ");
    output.append(metadataOptions);
    output.append("\n");
    output.append("\tfabricIndex: ");
    output.append(fabricIndex);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class PushAvStreamTransportClusterTransportMotionTriggerTimeControlStruct {
  public Integer initialDuration;
  public Integer augmentationDuration;
  public Long maxDuration;
  public Integer blindDuration;
  private static final long INITIAL_DURATION_ID = 0L;
  private static final long AUGMENTATION_DURATION_ID = 1L;
  private static final long MAX_DURATION_ID = 2L;
  private static final long BLIND_DURATION_ID = 3L;

  public PushAvStreamTransportClusterTransportMotionTriggerTimeControlStruct(
    Integer initialDuration,
    Integer augmentationDuration,
    Long maxDuration,
    Integer blindDuration
  ) {
    this.initialDuration = initialDuration;
    this.augmentationDuration = augmentationDuration;
    this.maxDuration = maxDuration;
    this.blindDuration = blindDuration;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(INITIAL_DURATION_ID, new UIntType(initialDuration)));
    values.add(new StructElement(AUGMENTATION_DURATION_ID, new UIntType(augmentationDuration)));
    values.add(new StructElement(MAX_DURATION_ID, new UIntType(maxDuration)));
    values.add(new StructElement(BLIND_DURATION_ID, new UIntType(blindDuration)));

    return new StructType(values);
  }

  public static PushAvStreamTransportClusterTransportMotionTriggerTimeControlStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer initialDuration = null;
    Integer augmentationDuration = null;
    Long maxDuration = null;
    Integer blindDuration = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == INITIAL_DURATION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          initialDuration = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == AUGMENTATION_DURATION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          augmentationDuration = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == MAX_DURATION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          maxDuration = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == BLIND_DURATION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          blindDuration = castingValue.value(Integer.class);
        }
      }
    }
    return new PushAvStreamTransportClusterTransportMotionTriggerTimeControlStruct(
      initialDuration,
      augmentationDuration,
      maxDuration,
      blindDuration
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("PushAvStreamTransportClusterTransportMotionTriggerTimeControlStruct {\n");
    output.append("\tinitialDuration: ");
    output.append(initialDuration);
    output.append("\n");
    output.append("\taugmentationDuration: ");
    output.append(augmentationDuration);
    output.append("\n");
    output.append("\tmaxDuration: ");
    output.append(maxDuration);
    output.append("\n");
    output.append("\tblindDuration: ");
    output.append(blindDuration);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class PushAvStreamTransportClusterTransportZoneOptionsStruct {
  public @Nullable Integer zone;
  public Optional<Integer> sensitivity;
  private static final long ZONE_ID = 1L;
  private static final long SENSITIVITY_ID = 2L;

  public PushAvStreamTransportClusterTransportZoneOptionsStruct(
    @Nullable Integer zone,
    Optional<Integer> sensitivity
  ) {
    this.zone = zone;
    this.sensitivity = sensitivity;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ZONE_ID, zone != null ? new UIntType(zone) : new NullType()));
    values.add(new StructElement(SENSITIVITY_ID, sensitivity.<BaseTLVType>map((nonOptionalsensitivity) -> new UIntType(nonOptionalsensitivity)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static PushAvStreamTransportClusterTransportZoneOptionsStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    @Nullable Integer zone = null;
    Optional<Integer> sensitivity = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ZONE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          zone = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == SENSITIVITY_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          sensitivity = Optional.of(castingValue.value(Integer.class));
        }
      }
    }
    return new PushAvStreamTransportClusterTransportZoneOptionsStruct(
      zone,
      sensitivity
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("PushAvStreamTransportClusterTransportZoneOptionsStruct {\n");
    output.append("\tzone: ");
    output.append(zone);
    output.append("\n");
    output.append("\tsensitivity: ");
    output.append(sensitivity);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class PushAvStreamTransportClusterMetadataOptionsStruct {
  public Integer multiplexing;
  public Boolean includeMotionZones;
  public Boolean enableMetadataPrivacySensitive;
  private static final long MULTIPLEXING_ID = 0L;
  private static final long INCLUDE_MOTION_ZONES_ID = 1L;
  private static final long ENABLE_METADATA_PRIVACY_SENSITIVE_ID = 2L;

  public PushAvStreamTransportClusterMetadataOptionsStruct(
    Integer multiplexing,
    Boolean includeMotionZones,
    Boolean enableMetadataPrivacySensitive
  ) {
    this.multiplexing = multiplexing;
    this.includeMotionZones = includeMotionZones;
    this.enableMetadataPrivacySensitive = enableMetadataPrivacySensitive;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(MULTIPLEXING_ID, new UIntType(multiplexing)));
    values.add(new StructElement(INCLUDE_MOTION_ZONES_ID, new BooleanType(includeMotionZones)));
    values.add(new StructElement(ENABLE_METADATA_PRIVACY_SENSITIVE_ID, new BooleanType(enableMetadataPrivacySensitive)));

    return new StructType(values);
  }

  public static PushAvStreamTransportClusterMetadataOptionsStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer multiplexing = null;
    Boolean includeMotionZones = null;
    Boolean enableMetadataPrivacySensitive = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == MULTIPLEXING_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          multiplexing = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == INCLUDE_MOTION_ZONES_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          includeMotionZones = castingValue.value(Boolean.class);
        }
      } else if (element.contextTagNum() == ENABLE_METADATA_PRIVACY_SENSITIVE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          enableMetadataPrivacySensitive = castingValue.value(Boolean.class);
        }
      }
    }
    return new PushAvStreamTransportClusterMetadataOptionsStruct(
      multiplexing,
      includeMotionZones,
      enableMetadataPrivacySensitive
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("PushAvStreamTransportClusterMetadataOptionsStruct {\n");
    output.append("\tmultiplexing: ");
    output.append(multiplexing);
    output.append("\n");
    output.append("\tincludeMotionZones: ");
    output.append(includeMotionZones);
    output.append("\n");
    output.append("\tenableMetadataPrivacySensitive: ");
    output.append(enableMetadataPrivacySensitive);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class PushAvStreamTransportClusterTransportTriggerOptionsStruct {
  public Integer triggerType;
  public @Nullable Optional<ArrayList<ChipStructs.PushAvStreamTransportClusterTransportZoneOptionsStruct>> motionZones;
  public @Nullable Optional<Integer> motionSensitivity;
  public Optional<ChipStructs.PushAvStreamTransportClusterTransportMotionTriggerTimeControlStruct> motionTimeControl;
  public Optional<Integer> maxPreRollLen;
  private static final long TRIGGER_TYPE_ID = 0L;
  private static final long MOTION_ZONES_ID = 1L;
  private static final long MOTION_SENSITIVITY_ID = 2L;
  private static final long MOTION_TIME_CONTROL_ID = 3L;
  private static final long MAX_PRE_ROLL_LEN_ID = 4L;

  public PushAvStreamTransportClusterTransportTriggerOptionsStruct(
    Integer triggerType,
    @Nullable Optional<ArrayList<ChipStructs.PushAvStreamTransportClusterTransportZoneOptionsStruct>> motionZones,
    @Nullable Optional<Integer> motionSensitivity,
    Optional<ChipStructs.PushAvStreamTransportClusterTransportMotionTriggerTimeControlStruct> motionTimeControl,
    Optional<Integer> maxPreRollLen
  ) {
    this.triggerType = triggerType;
    this.motionZones = motionZones;
    this.motionSensitivity = motionSensitivity;
    this.motionTimeControl = motionTimeControl;
    this.maxPreRollLen = maxPreRollLen;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(TRIGGER_TYPE_ID, new UIntType(triggerType)));
    values.add(new StructElement(MOTION_ZONES_ID, motionZones != null ? motionZones.<BaseTLVType>map((nonOptionalmotionZones) -> ArrayType.generateArrayType(nonOptionalmotionZones, (elementnonOptionalmotionZones) -> elementnonOptionalmotionZones.encodeTlv())).orElse(new EmptyType()) : new NullType()));
    values.add(new StructElement(MOTION_SENSITIVITY_ID, motionSensitivity != null ? motionSensitivity.<BaseTLVType>map((nonOptionalmotionSensitivity) -> new UIntType(nonOptionalmotionSensitivity)).orElse(new EmptyType()) : new NullType()));
    values.add(new StructElement(MOTION_TIME_CONTROL_ID, motionTimeControl.<BaseTLVType>map((nonOptionalmotionTimeControl) -> nonOptionalmotionTimeControl.encodeTlv()).orElse(new EmptyType())));
    values.add(new StructElement(MAX_PRE_ROLL_LEN_ID, maxPreRollLen.<BaseTLVType>map((nonOptionalmaxPreRollLen) -> new UIntType(nonOptionalmaxPreRollLen)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static PushAvStreamTransportClusterTransportTriggerOptionsStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer triggerType = null;
    @Nullable Optional<ArrayList<ChipStructs.PushAvStreamTransportClusterTransportZoneOptionsStruct>> motionZones = null;
    @Nullable Optional<Integer> motionSensitivity = null;
    Optional<ChipStructs.PushAvStreamTransportClusterTransportMotionTriggerTimeControlStruct> motionTimeControl = Optional.empty();
    Optional<Integer> maxPreRollLen = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == TRIGGER_TYPE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          triggerType = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == MOTION_ZONES_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          motionZones = Optional.of(castingValue.map((elementcastingValue) -> ChipStructs.PushAvStreamTransportClusterTransportZoneOptionsStruct.decodeTlv(elementcastingValue)));
        }
      } else if (element.contextTagNum() == MOTION_SENSITIVITY_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          motionSensitivity = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == MOTION_TIME_CONTROL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          motionTimeControl = Optional.of(ChipStructs.PushAvStreamTransportClusterTransportMotionTriggerTimeControlStruct.decodeTlv(castingValue));
        }
      } else if (element.contextTagNum() == MAX_PRE_ROLL_LEN_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          maxPreRollLen = Optional.of(castingValue.value(Integer.class));
        }
      }
    }
    return new PushAvStreamTransportClusterTransportTriggerOptionsStruct(
      triggerType,
      motionZones,
      motionSensitivity,
      motionTimeControl,
      maxPreRollLen
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("PushAvStreamTransportClusterTransportTriggerOptionsStruct {\n");
    output.append("\ttriggerType: ");
    output.append(triggerType);
    output.append("\n");
    output.append("\tmotionZones: ");
    output.append(motionZones);
    output.append("\n");
    output.append("\tmotionSensitivity: ");
    output.append(motionSensitivity);
    output.append("\n");
    output.append("\tmotionTimeControl: ");
    output.append(motionTimeControl);
    output.append("\n");
    output.append("\tmaxPreRollLen: ");
    output.append(maxPreRollLen);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class PushAvStreamTransportClusterCMAFContainerOptionsStruct {
  public Integer chunkDuration;
  public Optional<byte[]> CENCKey;
  private static final long CHUNK_DURATION_ID = 0L;
  private static final long CENC_KEY_ID = 1L;

  public PushAvStreamTransportClusterCMAFContainerOptionsStruct(
    Integer chunkDuration,
    Optional<byte[]> CENCKey
  ) {
    this.chunkDuration = chunkDuration;
    this.CENCKey = CENCKey;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(CHUNK_DURATION_ID, new UIntType(chunkDuration)));
    values.add(new StructElement(CENC_KEY_ID, CENCKey.<BaseTLVType>map((nonOptionalCENCKey) -> new ByteArrayType(nonOptionalCENCKey)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static PushAvStreamTransportClusterCMAFContainerOptionsStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer chunkDuration = null;
    Optional<byte[]> CENCKey = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == CHUNK_DURATION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          chunkDuration = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == CENC_KEY_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.ByteArray) {
          ByteArrayType castingValue = element.value(ByteArrayType.class);
          CENCKey = Optional.of(castingValue.value(byte[].class));
        }
      }
    }
    return new PushAvStreamTransportClusterCMAFContainerOptionsStruct(
      chunkDuration,
      CENCKey
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("PushAvStreamTransportClusterCMAFContainerOptionsStruct {\n");
    output.append("\tchunkDuration: ");
    output.append(chunkDuration);
    output.append("\n");
    output.append("\tCENCKey: ");
    output.append(CENCKey.isPresent() ? Arrays.toString(CENCKey.get()) : "");
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class PushAvStreamTransportClusterContainerOptionsStruct {
  public Integer containerType;
  public Optional<ChipStructs.PushAvStreamTransportClusterCMAFContainerOptionsStruct> CMAFContainerOptions;
  private static final long CONTAINER_TYPE_ID = 0L;
  private static final long CMAF_CONTAINER_OPTIONS_ID = 1L;

  public PushAvStreamTransportClusterContainerOptionsStruct(
    Integer containerType,
    Optional<ChipStructs.PushAvStreamTransportClusterCMAFContainerOptionsStruct> CMAFContainerOptions
  ) {
    this.containerType = containerType;
    this.CMAFContainerOptions = CMAFContainerOptions;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(CONTAINER_TYPE_ID, new UIntType(containerType)));
    values.add(new StructElement(CMAF_CONTAINER_OPTIONS_ID, CMAFContainerOptions.<BaseTLVType>map((nonOptionalCMAFContainerOptions) -> nonOptionalCMAFContainerOptions.encodeTlv()).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static PushAvStreamTransportClusterContainerOptionsStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer containerType = null;
    Optional<ChipStructs.PushAvStreamTransportClusterCMAFContainerOptionsStruct> CMAFContainerOptions = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == CONTAINER_TYPE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          containerType = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == CMAF_CONTAINER_OPTIONS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          CMAFContainerOptions = Optional.of(ChipStructs.PushAvStreamTransportClusterCMAFContainerOptionsStruct.decodeTlv(castingValue));
        }
      }
    }
    return new PushAvStreamTransportClusterContainerOptionsStruct(
      containerType,
      CMAFContainerOptions
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("PushAvStreamTransportClusterContainerOptionsStruct {\n");
    output.append("\tcontainerType: ");
    output.append(containerType);
    output.append("\n");
    output.append("\tCMAFContainerOptions: ");
    output.append(CMAFContainerOptions);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class PushAvStreamTransportClusterTransportOptionsStruct {
  public Integer streamUsage;
  public Optional<Integer> videoStreamID;
  public Optional<Integer> audioStreamID;
  public Integer endpointID;
  public String url;
  public ChipStructs.PushAvStreamTransportClusterTransportTriggerOptionsStruct triggerOptions;
  public Integer ingestMethod;
  public Integer containerFormat;
  public ChipStructs.PushAvStreamTransportClusterContainerOptionsStruct containerOptions;
  public Optional<ChipStructs.PushAvStreamTransportClusterMetadataOptionsStruct> metadataOptions;
  public Optional<Long> expiryTime;
  private static final long STREAM_USAGE_ID = 0L;
  private static final long VIDEO_STREAM_ID_ID = 1L;
  private static final long AUDIO_STREAM_ID_ID = 2L;
  private static final long ENDPOINT_ID_ID = 3L;
  private static final long URL_ID = 4L;
  private static final long TRIGGER_OPTIONS_ID = 5L;
  private static final long INGEST_METHOD_ID = 6L;
  private static final long CONTAINER_FORMAT_ID = 7L;
  private static final long CONTAINER_OPTIONS_ID = 8L;
  private static final long METADATA_OPTIONS_ID = 9L;
  private static final long EXPIRY_TIME_ID = 10L;

  public PushAvStreamTransportClusterTransportOptionsStruct(
    Integer streamUsage,
    Optional<Integer> videoStreamID,
    Optional<Integer> audioStreamID,
    Integer endpointID,
    String url,
    ChipStructs.PushAvStreamTransportClusterTransportTriggerOptionsStruct triggerOptions,
    Integer ingestMethod,
    Integer containerFormat,
    ChipStructs.PushAvStreamTransportClusterContainerOptionsStruct containerOptions,
    Optional<ChipStructs.PushAvStreamTransportClusterMetadataOptionsStruct> metadataOptions,
    Optional<Long> expiryTime
  ) {
    this.streamUsage = streamUsage;
    this.videoStreamID = videoStreamID;
    this.audioStreamID = audioStreamID;
    this.endpointID = endpointID;
    this.url = url;
    this.triggerOptions = triggerOptions;
    this.ingestMethod = ingestMethod;
    this.containerFormat = containerFormat;
    this.containerOptions = containerOptions;
    this.metadataOptions = metadataOptions;
    this.expiryTime = expiryTime;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(STREAM_USAGE_ID, new UIntType(streamUsage)));
    values.add(new StructElement(VIDEO_STREAM_ID_ID, videoStreamID.<BaseTLVType>map((nonOptionalvideoStreamID) -> new UIntType(nonOptionalvideoStreamID)).orElse(new EmptyType())));
    values.add(new StructElement(AUDIO_STREAM_ID_ID, audioStreamID.<BaseTLVType>map((nonOptionalaudioStreamID) -> new UIntType(nonOptionalaudioStreamID)).orElse(new EmptyType())));
    values.add(new StructElement(ENDPOINT_ID_ID, new UIntType(endpointID)));
    values.add(new StructElement(URL_ID, new StringType(url)));
    values.add(new StructElement(TRIGGER_OPTIONS_ID, triggerOptions.encodeTlv()));
    values.add(new StructElement(INGEST_METHOD_ID, new UIntType(ingestMethod)));
    values.add(new StructElement(CONTAINER_FORMAT_ID, new UIntType(containerFormat)));
    values.add(new StructElement(CONTAINER_OPTIONS_ID, containerOptions.encodeTlv()));
    values.add(new StructElement(METADATA_OPTIONS_ID, metadataOptions.<BaseTLVType>map((nonOptionalmetadataOptions) -> nonOptionalmetadataOptions.encodeTlv()).orElse(new EmptyType())));
    values.add(new StructElement(EXPIRY_TIME_ID, expiryTime.<BaseTLVType>map((nonOptionalexpiryTime) -> new UIntType(nonOptionalexpiryTime)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static PushAvStreamTransportClusterTransportOptionsStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer streamUsage = null;
    Optional<Integer> videoStreamID = Optional.empty();
    Optional<Integer> audioStreamID = Optional.empty();
    Integer endpointID = null;
    String url = null;
    ChipStructs.PushAvStreamTransportClusterTransportTriggerOptionsStruct triggerOptions = null;
    Integer ingestMethod = null;
    Integer containerFormat = null;
    ChipStructs.PushAvStreamTransportClusterContainerOptionsStruct containerOptions = null;
    Optional<ChipStructs.PushAvStreamTransportClusterMetadataOptionsStruct> metadataOptions = Optional.empty();
    Optional<Long> expiryTime = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == STREAM_USAGE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          streamUsage = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == VIDEO_STREAM_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          videoStreamID = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == AUDIO_STREAM_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          audioStreamID = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == ENDPOINT_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          endpointID = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == URL_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          url = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == TRIGGER_OPTIONS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          triggerOptions = ChipStructs.PushAvStreamTransportClusterTransportTriggerOptionsStruct.decodeTlv(castingValue);
        }
      } else if (element.contextTagNum() == INGEST_METHOD_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          ingestMethod = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == CONTAINER_FORMAT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          containerFormat = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == CONTAINER_OPTIONS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          containerOptions = ChipStructs.PushAvStreamTransportClusterContainerOptionsStruct.decodeTlv(castingValue);
        }
      } else if (element.contextTagNum() == METADATA_OPTIONS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          metadataOptions = Optional.of(ChipStructs.PushAvStreamTransportClusterMetadataOptionsStruct.decodeTlv(castingValue));
        }
      } else if (element.contextTagNum() == EXPIRY_TIME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          expiryTime = Optional.of(castingValue.value(Long.class));
        }
      }
    }
    return new PushAvStreamTransportClusterTransportOptionsStruct(
      streamUsage,
      videoStreamID,
      audioStreamID,
      endpointID,
      url,
      triggerOptions,
      ingestMethod,
      containerFormat,
      containerOptions,
      metadataOptions,
      expiryTime
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("PushAvStreamTransportClusterTransportOptionsStruct {\n");
    output.append("\tstreamUsage: ");
    output.append(streamUsage);
    output.append("\n");
    output.append("\tvideoStreamID: ");
    output.append(videoStreamID);
    output.append("\n");
    output.append("\taudioStreamID: ");
    output.append(audioStreamID);
    output.append("\n");
    output.append("\tendpointID: ");
    output.append(endpointID);
    output.append("\n");
    output.append("\turl: ");
    output.append(url);
    output.append("\n");
    output.append("\ttriggerOptions: ");
    output.append(triggerOptions);
    output.append("\n");
    output.append("\tingestMethod: ");
    output.append(ingestMethod);
    output.append("\n");
    output.append("\tcontainerFormat: ");
    output.append(containerFormat);
    output.append("\n");
    output.append("\tcontainerOptions: ");
    output.append(containerOptions);
    output.append("\n");
    output.append("\tmetadataOptions: ");
    output.append(metadataOptions);
    output.append("\n");
    output.append("\texpiryTime: ");
    output.append(expiryTime);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class PushAvStreamTransportClusterTransportConfigurationStruct {
  public Integer connectionID;
  public Integer transportStatus;
  public ChipStructs.PushAvStreamTransportClusterTransportOptionsStruct transportOptions;
  private static final long CONNECTION_ID_ID = 0L;
  private static final long TRANSPORT_STATUS_ID = 1L;
  private static final long TRANSPORT_OPTIONS_ID = 2L;

  public PushAvStreamTransportClusterTransportConfigurationStruct(
    Integer connectionID,
    Integer transportStatus,
    ChipStructs.PushAvStreamTransportClusterTransportOptionsStruct transportOptions
  ) {
    this.connectionID = connectionID;
    this.transportStatus = transportStatus;
    this.transportOptions = transportOptions;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(CONNECTION_ID_ID, new UIntType(connectionID)));
    values.add(new StructElement(TRANSPORT_STATUS_ID, new UIntType(transportStatus)));
    values.add(new StructElement(TRANSPORT_OPTIONS_ID, transportOptions.encodeTlv()));

    return new StructType(values);
  }

  public static PushAvStreamTransportClusterTransportConfigurationStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer connectionID = null;
    Integer transportStatus = null;
    ChipStructs.PushAvStreamTransportClusterTransportOptionsStruct transportOptions = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == CONNECTION_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          connectionID = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == TRANSPORT_STATUS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          transportStatus = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == TRANSPORT_OPTIONS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          transportOptions = ChipStructs.PushAvStreamTransportClusterTransportOptionsStruct.decodeTlv(castingValue);
        }
      }
    }
    return new PushAvStreamTransportClusterTransportConfigurationStruct(
      connectionID,
      transportStatus,
      transportOptions
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("PushAvStreamTransportClusterTransportConfigurationStruct {\n");
    output.append("\tconnectionID: ");
    output.append(connectionID);
    output.append("\n");
    output.append("\ttransportStatus: ");
    output.append(transportStatus);
    output.append("\n");
    output.append("\ttransportOptions: ");
    output.append(transportOptions);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class ChimeClusterChimeSoundStruct {
  public Integer chimeID;
  public String name;
  private static final long CHIME_ID_ID = 0L;
  private static final long NAME_ID = 1L;

  public ChimeClusterChimeSoundStruct(
    Integer chimeID,
    String name
  ) {
    this.chimeID = chimeID;
    this.name = name;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(CHIME_ID_ID, new UIntType(chimeID)));
    values.add(new StructElement(NAME_ID, new StringType(name)));

    return new StructType(values);
  }

  public static ChimeClusterChimeSoundStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer chimeID = null;
    String name = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == CHIME_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          chimeID = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == NAME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          name = castingValue.value(String.class);
        }
      }
    }
    return new ChimeClusterChimeSoundStruct(
      chimeID,
      name
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ChimeClusterChimeSoundStruct {\n");
    output.append("\tchimeID: ");
    output.append(chimeID);
    output.append("\n");
    output.append("\tname: ");
    output.append(name);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class EcosystemInformationClusterDeviceTypeStruct {
  public Long deviceType;
  public Integer revision;
  private static final long DEVICE_TYPE_ID = 0L;
  private static final long REVISION_ID = 1L;

  public EcosystemInformationClusterDeviceTypeStruct(
    Long deviceType,
    Integer revision
  ) {
    this.deviceType = deviceType;
    this.revision = revision;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(DEVICE_TYPE_ID, new UIntType(deviceType)));
    values.add(new StructElement(REVISION_ID, new UIntType(revision)));

    return new StructType(values);
  }

  public static EcosystemInformationClusterDeviceTypeStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long deviceType = null;
    Integer revision = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == DEVICE_TYPE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          deviceType = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == REVISION_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          revision = castingValue.value(Integer.class);
        }
      }
    }
    return new EcosystemInformationClusterDeviceTypeStruct(
      deviceType,
      revision
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("EcosystemInformationClusterDeviceTypeStruct {\n");
    output.append("\tdeviceType: ");
    output.append(deviceType);
    output.append("\n");
    output.append("\trevision: ");
    output.append(revision);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class EcosystemInformationClusterEcosystemDeviceStruct {
  public Optional<String> deviceName;
  public Optional<Long> deviceNameLastEdit;
  public Integer bridgedEndpoint;
  public Integer originalEndpoint;
  public ArrayList<ChipStructs.EcosystemInformationClusterDeviceTypeStruct> deviceTypes;
  public ArrayList<String> uniqueLocationIDs;
  public Long uniqueLocationIDsLastEdit;
  public Integer fabricIndex;
  private static final long DEVICE_NAME_ID = 0L;
  private static final long DEVICE_NAME_LAST_EDIT_ID = 1L;
  private static final long BRIDGED_ENDPOINT_ID = 2L;
  private static final long ORIGINAL_ENDPOINT_ID = 3L;
  private static final long DEVICE_TYPES_ID = 4L;
  private static final long UNIQUE_LOCATION_I_DS_ID = 5L;
  private static final long UNIQUE_LOCATION_I_DS_LAST_EDIT_ID = 6L;
  private static final long FABRIC_INDEX_ID = 254L;

  public EcosystemInformationClusterEcosystemDeviceStruct(
    Optional<String> deviceName,
    Optional<Long> deviceNameLastEdit,
    Integer bridgedEndpoint,
    Integer originalEndpoint,
    ArrayList<ChipStructs.EcosystemInformationClusterDeviceTypeStruct> deviceTypes,
    ArrayList<String> uniqueLocationIDs,
    Long uniqueLocationIDsLastEdit,
    Integer fabricIndex
  ) {
    this.deviceName = deviceName;
    this.deviceNameLastEdit = deviceNameLastEdit;
    this.bridgedEndpoint = bridgedEndpoint;
    this.originalEndpoint = originalEndpoint;
    this.deviceTypes = deviceTypes;
    this.uniqueLocationIDs = uniqueLocationIDs;
    this.uniqueLocationIDsLastEdit = uniqueLocationIDsLastEdit;
    this.fabricIndex = fabricIndex;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(DEVICE_NAME_ID, deviceName.<BaseTLVType>map((nonOptionaldeviceName) -> new StringType(nonOptionaldeviceName)).orElse(new EmptyType())));
    values.add(new StructElement(DEVICE_NAME_LAST_EDIT_ID, deviceNameLastEdit.<BaseTLVType>map((nonOptionaldeviceNameLastEdit) -> new UIntType(nonOptionaldeviceNameLastEdit)).orElse(new EmptyType())));
    values.add(new StructElement(BRIDGED_ENDPOINT_ID, new UIntType(bridgedEndpoint)));
    values.add(new StructElement(ORIGINAL_ENDPOINT_ID, new UIntType(originalEndpoint)));
    values.add(new StructElement(DEVICE_TYPES_ID, ArrayType.generateArrayType(deviceTypes, (elementdeviceTypes) -> elementdeviceTypes.encodeTlv())));
    values.add(new StructElement(UNIQUE_LOCATION_I_DS_ID, ArrayType.generateArrayType(uniqueLocationIDs, (elementuniqueLocationIDs) -> new StringType(elementuniqueLocationIDs))));
    values.add(new StructElement(UNIQUE_LOCATION_I_DS_LAST_EDIT_ID, new UIntType(uniqueLocationIDsLastEdit)));
    values.add(new StructElement(FABRIC_INDEX_ID, new UIntType(fabricIndex)));

    return new StructType(values);
  }

  public static EcosystemInformationClusterEcosystemDeviceStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Optional<String> deviceName = Optional.empty();
    Optional<Long> deviceNameLastEdit = Optional.empty();
    Integer bridgedEndpoint = null;
    Integer originalEndpoint = null;
    ArrayList<ChipStructs.EcosystemInformationClusterDeviceTypeStruct> deviceTypes = null;
    ArrayList<String> uniqueLocationIDs = null;
    Long uniqueLocationIDsLastEdit = null;
    Integer fabricIndex = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == DEVICE_NAME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          deviceName = Optional.of(castingValue.value(String.class));
        }
      } else if (element.contextTagNum() == DEVICE_NAME_LAST_EDIT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          deviceNameLastEdit = Optional.of(castingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == BRIDGED_ENDPOINT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          bridgedEndpoint = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == ORIGINAL_ENDPOINT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          originalEndpoint = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == DEVICE_TYPES_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          deviceTypes = castingValue.map((elementcastingValue) -> ChipStructs.EcosystemInformationClusterDeviceTypeStruct.decodeTlv(elementcastingValue));
        }
      } else if (element.contextTagNum() == UNIQUE_LOCATION_I_DS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          uniqueLocationIDs = castingValue.map((elementcastingValue) -> elementcastingValue.value(String.class));
        }
      } else if (element.contextTagNum() == UNIQUE_LOCATION_I_DS_LAST_EDIT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          uniqueLocationIDsLastEdit = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == FABRIC_INDEX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          fabricIndex = castingValue.value(Integer.class);
        }
      }
    }
    return new EcosystemInformationClusterEcosystemDeviceStruct(
      deviceName,
      deviceNameLastEdit,
      bridgedEndpoint,
      originalEndpoint,
      deviceTypes,
      uniqueLocationIDs,
      uniqueLocationIDsLastEdit,
      fabricIndex
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("EcosystemInformationClusterEcosystemDeviceStruct {\n");
    output.append("\tdeviceName: ");
    output.append(deviceName);
    output.append("\n");
    output.append("\tdeviceNameLastEdit: ");
    output.append(deviceNameLastEdit);
    output.append("\n");
    output.append("\tbridgedEndpoint: ");
    output.append(bridgedEndpoint);
    output.append("\n");
    output.append("\toriginalEndpoint: ");
    output.append(originalEndpoint);
    output.append("\n");
    output.append("\tdeviceTypes: ");
    output.append(deviceTypes);
    output.append("\n");
    output.append("\tuniqueLocationIDs: ");
    output.append(uniqueLocationIDs);
    output.append("\n");
    output.append("\tuniqueLocationIDsLastEdit: ");
    output.append(uniqueLocationIDsLastEdit);
    output.append("\n");
    output.append("\tfabricIndex: ");
    output.append(fabricIndex);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class EcosystemInformationClusterEcosystemLocationStruct {
  public String uniqueLocationID;
  public ChipStructs.EcosystemInformationClusterLocationDescriptorStruct locationDescriptor;
  public Long locationDescriptorLastEdit;
  public Integer fabricIndex;
  private static final long UNIQUE_LOCATION_ID_ID = 0L;
  private static final long LOCATION_DESCRIPTOR_ID = 1L;
  private static final long LOCATION_DESCRIPTOR_LAST_EDIT_ID = 2L;
  private static final long FABRIC_INDEX_ID = 254L;

  public EcosystemInformationClusterEcosystemLocationStruct(
    String uniqueLocationID,
    ChipStructs.EcosystemInformationClusterLocationDescriptorStruct locationDescriptor,
    Long locationDescriptorLastEdit,
    Integer fabricIndex
  ) {
    this.uniqueLocationID = uniqueLocationID;
    this.locationDescriptor = locationDescriptor;
    this.locationDescriptorLastEdit = locationDescriptorLastEdit;
    this.fabricIndex = fabricIndex;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(UNIQUE_LOCATION_ID_ID, new StringType(uniqueLocationID)));
    values.add(new StructElement(LOCATION_DESCRIPTOR_ID, locationDescriptor.encodeTlv()));
    values.add(new StructElement(LOCATION_DESCRIPTOR_LAST_EDIT_ID, new UIntType(locationDescriptorLastEdit)));
    values.add(new StructElement(FABRIC_INDEX_ID, new UIntType(fabricIndex)));

    return new StructType(values);
  }

  public static EcosystemInformationClusterEcosystemLocationStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    String uniqueLocationID = null;
    ChipStructs.EcosystemInformationClusterLocationDescriptorStruct locationDescriptor = null;
    Long locationDescriptorLastEdit = null;
    Integer fabricIndex = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == UNIQUE_LOCATION_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          uniqueLocationID = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == LOCATION_DESCRIPTOR_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          locationDescriptor = ChipStructs.EcosystemInformationClusterLocationDescriptorStruct.decodeTlv(castingValue);
        }
      } else if (element.contextTagNum() == LOCATION_DESCRIPTOR_LAST_EDIT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          locationDescriptorLastEdit = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == FABRIC_INDEX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          fabricIndex = castingValue.value(Integer.class);
        }
      }
    }
    return new EcosystemInformationClusterEcosystemLocationStruct(
      uniqueLocationID,
      locationDescriptor,
      locationDescriptorLastEdit,
      fabricIndex
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("EcosystemInformationClusterEcosystemLocationStruct {\n");
    output.append("\tuniqueLocationID: ");
    output.append(uniqueLocationID);
    output.append("\n");
    output.append("\tlocationDescriptor: ");
    output.append(locationDescriptor);
    output.append("\n");
    output.append("\tlocationDescriptorLastEdit: ");
    output.append(locationDescriptorLastEdit);
    output.append("\n");
    output.append("\tfabricIndex: ");
    output.append(fabricIndex);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class EcosystemInformationClusterLocationDescriptorStruct {
  public String locationName;
  public @Nullable Integer floorNumber;
  public @Nullable Integer areaType;
  private static final long LOCATION_NAME_ID = 0L;
  private static final long FLOOR_NUMBER_ID = 1L;
  private static final long AREA_TYPE_ID = 2L;

  public EcosystemInformationClusterLocationDescriptorStruct(
    String locationName,
    @Nullable Integer floorNumber,
    @Nullable Integer areaType
  ) {
    this.locationName = locationName;
    this.floorNumber = floorNumber;
    this.areaType = areaType;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(LOCATION_NAME_ID, new StringType(locationName)));
    values.add(new StructElement(FLOOR_NUMBER_ID, floorNumber != null ? new IntType(floorNumber) : new NullType()));
    values.add(new StructElement(AREA_TYPE_ID, areaType != null ? new UIntType(areaType) : new NullType()));

    return new StructType(values);
  }

  public static EcosystemInformationClusterLocationDescriptorStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    String locationName = null;
    @Nullable Integer floorNumber = null;
    @Nullable Integer areaType = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == LOCATION_NAME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          locationName = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == FLOOR_NUMBER_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Int) {
          IntType castingValue = element.value(IntType.class);
          floorNumber = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == AREA_TYPE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          areaType = castingValue.value(Integer.class);
        }
      }
    }
    return new EcosystemInformationClusterLocationDescriptorStruct(
      locationName,
      floorNumber,
      areaType
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("EcosystemInformationClusterLocationDescriptorStruct {\n");
    output.append("\tlocationName: ");
    output.append(locationName);
    output.append("\n");
    output.append("\tfloorNumber: ");
    output.append(floorNumber);
    output.append("\n");
    output.append("\tareaType: ");
    output.append(areaType);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class TlsCertificateManagementClusterTLSCertStruct {
  public Integer caid;
  public Optional<byte[]> certificate;
  private static final long CAID_ID = 0L;
  private static final long CERTIFICATE_ID = 1L;

  public TlsCertificateManagementClusterTLSCertStruct(
    Integer caid,
    Optional<byte[]> certificate
  ) {
    this.caid = caid;
    this.certificate = certificate;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(CAID_ID, new UIntType(caid)));
    values.add(new StructElement(CERTIFICATE_ID, certificate.<BaseTLVType>map((nonOptionalcertificate) -> new ByteArrayType(nonOptionalcertificate)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static TlsCertificateManagementClusterTLSCertStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer caid = null;
    Optional<byte[]> certificate = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == CAID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          caid = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == CERTIFICATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.ByteArray) {
          ByteArrayType castingValue = element.value(ByteArrayType.class);
          certificate = Optional.of(castingValue.value(byte[].class));
        }
      }
    }
    return new TlsCertificateManagementClusterTLSCertStruct(
      caid,
      certificate
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("TlsCertificateManagementClusterTLSCertStruct {\n");
    output.append("\tcaid: ");
    output.append(caid);
    output.append("\n");
    output.append("\tcertificate: ");
    output.append(certificate.isPresent() ? Arrays.toString(certificate.get()) : "");
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class TlsCertificateManagementClusterTLSClientCertificateDetailStruct {
  public Integer ccdid;
  public Optional<byte[]> clientCertificate;
  public Optional<ArrayList<byte[]>> intermediateCertificates;
  private static final long CCDID_ID = 0L;
  private static final long CLIENT_CERTIFICATE_ID = 1L;
  private static final long INTERMEDIATE_CERTIFICATES_ID = 2L;

  public TlsCertificateManagementClusterTLSClientCertificateDetailStruct(
    Integer ccdid,
    Optional<byte[]> clientCertificate,
    Optional<ArrayList<byte[]>> intermediateCertificates
  ) {
    this.ccdid = ccdid;
    this.clientCertificate = clientCertificate;
    this.intermediateCertificates = intermediateCertificates;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(CCDID_ID, new UIntType(ccdid)));
    values.add(new StructElement(CLIENT_CERTIFICATE_ID, clientCertificate.<BaseTLVType>map((nonOptionalclientCertificate) -> new ByteArrayType(nonOptionalclientCertificate)).orElse(new EmptyType())));
    values.add(new StructElement(INTERMEDIATE_CERTIFICATES_ID, intermediateCertificates.<BaseTLVType>map((nonOptionalintermediateCertificates) -> ArrayType.generateArrayType(nonOptionalintermediateCertificates, (elementnonOptionalintermediateCertificates) -> new ByteArrayType(elementnonOptionalintermediateCertificates))).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static TlsCertificateManagementClusterTLSClientCertificateDetailStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer ccdid = null;
    Optional<byte[]> clientCertificate = Optional.empty();
    Optional<ArrayList<byte[]>> intermediateCertificates = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == CCDID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          ccdid = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == CLIENT_CERTIFICATE_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.ByteArray) {
          ByteArrayType castingValue = element.value(ByteArrayType.class);
          clientCertificate = Optional.of(castingValue.value(byte[].class));
        }
      } else if (element.contextTagNum() == INTERMEDIATE_CERTIFICATES_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          intermediateCertificates = Optional.of(castingValue.map((elementcastingValue) -> elementcastingValue.value(byte[].class)));
        }
      }
    }
    return new TlsCertificateManagementClusterTLSClientCertificateDetailStruct(
      ccdid,
      clientCertificate,
      intermediateCertificates
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("TlsCertificateManagementClusterTLSClientCertificateDetailStruct {\n");
    output.append("\tccdid: ");
    output.append(ccdid);
    output.append("\n");
    output.append("\tclientCertificate: ");
    output.append(clientCertificate.isPresent() ? Arrays.toString(clientCertificate.get()) : "");
    output.append("\n");
    output.append("\tintermediateCertificates: ");
    output.append(intermediateCertificates);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class TlsClientManagementClusterTLSEndpointStruct {
  public Integer endpointID;
  public byte[] hostname;
  public Integer port;
  public Integer caid;
  public @Nullable Integer ccdid;
  public Integer status;
  private static final long ENDPOINT_ID_ID = 0L;
  private static final long HOSTNAME_ID = 1L;
  private static final long PORT_ID = 2L;
  private static final long CAID_ID = 3L;
  private static final long CCDID_ID = 4L;
  private static final long STATUS_ID = 5L;

  public TlsClientManagementClusterTLSEndpointStruct(
    Integer endpointID,
    byte[] hostname,
    Integer port,
    Integer caid,
    @Nullable Integer ccdid,
    Integer status
  ) {
    this.endpointID = endpointID;
    this.hostname = hostname;
    this.port = port;
    this.caid = caid;
    this.ccdid = ccdid;
    this.status = status;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ENDPOINT_ID_ID, new UIntType(endpointID)));
    values.add(new StructElement(HOSTNAME_ID, new ByteArrayType(hostname)));
    values.add(new StructElement(PORT_ID, new UIntType(port)));
    values.add(new StructElement(CAID_ID, new UIntType(caid)));
    values.add(new StructElement(CCDID_ID, ccdid != null ? new UIntType(ccdid) : new NullType()));
    values.add(new StructElement(STATUS_ID, new UIntType(status)));

    return new StructType(values);
  }

  public static TlsClientManagementClusterTLSEndpointStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer endpointID = null;
    byte[] hostname = null;
    Integer port = null;
    Integer caid = null;
    @Nullable Integer ccdid = null;
    Integer status = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == ENDPOINT_ID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          endpointID = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == HOSTNAME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.ByteArray) {
          ByteArrayType castingValue = element.value(ByteArrayType.class);
          hostname = castingValue.value(byte[].class);
        }
      } else if (element.contextTagNum() == PORT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          port = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == CAID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          caid = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == CCDID_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          ccdid = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == STATUS_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          status = castingValue.value(Integer.class);
        }
      }
    }
    return new TlsClientManagementClusterTLSEndpointStruct(
      endpointID,
      hostname,
      port,
      caid,
      ccdid,
      status
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("TlsClientManagementClusterTLSEndpointStruct {\n");
    output.append("\tendpointID: ");
    output.append(endpointID);
    output.append("\n");
    output.append("\thostname: ");
    output.append(Arrays.toString(hostname));
    output.append("\n");
    output.append("\tport: ");
    output.append(port);
    output.append("\n");
    output.append("\tcaid: ");
    output.append(caid);
    output.append("\n");
    output.append("\tccdid: ");
    output.append(ccdid);
    output.append("\n");
    output.append("\tstatus: ");
    output.append(status);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class UnitTestingClusterSimpleStruct {
  public Integer a;
  public Boolean b;
  public Integer c;
  public byte[] d;
  public String e;
  public Integer f;
  public Float g;
  public Double h;
  public Optional<Integer> i;
  private static final long A_ID = 0L;
  private static final long B_ID = 1L;
  private static final long C_ID = 2L;
  private static final long D_ID = 3L;
  private static final long E_ID = 4L;
  private static final long F_ID = 5L;
  private static final long G_ID = 6L;
  private static final long H_ID = 7L;
  private static final long I_ID = 8L;

  public UnitTestingClusterSimpleStruct(
    Integer a,
    Boolean b,
    Integer c,
    byte[] d,
    String e,
    Integer f,
    Float g,
    Double h,
    Optional<Integer> i
  ) {
    this.a = a;
    this.b = b;
    this.c = c;
    this.d = d;
    this.e = e;
    this.f = f;
    this.g = g;
    this.h = h;
    this.i = i;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(A_ID, new UIntType(a)));
    values.add(new StructElement(B_ID, new BooleanType(b)));
    values.add(new StructElement(C_ID, new UIntType(c)));
    values.add(new StructElement(D_ID, new ByteArrayType(d)));
    values.add(new StructElement(E_ID, new StringType(e)));
    values.add(new StructElement(F_ID, new UIntType(f)));
    values.add(new StructElement(G_ID, new FloatType(g)));
    values.add(new StructElement(H_ID, new DoubleType(h)));
    values.add(new StructElement(I_ID, i.<BaseTLVType>map((nonOptionali) -> new UIntType(nonOptionali)).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static UnitTestingClusterSimpleStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer a = null;
    Boolean b = null;
    Integer c = null;
    byte[] d = null;
    String e = null;
    Integer f = null;
    Float g = null;
    Double h = null;
    Optional<Integer> i = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == A_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          a = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == B_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          b = castingValue.value(Boolean.class);
        }
      } else if (element.contextTagNum() == C_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          c = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == D_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.ByteArray) {
          ByteArrayType castingValue = element.value(ByteArrayType.class);
          d = castingValue.value(byte[].class);
        }
      } else if (element.contextTagNum() == E_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          e = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == F_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          f = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == G_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Float) {
          FloatType castingValue = element.value(FloatType.class);
          g = castingValue.value(Float.class);
        }
      } else if (element.contextTagNum() == H_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Double) {
          DoubleType castingValue = element.value(DoubleType.class);
          h = castingValue.value(Double.class);
        }
      } else if (element.contextTagNum() == I_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          i = Optional.of(castingValue.value(Integer.class));
        }
      }
    }
    return new UnitTestingClusterSimpleStruct(
      a,
      b,
      c,
      d,
      e,
      f,
      g,
      h,
      i
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("UnitTestingClusterSimpleStruct {\n");
    output.append("\ta: ");
    output.append(a);
    output.append("\n");
    output.append("\tb: ");
    output.append(b);
    output.append("\n");
    output.append("\tc: ");
    output.append(c);
    output.append("\n");
    output.append("\td: ");
    output.append(Arrays.toString(d));
    output.append("\n");
    output.append("\te: ");
    output.append(e);
    output.append("\n");
    output.append("\tf: ");
    output.append(f);
    output.append("\n");
    output.append("\tg: ");
    output.append(g);
    output.append("\n");
    output.append("\th: ");
    output.append(h);
    output.append("\n");
    output.append("\ti: ");
    output.append(i);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class UnitTestingClusterTestFabricScoped {
  public Integer fabricSensitiveInt8u;
  public Optional<Integer> optionalFabricSensitiveInt8u;
  public @Nullable Integer nullableFabricSensitiveInt8u;
  public @Nullable Optional<Integer> nullableOptionalFabricSensitiveInt8u;
  public String fabricSensitiveCharString;
  public ChipStructs.UnitTestingClusterSimpleStruct fabricSensitiveStruct;
  public ArrayList<Integer> fabricSensitiveInt8uList;
  public Integer fabricIndex;
  private static final long FABRIC_SENSITIVE_INT8U_ID = 1L;
  private static final long OPTIONAL_FABRIC_SENSITIVE_INT8U_ID = 2L;
  private static final long NULLABLE_FABRIC_SENSITIVE_INT8U_ID = 3L;
  private static final long NULLABLE_OPTIONAL_FABRIC_SENSITIVE_INT8U_ID = 4L;
  private static final long FABRIC_SENSITIVE_CHAR_STRING_ID = 5L;
  private static final long FABRIC_SENSITIVE_STRUCT_ID = 6L;
  private static final long FABRIC_SENSITIVE_INT8U_LIST_ID = 7L;
  private static final long FABRIC_INDEX_ID = 254L;

  public UnitTestingClusterTestFabricScoped(
    Integer fabricSensitiveInt8u,
    Optional<Integer> optionalFabricSensitiveInt8u,
    @Nullable Integer nullableFabricSensitiveInt8u,
    @Nullable Optional<Integer> nullableOptionalFabricSensitiveInt8u,
    String fabricSensitiveCharString,
    ChipStructs.UnitTestingClusterSimpleStruct fabricSensitiveStruct,
    ArrayList<Integer> fabricSensitiveInt8uList,
    Integer fabricIndex
  ) {
    this.fabricSensitiveInt8u = fabricSensitiveInt8u;
    this.optionalFabricSensitiveInt8u = optionalFabricSensitiveInt8u;
    this.nullableFabricSensitiveInt8u = nullableFabricSensitiveInt8u;
    this.nullableOptionalFabricSensitiveInt8u = nullableOptionalFabricSensitiveInt8u;
    this.fabricSensitiveCharString = fabricSensitiveCharString;
    this.fabricSensitiveStruct = fabricSensitiveStruct;
    this.fabricSensitiveInt8uList = fabricSensitiveInt8uList;
    this.fabricIndex = fabricIndex;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(FABRIC_SENSITIVE_INT8U_ID, new UIntType(fabricSensitiveInt8u)));
    values.add(new StructElement(OPTIONAL_FABRIC_SENSITIVE_INT8U_ID, optionalFabricSensitiveInt8u.<BaseTLVType>map((nonOptionaloptionalFabricSensitiveInt8u) -> new UIntType(nonOptionaloptionalFabricSensitiveInt8u)).orElse(new EmptyType())));
    values.add(new StructElement(NULLABLE_FABRIC_SENSITIVE_INT8U_ID, nullableFabricSensitiveInt8u != null ? new UIntType(nullableFabricSensitiveInt8u) : new NullType()));
    values.add(new StructElement(NULLABLE_OPTIONAL_FABRIC_SENSITIVE_INT8U_ID, nullableOptionalFabricSensitiveInt8u != null ? nullableOptionalFabricSensitiveInt8u.<BaseTLVType>map((nonOptionalnullableOptionalFabricSensitiveInt8u) -> new UIntType(nonOptionalnullableOptionalFabricSensitiveInt8u)).orElse(new EmptyType()) : new NullType()));
    values.add(new StructElement(FABRIC_SENSITIVE_CHAR_STRING_ID, new StringType(fabricSensitiveCharString)));
    values.add(new StructElement(FABRIC_SENSITIVE_STRUCT_ID, fabricSensitiveStruct.encodeTlv()));
    values.add(new StructElement(FABRIC_SENSITIVE_INT8U_LIST_ID, ArrayType.generateArrayType(fabricSensitiveInt8uList, (elementfabricSensitiveInt8uList) -> new UIntType(elementfabricSensitiveInt8uList))));
    values.add(new StructElement(FABRIC_INDEX_ID, new UIntType(fabricIndex)));

    return new StructType(values);
  }

  public static UnitTestingClusterTestFabricScoped decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer fabricSensitiveInt8u = null;
    Optional<Integer> optionalFabricSensitiveInt8u = Optional.empty();
    @Nullable Integer nullableFabricSensitiveInt8u = null;
    @Nullable Optional<Integer> nullableOptionalFabricSensitiveInt8u = null;
    String fabricSensitiveCharString = null;
    ChipStructs.UnitTestingClusterSimpleStruct fabricSensitiveStruct = null;
    ArrayList<Integer> fabricSensitiveInt8uList = null;
    Integer fabricIndex = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == FABRIC_SENSITIVE_INT8U_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          fabricSensitiveInt8u = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == OPTIONAL_FABRIC_SENSITIVE_INT8U_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          optionalFabricSensitiveInt8u = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == NULLABLE_FABRIC_SENSITIVE_INT8U_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          nullableFabricSensitiveInt8u = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == NULLABLE_OPTIONAL_FABRIC_SENSITIVE_INT8U_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          nullableOptionalFabricSensitiveInt8u = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == FABRIC_SENSITIVE_CHAR_STRING_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          fabricSensitiveCharString = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == FABRIC_SENSITIVE_STRUCT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          fabricSensitiveStruct = ChipStructs.UnitTestingClusterSimpleStruct.decodeTlv(castingValue);
        }
      } else if (element.contextTagNum() == FABRIC_SENSITIVE_INT8U_LIST_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          fabricSensitiveInt8uList = castingValue.map((elementcastingValue) -> elementcastingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == FABRIC_INDEX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          fabricIndex = castingValue.value(Integer.class);
        }
      }
    }
    return new UnitTestingClusterTestFabricScoped(
      fabricSensitiveInt8u,
      optionalFabricSensitiveInt8u,
      nullableFabricSensitiveInt8u,
      nullableOptionalFabricSensitiveInt8u,
      fabricSensitiveCharString,
      fabricSensitiveStruct,
      fabricSensitiveInt8uList,
      fabricIndex
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("UnitTestingClusterTestFabricScoped {\n");
    output.append("\tfabricSensitiveInt8u: ");
    output.append(fabricSensitiveInt8u);
    output.append("\n");
    output.append("\toptionalFabricSensitiveInt8u: ");
    output.append(optionalFabricSensitiveInt8u);
    output.append("\n");
    output.append("\tnullableFabricSensitiveInt8u: ");
    output.append(nullableFabricSensitiveInt8u);
    output.append("\n");
    output.append("\tnullableOptionalFabricSensitiveInt8u: ");
    output.append(nullableOptionalFabricSensitiveInt8u);
    output.append("\n");
    output.append("\tfabricSensitiveCharString: ");
    output.append(fabricSensitiveCharString);
    output.append("\n");
    output.append("\tfabricSensitiveStruct: ");
    output.append(fabricSensitiveStruct);
    output.append("\n");
    output.append("\tfabricSensitiveInt8uList: ");
    output.append(fabricSensitiveInt8uList);
    output.append("\n");
    output.append("\tfabricIndex: ");
    output.append(fabricIndex);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class UnitTestingClusterNullablesAndOptionalsStruct {
  public @Nullable Integer nullableInt;
  public Optional<Integer> optionalInt;
  public @Nullable Optional<Integer> nullableOptionalInt;
  public @Nullable String nullableString;
  public Optional<String> optionalString;
  public @Nullable Optional<String> nullableOptionalString;
  public @Nullable ChipStructs.UnitTestingClusterSimpleStruct nullableStruct;
  public Optional<ChipStructs.UnitTestingClusterSimpleStruct> optionalStruct;
  public @Nullable Optional<ChipStructs.UnitTestingClusterSimpleStruct> nullableOptionalStruct;
  public @Nullable ArrayList<Integer> nullableList;
  public Optional<ArrayList<Integer>> optionalList;
  public @Nullable Optional<ArrayList<Integer>> nullableOptionalList;
  private static final long NULLABLE_INT_ID = 0L;
  private static final long OPTIONAL_INT_ID = 1L;
  private static final long NULLABLE_OPTIONAL_INT_ID = 2L;
  private static final long NULLABLE_STRING_ID = 3L;
  private static final long OPTIONAL_STRING_ID = 4L;
  private static final long NULLABLE_OPTIONAL_STRING_ID = 5L;
  private static final long NULLABLE_STRUCT_ID = 6L;
  private static final long OPTIONAL_STRUCT_ID = 7L;
  private static final long NULLABLE_OPTIONAL_STRUCT_ID = 8L;
  private static final long NULLABLE_LIST_ID = 9L;
  private static final long OPTIONAL_LIST_ID = 10L;
  private static final long NULLABLE_OPTIONAL_LIST_ID = 11L;

  public UnitTestingClusterNullablesAndOptionalsStruct(
    @Nullable Integer nullableInt,
    Optional<Integer> optionalInt,
    @Nullable Optional<Integer> nullableOptionalInt,
    @Nullable String nullableString,
    Optional<String> optionalString,
    @Nullable Optional<String> nullableOptionalString,
    @Nullable ChipStructs.UnitTestingClusterSimpleStruct nullableStruct,
    Optional<ChipStructs.UnitTestingClusterSimpleStruct> optionalStruct,
    @Nullable Optional<ChipStructs.UnitTestingClusterSimpleStruct> nullableOptionalStruct,
    @Nullable ArrayList<Integer> nullableList,
    Optional<ArrayList<Integer>> optionalList,
    @Nullable Optional<ArrayList<Integer>> nullableOptionalList
  ) {
    this.nullableInt = nullableInt;
    this.optionalInt = optionalInt;
    this.nullableOptionalInt = nullableOptionalInt;
    this.nullableString = nullableString;
    this.optionalString = optionalString;
    this.nullableOptionalString = nullableOptionalString;
    this.nullableStruct = nullableStruct;
    this.optionalStruct = optionalStruct;
    this.nullableOptionalStruct = nullableOptionalStruct;
    this.nullableList = nullableList;
    this.optionalList = optionalList;
    this.nullableOptionalList = nullableOptionalList;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(NULLABLE_INT_ID, nullableInt != null ? new UIntType(nullableInt) : new NullType()));
    values.add(new StructElement(OPTIONAL_INT_ID, optionalInt.<BaseTLVType>map((nonOptionaloptionalInt) -> new UIntType(nonOptionaloptionalInt)).orElse(new EmptyType())));
    values.add(new StructElement(NULLABLE_OPTIONAL_INT_ID, nullableOptionalInt != null ? nullableOptionalInt.<BaseTLVType>map((nonOptionalnullableOptionalInt) -> new UIntType(nonOptionalnullableOptionalInt)).orElse(new EmptyType()) : new NullType()));
    values.add(new StructElement(NULLABLE_STRING_ID, nullableString != null ? new StringType(nullableString) : new NullType()));
    values.add(new StructElement(OPTIONAL_STRING_ID, optionalString.<BaseTLVType>map((nonOptionaloptionalString) -> new StringType(nonOptionaloptionalString)).orElse(new EmptyType())));
    values.add(new StructElement(NULLABLE_OPTIONAL_STRING_ID, nullableOptionalString != null ? nullableOptionalString.<BaseTLVType>map((nonOptionalnullableOptionalString) -> new StringType(nonOptionalnullableOptionalString)).orElse(new EmptyType()) : new NullType()));
    values.add(new StructElement(NULLABLE_STRUCT_ID, nullableStruct != null ? nullableStruct.encodeTlv() : new NullType()));
    values.add(new StructElement(OPTIONAL_STRUCT_ID, optionalStruct.<BaseTLVType>map((nonOptionaloptionalStruct) -> nonOptionaloptionalStruct.encodeTlv()).orElse(new EmptyType())));
    values.add(new StructElement(NULLABLE_OPTIONAL_STRUCT_ID, nullableOptionalStruct != null ? nullableOptionalStruct.<BaseTLVType>map((nonOptionalnullableOptionalStruct) -> nonOptionalnullableOptionalStruct.encodeTlv()).orElse(new EmptyType()) : new NullType()));
    values.add(new StructElement(NULLABLE_LIST_ID, nullableList != null ? ArrayType.generateArrayType(nullableList, (elementnullableList) -> new UIntType(elementnullableList)) : new NullType()));
    values.add(new StructElement(OPTIONAL_LIST_ID, optionalList.<BaseTLVType>map((nonOptionaloptionalList) -> ArrayType.generateArrayType(nonOptionaloptionalList, (elementnonOptionaloptionalList) -> new UIntType(elementnonOptionaloptionalList))).orElse(new EmptyType())));
    values.add(new StructElement(NULLABLE_OPTIONAL_LIST_ID, nullableOptionalList != null ? nullableOptionalList.<BaseTLVType>map((nonOptionalnullableOptionalList) -> ArrayType.generateArrayType(nonOptionalnullableOptionalList, (elementnonOptionalnullableOptionalList) -> new UIntType(elementnonOptionalnullableOptionalList))).orElse(new EmptyType()) : new NullType()));

    return new StructType(values);
  }

  public static UnitTestingClusterNullablesAndOptionalsStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    @Nullable Integer nullableInt = null;
    Optional<Integer> optionalInt = Optional.empty();
    @Nullable Optional<Integer> nullableOptionalInt = null;
    @Nullable String nullableString = null;
    Optional<String> optionalString = Optional.empty();
    @Nullable Optional<String> nullableOptionalString = null;
    @Nullable ChipStructs.UnitTestingClusterSimpleStruct nullableStruct = null;
    Optional<ChipStructs.UnitTestingClusterSimpleStruct> optionalStruct = Optional.empty();
    @Nullable Optional<ChipStructs.UnitTestingClusterSimpleStruct> nullableOptionalStruct = null;
    @Nullable ArrayList<Integer> nullableList = null;
    Optional<ArrayList<Integer>> optionalList = Optional.empty();
    @Nullable Optional<ArrayList<Integer>> nullableOptionalList = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == NULLABLE_INT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          nullableInt = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == OPTIONAL_INT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          optionalInt = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == NULLABLE_OPTIONAL_INT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          nullableOptionalInt = Optional.of(castingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == NULLABLE_STRING_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          nullableString = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == OPTIONAL_STRING_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          optionalString = Optional.of(castingValue.value(String.class));
        }
      } else if (element.contextTagNum() == NULLABLE_OPTIONAL_STRING_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          nullableOptionalString = Optional.of(castingValue.value(String.class));
        }
      } else if (element.contextTagNum() == NULLABLE_STRUCT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          nullableStruct = ChipStructs.UnitTestingClusterSimpleStruct.decodeTlv(castingValue);
        }
      } else if (element.contextTagNum() == OPTIONAL_STRUCT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          optionalStruct = Optional.of(ChipStructs.UnitTestingClusterSimpleStruct.decodeTlv(castingValue));
        }
      } else if (element.contextTagNum() == NULLABLE_OPTIONAL_STRUCT_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          nullableOptionalStruct = Optional.of(ChipStructs.UnitTestingClusterSimpleStruct.decodeTlv(castingValue));
        }
      } else if (element.contextTagNum() == NULLABLE_LIST_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          nullableList = castingValue.map((elementcastingValue) -> elementcastingValue.value(Integer.class));
        }
      } else if (element.contextTagNum() == OPTIONAL_LIST_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          optionalList = Optional.of(castingValue.map((elementcastingValue) -> elementcastingValue.value(Integer.class)));
        }
      } else if (element.contextTagNum() == NULLABLE_OPTIONAL_LIST_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          nullableOptionalList = Optional.of(castingValue.map((elementcastingValue) -> elementcastingValue.value(Integer.class)));
        }
      }
    }
    return new UnitTestingClusterNullablesAndOptionalsStruct(
      nullableInt,
      optionalInt,
      nullableOptionalInt,
      nullableString,
      optionalString,
      nullableOptionalString,
      nullableStruct,
      optionalStruct,
      nullableOptionalStruct,
      nullableList,
      optionalList,
      nullableOptionalList
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("UnitTestingClusterNullablesAndOptionalsStruct {\n");
    output.append("\tnullableInt: ");
    output.append(nullableInt);
    output.append("\n");
    output.append("\toptionalInt: ");
    output.append(optionalInt);
    output.append("\n");
    output.append("\tnullableOptionalInt: ");
    output.append(nullableOptionalInt);
    output.append("\n");
    output.append("\tnullableString: ");
    output.append(nullableString);
    output.append("\n");
    output.append("\toptionalString: ");
    output.append(optionalString);
    output.append("\n");
    output.append("\tnullableOptionalString: ");
    output.append(nullableOptionalString);
    output.append("\n");
    output.append("\tnullableStruct: ");
    output.append(nullableStruct);
    output.append("\n");
    output.append("\toptionalStruct: ");
    output.append(optionalStruct);
    output.append("\n");
    output.append("\tnullableOptionalStruct: ");
    output.append(nullableOptionalStruct);
    output.append("\n");
    output.append("\tnullableList: ");
    output.append(nullableList);
    output.append("\n");
    output.append("\toptionalList: ");
    output.append(optionalList);
    output.append("\n");
    output.append("\tnullableOptionalList: ");
    output.append(nullableOptionalList);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class UnitTestingClusterNestedStruct {
  public Integer a;
  public Boolean b;
  public ChipStructs.UnitTestingClusterSimpleStruct c;
  public Optional<ChipStructs.UnitTestingClusterTestGlobalStruct> d;
  private static final long A_ID = 0L;
  private static final long B_ID = 1L;
  private static final long C_ID = 2L;
  private static final long D_ID = 3L;

  public UnitTestingClusterNestedStruct(
    Integer a,
    Boolean b,
    ChipStructs.UnitTestingClusterSimpleStruct c,
    Optional<ChipStructs.UnitTestingClusterTestGlobalStruct> d
  ) {
    this.a = a;
    this.b = b;
    this.c = c;
    this.d = d;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(A_ID, new UIntType(a)));
    values.add(new StructElement(B_ID, new BooleanType(b)));
    values.add(new StructElement(C_ID, c.encodeTlv()));
    values.add(new StructElement(D_ID, d.<BaseTLVType>map((nonOptionald) -> nonOptionald.encodeTlv()).orElse(new EmptyType())));

    return new StructType(values);
  }

  public static UnitTestingClusterNestedStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer a = null;
    Boolean b = null;
    ChipStructs.UnitTestingClusterSimpleStruct c = null;
    Optional<ChipStructs.UnitTestingClusterTestGlobalStruct> d = Optional.empty();
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == A_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          a = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == B_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          b = castingValue.value(Boolean.class);
        }
      } else if (element.contextTagNum() == C_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          c = ChipStructs.UnitTestingClusterSimpleStruct.decodeTlv(castingValue);
        }
      } else if (element.contextTagNum() == D_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          d = Optional.of(ChipStructs.UnitTestingClusterTestGlobalStruct.decodeTlv(castingValue));
        }
      }
    }
    return new UnitTestingClusterNestedStruct(
      a,
      b,
      c,
      d
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("UnitTestingClusterNestedStruct {\n");
    output.append("\ta: ");
    output.append(a);
    output.append("\n");
    output.append("\tb: ");
    output.append(b);
    output.append("\n");
    output.append("\tc: ");
    output.append(c);
    output.append("\n");
    output.append("\td: ");
    output.append(d);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class UnitTestingClusterNestedStructList {
  public Integer a;
  public Boolean b;
  public ChipStructs.UnitTestingClusterSimpleStruct c;
  public ArrayList<ChipStructs.UnitTestingClusterSimpleStruct> d;
  public ArrayList<Long> e;
  public ArrayList<byte[]> f;
  public ArrayList<Integer> g;
  private static final long A_ID = 0L;
  private static final long B_ID = 1L;
  private static final long C_ID = 2L;
  private static final long D_ID = 3L;
  private static final long E_ID = 4L;
  private static final long F_ID = 5L;
  private static final long G_ID = 6L;

  public UnitTestingClusterNestedStructList(
    Integer a,
    Boolean b,
    ChipStructs.UnitTestingClusterSimpleStruct c,
    ArrayList<ChipStructs.UnitTestingClusterSimpleStruct> d,
    ArrayList<Long> e,
    ArrayList<byte[]> f,
    ArrayList<Integer> g
  ) {
    this.a = a;
    this.b = b;
    this.c = c;
    this.d = d;
    this.e = e;
    this.f = f;
    this.g = g;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(A_ID, new UIntType(a)));
    values.add(new StructElement(B_ID, new BooleanType(b)));
    values.add(new StructElement(C_ID, c.encodeTlv()));
    values.add(new StructElement(D_ID, ArrayType.generateArrayType(d, (elementd) -> elementd.encodeTlv())));
    values.add(new StructElement(E_ID, ArrayType.generateArrayType(e, (elemente) -> new UIntType(elemente))));
    values.add(new StructElement(F_ID, ArrayType.generateArrayType(f, (elementf) -> new ByteArrayType(elementf))));
    values.add(new StructElement(G_ID, ArrayType.generateArrayType(g, (elementg) -> new UIntType(elementg))));

    return new StructType(values);
  }

  public static UnitTestingClusterNestedStructList decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Integer a = null;
    Boolean b = null;
    ChipStructs.UnitTestingClusterSimpleStruct c = null;
    ArrayList<ChipStructs.UnitTestingClusterSimpleStruct> d = null;
    ArrayList<Long> e = null;
    ArrayList<byte[]> f = null;
    ArrayList<Integer> g = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == A_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          a = castingValue.value(Integer.class);
        }
      } else if (element.contextTagNum() == B_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Boolean) {
          BooleanType castingValue = element.value(BooleanType.class);
          b = castingValue.value(Boolean.class);
        }
      } else if (element.contextTagNum() == C_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Struct) {
          StructType castingValue = element.value(StructType.class);
          c = ChipStructs.UnitTestingClusterSimpleStruct.decodeTlv(castingValue);
        }
      } else if (element.contextTagNum() == D_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          d = castingValue.map((elementcastingValue) -> ChipStructs.UnitTestingClusterSimpleStruct.decodeTlv(elementcastingValue));
        }
      } else if (element.contextTagNum() == E_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          e = castingValue.map((elementcastingValue) -> elementcastingValue.value(Long.class));
        }
      } else if (element.contextTagNum() == F_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          f = castingValue.map((elementcastingValue) -> elementcastingValue.value(byte[].class));
        }
      } else if (element.contextTagNum() == G_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          g = castingValue.map((elementcastingValue) -> elementcastingValue.value(Integer.class));
        }
      }
    }
    return new UnitTestingClusterNestedStructList(
      a,
      b,
      c,
      d,
      e,
      f,
      g
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("UnitTestingClusterNestedStructList {\n");
    output.append("\ta: ");
    output.append(a);
    output.append("\n");
    output.append("\tb: ");
    output.append(b);
    output.append("\n");
    output.append("\tc: ");
    output.append(c);
    output.append("\n");
    output.append("\td: ");
    output.append(d);
    output.append("\n");
    output.append("\te: ");
    output.append(e);
    output.append("\n");
    output.append("\tf: ");
    output.append(f);
    output.append("\n");
    output.append("\tg: ");
    output.append(g);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class UnitTestingClusterDoubleNestedStructList {
  public ArrayList<ChipStructs.UnitTestingClusterNestedStructList> a;
  private static final long A_ID = 0L;

  public UnitTestingClusterDoubleNestedStructList(
    ArrayList<ChipStructs.UnitTestingClusterNestedStructList> a
  ) {
    this.a = a;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(A_ID, ArrayType.generateArrayType(a, (elementa) -> elementa.encodeTlv())));

    return new StructType(values);
  }

  public static UnitTestingClusterDoubleNestedStructList decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    ArrayList<ChipStructs.UnitTestingClusterNestedStructList> a = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == A_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.Array) {
          ArrayType castingValue = element.value(ArrayType.class);
          a = castingValue.map((elementcastingValue) -> ChipStructs.UnitTestingClusterNestedStructList.decodeTlv(elementcastingValue));
        }
      }
    }
    return new UnitTestingClusterDoubleNestedStructList(
      a
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("UnitTestingClusterDoubleNestedStructList {\n");
    output.append("\ta: ");
    output.append(a);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class UnitTestingClusterTestListStructOctet {
  public Long member1;
  public byte[] member2;
  private static final long MEMBER1_ID = 0L;
  private static final long MEMBER2_ID = 1L;

  public UnitTestingClusterTestListStructOctet(
    Long member1,
    byte[] member2
  ) {
    this.member1 = member1;
    this.member2 = member2;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(MEMBER1_ID, new UIntType(member1)));
    values.add(new StructElement(MEMBER2_ID, new ByteArrayType(member2)));

    return new StructType(values);
  }

  public static UnitTestingClusterTestListStructOctet decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    Long member1 = null;
    byte[] member2 = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == MEMBER1_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          member1 = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == MEMBER2_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.ByteArray) {
          ByteArrayType castingValue = element.value(ByteArrayType.class);
          member2 = castingValue.value(byte[].class);
        }
      }
    }
    return new UnitTestingClusterTestListStructOctet(
      member1,
      member2
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("UnitTestingClusterTestListStructOctet {\n");
    output.append("\tmember1: ");
    output.append(member1);
    output.append("\n");
    output.append("\tmember2: ");
    output.append(Arrays.toString(member2));
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
public static class UnitTestingClusterTestGlobalStruct {
  public String name;
  public @Nullable Long myBitmap;
  public @Nullable Optional<Integer> myEnum;
  private static final long NAME_ID = 0L;
  private static final long MY_BITMAP_ID = 1L;
  private static final long MY_ENUM_ID = 2L;

  public UnitTestingClusterTestGlobalStruct(
    String name,
    @Nullable Long myBitmap,
    @Nullable Optional<Integer> myEnum
  ) {
    this.name = name;
    this.myBitmap = myBitmap;
    this.myEnum = myEnum;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(NAME_ID, new StringType(name)));
    values.add(new StructElement(MY_BITMAP_ID, myBitmap != null ? new UIntType(myBitmap) : new NullType()));
    values.add(new StructElement(MY_ENUM_ID, myEnum != null ? myEnum.<BaseTLVType>map((nonOptionalmyEnum) -> new UIntType(nonOptionalmyEnum)).orElse(new EmptyType()) : new NullType()));

    return new StructType(values);
  }

  public static UnitTestingClusterTestGlobalStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    String name = null;
    @Nullable Long myBitmap = null;
    @Nullable Optional<Integer> myEnum = null;
    for (StructElement element: ((StructType)tlvValue).value()) {
      if (element.contextTagNum() == NAME_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.String) {
          StringType castingValue = element.value(StringType.class);
          name = castingValue.value(String.class);
        }
      } else if (element.contextTagNum() == MY_BITMAP_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          myBitmap = castingValue.value(Long.class);
        }
      } else if (element.contextTagNum() == MY_ENUM_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          myEnum = Optional.of(castingValue.value(Integer.class));
        }
      }
    }
    return new UnitTestingClusterTestGlobalStruct(
      name,
      myBitmap,
      myEnum
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("UnitTestingClusterTestGlobalStruct {\n");
    output.append("\tname: ");
    output.append(name);
    output.append("\n");
    output.append("\tmyBitmap: ");
    output.append(myBitmap);
    output.append("\n");
    output.append("\tmyEnum: ");
    output.append(myEnum);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
}
