from pathlib import Path
import os

CHIP_ROOT1 = os.getenv('CHIP_ROOT') or next(filter(lambda p: ( p / 'SPECIFICATION_VERSION').is_file() or p == Path.home(), Path(__file__).parents))
RUNNER_SCRIPT_DIR = os.path.join(CHIP_ROOT1, 'scripts/tests')

print(RUNNER_SCRIPT_DIR)

CHIP_ROOT2 = next(filter(lambda p: (p / 'SPECIFICATION_VERSION').is_file(), Path(__file__).parents))

print(CHIP_ROOT2)


