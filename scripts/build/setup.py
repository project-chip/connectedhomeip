# SPDX-FileCopyrightText: 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0


"""The build_examples package."""

import setuptools  # type: ignore

setuptools.setup(
    name='build_examples',
    version='0.0.1',
    author='Project CHIP Authors',
    description='Generate build commands for the CHIP SDK Examples',
    packages=setuptools.find_packages(),
    package_data={'build_examples': ['py.typed']},
    install_requires=[
        'click >= 6.7.0',
        'coloredlogs >= 7.3.0'
    ],
    zip_safe=False,
)
