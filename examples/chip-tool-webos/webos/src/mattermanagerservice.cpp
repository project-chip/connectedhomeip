#if !CHIP_TOOL_MODE

#include <iostream>
#include <fstream>
#include <pbnjson.hpp>
#include <functional>
#include <support/BytesToHex.h>
#include <string>
#include <cstring>

#include "mattermanagerservice.h"
#include "lsutils.h"
#include "logging.h"

#include "commands/common/Commands.h"

#include "commands/discover/Commands.h"
#include "commands/pairing/Commands.h"
#include "commands/payload/Commands.h"

#include <zap-generated/cluster/Commands.h>
//#include <zap-generated/reporting/Commands.h>

#include <setup_payload/ManualSetupPayloadParser.h>
#include <setup_payload/QRCodeSetupPayloadParser.h>
#include <setup_payload/SetupPayload.h>


MatterManagerService* MatterManagerService::managerInstance = nullptr;


MatterManagerService::MatterManagerService(void) :
	LS::Handle("com.webos.service.matter"),
	mMainLoop(0),
	mSendCommandWatch(0)
{
	MATTER_DEBUG("Starting chip manager service ...");

	LS_CREATE_CATEGORY_BEGIN(MatterManagerService, root)
		LS_CATEGORY_METHOD(pair)
		LS_CATEGORY_METHOD(openCommissioningWindow)
		LS_CATEGORY_METHOD(revokeCommissioningWindow)
		LS_CATEGORY_METHOD(sendCommand)
	LS_CREATE_CATEGORY_END

	registerCategory("/", LS_CATEGORY_TABLE_NAME(root), NULL, NULL);
	setCategoryData("/", this);

	registerCommands();
}

MatterManagerService::~MatterManagerService(void)
{
	MATTER_DEBUG("Shutting down chip manager serivce ...");
}

void MatterManagerService::shutdown(void)
{
	MATTER_DEBUG("Shutting down...");
	if (mMainLoop)
	{
		g_main_loop_quit(mMainLoop);
	}
}

void MatterManagerService::updateDiscoverCommissionableNodes(const chip::Dnssd::DiscoveredNodeData & nodeData)
{
	MATTER_DEBUG("Update discovered commissionalbe nodes");

	if (!mSendCommandWatch)
	{
		MATTER_DEBUG("Ignore updateDiscoverCommissionablesNodes becasue mSendCommandWatch is NULL");
		return ;
	}

	pbnjson::JValue responseObj = pbnjson::Object();
	pbnjson::JValue nodesObj = pbnjson::Array();

	pbnjson::JValue nodeObj = pbnjson::Object();

	char rotatingId[chip::Dnssd::kMaxRotatingIdLen * 2 + 1] = "";
	chip::Encoding::BytesToUppercaseHexString(nodeData.rotatingId, nodeData.rotatingIdLen, rotatingId, sizeof(rotatingId));

	nodeObj.put("hostName", nodeData.hostName);
	nodeObj.put("port", std::to_string(nodeData.port));
	nodeObj.put("longDiscriminator", std::to_string(nodeData.longDiscriminator));
	nodeObj.put("vendorId", std::to_string(nodeData.vendorId));
	nodeObj.put("productId", std::to_string(nodeData.productId));
	nodeObj.put("commissioningMode", std::to_string(nodeData.commissioningMode));
	nodeObj.put("deviceType", std::to_string(nodeData.deviceType));
	nodeObj.put("deviceName", nodeData.deviceName);
	nodeObj.put("rotatingId", rotatingId);
	nodeObj.put("pairingInstruction", nodeData.pairingInstruction);
	nodeObj.put("pairingHint", std::to_string(nodeData.pairingHint));

	pbnjson::JValue addressObj = pbnjson::Array();

	for (int j = 0; j < nodeData.numIPs; ++j)
	{

		char buf[chip::Inet::IPAddress::kMaxStringLength];
		nodeData.ipAddress[j].ToString(buf);

		addressObj.append(buf);
	}
	nodeObj.put("address", addressObj);

	nodesObj.append(nodeObj);

	responseObj.put("nodes", nodesObj);
	responseObj.put("returnValue", true);

	LSUtils::postToClient(mSendCommandWatch->getMessage(), responseObj);

	abortSendCommand();
}

void MatterManagerService::receivePairingResult(bool ret)
{
	MATTER_DEBUG("Receive pairing result : %d ", ret);

	if (!mSendCommandWatch)
	{
		MATTER_DEBUG("Ignore receivePairingResult becasue mSendCommandWatch is NULL");
		return ;
	}

	if (!ret)
	{
		LSUtils::respondWithError(mSendCommandWatch->getMessage(), "Failed to pair");
		return;
	}

	pbnjson::JValue responseObj = pbnjson::Object();
	responseObj.put("returnValue", true);

	LSUtils::postToClient(mSendCommandWatch->getMessage(), responseObj);

	abortSendCommand();
}

void MatterManagerService::receiveOpenCommissioningWindowResult(bool ret)
{
	MATTER_DEBUG("Receive open-commissioning-window result : %d ", ret);

	if (!mSendCommandWatch)
	{
		MATTER_DEBUG("Ignore updateDiscoverCommissionablesNodes becasue mSendCommandWatch is NULL");
		return ;
	}

	if (!ret)
	{
		LSUtils::respondWithError(mSendCommandWatch->getMessage(), "Failed to open commissioning window");
		return;
	}

	pbnjson::JValue responseObj = pbnjson::Object();
	responseObj.put("returnValue", true);

	LSUtils::postToClient(mSendCommandWatch->getMessage(), responseObj);

	abortSendCommand();

}

void MatterManagerService::receiveRevokeCommissioningWindowResult(bool ret)
{
	MATTER_DEBUG("Receive revoke-commissioning-window result : %d ", ret);

	if (!ret)
	{
		LSUtils::respondWithError(mRevokeCommissioningWindowRequest, "Failed to open commissioning window");
		return;
	}

	pbnjson::JValue responseObj = pbnjson::Object();
	responseObj.put("returnValue", true);

	LSUtils::postToClient(mRevokeCommissioningWindowRequest, responseObj);

}

void MatterManagerService::receiveParseSetupPayloadResult(chip::SetupPayload & payload)
{
	MATTER_DEBUG("Received parse-setup-payload");

	if (!mSendCommandWatch)
	{
		MATTER_DEBUG("Ignore receiveParseSetupPayloadResult becasue mSendCommandWatch is NULL");
		return ;
	}

	pbnjson::JValue responseObj = pbnjson::Object();
	pbnjson::JValue payloadObj = pbnjson::Object();

	payloadObj.put("commissioningFlow", std::to_string(to_underlying(payload.commissioningFlow)));
	payloadObj.put("vendorId", std::to_string(payload.vendorID));
	payloadObj.put("version", std::to_string(payload.version));
	payloadObj.put("productId", std::to_string(payload.productID));
	payloadObj.put("discriminator", std::to_string(payload.discriminator));
	payloadObj.put("setupPinCode", std::to_string(payload.setUpPINCode));
	payloadObj.put("rendezvousInformation", std::to_string(payload.rendezvousInformation.Raw()));


	responseObj.put("payload", payloadObj);
	responseObj.put("returnValue", true);

	LSUtils::postToClient(mSendCommandWatch->getMessage(), responseObj);

	abortSendCommand();
}

bool MatterManagerService::pair(LSMessage &msg)
{
	MATTER_INFO("MANAGER_SERVICE", 0, "Luna API is called : [%s : %d]", __FUNCTION__, __LINE__);

	LS::Message request(&msg);
	pbnjson::JValue requestObj;
	int parseError = 0;

	const std::string schema =  STRICT_SCHEMA(PROPS_5(
                                    PROP(pairingType, string), PROP(setupPinCode, integer), PROP(discriminator, integer), PROP(deviceRemoteIp, string),
									PROP(deviceRemotePort, integer)));

	if (!LSUtils::parsePayload(request.getPayload(), requestObj, schema, &parseError))
	{
		if (parseError == JSON_PARSE_SCHEMA_ERROR)
			LSUtils::respondWithError(request, "The JSON input does not match the expected schema");
		else
			LSUtils::respondWithError(request, "Invalid JSON input");
		return true;
	}

	std::string pairingType;
	if (requestObj.hasKey("pairingType"))
	{
		pairingType = requestObj["pairingType"].asString();
		if (pairingType.compare("ethernet") != 0)
		{
			LSUtils::respondWithError(request, "ethernet is only supported");
		}
	}

	int setupPinCode = 0;
	if (requestObj.hasKey("setupPinCode"))
		setupPinCode = requestObj["setupPinCode"].asNumber<int32_t>();

	int discriminator = 0;
	if (requestObj.hasKey("discriminator"))
		discriminator = requestObj["discriminator"].asNumber<int32_t>();

	std::string deviceRemoteIp;
	if (requestObj.hasKey("deviceRemoteIp"))
		deviceRemoteIp = requestObj["deviceRemoteIp"].asString();

	int deviceRemotePort = 0;
	if (requestObj.hasKey("deviceRemotePort"))
		deviceRemotePort = requestObj["deviceRemotePort"].asNumber<int32_t>();

	mPairingRequest = request;
	sendPairingCommand(setupPinCode, discriminator, deviceRemoteIp, deviceRemotePort);

	return true;
}

bool MatterManagerService::openCommissioningWindow(LSMessage &msg)
{
	MATTER_INFO("MANAGER_SERVICE", 0, "Luna API is called : [%s : %d]", __FUNCTION__, __LINE__);

	LS::Message request(&msg);
	pbnjson::JValue requestObj;
	int parseError = 0;

	const std::string schema =  STRICT_SCHEMA(PROPS_5(
                                    PROP(timeout, integer), PROP(option, integer), PROP(discriminator, integer), PROP(iteration, integer), PROP(endpointId, integer)));

	if (!LSUtils::parsePayload(request.getPayload(), requestObj, schema, &parseError))
	{
		if (parseError == JSON_PARSE_SCHEMA_ERROR)
			LSUtils::respondWithError(request, "The JSON input does not match the expected schema");
		else
			LSUtils::respondWithError(request, "Invalid JSON input");
		return true;
	}

	int timeout = 0;
	if (requestObj.hasKey("timeout"))
		timeout = requestObj["timeout"].asNumber<int32_t>();

	int option = 0;
	if (requestObj.hasKey("option"))
		option = requestObj["option"].asNumber<int32_t>();

	int discriminator = 0;
	if (requestObj.hasKey("discriminator"))
		discriminator = requestObj["discriminator"].asNumber<int32_t>();

	int iteration = 0;
	if (requestObj.hasKey("iteration"))
		iteration = requestObj["iteration"].asNumber<int32_t>();

	int endpointId = 0;
	if (requestObj.hasKey("endpointId"))
		endpointId = requestObj["endpointId"].asNumber<int32_t>();

	mOpenCommissioningWindowRequest = request;
	sendOpenCommissioningWindowCommand(timeout, option, discriminator, iteration, endpointId);

	return true;
}

bool MatterManagerService::revokeCommissioningWindow(LSMessage &msg)
{
	MATTER_INFO("MANAGER_SERVICE", 0, "Luna API is called : [%s : %d]", __FUNCTION__, __LINE__);

	LS::Message request(&msg);
	pbnjson::JValue requestObj;
	int parseError = 0;

	const std::string schema =  STRICT_SCHEMA(PROPS_1(
                                    PROP(endpointId, integer)));

	if (!LSUtils::parsePayload(request.getPayload(), requestObj, schema, &parseError))
	{
		if (parseError == JSON_PARSE_SCHEMA_ERROR)
			LSUtils::respondWithError(request, "The JSON input does not match the expected schema");
		else
			LSUtils::respondWithError(request, "Invalid JSON input");
		return true;
	}

	int endpointId = 0;
	if (requestObj.hasKey("endpointId"))
		endpointId = requestObj["endpointId"].asNumber<int32_t>();

	mRevokeCommissioningWindowRequest = request;
	sendRevokeCommissioningWindowCommand(endpointId);

	return true;
}

bool MatterManagerService::sendCommand(LSMessage &msg)
{
	MATTER_INFO("MANAGER_SERVICE", 0, "Luna API is called : [%s : %d]", __FUNCTION__, __LINE__);

	LS::Message request(&msg);
	pbnjson::JValue requestObj;
	int parseError = 0;

	const std::string schema =  STRICT_SCHEMA(PROPS_5(
                                    PROP(clusterName, string), PROP(commandName, string), ARRAY(parameters, string), PROP(attributeName, string), PROP(attributeValue, string)));

	if (!LSUtils::parsePayload(request.getPayload(), requestObj, schema, &parseError))
	{
		if (parseError == JSON_PARSE_SCHEMA_ERROR)
			LSUtils::respondWithError(request, "The JSON input does not match the expected schema");
		else
			LSUtils::respondWithError(request, "Invalid JSON input");
		return true;
	}

	if (mSendCommandWatch)
	{
		LSUtils::respondWithError(request, "Allow only one client");
	}

	mSendCommandWatch = new LSUtils::ClientWatch(get(), &msg, [this]() {
		notifySendCommandDropped();
	});


	std::string clusterName;
	if (requestObj.hasKey("clusterName"))
		clusterName = requestObj["clusterName"].asString();

	std::string commandName;
	if (requestObj.hasKey("commandName"))
		commandName = requestObj["commandName"].asString();

	std::vector<std::string> parameters;
	if (requestObj.hasKey("parameters"))
	{
		auto parametersObjArray = requestObj["parameters"];
		// mParamLength = parametersObjArray.arraySize();
		for (int n = 0; n < parametersObjArray.arraySize(); n++)
		{
			pbnjson::JValue element = parametersObjArray[n];
			parameters.push_back(element.asString());
		}
	}

	std::string attributeName;
	if (requestObj.hasKey("attributeName"))
		attributeName = requestObj["attributeName"].asString();

	std::string attributeValue;
	if (requestObj.hasKey("attributeValue"))
		attributeValue = requestObj["attributeValue"].asString();


	int result = makeAndSendCommand(clusterName, commandName, parameters, attributeName, attributeValue);

	sendRespond(result, clusterName);

	return true;
}

void MatterManagerService::sendRespond(int result, std::string clusterName)
{
	if (!mSendCommandWatch)
		return;

	if (!clusterName.compare("discover"))
	{
		pbnjson::JValue responseObj = pbnjson::Object();
		pbnjson::JValue nodesObj = pbnjson::Array();
		responseObj.put("nodes", nodesObj);
		responseObj.put("returnValue", true);
		LSUtils::postToClient(mSendCommandWatch->getMessage(), responseObj);

		abortSendCommand();
		return;
	}

	if (result == EXIT_SUCCESS)
	{
		pbnjson::JValue responseObj = pbnjson::Object();
		responseObj.put("returnValue", true);
		LSUtils::postToClient(mSendCommandWatch->getMessage(), responseObj);
	}
	else
	{
		LSUtils::respondWithError(mSendCommandWatch->getMessage(), "Failed to sendCommand");
	}
	abortSendCommand();
}

gboolean MatterManagerService::shutdownNotification(gpointer user_data)
{
	MatterManagerService *watch = static_cast<MatterManagerService*>(user_data);
	if (nullptr == watch)
		return FALSE;

	watch->shutdown();

	return FALSE;
}

void MatterManagerService::abortSendCommand(void)
{
	if (mSendCommandWatch)
	{
		delete mSendCommandWatch;
		mSendCommandWatch = 0;
	}

	g_timeout_add(1, &MatterManagerService::shutdownNotification, this);
}

bool MatterManagerService::notifySendCommandDropped(void)
{
	if (mSendCommandWatch)
		abortSendCommand();
	return true;
}

void MatterManagerService::registerCommands(void)
{
	registerCommandsDiscover(mCommands);
	registerCommandsPairing(mCommands);
	registerCommandsPayload(mCommands);
	registerClusters(mCommands);
	// registerClusterAdministratorCommissioning(mCommands);
}

int MatterManagerService::sendChipCommand(int argc, char **argv)
{
	MATTER_INFO("MANAGER_SERVICE", 0, "Chip Command is called : [%s : %d]", __FUNCTION__, __LINE__);

	printf("CHIP_COMMAND : ");
	for (int i=0; i < argc; i++)
	{
		printf("%s ", argv[i]);
	}

	printf("\n");

	return mCommands.Run(argc, argv);
}

int MatterManagerService::sendPairingCommand(int setupPinCode, int discriminator, std::string deviceRemoteIp, int deviceRemotePort)
{
	MATTER_INFO("MANAGER_SERVICE", 0, "Chip Command is called : [%s : %d]", __FUNCTION__, __LINE__);

	int argc = 7;
	char *argv[7];

	argv[0] = (char*)"./webos-chip-tool";
	argv[1] = (char*)"pairing";
	argv[2] = (char*)"ethernet";


	std::string setupPinCodeStr = std::to_string(setupPinCode);
	std::string discriminatorStr = std::to_string(discriminator);
	std::string deviceRemotePortStr = std::to_string(deviceRemotePort);


	argv[3] = (char*)setupPinCodeStr.c_str();
	argv[4] = (char*)discriminatorStr.c_str();
	argv[5] = (char*)deviceRemoteIp.c_str();
	argv[6] = (char*)deviceRemotePortStr.c_str();

	return sendChipCommand(argc, argv);
}

int MatterManagerService::sendOpenCommissioningWindowCommand(int timeout, int option, int discriminator, int iteration, int endpointId)
{
	MATTER_INFO("MANAGER_SERVICE", 0, "Chip Command is called : [%s : %d]", __FUNCTION__, __LINE__);

	int ret = 0;

	if (option == 0)
	{
		int argc = 5;
		char *argv[5];

		argv[0] = (char*)"./webos-chip-tool";
		argv[1] = (char*)"administratorcommissioning";
		argv[2] = (char*)"open-basic-commissioning-window";

		std::string timeoutStr = std::to_string(timeout);
		std::string endpointIdStr = std::to_string(endpointId);

		argv[3] = (char*)timeoutStr.c_str();
		argv[4]	= (char*)endpointIdStr.c_str();

		ret = sendChipCommand(argc, argv);
	}
	else if (option == 1)
	{
		MATTER_DEBUG("Not supported option 1");
	}
	else if (option == 2)
	{
		int argc = 10;
		char *argv[10];

		argv[0] = (char*)"./webos-chip-tool";
		argv[1] = (char*)"administratorcommissioning";
		argv[2] = (char*)"open-commissioning-window";

		std::string timeoutStr = std::to_string(timeout);
		std::string discriminatorStr = std::to_string(discriminator);
		std::string iterationStr = std::to_string(iteration);
		std::string PAKEVerifierStr("hex:aba60c30416b8f4177f5e16ad514cfd9577513f02fd60506b1049d0f2c7310010e5e40bfd86b4ef681a88b71e9e2a853985a7def916ea30e01b8722fbf7d0e38856c12cd64c225bb24ef21417e0e44e5");
		std::string SaltStr("hex:5350414b453250204b65792053616c74");
		std::string endpointIdStr = std::to_string(endpointId);

		argv[3] = (char*)timeoutStr.c_str();
		argv[4] = (char*)PAKEVerifierStr.c_str();
		argv[5] = (char*)discriminatorStr.c_str();
		argv[6] = (char*)iterationStr.c_str();
		argv[7] = (char*)SaltStr.c_str();
		argv[8] = (char*)"1";
		argv[9] = (char*)endpointIdStr.c_str();;

		ret = sendChipCommand(argc, argv);
	}
	else
	{
		MATTER_DEBUG("Invalid option...");
	}

	return ret;
}

int MatterManagerService::sendRevokeCommissioningWindowCommand(int endpointId)
{
	MATTER_INFO("MANAGER_SERVICE", 0, "Chip Command is called : [%s : %d]", __FUNCTION__, __LINE__);

	int argc = 4;
	char *argv[5];

	argv[0] = (char*)"./webos-chip-tool";
	argv[1] = (char*)"administratorcommissioning";
	argv[2] = (char*)"revoke-commissioning";

	std::string endpointIdStr = std::to_string(endpointId);

	argv[3] = (char*)endpointIdStr.c_str();

	return sendChipCommand(argc, argv);
}

int MatterManagerService::makeAndSendCommand(std::string clusterName, std::string commandName, std::vector<std::string> parameters, std::string attributeName, std::string attributeValue)
{
	MATTER_INFO("MANAGER_SERVICE", 0, "Chip Command is called : [%s : %d]", __FUNCTION__, __LINE__);

	if (clusterName.empty() || commandName.empty())
	{
		return -1;
	}

	int argc = 3;
	int parametersLength = parameters.size();

	argc += parametersLength;

	if (!attributeName.empty())
		argc += 1;

	if (!attributeValue.empty())
		argc += 1;

	char *argv[argc];
	int argvNum = 0;

	argv[argvNum++] = (char*)"./chip-service";
	argv[argvNum++] = (char*)clusterName.c_str();
	argv[argvNum++] = (char*)commandName.c_str();

	if (!attributeName.empty())
		argv[argvNum++] = (char*)attributeName.c_str();

	if (!attributeValue.empty())
		argv[argvNum++] = (char*)attributeValue.c_str();

	std::string param1;
	std::string param2;
	std::string param3;
	std::string param4;
	std::string param5;
	std::string param6;

	int n = 1;

	for (auto parameter : parameters)
	{
		if (n == 1)
		{
			param1 = parameter;
			n++;
		}
		else if (n == 2)
		{
			param2 = parameter;
			n++;
		}
		else if (n == 3)
		{
			param3 = parameter;
			n++;
		}
		else if (n == 4)
		{
			param4 = parameter;
			n++;
		}
		else if (n == 5)
		{
			param5 = parameter;
			n++;
		}
		else if (n == 6)
		{
			param6 = parameter;
			n++;
		}
	}

	if (!param1.empty())
	{
		argv[argvNum++] = (char*)param1.c_str();
	}

	if (!param2.empty())
	{
		argv[argvNum++] = (char*)param2.c_str();
	}

	if (!param3.empty())
	{
		argv[argvNum++] = (char*)param3.c_str();
	}

	if (!param4.empty())
	{
		argv[argvNum++] = (char*)param4.c_str();
	}

	if (!param5.empty())
	{
		argv[argvNum++] = (char*)param5.c_str();
	}

	if (!param6.empty())
	{
		argv[argvNum++] = (char*)param6.c_str();
	}

	return sendChipCommand(argc, argv);
}

#endif // CHIP_TOOL_MODE
