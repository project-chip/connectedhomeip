# Copyright (c) 2024 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import re
from dataclasses import dataclass
from typing import Dict, List, Optional, Union

import yaml


@dataclass
class Metadata:
    py_script_path: Optional[str] = None
    run: Optional[str] = None
    app: Optional[str] = None
    factoryreset: bool = False
    comissioning_method: Optional[str] = None
    discriminator: Optional[int] = None
    kvs: Optional[str] = None
    storage_path: Optional[str] = None
    on_network_commission: Optional[str] = None
    passcode: Optional[int] = None
    endpoint: Optional[str] = None
    manual_code: Optional[str] = None
    PICS: Optional[str] = None
    tests: Optional[str] = None

    def copy_from_dict(self, attr_dict: Dict[str, str]) -> None:
        """
        Sets the value of the attributes from a dictionary.

        Attributes:

        attr_dict:
          Dictionary that stores attributes value that should
          be transferred to this class.
        """

        if "app" in attr_dict:
            self.app = attr_dict["app"]

        if "run" in attr_dict:
            self.run = attr_dict["run"]

        if "discriminator" in attr_dict:
            self.discriminator = attr_dict["discriminator"]

        if "passcode" in attr_dict:
            self.passcode = attr_dict["passcode"]

        if "py_script_path" in attr_dict:
            self.py_script_path = attr_dict["py_script_path"]

        # TODO - set other attributes as well


class MetadataReader:
    """
    A class to parse run arguments from the test scripts and 
    resolve them to environment specific values.
    """

    def __init__(self, env_yaml_file_path: str):
        """
        Reads the YAML file and Constructs the environment object

        Parameters:

        env_yaml_file_path:
          Path to the environment file that contains the YAML configuration.
        """
        with open(env_yaml_file_path) as stream:
            self.env = yaml.safe_load(stream)

    def __resolve_env_vals__(self, metadata_dict: Dict[str, Union[str, bool]]) -> None:
        """
        Resolves the argument defined in the test script to environment values.
        For example, if a test script defines "all_clusters" as the value for app
        name, we will check the environment configuration to see what raw value is
        assocaited with the "all_cluster" variable and set the value for "app" option
        to this raw value.

        Parameter:

        metadata_dict:
          Dictionary where each key represent a particular argument and its value represent
          the value for that argument defined in the test script.
        """

        for run_arg, run_arg_val in metadata_dict.items():

            if not type(run_arg_val) == str or run_arg == "run":
                metadata_dict[run_arg] = run_arg_val
                continue

            if run_arg_val is None:
                continue

            sub_args = run_arg_val.split('/')

            if len(sub_args) not in [1, 2]:
                err = """The argument is not in the correct format. 
                The argument must follow the format of arg1 or arg1/arg2. 
                For example, arg1 represents the argument type and optionally arg2 
                represents a specific variable defined in the environment file whose
                value should be used as the argument value. If arg2 is not specified,
                we will just use the first value associated with arg1 in the environment file."""
                raise Exception(err)

            if len(sub_args) == 1:
                run_arg_val = self.env.get(sub_args[0])

            elif len(sub_args) == 2:
                run_arg_val = self.env.get(sub_args[0]).get(sub_args[1])

            # if a argument has been specified in the comment header
            # but can't be found in the env file, consider it to be
            # boolean value.
            if run_arg_val is None:
                run_arg_val = True

            metadata_dict[run_arg] = run_arg_val

    def __read_args__(self, run_args_lines: List[str]) -> Dict[str, str]:
        """
        Parses a list of lines and extracts argument
        values from it.

        Parameters:

        run_args_lines:
          Line in test script header that contains run argument definition.
          Each line will contain a list of run arguments separated by a space.
          Line below is one example of what the run argument line will look like:
          "app/all-clusters discriminator KVS storage-path"

          In this case the line defines that app, discriminator, KVS, and storage-path
          are the arguments that should be used with this run.

          An argument can be defined multiple times in the same line or in different lines.
          The last definition will override any previous definition. For example,
          "KVS/kvs1 KVS/kvs2 KVS/kvs3" line will lead to KVS value of kvs3.
        """
        metadata_dict={}
        
        for run_line in run_args_lines:
            for run_arg_word in run_line.strip().split():
                '''
                We expect the run arg to be defined in one of the 
                following two formats:
                1. run_arg
                2. run_arg/run_arg_val

                Examples: "discriminator" and "app/all_clusters"

                '''
                run_arg = run_arg_word.split('/', 1)[0]
                metadata_dict[run_arg] = run_arg_word

        return metadata_dict

    def parse_script(self, py_script_path: str) -> List[Metadata]:
        """
        Parses a script and returns a list of metadata object where
        each element of that list representing run arguments associated
        with a particular run.

        Parameter:

        py_script_path:
          path to the python test script

        Return:

        List[Metadata]
          List of Metadata object where each Metadata element represents
          the run arguments associated with a particular run defined in
          the script file.
        """

        runs_def_ptrn = re.compile(r'^\s*#\s*test-runner-runs:\s*(.*)$')
        args_def_ptrn = re.compile(r'^\s*#\s*test-runner-run/([a-zA-Z0-9_]+):\s*(.*)$')

        runs_arg_lines = {}
        runs_metadata = []

        with open(py_script_path, 'r', encoding='utf8') as py_script:
            for line in py_script.readlines():

                runs_match = runs_def_ptrn.match(line.strip())
                args_match = args_def_ptrn.match(line.strip())

                if runs_match:
                    for run in runs_match.group(1).strip().split():
                        runs_arg_lines[run] = []

                elif args_match:
                    runs_arg_lines[args_match.group(1)].append(args_match.group(2))

        for run, line in runs_arg_lines.items():
            metadata_dict = self.__read_args__(line)
            self.__resolve_env_vals__(metadata_dict)

            # store the run value and script location in the
            # metadata object
            metadata_dict['py_script_path'] = py_script_path
            metadata_dict['run'] = run

            metadata = Metadata()

            metadata.copy_from_dict(metadata_dict)
            runs_metadata.append(metadata)

        return runs_metadata