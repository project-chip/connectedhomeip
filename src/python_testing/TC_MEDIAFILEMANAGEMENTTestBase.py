#
#    Copyright (c) 2026 Project CHIP Authors
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

"""Shared helpers for the Media File Management cluster (0x0511) test cases.

Reading TotalStorage, AvailableStorage and AvailableFiles requires Manage privilege, and
AddFile / DeleteFile require Manage to invoke, so every test in this suite expects the TH
to hold at least Manage on the endpoint under test. The default commissioning controller
is an Administrator, which subsumes Manage.

Several commands cause the DUT to pull data from the TH over BDX. ``serve_bdx_upload``
wraps that flow: it arms the controller to serve one transfer, waits for the DUT to
initiate it, and always cancels the future when nothing arrives so that a later transfer
in the same run is not blocked.
"""

import asyncio
import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.bdx import BdxTransfer
from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError, Status
from matter.testing import matter_asserts

log = logging.getLogger(__name__)

_CLUSTER = Clusters.MediaFileManagement

# FileDescriptionStruct / command field constraints from the cluster specification.
MAX_NAME_LENGTH = 128
MAX_MIME_TYPE_LENGTH = 256
MAX_IMAGE_URI_LENGTH = 256
MAX_CLIENT_NAME_LENGTH = 128
MAX_AVAILABLE_FILES = 256
MAX_SUPPORTED_MIME_TYPES = 64

# A MIME type that no conformant DUT should advertise, used for the negative MIME cases.
UNSUPPORTED_MIME_TYPE = "application/vnd.matter.test-unsupported"


class MEDIAFILEMANAGEMENTTestBase:

    # Cluster capability discovery

    async def read_mfm_capabilities(self, endpoint) -> None:
        """Read AttributeList and FeatureMap, caching them for the supports_* helpers."""
        self._mfm_attribute_list = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=_CLUSTER, attribute=_CLUSTER.Attributes.AttributeList)
        self._mfm_feature_map = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=_CLUSTER, attribute=_CLUSTER.Attributes.FeatureMap)

    def supports_mfm_attribute(self, attribute) -> bool:
        return attribute.attribute_id in self._mfm_attribute_list

    @property
    def supports_media_sharing(self) -> bool:
        return bool(self._mfm_feature_map & _CLUSTER.Bitmaps.Feature.kMediaSharing)

    # Attribute reads

    async def read_mfm_attribute_expect_success(self, endpoint, attribute):
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=_CLUSTER, attribute=attribute)

    async def read_available_files(self, endpoint) -> list:
        return await self.read_mfm_attribute_expect_success(endpoint, _CLUSTER.Attributes.AvailableFiles)

    async def read_available_storage(self, endpoint) -> int:
        return await self.read_mfm_attribute_expect_success(endpoint, _CLUSTER.Attributes.AvailableStorage)

    async def read_supported_mime_types(self, endpoint) -> list:
        return await self.read_mfm_attribute_expect_success(endpoint, _CLUSTER.Attributes.SupportedMimeTypes)

    @staticmethod
    def find_file_by_id(files: list, file_id: int):
        """Return the FileDescriptionStruct with this FileID, or None."""
        return next((entry for entry in files if entry.fileID == file_id), None)

    @staticmethod
    def verify_file_description(file_description, label: str) -> None:
        """Assert a FileDescriptionStruct satisfies its specification constraints."""
        matter_asserts.assert_valid_uint64(file_description.fileID, f"{label}.FileID")
        matter_asserts.assert_is_string(file_description.name, f"{label}.Name")
        matter_asserts.assert_string_length(file_description.name, f"{label}.Name", max_length=MAX_NAME_LENGTH)
        matter_asserts.assert_valid_uint64(file_description.size, f"{label}.Size")
        matter_asserts.assert_is_string(file_description.mimeType, f"{label}.MimeType")
        matter_asserts.assert_string_length(
            file_description.mimeType, f"{label}.MimeType", max_length=MAX_MIME_TYPE_LENGTH)
        matter_asserts.assert_is_string(file_description.imageUri, f"{label}.ImageURI")
        matter_asserts.assert_string_length(
            file_description.imageUri, f"{label}.ImageURI", max_length=MAX_IMAGE_URI_LENGTH)

    # MIME type selection

    @staticmethod
    def pick_supported_mime_type(supported_mime_types: list, fallback: str) -> str:
        """Return a MIME type the DUT accepts; an empty list means no restriction."""
        return supported_mime_types[0] if supported_mime_types else fallback

    @staticmethod
    def pick_unsupported_mime_type(supported_mime_types: list) -> str | None:
        """Return a MIME type the DUT does not advertise, or None when it restricts nothing.

        An empty SupportedMimeTypes list means the DUT imposes no restriction, so no
        negative case can be constructed.
        """
        if not supported_mime_types:
            return None
        if UNSUPPORTED_MIME_TYPE not in supported_mime_types:
            return UNSUPPORTED_MIME_TYPE
        for index in range(100):
            candidate = f"{UNSUPPORTED_MIME_TYPE}-{index}"
            if candidate not in supported_mime_types:
                return candidate
        return None

    # BDX

    def bdx_uri(self, designator: str) -> str:
        """Build a bdx: URI pointing at the TH, as required for the ImageURI fields.

        The specification requires the bdx: scheme pointing at the client's node id; the
        OTA Provider cluster fixes the shape as bdx://<16 upper-case hex digits>/<designator>.
        """
        return f"bdx://{self.default_controller.nodeId:016X}/{designator}"

    async def serve_bdx_upload(self, data: bytes, timeout_sec: float = 30.0,
                               send_command=None) -> BdxTransfer.InitMessage | None:
        """Serve one BDX transfer that the DUT initiates in order to pull ``data``.

        ``send_command`` is the coroutine that triggers the DUT. It is started before the
        wait because a DUT may open the transfer before its command response is delivered,
        and it is always awaited to completion so its result or exception is not lost.

        Returns the transfer's init message, or None if the DUT never started a transfer.
        """
        bdx_future = self.default_controller.TestOnlyPrepareToSendBdxData(data)
        command_task = asyncio.ensure_future(send_command) if send_command is not None else None
        try:
            waitables = [bdx_future] + ([command_task] if command_task else [])
            done, _ = await asyncio.wait(waitables, timeout=timeout_sec,
                                         return_when=asyncio.FIRST_COMPLETED)
            # The command response often lands first; keep waiting out the budget for the
            # transfer itself rather than giving up on the first completion.
            if bdx_future not in done:
                await asyncio.wait([bdx_future], timeout=timeout_sec)

            init_message = None
            if bdx_future.done() and not bdx_future.cancelled():
                transfer: BdxTransfer.BdxTransfer = bdx_future.result()
                init_message = transfer.init_message
                log.info("DUT initiated a BDX transfer for designator %r (length %s)",
                         init_message.FileDesignator, init_message.Length)
                await transfer.accept_and_send_data()
            else:
                log.info("The DUT did not initiate a BDX transfer within %.0fs", timeout_sec)
            return init_message
        finally:
            # The controller requires an unused future to be cancelled so it does not
            # interfere with a later transfer in the same run.
            if not bdx_future.done():
                bdx_future.cancel()
            if command_task is not None:
                # Awaiting here rather than cancelling keeps the command's response (or its
                # error) available to the caller, which asserts on it.
                await command_task

    # Commands

    async def send_add_file(self, endpoint, name: str, size: int, mime_type: str, image_uri: str):
        """Send AddFile and return the AddFileResponse."""
        return await self.send_single_cmd(
            cmd=_CLUSTER.Commands.AddFile(name=name, size=size, mimeType=mime_type, imageUri=image_uri),
            endpoint=endpoint)

    async def send_delete_file(self, endpoint, file_id: int) -> None:
        """Send DeleteFile, expecting success."""
        await self.send_single_cmd(cmd=_CLUSTER.Commands.DeleteFile(fileID=file_id), endpoint=endpoint)

    async def send_delete_file_expect_failure(self, endpoint, file_id: int) -> InteractionModelError:
        """Send DeleteFile for a FileID that does not exist and return the resulting error.

        The specification's Effect on Receipt requires NOT_FOUND, and the reference
        implementation returns that, while the test plan text names the cluster-specific
        InvalidFileID (2). Either is accepted here and the observed one is logged, so the
        test does not pick a side in that discrepancy.
        """
        try:
            await self.send_single_cmd(cmd=_CLUSTER.Commands.DeleteFile(fileID=file_id), endpoint=endpoint)
            asserts.fail(f"DeleteFile with the non-existent FileID {file_id} should not have succeeded")
        except InteractionModelError as e:
            accepted = (e.status == Status.NotFound
                        or (e.hasClusterStatus and e.clusterStatus == _CLUSTER.Enums.FileStatusEnum.kInvalidFileID))
            asserts.assert_true(
                accepted,
                f"DeleteFile for a non-existent FileID returned {e}; expected NOT_FOUND (per the specification's "
                f"Effect on Receipt) or cluster status InvalidFileID (2) (per the test plan)")
            log.info("DeleteFile for a non-existent FileID was rejected with %s", e)
            return e

    async def send_offer_file(self, endpoint, client_name: str, name: str, size: int, mime_type: str,
                              image_uri: str) -> None:
        await self.send_single_cmd(
            cmd=_CLUSTER.Commands.OfferFile(clientName=client_name, name=name, size=size,
                                            mimeType=mime_type, imageUri=image_uri),
            endpoint=endpoint)

    def offer_file_command(self, client_name: str, name: str, size: int, mime_type: str, image_uri: str):
        return _CLUSTER.Commands.OfferFile(clientName=client_name, name=name, size=size,
                                           mimeType=mime_type, imageUri=image_uri)

    async def send_request_shared_files(self, endpoint, client_name: str, request_id: int,
                                        supported_mime_types=None) -> None:
        await self.send_single_cmd(
            cmd=_CLUSTER.Commands.RequestSharedFiles(
                clientName=client_name, requestID=request_id,
                supportedMimeTypes=supported_mime_types if supported_mime_types is not None else NullValue),
            endpoint=endpoint)

    async def send_get_shared_file(self, endpoint, response_id: int):
        """Send GetSharedFile and return the GetSharedFileResponse."""
        return await self.send_single_cmd(
            cmd=_CLUSTER.Commands.GetSharedFile(responseID=response_id), endpoint=endpoint)
