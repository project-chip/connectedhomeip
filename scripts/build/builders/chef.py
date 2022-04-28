import os
import json
import subprocess
from enum import Enum, auto

from .builder import Builder


class ChefApp(Enum):

    ALL_DEVICES = auto()

    def AppPath(self):
        if self == ChefApp.ALL_DEVICES:
            return 'chef'

    def AppNamePrefix(self):
        if self == ChefApp.ALL_DEVICES:
            return 'chip-chef-examples'
        else:
            raise Exception('Unknown app type: %r' % self)


class ChefBuilderUtil():

    def __init__(self, chef_builder):
        self.chef_builder = chef_builder

    def load_platforms(self):
        chef_platforms = os.path.join(self.chef_builder.root, 'platforms.json')
        with open(chef_platforms) as platforms:
            for platform in json.load(platforms)['platforms']:
                yield platform

    def load_devices(self):
        devices_dir = os.path.join(self.chef_builder.root, 'devices')
        for device in os.listdir(devices_dir):
            if device.endswith('.zap'):
                yield device[:len(device)-len('.zap')]

    def create_build_command(self, zap, target):
        return './chef.py -zbr -d {} -t {}'.format(zap, target)

    def build(self):
        for device in self.load_devices():
            for platform in filter(lambda x: x == 'linux', self.load_platforms()): # TODO: support all plats
                command = self.create_build_command(device, platform)
                subprocess.check_call(command, cwd=self.chef_builder.root, shell=True)

    def create_manifest(self):
        manifest = {}
        for platform in filter(lambda x: x == 'linux', self.load_platforms()): # TODO: support all plats
            manifest[platform] = '{}/{}/out/'.format(self.chef_builder.root, platform)
        return manifest


class ChefBuilder(Builder):

    def __init__(self,
                 root,
                 runner,
                 app: ChefApp = ChefApp.ALL_DEVICES,
                 enable_rpcs: bool = False):
        super(ChefBuilder, self).__init__(root=os.path.join(root, 'examples', app.AppPath()),runner=runner)
        self.app = app
        self.enable_rpcs = enable_rpcs
        self.build_util = ChefBuilderUtil(self)

    def generate(self):
        pass

    def _build(self):
        self.build_util.build()

    def build_outputs(self):
        return self.build_util.create_manifest()
