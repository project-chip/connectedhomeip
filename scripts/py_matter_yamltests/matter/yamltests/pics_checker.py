#
#    Copyright (c) 2023 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the 'License');
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an 'AS IS' BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.

import unicodedata
from typing import List

_COMMENT_CHARACTER = '#'
_VALUE_SEPARATOR = '='
_VALUE_DISABLED = '0'
_VALUE_ENABLED = '1'
_CONTROL_CHARACTER_IDENTIFIER = 'C'


class InvalidPICSConfigurationError(Exception):
    "Raised when the configured pics entry can not be parsed."
    pass


class InvalidPICSConfigurationValueError(Exception):
    "Raised when the configured pics value is not an authorized value."
    pass


class InvalidPICSParsingError(Exception):
    "Raised when a parsing error occured."
    pass


class PICSChecker():
    """Class to compute a PICS expression"""

    def __init__(self, pics_file: str):
        self.__pics = {}
        self.__expression_index = 0

        if pics_file is not None:
            self.__pics = self.__parse(pics_file)

    def check(self, pics) -> bool:
        if pics is None:
            return True

        self.__expression_index = 0
        tokens = self.__tokenize(pics)
        return self.__evaluate_expression(tokens, self.__pics)

    def __parse(self, pics_file: str):
        pics = {}
        with open(pics_file) as f:
            line = f.readline()
            while line:
                preprocessed_line = self.__preprocess_input(line)
                if preprocessed_line:
                    items = preprocessed_line.split(_VALUE_SEPARATOR)
                    # There should always be one key and one value, nothing else.
                    if len(items) != 2:
                        raise InvalidPICSConfigurationError(
                            f'Invalid expression: {line}')

                    key, value = items
                    if value != _VALUE_DISABLED and value != _VALUE_ENABLED:
                        raise InvalidPICSConfigurationValueError(
                            f'Invalid expression: {line}')

                    pics[key] = value == _VALUE_ENABLED

                line = f.readline()
        return pics

    def __evaluate_expression(self, tokens: List[str], pics: dict):
        leftExpr = self.__evaluate_sub_expression(tokens, pics)
        if self.__expression_index >= len(tokens):
            return leftExpr

        token = tokens[self.__expression_index]

        if token == ')':
            return leftExpr

        token = tokens[self.__expression_index]

        if token == '&&':
            self.__expression_index += 1
            rightExpr = self.__evaluate_expression(tokens, pics)
            return leftExpr and rightExpr

        if token == '||':
            self.__expression_index += 1
            rightExpr = self.__evaluate_expression(tokens, pics)
            return leftExpr or rightExpr

        raise InvalidPICSParsingError(f'Unknown token: {token}')

    def __evaluate_sub_expression(self, tokens: List[str], pics: dict):
        token = tokens[self.__expression_index]
        if token == '(':
            self.__expression_index += 1
            expr = self.__evaluate_expression(tokens, pics)
            if tokens[self.__expression_index] != ')':
                raise KeyError('Missing ")"')

            self.__expression_index += 1
            return expr

        if token == '!':
            self.__expression_index += 1
            expr = self.__evaluate_sub_expression(tokens, pics)
            return not expr

        token = self.__normalize(token)
        self.__expression_index += 1

        if pics.get(token) is None:
            # By default, let's consider that if a PICS item is not defined, it is |false|.
            # It allows to create a file that only contains enabled features.
            return False

        return pics.get(token)

    def __tokenize(self, expression: str):
        token = ''
        tokens = []

        for c in expression:
            if c == ' ' or c == '\t' or c == '\n':
                pass
            elif c == '(' or c == ')' or c == '!':
                if token:
                    tokens.append(token)
                    token = ''
                tokens.append(c)
            elif c == '&' or c == '|':
                if token and token[-1] == c:
                    token = token[:-1]
                    if token:
                        tokens.append(token)
                        token = ''
                    tokens.append(c + c)
                else:
                    token += c
            else:
                token += c

        if token:
            tokens.append(token)
            token = ''

        return tokens

    def __preprocess_input(self, value: str):
        value = self.__remove_comments(value)
        value = self.__remove_control_characters(value)
        value = self.__remove_whitespaces(value)
        value = self.__make_lowercase(value)
        return value

    def __remove_comments(self, value: str) -> str:
        return value if not value else value.split(_COMMENT_CHARACTER, 1)[0]

    def __remove_control_characters(self, value: str) -> str:
        return ''.join(c for c in value if unicodedata.category(c)[0] != _CONTROL_CHARACTER_IDENTIFIER)

    def __remove_whitespaces(self, value: str) -> str:
        return value.replace(' ', '')

    def __make_lowercase(self, value: str) -> str:
        return value.lower()

    def __normalize(self, token: str):
        # Convert to all-lowercase so people who mess up cases don't have things
        # break on them in subtle ways.
        token = self.__make_lowercase(token)

        # TODO strip off "(Additional Context)" bits from the end of the code.
        return token
