import modules.util as _util
import modules.arguments as _args
import importlib
import os


class Version:

    def __init__(self, y) -> None:
        self.tag = None
        self.module = None
        self.supported = []
        self.date = None
        self.summary = None
        self.parse(y)

    def __str__(self) -> str:
        compat = (len(self.supported) > 0) and " ({})".format(', '.join(self.supported)) or ''
        return "{}{} {} [{}]".format(self.tag, compat, self.date, self.module)

    def parse(self, y):
        self.tag = y['version']
        self.module = ('module' in y) and y['module'] or ('v' + self.tag.replace('.', '_'))
        if 'supports' in y:
            sy = y['supports']
            versions = isinstance(sy, str) and sy.split(',') or sy
            for v in versions:
                self.supported.append(v.strip())
        if 'date' in y:
            self.date = y['date']
        self.summary = y['summary']


class VersionManager:

    DEFAULT = Version({ 'version': '2.1', 'module': 'v2_0', 'date': '2023-11-01', 'summary': 'Default'})

    def __init__(self, paths) -> None:
        self.paths = paths
        self.versions = [ VersionManager.DEFAULT ]
        filename = paths.base('versions.yaml')
        if os.path.exists(filename):
            for y in _util.YamlFile(filename).read():
                self.versions.append(Version(y))

    def find(self, ver = None) -> Version:
        if ver is None: return VersionManager.DEFAULT
        v = None
        count = len(self.versions)
        # Order versions by tag
        vers = sorted(self.versions, key=lambda v: v.tag)
        # Search for the newest version that matches the given prefix
        ver_len = len(ver)
        for w in vers:
            pre = w.tag[:ver_len]
            # print("V:{} W:{}; PRE:{}".format(ver, w, pre))
            if pre == ver:
                # Version match
                v = w
            # Search within supported versions
            for c in w.supported:
                comp = c[:ver_len]
                if comp == ver:
                    v = w
            if pre > ver:
                break
        if v is None:
            # No matching version found
            _util.fail("Unsupported version \"{}\"".format(ver))
        return v
