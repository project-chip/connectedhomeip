package chip.devicecontroller;

/**
 * Delegate for device attestation verifiers.
 *
 * <p>If DeviceAttestationDelegate is implemented, then onDeviceAttestationCompleted will always be
 * called when device attestation completes.
 *
 * <p>If device attestation fails, {@link ChipDeviceController#continueCommissioning(long, boolean)}
 * is expected to be called to continue or stop the commissioning.
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
  /**
   * The callback will be invoked when device attestation completed with device info for additional
   * verification.
   *
   * <p>This allows the callback to stop commissioning after examining the device info (DAC, PAI,
   * CD).
   *
   * @param devicePtr Handle of device being commissioned
   * @param attestationInfo Attestation information for the device, null is errorCode is 0.
   * @param errorCode Error code on attestation failure. 0 if succeed.
   */
  void onDeviceAttestationCompleted(
      long devicePtr, AttestationInfo attestationInfo, long errorCode);
}
