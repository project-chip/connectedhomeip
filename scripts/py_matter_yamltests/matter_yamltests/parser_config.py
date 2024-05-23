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
