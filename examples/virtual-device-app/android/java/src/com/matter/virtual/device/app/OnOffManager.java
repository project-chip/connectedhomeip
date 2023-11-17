package com.matter.virtual.device.app;

public interface OnOffManager {

  /** initialize OnOff value by DeviceApp */
  void initAttributeValue();

  /**
   * Notify that the OnOff value be changed by matter and should effect it. Note, set by DeviceApp
   * will also trigger this function, so must check if value is same
   *
   * @param value
   */
  void handleOnOffChanged(boolean value);
}
