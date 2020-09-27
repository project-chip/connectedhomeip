package com.google.chip.chiptool.commissioner.thread;

import io.openthread.commissioner.ByteArray;

public class CommissionerUtils {

  public static byte[] getByteArray(ByteArray byteArray) {
    byte[] ret = new byte[byteArray.size()];
    for (int i = 0; i < byteArray.size(); ++i) {
      ret[i] = (byte) byteArray.get(i).shortValue();
    }
    return ret;
  }

  public static ByteArray getByteArray(byte[] byteArray) {
    ByteArray ret = new ByteArray();
    for (byte b : byteArray) {
      ret.add((short)(((short)b) & 0xff));
    }
    return ret;
  }

  public static String getHexString(byte[] byteArray) {
    StringBuilder strbuilder = new StringBuilder();

    for (byte b : byteArray) {
      strbuilder.append(String.format("%02x", b));
    }
    return strbuilder.toString();
  }

  public static String getHexString(ByteArray byteArray) {
    return getHexString(getByteArray(byteArray));
  }
}
