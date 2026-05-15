import os
from enum import Enum, auto

from runner.runner import Runner

from .builder import BuilderOutput, OutDirLock, lock_output_dir
from .gn import GnBuilder


class GenioApp(Enum):
    LIGHT = auto()
    SHELL = auto()

    def ExampleName(self):
        if self == GenioApp.LIGHT:
            return 'lighting-app'
        if self == GenioApp.SHELL:
            return 'shell'
        raise Exception('Unknown app type: %r' % self)

    def AppNamePrefix(self):
        if self == GenioApp.LIGHT:
            return 'chip-mt793x-lighting-app-example'
        if self == GenioApp.SHELL:
            return 'chip-mt793x-shell-example'
        raise Exception('Unknown app type: %r' % self)

    def FlashBundleName(self):
        if self == GenioApp.LIGHT:
            return 'lighting_app.flashbundle.txt'
        if self == GenioApp.SHELL:
            return 'shell.flashbundle.txt'
        raise Exception('Unknown app type: %r' % self)

    def BuildRoot(self, root):
        return os.path.join(root, 'examples', self.ExampleName(), 'genio')


class GenioBuilder(GnBuilder):

    def __init__(self, root: str, runner: Runner, output_dir_lock: OutDirLock, app: GenioApp = GenioApp.LIGHT):
        super(GenioBuilder, self).__init__(root=app.BuildRoot(root), runner=runner, output_dir_lock=output_dir_lock)
        self.app = app

    @lock_output_dir
    def build_outputs(self):
        extensions = ['out', 'bin']
        if self.options.enable_link_map_file:
            extensions.append('out.map')
        for ext in extensions:
            name = f"{self.app.AppNamePrefix()}.{ext}"
            yield BuilderOutput(os.path.join(self.output_dir, name), name)
