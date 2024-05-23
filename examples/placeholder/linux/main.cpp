/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "AppMain.h"
#include "AppOptions.h"
#include "InteractiveServer.h"

#include <app/util/MatterCallbacks.h>

namespace {
class InteractiveServerRedirectCallbacks : public chip::DataModelCallbacks
{
public:
    void AttributeOperation(OperationType operation, OperationOrder order, const chip::app::ConcreteAttributePath & path) override
    {
        if (order != OperationOrder::Post)
        {
            return;
        }

        // TODO: is there any value in checking the return of read/write attributes?
        //       they seem to only return true/false based on isRead (i.e. commissioning complete)
        switch (operation)
        {
        case OperationType::Read:
            (void) InteractiveServer::GetInstance().ReadAttribute(path);
            break;
        case OperationType::Write:
            (void) InteractiveServer::GetInstance().WriteAttribute(path);
            break;
        }
    }

    void PostCommandReceived(const chip::app::ConcreteCommandPath & commandPath,
                             const chip::Access::SubjectDescriptor & subjectDescriptor) override
    {
        (void) InteractiveServer::GetInstance().Command(commandPath);
    }
};

InteractiveServerRedirectCallbacks gDmCallbacks;

} // namespace

void ApplicationInit() {}

void ApplicationShutdown() {}

int main(int argc, char * argv[])
{
    VerifyOrDie(ChipLinuxAppInit(argc, argv, AppOptions::GetOptions()) == 0);

    LinuxDeviceOptions::GetInstance().dacProvider = AppOptions::GetDACProvider();

    auto & server = InteractiveServer::GetInstance();
    if (AppOptions::GetInteractiveMode())
    {
        server.Run(AppOptions::GetInteractiveModePort());
    }

    chip::DataModelCallbacks::SetInstance(&gDmCallbacks);
    ChipLinuxAppMainLoop();

    return 0;
}
