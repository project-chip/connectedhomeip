/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
#include <lib/support/Span.h>
#include <protocols/Protocols.h>

namespace chip {
namespace TestData {

struct SamplePayload
{
    Protocols::Id protocolId;
    uint8_t messageType;
    ByteSpan payload;
};

extern const SamplePayload secure_channel_mrp_ack;
extern const SamplePayload secure_channel_pkbdf_param_request;
extern const SamplePayload secure_channel_pkbdf_param_response;
extern const SamplePayload secure_channel_pase_pake1;
extern const SamplePayload secure_channel_pase_pake2;
extern const SamplePayload secure_channel_pase_pake3;
extern const SamplePayload secure_channel_status_report;

extern const SamplePayload im_protocol_read_request;
extern const SamplePayload im_protocol_report_data;
extern const SamplePayload im_protocol_invoke_request;
extern const SamplePayload im_protocol_invoke_response;

// different data reports for content tests
extern const SamplePayload im_protocol_report_data_acl;
extern const SamplePayload im_protocol_report_data_window_covering;
extern const SamplePayload im_protocol_invoke_request_change_channel;

// different event reports
extern const SamplePayload im_protocol_event_software_fault;
extern const SamplePayload im_protocol_event_multipress;

} // namespace TestData
} // namespace chip
