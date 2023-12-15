
import logging

import chip.clusters as Clusters
from chip.interaction_model import Status
from matter_testing_support import MatterBaseTest, async_test_body
from mobly import asserts

# List of attributes tests in the following format: ['Attribute name',read/write, Value to Write, ExpectedValue, Test step]
Attribute_names = [
    ['State', 'r', None, None],
    ['SupplyState', 'r', None, None],
    ['FaultState', 'r', None, None],
    ['ChargingEnabledUntil', 'r', None, None],
    ['DischargingEnabledUntil', 'r', None, None],
    ['CircuitCapacity', 'r', None, None],
    ['MinimumChargeCurrent', 'r', None, None],
    ['MaximumChargeCurrent', 'r', None, None],
    ['MaximumDischargeCurrent', 'r', None, None],
    ['UserMaximumChargeCurrent', 'rw', 20000, 20000],
    ['UserMaximumChargeCurrent', 'rw', 90000, 20000],    # Constraints test
    ['UserMaximumChargeCurrent', 'rw', -90000, 20000],    # Constraints test
    ['RandomizationDelayWindow', 'rw', 699, 699],
    ['RandomizationDelayWindow', 'rw', 90000, 699],      # Constraints test
    ['NumberOfWeeklyTargets', 'r', None, None],
    ['NumberOfDailyTargets', 'r', None, None],
    ['NextChargeStartTime', 'r', None, None],
    ['NextChargeTargetTime', 'r', None, None],
    ['NextChargeRequiredEnergy', 'r', None, None],
    ['NextChargeTargetSoC', 'r', None, None],
    ['ApproximateEVEfficiency', 'rw', 4800, 4800],
    ['StateOfCharge', 'r', None, None],
    ['BatteryCapacity', 'r', None, None],
    ['VehicleID', 'r', None, None],
    ['SessionID', 'r', None, None],
    ['SessionDuration', 'r', None, None],
    ['SessionEnergyCharged', 'r', None, None],
    ['SessionEnergyDischarged', 'r', None, None],
]


class EvseTest(MatterBaseTest):
    # Evse read and write tests
    @async_test_body
    async def test_read(self):
        for attribute_name, rw, value_to_write, expected_value in Attribute_names:
            logging.info(f"**** Starting read test for {attribute_name}")
            dev_ctrl = self.default_controller
            full_method = getattr(Clusters.EnergyEvse.Attributes, attribute_name)
            attribute = await self.read_single_attribute(
                dev_ctrl,
                self.dut_node_id,
                1,
                full_method
            )
            logging.info(f"**** Found {attribute_name}: {attribute}")
            asserts.assert_false(isinstance(attribute, Clusters.Attribute.ValueDecodeFailure), "We should not get a decode failure")
            # asserts.assert_equal(attribute, expected_value[1],
            #                      f"{attribute_name} should return {expected_value} but instead returns: {attribute}")
            asserts.assert_not_equal(attribute, None, f"{attribute_name} should return a value but instead returns None")

    @async_test_body
    async def test_write(self):
        for attribute_name, rw, value_to_write, expected_value in Attribute_names:
            dev_ctrl = self.default_controller
            full_method = getattr(Clusters.EnergyEvse.Attributes, attribute_name)

            if 'w' in rw:
                logging.info(f"**** Testing Write of {attribute_name} - Expect this to write successfully")
                result = await self.default_controller.WriteAttribute(self.dut_node_id, [(1, full_method(value_to_write))])
                if expected_value == value_to_write:
                    # The written value is in range, so should be accepted
                    logging.info(f"**** Writing valid value {value_to_write}, expecting Success.")
                    asserts.assert_equal(result[0].Status, Status.Success, f"{attribute_name} write failed")
                else:
                    # The written value is outside range, so should be rejected with Constraint Error
                    logging.info(f"**** Writing invalid value {value_to_write}, expecting ConstraintError.")
                    asserts.assert_equal(result[0].Status, Status.ConstraintError,
                                         f"{attribute_name} should fail with ConstraintError.")

                # Test read back
                attribute = await self.read_single_attribute(dev_ctrl, self.dut_node_id, 1, full_method)
                logging.info(f"**** Wrote {value_to_write}, expected {expected_value} got back {attribute}")
                asserts.assert_equal(attribute, expected_value,
                                     f"{attribute_name} should return {expected_value} but instead returns: {attribute}")
            else:
                logging.info(f"**** Testing Write of {attribute_name} - Expect this to fail since Attribute is ReadOnly")
                result = await self.default_controller.WriteAttribute(self.dut_node_id, [(1, full_method(1))])
                asserts.assert_equal(result[0].Status, Status.UnsupportedWrite,
                                     f"{attribute_name} should not be a writeable attribute and therefore writing should fail.")
