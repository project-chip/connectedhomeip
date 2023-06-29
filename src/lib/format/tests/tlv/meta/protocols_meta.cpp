#include <tlv/meta/protocols_meta.h>

namespace chip {
namespace TLVMeta {
namespace {

using namespace chip::FlatTree;
using namespace chip::TLV;

Entry<ItemInfo> _SecureChannelProtocol[] = {
  { { AttributeTag(0), "msg_counter_sync_request"}, kInvalidNodeIndex }, // SecureChannelProtocol::octet_string
  { { AttributeTag(1), "msg_counter_sync_response"}, kInvalidNodeIndex }, // SecureChannelProtocol::octet_string
  { { AttributeTag(16), "mrp_ack"}, kInvalidNodeIndex }, // SecureChannelProtocol::octet_string
  { { AttributeTag(32), "pbkdf_param_request"}, 3 }, // SecureChannelProtocol::PBKDFParamRequest
  { { AttributeTag(33), "pbkdf_param_response"}, 5 }, // SecureChannelProtocol::PBKDFParamResponse
  { { AttributeTag(34), "pase_pake1"}, 6 }, // SecureChannelProtocol::PasePake1
  { { AttributeTag(35), "pase_pake2"}, 7 }, // SecureChannelProtocol::PasePake2
  { { AttributeTag(36), "pase_pake3"}, 8 }, // SecureChannelProtocol::PasePake3
  { { AttributeTag(48), "case_sigma1"}, 9 }, // SecureChannelProtocol::CaseSigma1
  { { AttributeTag(49), "case_sigma2"}, 10 }, // SecureChannelProtocol::CaseSigma2
  { { AttributeTag(50), "case_sigma3"}, 11 }, // SecureChannelProtocol::CaseSigma3
  { { AttributeTag(51), "case_sigma2_resume"}, 12 }, // SecureChannelProtocol::CaseSigma2Resume
  { { AttributeTag(64), "status_report"}, kInvalidNodeIndex }, // SecureChannelProtocol::octet_string
};

Entry<ItemInfo> _SecureChannelProtocol_ICDParameterStruct[] = {
  { { ContextTag(1), "sleepy_idle_interval"}, kInvalidNodeIndex }, // SecureChannelProtocol::int32u
  { { ContextTag(2), "sleepy_active_interval"}, kInvalidNodeIndex }, // SecureChannelProtocol::int32u
};

Entry<ItemInfo> _SecureChannelProtocol_PBKDFParamRequest[] = {
  { { ContextTag(1), "initiator_random"}, kInvalidNodeIndex }, // SecureChannelProtocol::octet_string
  { { ContextTag(2), "initiator_session_id"}, kInvalidNodeIndex }, // SecureChannelProtocol::int16u
  { { ContextTag(3), "passcode_id"}, kInvalidNodeIndex }, // SecureChannelProtocol::int16u
  { { ContextTag(4), "has_pbkdf_parameters"}, kInvalidNodeIndex }, // SecureChannelProtocol::boolean
  { { ContextTag(5), "initiator_icd_params"}, 2 }, // SecureChannelProtocol::ICDParameterStruct
};

Entry<ItemInfo> _SecureChannelProtocol_CryptoPBKDFParameterSet[] = {
  { { ContextTag(1), "iterations"}, kInvalidNodeIndex }, // SecureChannelProtocol::int32u
  { { ContextTag(2), "salt"}, kInvalidNodeIndex }, // SecureChannelProtocol::octet_string
};

Entry<ItemInfo> _SecureChannelProtocol_PBKDFParamResponse[] = {
  { { ContextTag(1), "initiator_random"}, kInvalidNodeIndex }, // SecureChannelProtocol::octet_string
  { { ContextTag(2), "responder_random"}, kInvalidNodeIndex }, // SecureChannelProtocol::octet_string
  { { ContextTag(3), "responder_session_id"}, kInvalidNodeIndex }, // SecureChannelProtocol::int16u
  { { ContextTag(4), "pbkdf_parameters"}, 4 }, // SecureChannelProtocol::CryptoPBKDFParameterSet
  { { ContextTag(5), "responder_icd_params"}, kInvalidNodeIndex }, // SecureChannelProtocol::IDCParameterStruct
};

Entry<ItemInfo> _SecureChannelProtocol_PasePake1[] = {
  { { ContextTag(1), "pA"}, kInvalidNodeIndex }, // SecureChannelProtocol::octet_string
};

Entry<ItemInfo> _SecureChannelProtocol_PasePake2[] = {
  { { ContextTag(1), "pB"}, kInvalidNodeIndex }, // SecureChannelProtocol::octet_string
  { { ContextTag(2), "cB"}, kInvalidNodeIndex }, // SecureChannelProtocol::octet_string
};

Entry<ItemInfo> _SecureChannelProtocol_PasePake3[] = {
  { { ContextTag(1), "cA"}, kInvalidNodeIndex }, // SecureChannelProtocol::octet_string
};

Entry<ItemInfo> _SecureChannelProtocol_CaseSigma1[] = {
  { { ContextTag(1), "initiator_random"}, kInvalidNodeIndex }, // SecureChannelProtocol::octet_string
  { { ContextTag(2), "initiator_session_id"}, kInvalidNodeIndex }, // SecureChannelProtocol::int16u
  { { ContextTag(3), "destination_id"}, kInvalidNodeIndex }, // SecureChannelProtocol::octet_string
  { { ContextTag(4), "initiator_eph_pub_key"}, kInvalidNodeIndex }, // SecureChannelProtocol::octet_string
  { { ContextTag(5), "initiator_icd_params"}, 2 }, // SecureChannelProtocol::ICDParameterStruct
  { { ContextTag(6), "resumption_id"}, kInvalidNodeIndex }, // SecureChannelProtocol::octet_string
  { { ContextTag(7), "initiator_resume_mic"}, kInvalidNodeIndex }, // SecureChannelProtocol::octet_string
};

Entry<ItemInfo> _SecureChannelProtocol_CaseSigma2[] = {
  { { ContextTag(1), "responder_random"}, kInvalidNodeIndex }, // SecureChannelProtocol::octet_string
  { { ContextTag(2), "responder_sessoion_id"}, kInvalidNodeIndex }, // SecureChannelProtocol::int16u
  { { ContextTag(3), "responder_eph_pub_key"}, kInvalidNodeIndex }, // SecureChannelProtocol::octet_string
  { { ContextTag(4), "encrypted2"}, kInvalidNodeIndex }, // SecureChannelProtocol::octet_string
  { { ContextTag(5), "responder_icd_params"}, 2 }, // SecureChannelProtocol::ICDParameterStruct
};

Entry<ItemInfo> _SecureChannelProtocol_CaseSigma3[] = {
  { { ContextTag(1), "encrypted"}, kInvalidNodeIndex }, // SecureChannelProtocol::octet_string
};

Entry<ItemInfo> _SecureChannelProtocol_CaseSigma2Resume[] = {
  { { ContextTag(1), "resumption_id"}, kInvalidNodeIndex }, // SecureChannelProtocol::octet_string
  { { ContextTag(2), "sigma2_resume_mic"}, kInvalidNodeIndex }, // SecureChannelProtocol::octet_string
  { { ContextTag(3), "responder_sessoion_id"}, kInvalidNodeIndex }, // SecureChannelProtocol::int16u
  { { ContextTag(4), "responder_icd_params"}, 2 }, // SecureChannelProtocol::ICDParameterStruct
};

Entry<ItemInfo> _IMProtocol[] = {
  { { AttributeTag(1), "status_response"}, 19 }, // IMProtocol::StatusResponseMessage
  { { AttributeTag(2), "read_request"}, 20 }, // IMProtocol::ReadRequestMessage
  { { AttributeTag(3), "subscribe_request"}, 21 }, // IMProtocol::SubscribeRequestMessage
  { { AttributeTag(4), "subscribe_response"}, 22 }, // IMProtocol::SubscribeResponseMessage
  { { AttributeTag(5), "report_data"}, 30 }, // IMProtocol::ReportDataMessage
  { { AttributeTag(6), "write_request"}, 32 }, // IMProtocol::WriteRequestMessage
  { { AttributeTag(7), "write_response"}, 34 }, // IMProtocol::WriteResponseMessage
  { { AttributeTag(8), "invoke_request"}, 37 }, // IMProtocol::InvokeRequestMessage
  { { AttributeTag(9), "invoke_response"}, 40 }, // IMProtocol::InvokeResponseMessage
  { { AttributeTag(10), "timed_request"}, 41 }, // IMProtocol::TimedRequestMessage
};

Entry<ItemInfo> _IMProtocol_AttributePathIB[] = {
  { { ContextTag(0), "enable_tag_compression"}, kInvalidNodeIndex }, // IMProtocol::boolean
  { { ContextTag(1), "node_id"}, kInvalidNodeIndex }, // IMProtocol::int64u
  { { ContextTag(2), "endpoint_id"}, kInvalidNodeIndex }, // IMProtocol::int16u
  { { ContextTag(3), "cluster_id"}, kInvalidNodeIndex }, // IMProtocol::int32u
  { { ContextTag(4), "attribute_id"}, kInvalidNodeIndex }, // IMProtocol::int32u
  { { ContextTag(5), "list_index"}, kInvalidNodeIndex }, // IMProtocol::int16u
};

Entry<ItemInfo> _IMProtocol_EventPathIB[] = {
  { { ContextTag(0), "node_id"}, kInvalidNodeIndex }, // IMProtocol::int64u
  { { ContextTag(1), "endpoint_id"}, kInvalidNodeIndex }, // IMProtocol::int16u
  { { ContextTag(2), "cluster_id"}, kInvalidNodeIndex }, // IMProtocol::int32u
  { { ContextTag(3), "event_id"}, kInvalidNodeIndex }, // IMProtocol::int32u
  { { ContextTag(4), "is_urgent"}, kInvalidNodeIndex }, // IMProtocol::boolean
};

Entry<ItemInfo> _IMProtocol_EventFilterIB[] = {
  { { ContextTag(0), "node_id"}, kInvalidNodeIndex }, // IMProtocol::int64u
  { { ContextTag(1), "event_min"}, kInvalidNodeIndex }, // IMProtocol::int64u
};

Entry<ItemInfo> _IMProtocol_ClusterPathIB[] = {
  { { ContextTag(0), "node_id"}, kInvalidNodeIndex }, // IMProtocol::int64u
  { { ContextTag(1), "endpoint_id"}, kInvalidNodeIndex }, // IMProtocol::int16u
  { { ContextTag(2), "cluster_id"}, kInvalidNodeIndex }, // IMProtocol::int32u
};

Entry<ItemInfo> _IMProtocol_DataVersionFilterIB[] = {
  { { ContextTag(0), "path"}, 17 }, // IMProtocol::ClusterPathIB
  { { ContextTag(1), "data_version"}, kInvalidNodeIndex }, // IMProtocol::int32u
};

Entry<ItemInfo> _IMProtocol_StatusResponseMessage[] = {
  { { ContextTag(0), "status"}, kInvalidNodeIndex }, // IMProtocol::int8u
  { { ContextTag(255), "interaction_model_revison"}, kInvalidNodeIndex }, // IMProtocol::int8u
};

Entry<ItemInfo> _IMProtocol_ReadRequestMessage[] = {
  { { ContextTag(0), "attribute_requests"}, 50 }, // IMProtocol::AttributePathIB[]
  { { ContextTag(1), "event_requests"}, 46 }, // IMProtocol::EventPathIB[]
  { { ContextTag(2), "event_filters"}, 43 }, // IMProtocol::EventFilterIB[]
  { { ContextTag(3), "fabric_filtered"}, kInvalidNodeIndex }, // IMProtocol::boolean
  { { ContextTag(4), "data_version_filters"}, 45 }, // IMProtocol::DataVersionFilterIB[]
  { { ContextTag(255), "interaction_model_revison"}, kInvalidNodeIndex }, // IMProtocol::int8u
};

Entry<ItemInfo> _IMProtocol_SubscribeRequestMessage[] = {
  { { ContextTag(0), "keep_subscriptions"}, kInvalidNodeIndex }, // IMProtocol::boolean
  { { ContextTag(1), "min_minterval_floor"}, kInvalidNodeIndex }, // IMProtocol::int16u
  { { ContextTag(2), "max_minterval_ceiling"}, kInvalidNodeIndex }, // IMProtocol::int16u
  { { ContextTag(3), "attribute_requests"}, 14 }, // IMProtocol::AttributePathIB
  { { ContextTag(4), "event_requests"}, 15 }, // IMProtocol::EventPathIB
  { { ContextTag(5), "event_filters"}, 16 }, // IMProtocol::EventFilterIB
  { { ContextTag(7), "fabric_filtered"}, kInvalidNodeIndex }, // IMProtocol::boolean
  { { ContextTag(8), "data_version_filters"}, 18 }, // IMProtocol::DataVersionFilterIB
  { { ContextTag(255), "interaction_model_revison"}, kInvalidNodeIndex }, // IMProtocol::int8u
};

Entry<ItemInfo> _IMProtocol_SubscribeResponseMessage[] = {
  { { ContextTag(0), "subscription_id"}, kInvalidNodeIndex }, // IMProtocol::int32u
  { { ContextTag(1), "max_interval"}, kInvalidNodeIndex }, // IMProtocol::int16u
  { { ContextTag(255), "interaction_model_revison"}, kInvalidNodeIndex }, // IMProtocol::int8u
};

Entry<ItemInfo> _IMProtocol_StatusIB[] = {
  { { ContextTag(0), "status"}, kInvalidNodeIndex }, // IMProtocol::int8u
  { { ContextTag(1), "cluster_status"}, kInvalidNodeIndex }, // IMProtocol::int8u
};

Entry<ItemInfo> _IMProtocol_AttributeStatus[] = {
  { { ContextTag(0), "path"}, 14 }, // IMProtocol::AttributePathIB
  { { ContextTag(1), "status"}, 23 }, // IMProtocol::StatusIB
};

Entry<ItemInfo> _IMProtocol_AttributeData[] = {
  { { ContextTag(0), "data_version"}, kInvalidNodeIndex }, // IMProtocol::int32u
  { { ContextTag(1), "path"}, 14 }, // IMProtocol::AttributePathIB
  { { ContextTag(2), "data"}, kInvalidNodeIndex }, // IMProtocol::LONG_OCTET_STRING
};

Entry<ItemInfo> _IMProtocol_AttributeReportIB[] = {
  { { ContextTag(0), "attribute_status"}, 24 }, // IMProtocol::AttributeStatus
  { { ContextTag(1), "attribute_data"}, 25 }, // IMProtocol::AttributeData
};

Entry<ItemInfo> _IMProtocol_EventStatusIB[] = {
  { { ContextTag(0), "path"}, 15 }, // IMProtocol::EventPathIB
  { { ContextTag(1), "status"}, 23 }, // IMProtocol::StatusIB
};

Entry<ItemInfo> _IMProtocol_EventDataIB[] = {
  { { ContextTag(0), "path"}, 15 }, // IMProtocol::EventPathIB
  { { ContextTag(1), "event_number"}, kInvalidNodeIndex }, // IMProtocol::int64u
  { { ContextTag(2), "priority"}, kInvalidNodeIndex }, // IMProtocol::int8u
  { { ContextTag(3), "epoch_timestamp"}, kInvalidNodeIndex }, // IMProtocol::int64u
  { { ContextTag(4), "system_timestamp"}, kInvalidNodeIndex }, // IMProtocol::int64u
  { { ContextTag(5), "delta_epoch_timestamp"}, kInvalidNodeIndex }, // IMProtocol::int64u
  { { ContextTag(6), "delta_system_timestamp"}, kInvalidNodeIndex }, // IMProtocol::int64u
  { { ContextTag(2), "data"}, kInvalidNodeIndex }, // IMProtocol::LONG_OCTET_STRING
};

Entry<ItemInfo> _IMProtocol_EventReportIB[] = {
  { { ContextTag(0), "event_status"}, 27 }, // IMProtocol::EventStatusIB
  { { ContextTag(1), "event_data"}, 28 }, // IMProtocol::EventDataIB
};

Entry<ItemInfo> _IMProtocol_ReportDataMessage[] = {
  { { ContextTag(0), "subscription_id"}, kInvalidNodeIndex }, // IMProtocol::int32u
  { { ContextTag(1), "attribute_reports"}, 47 }, // IMProtocol::AttributeReportIB[]
  { { ContextTag(2), "event_reports"}, 49 }, // IMProtocol::EventReportIB[]
  { { ContextTag(3), "more_cunked_messages"}, kInvalidNodeIndex }, // IMProtocol::boolean
  { { ContextTag(4), "suppress_response"}, kInvalidNodeIndex }, // IMProtocol::boolean
  { { ContextTag(255), "interaction_model_revison"}, kInvalidNodeIndex }, // IMProtocol::int8u
};

Entry<ItemInfo> _IMProtocol_AttributeDataIB[] = {
  { { ContextTag(0), "data_version"}, kInvalidNodeIndex }, // IMProtocol::int32u
  { { ContextTag(1), "path"}, 14 }, // IMProtocol::AttributePathIB
  { { ContextTag(2), "data"}, kInvalidNodeIndex }, // IMProtocol::LONG_OCTET_STRING
};

Entry<ItemInfo> _IMProtocol_WriteRequestMessage[] = {
  { { ContextTag(0), "suppres_response"}, kInvalidNodeIndex }, // IMProtocol::boolean
  { { ContextTag(1), "timed_request"}, kInvalidNodeIndex }, // IMProtocol::boolean
  { { ContextTag(2), "write_requests"}, 42 }, // IMProtocol::AttributeDataIB[]
  { { ContextTag(3), "more_chunked_messages"}, kInvalidNodeIndex }, // IMProtocol::boolean
  { { ContextTag(255), "interaction_model_revison"}, kInvalidNodeIndex }, // IMProtocol::int8u
};

Entry<ItemInfo> _IMProtocol_AttributeStatusIB[] = {
  { { ContextTag(0), "path"}, 14 }, // IMProtocol::AttributePathIB
  { { ContextTag(1), "status"}, 23 }, // IMProtocol::StatusIB
};

Entry<ItemInfo> _IMProtocol_WriteResponseMessage[] = {
  { { ContextTag(0), "write_responses"}, 44 }, // IMProtocol::AttributeStatusIB[]
  { { ContextTag(255), "interaction_model_revison"}, kInvalidNodeIndex }, // IMProtocol::int8u
};

Entry<ItemInfo> _IMProtocol_CommandPathIB[] = {
  { { ContextTag(1), "endpoint_id"}, kInvalidNodeIndex }, // IMProtocol::int16u
  { { ContextTag(2), "cluster_id"}, kInvalidNodeIndex }, // IMProtocol::int32u
  { { ContextTag(3), "command_id"}, kInvalidNodeIndex }, // IMProtocol::int32u
};

Entry<ItemInfo> _IMProtocol_CommandDataIB[] = {
  { { ContextTag(0), "path"}, 35 }, // IMProtocol::CommandPathIB
  { { ContextTag(2), "data"}, kInvalidNodeIndex }, // IMProtocol::LONG_OCTET_STRING
};

Entry<ItemInfo> _IMProtocol_InvokeRequestMessage[] = {
  { { ContextTag(0), "suppress_response"}, kInvalidNodeIndex }, // IMProtocol::boolean
  { { ContextTag(1), "timed_request"}, kInvalidNodeIndex }, // IMProtocol::boolean
  { { ContextTag(2), "invoke_requests"}, 48 }, // IMProtocol::CommandDataIB[]
  { { ContextTag(255), "interaction_model_revison"}, kInvalidNodeIndex }, // IMProtocol::int8u
};

Entry<ItemInfo> _IMProtocol_CommandStatusIB[] = {
  { { ContextTag(0), "path"}, 35 }, // IMProtocol::CommandPathIB
  { { ContextTag(1), "status"}, 23 }, // IMProtocol::StatusIB
};

Entry<ItemInfo> _IMProtocol_InvokeResponseIB[] = {
  { { ContextTag(0), "command"}, 36 }, // IMProtocol::CommandDataIB
  { { ContextTag(1), "status"}, 38 }, // IMProtocol::CommandStatusIB
};

Entry<ItemInfo> _IMProtocol_InvokeResponseMessage[] = {
  { { ContextTag(0), "suppress_response"}, kInvalidNodeIndex }, // IMProtocol::boolean
  { { ContextTag(1), "invoke_responses"}, 39 }, // IMProtocol::InvokeResponseIB
  { { ContextTag(255), "interaction_model_revison"}, kInvalidNodeIndex }, // IMProtocol::int8u
};

Entry<ItemInfo> _IMProtocol_TimedRequestMessage[] = {
  { { ContextTag(0), "timeout"}, kInvalidNodeIndex }, // IMProtocol::int16u
  { { ContextTag(255), "interaction_model_revison"}, kInvalidNodeIndex }, // IMProtocol::int8u
};

Entry<ItemInfo> _IMProtocol_AttributeDataIB_list_[] = {
  { { AnonymousTag(), "[]"}, 31 }, // IMProtocol_AttributeDataIB[]
};

Entry<ItemInfo> _IMProtocol_EventFilterIB_list_[] = {
  { { AnonymousTag(), "[]"}, 16 }, // IMProtocol_EventFilterIB[]
};

Entry<ItemInfo> _IMProtocol_AttributeStatusIB_list_[] = {
  { { AnonymousTag(), "[]"}, 33 }, // IMProtocol_AttributeStatusIB[]
};

Entry<ItemInfo> _IMProtocol_DataVersionFilterIB_list_[] = {
  { { AnonymousTag(), "[]"}, 18 }, // IMProtocol_DataVersionFilterIB[]
};

Entry<ItemInfo> _IMProtocol_EventPathIB_list_[] = {
  { { AnonymousTag(), "[]"}, 15 }, // IMProtocol_EventPathIB[]
};

Entry<ItemInfo> _IMProtocol_AttributeReportIB_list_[] = {
  { { AnonymousTag(), "[]"}, 26 }, // IMProtocol_AttributeReportIB[]
};

Entry<ItemInfo> _IMProtocol_CommandDataIB_list_[] = {
  { { AnonymousTag(), "[]"}, 36 }, // IMProtocol_CommandDataIB[]
};

Entry<ItemInfo> _IMProtocol_EventReportIB_list_[] = {
  { { AnonymousTag(), "[]"}, 29 }, // IMProtocol_EventReportIB[]
};

Entry<ItemInfo> _IMProtocol_AttributePathIB_list_[] = {
  { { AnonymousTag(), "[]"}, 14 }, // IMProtocol_AttributePathIB[]
};

Entry<ItemInfo> _BdxProtocol[] = {
  { { AttributeTag(1), "send_init"}, kInvalidNodeIndex }, // BdxProtocol::octet_string
  { { AttributeTag(2), "send_accept"}, kInvalidNodeIndex }, // BdxProtocol::octet_string
  { { AttributeTag(4), "receive_init"}, kInvalidNodeIndex }, // BdxProtocol::octet_string
  { { AttributeTag(5), "receive_accept"}, kInvalidNodeIndex }, // BdxProtocol::octet_string
  { { AttributeTag(16), "block_query"}, kInvalidNodeIndex }, // BdxProtocol::octet_string
  { { AttributeTag(17), "block"}, kInvalidNodeIndex }, // BdxProtocol::octet_string
  { { AttributeTag(18), "block_eof"}, kInvalidNodeIndex }, // BdxProtocol::octet_string
  { { AttributeTag(19), "block_ack"}, kInvalidNodeIndex }, // BdxProtocol::octet_string
  { { AttributeTag(20), "block_ack_eof"}, kInvalidNodeIndex }, // BdxProtocol::octet_string
  { { AttributeTag(21), "block_query_with_skip"}, kInvalidNodeIndex }, // BdxProtocol::octet_string
};

Entry<ItemInfo> _UserDirectedCommissioningProtocol[] = {
  { { AttributeTag(0), "identification_declaration"}, 53 }, // UserDirectedCommissioningProtocol::IdentificationDeclarationStruct
};

Entry<ItemInfo> _UserDirectedCommissioningProtocol_IdentificationDeclarationStruct[] = {
  { { ContextTag(1), "instance_name"}, kInvalidNodeIndex }, // UserDirectedCommissioningProtocol::octet_string
};

Entry<ItemInfo> _all_clusters[] = {
  { { ClusterTag(0xFFFF0000), "SecureChannelProtocol" }, 1 },
  { { ClusterTag(0xFFFF0001), "IMProtocol" }, 13 },
  { { ClusterTag(0xFFFF0002), "BdxProtocol" }, 51 },
  { { ClusterTag(0xFFFF0003), "UserDirectedCommissioningProtocol" }, 52 },
};

} // namespace

#define _ENTRY(n) { sizeof(n) / sizeof(n[0]), n}

std::array<Node<ItemInfo>, 53 + 1> protocols_meta = { {
  _ENTRY(_all_clusters),
  _ENTRY(_SecureChannelProtocol), // 1
  _ENTRY(_SecureChannelProtocol_ICDParameterStruct), // 2
  _ENTRY(_SecureChannelProtocol_PBKDFParamRequest), // 3
  _ENTRY(_SecureChannelProtocol_CryptoPBKDFParameterSet), // 4
  _ENTRY(_SecureChannelProtocol_PBKDFParamResponse), // 5
  _ENTRY(_SecureChannelProtocol_PasePake1), // 6
  _ENTRY(_SecureChannelProtocol_PasePake2), // 7
  _ENTRY(_SecureChannelProtocol_PasePake3), // 8
  _ENTRY(_SecureChannelProtocol_CaseSigma1), // 9
  _ENTRY(_SecureChannelProtocol_CaseSigma2), // 10
  _ENTRY(_SecureChannelProtocol_CaseSigma3), // 11
  _ENTRY(_SecureChannelProtocol_CaseSigma2Resume), // 12
  _ENTRY(_IMProtocol), // 13
  _ENTRY(_IMProtocol_AttributePathIB), // 14
  _ENTRY(_IMProtocol_EventPathIB), // 15
  _ENTRY(_IMProtocol_EventFilterIB), // 16
  _ENTRY(_IMProtocol_ClusterPathIB), // 17
  _ENTRY(_IMProtocol_DataVersionFilterIB), // 18
  _ENTRY(_IMProtocol_StatusResponseMessage), // 19
  _ENTRY(_IMProtocol_ReadRequestMessage), // 20
  _ENTRY(_IMProtocol_SubscribeRequestMessage), // 21
  _ENTRY(_IMProtocol_SubscribeResponseMessage), // 22
  _ENTRY(_IMProtocol_StatusIB), // 23
  _ENTRY(_IMProtocol_AttributeStatus), // 24
  _ENTRY(_IMProtocol_AttributeData), // 25
  _ENTRY(_IMProtocol_AttributeReportIB), // 26
  _ENTRY(_IMProtocol_EventStatusIB), // 27
  _ENTRY(_IMProtocol_EventDataIB), // 28
  _ENTRY(_IMProtocol_EventReportIB), // 29
  _ENTRY(_IMProtocol_ReportDataMessage), // 30
  _ENTRY(_IMProtocol_AttributeDataIB), // 31
  _ENTRY(_IMProtocol_WriteRequestMessage), // 32
  _ENTRY(_IMProtocol_AttributeStatusIB), // 33
  _ENTRY(_IMProtocol_WriteResponseMessage), // 34
  _ENTRY(_IMProtocol_CommandPathIB), // 35
  _ENTRY(_IMProtocol_CommandDataIB), // 36
  _ENTRY(_IMProtocol_InvokeRequestMessage), // 37
  _ENTRY(_IMProtocol_CommandStatusIB), // 38
  _ENTRY(_IMProtocol_InvokeResponseIB), // 39
  _ENTRY(_IMProtocol_InvokeResponseMessage), // 40
  _ENTRY(_IMProtocol_TimedRequestMessage), // 41
  _ENTRY(_IMProtocol_AttributeDataIB_list_), // 42
  _ENTRY(_IMProtocol_EventFilterIB_list_), // 43
  _ENTRY(_IMProtocol_AttributeStatusIB_list_), // 44
  _ENTRY(_IMProtocol_DataVersionFilterIB_list_), // 45
  _ENTRY(_IMProtocol_EventPathIB_list_), // 46
  _ENTRY(_IMProtocol_AttributeReportIB_list_), // 47
  _ENTRY(_IMProtocol_CommandDataIB_list_), // 48
  _ENTRY(_IMProtocol_EventReportIB_list_), // 49
  _ENTRY(_IMProtocol_AttributePathIB_list_), // 50
  _ENTRY(_BdxProtocol), // 51
  _ENTRY(_UserDirectedCommissioningProtocol), // 52
  _ENTRY(_UserDirectedCommissioningProtocol_IdentificationDeclarationStruct), // 53
} };

} // namespace TLVMeta
} // namespace chip
