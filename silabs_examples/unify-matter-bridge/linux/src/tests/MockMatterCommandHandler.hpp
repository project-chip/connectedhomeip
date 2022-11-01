/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#ifndef MOCK_MATTER_COMMAND_HANDLER_HPP
#define MOCK_MATTER_COMMAND_HANDLER_HPP

#include <app/CommandHandlerInterface.h>

class MockCommandHandlerCallback : public chip::app::CommandHandler::Callback
{
public:
    void OnDone(chip::app::CommandHandler & apCommandHandler) final { onFinalCalledTimes++; }
    void DispatchCommand(chip::app::CommandHandler & apCommandObj, const chip::app::ConcreteCommandPath & aCommandPath,
                         chip::TLV::TLVReader & apPayload) final
    {}
    chip::Protocols::InteractionModel::Status CommandExists(const chip::app::ConcreteCommandPath & aCommandPath)
    {
        return chip::Protocols::InteractionModel::Status::Success;
    }

    int onFinalCalledTimes = 0;
};

#endif