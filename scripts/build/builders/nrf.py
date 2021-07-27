import logging
import os
import shlex

from enum import Enum, auto

from .builder import Builder


class NrfApp(Enum):
  LIGHT = auto()
  LOCK = auto()
  SHELL=auto()

  def ExampleName(self):
    if self == NrfApp.LIGHT:
      return 'lighting-app'
    elif self == NrfApp.LOCK:
      return 'lock-app'
    elif self == NrfApp.SHELL:
      return 'shell'
    else:
      raise Exception('Unknown app type: %r' % self)

  def AppNamePrefix(self):
    if self == NrfApp.LIGHT:
      return 'chip-nrf-lighting-example'
    elif self == NrfApp.LOCK:
      return 'chip-nrf-lock-example'
    elif self == NrfApp.SHELL:
      return 'chip-nrf-shell'
    else:
      raise Exception('Unknown app type: %r' % self)


class NrfBoard(Enum):
  NRF52840 = auto()
  NRF5340 = auto()

  def GnArgName(self):
    if self == NrfBoard.NRF52840:
      return 'nrf52840dk_nrf52840'
    elif self == NrfBoard.NRF5340:
      return 'nrf5340dk_nrf5340_cpuapp'
    else:
      raise Exception('Unknown board type: %r' % self)


class NrfConnectBuilder(Builder):

  def __init__(self,
               root,
               runner,
               output_dir: str,
               app: NrfApp = NrfApp.LIGHT,
               board: NrfBoard = NrfBoard.NRF52840):
    super(NrfConnectBuilder, self).__init__(root, runner, output_dir)
    self.app = app
    self.board = board

  def generate(self):
    if not os.path.exists(self.output_dir):
        # NRF does a in-place update  of SDK tools
        if 'ZEPHYR_BASE' not in os.environ:
            raise Exception("NRF builds require ZEPHYR_BASE to be set")

        zephyr_base = os.environ['ZEPHYR_BASE']
        nrfconnect_sdk = os.path.dirname(zephyr_base)

        # NRF builds will both try to change .west/config in nrfconnect and 
        # overall perform a git fetch on that location
        if not os.access(nrfconnect_sdk, os.W_OK):
            raise Exception("Directory %s not writable. NRFConnect builds require updates to this directory." % nrfconnect_sdk)

        # NOTE: update_ncs is available but SHOULD NOT be needed on docker builds.
        # We are specifically NOT adding to below:
        #
        #   python3 scripts/setup/nrfconnect/update_ncs.py --update --shallow
        #
        # ZEPHYR_BASE is assumed already updated to the correct version
        #
        # TODO: can this 'correct version' be checked by the build script?

        cmd = '''
[[ -n $ZEPHYR_BASE ]] && source "$ZEPHYR_BASE/zephyr-env.sh";
export GNUARMEMB_TOOLCHAIN_PATH="$PW_PIGWEED_CIPD_INSTALL_DIR";
west build --cmake-only -d {outdir} -b {board} {sourcedir}
        '''.format(
            outdir = shlex.quote(self.output_dir),
            board = self.board.GnArgName(),
            sourcedir=shlex.quote(os.path.join(self.root, 'examples', self.app.ExampleName(), 'nrfconnect'))
        ).strip()

        self._Execute(['bash', '-c', cmd], title='Generating ' + self.identifier)


  def build(self):
    logging.info('Compiling NrfConnect at %s', self.output_dir)

    self.generate()
    self._Execute(['ninja', '-C', self.output_dir], title='Building ' + self.identifier)

  def outputs(self):
    return {
        '%s.elf' % self.app.AppNamePrefix(): os.path.join(self.output_dir, 'zephyr', 'zephyr.elf'),
        '%s.map' % self.app.AppNamePrefix(): os.path.join(self.output_dir, 'zephyr', 'zephyr.map'),
    }
