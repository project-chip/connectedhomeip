package chip.devicecontroller;

/** Callbacks for getting a device connected with PASE or CASE, depending on the context. */
public interface GetConnectedDeviceCallback {
  void onDeviceConnected(long devicePointer);

  void onConnectionFailure(long nodeId, Exception error);
}
