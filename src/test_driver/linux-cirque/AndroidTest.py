rmat(chip_repo=CHIP_REPO)
            mount[1] = mount[1].format(chip_repo=CHIP_REPO)
        v["mount_pairs"] = _mount_pairs
    return config


DEVICE_CONFIG = {}
with open(os.path.join(CHIP_REPO, "src/test_driver/linux-cirque/topologies/one_node_one_android.json"), "r") as f:
    config_operations = [_parse_mount_dir]
    DEVICE_CONFIG = json.load(f)
    for op in config_operations:
        DEVICE_CONFIG = op(DEVICE_CONFIG)

if __name__ == "__main__":
    sys.exit(TestAndroidController(DEVICE_CONFIG).run_test())