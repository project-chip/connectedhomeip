# Copyright (c) 2023 Project CHIP Authors
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
from pathlib import Path
import re
import yaml


@dataclass
class Metadata:
    py_script_path: str = None
    run: str = None
    app: str = None
    factoryreset: bool = False
    comissioning_method: str = None
    discriminator: int = None
    kvs: str = None
    storage_path: str = None
    on_network_commission: str = None
    passcode: int = None
    trace_to: str = None
    hex_arg: str = None
    endpoint: str = None
    manual_code: str = None
    bool_arg: str = None
    PICS: str = None
    tests: str = None
    int_arg: str = None
    trace_to_appjson: str = None
    trace_to_testjson: str = None
    trace_toperfetto: str = None

    def print(self):
        print(repr(self))


class Metadata_Reader:
    # This class will be initialized with
    # the name of the script folder and the environment
    # file name
    
    def __init__(self, script_folder, env_file_name):
        self.script_folder = script_folder
        self.env = self.__build_env_object__(env_file_name)


    # builds the environment object 
    def __build_env_object__(self, env_file_name):
        
        with open(env_file_name) as stream:
            try:
                env = yaml.safe_load(stream)
            except yaml.YAMLError as yaml_exception:
                raise yaml_exception

        return env

    # resolves the run arguments associated with the environment  
    def __resolve_env_vals__(self, metadata_dict):

        for run_arg in metadata_dict:

            run_arg_val = metadata_dict[run_arg]

            if not type(run_arg_val)==str or run_arg=="run":
                metadata_dict[run_arg]=run_arg_val
                continue
            
            if run_arg_val == None:
                continue

            sub_args = run_arg_val.split('/')
                        
            if len(sub_args)==1:
                run_arg_val=self.env.get(sub_args[0])
                
            elif len(sub_args)==2:
                run_arg_val=self.env.get(sub_args[0]).get(sub_args[1])

            # if a argument has been specified in the comment header
            # but can't be found in the env file, consider it to be
            # boolean value.
            if run_arg_val == None:
                run_arg_val = True

            if not self.__is_run_arg_valid__(run_arg_val):
                raise Exception(str(run_arg_val)+" is not a valid value for "+str(run_arg))

            metadata_dict[run_arg] = run_arg_val


    # determines if the defined run arguments are valid
    def __is_run_arg_valid__(self, run_arg_val):
        return True


    # reads the test script file and parses out the run arguments defined in the file
    def __parse_script__(self, py_script_path, runs_metadata):
        runs_def_ptrn=re.compile(r'^\s*#\s*test-runner-runs:\s*(.*)$')
        args_def_ptrn=re.compile(r'^\s*#\s*test-runner-run/([a-zA-Z0-9_]+)/(script|app):\s*(.*)$')

        runs_arg_lines = {}
        
    
        with open(py_script_path, 'r', encoding='utf8') as py_script:
            for line in py_script.readlines():
            
                runs_match = runs_def_ptrn.match(line.strip())
                args_match = args_def_ptrn.match(line.strip())
            
                if runs_match:
                    for run in runs_match.group(1).strip().split():
                        runs_arg_lines[run]=[]

                elif args_match:
                    runs_arg_lines[args_match.group(1)].append(args_match.group(3))

        for run in runs_arg_lines:
            metadata = Metadata()
            metadata_dict = vars(metadata)
            metadata_dict['py_script_path'] = str(py_script_path)
            metadata_dict['run'] = str(run)
            self.read_args(runs_arg_lines[run], metadata_dict)
            self.__resolve_env_vals__(metadata_dict)
            runs_metadata[str(metadata.py_script_path)+"+"+str(metadata.run)] = metadata


    # gets the run metadata associated with all the test scripts in a particular folder
    def get_runs_metadata(self):
        runs_metadata = {}
        for path in Path(self.script_folder).glob('*.py'):
            self.__parse_script__(path, runs_metadata)

        return runs_metadata
                
            
    # goes through run argument definition and extracts run arguments from it
    def read_args(self,run_args_lines,metadata_dict):
        for run_line in run_args_lines:
            for run_arg_word in run_line.strip().split():
                run_arg=run_arg_word.split('/',1)[0]
                if run_arg in metadata_dict:
                    metadata_dict[run_arg] = run_arg_word




    
