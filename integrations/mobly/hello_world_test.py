import chip_device
from mobly import asserts
from mobly import base_test
from mobly import test_runner

class HelloWorldTest(base_test.BaseTestClass):

  def setup_class(self):
    # Registering chip_device controller module declares the test's
    # dependency on CHIP/Pigweed device hardware. By default, we expect at least one
    # object is created from this.
    self.ads = self.register_controller(chip_device)
    self.dut = self.ads[0]
#    # Start Mobly Bundled Snippets (MBS).
#    self.dut.load_snippet('mbs', 'com.google.android.mobly.snippet.bundled')

  def test_hello(self):
    expected = "hello!"
    status, payload = self.dut.rpcs().EchoService.Echo(msg=expected)
    asserts.assert_true(status.ok(), 'Status is %s' % status)
    asserts.assert_equal(payload.msg, expected, 'Returned payload is "%s" expected "%s"'
                         % (payload.msg, expected))


if __name__ == '__main__':
  test_runner.main()
