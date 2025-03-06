/*
 *   Copyright (c) 2025 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "ProviderCommand.h"
#include <commands/common/RemoteDataModelLogger.h>
#include <commands/interactive/InteractiveCommands.h>
#include <rtc/rtc.hpp>
#include <thread>
#include <unistd.h>

using namespace ::chip;
using namespace std::chrono_literals;

namespace webrtc {

CHIP_ERROR ProviderCommand::RunCommand()
{
    // print to console
    fprintf(stderr, "Run ProviderCommand.\n");

    rtc::InitLogger(rtc::LogLevel::Warning);

    rtc::Configuration config;

    auto pc = std::make_shared<rtc::PeerConnection>(config);

    pc->onLocalDescription([](rtc::Description description) {
        std::cout << "Local Description (Paste this to the other peer):" << std::endl;
        std::cout << std::string(description) << std::endl;
    });

    pc->onLocalCandidate([](rtc::Candidate candidate) {
        std::cout << "Local Candidate (Paste this to the other peer after the local description):" << std::endl;
        std::cout << std::string(candidate) << std::endl << std::endl;
    });

    pc->onStateChange([](rtc::PeerConnection::State state) { std::cout << "[State: " << state << "]" << std::endl; });

    pc->onGatheringStateChange(
        [](rtc::PeerConnection::GatheringState state) { std::cout << "[Gathering State: " << state << "]" << std::endl; });

    auto dc = pc->createDataChannel("test"); // this is the offerer, so create a data channel

    dc->onOpen([&]() { std::cout << "[DataChannel open: " << dc->label() << "]" << std::endl; });

    dc->onClosed([&]() { std::cout << "[DataChannel closed: " << dc->label() << "]" << std::endl; });

    dc->onMessage([](auto data) {
        if (std::holds_alternative<std::string>(data))
        {
            std::cout << "[Received: " << std::get<std::string>(data) << "]" << std::endl;
        }
    });

    StopReadCommandThread();

    std::this_thread::sleep_for(1s);

    bool exit = false;
    while (!exit)
    {
        std::cout << std::endl
                  << "**********************************************************************************"
                     "*****"
                  << std::endl
                  << "* 0: Exit /"
                  << " 1: Enter remote description /"
                  << " 2: Enter remote candidate /"
                  << " 3: Send message /"
                  << " 4: Print Connection Info *" << std::endl
                  << "[Command]: ";

        int command = -1;
        std::cin >> command;
        std::cin.ignore();

        switch (command)
        {
        case 0: {
            exit = true;
            StartReadCommandThread();
            break;
        }
        case 1: {
            // Parse Description
            std::cout << "[Description]: ";
            std::string sdp, line;
            while (getline(std::cin, line) && !line.empty())
            {
                sdp += line;
                sdp += "\r\n";
            }
            pc->setRemoteDescription(sdp);
            std::this_thread::sleep_for(1s);
            break;
        }
        case 2: {
            // Parse Candidate
            std::cout << "[Candidate]: ";
            std::string candidate;
            getline(std::cin, candidate);
            pc->addRemoteCandidate(candidate);
            std::this_thread::sleep_for(2s);
            break;
        }
        case 3: {
            // Send Message
            if (!dc->isOpen())
            {
                std::cout << "** Channel is not Open ** ";
                break;
            }
            std::cout << "[Message]: ";
            std::string message;
            getline(std::cin, message);
            dc->send(message);
            break;
        }
        case 4: {
            // Connection Info
            if (!dc || !dc->isOpen())
            {
                std::cout << "** Channel is not Open ** ";
                break;
            }
            rtc::Candidate local, remote;
            std::optional<std::chrono::milliseconds> rtt = pc->rtt();
            if (pc->getSelectedCandidatePair(&local, &remote))
            {
                std::cout << "Local: " << local << std::endl;
                std::cout << "Remote: " << remote << std::endl;
                std::cout << "Bytes Sent:" << pc->bytesSent() << " / Bytes Received:" << pc->bytesReceived()
                          << " / Round-Trip Time:";
                if (rtt.has_value())
                    std::cout << rtt.value().count();
                else
                    std::cout << "null";
                std::cout << " ms";
            }
            else
            {
                std::cout << "Could not get Candidate Pair Info" << std::endl;
            }
            break;
        }
        default: {
            std::cout << "** Invalid Command **" << std::endl;
            break;
        }
        }
    }

    if (dc)
        dc->close();

    if (pc)
        pc->close();

    return CHIP_NO_ERROR;
}

} // namespace webrtc
