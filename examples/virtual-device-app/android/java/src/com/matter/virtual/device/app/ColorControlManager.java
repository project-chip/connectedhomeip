package com.matter.virtual.device.app;

public interface ColorControlManager {
  /*
   * Set up the initial value when device is powered on
   */
  void initAttributeValue(int endpoint);

  void HandleEnhancedColorModeChanged(int value);

  void HandleColorModeChanged(int value);

  void HandleCurrentHueChanged(int value);

  void HandleCurrentSaturationChanged(int value);

  void HandleColorTemperatureChanged(int value);
}
