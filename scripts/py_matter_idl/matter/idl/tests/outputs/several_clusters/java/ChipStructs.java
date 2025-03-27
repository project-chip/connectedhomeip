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
public static class SecondClusterFabricDescriptorStruct {
  public byte[] rootPublicKey;
  public Integer vendorID;
  public Long fabricID;
  public Long nodeID;
  public String label;
  public Integer fabricIndex;
  private static final long ROOT_PUBLIC_KEY_ID = 1L;
  private static final long VENDOR_ID_ID = 2L;
  private static final long FABRIC_ID_ID = 3L;
  private static final long NODE_ID_ID = 4L;
  private static final long LABEL_ID = 5L;
  private static final long FABRIC_INDEX_ID = 254L;

  public SecondClusterFabricDescriptorStruct(
    byte[] rootPublicKey,
    Integer vendorID,
    Long fabricID,
    Long nodeID,
    String label,
    Integer fabricIndex
  ) {
    this.rootPublicKey = rootPublicKey;
    this.vendorID = vendorID;
    this.fabricID = fabricID;
    this.nodeID = nodeID;
    this.label = label;
    this.fabricIndex = fabricIndex;
  }

  public StructType encodeTlv() {
    ArrayList<StructElement> values = new ArrayList<>();
    values.add(new StructElement(ROOT_PUBLIC_KEY_ID, new ByteArrayType(rootPublicKey)));
    values.add(new StructElement(VENDOR_ID_ID, new UIntType(vendorID)));
    values.add(new StructElement(FABRIC_ID_ID, new UIntType(fabricID)));
    values.add(new StructElement(NODE_ID_ID, new UIntType(nodeID)));
    values.add(new StructElement(LABEL_ID, new StringType(label)));
    values.add(new StructElement(FABRIC_INDEX_ID, new UIntType(fabricIndex)));

    return new StructType(values);
  }

  public static SecondClusterFabricDescriptorStruct decodeTlv(BaseTLVType tlvValue) {
    if (tlvValue == null || tlvValue.type() != TLVType.Struct) {
      return null;
    }
    byte[] rootPublicKey = null;
    Integer vendorID = null;
    Long fabricID = null;
    Long nodeID = null;
    String label = null;
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
      } else if (element.contextTagNum() == FABRIC_INDEX_ID) {
        if (element.value(BaseTLVType.class).type() == TLVType.UInt) {
          UIntType castingValue = element.value(UIntType.class);
          fabricIndex = castingValue.value(Integer.class);
        }
      }
    }
    return new SecondClusterFabricDescriptorStruct(
      rootPublicKey,
      vendorID,
      fabricID,
      nodeID,
      label,
      fabricIndex
    );
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("SecondClusterFabricDescriptorStruct {\n");
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
    output.append("\tfabricIndex: ");
    output.append(fabricIndex);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
}
