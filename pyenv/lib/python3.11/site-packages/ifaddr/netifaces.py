# netifaces compatibility layer

import ifaddr

from typing import List


def interfaces() -> List[str]:
    adapters = ifaddr.get_adapters(include_unconfigured=True)
    return [a.name for a in adapters]
