#!/usr/bin/env python3

# Copyright (c) 2022 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import logging
import os
from typing import Optional

try:
    from matter_idl.zapxml import ParseSource, ParseXmls
except ImportError:
    import sys

    sys.path.append(os.path.abspath(
        os.path.join(os.path.dirname(__file__), '..')))
    from matter_idl.zapxml import ParseSource, ParseXmls


if __name__ == '__main__':
    import click
    from matter_idl.generators import GeneratorStorage
    from matter_idl.generators.idl import IdlGenerator

    class InMemoryStorage(GeneratorStorage):
        def __init__(self):
            super().__init__()
            self.content: Optional[str] = None

        def get_existing_data(self, relative_path: str):
            # Force re-generation each time
            return None

        def write_new_data(self, relative_path: str, content: str):
            if self.content:
                raise Exception(
                    "Unexpected extra data: single file generation expected")
            self.content = content

    # Supported log levels, mapping string values required for argument
    # parsing into logging constants
    __LOG_LEVELS__ = {
        'debug': logging.DEBUG,
        'info': logging.INFO,
        'warn': logging.WARN,
        'fatal': logging.FATAL,
    }

    @ click.command()
    @ click.option(
        '--log-level',
        default='INFO',
        show_default=True,
        type=click.Choice(list(__LOG_LEVELS__.keys()), case_sensitive=False),
        help='Determines the verbosity of script output.')
    @ click.option(
        '--no-print',
        show_default=True,
        default=False,
        is_flag=True,
        help='Do not pring output data (parsed data)')
    @ click.argument('filenames', nargs=-1)
    def main(log_level, no_print, filenames):
        logging.basicConfig(
            level=__LOG_LEVELS__[log_level],
            format='%(asctime)s %(levelname)-7s %(message)s',
        )

        logging.info("Starting to parse ...")

        sources = [ParseSource(source=name) for name in filenames]
        data = ParseXmls(sources)
        logging.info("Parse completed")

        if not no_print:
            storage = InMemoryStorage()
            IdlGenerator(storage=storage, idl=data).render(dry_run=False)
            print(storage.content)

    main(auto_envvar_prefix='CHIP')
