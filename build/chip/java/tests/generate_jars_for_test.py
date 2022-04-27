import os
import subprocess
import sys

"""Generate JARs used to test the java_prebuilt rule."""

chip_root = os.getenv('PW_PROJECT_ROOT')
test_dir = chip_root + '/build/chip/java/tests'
tmp_dir = '/tmp/chip_java_build_test'


def generateJar(source_file, output_name):
    tmp_classes_dir = tmp_dir + '/classes'
    os.makedirs(tmp_dir, exist_ok=True)
    os.makedirs(tmp_dir + '/classes', exist_ok=True)
    javac_runner_command = [
        'python3',
        chip_root + '/build/chip/java/javac_runner.py',
        '--classdir',
        tmp_classes_dir,
        '--outfile',
        tmp_dir + '/prebuilt_jar.classlist',
        '--build-config',
        test_dir + '/empty_build_config.json',
        '--',
        '-d',
        tmp_classes_dir,
        source_file,
    ]
    subprocess.check_call(javac_runner_command)

    jar_runner_command = [
        'python3',
        chip_root + '/build/chip/java/jar_runner.py',
        'cf',
        test_dir + '/' + output_name,
        '-C',
        tmp_classes_dir,
        '.',
    ]
    subprocess.check_call(jar_runner_command)


def main():
    generateJar(test_dir + '/IncludedInJar.java', 'prebuilt_jar.jar')
    generateJar(test_dir + '/IncludedInChildJar.java', 'child_jar.jar')


if __name__ == '__main__':
    sys.exit(main())
