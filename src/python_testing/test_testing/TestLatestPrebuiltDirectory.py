from mobly import asserts

from matter.testing.matter_testing import MatterBaseTest
from matter.testing.spec_parsing import VERSION_TO_DM, PrebuiltDataModelDirectory, build_xml_data_model, latest_prebuilt_directory


class TestLatestPrebuiltDirectory(MatterBaseTest):
    def test_latest_prebuilt_directory_returns_newest_version(self):
        """latest_prebuilt_directory() must point at the newest entry in VERSION_TO_DM."""
        asserts.assert_equal(
            latest_prebuilt_directory(),
            VERSION_TO_DM[max(VERSION_TO_DM)],
            "latest_prebuilt_directory drifted from VERSION_TO_DM",
        )

    def test_prebuilt_dm_enum_and_version_map_in_sync(self):
        """Every PrebuiltDataModelDirectory (except pre-1.3 legacy) must be reachable via VERSION_TO_DM.

        Without this, adding a new enum value (e.g. k1_7) but forgetting to register
        it in VERSION_TO_DM leaves latest_prebuilt_directory silently pinned to the
        previous release.
        """
        # k1_2 predates the SpecificationVersion attribute and is intentionally
        # not mapped from a version code.
        legacy_exempt = {PrebuiltDataModelDirectory.k1_2}
        mapped = set(VERSION_TO_DM.values())
        missing = set(PrebuiltDataModelDirectory) - legacy_exempt - mapped
        asserts.assert_equal(
            missing, set(),
            f"PrebuiltDataModelDirectory entries missing from VERSION_TO_DM: "
            f"{sorted(m.name for m in missing)}",
        )

    def test_build_xml_data_model_populates_every_section(self):
        """build_xml_data_model must return non-empty clusters, device types, namespaces, and global data types."""
        dm = build_xml_data_model(latest_prebuilt_directory())
        asserts.assert_true(dm.clusters, "clusters section is empty")
        asserts.assert_true(dm.device_types, "device_types section is empty")
        asserts.assert_true(dm.namespaces, "namespaces section is empty")
        asserts.assert_true(dm.global_data_types.get('structs'), "global structs empty")
        asserts.assert_true(dm.global_data_types.get('enums'), "global enums empty")
        asserts.assert_true(dm.global_data_types.get('bitmaps'), "global bitmaps empty")
