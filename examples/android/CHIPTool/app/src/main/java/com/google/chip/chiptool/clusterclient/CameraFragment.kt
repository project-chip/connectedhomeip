package com.google.chip.chiptool.clusterclient

import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.os.Environment
import android.util.Base64
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Toast
import androidx.core.content.FileProvider
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import chip.devicecontroller.ChipClusters
import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.ChipStructs
import chip.devicecontroller.WebRTCTransportProvideOfferCallback
import chip.devicecontroller.WebRTCTransportRequestorDelegate
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.R
import com.google.chip.chiptool.databinding.CameraFragmentBinding
import java.io.File
import java.io.FileOutputStream
import java.util.Optional
import kotlin.coroutines.resume
import kotlin.coroutines.resumeWithException
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.suspendCancellableCoroutine
import kotlinx.coroutines.withContext
import org.webrtc.DataChannel
import org.webrtc.DefaultVideoDecoderFactory
import org.webrtc.DefaultVideoEncoderFactory
import org.webrtc.EglBase
import org.webrtc.IceCandidate
import org.webrtc.MediaConstraints
import org.webrtc.MediaStream
import org.webrtc.MediaStreamTrack
import org.webrtc.PeerConnection
import org.webrtc.PeerConnectionFactory
import org.webrtc.RtpReceiver
import org.webrtc.RtpTransceiver
import org.webrtc.SdpObserver
import org.webrtc.SessionDescription
import org.webrtc.VideoTrack

class CameraFragment : Fragment() {
  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController(requireContext())

  private lateinit var scope: CoroutineScope
  private lateinit var addressUpdateFragment: AddressUpdateFragment

  private var _binding: CameraFragmentBinding? = null
  private val binding
    get() = _binding!!

  private var eglBase: EglBase? = null
  private var peerConnectionFactory: PeerConnectionFactory? = null
  private var peerConnection: PeerConnection? = null
  private var remoteVideoTrack: VideoTrack? = null

  private var currentLiveViewVideoStreamId: Int? = null
  private var currentWebRTCSessionId: Int? = null

  private val pendingIceCandidates = mutableListOf<IceCandidate>()

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    _binding = CameraFragmentBinding.inflate(inflater, container, false)
    scope = viewLifecycleOwner.lifecycleScope

    addressUpdateFragment =
      childFragmentManager.findFragmentById(R.id.addressUpdateFragment) as AddressUpdateFragment

    initWebRTC()
    createPeerConnection()

    setupWebRTCTransportRequestor()

    setupLocalLiveViewListeners()
    setupRemoteLiveViewListeners()
    setupSnapshotViewListeners()

    setupRadioGroupListener()

    return binding.root
  }

  private fun setupWebRTCTransportRequestor() {
    deviceController.startWebRTCTransportRequestor(
      object : WebRTCTransportRequestorDelegate {
        override fun onOffer(sessionId: Int, offer: String?): Int {
          Log.d(TAG, "Received WebRTC Offer (Session: $sessionId)")

          offer?.let { setRemoteDescription(it) }
          return 0
        }

        override fun onAnswer(sessionId: Int, answer: String?): Int {
          Log.d(TAG, "Received WebRTC Answer (Session: $sessionId)")

          answer?.let {
            setRemoteDescription(it)

            if (pendingIceCandidates.isNotEmpty()) {
              scope.launch {
                sendIceCandidates(sessionId, pendingIceCandidates.toList())
                pendingIceCandidates.clear()
              }
            }
          }
          return 0
        }

        override fun onIceCandidates(
          sessionId: Int,
          candidates: Array<out chip.devicecontroller.IceCandidate?>?
        ): Int {
          Log.d(
            TAG,
            "Received ICE Candidates (Session: $sessionId, Count: ${candidates?.size ?: 0})"
          )

          candidates?.forEach { matterCandidate ->
            matterCandidate?.let {
              val webrtcCandidate = IceCandidate(it.sdpMid, it.sdpMLineIndex, it.candidate)
              Log.d(TAG, "ICE Candidate : ${it.candidate}, ${it.sdpMid}, ${it.sdpMLineIndex}")
            }
          }

          return 0
        }

        override fun onEnd(sessionId: Int, reason: Int): Int {
          Log.d(TAG, "WebRTC Session Ended (Session: $sessionId, Reason: $reason)")
          peerConnection?.close()
          peerConnection = null
          return 0
        }
      }
    )
  }

  private fun setupLocalLiveViewListeners() {
    binding.liveviewLocalAllocateVideoStreamBtn.setOnClickListener {
      scope.launch { sendAllocateVideoStream() }
    }

    binding.startLiveviewBtn.setOnClickListener { scope.launch { startLiveView() } }

    binding.stopLiveviewBtn.setOnClickListener {
      val streamID =
        binding.liveviewLocalWebRTCSessionIdTextView.text.toString().toUIntOrNull() ?: 0U
      scope.launch { stopLiveView(streamID) }
    }

    binding.liveviewLocalDeallocateVideoStreamBtn.setOnClickListener {
      val streamID = binding.liveviewLocalVideoStreamIdTextView.text.toString().toUIntOrNull() ?: 0U
      scope.launch { sendDeallocateVideoStream(streamID) }
    }
  }

  private suspend fun sendAllocateVideoStream() {
    val devicePtr =
      try {
        ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId)
      } catch (e: IllegalStateException) {
        Log.d(TAG, "getConnectedDevicePointer exception", e)
        scope.launch(Dispatchers.Main) {
          Toast.makeText(requireContext(), "Get DevicePointer fail! : $e", Toast.LENGTH_LONG).show()
        }
        return
      }

    val cluster =
      ChipClusters.CameraAvStreamManagementCluster(devicePtr, addressUpdateFragment.endpointId)

    // https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/9c894019866348d324bf07a8664f56603e4b8537/src/app_clusters/cameras.adoc?plain=1#L139
    val streamUsage = 3U // LiveView

    // https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/9c894019866348d324bf07a8664f56603e4b8537/src/app_clusters/CameraAVStreamManagement.adoc?plain=1#L229
    val videoCodec = 0U // H264

    val minFrameRateWidth = 30
    val maxFrameRateWidth = 120

    val minResolution = ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct(640, 480)
    val maxResolution = ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct(1920, 1080)

    val minBitRate = 10000L
    val maxBitRate = 10000L
    val keyFrameInterval = 4000
    val watermarkEnabled = Optional.of<Boolean>(false)
    val osdEnabled = Optional.of<Boolean>(false)

    cluster.videoStreamAllocate(
      object : ChipClusters.CameraAvStreamManagementCluster.VideoStreamAllocateResponseCallback {
        override fun onSuccess(videoStreamID: Int?) {
          Log.d(TAG, "onSuccess VideoStreamID : $videoStreamID")
          scope.launch(Dispatchers.Main) {
            Toast.makeText(
                requireContext(),
                "videoStreamAllocate Success : $videoStreamID",
                Toast.LENGTH_LONG
              )
              .show()
            binding.liveviewLocalVideoStreamIdTextView.text = videoStreamID?.toString() ?: "0"
            currentLiveViewVideoStreamId = videoStreamID
          }
        }

        override fun onError(error: java.lang.Exception?) {
          Log.d(TAG, "onError exception", error)
          scope.launch(Dispatchers.Main) {
            Toast.makeText(
                requireContext(),
                "videoStreamAllocate Error : $error",
                Toast.LENGTH_LONG
              )
              .show()
          }
        }
      },
      streamUsage.toInt(),
      videoCodec.toInt(),
      minFrameRateWidth,
      maxFrameRateWidth,
      minResolution,
      maxResolution,
      minBitRate,
      maxBitRate,
      keyFrameInterval,
      watermarkEnabled,
      osdEnabled
    )
  }

  private suspend fun startLiveView() {
    // Start WebRTC Offer
    pendingIceCandidates.clear()
    val sessionDescription = createWebRTCOffer()
    setWebRTCLocalDescription(sessionDescription)

    Log.d(TAG, "Local Description set successfully")

    sendProvideOffer(sessionDescription.description)
  }

  private fun sendProvideOffer(offerSdp: String) {
    Log.d(TAG, "Sending ProvideOffer with SDP length: ${offerSdp.length}")

    currentWebRTCSessionId = null
    deviceController.webRTCTransportProvideOffer(
      addressUpdateFragment.deviceId,
      addressUpdateFragment.endpointId,
      currentLiveViewVideoStreamId,
      null,
      offerSdp,
      object : WebRTCTransportProvideOfferCallback {
        override fun onResponse(webRTCSessionID: Int, videoStreamID: Int?, audioStreamID: Int?) {
          Log.d(
            TAG,
            "ProvideOffer onResponse - SessionID: $webRTCSessionID, videoStreamID: $videoStreamID"
          )
          currentWebRTCSessionId = webRTCSessionID
          scope.launch(Dispatchers.Main) {
            binding.liveviewLocalWebRTCSessionIdTextView.text = webRTCSessionID.toString()
          }
        }

        override fun onError(errorCode: Int) {
          Log.e(TAG, "ProvideOffer onError - Code: $errorCode")
        }
      }
    )
  }

  private suspend fun stopLiveView(sessionID: UInt) {
    val devicePtr =
      try {
        ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId)
      } catch (e: IllegalStateException) {
        Log.d(TAG, "getConnectedDevicePointer exception", e)
        scope.launch(Dispatchers.Main) {
          Toast.makeText(requireContext(), "Get DevicePointer fail! : $e", Toast.LENGTH_LONG).show()
        }
        return
      }

    val cluster =
      ChipClusters.WebRTCTransportProviderCluster(devicePtr, addressUpdateFragment.endpointId)

    // https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/9c894019866348d324bf07a8664f56603e4b8537/src/app_clusters/webrtc.adoc?plain=1#L385
    val reason = 2 // UserHangup

    cluster.endSession(
      object : ChipClusters.DefaultClusterCallback {
        override fun onSuccess() {
          Log.d(TAG, "onSuccess")
          scope.launch(Dispatchers.Main) {
            Toast.makeText(requireContext(), "endSession Success", Toast.LENGTH_LONG).show()
          }
        }

        override fun onError(error: java.lang.Exception?) {
          Log.d(TAG, "onError exception", error)
          scope.launch(Dispatchers.Main) {
            Toast.makeText(requireContext(), "endSession Error : $error", Toast.LENGTH_LONG).show()
          }
        }
      },
      sessionID.toInt(),
      reason
    )
  }

  private suspend fun sendDeallocateVideoStream(streamID: UInt) {
    val devicePtr =
      try {
        ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId)
      } catch (e: IllegalStateException) {
        Log.d(TAG, "getConnectedDevicePointer exception", e)
        scope.launch(Dispatchers.Main) {
          Toast.makeText(requireContext(), "Get DevicePointer fail! : $e", Toast.LENGTH_LONG).show()
        }
        return
      }

    val cluster =
      ChipClusters.CameraAvStreamManagementCluster(devicePtr, addressUpdateFragment.endpointId)

    cluster.videoStreamDeallocate(
      object : ChipClusters.DefaultClusterCallback {
        override fun onSuccess() {
          Log.d(TAG, "onSuccess")
          scope.launch(Dispatchers.Main) {
            Toast.makeText(requireContext(), "videoStreamDeallocate Success", Toast.LENGTH_LONG)
              .show()
          }
        }

        override fun onError(error: java.lang.Exception?) {
          Log.d(TAG, "onError exception", error)
          scope.launch(Dispatchers.Main) {
            Toast.makeText(
                requireContext(),
                "videoStreamDeallocate Error : $error",
                Toast.LENGTH_LONG
              )
              .show()
          }
        }
      },
      streamID.toInt()
    )
  }

  private fun setupRemoteLiveViewListeners() {
    binding.createOfferBtn.setOnClickListener {
      scope.launch {
        pendingIceCandidates.clear()
        val sessionDescription = createWebRTCOffer()
        setWebRTCLocalDescription(sessionDescription)
      }
    }

    binding.setAnswerBtn.setOnClickListener {
      val base64AnswerSdp = binding.answerSdpEditText.text.toString()
      if (base64AnswerSdp.isNotBlank()) {
        try {
          val decodedBytes = Base64.decode(base64AnswerSdp, Base64.DEFAULT)
          val decodedSdpStr = String(decodedBytes, Charsets.UTF_8)

          Log.d(TAG, "Decoded Answer SDP:\n$decodedSdpStr")

          setRemoteDescription(decodedSdpStr)
        } catch (e: IllegalArgumentException) {
          Log.e(TAG, "Failed to decode Base64 SDP", e)
          Toast.makeText(requireContext(), "Invalid Base64 format.", Toast.LENGTH_SHORT).show()
        }
      } else {
        Toast.makeText(requireContext(), "Please enter Answer SDP.", Toast.LENGTH_SHORT).show()
      }
    }
  }

  private fun setupSnapshotViewListeners() {
    binding.snapshotAllocateStreamBtn.setOnClickListener {
      scope.launch { sendAllocateSnapshotStream() }
    }

    binding.snapshotCaptureSnapshotBtn.setOnClickListener {
      val streamID = binding.snapshotStreamIdTextView.text.toString().toIntOrNull() ?: 0
      scope.launch { sendCaptureSnapshot(streamID) }
    }

    binding.snapshotDeallocateStreamBtn.setOnClickListener {
      val streamID = binding.snapshotStreamIdTextView.text.toString().toIntOrNull() ?: 0
      scope.launch { sendDeallocateCaptureStream(streamID) }
    }
  }

  private fun setupRadioGroupListener() {
    binding.radioGroupMode.setOnCheckedChangeListener { _, checkedId ->
      when (checkedId) {
        R.id.LiveViewLocalRadioButton -> {
          binding.liveViewLocalScrollView.visibility = View.VISIBLE
          binding.liveViewRemoteScrollView.visibility = View.GONE
          binding.snapshotScrollView.visibility = View.GONE
          binding.videoView.visibility = View.VISIBLE
        }
        R.id.LiveViewRemoteRadioButton -> {
          binding.liveViewLocalScrollView.visibility = View.GONE
          binding.liveViewRemoteScrollView.visibility = View.VISIBLE
          binding.snapshotScrollView.visibility = View.GONE
          binding.videoView.visibility = View.VISIBLE
        }
        R.id.SnapshotRadioButton -> {
          binding.liveViewLocalScrollView.visibility = View.GONE
          binding.liveViewRemoteScrollView.visibility = View.GONE
          binding.snapshotScrollView.visibility = View.VISIBLE
          binding.videoView.visibility = View.GONE
        }
      }
    }
  }

  private fun initWebRTC() {
    eglBase = EglBase.create()

    binding.videoView.init(eglBase?.eglBaseContext, null)
    binding.videoView.setEnableHardwareScaler(true)
    binding.videoView.setMirror(false)

    PeerConnectionFactory.initialize(
      PeerConnectionFactory.InitializationOptions.builder(requireContext())
        .setEnableInternalTracer(true)
        .createInitializationOptions()
    )

    val options = PeerConnectionFactory.Options()
    val defaultVideoEncoderFactory = DefaultVideoEncoderFactory(eglBase?.eglBaseContext, true, true)
    val defaultVideoDecoderFactory = DefaultVideoDecoderFactory(eglBase?.eglBaseContext)

    peerConnectionFactory =
      PeerConnectionFactory.builder()
        .setOptions(options)
        .setVideoEncoderFactory(defaultVideoEncoderFactory)
        .setVideoDecoderFactory(defaultVideoDecoderFactory)
        .createPeerConnectionFactory()
  }

  private fun createPeerConnection() {
    val iceServers =
      listOf(PeerConnection.IceServer.builder("stun:stun.l.google.com:19302").createIceServer())
    val rtcConfig = PeerConnection.RTCConfiguration(iceServers)

    peerConnection = peerConnectionFactory?.createPeerConnection(rtcConfig, peerConnectionObserver)

    peerConnection?.addTransceiver(
      MediaStreamTrack.MediaType.MEDIA_TYPE_VIDEO,
      RtpTransceiver.RtpTransceiverInit(RtpTransceiver.RtpTransceiverDirection.RECV_ONLY)
    )

    peerConnection?.addTransceiver(
      MediaStreamTrack.MediaType.MEDIA_TYPE_AUDIO,
      RtpTransceiver.RtpTransceiverInit(RtpTransceiver.RtpTransceiverDirection.RECV_ONLY)
    )
  }

  private val peerConnectionObserver =
    object : PeerConnection.Observer {
      override fun onSignalingChange(newState: PeerConnection.SignalingState?) {
        Log.d(TAG, "SignalingState: $newState")
        scope.launch(Dispatchers.Main) {
          binding.sessionStateTextView.text = "Signaling State: $newState"
        }
      }

      override fun onIceConnectionChange(newState: PeerConnection.IceConnectionState?) {
        Log.d(TAG, "IceConnectionState: $newState")
        scope.launch(Dispatchers.Main) {
          binding.sessionStateTextView.text = "ICE Connection: $newState"
        }
      }

      override fun onIceConnectionReceivingChange(receiving: Boolean) {}

      override fun onIceGatheringChange(newState: PeerConnection.IceGatheringState?) {
        scope.launch(Dispatchers.Main) {
          binding.iceInfoTextView.text = "ICE Gathering: $newState"

          if (newState == PeerConnection.IceGatheringState.COMPLETE) {
            Toast.makeText(requireContext(), "ICE Gathering Complete.", Toast.LENGTH_SHORT).show()

            val isLocalMode =
              binding.radioGroupMode.checkedRadioButtonId == R.id.LiveViewLocalRadioButton
            if (isLocalMode) {
              val sessionId = currentWebRTCSessionId

              val isAnswerReceived = peerConnection?.remoteDescription != null
              if (sessionId != null && pendingIceCandidates.isNotEmpty()) {
                if (isAnswerReceived) {
                  Log.d(TAG, "Answer already received. Sending ICE Candidates.")
                  sendIceCandidates(sessionId, pendingIceCandidates.toList())
                  pendingIceCandidates.clear()
                } else {
                  Log.d(TAG, "Waiting for Answer. ICE Candidates will be sent later.")
                }
              }
            } else {
              val completeSdp = peerConnection?.localDescription?.description
              if (completeSdp != null) {
                Log.d(TAG, "Complete Offer SDP with ICE Candidates:\n$completeSdp")

                val base64Sdp =
                  Base64.encodeToString(completeSdp.toByteArray(Charsets.UTF_8), Base64.NO_WRAP)
                binding.sdpInfoTextView.text = base64Sdp
              }
            }
          }
        }
      }

      override fun onIceCandidate(candidate: IceCandidate?) {
        if (candidate != null) {
          Log.d(TAG, "New ICE Candidate generated: ${candidate.sdp}")
          pendingIceCandidates.add(candidate)
        }
      }

      override fun onIceCandidatesRemoved(candidates: Array<out IceCandidate>?) {}

      override fun onAddStream(stream: MediaStream?) {}

      override fun onRemoveStream(stream: MediaStream?) {}

      override fun onDataChannel(channel: DataChannel?) {}

      override fun onRenegotiationNeeded() {}

      override fun onAddTrack(receiver: RtpReceiver?, mediaStreams: Array<out MediaStream>?) {
        super.onAddTrack(receiver, mediaStreams)
        val track = receiver?.track()

        if (track is VideoTrack) {
          Log.d(TAG, "Remote Video Track received")
          remoteVideoTrack = track

          scope.launch(Dispatchers.Main) { remoteVideoTrack?.addSink(binding.videoView) }
        }
      }
    }

  private suspend fun createWebRTCOffer(): SessionDescription =
    suspendCancellableCoroutine { continuation ->
      val constraints = MediaConstraints()
      peerConnection?.createOffer(
        object : SdpObserver {
          override fun onCreateSuccess(sessionDescription: SessionDescription) {
            continuation.resume(sessionDescription)
          }

          override fun onSetSuccess() {}

          override fun onCreateFailure(error: String?) {
            continuation.resumeWithException(Exception("Failed to create Offer: $error"))
          }

          override fun onSetFailure(error: String?) {}
        },
        constraints
      )
    }

  private suspend fun setWebRTCLocalDescription(sessionDescription: SessionDescription) =
    suspendCancellableCoroutine { continuation ->
      peerConnection?.setLocalDescription(
        object : SdpObserver {
          override fun onCreateSuccess(sdp: SessionDescription?) {}

          override fun onSetSuccess() {
            continuation.resume(Unit)
          }

          override fun onCreateFailure(error: String?) {}

          override fun onSetFailure(error: String?) {
            continuation.resumeWithException(Exception("Failed to set Local Description: $error"))
          }
        },
        sessionDescription
      )
    }

  private suspend fun sendIceCandidates(sessionId: Int, webrtcCandidates: List<IceCandidate>) {
    if (webrtcCandidates.isEmpty()) return

    val devicePtr =
      try {
        ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId, true)
      } catch (e: IllegalStateException) {
        Log.e(TAG, "getConnectedDevicePointer exception", e)
        return
      }

    val cluster =
      ChipClusters.WebRTCTransportProviderCluster(devicePtr, addressUpdateFragment.endpointId)

    val candidatesArray =
      ArrayList(
        webrtcCandidates.map { webrtcCandidate ->
          ChipStructs.WebRTCTransportProviderClusterICECandidateStruct(
            webrtcCandidate.sdp,
            webrtcCandidate.sdpMid,
            webrtcCandidate.sdpMLineIndex
          )
        }
      )

    Log.d(TAG, "Sending ${candidatesArray.size} ICE Candidates at once")

    cluster.provideICECandidates(
      object : ChipClusters.DefaultClusterCallback {
        override fun onSuccess() {
          Log.d(TAG, "Successfully sent ICE Candidates in batch (Session: $sessionId)")
        }

        override fun onError(error: Exception?) {
          Log.e(TAG, "Failed to send ICE Candidates in batch", error)
        }
      },
      sessionId,
      candidatesArray
    )
  }

  private fun setRemoteDescription(sdp: String) {
    val remoteType =
      if (peerConnection?.localDescription == null) SessionDescription.Type.OFFER
      else SessionDescription.Type.ANSWER
    val sessionDescription = SessionDescription(remoteType, sdp)
    peerConnection?.setRemoteDescription(
      object : SdpObserver {
        override fun onCreateSuccess(sdp: SessionDescription?) {}

        override fun onSetSuccess() {
          Log.e(TAG, "Answer SDP applied successfully!")
          scope.launch(Dispatchers.Main) {
            Toast.makeText(requireContext(), "Answer SDP applied successfully!", Toast.LENGTH_SHORT)
              .show()
          }
        }

        override fun onCreateFailure(error: String?) {}

        override fun onSetFailure(error: String?) {
          Log.e(TAG, "Failed to apply Answer: $error")
          scope.launch(Dispatchers.Main) {
            Toast.makeText(requireContext(), "Failed to apply Answer: $error", Toast.LENGTH_LONG)
              .show()
          }
        }
      },
      sessionDescription
    )
  }

  private suspend fun sendAllocateSnapshotStream() {
    val endpointId = addressUpdateFragment.endpointId
    val devicePtr =
      try {
        ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId)
      } catch (e: IllegalStateException) {
        Log.d(TAG, "getConnectedDevicePointer exception", e)
        return
      }
    val cluster = ChipClusters.CameraAvStreamManagementCluster(devicePtr, endpointId)

    // Set min resolution to 640x480
    val minResolution = ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct(640, 480)
    // Set max resolution to 1920x1080
    val maxResolution = ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct(1920, 1080)

    // 0 means JPEG
    val imageCodec = 0
    // Set max frame rate to 30
    val maxFrameRate = 30
    // Set quality to 1
    val quality = 1

    scope.launch(Dispatchers.Main) {
      Toast.makeText(requireContext(), "Allocating Snapshot Stream...", Toast.LENGTH_SHORT).show()
    }

    cluster.snapshotStreamAllocate(
      object : ChipClusters.CameraAvStreamManagementCluster.SnapshotStreamAllocateResponseCallback {
        override fun onSuccess(snapshotStreamID: Int?) {
          if (snapshotStreamID != null) {
            Log.d(TAG, "Allocate Success. Stream ID: $snapshotStreamID")
            scope.launch(Dispatchers.Main) {
              binding.snapshotStreamIdTextView.text = snapshotStreamID.toString()
            }
          } else {
            scope.launch(Dispatchers.Main) {
              Toast.makeText(
                  requireContext(),
                  "Allocate Failed: Stream ID is null",
                  Toast.LENGTH_SHORT
                )
                .show()
            }
          }
        }

        override fun onError(error: Exception) {
          Log.e(TAG, "Allocate Error", error)
          scope.launch(Dispatchers.Main) {
            Toast.makeText(requireContext(), "Allocate Error: ${error.message}", Toast.LENGTH_SHORT)
              .show()
          }
        }
      },
      imageCodec,
      maxFrameRate,
      minResolution,
      maxResolution,
      quality,
      Optional.of(false), // Set watermarkEnabled to false
      Optional.of(false) // Set OSDEnabled to false
    )
  }

  private suspend fun sendCaptureSnapshot(snapshotStreamID: Int) {
    val endpointId = addressUpdateFragment.endpointId
    val downloadFile =
      createSnapshotFile(
        deviceController.fabricIndex.toUInt(),
        addressUpdateFragment.deviceId.toULong()
      )

    if (downloadFile == null) {
      scope.launch(Dispatchers.Main) {
        Toast.makeText(
            requireContext(),
            getString(R.string.camera_cannot_write_storage),
            Toast.LENGTH_SHORT
          )
          .show()
      }
      return
    }

    val downloadFileOutputStream = withContext(Dispatchers.IO) { FileOutputStream(downloadFile) }

    scope.launch(Dispatchers.Main) {
      Toast.makeText(requireContext(), "Capturing Snapshot...", Toast.LENGTH_SHORT).show()
    }

    val devicePtr =
      try {
        ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId, true)
      } catch (e: IllegalStateException) {
        Log.d(TAG, "getConnectedDevicePointer exception", e)
        scope.launch(Dispatchers.Main) {
          Toast.makeText(requireContext(), "Get DevicePointer fail!", Toast.LENGTH_SHORT).show()
        }
        return
      }

    val cluster = ChipClusters.CameraAvStreamManagementCluster(devicePtr, endpointId)
    // Set resolution to 640x480
    val requestResolution =
      ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct(640, 480)
    cluster.captureSnapshot(
      object : ChipClusters.CameraAvStreamManagementCluster.CaptureSnapshotResponseCallback {
        override fun onSuccess(
          data: ByteArray?,
          imageCodec: Int?,
          resolution: ChipStructs.CameraAvStreamManagementClusterVideoResolutionStruct?
        ) {
          Log.d(TAG, "Capture Snapshot Success! $imageCodec")
          if (data == null) {
            downloadFileOutputStream.close()
            Log.e(TAG, "Capture Snapshot returned null data")
            return
          }
          val bytes =
            data
              ?: run {
                Log.e(TAG, "Capture Snapshot returned null data")
                downloadFileOutputStream.close()
                return
              }
          downloadFileOutputStream.use { it.write(bytes) }
          downloadFileOutputStream.close()
          scope.launch { showNotification(downloadFile) }
        }

        override fun onError(error: java.lang.Exception?) {
          Log.e(TAG, "Capture Snapshot Error", error)
          runCatching { downloadFileOutputStream.close() }
          scope.launch(Dispatchers.Main) {
            Toast.makeText(
                requireContext(),
                "Capture Snapshot Error: ${error?.message ?: "null"}",
                Toast.LENGTH_SHORT
              )
              .show()
          }
        }
      },
      snapshotStreamID,
      requestResolution
    )
  }

  private suspend fun sendDeallocateCaptureStream(snapshotStreamID: Int) {
    val endpointId = addressUpdateFragment.endpointId
    val devicePtr =
      try {
        ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId)
      } catch (e: IllegalStateException) {
        Log.d(TAG, "getConnectedDevicePointer exception", e)
        scope.launch(Dispatchers.Main) {
          Toast.makeText(requireContext(), "Get DevicePointer fail!", Toast.LENGTH_SHORT).show()
        }
        return
      }
    val cluster = ChipClusters.CameraAvStreamManagementCluster(devicePtr, endpointId)

    cluster.snapshotStreamDeallocate(
      object : ChipClusters.DefaultClusterCallback {
        override fun onError(error: Exception) {
          Log.e(TAG, "Deallocate Error for Stream ID $snapshotStreamID", error)
        }

        override fun onSuccess() {
          Log.d(TAG, "Deallocate Success for Stream ID: $snapshotStreamID")
          scope.launch(Dispatchers.Main) { binding.snapshotStreamIdTextView.text = "" }
        }
      },
      snapshotStreamID
    )
  }

  private fun isExternalStorageWritable(): Boolean {
    return Environment.getExternalStorageState() == Environment.MEDIA_MOUNTED
  }

  private fun createSnapshotFile(fabricIndex: UInt, nodeId: ULong): File? {
    if (!isExternalStorageWritable()) {
      return null
    }
    val now = System.currentTimeMillis()
    val fileName = "captureSnapshot_${fabricIndex}_${nodeId}_$now.jpg"
    return File(requireContext().getExternalFilesDir(Environment.DIRECTORY_PICTURES), fileName)
  }

  private fun showNotification(file: File) {
    val intent =
      Intent(Intent.ACTION_VIEW).apply {
        setDataAndType(getFileUri(file), "image/jpeg")
        addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION)
      }

    requireActivity().startActivity(intent)
  }

  private fun getFileUri(file: File): Uri {
    return FileProvider.getUriForFile(
      requireContext(),
      "${requireContext().packageName}.provider",
      file
    )
  }

  override fun onDestroyView() {
    super.onDestroyView()

    try {
      deviceController.finishWebRTCTransportRequestor()
      remoteVideoTrack?.removeSink(binding.videoView)
      peerConnection?.close()
      peerConnection?.dispose()
      peerConnectionFactory?.dispose()
      binding.videoView.release()
      eglBase?.release()
    } catch (e: Exception) {
      e.printStackTrace()
    }

    _binding = null
  }

  companion object {
    private const val TAG = "CameraFragment"

    fun newInstance(): CameraFragment = CameraFragment()
  }
}
