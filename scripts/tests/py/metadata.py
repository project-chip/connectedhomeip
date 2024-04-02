#!/usr/local/bin/python3
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

from dataclasses import dataclass
import re
import yaml
from typing import Optional
from typing import Dict
from typing import List
from typing import Union

@dataclass
class Metadata:
    py_script_path: Optional[str] = None
    run: Optional[str] = None
    app: Optional[str] = None
    factoryreset: Optional[bool] = False
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


class Metadata_Reader:
    """
    A class to parse run arguments from the test scripts and 
    resolve them to environment specific values.

    Attributes:
    
    env: str
       A dictionary that reprsents a environment configuration in
       YAML format. 
    """
    
    def __init__(self, env_file_name: str):
        """
        Constructs the environment object

        Parameters:
        
        env_file_name: str
          Path to the environment file that contains the YAML configuration.
        """
        self.env = self.__build_env_object__(env_file_name)
        


    def __build_env_object__(self, env_yaml_file_path: str) -> Dict[str, Union[str, int, Dict]]:
        """
        Reads a YAML file and converts it into a dictionary

        Parameters:

        env_yaml_file_path: str
          path to the YAML file that contains environment configuration

        Returns:
         Dict - a Python Dictionary representation of the environment YAML file
        
        """
        with open(env_yaml_file_path) as stream:
            return yaml.safe_load(stream)

    
    def __resolve_env_vals__(self, metadata_dict: Dict[str, str]) -> None:
        """
        Resolves the argument defined in the test script to environment values.
        For example, if a test script defines "all_clusters" as the value for app
        name, we will check the environment configuration to see what raw value is
        assocaited with the "all_cluster" variable and set the value for "app" option
        to this raw value.

        Parameter:
        
        metadata_dict: Dict[str, str]
          Dictionary where each key represent a particular argument and its value represent
          the value for that argument defined in the test script.

        Return:

        None
        """

        for run_arg,run_arg_val in metadata_dict.items():

            if not type(run_arg_val)==str or run_arg=="run":
                metadata_dict[run_arg]=run_arg_val
                continue
            
            if run_arg_val is None:
                continue

            sub_args = run_arg_val.split('/')
            
            if len(sub_args) not in [1,2]:
                err = """The argument is not in the correct format. 
                The argument must follow the format of arg1 or arg1/arg2. 
                For example, arg1 represents the argument type and optionally arg2 
                represents a specific variable defined in the environment file whose
                value should be used as the argument value. If arg2 is not specified,
                we will just use the first value associated with arg1 in the environment file."""
                raise Exception(err)
                        
            if len(sub_args)==1:
                run_arg_val=self.env.get(sub_args[0])
                
            elif len(sub_args)==2:
                run_arg_val=self.env.get(sub_args[0]).get(sub_args[1])

            # if a argument has been specified in the comment header
            # but can't be found in the env file, consider it to be
            # boolean value.
            if run_arg_val is None:
                run_arg_val = True

            metadata_dict[run_arg] = run_arg_val

                
            
    def __read_args__(self,run_args_lines: List[str],metadata_dict: Dict[str, str]) -> None:
        """
        Parses a list of lines and extracts argument
        values from it.

        Parameters:
        
        run_args_lines: List[str]
          Raw lines in argument header

        metadata_dict: Dict[str, str]
          Dictionary where the extracted arguments will be stored.
          This represents the side effect of this function.

        Return:
        None

        """
        for run_line in run_args_lines:
            for run_arg_word in run_line.strip().split():
                run_arg=run_arg_word.split('/',1)[0]
                if run_arg in metadata_dict:
                    metadata_dict[run_arg] = run_arg_word


    def parse_script(self, py_script_path: str) -> List[Metadata]:
        """
        Parses a script and returns a list of metadata object where
        each element of that list representing run arguments associated
        with a particular run.

        Parameter:
        
        py_script_path: str
          path to the python test script

        Return:
        
        List[Metadata]
          List of Metadata object where each Metadata element represents
          the run arguments associated with a particular run defined in
          the script file.
        """
        
        runs_def_ptrn=re.compile(r'^\s*#\s*test-runner-runs:\s*(.*)$')
        args_def_ptrn=re.compile(r'^\s*#\s*test-runner-run/([a-zA-Z0-9_]+):\s*(.*)$')

        runs_arg_lines = {}
        runs_metadata = []
        
    
        with open(py_script_path, 'r', encoding='utf8') as py_script:
            for line in py_script.readlines():
            
                runs_match = runs_def_ptrn.match(line.strip())
                args_match = args_def_ptrn.match(line.strip())
            
                if runs_match:
                    for run in runs_match.group(1).strip().split():
                        runs_arg_lines[run]=[]

                elif args_match:
                    runs_arg_lines[args_match.group(1)].append(args_match.group(2))

        for run in runs_arg_lines:
            metadata = Metadata()
            metadata_dict = vars(metadata)
            self.__read_args__(runs_arg_lines[run], metadata_dict)
            self.__resolve_env_vals__(metadata_dict)

            # store the run value and script location in the
            # metadata object
            metadata_dict['py_script_path'] = str(py_script_path)
            metadata_dict['run'] = str(run)
            
            runs_metadata.append(metadata)

        return runs_metadata






    
