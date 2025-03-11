// Copyright (c) 2020 Project CHIP Authors

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

// http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "libwebrtc.h"
#include "WebrtcObserver.h"
#include "callbacks.h"
#include "FakeVideoTrackSource.h"

#include <iostream>
#include <memory>
#include <queue>
#include <string>
#include <thread>
#include <sstream>
#include <map>

#include <api/audio_codecs/builtin_audio_decoder_factory.h>
#include <api/audio_codecs/builtin_audio_encoder_factory.h>
#include <api/video_codecs/builtin_video_decoder_factory.h>
#include <api/video_codecs/builtin_video_encoder_factory.h>
#include <api/peer_connection_interface.h>
#include "api/create_peerconnection_factory.h"
#include "api/video_codecs/video_decoder_factory.h"
#include "api/video_codecs/video_decoder_factory_template.h"
#include "api/video_codecs/video_decoder_factory_template_dav1d_adapter.h"
#include "api/video_codecs/video_decoder_factory_template_libvpx_vp8_adapter.h"
#include "api/video_codecs/video_decoder_factory_template_libvpx_vp9_adapter.h"
#include "api/video_codecs/video_decoder_factory_template_open_h264_adapter.h"
#include "api/video_codecs/video_encoder_factory.h"
#include "api/video_codecs/video_encoder_factory_template.h"
#include "api/video_codecs/video_encoder_factory_template_libaom_av1_adapter.h"
#include "api/video_codecs/video_encoder_factory_template_libvpx_vp8_adapter.h"
#include "api/video_codecs/video_encoder_factory_template_libvpx_vp9_adapter.h"
#include "api/video_codecs/video_encoder_factory_template_open_h264_adapter.h"
#include "api/stats/rtc_stats_report.h"
#include "api/stats/rtc_stats_collector_callback.h"
#include "pc/video_track_source.h"
#include <rtc_base/physical_socket_server.h>
#include <rtc_base/ssl_adapter.h>
#include <rtc_base/thread.h>
#include <rtc_base/logging.h>
#include <api/rtc_error.h>
#include <api/jsep.h>
#include <rtc_base/ref_counted_object.h>
#include "modules/video_capture/video_capture.h"
#include "modules/video_capture/video_capture_factory.h"
#include "api/video/video_frame.h"
#include "api/video/video_source_interface.h"
#include "media/base/video_adapter.h"
#include "media/base/video_broadcaster.h"
#include "rtc_base/checks.h"
#include "rtc_base/rtc_certificate_generator.h"
#include "rtc_base/synchronization/mutex.h"
#include "api/video/i420_buffer.h"
#include "api/video/video_frame_buffer.h"
#include "api/video/video_rotation.h"
#include "absl/memory/memory.h"
#include "absl/types/optional.h"
#include "rtc_base/strings/json.h"


namespace pywebrtc
{

  const char kCandidateSdpMidName[] = "sdpMid";
  const char kCandidateSdpMlineIndexName[] = "sdpMLineIndex";
  const char kCandidateSdpName[] = "candidate";

  const char kAudioLabel[] = "audio_label";
  const char kVideoLabel[] = "video_label";
  const char kStreamId[] = "stream_id";

  ClientStatus::ClientStatus(int client_id){
	id = client_id;
	LocalSDPState = OfferWaiting; // OfferWaiting
	RemoteSDPState = AnswerWaiting; // AnswerWaiting
  }
  

  void set_callbacks(void* Client, SdpOfferCallback offer_callback, SdpAnswerCallback answer_callback, IceCallback ice_callback, ErrorCallback error_callback, PeerConnectedCallback peer_connected_callback, PeerDisconnectedCallback peer_disconnected_callback, StatsCollectedCallback stats_callback)
      {
       RTCClient* client = (RTCClient*) Client;
       client->offer_cb = offer_callback;
       client->answer_cb = answer_callback;
       client->ice_cb = ice_callback;
       client->error_cb = error_callback;
       client->peer_connected_cb = peer_connected_callback;
       client->peer_disconnected_cb = peer_disconnected_callback;
       client->stats_cb = stats_callback;
      }
  
  void AddTracks(void* Client) {
    RTCClient* client = (RTCClient*) Client;

    if(client == nullptr){
       RTC_LOG(LS_ERROR) << "[LIBWEBRTC] Invalid client";
       return;
    }

    rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track(
        client->peer_connection_factory->CreateAudioTrack(
            kAudioLabel,
            client->peer_connection_factory->CreateAudioSource(cricket::AudioOptions())
                .get()));
    auto result_or_error = client->webrtc_observer->peer_connection->AddTrack(audio_track, {kStreamId});
    if (!result_or_error.ok()) {
      RTC_LOG(LS_ERROR) << "[LIBWEBRTC] Failed to add audio track to PeerConnection: "
                        << result_or_error.error().message();
    }
 
    rtc::scoped_refptr<FakeVideoTrackSource> video_device =
        FakeVideoTrackSource::Create();
    if (video_device) {
      rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track_(
          client->peer_connection_factory->CreateVideoTrack(video_device, kVideoLabel));

     result_or_error = client->webrtc_observer->peer_connection->AddTrack(video_track_, {kStreamId});
     if (!result_or_error.ok()) {
        RTC_LOG(LS_ERROR) << "[LIBWEBRTC] Failed to add video track to PeerConnection: "
                          << result_or_error.error().message();
      }
    } else {
      RTC_LOG(LS_ERROR) << "[LIBWEBRTC] OpenVideoCaptureDevice failed";
    }
 
  }

  void GetStats(void* Client)
  {
        RTCClient* client = (RTCClient*) Client;
        if(client == nullptr){
       		RTC_LOG(LS_ERROR) << "[LIBWEBRTC] Invalid client";
       		return;
    	}

	if (client->webrtc_observer->peer_connection != nullptr)
    		client->webrtc_observer->peer_connection->GetStats(client->pc_stats_collector_.get());
  }

  void CreateOffer(void* Client)
  {
    RTCClient* client = (RTCClient*) Client;
    if(client == nullptr){
       RTC_LOG(LS_ERROR) << "[LIBWEBRTC] Invalid client";
       return;
    } 
    client->webrtc_observer->peer_connection->CreateOffer(client->webrtc_observer.get()/*CreateSDPObserver.get()*/, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
  }

  void CreateAnswer(void* Client, const std::string& offer, std::function<void(std::string)> callback, int index) {

	  //to be implemented later
  }

  void SetAnswer(void* Client, const std::string& answer) {
    RTCClient* client = (RTCClient*) Client;
    if(client == nullptr){
       RTC_LOG(LS_ERROR) << "[LIBWEBRTC] Invalid client";
       return;
    }
    webrtc::SdpParseError error;
    std::unique_ptr<webrtc::SessionDescriptionInterface> session_description(
        webrtc::CreateSessionDescription("answer", answer, &error));
    if (session_description == nullptr) {
      RTC_LOG(LS_ERROR)<<"[LIBWEBRTC] Answer unable to set";
      return;
    }
    client->webrtc_observer->peer_connection->SetRemoteDescription(
            client->webrtc_observer->SetSDPObserver.get()/*.SetSDPObserver.get()*/, session_description.release());
    
  }

  const char* get_local_sdp(void* Client){
  	RTCClient* client = (RTCClient*) Client;
    	if(client == nullptr){
       		RTC_LOG(LS_ERROR) << "[LIBWEBRTC] Invalid client";
       		return "Invalid client";
    	}

	const webrtc::SessionDescriptionInterface* local_desc = client->webrtc_observer->peer_connection->local_description();
  	if (!local_desc) {
        	RTC_LOG(LS_ERROR) << "[LIBWEBRTC] Local description not set";
		if(client->error_cb){
			client->error_cb("Local description not set", client->client_id);
		}
		else{
			RTC_LOG(LS_ERROR) << "[LIBWEBRTC] Error callback not initialised";
		}
		return "Error in Local description";
    	}

	std::string sdp;
    	local_desc->ToString(&sdp);
    	return sdp.c_str();
  }

  void SetCandidate(void* Client, const std::string& candidates) {
    RTCClient* client = (RTCClient*) Client;
    if(client == nullptr){
       RTC_LOG(LS_ERROR) << "[LIBWEBRTC] Invalid client";
       return;    	
    }
    webrtc::SdpParseError error;
    std::unique_ptr<webrtc::IceCandidateInterface> Candidate(
        webrtc::CreateIceCandidate(/*sdpMid*/"0", /* stoi(parsedData["sdpMLineIndex"]) */ 0, candidates, &error));

    RTC_LOG(LS_VERBOSE)<<candidates;

    if (!Candidate.get()) {
      RTC_LOG(LS_WARNING) << "[LIBWEBRTC] Can't parse received candidate message. "
                             "SdpParseError was: "
                          << error.description;
      return;
    }
    if (!client->webrtc_observer->peer_connection->AddIceCandidate(Candidate.get())) {
      RTC_LOG(LS_WARNING) << "[LIBWEBRTC] Failed to apply the received candidate";
      return;
    } 
  }

  void send_data(void* Client, const std::string& data) {
    RTCClient* client = (RTCClient*) Client;
    if(client == nullptr){
       RTC_LOG(LS_ERROR) << "[LIBWEBRTC] Invalid client";
       return;
    }
    webrtc::DataBuffer buffer(rtc::CopyOnWriteBuffer(data.c_str(), data.size()), true);
    client->webrtc_observer->data_channel->Send(buffer);
  }

  void close_peer_connection(void* Client) {
    rtc::CleanupSSL();
    RTCClient* client = (RTCClient*) Client;
    if(client == nullptr){
       RTC_LOG(LS_ERROR) << "[LIBWEBRTC] Invalid client";
       return;
    }
    client->webrtc_observer->peer_connection->Close();
    client->webrtc_observer->peer_connection = nullptr;
    client->webrtc_observer->data_channel = nullptr;
    client->peer_connection_factory = nullptr;
    client->network_thread->Stop();
    client->worker_thread->Stop();
    client->signaling_thread->Stop();
  }

  void destroy_client(void* Client){
  	if(Client == nullptr){
                RTC_LOG(LS_ERROR) << "[LIBWEBRTC] Client already destroyed";
        	return;
	}
	free(Client);
  }

  void* create_webrtc_client(int id){
  	RTCClient* client = new RTCClient(id);

    	if(client == nullptr){
       		RTC_LOG(LS_ERROR) << "[LIBWEBRTC] Failed to create client";
       		return nullptr;
    	}

	return client;
  }

  void initialise_peer_connection(void* Client) {
    rtc::LogMessage::LogToDebug(rtc::LS_INFO);
      RTC_LOG(LS_INFO) << std::this_thread::get_id() << ":"
              << "Main thread";

    RTCClient* client = (RTCClient*) Client;
    if(client == nullptr){
       RTC_LOG(LS_ERROR) << "[LIBWEBRTC] Invalid client";
       return;
    }

    client->network_thread = rtc::Thread::Create();
    client->network_thread->Start();
    client->worker_thread = rtc::Thread::Create();
    client->worker_thread->Start();
    client->signaling_thread = rtc::Thread::CreateWithSocketServer();
    client->signaling_thread->Start();

    webrtc::PeerConnectionInterface::IceServer ice_server;
    ice_server.uri = "stun:stun.l.google.com:19302";

    client->configuration.servers.push_back(ice_server);

    rtc::InitializeSSL();

    client->peer_connection_factory = webrtc::CreatePeerConnectionFactory(
        nullptr, nullptr,
        client->signaling_thread.get(), nullptr /* default_adm */,
        webrtc::CreateBuiltinAudioEncoderFactory(),
        webrtc::CreateBuiltinAudioDecoderFactory(),
        std::make_unique<webrtc::VideoEncoderFactoryTemplate<
            webrtc::LibvpxVp8EncoderTemplateAdapter,
            webrtc::LibvpxVp9EncoderTemplateAdapter,
            webrtc::OpenH264EncoderTemplateAdapter ,
            webrtc::LibaomAv1EncoderTemplateAdapter>>(),
        std::make_unique<webrtc::VideoDecoderFactoryTemplate<
            webrtc::LibvpxVp8DecoderTemplateAdapter,
            webrtc::LibvpxVp9DecoderTemplateAdapter,
            webrtc::OpenH264DecoderTemplateAdapter,
            webrtc::Dav1dDecoderTemplateAdapter>>(),
        nullptr /* audio_mixer */, nullptr /* audio_processing */);


    if (client->peer_connection_factory.get() == nullptr) {
      RTC_LOG(LS_ERROR) << "[LIBWEBRTC] Error on CreatePeerConnectionFactory.";
      return;
    }

    client->configuration.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;

      client->webrtc_observer->peer_connection = 
        client->peer_connection_factory->CreatePeerConnection(
            client->configuration, nullptr, nullptr, client->webrtc_observer.get()/*.PeerObserver*/);
    if (client->webrtc_observer->peer_connection) {
      RTC_LOG(LS_INFO) <<"[LIBWEBRTC] peer connection initialised";
    } else {
      RTC_LOG(LS_ERROR) <<"[LIBWEBRTC] peer connection initialised failed";
      return;
    }

    AddTracks((void*) client);

    return;

  }
}

