/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "PICSBooleanExpressionParser.h"
#include "PICSNormalizer.h"

#include <lib/support/CodeUtils.h>

bool PICSBooleanExpressionParser::Eval(std::string expression, std::map<std::string, bool> & PICS)
{
    std::vector<std::string> tokens;
    uint8_t index = 0;

    Tokenize(expression, tokens);
    return EvaluateExpression(tokens, PICS, index);
}

void PICSBooleanExpressionParser::Tokenize(std::string & expression, std::vector<std::string> & tokens)
{
    if (expression.empty())
    {
        return;
    }

    std::string s;

    for (char c : expression)
    {
        switch (c)
        {
        case ' ':
        case '\n':
        case '\t':
            if (s.empty())
            {
                continue;
            }
            break;

        case '(':
        case ')':
        case '!':
            if (!s.empty() > 0)
            {
                tokens.push_back(s);
                s.clear();
            }

            tokens.push_back(std::string(1, c));
            break;

        case '&':
        case '|':
            if (!s.empty() && s.back() == c)
            {
                s.pop_back();
                if (s.size())
                {
                    tokens.push_back(s);
                    s.clear();
                }

                tokens.push_back(std::string(1, c) + std::string(1, c));
                break;
            }

            s.push_back(c);
            break;

        default:
            s.push_back(c);
            break;
        }
    }

    if (s.size())
    {
        tokens.push_back(s);
    }
}

bool PICSBooleanExpressionParser::EvaluateExpression(std::vector<std::string> & tokens, std::map<std::string, bool> & PICS,
                                                     uint8_t & index)
{
    bool leftExpr = EvaluateSubExpression(tokens, PICS, index);
    if (index >= tokens.size())
    {
        return leftExpr;
    }

    if (tokens[index] == ")")
    {
        return leftExpr;
    }

    std::string token = tokens[index];
    if (token == "&&")
    {
        index++;
        bool rightExpr = EvaluateExpression(tokens, PICS, index);
        return leftExpr && rightExpr;
    }
    if (token == "||")
    {
        index++;
        bool rightExpr = EvaluateExpression(tokens, PICS, index);
        return leftExpr || rightExpr;
    }

    ChipLogError(chipTool, "Unknown token: '%s'", token.c_str());
    chipDie();
}

bool PICSBooleanExpressionParser::EvaluateSubExpression(std::vector<std::string> & tokens, std::map<std::string, bool> & PICS,
                                                        uint8_t & index)
{
    std::string token = tokens[index];
    if (token == "(")
    {
        index++;
        bool expr = EvaluateExpression(tokens, PICS, index);
        if (tokens[index] != ")")
        {
            ChipLogError(chipTool, "Missing ')'");
            chipDie();
        }

        index++;
        return expr;
    }
    if (token == "!")
    {
        index++;
        bool expr = EvaluateSubExpression(tokens, PICS, index);
        return !expr;
    }

    token = PICSNormalizer::Normalize(token);

    index++;

    if (PICS.find(token) == PICS.end())
    {
        // By default, let's consider that if a PICS item is not defined, it is |false|.
        // It allows to create a file that only contains enabled features.
        return false;
    }

    return PICS[token];
}
