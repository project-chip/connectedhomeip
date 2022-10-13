package chip.devicecontroller;

/**
 * Only one of the following delegate callbacks should be implemented.
 * <p>
 * If DeviceAttestationFailureCallback is implemented, then it will be called
 * when device
 * attestation fails, and the client can decide to continue or stop the
 * commissioning.
 * <p>
 * If DeviceAttestationFailureCallback is implemented, then it
 * will always be called when device attestation completes.
 */
public interface DeviceAttestationDelegate {

    public interface DeviceAttestationCompletionCallback extends DeviceAttestationDelegate {
        /**
         * The callback will be invoked when device attestation completed with device
         * info for additional verification.
         * If this callback is implemented, continueCommissioningDevice on
         * {@link ChipDeviceController} is expected
         * to be called if commissioning should continue.
         * <p>
         * This allows the callback to stop commissioning after examining the device
         * info (DAC, PAI, CD).
         *
         * @param deviceControllerPtr Controller corresponding to the commissioning
         *                            process
         * @param devicePtr           Handle of device being commissioned
         * @param attestationInfo     Attestation information for the device
         * @param errorCode           Error code on attestation failure. 0 if success.
         */
        void onDeviceAttestationCompleted(long deviceControllerPtr, long devicePtr, AttestationInfo attestationInfo,
                int errorCode);
    }

    public interface DeviceAttestationFailureCallback extends DeviceAttestationDelegate {
        /**
         * The callback will be invoked when device attestation failed.
         *
         * @param deviceControllerPtr Controller corresponding to the commissioning
         *                            process
         * @param devicePtr           Handle of device being commissioned
         * @param errorCode           Error code for the failure.
         */
        void onDeviceAttestationFailed(long deviceControllerPtr, long devicePtr, int errorCode);
    }
}
