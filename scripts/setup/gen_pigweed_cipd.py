import argparse
import json

_LIST_OF_PACKAGES_TO_EXCLUDE = {'rust'}


def include_package(package: dict) -> bool:
    if 'path' in package:
        path_full = package['path']
        path = path_full.split('/')
        exclusion_match = any(dir in _LIST_OF_PACKAGES_TO_EXCLUDE for dir in path)
        if exclusion_match:
            return False
    return True


def generate_new_cipd_package_json(input, output):
    with open(input) as ins:
        packages = json.load(ins)
        file_packages = packages.get('packages')
        new_file_packages = [x for x in file_packages if include_package(x)]

    new_packages = {'packages': new_file_packages}
    with open(output, 'w') as f:
        json.dump(new_packages, f, indent=2)


def main():
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument(
        '--input', '-i', required=True
    )
    parser.add_argument(
        '--output', '-o', required=True
    )
    generate_new_cipd_package_json(**vars(parser.parse_args()))


if __name__ == '__main__':
    main()
