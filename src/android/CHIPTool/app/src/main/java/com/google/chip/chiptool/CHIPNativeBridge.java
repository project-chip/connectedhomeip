package com.google.chip.chiptool;

// Manages a single instance of CHIPDeviceController
// TODO: Rename this class to CHIPController when JNI ChipDeviceController API is available.
public class CHIPNativeBridge {

  // TODO: Add instance ref for CHIPDeviceController here

  static {
    System.loadLibrary("native-lib");
  }

  private static CHIPNativeBridge instance = null;

  private CHIPNativeBridge() {
    // TODO: Construct CHIPDeviceController here and initialize callbacks
  }

  public static CHIPNativeBridge getInstance() {
    if (instance == null) instance = new CHIPNativeBridge();
    return instance;
  }

  // TODO: Sample function to demonstrate end-to-end calls work. Remove this after JNI layer
  // becomes available.
  public native String base41Encode();

  public boolean isConnected() {
    return false;
  }

  // Human readable connection info for display purposes.
  public String getConnectionInfo() {
    if (!isConnected()) {
      return "NO CONNECTION";
    }

    // TODO: Get from managed ChipDeviceController
    return "127.0.0.1";
  }

  // TODO Add connection management methods here
}
