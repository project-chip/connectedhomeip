#pragma once

#include <string>


std::string generateMonotonicPeerConnectionId();
std::string json_escape(const std::string &input);
void webrtc_bridge_message_received_cb(void *data, int len);