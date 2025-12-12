#
#    Copyright (c) 2025 Project CHIP Authors
#    All rights reserved.
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
import importlib
import importlib.resources as pkg_resources
import os
import zipfile
from enum import Enum, auto
from importlib.abc import Traversable
from typing import Union


class CredentialSource(Enum):
    kDevelopment = auto()
    kProduction = auto()

    @property
    def dirname(self):
        if self == CredentialSource.kDevelopment:
            return "development"
        if self == CredentialSource.kProduction:
            return "production"
        raise KeyError("Invalid enum: %r" % self)


class CertificatePurpose(Enum):
    kPAA = auto()
    kCD = auto()

    @property
    def dirname(self):
        if self == CertificatePurpose.kPAA:
            return "paa-root-certs"
        if self == CertificatePurpose.kCD:
            return "cd-certs"
        raise KeyError("Invalid enum: %r" % self)


def _get_credentials(credential_source: Union[CredentialSource, Traversable], certificate_purpose: CertificatePurpose) -> Traversable:
    # Early return if data_model_directory is already a Traversable type
    if not isinstance(credential_source, CredentialSource):
        # data_model_directory is a Traversable (e.g. pathlib.Path to an extracted root)
        # Return directly as per the docstring - it should already contain the correct directory structure
        return credential_source

    # If it's a prebuilt directory, build the path based on the requested certificates
    zip_file_traversable = pkg_resources.files(importlib.import_module('matter.testing')).joinpath(
        'credentials').joinpath(credential_source.dirname).joinpath('allfiles.zip')

    # Avoid returning a zipfile.Path backed by a closed file handle. Build Path from the filesystem path
    # so the ZipFile lifecycle is managed by zipfile.Path itself.
    # mypy: Traversable does not declare __fspath__, but runtime object from importlib.resources
    # is a FileSystem resource that implements it. Safe to coerce for zipfile.Path usage.
    zip_path = os.fspath(zip_file_traversable)  # type: ignore[call-overload]
    zip_root = zipfile.Path(zip_path)
    return zip_root / certificate_purpose.dirname


def get_paa_root_certs(credential_source: Union[CredentialSource, Traversable]) -> Traversable:
    """
    Get the directory of the PAA root certs of a specific source from either a directory or stored zip.

    `credential_source` given as a path MUST be of type Traversable (often `pathlib.Path(somepathstring)`).
    """
    return _get_credentials(credential_source, CertificatePurpose.kPAA)


def get_cd_certs(credential_source: Union[CredentialSource, Traversable]) -> Traversable:
    """
    Get the directory of the CD certs of a specific source from either a directory or stored zip.

    `credential_source` given as a path MUST be of type Traversable (often `pathlib.Path(somepathstring)`).
    """
    return _get_credentials(credential_source, CertificatePurpose.kCD)
