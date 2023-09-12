import logging

from zeroconf import ServiceBrowser, ServiceInfo, ServiceListener, Zeroconf

_MDNS_TYPES = {
    "_matterd._udp.local.": "COMMISSIONER",
    "_matterc._udp.local.": "COMMISSIONABLE",
    "_matter._tcp.local.": "OPERATIONAL",
    "_meshcop._udp.local.": "THREAD_BORDER_ROUTER",
}


class MatterMdnsListener(ServiceListener):

    def __init__(self, artifact_dir: str) -> None:
        super().__init__()
        self.artifact_dir = artifact_dir
        self.logger = logging.getLogger(__file__)

    @staticmethod
    def log_addr(info: ServiceInfo) -> str:
        ret = "\n"
        for addr in info.parsed_scoped_addresses():
            ret += f"{addr}\n"
        return ret

    @staticmethod
    def log_vid_pid(info: ServiceInfo) -> str:
        if info.properties is not None and b'VP' in info.properties:
            vid_pid = str(info.properties[b'VP'])
            vid_pid = vid_pid[2:len(vid_pid) - 1].split('+')
            vid = hex(int(vid_pid[0]))
            pid = hex(int(vid_pid[1]))
            return f"\nVID: {vid} PID: {pid}\n"
        return ""

    def handle_service_info(
            self,
            zc: Zeroconf,
            type_: str,
            name: str,
            delta_type: str) -> None:
        info = zc.get_service_info(type_, name)
        to_log = f"SERVICE {delta_type}\n"
        to_log += f"{name}\n"
        to_log += f"{info.properties}\n"
        to_log += _MDNS_TYPES[type_]
        to_log += self.log_vid_pid(info)
        to_log += self.log_addr(info)
        self.logger.info(to_log)

    def add_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        self.handle_service_info(zc, type_, name, "ADDED")

    def update_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        self.handle_service_info(zc, type_, name, "UPDATED")

    def remove_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        to_log = f"Service {name} removed\n"
        to_log += _MDNS_TYPES[type_]
        self.logger.warning(to_log)

    def browse_interactive(self) -> None:
        zc = Zeroconf()
        ServiceBrowser(zc, list(_MDNS_TYPES.keys()), self)
        try:
            input("Browsing Matter mDNS, press enter to stop\n")
        finally:
            zc.close()
