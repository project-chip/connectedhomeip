/*
 *   Copyright (c) 2021 Project CHIP Authors
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

#pragma once

#include <fstream>
#include <iostream>
#include <lib/support/logging/CHIPLogging.h>
#include <string>
#include <sys/time.h>

namespace chip {
namespace trace {

class TraceStream
{
public:
    virtual ~TraceStream()                                                   = default;
    virtual void StartEvent(const std::string & label)                       = 0;
    virtual void AddField(const std::string & tag, const std::string & data) = 0;
    virtual void FinishEvent()                                               = 0;
};

class TraceStreamLog : public TraceStream
{
public:
    void StartEvent(const std::string & label) override
    {
        mStreamLine   = ">>>" + label + "<<<\t";
        mIsFirstField = true;
    }

    void AddField(const std::string & tag, const std::string & data) override
    {
        mStreamLine += (mIsFirstField ? "" : "\t") + tag + "|" + data;
        mIsFirstField = false;
    }

    void FinishEvent() override { ChipLogAutomation("TraceStream:%s", mStreamLine.c_str()); }

protected:
    std::string mStreamLine;
    bool mIsFirstField = true;
};

class TraceStreamFile : public TraceStream
{
public:
    TraceStreamFile(const char * fileName) { mFile.open(fileName); }
    ~TraceStreamFile() { mFile.close(); }

    void AddField(const std::string & tag, const std::string & data) override
    {
        if (mFile.is_open())
        {
            mFile << "    " << tag.data() << "\t" << data.data() << "\n";
            mFile.flush();
        }
    }

    void StartEvent(const std::string & label) override
    {
        if (mFile.is_open())
        {
            struct timeval tv;
            gettimeofday(&tv, nullptr);
            mFile << "\f[" << tv.tv_sec << "." << tv.tv_usec << "]\t" << label << "\n";
            mFile.flush();
        }
    }

    void FinishEvent() override {}

private:
    std::ofstream mFile;
};

void AddTraceStream(TraceStream * stream);
void InitTrace();
void DeInitTrace();

} // namespace trace
} // namespace chip
