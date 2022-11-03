# SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0


"""The idl package."""

import setuptools  # type: ignore

setuptools.setup(
    name='idl',
    version='0.0.1',
    author='Project CHIP Authors',
    description='Parse matter idl files',
    packages=setuptools.find_packages(),
    package_data={'idl': ['py.typed']},
    zip_safe=False,
)
