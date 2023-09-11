/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Declaration of PICS Boolean Expression parser, a class that
 *        implements PICS condition parsing for YAML tests.
 */

#include <cstdint>
#include <map>
#include <string>
#include <vector>

class PICSBooleanExpressionParser
{
public:
    /**
     * @brief
     *  This function returns a boolean which is the result of evaluating the
     *  boolean logic expressed into the PICS expression.
     *
     * @param [in] expression An expression containing PICS code such as
     *                        "!DT_CTRL_CONCATENATED_QR_CODE_1 && DT_CTRL_CONCATENATED_QR_CODE_2"
     * @param [in] PICS       A map of enabled/disabled PICS code
     *
     * @returns A boolean as the result of evaluating the expression.
     */
    static bool Eval(std::string expression, std::map<std::string, bool> & PICS);

private:
    static void Tokenize(std::string & expression, std::vector<std::string> & tokens);
    static bool EvaluateExpression(std::vector<std::string> & tokens, std::map<std::string, bool> & PICS, uint8_t & index);
    static bool EvaluateSubExpression(std::vector<std::string> & tokens, std::map<std::string, bool> & PICS, uint8_t & index);
};
