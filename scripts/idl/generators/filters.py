# SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0

import logging
import stringcase


def RegisterCommonFilters(filtermap):
    """
    Register filters that are NOT considered platform-generator specific.

    Codegen often needs standardized names, like "method names are CamelCase"
    or "command names need-to-be-spinal-case" so these filters are often
    generally registered on all generators.
    """

    # General casing for output naming
    filtermap['camelcase'] = stringcase.camelcase
    filtermap['capitalcase'] = stringcase.capitalcase
    filtermap['constcase'] = stringcase.constcase
    filtermap['pascalcase'] = stringcase.pascalcase
    filtermap['snakecase'] = stringcase.snakecase
    filtermap['spinalcase'] = stringcase.spinalcase
