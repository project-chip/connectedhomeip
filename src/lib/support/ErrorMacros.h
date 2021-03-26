/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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

#include <nlassert.h>

#include <core/CHIPError.h>

/**
 *  @def SuccessOrExit(aStatus)
 *
 *  @brief
 *    This checks for the specified status, which is expected to
 *    commonly be successful (CHIP_NO_ERROR), and branches to
 *    the local label 'exit' if the status is unsuccessful.
 *
 *  Example Usage:
 *
 *  @code
 *  CHIP_ERROR TryHard()
 *  {
 *      CHIP_ERROR err;
 *
 *      err = TrySomething();
 *      SuccessOrExit(err);
 *
 *      err = TrySomethingElse();
 *      SuccessOrExit(err);
 *
 *  exit:
 *      return err;
 *  }
 *  @endcode
 *
 *  @param[in]  aStatus     A scalar status to be evaluated against zero (0).
 *
 */
#define SuccessOrExit(aStatus) nlEXPECT((aStatus) == CHIP_NO_ERROR, exit)

/**
 *  @def VerifyOrExit(aCondition, anAction)
 *
 *  @brief
 *    This checks for the specified condition, which is expected to
 *    commonly be true, and both executes @a anAction and branches to
 *    the local label 'exit' if the condition is false.
 *
 *  Example Usage:
 *
 *  @code
 *  CHIP_ERROR MakeBuffer(const uint8_t *& buf)
 *  {
 *      CHIP_ERROR err = CHIP_NO_ERROR;
 *
 *      buf = (uint8_t *)malloc(1024);
 *      VerifyOrExit(buf != NULL, err = CHIP_ERROR_NO_MEMORY);
 *
 *      memset(buf, 0, 1024);
 *
 *  exit:
 *      return err;
 *  }
 *  @endcode
 *
 *  @param[in]  aCondition  A Boolean expression to be evaluated.
 *  @param[in]  anAction    An expression or block to execute when the
 *                          assertion fails.
 *
 */
#define VerifyOrExit(aCondition, anAction) nlEXPECT_ACTION(aCondition, exit, anAction)

/**
 *  @def ExitNow(...)
 *
 *  @brief
 *    This unconditionally executes @a ... and branches to the local
 *    label 'exit'.
 *
 *  @note The use of this interface implies neither success nor
 *        failure for the overall exit status of the enclosing function
 *        body.
 *
 *  Example Usage:
 *
 *  @code
 *  CHIP_ERROR ReadAll(Reader& reader)
 *  {
 *      CHIP_ERROR err;
 *
 *      while (true)
 *      {
 *          err = reader.ReadNext();
 *          if (err == CHIP_ERROR_AT_END)
 *              ExitNow(err = CHIP_NO_ERROR);
 *          SuccessOrExit(err);
 *          DoSomething();
 *      }
 *
 *  exit:
 *      return err;
 *  }
 *  @endcode
 *
 *  @param[in]  ...         An optional expression or block to execute
 *                          when the assertion fails.
 *
 */
// clang-format off
#define ExitNow(...)                                                   \
    do {                                                               \
        __VA_ARGS__;                                                   \
        goto exit;                                                     \
    } while (0)
// clang-format on
