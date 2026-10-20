import subprocess
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

import generate_spec_xml


class TestSpecificationIncludeLists(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.output = Path(self.directory.name)
        self.files = {
            'clusters': ['CoreCluster.xml', 'AppCluster.xml', 'UnusedCluster.xml', 'Cluster.xml'],
            'device_types': ['DeviceType.xml', 'UnusedDeviceType.xml', 'Type.xml'],
            'namespaces': ['Namespace.xml', 'UnusedNamespace.xml', 'space.xml'],
        }
        self.create_xml_files()

    def create_xml_files(self):
        for category, names in self.files.items():
            directory = self.output / category
            directory.mkdir(exist_ok=True)
            for name in names:
                (directory / name).write_text('<root/>', encoding='utf-8')

    def scrape_with_outputs(self, outputs):
        def make_output(target, *arguments):
            self.assertIn(target, outputs, f'Unexpected Make target: {target}')
            return outputs[target]

        with patch.object(generate_spec_xml, 'make_asciidoc', side_effect=make_output) as make, \
                patch.object(generate_spec_xml.subprocess, 'run'):
            generate_spec_xml.scrape_all('/alchemy', '/spec', str(self.output), False, 'Current', False)
        return [call.args[0] for call in make.call_args_list]

    def assert_selected_files(self):
        expected = {
            'clusters': {'CoreCluster.xml', 'AppCluster.xml'},
            'device_types': {'DeviceType.xml'},
            'namespaces': {'Namespace.xml'},
        }
        for category, names in expected.items():
            with self.subTest(category=category):
                self.assertEqual({path.name for path in (self.output / category).iterdir()}, names)

    def test_combined_book_reuses_main_include_list(self):
        for prefix in ('', 'src/', '/documents/src/'):
            with self.subTest(prefix=prefix):
                self.create_xml_files()
                includes = '\n'.join(prefix + name for name in (
                    'appclusters.adoc', 'device_library.adoc', 'standard_namespaces.adoc',
                    'CoreCluster.adoc', 'AppCluster.adoc', 'DeviceType.adoc', 'Namespace.adoc',
                ))
                targets = self.scrape_with_outputs({'pdf': includes})
                self.assertEqual(targets, ['pdf'])
                self.assert_selected_files()

    def test_legacy_books_keep_separate_include_lists(self):
        targets = self.scrape_with_outputs({
            'pdf': 'CoreCluster.adoc',
            'pdf-appclusters-book': 'AppCluster.adoc',
            'pdf-devicelibrary-book': 'DeviceType.adoc',
            'pdf-standardnamespaces-book': 'Namespace.adoc',
        })
        self.assertEqual(targets, ['pdf', 'pdf-appclusters-book', 'pdf-devicelibrary-book',
                                   'pdf-standardnamespaces-book'])
        self.assert_selected_files()

    def test_incomplete_combined_book_keeps_legacy_path(self):
        targets = self.scrape_with_outputs({
            'pdf': 'CoreCluster.adoc\nappclusters.adoc\ndevice_library.adoc\n',
            'pdf-appclusters-book': 'AppCluster.adoc\n',
            'pdf-devicelibrary-book': 'DeviceType.adoc\n',
            'pdf-standardnamespaces-book': 'Namespace.adoc\n',
        })
        self.assertEqual(len(targets), 4)
        self.assert_selected_files()

    def test_optional_models_follow_selected_include_records(self):
        for combined in (False, True):
            for enabled in (False, True):
                with self.subTest(combined=combined, enabled=enabled):
                    self.create_xml_files()
                    optional = {
                        'clusters': 'InProgressCluster',
                        'device_types': 'InProgressDeviceType',
                        'namespaces': 'InProgressNamespace',
                    }
                    for category, name in optional.items():
                        (self.output / category / f'{name}.xml').write_text('<root/>', encoding='utf-8')
                    outputs = {
                        'pdf': 'CoreCluster.adoc',
                        'pdf-appclusters-book': 'AppCluster.adoc',
                        'pdf-devicelibrary-book': 'DeviceType.adoc',
                        'pdf-standardnamespaces-book': 'Namespace.adoc',
                    }
                    if combined:
                        outputs = {'pdf': '\n'.join([
                            'appclusters.adoc', 'device_library.adoc', 'standard_namespaces.adoc', *outputs.values(),
                        ])}
                    if enabled:
                        if combined:
                            outputs['pdf'] += '\n' + '\n'.join(f'{name}.adoc' for name in optional.values())
                        else:
                            for target, name in zip(list(outputs)[1:], optional.values()):
                                outputs[target] += f'\n{name}.adoc'
                    self.scrape_with_outputs(outputs)
                    for category, name in optional.items():
                        self.assertEqual((self.output / category / f'{name}.xml').exists(), enabled)
                        if enabled:
                            (self.output / category / f'{name}.xml').unlink()
                    self.assert_selected_files()

    def test_main_book_failure_is_not_suppressed(self):
        error = subprocess.CalledProcessError(2, ['make', 'pdf'])
        with (
            patch.object(generate_spec_xml, 'make_asciidoc', side_effect=error),
            patch.object(generate_spec_xml.subprocess, 'run'),
            self.assertRaises(subprocess.CalledProcessError) as raised,
        ):
            generate_spec_xml.scrape_all('/alchemy', '/spec', str(self.output), False, 'Current', False)
        self.assertIs(raised.exception, error)
        self.assertTrue((self.output / 'clusters/UnusedCluster.xml').exists())

    def test_legacy_book_failure_is_not_suppressed(self):
        error = subprocess.CalledProcessError(2, ['make', 'pdf-appclusters-book'])
        with (
            patch.object(generate_spec_xml, 'make_asciidoc', side_effect=['CoreCluster.adoc\n', error]),
            patch.object(generate_spec_xml.subprocess, 'run'),
            self.assertRaises(subprocess.CalledProcessError) as raised,
        ):
            generate_spec_xml.scrape_all('/alchemy', '/spec', str(self.output), False, 'Current', False)
        self.assertIs(raised.exception, error)
        self.assertTrue((self.output / 'clusters/UnusedCluster.xml').exists())


if __name__ == '__main__':
    unittest.main()
