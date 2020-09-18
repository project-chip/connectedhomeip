#!/usr/bin/env python3

#
#    Copyright (c) 2020 Project CHIP Authors
#    Copyright (c) 2016-2017 Nest Labs, Inc.
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

#
#    @file
#       Implements utilities to parse standard content output by the
#       Chip standalone test clients and servers (resource stats, leak
#       detection, fault-injection, etc)
#

import sys
import re
import pprint
import traceback
import unittest

import happy.HappyStateDelete


from happy.Utils import *


def cleanup_after_exception():
    print("Deleting Happy state..")
    opts = happy.HappyStateDelete.option()
    state_delete = happy.HappyStateDelete.HappyStateDelete(opts)
    state_delete.run()
    print("Happy state deleted.")


def run_unittest():
    """
    Wrapper for unittest.main() that ensures the Happy state is deleted in case
    of failure or error.
    This is meant to be used with mock-to-mock test cases that always delete the topology they create.
    If the test case can reuse an existing topology and therefore does not always delete the topology,
    it should handle exceptions in setUp and tearDown explicitly.
    Unittest traps all exceptions but not KeyboardInterrupt.
    So, a KeyboardInterrupt will cause the Happy state not to be deleted.
    An exception raised during a test results in the test ending with an "error"
    as opposed to a "failure" (see the docs for more details). tearDown is invoked
    in that case.
    If an exception is raised during setUp, tearDown is not invoked, and so the
    Happy state is not cleaned up.
    Note that an invocation of sys.exit() from the Happy code results in an
    exception itself (and then it depends if that is happening during setUp,
    during the test, or during tearDown).
    So,
      1. we must cleanup in case of KeyboardInterrupt, and
      2. to keep it simple, we cleanup in case of any SystemExit that carries
         an error (we don't care if unittest was able to run tearDown or not).
    """
    try:
        unittest.main()

    except KeyboardInterrupt:
        print("\n\nChipUtilities.run_unittest caught KeyboardInterrupt")
        cleanup_after_exception()
        raise

    except SystemExit as e:
        if e.args[0] not in [0, False]:
            print(
                "\n\nChipUtilities.run_unittest caught some kind of test error or failure")
            cleanup_after_exception()
        raise e
