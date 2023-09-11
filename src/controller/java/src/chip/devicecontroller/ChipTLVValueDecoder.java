/*
 * SPDX-FileCopyrightText: 2020-2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

package chip.devicecontroller;

import chip.devicecontroller.model.ChipAttributePath;
import chip.devicecontroller.model.ChipEventPath;

public class ChipTLVValueDecoder {
  public static native <T> T decodeAttributeValue(ChipAttributePath attributePath, byte[] tlv);

  public static native <T> T decodeEventValue(ChipEventPath eventPath, byte[] tlv);
}
