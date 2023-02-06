package chip.devicecontroller;

/**
 * Only one of the following delegate callbacks should be implemented.
 *
 * <p>If one of the following callbacks is implemented, {@link
 * ChipDeviceController#continueCommissioning(long, boolean)} is expected to be called if
 * commissioning should continue.
 *
 * <p>If DeviceAttestationCompletionCallback is implemented, then it will always be called when
 * device attestation completes.
 *
 * <p>If DeviceAttestationFailureCallback is implemented, then it will be called when device
 * attestation fails, and the client can decide to continue or stop the commissioning.
 *
 * <p>For example:
 *
 * <pre>
 * // Continue commissioning
 * deviceController.continueCommissioning(devicePtr, true)
 *
 * // Stop commissioning
 * deviceController.continueCommissioning(devicePtr, false)
 * </pre>
 */
public interface DeviceAttestationDelegate {

  public interface DeviceAttestationCompletionCallback extends DeviceAttestationDelegate {
    /**
     * The callback will be invoked when device attestation completed with device info for
     * additional verification.
     *
     * <p>This allows the callback to stop commissioning after examining the device info (DAC, PAI,
     * CD).
     *
     * @param devicePtr Handle of device being commissioned
     * @param attestationInfo Attestation information for the device
     * @param errorCode Error code on attestation failure. 0 if success.
     */
    void onDeviceAttestationCompleted(
        long devicePtr, AttestationInfo attestationInfo, int errorCode);
  }

  public interface DeviceAttestationFailureCallback extends DeviceAttestationDelegate {
    /**
     * The callback will be invoked when device attestation failed
     *
     * @param devicePtr Handle of device being commissioned
     * @param errorCode Error code for the failure.
     */
    void onDeviceAttestationFailed(long devicePtr, int errorCode);
  }
}
