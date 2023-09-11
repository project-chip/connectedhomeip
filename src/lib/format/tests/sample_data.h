/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
