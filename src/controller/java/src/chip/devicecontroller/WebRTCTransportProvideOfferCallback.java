package chip.devicecontroller;

/** Callback interface for handling the response of the ProvideOffer command. */
public interface WebRTCTransportProvideOfferCallback {
  /**
   * Called when the ProvideOffer command is successfully executed.
   *
   * @param webRTCSessionID The ID of the WebRTC session.
   * @param videoStreamID The ID of the video stream, or null if not provided.
   * @param audioStreamID The ID of the audio stream, or null if not provided.
   */
  void onResponse(int webRTCSessionID, Integer videoStreamID, Integer audioStreamID);

  /**
   * Called when an error occurs during the ProvideOffer command execution.
   *
   * @param errorCode The error code representing the failure reason.
   */
  void onError(int errorCode);
}
