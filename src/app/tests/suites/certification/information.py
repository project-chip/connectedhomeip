#!/usr/bin/env python3
#
#    Copyright (c) 2021 Project CHIP Authors
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

import argparse
import sys
from enum import Enum
from os import listdir, path

import yaml

# Test status description:
#  * Missing: Tests has not been written yet (default)
#  * Pending: Tests are not running in CI, and all tests are disabled
#  * Partial: Tests are running in CI, but some tests are disabled
#  * Complete: Tests are running in CI


class TestStatus(Enum):
    missing = 1
    pending = 2
    partial = 3
    complete = 4


class ArgOptions(Enum):
    summary = 1
    unknown = 2
    missing = 3
    pending = 4
    partial = 5
    complete = 6


def checkPythonVersion():
    if sys.version_info[0] < 3:
        print('Must use Python 3. Current version is ' +
              str(sys.version_info[0]))
        exit(1)


def parseTestPlans(filepath):
    tests_names = []
    tests_statuses = []

    for name, test_plan in parseYaml(filepath)['Test Plans'].items():
        for section, tests in test_plan['tests'].items():
            for index, test in enumerate(tests):
                test_name = '_'.join([
                    'Test_TC',
                    test_plan['shortname'],
                    str(section),
                    str(index + 1)
                ])

                tests_names.append(test_name)
                tests_statuses.append(parseTestPlan(getPathFor(test_name)))

    return dict(zip(tests_names, tests_statuses))


def parseTestPlan(filepath):
    if not path.exists(filepath):
        return TestStatus.missing

    is_pending_test = True

    for test_definition in parseYaml(filepath)['tests']:
        if 'disabled' in test_definition:
            if is_pending_test is False:
                return TestStatus.partial
        else:
            is_pending_test = False

    if is_pending_test is True:
        return TestStatus.pending

    return TestStatus.complete


def parseYaml(filepath):
    with open(filepath) as file:
        return yaml.load(file, Loader=yaml.FullLoader)


def getPathFor(filename):
    return path.join(path.dirname(__file__), filename + '.yaml')


def printSummaryFor(name, summary):
    count = summary[name]
    total = summary['total']
    percent = round(count/total*100, 2)
    print(' * ' + name.ljust(10) + ': ' + str(count).rjust(3) +
          ' (' + str(percent).rjust(5) + '%)')


def printSummary(statuses):
    summary = {
        'total': len(statuses),
        'missings': sum(TestStatus.missing == status for status in statuses.values()),
        'pendings': sum(TestStatus.pending == status for status in statuses.values()),
        'partials': sum(TestStatus.partial == status for status in statuses.values()),
        'completes': sum(TestStatus.complete == status for status in statuses.values()),
    }

    print('Tests count: ', summary['total'])
    printSummaryFor('missings', summary)
    printSummaryFor('pendings', summary)
    printSummaryFor('partials', summary)
    printSummaryFor('completes', summary)


def printUnknown(statuses):
    filtered = list(filter(lambda name: name.startswith(
        'Test_TC_'), listdir(path.dirname(__file__))))
    dir_test_names = [path.splitext(name)[0] for name in filtered]

    known_test_names = [name for name in statuses]
    unknown_test_names = list(
        filter(lambda name: name not in known_test_names, dir_test_names))

    print('List of tests that are not part of the test plan:')
    for name in unknown_test_names:
        print(' *', name)


def printList(statuses, name):
    filtered = dict(
        filter(lambda item: TestStatus[name] == item[1], statuses.items()))

    print('List of tests with status:', name)
    for name in filtered:
        print(' *', name)


def main():
    checkPythonVersion()

    default_options = ArgOptions.summary.name
    default_choices = [name for name in ArgOptions.__members__]

    parser = argparse.ArgumentParser(
        description='Extract information from the set of certifications tests')
    parser.add_argument('-s', '--show', default=default_options, choices=default_choices,
                        help='The information that needs to be returned from the test set')
    args = parser.parse_args()

    statuses = parseTestPlans(getPathFor('tests'))

    if (ArgOptions.summary.name == args.show):
        printSummary(statuses)
    elif (ArgOptions.unknown.name == args.show):
        printUnknown(statuses)
    elif (args.show in ArgOptions.__members__):
        printList(statuses, args.show)
    else:
        parser.print_help()


if __name__ == '__main__':
    main()
