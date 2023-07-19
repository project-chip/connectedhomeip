import os
from enum import Enum, auto

from .gn import GnBuilder


class GenioApp(Enum):
    LIGHT = auto()
    SHELL = auto()

    def ExampleName(self):
        if self == GenioApp.LIGHT:
            return 'lighting-app'
        elif self == GenioApp.SHELL:
            return 'shell'
        else:
            raise Exception('Unknown app type: %r' % self)

    def AppNamePrefix(self):
        if self == GenioApp.LIGHT:
            return 'chip-mt793x-lighting-app-example'
        elif self == GenioApp.SHELL:
            return 'chip-mt793x-shell-example'
        else:
            raise Exception('Unknown app type: %r' % self)

    def FlashBundleName(self):
        if self == GenioApp.LIGHT:
            return 'lighting_app.flashbundle.txt'
        elif self == GenioApp.SHELL:
            return 'shell.flashbundle.txt'
        else:
            raise Exception('Unknown app type: %r' % self)

    def BuildRoot(self, root):
        return os.path.join(root, 'examples', self.ExampleName(), 'genio')


class GenioBuilder(GnBuilder):

    def __init__(self,
                 root,
                 runner,
                 app: GenioApp = GenioApp.LIGHT):
        super(GenioBuilder, self).__init__(
            root=app.BuildRoot(root),
            runner=runner)
        self.app = app

    def build_outputs(self):
        items = {}
        for extension in ['out', 'out.map', 'bin']:
            name = '%s.%s' % (self.app.AppNamePrefix(), extension)
            items[name] = os.path.join(self.output_dir, name)

        return items
