# Copyright (c) 2023 Texas Instruments Incorporated
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


"""A helper to download github artifacts

This script downloads the zip archive of pre-built libraries for linking with
the ot-ti software.

Run with:
    python download_openthread_library.py <ot-ti github repo> <version> <output directory> <Expected outputs>
"""

import urllib.request
import zipfile
import sys
import os

github_repo      = sys.argv[1]
version          = sys.argv[2]
root_out_dir     = sys.argv[3]
expected_outputs = sys.argv[4]

url = github_repo + '/releases/download/' + version + '/gitrelease-openthread-ti-' + version + '.zip'

if os.path.isdir(expected_outputs):
    if not os.listdir(expected_outputs):
        print("Downloading.. \n" + url)
        fh, _ = urllib.request.urlretrieve(url)
        fo = zipfile.ZipFile(fh, 'r')
        fo.extractall(path=root_out_dir)
    else:    
        print("Thread libraries already present at: \n" + expected_outputs + "\nSkipping download..")
else:
    print("Error: Download destination doesn't exist\n" + expected_outputs)
    sys.exit(2)
