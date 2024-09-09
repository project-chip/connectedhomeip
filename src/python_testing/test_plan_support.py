#
#    Copyright (c) 2024 Project CHIP Authors
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
import typing


def read_attribute(attribute: str, cluster: typing.Optional[str] = None):
    attr = f'reads the {attribute} attribute'
    if cluster:
        return f'{attr} from {cluster}'
    else:
        return attr


def save_as(val: str) -> str:
    return f' and saves the value as {val}'


def verify_status(status: str) -> str:
    return f'Verify DUT responds w/ status {status}'


def verify_success() -> str:
    return verify_status('SUCCESS')

# -----------------------
# Commissioning strings
# -----------------------


def commission_if_required(controller: typing.Optional[str] = None) -> str:
    controller_str = f'to {controller} ' if controller is not None else ''
    return f'Commission DUT {controller_str}if not already done'


def commission_from_existing(existing_controller_name: str, new_controller_name: str) -> str:
    # NOTE to implementers: This text corresponds to the actions taken by CommissioningBuildingBlocks.AddNOCForNewFabricFromExisting.
    # This function should be used in the TestSteps description when you use that function.
    # AddNOCForNewFabricFromExisting is used when the generated certificates are required for use in the test.
    # It written one step so we can just use the function directly without needing to annotate the sub-steps for the TH.
    return (f'Create a new controller on a new fabric called {new_controller_name}.\n'
            f'Commission the new controller from {existing_controller_name} as follows:\n\n'
            f'- {existing_controller_name} sends an ArmFailsafe command, followed by a CSRRequest command.\n'
            f'- Generate credentials on {new_controller_name} using the returned CSR.\n'
            f'- Save the RCAC as `rcac_{new_controller_name}. Save the ICAC as `icac_{new_controller_name}`. Save the NOC as `noc_{new_controller_name}`. Save the IPK as ipk_{new_controller_name}.\n'
            f'-  {existing_controller_name} sends the AddTrustedRootCertificate command with `rcac_{new_controller_name}`'
            f'- {existing_controller_name} sends the AddNOC command with the fields set as follows:\n'
            f'  * NOCValue:        `noc_{new_controller_name}`\n'
            f'  * ICACValue:       `icac_{new_controller_name}`\n'
            f'  * IPKValue:        `ipk_{new_controller_name}`\n'
            f'  * CaseAdminSubject: {new_controller_name} node ID\n'
            f'  * AdminVendorId:    {new_controller_name} vendor ID\n'
            f'- {new_controller_name} connects over CASE and sends the commissioning complete command')


def open_commissioning_window(controller: str = 'TH') -> str:
    return f'{controller} opens a commissioning window on the DUT'


def remove_fabric(index_var: str, controller: str):
    return f'{controller} sends the RemoveFabric command to the Node Operational Credentials cluster with the FabricIndex set to {index_var}.'


def verify_commissioning_successful() -> str:
    return 'Verify the commissioning is successful.'


def if_feature_supported(feature: str) -> str:
    return f"If the {feature} is supported"
