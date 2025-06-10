import unittest


class TestMatterLinter(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def testSimpleTest(self):
        self.assertTrue(True)


if __name__ == '__main__':
    unittest.main()
