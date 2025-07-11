#include <string.h>

#include <pw_unit_test/framework.h>

#include <functional>
#include <optional>
#include <protocols/bdx/BdxTransferDiagnosticLog.h>

using namespace ::chip;
using namespace ::chip::bdx;
using namespace ::chip::Protocols;
using namespace ::chip::System::Clock::Literals;

namespace {

class TestTransferDiagnosticLog : public ::testing::Test
{
protected:
    TransferSession mTransferSession{};
};

TEST_F(TestTransferDiagnosticLog, TestTransferDiagnosticLog) {}

} // namespace
