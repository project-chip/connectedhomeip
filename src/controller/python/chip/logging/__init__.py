#
# SPDX-FileCopyrightText: 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

import logging

from chip.logging.library_handle import _GetLoggingLibraryHandle
from chip.logging.types import LogRedirectCallback_t

# Defines match support/logging/Constants.h (LogCategory enum)
ERROR_CATEGORY_NONE = 0
ERROR_CATEGORY_ERROR = 1
ERROR_CATEGORY_PROGRESS = 2
ERROR_CATEGORY_DETAIL = 3


@LogRedirectCallback_t
def _RedirectToPythonLogging(category, module, message):

    module = module.decode('utf-8')
    message = message.decode('utf-8')

    logger = logging.getLogger('chip.%s' % module)

    if category == ERROR_CATEGORY_ERROR:
        logger.error("%s", message)
    elif category == ERROR_CATEGORY_PROGRESS:
        logger.info("%s", message)
    elif category == ERROR_CATEGORY_DETAIL:
        logger.debug("%s", message)
    else:
        # All logs are expected to have some reasonable category. This treats
        # unknonw/None as critical.
        logging.critical("%s", message)


def RedirectToPythonLogging():
    """Redireects CHIP logging to python logging module."""

    handle = _GetLoggingLibraryHandle()
    handle.pychip_logging_set_callback(_RedirectToPythonLogging)
