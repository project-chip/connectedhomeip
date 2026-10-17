/*
 *   Copyright (c) 2026 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
package chip.devicecontroller;

/**
 * Data model representing a WebRTC ICE Candidate. This class maps to the C++ IceCandidate struct in
 * the JNI layer.
 */
public class IceCandidate {

  // The actual SDP candidate string.
  private String candidate;

  // The media stream identification.
  private String sdpMid;

  // The index of the media description in the SDP.
  private int sdpMLineIndex;

  /** Constructor used by the JNI layer to instantiate this object. */
  public IceCandidate(String candidate, String sdpMid, int sdpMLineIndex) {
    this.candidate = candidate;
    this.sdpMid = sdpMid;
    this.sdpMLineIndex = sdpMLineIndex;
  }

  public String getCandidate() {
    return candidate;
  }

  public String getSdpMid() {
    return sdpMid;
  }

  public int getSdpMLineIndex() {
    return sdpMLineIndex;
  }
}
