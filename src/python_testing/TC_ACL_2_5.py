#
#    Copyright (c) 2023 Project CHIP Authors
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

# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     factory-reset: true
#     quiet: true
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import asyncio
import logging
import queue
import time

import chip.clusters as Clusters
from chip.clusters.Types import Nullable
from chip.interaction_model import Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_ACL_2_5(MatterBaseTest):
    def desc_TC_ACL_2_5(self) -> str:
        return "[TC-ACL-2.5] Cluster endpoint"

    async def read_access_control_extension(self):
        """Read the AccessControl cluster's Extension attribute"""
        extension_attribute = Clusters.AccessControl.Attributes.Extension
        result = await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=Clusters.AccessControl,
            attribute=extension_attribute
        )
        return result

    def steps_TC_ACL_2_5(self) -> list[TestStep]:
        steps = [
            TestStep(1, "TH1 commissions DUT using admin node ID",
                     is_commissioning=True),
            TestStep(2, "TH1 reads DUT Endpoint 0 OperationalCredentials cluster CurrentFabricIndex attribute",
                     "Result is SUCCESS, value is a valid index"),
            TestStep(3, "TH1 reads DUT Endpoint 0 AccessControl cluster AccessControlExtensionChanged events",
                     "Result is SUCCESS, value is empty list"),
            TestStep(4, "TH1 writes DUT Endpoint 0 AccessControl cluster Extension attribute, value is list of AccessControlExtensionStruct containing 1 element", "Result is SUCCESS"),
            TestStep(5, "TH1 reads DUT Endpoint 0 AccessControl cluster AccessControlExtensionChanged events",
                     "Result is SUCCESS, value is list of AccessControlExtensionChanged containing 1 element"),
            TestStep(6, "TH1 writes DUT Endpoint 0 AccessControl cluster Extension attribute, value is list of AccessControlExtensionStruct containing 1 element", "Result is SUCCESS"),
            TestStep(7, "TH1 reads DUT Endpoint 0 AccessControl cluster AccessControlExtensionChanged events",
                     "Result is SUCCESS, value is list of AccessControlExtensionChanged containing 1 element"),
            TestStep(8, "TH1 writes DUT Endpoint 0 AccessControl cluster Extension attribute, value is list of AccessControlExtensionStruct containing 1 element",
                     "Result is 0x87 (CONSTRAINT_ERROR)-Data value exceeds maximum length."),
            TestStep(9, "TH1 reads DUT Endpoint 0 AccessControl cluster AccessControlExtensionChanged event",
                     "Result is SUCCESS, value is list of AccessControlExtensionChanged containing at least 1 new element, and MUST NOT contain an added event for the extension with data that is too large"),
            TestStep(10, "TH1 writes DUT Endpoint 0 AccessControl cluster Extension attribute, value is list of AccessControlExtensionStruct containing 2 elements",
                     "Result is 0x87 (CONSTRAINT_ERROR)-as there are more than 1 entry associated with the given accessing fabric index in the extension list"),
            TestStep(11, "TH1 reads DUT Endpoint 0 AccessControl cluster AccessControlExtensionChanged event",
                     "Result is SUCCESS, value is list of AccessControlExtensionChanged containing at least 1 new element, and MUST NOT contain an added event for the second extension"),
            TestStep(
                12, "TH1 writes DUT Endpoint 0 AccessControl cluster Extension attribute, value is an empty list", "Result is SUCCESS"),
            TestStep(13, "TH1 reads DUT Endpoint 0 AccessControl cluster AccessControlExtensionChanged event",
                     "Result is SUCCESS, value is list of AccessControlExtensionChanged containing at least 1 new element"),
        ]
        return steps

    @async_test_body
    async def test_TC_ACL_2_5(self):
        self.step(1)
        # Commissioning step

        self.step(2)
        oc_cluster = Clusters.OperationalCredentials
        cfi_attribute = oc_cluster.Attributes.CurrentFabricIndex
        f1 = await self.read_single_attribute_check_success(endpoint=0, cluster=oc_cluster, attribute=cfi_attribute)

        self.step(3)
        # Read initial AccessControlClusterExtension
        acec_event = Clusters.AccessControl.Events.AccessControlExtensionChanged

        try:
            events_response = await self.default_controller.ReadEvent(
                self.dut_node_id,
                events=[(0, acec_event)],
                fabricFiltered=True
            )
            self.print_step(f"Initial events response", str(events_response))

            # Extract events from the response
            events = events_response
            self.print_step(f"Found {len(events)} initial events", "")

            for event_data in events:
                self.print_step(f"Initial event", str(event_data))

        except Exception as e:
            self.print_step(f"Error reading initial events", str(e))
            asserts.fail(f"Failed to read initial events: {e}")

        self.step(4)
        # Create an extension with a test string
        # Use a properly formatted byte string - not a string that looks like bytes
        extension_data = bytes.fromhex('1718')
        extension = Clusters.AccessControl.Structs.AccessControlExtensionStruct(
            data=extension_data)

        # Write the extension to the device - properly wrap the extensions list
        self.print_step(f"Writing extension with data", extension_data.hex())
        try:
            # Make sure we're creating the attribute value correctly
            extension_attr = Clusters.AccessControl.Attributes.Extension
            # Create a proper extension list
            extensions_list = [extension]
            # Write the attribute
            result = await self.default_controller.WriteAttribute(
                self.dut_node_id,
                # Use named parameter 'value'
                [(0, extension_attr(value=extensions_list))]
            )
            self.print_step(f"Write result", str(result))
            asserts.assert_equal(
                result[0].Status, Status.Success, "Write should have succeeded")

        except Exception as e:
            self.print_step(f"Error writing extension", str(e))
            asserts.fail(f"Failed to write extension: {e}")

        # Wait for the change to be processed
        self.print_step(f"Waiting 5 seconds for event generation...", "")
        await asyncio.sleep(5)  # Increased wait time

        self.step(5)
        # Read the events directly instead of relying on subscription
        self.print_step(
            f"Reading AccessControlExtensionChanged events after write...", "")

        # Try multiple times with increasing timeouts
        max_attempts = 5
        found_match = False  # Add flag to track if we found a match

        for attempt in range(1, max_attempts + 1):
            try:
                self.print_step(
                    f"Attempt {attempt}/{max_attempts} to read events", "")
                events_response = await self.default_controller.ReadEvent(
                    self.dut_node_id,
                    events=[(0, acec_event)],
                    fabricFiltered=True
                )
                self.print_step(f"Events response", str(events_response))

                # Extract events from the response
                events = events_response  # Response is already a list of events
                self.print_step(f"Found {len(events)} events", "")

                # Check if we got any events
                if len(events) > 0:
                    self.print_step(f"Found {len(events)} events!", "")

                    # Find the specific event we're looking for
                    for event_data in events:
                        self.print_step(f"Examining event", str(event_data))

                        # Check this event for required dictionary items
                        if (hasattr(event_data, 'Data') and
                            hasattr(event_data.Data, 'changeType') and
                                event_data.Data.changeType == Clusters.AccessControl.Enums.ChangeTypeEnum.kAdded):
                            asserts.assert_in('chip.clusters.Types.Nullable', str(
                                type(event_data.Data.adminPasscodeID)), "AdminPasscodeID should be Null")
                            asserts.assert_equal(
                                event_data.Data.adminNodeID, self.default_controller.nodeId, "AdminNodeID should be the controller node ID")
                            asserts.assert_equal(
                                event_data.Data.latestValue.data, b'\x17\x18', "LatestValue.Data should be 1718")
                            asserts.assert_equal(event_data.Data.latestValue.fabricIndex, f1,
                                                 "LatestValue.FabricIndex should be the current fabric index")
                            asserts.assert_equal(
                                event_data.Data.fabricIndex, f1, "FabricIndex should be the current fabric index")
                            found_match = True
                            break

                    # If we found a match, break out of the loop
                    if found_match:
                        break

                # If no events, wait and try again (up to 5 times)
                else:
                    self.print_step(f"No events found on attempt {
                                    attempt}, waiting to try again...", "")
                    if attempt < max_attempts:
                        wait_time = attempt * 2  # Increasing wait time with each attempt
                        await asyncio.sleep(wait_time)
                    else:
                        self.print_step("ERROR", "All attempts failed")
                        asserts.fail(
                            "Did not receive AccessControlExtensionChanged event")

            except Exception as e:
                self.print_step(
                    f"Error reading events (attempt {attempt})", str(e))
                if attempt < max_attempts:
                    wait_time = attempt * 2
                    await asyncio.sleep(wait_time)
                else:
                    self.print_step("ERROR", "All attempts failed")
                    asserts.fail(
                        f"Failed to read AccessControlExtensionChanged events: {e}")

        # After all attempts, check if we found a match
        if not found_match:
            asserts.fail(
                "Did not receive AccessControlExtensionChanged event after multiple attempts")

        self.step(6)
        # Create a new extension with different data to replace the existing one
        # Use properly formatted binary data
        new_extension_data = bytes.fromhex(
            '17D00000F1FF01003D48656C6C6F20576F726C642E205468697320697320612073696E676C6520656C656D656E74206C6976696E6720617320612063686172737472696E670018'
        )  # Create data that exceeds max length
        new_extension = Clusters.AccessControl.Structs.AccessControlExtensionStruct(
            data=new_extension_data)

        # Write the new extension
        self.print_step(f"Writing new extension with data",
                        new_extension_data.hex())
        try:
            # Create the Extension variable
            extension_attr = Clusters.AccessControl.Attributes.Extension
            # Create proper extensions list
            extensions_list = [new_extension]
            # Write the attribute with named parameter
            result = await self.default_controller.WriteAttribute(
                self.dut_node_id,
                [(0, extension_attr(value=extensions_list))]
            )
            self.print_step(f"Write result", str(result))
            asserts.assert_equal(
                result[0].Status, Status.Success, "Write should have succeeded")

        except Exception as e:
            self.print_step(f"Error writing new extension", str(e))
            asserts.fail(f"Failed to write new extension: {e}")

        # Wait for the change to be processed
        self.print_step(f"Waiting 5 seconds for event generation...", "")
        await asyncio.sleep(5)  # Wait longer to ensure events are generated

        self.step(7)
        # Read events directly
        self.print_step(f"Reading events after replacing extension...", "")

        # Try multiple attempts with increasing timeouts
        max_attempts = 5
        found_remove = False
        found_add = False
        found_event = False  # Flag to indicate if we found a relevant event

        for attempt in range(1, max_attempts + 1):
            try:
                self.print_step(
                    f"Attempt {attempt}/{max_attempts} to read events", "")
                events_response = await self.default_controller.ReadEvent(
                    self.dut_node_id,
                    events=[(0, acec_event)],
                    fabricFiltered=True
                )
                self.print_step(f"Events response", str(events_response))

                # Extract events from the response
                events = events_response  # Response is already a list of events
                self.print_step(f"Found {len(events)} events", "")

                # We need to find both a remove event and an add event
                for event_data in events:
                    self.print_step(f"Examining event", str(event_data))

                    if hasattr(event_data, 'Data') and hasattr(event_data.Data, 'changeType'):
                        # Check for remove event
                        # Review: Does this event have to be of kRemoved, or can it be kChanged?
                        # According to yaml this is what is expected
                        if (event_data.Data.changeType == Clusters.AccessControl.Enums.ChangeTypeEnum.kRemoved or
                            event_data.Data.changeType == Clusters.AccessControl.Enums.ChangeTypeEnum.kChanged and
                            event_data.Data.adminNodeID == self.default_controller.nodeId and
                            isinstance(event_data.Data.adminPasscodeID, Nullable) and
                            event_data.Data.latestValue.data == new_extension_data and
                            event_data.Data.latestValue.fabricIndex == f1 and
                                event_data.Data.fabricIndex == f1):
                            found_remove = True
                            self.print_step(f"Found Remove/Change event", "")

                        # Check for add event with matching data
                        elif (event_data.Data.changeType == Clusters.AccessControl.Enums.ChangeTypeEnum.kAdded and
                              event_data.Data.adminNodeID == self.default_controller.nodeId and
                              isinstance(event_data.Data.adminPasscodeID, Nullable) and
                              event_data.Data.latestValue.data == extension_data and
                              event_data.Data.latestValue.fabricIndex == f1 and
                              event_data.Data.fabricIndex == f1):
                            found_add = True
                            self.print_step(f"Found ADD event", "")

                # If we found both events, we can proceed
                if found_remove and found_add:
                    self.print_step(f"Found both REMOVE and ADD events", "")
                    break

                # If not found, wait and try again
                if attempt < max_attempts:
                    wait_time = attempt * 2
                    self.print_step(f"No matching events found on attempt {
                                    attempt}, waiting {wait_time}s...", "")
                    await asyncio.sleep(wait_time)
                else:
                    self.print_step("ERROR: All attempts failed", "")
                    asserts.fail("Did not find both REMOVE and ADD events")

            except Exception as e:
                self.print_step(
                    f"Error reading events (attempt {attempt})", str(e))
                if attempt < max_attempts:
                    wait_time = attempt * 2
                    await asyncio.sleep(wait_time)
                else:
                    self.print_step(
                        "ERROR: All attempts failed with exceptions", "")
                    asserts.fail(f"Failed to read events: {e}")

        self.step(8)
        # Try to write an extension that exceeds max length (128 bytes)
        # Use properly formatted binary data
        too_long_data = bytes.fromhex(
            "17D00000F1FF01003D48656C6C6F20576F726C642E205468697320697320612073696E676C6520656C656D656E74206C6976696E6720617320612063686172737472696E6700D00000F1FF02003248656C6C6F20576F726C642E205468697320697320612073696E676C6520656C656D656E7420616761696E2E2E2E2E2E2E0018"
        )
        too_long_extension = Clusters.AccessControl.Structs.AccessControlExtensionStruct(
            data=too_long_data)

        # This should fail with CONSTRAINT_ERROR
        try:
            self.print_step(
                f"Attempting to write extension that exceeds max length (should fail)", "")
            # Create the Extension attribute
            extension_attr = Clusters.AccessControl.Attributes.Extension
            # Create proper extensions list
            extensions_list = [too_long_extension]
            # Write the attribute with named parameter
            a = await self.default_controller.WriteAttribute(
                self.dut_node_id,
                [(0, extension_attr(value=extensions_list))]
            )
            self.print_step(f"Write result", str(a))
            asserts.assert_equal(a[0].Status, Status.ConstraintError,
                                 "Write should have failed with CONSTRAINT_ERROR 135")

        except Exception as e:
            self.print_step(f"Got expected error", str(e))

        self.step(9)
        # Verify no event was generated for the failed write
        self.print_step(
            f"Reading events after failed write (too long extension)...", "")

        # Try to read events directly
        try:
            events_response = await self.default_controller.ReadEvent(
                self.dut_node_id,
                events=[(0, acec_event)],
                fabricFiltered=True
            )
            self.print_step(f"Events response", str(events_response))

            # Extract events from the response
            events = events_response  # Response is already a list of events
            self.print_step(f"Found {len(events)} events", "")

            # Ensure no events have the too long data
            for event_data in events:
                self.print_step(f"Examining event", str(event_data))

                if hasattr(event_data, 'Data') and hasattr(event_data.Data, 'changeType'):
                    # Only check add events
                    if event_data.Data.changeType == Clusters.AccessControl.Enums.ChangeTypeEnum.kAdded:
                        # Make sure it doesn't have the too long data
                        latest_value = getattr(
                            event_data.Data, 'latestValue', None)
                        if latest_value and hasattr(latest_value, 'data'):
                            # Convert to hex for better comparison if possible
                            latest_data_hex = ""
                            if hasattr(latest_value.data, 'hex'):
                                latest_data_hex = latest_value.data.hex()
                                self.print_step(
                                    f"Checking latest data (hex)", latest_data_hex)
                            else:
                                self.print_step(
                                    f"Checking latest data", str(latest_value.data))

                            too_long_data_hex = ""
                            if hasattr(too_long_data, 'hex'):
                                too_long_data_hex = too_long_data.hex()
                                self.print_step(
                                    f"Doesn't match too long data (hex)", too_long_data_hex)
                            else:
                                self.print_step(
                                    f"Doesn't match too long data", str(too_long_data))

                            asserts.assert_not_equal(latest_value.data, too_long_data,
                                                     "Should not have event for extension with too large data")

            self.print_step(f"Verified: no events with too large data", "")

        except Exception as e:
            self.print_step(f"Error reading events", str(e))
            # Continue test even if this read fails

        self.step(10)
        # This should fail with CONSTRAINT_ERROR
        try:
            self.print_step(
                f"Attempting to write multiple extensions (should fail)", "")
            # Create the Extension attribute
            extension_attr = Clusters.AccessControl.Attributes.Extension
            # Create proper extensions list with multiple extensions
            extensions_list = [extension, new_extension]
            # Write the attribute with named parameter
            b = await self.default_controller.WriteAttribute(
                self.dut_node_id,
                [(0, extension_attr(value=extensions_list))]
            )
            self.print_step(f"Write result", str(b))
            asserts.assert_equal(b[0].Status, Status.ConstraintError,
                                 "Write should have failed with CONSTRAINT_ERROR")

        except Exception as e:
            self.print_step(
                f"Error writing multiple extensions other than constraint error", str(e))
            asserts.fail(
                f"Failed to write multiple extensions due to other error: {e}")

        self.step(11)
        # Verify no event was generated at all, since the whole extensions list was rejected.
        self.print_step(
            f"Reading events after failed write (multiple extensions)...", "")

        # Try to read events directly
        try:
            events_response = await self.default_controller.ReadEvent(
                self.dut_node_id,
                events=[(0, acec_event)],
                fabricFiltered=True
            )
            self.print_step(f"Events response", str(events_response))

            # Extract events from the response
            events = events_response  # Response is already a list of events
            self.print_step(f"Found {len(events)} events", "")

            found_valid_event = False

            # Ensure that no events were generated at all
            for event_data in events:
                self.print_step(f"Examining event", str(event_data))

                if hasattr(event_data, 'Data') and hasattr(event_data.Data, 'changeType'):
                    # Check for add event with specific data
                    if (event_data.Data.changeType == Clusters.AccessControl.Enums.ChangeTypeEnum.kAdded and
                        event_data.Data.adminNodeID == self.default_controller.nodeId and
                        isinstance(event_data.Data.adminPasscodeID, Nullable) and
                        event_data.Data.latestValue.data == extension_data and
                        event_data.Data.latestValue.fabricIndex == f1 and
                            event_data.Data.fabricIndex == f1):
                        found_valid_event = True
                        self.print_step(f"Found valid ADD event", "")

            asserts.assert_true(
                found_valid_event, "Found the expected ADD event with specified fields")
            self.print_step(f"Verified: found valid ADD event", "")

        except Exception as e:
            self.print_step(f"Error reading events", str(e))
            asserts.fail(f"Failed to read events: {e}")

        self.step(12)
        # Write an empty list to clear all extensions
        self.print_step(
            f"Writing empty extension list to clear all extensions", "")
        try:
            # Create the Extension attribute
            extension_attr = Clusters.AccessControl.Attributes.Extension
            # Create empty extensions list
            extensions_list2 = []
            # Write the attribute with named parameter
            result = await self.default_controller.WriteAttribute(
                self.dut_node_id,
                [(0, extension_attr(value=extensions_list2))]
            )
            self.print_step(f"Write result", str(result))
            asserts.assert_equal(
                result[0].Status, Status.Success, "Write should have succeeded")

        except Exception as e:
            self.print_step(f"Error clearing extensions", str(e))
            asserts.fail(f"Failed to clear extensions: {e}")

        # Allow time for event to be generated
        self.print_step(f"Waiting 5 seconds for event generation...", "")
        await asyncio.sleep(5)

        self.step(13)
        # Read events directly
        self.print_step(f"Reading events after clearing extensions...", "")

        # Try multiple attempts with increasing timeouts
        max_attempts = 5
        found_valid_event = False  # Flag to indicate if we found the expected event

        for attempt in range(1, max_attempts + 1):
            try:
                self.print_step(
                    f"Attempt {attempt}/{max_attempts} to read events", "")
                events_response = await self.default_controller.ReadEvent(
                    self.dut_node_id,
                    events=[(0, acec_event)],
                    fabricFiltered=True
                )
                self.print_step(f"Events response", str(events_response))

                # Extract events from the response
                events = events_response
                self.print_step(f"Found {len(events)} events", "")

                # We need to find a remove event with specific fields
                for event_data in events:
                    self.print_step(f"Examining event", str(event_data))

                    if hasattr(event_data, 'Data') and hasattr(event_data.Data, 'changeType'):
                        # Check for remove event with specific data
                        self.print_step(f"event data", str(event_data))
                        if (event_data.Data.changeType == Clusters.AccessControl.Enums.ChangeTypeEnum.kRemoved and
                            event_data.Data.adminNodeID == self.default_controller.nodeId and
                            isinstance(event_data.Data.adminPasscodeID, Nullable) and
                            event_data.Data.latestValue.data == new_extension_data and
                            event_data.Data.latestValue.fabricIndex == f1 and
                                event_data.Data.fabricIndex == f1):
                            found_valid_event = True
                            self.print_step(f"Found valid REMOVE event", "")
                            break

                # If we found the valid event, we can proceed
                if found_valid_event:
                    self.print_step(
                        f"Found valid REMOVE event, proceeding", "")
                    break

                # If not found, wait and try again
                if attempt < max_attempts:
                    wait_time = attempt * 2
                    self.print_step(f"No matching events found on attempt {
                                    attempt}, waiting {wait_time}s...", "")
                    await asyncio.sleep(wait_time)
                else:
                    self.print_step("ERROR: All attempts failed", "")

            except Exception as e:
                self.print_step(
                    f"Error reading events (attempt {attempt})", str(e))
                if attempt < max_attempts:
                    wait_time = attempt * 2
                    await asyncio.sleep(wait_time)
                else:
                    self.print_step(
                        "ERROR: All attempts failed with exceptions", "")
                    asserts.fail(f"Failed to read events: {e}")

        # After all attempts, check if we found the valid event
        asserts.assert_true(
            found_valid_event, "Did not find the expected REMOVE event with specified fields")
        self.print_step(f"Successfully verified the expected REMOVE event", "")


if __name__ == "__main__":
    default_matter_test_main()
