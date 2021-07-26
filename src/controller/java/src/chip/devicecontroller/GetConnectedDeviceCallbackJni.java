package chip.devicecontroller;

/** JNI wrapper callback class for getting a connected device. */
public class GetConnectedDeviceCallbackJni {
  private GetConnectedDeviceCallback wrappedCallback;
  private long callbackHandle;

  public GetConnectedDeviceCallbackJni(GetConnectedDeviceCallback wrappedCallback) {
    this.wrappedCallback = wrappedCallback;
    this.callbackHandle = newCallback(wrappedCallback);
  }

  long getCallbackHandle() {
    return callbackHandle;
  }

  private native long newCallback(GetConnectedDeviceCallback wrappedCallback);

  private native void deleteCallback(long callbackHandle);

  // TODO(#8578): Replace finalizer with PhantomReference.
  @SuppressWarnings("deprecation")
  protected void finalize() throws Throwable {
    super.finalize();

    if (callbackHandle != 0) {
      deleteCallback(callbackHandle);
      callbackHandle = 0;
    }
  }

  /** Callbacks for getting a device connected with PASE or CASE, depending on the context. */
  public interface GetConnectedDeviceCallback {
    void onDeviceConnected(long devicePointer);

    void onConnectionFailure(long nodeId, Exception error);
  }
}
