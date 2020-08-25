/*
 *   Copyright (c) 2020 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
package chip.devicecontroller;

/** Controller to interact with the CHIP device. */
public class ChipDeviceController {

  private long deviceControllerPtr;
  private CompletionListener completionListener;

  public ChipDeviceController() {
    deviceControllerPtr = newDeviceController();
  }

  public void setCompletionListener(CompletionListener listener) {
    completionListener = listener;
  }

  public void beginConnectDevice(String ipAddress) {
    beginConnectDevice(deviceControllerPtr, ipAddress);
  }

  public boolean isConnected() {
    return isConnected(deviceControllerPtr);
  }

  public void beginSendMessage(String message) {
    beginSendMessage(deviceControllerPtr, message);
  }

  public void beginSendCommand(ChipCommandType command) {
    beginSendCommand(deviceControllerPtr, command);
  }

  public boolean disconnectDevice() {
    return disconnectDevice(deviceControllerPtr);
  }

  public void onConnectDeviceComplete() {
    completionListener.onConnectDeviceComplete();
  }

  public void onSendMessageComplete(String message) {
    completionListener.onSendMessageComplete(message);
  }

  public void onError(Throwable error) {
    completionListener.onError(error);
  }

  private native long newDeviceController();

  private native void beginConnectDevice(long deviceControllerPtr, String ipAddress);

  private native boolean isConnected(long deviceControllerPtr);

  private native void beginSendMessage(long deviceControllerPtr, String message);

  private native void beginSendCommand(long deviceControllerPtr, ChipCommandType command);

  private native boolean disconnectDevice(long deviceControllerPtr);

  private native void deleteDeviceController(long deviceControllerPtr);

  static {
    System.loadLibrary("CHIPController");
  }

  protected void finalize() throws Throwable {
    super.finalize();

    if (deviceControllerPtr != 0) {
      deleteDeviceController(deviceControllerPtr);
      deviceControllerPtr = 0;
    }
  }

  /** Interface to listen for callbacks from CHIPDeviceController. */
  public interface CompletionListener {

    /** Notifies the completion of "ConnectDevice" command. */
    void onConnectDeviceComplete();

    /** Notifies the completion of "SendMessage" echo command. */
    void onSendMessageComplete(String message);

    /** Notifies the listener of the error. */
    void onError(Throwable error);
  }
}
