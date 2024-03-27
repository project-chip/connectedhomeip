import unittest
from metadata import Metadata
from metadata import Metadata_Reader


class TestMetadataReader(unittest.TestCase):
    # build the test Metadata_Reader object
    #defines expected arguments associated with runs
    def setUp(self):

        # build the reader object
        self.reader=Metadata_Reader("/src/python_testing", "/scripts/tests/py/env.yaml")
        self.actual_runs_metadata = self.reader.get_runs_metadata()
        self.expected_runs_metadata = {}

        self.expected_runs_metadata["/src/python_testing/TC_RR_1_1.py+run1"] = Metadata(app="out/linux-x64-all-clusters-ipv6only-no-ble-no-wifi-tsan-clang-test/chip-all-clusters-app",
                   discriminator=1234, py_script_path="/src/python_testing/TC_RR_1_1.py", run="run1", passcode=20202021)
        
    # gets the metadata associated with each run by parsing the test script and compares it to the expected metadata values
    def test_case1(self):
        for actual_run in self.actual_runs_metadata:
            self.assertEqual(self.actual_runs_metadata[actual_run], self.expected_runs_metadata[actual_run])

    
if __name__=="__main__":
    unittest.main()
            
