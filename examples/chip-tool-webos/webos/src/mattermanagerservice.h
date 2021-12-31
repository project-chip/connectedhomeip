#ifndef MATTER_MANAGER_H_
#define MATTER_MANAGER_H_

#if !CHIP_TOOL_MODE
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "clientwatch.h"
#include <glib.h>
#include <luna-service2/lunaservice.hpp>

#include "commands/common/Commands.h"
#include <setup_payload/SetupPayload.h>

using namespace ::chip;

namespace LS {
class Message;
class ServerStatus;
} // namespace LS

namespace LSUtils {
class ClientWatch;
}

class MatterManagerService : public LS::Handle
{
public:
    MatterManagerService(void);
    MatterManagerService(const MatterManagerService & other);
    ~MatterManagerService(void);

public:
    static MatterManagerService * GetInstance(void)
    {
        if (managerInstance == nullptr)
            managerInstance = new MatterManagerService();
        return managerInstance;
    }

    void setMainLoop(GMainLoop * mainLoop) { mMainLoop = mainLoop; }
    void shutdown(void);

    void updateDiscoverCommissionableNodes(const chip::Dnssd::DiscoveredNodeData & nodeData);
    void receivePairingResult(bool ret);
    void receiveOpenCommissioningWindowResult(bool ret);
    void receiveRevokeCommissioningWindowResult(bool ret);
    void receiveParseSetupPayloadResult(chip::SetupPayload & payload);

private:
    bool pair(LSMessage & msg);
    bool openCommissioningWindow(LSMessage & msg);
    bool revokeCommissioningWindow(LSMessage & msg);
    bool sendCommand(LSMessage & msg);

    int makeAndSendCommand(std::string clusterName, std::string commandName, std::vector<std::string> parameters,
                           std::string attributeName, std::string attributeValue);

    void registerCommands(void);
    int sendChipCommand(int argc, char ** argv);
    int sendPairingCommand(int setupPinCode, int discrimimator, std::string deviceRemoteIp, int deviceRemotePort);
    int sendOpenCommissioningWindowCommand(int timeout, int option, int discriminator, int iteration, int endpointId);
    int sendRevokeCommissioningWindowCommand(int endpointId);

    void abortSendCommand(void);
    bool notifySendCommandDropped(void);
    void sendRespond(int result, std::string clusterName);

    static gboolean shutdownNotification(gpointer user_data);

private:
    static MatterManagerService * managerInstance;
    GMainLoop * mMainLoop;

    LS::Message mPairingRequest;
    LS::Message mOpenCommissioningWindowRequest;
    LS::Message mRevokeCommissioningWindowRequest;

    LSUtils::ClientWatch * mSendCommandWatch;

    Commands mCommands;
};

#endif

#endif // CHIP_TOOL_MODE
