package chip.devicecontroller;

/** Callback interface for handling the response of the SolicitOffer command. */
public interface WebRTCTransportSolicitOfferCallback {
  /**
   * Called when the SolicitOffer command is successfully executed.
   *
   * @param webRTCSessionID The ID of the WebRTC session.
   * @param deferredOffer Indicates whether the offer is deferred.
   * @param videoStreamID The ID of the video stream, or null if not provided.
   * @param audioStreamID The ID of the audio stream, or null if not provided.
   */
  void onResponse(
      int webRTCSessionID, boolean deferredOffer, Integer videoStreamID, Integer audioStreamID);

  /**
   * Called when an error occurs during the SolicitOffer command execution.
   *
   * @param errorCode The error code representing the failure reason.
   */
  void onError(int errorCode);
}
