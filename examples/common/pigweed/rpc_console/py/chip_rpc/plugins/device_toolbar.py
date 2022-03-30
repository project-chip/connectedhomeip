#
#    Copyright (c) 2022 Project CHIP Authors
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
""" Toolbar plugin for viewing device status in console.

Uses the Device RPC service to read device info and state and display it
in the toolbar whenever the 'Refresh' button is pressed.
"""

from prompt_toolkit.layout import WindowAlign

from pw_console.plugin_mixin import PluginMixin
from pw_console.widgets import ToolbarButton, WindowPaneToolbar


class DeviceToolbar(WindowPaneToolbar, PluginMixin):
    """Toolbar for displaying Matter device info."""
    TOOLBAR_HEIGHT = 1

    def _format_field(self, name, value):
        return [('class:theme-bg-active class:theme-fg-active', '{}:'.format(name)),
                ('class:theme-bg-active class:theme-fg-cyan', '{}'.format(value)),
                ('', '  ')]

    def _update_toolbar_text(self):
        """ Read the device info using RPCs, and populate the toolbar values.
        """
        tokens = []
        self.plugin_logger.debug('DeviceToolbar _update_toolbar_text')

        status, device_info = self.rpcs.chip.rpc.Device.GetDeviceInfo()
        if not status.ok():
            self.formatted_text = [self._format_field(
                "ERROR GetDeviceInfo", status.name)]
            return

        tokens.extend(self._format_field("VID", device_info.vendor_id))
        tokens.extend(self._format_field("PID", device_info.product_id))
        tokens.extend(self._format_field("SN", device_info.serial_number))

        status, pairing = self.rpcs.chip.rpc.Device.GetPairingState()
        if not status.ok():
            self.formatted_text = [
                self._format_field("ERROR GetPairingState", status.name)]
            return

        if pairing:
            self.formatted_state = ('class:theme-fg-blue', "PAIRING")
            tokens.extend(self._format_field(
                "Pairing Code", device_info.pairing_info.code))
            tokens.extend(self._format_field(
                "Pairing Discriminator", device_info.pairing_info.discriminator))
        else:
            status, device_state = self.rpcs.chip.rpc.Device.GetDeviceState()
            if not status.ok():
                self.formatted_text = [
                    self._format_field("ERROR GetDeviceState", status.name)]
                return

            if len(device_state.fabric_info) == 0 or device_state.fabric_info[0].node_id == 0:
                self.formatted_state = ('class:theme-fg-blue', "DISCONNECTED")
            else:
                self.formatted_state = ('class:theme-fg-blue', "CONNECTED")
                # Only show the first fabric info if multiple.
                tokens.extend(self._format_field(
                    "Fabric", device_info.fabric_state[0].fabric_id))
                tokens.extend(self._format_field(
                    "Node", device_info.fabric_state[0].node_id))
        self.formatted_text = tokens

    def get_left_text_tokens(self):
        """Formatted text to display on the far left side."""
        return self.formatted_text

    def get_right_text_tokens(self):
        """Formatted text to display on the far right side."""
        return [self.formatted_state]

    def __init__(self, rpcs, *args, **kwargs):
        super().__init__(*args,
                         center_section_align=WindowAlign.RIGHT,
                         **kwargs)

        self.rpcs = rpcs

        self.show_toolbar = True
        self.formatted_text = []
        self.formatted_state = ('class:theme-fg-blue',
                                'MATTER Toolbar')

        # Buttons for display in the center
        self.add_button(
            ToolbarButton(description='Refresh',
                          mouse_handler=self._update_toolbar_text))

        # Set plugin options
        self.plugin_init(
            plugin_logger_name='chip_rpc_device_toolbar',
        )
