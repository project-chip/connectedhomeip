#include "ota-requestor-interface.h"

// OTA Software Update Requestor Cluster AnnounceOtaProvider Command callback
bool emberAfOtaSoftwareUpdateRequestorClusterAnnounceOtaProviderCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::OtaSoftwareUpdateRequestor::Commands::AnnounceOtaProvider::DecodableType & commandData)
{
    EmberAfStatus status;
    OTARequestorInterface * requestor = GetRequestorInstance();

    if (requestor != nullptr)
    {
        status = requestor->HandleAnnounceOTAProvider(commandObj, commandPath, commandData);
    }
    else
    {
        status = EMBER_ZCL_STATUS_FAILURE;
    }

    emberAfSendImmediateDefaultResponse(status);
    return true;
}

// TBD for now. Not clear what is the API for registering a command response handler
/* Callbacks for QueryImage response */
// Callback<OtaSoftwareUpdateProviderClusterQueryImageResponseCallback> mQueryImageResponseCallback(QueryImageResponseHandler,
// nullptr);

// void QueryImageResponseHandler(void * context, const QueryImageResponse::DecodableType & response)
//{ }
