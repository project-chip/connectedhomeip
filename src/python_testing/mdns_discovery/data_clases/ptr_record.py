from dataclasses import dataclass, field

@dataclass
class PtrRecord:
    service_type: str
    service_name: str
    instance_name: str = field(init=False)

    def __post_init__(self):
        # Removes the service_type from the end of the service_name
        if self.service_name.endswith(self.service_type):
            self.instance_name = self.service_name[: -len(self.service_type)].rstrip('.')
        else:
            # Fallback in case format is unexpected
            self.instance_name = self.service_name
