/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#pragma once

#if CHIP_ENABLE_OPENTHREAD
#include <openthread/cli.h>
#include <openthread/instance.h>

static_assert(OPENTHREAD_API_VERSION >= 85, "Invalid Open Thread version");
#endif // CHIP_ENABLE_OPENTHREAD

#include "ot_cli_service/ot_cli_service.rpc.pb.h"
#include "platform/ConfigurationManager.h"
#include "pw_log/log.h"

namespace chip {
namespace rpc {

class OtCli : public pw_rpc::nanopb::OtCli::Service<OtCli>
{
public:
    virtual ~OtCli() = default;

    virtual void Command(const ::chip_rpc_OtCliCommand & request, ServerWriter<::chip_rpc_OtCliResponse> & writer)
    {
#if CHIP_ENABLE_OPENTHREAD
        LazyInit();
        rpc_writer = &writer;
        otCliInputLine(const_cast<char *>(request.command));
        rpc_writer->Finish();
        rpc_writer = nullptr;
#endif // CHIP_ENABLE_OPENTHREAD
    }

private:
    ServerWriter<::chip_rpc_OtCliResponse> * rpc_writer = nullptr;

#if CHIP_ENABLE_OPENTHREAD
    bool mInitComplete = false;

    void LazyInit()
    {
        if (mInitComplete)
        {
            return;
        }
        otCliInit(otInstanceInitSingle(), &OnOtCliOutput, this);
        mInitComplete = true;
    }

    static int OnOtCliOutput(void * context, const char * format, va_list arguments)
    {
        OtCli * ot_cli = reinterpret_cast<OtCli *>(context);
        if (ot_cli->rpc_writer)
        {
            chip_rpc_OtCliResponse out;
            int rval = vsnprintf(out.response, sizeof(out.response), format, arguments);
            ot_cli->rpc_writer->Write(out);
            return rval;
        }
        return 0;
    }
#endif // CHIP_ENABLE_OPENTHREAD
};

} // namespace rpc
} // namespace chip
