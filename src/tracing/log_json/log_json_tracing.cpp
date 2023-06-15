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

#include <tracing/log_json/log_json_tracing.h>

#include <lib/address_resolve/TracingStructs.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/StringBuilder.h>

#include <json/json.h>

#include <sstream>
#include <string>

namespace chip {
namespace Tracing {
namespace LogJson {

namespace {

using chip::StringBuilder;

std::string ScopeToString(Scope scope)
{
    switch (scope)
    {
    case Scope::CASESession_EstablishSession:
        return "CASESession::EstablishSession";
    case Scope::CASESession_HandleSigma1:
        return "CASESession::HandleSigma1";
    case Scope::CASESession_HandleSigma1_and_SendSigma2:
        return "CASESession_HandleSigma1_and::SendSigma2";
    case Scope::CASESession_HandleSigma2:
        return "CASESession::HandleSigma2";
    case Scope::CASESession_HandleSigma2_and_SendSigma3:
        return "CASESession_HandleSigma2_and::SendSigma3";
    case Scope::CASESession_HandleSigma2Resume:
        return "CASESession::HandleSigma2Resume";
    case Scope::CASESession_HandleSigma3:
        return "CASESession::HandleSigma3";
    case Scope::CASESession_SendSigma1:
        return "CASESession::SendSigma1";
    case Scope::CASESession_SendSigma2:
        return "CASESession::SendSigma2";
    case Scope::CASESession_SendSigma2Resume:
        return "CASESession::SendSigma2Resume";
    case Scope::CASESession_SendSigma3:
        return "CASESession::SendSigma3";
    case Scope::DeviceCommissioner_Commission:
        return "DeviceCommissioner::Commission";
    case Scope::DeviceCommissioner_CommissioningStageComplete:
        return "DeviceCommissioner::CommissioningStageComplete";
    case Scope::DeviceCommissioner_continueCommissioningDevice:
        return "DeviceCommissioner::continueCommissioningDevice";
    case Scope::DeviceCommissioner_EstablishPASEConnection:
        return "DeviceCommissioner::EstablishPASEConnection";
    case Scope::DeviceCommissioner_FindCommissioneeDevice:
        return "DeviceCommissioner::FindCommissioneeDevice";
    case Scope::DeviceCommissioner_IssueNOCChain:
        return "DeviceCommissioner::IssueNOCChain";
    case Scope::DeviceCommissioner_OnAddNOCFailureResponse:
        return "DeviceCommissioner::OnAddNOCFailureResponse";
    case Scope::DeviceCommissioner_OnAttestationFailureResponse:
        return "DeviceCommissioner::OnAttestationFailureResponse";
    case Scope::DeviceCommissioner_OnAttestationResponse:
        return "DeviceCommissioner::OnAttestationResponse";
    case Scope::DeviceCommissioner_OnCertificateChainFailureResponse:
        return "DeviceCommissioner::OnCertificateChainFailureResponse";
    case Scope::DeviceCommissioner_OnCertificateChainResponse:
        return "DeviceCommissioner::OnCertificateChainResponse";
    case Scope::DeviceCommissioner_OnCSRFailureResponse:
        return "DeviceCommissioner::OnCSRFailureResponse";
    case Scope::DeviceCommissioner_OnDeviceAttestationInformationVerification:
        return "DeviceCommissioner::OnDeviceAttestationInformationVerification";
    case Scope::DeviceCommissioner_OnDeviceNOCChainGeneration:
        return "DeviceCommissioner::OnDeviceNOCChainGeneration";
    case Scope::DeviceCommissioner_OnOperationalCertificateAddResponse:
        return "DeviceCommissioner::OnOperationalCertificateAddResponse";
    case Scope::DeviceCommissioner_OnOperationalCertificateSigningRequest:
        return "DeviceCommissioner::OnOperationalCertificateSigningRequest";
    case Scope::DeviceCommissioner_OnOperationalCredentialsProvisioningCompletion:
        return "DeviceCommissioner::OnOperationalCredentialsProvisioningCompletion";
    case Scope::DeviceCommissioner_OnRootCertFailureResponse:
        return "DeviceCommissioner::OnRootCertFailureResponse";
    case Scope::DeviceCommissioner_OnRootCertSuccessResponse:
        return "DeviceCommissioner::OnRootCertSuccessResponse";
    case Scope::DeviceCommissioner_PairDevice:
        return "DeviceCommissioner::PairDevice";
    case Scope::DeviceCommissioner_ProcessOpCSR:
        return "DeviceCommissioner::ProcessOpCSR";
    case Scope::DeviceCommissioner_SendAttestationRequestCommand:
        return "DeviceCommissioner::SendAttestationRequestCommand";
    case Scope::DeviceCommissioner_SendCertificateChainRequestCommand:
        return "DeviceCommissioner::SendCertificateChainRequestCommand";
    case Scope::DeviceCommissioner_SendOperationalCertificate:
        return "DeviceCommissioner::SendOperationalCertificate";
    case Scope::DeviceCommissioner_SendOperationalCertificateSigningRequestCommand:
        return "DeviceCommissioner::SendOperationalCertificateSigningRequestCommand";
    case Scope::DeviceCommissioner_SendTrustedRootCertificate:
        return "DeviceCommissioner::SendTrustedRootCertificate";
    case Scope::DeviceCommissioner_UnpairDevice:
        return "DeviceCommissioner::UnpairDevice";
    case Scope::DeviceCommissioner_ValidateAttestationInfo:
        return "DeviceCommissioner::ValidateAttestationInfo";
    case Scope::DeviceCommissioner_ValidateCSR:
        return "DeviceCommissioner::ValidateCSR";
    case Scope::GeneralCommissioning_ArmFailSafe:
        return "GeneralCommissioning::ArmFailSafe";
    case Scope::GeneralCommissioning_CommissioningComplete:
        return "GeneralCommissioning::CommissioningComplete";
    case Scope::GeneralCommissioning_SetRegulatoryConfig:
        return "GeneralCommissioning::SetRegulatoryConfig";
    case Scope::NetworkCommissioning_HandleAddOrUpdateThreadNetwork:
        return "NetworkCommissioning::HandleAddOrUpdateThreadNetwork";
    case Scope::NetworkCommissioning_HandleAddOrUpdateWiFiNetwork:
        return "NetworkCommissioning::HandleAddOrUpdateWiFiNetwork";
    case Scope::NetworkCommissioning_HandleConnectNetwork:
        return "NetworkCommissioning::HandleConnectNetwork";
    case Scope::NetworkCommissioning_HandleRemoveNetwork:
        return "NetworkCommissioning::HandleRemoveNetwork";
    case Scope::NetworkCommissioning_HandleReorderNetwork:
        return "NetworkCommissioning::HandleReorderNetwork";
    case Scope::NetworkCommissioning_HandleScanNetwork:
        return "NetworkCommissioning::HandleScanNetwork";
    case Scope::OperationalCredentials_AddNOC:
        return "OperationalCredentials::AddNOC";
    case Scope::OperationalCredentials_AddTrustedRootCertificate:
        return "OperationalCredentials::AddTrustedRootCertificate";
    case Scope::OperationalCredentials_AttestationRequest:
        return "OperationalCredentials::AttestationRequest";
    case Scope::OperationalCredentials_CertificateChainRequest:
        return "OperationalCredentials::CertificateChainRequest";
    case Scope::OperationalCredentials_CSRRequest:
        return "OperationalCredentials::CSRRequest";
    case Scope::OperationalCredentials_RemoveFabric:
        return "OperationalCredentials::RemoveFabric";
    case Scope::OperationalCredentials_UpdateFabricLabel:
        return "OperationalCredentials::UpdateFabricLabel";
    case Scope::OperationalCredentials_UpdateNOC:
        return "OperationalCredentials::UpdateNOC";
    case Scope::PASESession_GeneratePASEVerifier:
        return "PASESession::GeneratePASEVerifier";
    case Scope::PASESession_HandleMsg1_and_SendMsg2:
        return "PASESession_HandleMsg1_and::SendMsg2";
    case Scope::PASESession_HandleMsg2_and_SendMsg3:
        return "PASESession_HandleMsg2_and::SendMsg3";
    case Scope::PASESession_HandleMsg3:
        return "PASESession::HandleMsg3";
    case Scope::PASESession_HandlePBKDFParamRequest:
        return "PASESession::HandlePBKDFParamRequest";
    case Scope::PASESession_HandlePBKDFParamResponse:
        return "PASESession::HandlePBKDFParamResponse";
    case Scope::PASESession_Pair:
        return "PASESession::Pair";
    case Scope::PASESession_SendMsg1:
        return "PASESession::SendMsg1";
    case Scope::PASESession_SendPBKDFParamRequest:
        return "PASESession::SendPBKDFParamRequest";
    case Scope::PASESession_SendPBKDFParamResponse:
        return "PASESession::SendPBKDFParamResponse";
    case Scope::PASESession_SetupSpake2p:
        return "PASESession::SetupSpake2p";
    case Scope::Resolve_IncrementalRecordParse:
        return "Resolve::IncrementalRecordParse";
    default: {
        StringBuilder<64> builder;
        builder.Add("Scope::Unknown(");
        builder.Add(static_cast<int>(scope));
        builder.Add(")");
        return builder.c_str();
    }
    }
}

std::string InstantToString(Instant instant)
{
    switch (instant)
    {
    case Instant::Resolve_TxtNotApplicable:
        return "Resolve::TxtNotApplicable";
    case Instant::Resolve_Ipv4NotApplicable:
        return "Resolve::Ipv4NotApplicable";
    case Instant::Resolve_Ipv6NotApplicable:
        return "Resolve::Ipv6NotApplicable";
    default: {
        StringBuilder<64> builder;
        builder.Add("Instant::Unknown(");
        builder.Add(static_cast<int>(instant));
        builder.Add(")");
        return builder.c_str();
    }
    }
}
/// Writes the given value to chip log
void LogJsonValue(Json::Value const & value)
{
    Json::StreamWriterBuilder builder;

    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    std::stringstream output;

    writer->write(value, &output);

    ChipLogProgress(Automation, "%s", output.str().c_str());
}

} // namespace

void LogJsonBackend::TraceBegin(Scope scope)
{
    Json::Value value;
    value["event"] = "begin";
    value["scope"] = ScopeToString(scope);
    LogJsonValue(value);
}

void LogJsonBackend::TraceEnd(Scope scope)
{
    Json::Value value;
    value["event"] = "end";
    value["scope"] = ScopeToString(scope);
    LogJsonValue(value);
}

void LogJsonBackend::TraceInstant(Instant instant)
{
    Json::Value value;
    value["event"]   = "instant";
    value["instant"] = InstantToString(instant);
    LogJsonValue(value);
}

void LogJsonBackend::LogMessageSend(MessageSendInfo &)
{
    Json::Value value;
    value["TODO"] = "LogMessageSend";
    LogJsonValue(value);
}

void LogJsonBackend::LogMessageReceived(MessageReceiveInfo &)
{
    Json::Value value;
    value["TODO"] = "LogMessageReceived";
    LogJsonValue(value);
}

void LogJsonBackend::LogNodeLookup(NodeLookupInfo & info)
{
    Json::Value value;

    value["event"]                = "LogNodeLookup";
    value["node_id"]              = info.request->GetPeerId().GetNodeId();
    value["compressed_fabric_id"] = info.request->GetPeerId().GetCompressedFabricId();
    value["min_lookup_time_ms"]   = info.request->GetMinLookupTime().count();
    value["max_lookup_time_ms"]   = info.request->GetMaxLookupTime().count();

    LogJsonValue(value);
}

void LogJsonBackend::LogNodeDiscovered(NodeDiscoveredInfo & info)
{
    Json::Value value;
    value["event"] = "LogNodeDiscovered";

    value["node_id"]              = info.peerId->GetNodeId();
    value["compressed_fabric_id"] = info.peerId->GetCompressedFabricId();

    switch (info.type)
    {
    case chip::Tracing::DiscoveryInfoType::kIntermediateResult:
        value["type"] = "intermediate";
        break;
    case chip::Tracing::DiscoveryInfoType::kResolutionDone:
        value["type"] = "done";
        break;
    case chip::Tracing::DiscoveryInfoType::kRetryDifferent:
        value["type"] = "retry-different";
        break;
    }

    {
        Json::Value result;

        char address_buff[chip::Transport::PeerAddress::kMaxToStringSize];

        info.result->address.ToString(address_buff);

        result["supports_tcp"] = info.result->supportsTcp;
        result["address"]      = address_buff;

        result["mrp"]["idle_retransmit_timeout_ms"]   = info.result->mrpRemoteConfig.mIdleRetransTimeout.count();
        result["mrp"]["active_retransmit_timeout_ms"] = info.result->mrpRemoteConfig.mActiveRetransTimeout.count();

        value["result"] = result;
    }

    LogJsonValue(value);
}

void LogJsonBackend::LogNodeDiscoveryFailed(NodeDiscoveryFailedInfo & info)
{
    Json::Value value;

    value["event"]                = "LogNodeDiscoveryFailed";
    value["node_id"]              = info.peerId->GetNodeId();
    value["compressed_fabric_id"] = info.peerId->GetCompressedFabricId();
    value["error"]                = chip::ErrorStr(info.error);

    LogJsonValue(value);
}

} // namespace LogJson
} // namespace Tracing
} // namespace chip
