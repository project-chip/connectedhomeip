#include <string.h>

#include <pw_unit_test/framework.h>

#include <protocols/bdx/StatusCode.h>

using namespace ::chip::bdx;

TEST(StatusCode, GetBdxStatusCodeFromChipError)
{
    EXPECT_EQ(StatusCode::kUnknown, GetBdxStatusCodeFromChipError(CHIP_ERROR_INTERNAL));
    EXPECT_EQ(StatusCode::kUnexpectedMessage, GetBdxStatusCodeFromChipError(CHIP_ERROR_INCORRECT_STATE));
    EXPECT_EQ(StatusCode::kBadMessageContents, GetBdxStatusCodeFromChipError(CHIP_ERROR_INVALID_ARGUMENT));
    EXPECT_EQ(StatusCode::kTransferMethodNotSupported, GetBdxStatusCodeFromChipError(CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE));
    EXPECT_EQ(StatusCode::kFileDesignatorUnknown, GetBdxStatusCodeFromChipError(CHIP_ERROR_UNKNOWN_RESOURCE_ID));
}
