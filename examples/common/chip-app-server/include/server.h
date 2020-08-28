#ifndef CHIP_APP_SERVER_SERVER_H
#define CHIP_APP_SERVER_SERVER_H

#include <inet/IPAddress.h>
#include <inet/InetLayer.h>
#include <system/SystemPacketBuffer.h>
#include <transport/SecureSessionMgr.h>
#include <transport/UDP.h>

using DemoSessionManager = chip::SecureSessionMgr<chip::Transport::UDP>;

void StartServer(DemoSessionManager * sessions);
void InitDataModelHandler();

#endif // CHIP_APP_SERVER_SERVER_H
