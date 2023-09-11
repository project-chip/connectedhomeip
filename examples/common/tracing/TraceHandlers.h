/*
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
