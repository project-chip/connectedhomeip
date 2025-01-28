import push_av_server
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main


class TC_PAVS_1_0(MatterBaseTest):
    """
    NOTE: this class is only a guide to understand what APIs I'd need to integrate in the push av server
    for a better integration. It is not designed to be merged nor does it actually run (no chip module).
    """

    def setup_class(self):
        super().setup_class()

        self.av_ctx = push_av_server.PushAvContext("127.0.0.1", 1234, None, "localhost")
        self.av_ctx.start_in_background()

    def teardown_class(self):
        super().teardown_class()
        self.av_ctx.terminate()

    def steps_TC_PAVS_1_0(self):
        return [TestStep(1, "Commissioning, already done", is_commissioning=True),
                TestStep(2, "Install CA onto the device"),
                TestStep(3, "Obtain device CSR, generate cert, provision cert onto device"),
                TestStep(4, "Create media streams"),
                TestStep(5, "Allocate push transport"),
                TestStep(6, "Trigger a recording"),
                TestStep(7, "Deallocate transport")
                ]

    @async_test_body
    async def test_TC_PAVS_1_0(self):
        # commissioning - already done
        self.step(1)

        self.step(2)
        # Access CA cert via the push_av_server package.
        print(self.av_ctx.device_hierarchy.root_cert)
        # read TLSCertificateManagament attributes to validate state
        # Send the TLSCertificateManagament.ProvisionRootCertificate command
        # Assert we got a response that contains a CA id
        # read TLSCertificateManagament attributes to validate state

        self.step(3)

        self.step("3b")
        # Generate nonce
        # send TLSCertificateManagement.TLSClientCSR, receive TLSClientCSRResponse
        print(self.av_ctx.device_hierarchy.gen_cert("device name", "csr"))
        # send ProvisionClientCertificate, receive ProvisionClientCertificateResponse

        self.step(4)
        # (note: assum this step is a requirement and not the focus of these TCs)
        # send VideoStreamAllocate, receive VideoStreamAllocateResponse
        #   StreamType: StreamTypeEnum.Recording
        #   VideoCodec: VideoCodecEnum.H264 (HEVC, VVC, AV1 are all optionals)
        #   MinFrameRate: 0
        #   MaxFrameRate: 60
        #   MinResolution: 0
        #   MaxResolution: 4k
        #   MinBitRate: 0
        #   MaxBitRate: inf
        #   MinFragmentLen: 0
        #   MaxFragmentLen: info
        # send AudioStreamAllocate, receive AudioStreamAllocateResponse
        #   StreamType: StreamTypeEnum.Recording
        #   AudioCodec: AudioCodecEnum.OPUS (AAC-LC is optional)
        #   ChannelCount: 1 (note: or 2? what's the requirements that works for most cameras)
        #   SampleRate: TBD (48, 32, 16khz)
        #   BitRate: TBD
        #   BitDepth: TBD

        self.step(5)
        # send AllocatePushTransport, receive AllocatePushTransportResponse
        #   PushAVStreamTransportOptionsStruct:
        #      video stream id: from step 4
        #      audio stream id: from step 4
        #      tls endpoint id: from step 3b
        #      url: local dns + known path from step 2
        #      triggerOptions: PushAVStreamTransportMotionTriggerTimeControlStruct
        #          InitialDuration: default
        #          AugmentationDuration: default
        #          MaxDuration: default
        #          BlindDuration: default
        #          (note: are we testing this in this test plan or in webrtc?)
        #      containerFormat: PushAVStreamTransportContainerFormatEnum.CMAF (only one at the time)
        #      ingestMethod: PushAVStreamTransportIngestMethodEnum.CMAFIngest (only one at the time)
        #      containerOptions: PushAVStreamTransportContainerOptionsStruct
        #          ContainerType: PushAVStreamTransportContainerFormatEnum.CMAF (only one at the time)
        #          CMAFContainerOptions: PushAVStreamTransportCMAFContainerOptionsStruct
        #              ChunkDuration: default
        #              CENCKey: null. (note: do we test this in the harnes or do we not?)
        #      metadataOptions: PushAVStreamTransportMetadataOptionsStruct
        #          Multiplexing: PushAVStreamTransportStreamMultiplexingEnum.Interleaved
        #          IncludeMotionsZones: false
        #          EnablePrivacySensitive: false
        #      expiryTime: null? Not entirely sure how to test this one yet.

        # find stream config and assert
        # modify stream
        # find stream config and assert

        # set transport status
        # find stream config and assert
        # reset transport status
        # find stream config and assert

        self.step(6)
        # subscribe to PushTransport events (note: forgot if it's required or not, I think it is)
        # send ManuallyTriggerTransport
        #   ConnectionId: from step 5
        #   Action: PushAVStreamTransport_ActionEnum
        #   ActivationReason: PushAVStreamTransportTriggerActivationReasonEnum
        #   MinDuration: 5
        # read
        # listen for PushTransportStart and PushTransportEnd event
        # wait for start event, validate conn id and options
        # wait for end event, validate conn id and options

        # Check metadata of stream sent to our web server
        # ffmpeg convert the cmaf tracks into something more easily read by viewers
        # manual step to inspect the video

        self.step(7)
        # TBD. deallocation logic

    async def test_wrong_cert_chain(self):
        """Intended as an example of how to install an unrecognised """
        wrong_chain = push_av_server.CAHierarchy(push_av_server.wd.mkdir("certs", "wrong"), "wrong chain", "client")

        wrong_chain.root_cert_path  # Install onto the device
        [path_to_key, path_to_cert] = wrong_chain.gen_cert("dns", "csr")  # Sign for the device
        # Install onto the device
        print(f"{path_to_cert}, {path_to_key}")  # to not have unused values

        # After setting up the device, trigger an upload which will use the wrong cert and should fail


if __name__ == "__main__":
    default_matter_test_main()
