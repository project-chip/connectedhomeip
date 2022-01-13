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
    virtual ~TraceStream()                                                 = default;
    virtual void Stream(const std::string & tag, const std::string & data) = 0;
    virtual void Handler(const std::string & label)                        = 0;
};

class TraceStreamLog : public TraceStream
{
public:
    void Stream(const std::string & tag, const std::string & data) override
    {
        ChipLogAutomation("    %s: %s", tag.data(), data.data());
    }
    void Handler(const std::string & label) override { ChipLogAutomation("%s", label.data()); }
};

class TraceStreamFile : public TraceStream
{
public:
    TraceStreamFile(const char * fileName) { mFile.open(fileName); }
    ~TraceStreamFile() { mFile.close(); }

    void Stream(const std::string & tag, const std::string & data) override
    {
        if (mFile.is_open())
        {
            mFile << "    " << tag.data() << ": " << data.data() << "\n";
            mFile.flush();
        }
    }
    void Handler(const std::string & label) override
    {
        if (mFile.is_open())
        {
            struct timeval tv;
            gettimeofday(&tv, nullptr);
            mFile << label << " [" << tv.tv_sec << "." << tv.tv_usec << "]\n";
            mFile.flush();
        }
    }

private:
    std::ofstream mFile;
};

void SetTraceStream(TraceStream * stream);
void InitTrace();
void DeInitTrace();

} // namespace trace
} // namespace chip
