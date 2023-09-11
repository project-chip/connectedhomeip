#
# SPDX-FileCopyrightText: 2023 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0

from .yaml_loader import YamlLoader


class TestConfigParser:
    """
    TestConfigParser exposes a single static method for reading the config section
    of a given YAML test file as a dictionary.

    For example a command line tool can use it to dynamically add extra options
    but using test specific options in addition to the generic options.
    """

    def get_config(test_file: str):
        config_options = {}

        yaml_loader = YamlLoader()
        _, _, _, config, _ = yaml_loader.load(test_file)
        config_options = {key: value if not isinstance(
            value, dict) else value['defaultValue'] for key, value in config.items()}
        return config_options
