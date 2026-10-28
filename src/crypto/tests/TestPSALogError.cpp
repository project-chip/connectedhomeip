/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <cstdarg>
#include <cstdio>
#include <string>
#include <vector>

#include <pw_unit_test/framework.h>

#include <crypto/CHIPCryptoPALPSA.h>
#include <lib/support/EnforceFormat.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::Crypto;

#if CHIP_CRYPTO_PSA && CHIP_ERROR_LOGGING && CHIP_LOG_FILTERING

namespace {

std::vector<std::string> gLogLines;

ENFORCE_FORMAT(3, 0) void AccumulateLogLineCallback(const char * module, uint8_t category, const char * msg, va_list args)
{
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), msg, args);
    gLogLines.emplace_back(buffer);
}

struct TestPSALogError : public ::testing::Test
{
    void SetUp() override
    {
        mSavedFilter = Logging::GetLogFilter();
        Logging::SetLogFilter(Logging::kLogCategory_Max);
        Logging::SetLogRedirectCallback(&AccumulateLogLineCallback);
        gLogLines.clear();
    }

    void TearDown() override
    {
        Logging::SetLogRedirectCallback(nullptr);
        Logging::SetLogFilter(mSavedFilter);
    }

private:
    uint8_t mSavedFilter = Logging::kLogCategory_Max;
};

TEST_F(TestPSALogError, TestSuccessIsNotLogged)
{
    LogPsaError(PSA_SUCCESS);
    EXPECT_TRUE(gLogLines.empty());
}

TEST_F(TestPSALogError, TestErrorIsLogged)
{
    [[maybe_unused]] int line = 0;
    LogPsaError(PSA_ERROR_GENERIC_ERROR);
    line = __LINE__ - 1;

    ASSERT_EQ(gLogLines.size(), 1u);

    const std::string expectedCode = "PSA error: " + std::to_string(static_cast<int>(PSA_ERROR_GENERIC_ERROR));
    EXPECT_EQ(gLogLines[0].rfind(expectedCode, 0), 0u);

#if CHIP_CRYPTO_PSA_LOG_ERROR_LOCATION
    // The location of the caller must be reported, not the location of LogPsaError itself.
    EXPECT_NE(gLogLines[0].find("TestPSALogError.cpp:" + std::to_string(line)), std::string::npos) << gLogLines[0];
#else
    EXPECT_EQ(gLogLines[0], expectedCode);
#endif
}

} // namespace

#endif // CHIP_CRYPTO_PSA && _CHIP_USE_LOGGING && CHIP_LOG_FILTERING
