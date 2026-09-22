#
#    Copyright (c) 2026 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

"""Test side interface to the Wi-Fi fixture of the Matter Test Harness.

The fixture is a container the harness starts for a test suite whose configuration names
Wi-Fi interfaces. It owns those radios and runs hostapd and wpa_supplicant with no
interface attached to either, so a test case can claim a radio for whichever role it
needs. A test case does that through WiFiFixture, which hands out the radios and the
connections to drive them, and then through a role class such as AccessPointFixture. A test
class reaches the fixture by way of WiFiFixtureMixin, which also puts it back as it was
after each test case.

A test case that asks for a radio the harness was not configured with fails rather than
skipping: a Wi-Fi test that quietly passes without a radio has tested nothing.
"""

import logging
import os
import re
import socket
import time
from collections import deque
from collections.abc import Mapping
from dataclasses import dataclass
from itertools import count
from pathlib import Path

from mobly.logger import PrefixLoggerAdapter

# The harness pours everything the test process logged into the run log, where a line from
# here sits among thousands from the SDK. The [MatterTest] those carry is baked into the
# format string of the handler for the whole run, so a prefix of our own has to go in the
# message. Keep the lines short and on one line each for the same reason.
log = PrefixLoggerAdapter(logging.getLogger(__name__), {PrefixLoggerAdapter.EXTRA_KEY_LOG_PREFIX: "[WiFiFixture]"})

# The directory shared with the fixture container, at the same path in both. A control
# client has to bind its own reply socket in here as well: the daemon replies to that
# pathname resolved in its own mount namespace, so a reply socket anywhere else means
# every command times out rather than failing. The variable is the one the fixture
# itself reads, and is only for driving a fixture started by hand -- note that
# hostapd_cli and wpa_cli have the default baked in and need -p to follow.
WIFI_FIXTURE_ROOT = Path(os.environ.get("WIFI_FIXTURE_ROOT", "/run/wifi-fixture"))

HOSTAPD = "hostapd"
WPA_SUPPLICANT = "wpa_supplicant"

# Commands are answered from the daemon's event loop, so this only has to cover a busy
# daemon rather than any work the command itself does.
DEFAULT_TIMEOUT = 10

# Adding a BSS runs the whole interface setup, which includes bringing the netdev up and
# waiting for the driver to confirm the channel.
ADD_TIMEOUT = 30

# A terminated daemon is restarted by the fixture's supervisor, which backs off for two
# seconds if the daemon it was watching exited quickly.
RESTART_TIMEOUT = 30

# Re-reading a configuration is parsing a handful of files and applying what changed to a
# BSS that stays up, so the radio is not reprogrammed and this only has to cover the files.
RELOAD_TIMEOUT = 30

# A station is listed by hostapd as soon as it has associated, and gains the authorized
# flag once the four-way handshake has completed. When it does either is the client's
# doing, so this has to cover a client that scans for the network first.
STATION_TIMEOUT = 30

# Events already sent are waiting in the socket buffer, so this only has to be long enough
# for the recv of the one that is not there.
DRAIN_TIMEOUT = 0.1

POLL_INTERVAL = 0.2

# One datagram per reply, and hostapd builds those in a 4096 byte buffer.
RECEIVE_SIZE = 8192

# hostapd sends this from ap_sta_set_authorized(), so it marks the same point as the
# authorized flag of a station rather than the association that precedes it.
AP_STA_CONNECTED = "AP-STA-CONNECTED"


class WiFiFixtureError(Exception):
    """Raised when the Wi-Fi fixture is absent, or refuses what it was asked to do."""


@dataclass(frozen=True)
class Event:
    """An unsolicited message from a daemon, delivered to connections that have attached."""

    name: str
    arguments: str = ""
    # Only set on a global control socket, which serves every interface of its daemon.
    ifname: str | None = None

    def __str__(self) -> str:
        return f"{self.name} {self.arguments}".strip()


# Events carry a priority, and on a global socket the interface they are about:
# `<3>AP-STA-CONNECTED 11:22:33:44:55:66`, `IFNAME=wlan0 <3>AP-STA-DISCONNECTED ...`. A
# reply never looks like this, which is what makes the two tellable apart on a socket
# carrying both.
_EVENT = re.compile(r"(?:IFNAME=(?P<ifname>\S+) )?<\d+>(?P<name>\S*)(?: (?P<arguments>.*))?", re.DOTALL)


def _parse_event(message: str) -> Event | None:
    """Returns the event a message carries, or None if it is a command reply."""
    match = _EVENT.fullmatch(message.rstrip("\n"))
    if match is None:
        return None
    return Event(name=match["name"], arguments=match["arguments"] or "", ifname=match["ifname"])


def _parse_properties(reply: str) -> dict[str, str]:
    """Turns the key=value lines of a control interface reply into a dictionary.

    Lines that are not a pair are dropped, which is what makes this usable for a station
    block as well: hostapd puts the MAC address on a line of its own ahead of the pairs.
    """
    properties = {}
    for line in reply.splitlines():
        key, separator, value = line.partition("=")
        if separator:
            properties[key] = value
    return properties


@dataclass(frozen=True)
class Radio:
    """A Wi-Fi radio the fixture owns, as published in its interfaces file.

    Both names are needed and they are not interchangeable: ifname goes in the interface=
    line of a configuration file, phy in the command that applies it.
    """

    ifname: str
    phy: str


@dataclass(frozen=True)
class Station:
    """A client of an access point, as hostapd reports it.

    The properties are whatever the daemon chose to say about it, which depends on what
    the client negotiated; only the address and the flags are always there.
    """

    address: str
    properties: Mapping[str, str]

    @property
    def flags(self) -> tuple[str, ...]:
        # One string of bracketed names, e.g. [AUTH][ASSOC][AUTHORIZED][WMM].
        return tuple(re.findall(r"\[([^]]*)]", self.properties.get("flags", "")))

    @property
    def authorized(self) -> bool:
        """Whether the four-way handshake has completed, i.e. the client is on the network."""
        return "AUTHORIZED" in self.flags

    @property
    def eap_identity(self) -> str | None:
        """The identity this client sent in its EAP Identity Response, if it sent one.

        None where the network does not use 802.1X, and also where the client got onto it
        without hostapd ever asking: an EAP method can carry its own identity instead, and a
        station that resumed a cached TLS session was not asked again.
        """
        identity = self.properties.get("dot1xAuthSessionUserName")
        # What hostapd puts there when it has no identity for the station.
        return None if identity in (None, "N/A") else identity

    def __str__(self) -> str:
        return f"{self.address} {self.properties.get('flags', '')}".strip()


def _parse_station(reply: str) -> Station:
    """Reads a station block, which is the MAC address on a line of its own and then pairs."""
    return Station(address=reply.splitlines()[0], properties=_parse_properties(reply))


class ControlConnection:
    """A connection to one hostap control socket.

    Created by WiFiFixture, which is the only thing that knows where the sockets are and
    where a reply socket is allowed to go. Events and replies share the socket once attach
    has been called, so events that arrive while a command is in flight are held rather
    than dropped, and next_event hands them out in the order they were sent.
    """

    _sequence = count()

    def __init__(self, path: Path) -> None:
        self.path = path
        self._socket = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
        self._events: deque[Event] = deque()
        # Next to the daemon's own socket, so it is inside the shared directory and both
        # ends resolve the path to the same file.
        client_path = path.parent / f"client-{os.getpid()}-{next(self._sequence)}"
        self._client_path: Path | None = None
        try:
            self._socket.bind(str(client_path))
            # Only ours to remove once the bind has succeeded, so that a name that
            # somehow belongs to another client is left alone.
            self._client_path = client_path
            self._socket.connect(str(path))
        except OSError as error:
            self.close()
            raise WiFiFixtureError(f"Cannot reach the control socket {path}: {error}")

    def __enter__(self) -> "ControlConnection":
        return self

    def __exit__(self, *exception_info: object) -> None:
        self.close()

    def close(self) -> None:
        self._socket.close()
        if self._client_path is not None:
            self._client_path.unlink(missing_ok=True)
            self._client_path = None

    def request(self, command: str, timeout: float = DEFAULT_TIMEOUT) -> str:
        """Sends a command and returns the reply, without its trailing newline."""
        try:
            self._socket.send(command.encode())
        except OSError as error:
            raise WiFiFixtureError(f"Cannot send '{command}' to {self.path}: {error}")

        deadline = time.monotonic() + timeout
        while True:
            message = self._receive(deadline)
            if message is None:
                raise WiFiFixtureError(f"No reply to '{command}' from {self.path} within {timeout} seconds")
            if not self._queue_event(message):
                return message.rstrip("\n")

    def request_ok(self, command: str, timeout: float = DEFAULT_TIMEOUT) -> None:
        """Sends a command that the daemon answers with OK, and fails on anything else."""
        reply = self.request(command, timeout)
        if reply != "OK":
            raise WiFiFixtureError(f"'{command}' was refused by {self.path}: {reply or '(empty reply)'}")

    def attach(self) -> None:
        """Subscribes this connection to the daemon's events.

        Only events sent after this point arrive, so attach before doing the thing whose
        outcome an event is meant to report.
        """
        self.request_ok("ATTACH")

    def next_event(self, timeout: float = DEFAULT_TIMEOUT) -> Event | None:
        """Returns the next event, or None if none arrives within the timeout."""
        deadline = time.monotonic() + timeout
        while True:
            if self._events:
                return self._events.popleft()
            message = self._receive(deadline)
            if message is None:
                return None
            if not self._queue_event(message):
                # A reply to a command that has already given up on it.
                log.debug("Discarding a late reply from %s: %s", self.path, message)

    def drain_events(self) -> None:
        """Reads whatever the daemon has sent that nothing waited for, so that it is logged."""
        while self.next_event(DRAIN_TIMEOUT) is not None:
            pass

    def _queue_event(self, message: str) -> bool:
        """Logs and queues a message if it is an event, and returns whether it was one."""
        event = _parse_event(message)
        if event is None:
            return False
        log.info("%s: %s", event.ifname or self.path.name, event)
        self._events.append(event)
        return True

    def _receive(self, deadline: float) -> str | None:
        """Returns the next message on the socket, or None once the deadline has passed."""
        remaining = deadline - time.monotonic()
        if remaining <= 0:
            return None
        self._socket.settimeout(remaining)
        try:
            return self._socket.recv(RECEIVE_SIZE).decode(errors="replace")
        except TimeoutError:
            return None
        except OSError as error:
            raise WiFiFixtureError(f"Cannot read from {self.path}: {error}")


class WiFiFixture:
    """The Wi-Fi fixture of the harness, and the radios it owns.

    Class methods throughout, because there is one fixture and the harness has already
    started it by the time a test case runs.
    """

    _verified = False

    @classmethod
    def radios(cls) -> tuple[Radio, ...]:
        """Returns the radios the fixture claimed, in configuration order."""
        cls.verify()
        return cls._read_interfaces()

    @classmethod
    def require_radios(cls, count: int = 1) -> tuple[Radio, ...]:
        """Returns the fixture's radios, failing unless there are at least count of them."""
        radios = cls.radios()
        if len(radios) < count:
            raise WiFiFixtureError(
                f"This test case needs {count} Wi-Fi radios, but the fixture has {len(radios)}. "
                "Name the radios it should own in network.wifi.interfaces of the test harness configuration.")
        return radios

    @classmethod
    def verify(cls) -> None:
        """Checks that the fixture is there and both of its daemons answer, once per process."""
        if cls._verified:
            return
        for daemon in (HOSTAPD, WPA_SUPPLICANT):
            socket_path = cls.global_socket(daemon)
            if not socket_path.exists():
                raise WiFiFixtureError(
                    f"The Wi-Fi fixture is not running: there is no control socket at {socket_path}. "
                    "Name the radios it should own in network.wifi.interfaces of the test harness configuration.")
            cls._ping(daemon)
        cls._verified = True

    @classmethod
    def connect(cls, daemon: str, target: str = "global") -> ControlConnection:
        """Opens a connection to a daemon's global control socket, or to one of its interfaces.

        Connections are cheap and meant to be short lived. The fixture wipes a daemon's
        directory whenever it restarts it, which takes any reply socket bound there with
        it, so a connection held across a restart stops being answered.
        """
        cls.verify()
        return cls._open(daemon, target)

    @classmethod
    def request(cls, daemon: str, command: str, timeout: float = DEFAULT_TIMEOUT) -> str:
        """Sends one command to a daemon's global control socket and returns the reply."""
        with cls.connect(daemon) as control:
            return control.request(command, timeout)

    @classmethod
    def reset(cls, *daemons: str) -> None:
        """Returns the fixture to its initial state by restarting the given daemons, or both.

        Terminating a daemon is the only reset the fixture has: its supervisor wipes and
        recreates the daemon's directory before starting it again, so this releases every
        radio the daemon held and removes every configuration file written for it.
        """
        cls.verify()
        for daemon in daemons or (HOSTAPD, WPA_SUPPLICANT):
            log.info("Restarting %s of the Wi-Fi fixture", daemon)
            with cls._open(daemon) as control:
                control.request_ok("TERMINATE")
            cls._wait_for_restart(daemon)

    @classmethod
    def reset_if_used(cls) -> None:
        """Resets the fixture if this process has touched it, and does nothing otherwise.

        Every entry point verifies the fixture before it does anything, so the latch that
        records having done so also records that something wanted a radio. That makes this
        safe to call from the teardown of a test class whose test cases do not all use
        Wi-Fi, or that runs somewhere there is no fixture at all.
        """
        if cls._verified:
            cls.reset()

    @classmethod
    def directory(cls, daemon: str) -> Path:
        """Returns the directory a daemon keeps its sockets and configuration files in."""
        return WIFI_FIXTURE_ROOT / daemon

    @classmethod
    def global_socket(cls, daemon: str) -> Path:
        return cls.directory(daemon) / "global"

    @classmethod
    def _open(cls, daemon: str, target: str = "global") -> ControlConnection:
        return ControlConnection(cls.directory(daemon) / target)

    @classmethod
    def _ping(cls, daemon: str) -> None:
        with cls._open(daemon) as control:
            reply = control.request("PING")
        if reply != "PONG":
            raise WiFiFixtureError(f"The {daemon} of the Wi-Fi fixture answered a ping with '{reply}'")

    @classmethod
    def _wait_for_restart(cls, daemon: str, timeout: float = RESTART_TIMEOUT) -> None:
        socket_path = cls.global_socket(daemon)
        deadline = time.monotonic() + timeout
        # The supervisor deletes the directory before starting the daemon again, so the
        # socket going away is what tells the new daemon apart from the one being stopped.
        while socket_path.exists():
            if time.monotonic() > deadline:
                raise WiFiFixtureError(f"The {daemon} of the Wi-Fi fixture did not stop within {timeout} seconds")
            time.sleep(POLL_INTERVAL)
        while True:
            if socket_path.exists():
                try:
                    cls._ping(daemon)
                    return
                except WiFiFixtureError:
                    pass  # Started, but not listening yet.
            if time.monotonic() > deadline:
                raise WiFiFixtureError(f"The {daemon} of the Wi-Fi fixture did not come back within {timeout} seconds")
            time.sleep(POLL_INTERVAL)

    @classmethod
    def _read_interfaces(cls) -> tuple[Radio, ...]:
        path = WIFI_FIXTURE_ROOT / "interfaces"
        try:
            text = path.read_text()
        except OSError as error:
            raise WiFiFixtureError(f"Cannot read the radios of the Wi-Fi fixture from {path}: {error}")

        radios = []
        for line in text.splitlines():
            fields = line.split()
            if len(fields) != 2:
                raise WiFiFixtureError(f"Cannot read the radios of the Wi-Fi fixture: '{line}' is not an interface and a phy")
            radios.append(Radio(ifname=fields[0], phy=fields[1]))
        return tuple(radios)


def hostapd_file(radio: Radio, name: str) -> Path:
    """Returns the path of a file a hostapd configuration for this radio refers to.

    Beside the configuration itself, in the directory the fixture shares with the daemon,
    so that the path this process writes to and the path hostapd reads resolve to the same
    file. The name of the radio is part of the file name, so two access points running at
    once do not write over each other.
    """
    return WiFiFixture.directory(HOSTAPD) / f"{radio.ifname}.{name}"


class AccessPointFixture:
    """An access point on one of the fixture's radios, for the duration of a with block.

    The configuration is the caller's. Only the few options that describe the machine
    rather than the network are filled in here, so that covering another Wi-Fi mode is an
    edit to a test case rather than to the fixture.
    """

    # Derived from the radio this instance was given, or from where the fixture keeps its
    # sockets, so a caller setting them would only be contradicting itself. A subclass that
    # fills in more of the configuration than this one does extends the tuple, and applies
    # its own options after __init__ rather than passing them through it.
    _FIXTURE_OWNED_OPTIONS = ("interface", "driver", "ctrl_interface")

    # Enough to bring up a 2.4 GHz BSS, and overridable like everything else. HT is on
    # because a Matter device is expected to do at least Wi-Fi 4, so a fixture network
    # without it is not representative; it also turns WMM on by itself.
    _DEFAULT_OPTIONS: Mapping[str, object] = {"hw_mode": "g", "channel": 6, "ieee80211n": 1}

    def __init__(self, radio: Radio, options: Mapping[str, object]) -> None:
        owned = [option for option in self._FIXTURE_OWNED_OPTIONS if option in options]
        if owned:
            raise WiFiFixtureError(f"The Wi-Fi fixture sets the hostapd options {', '.join(owned)} itself")
        if "ssid" not in options and "ssid2" not in options:
            raise WiFiFixtureError("An access point needs an ssid hostapd option")

        self.radio = radio
        self.options = {**self._DEFAULT_OPTIONS, **options}
        self._control: ControlConnection | None = None
        self._started = False

    @property
    def ifname(self) -> str:
        return self.radio.ifname

    def __enter__(self) -> "AccessPointFixture":
        self.start()
        return self

    def __exit__(self, *exception_info: object) -> None:
        self.close()

    def start(self) -> None:
        """Brings the access point up, and returns once hostapd reports it as enabled."""
        config_path = hostapd_file(self.radio, "conf")
        config_path.write_text(self._config_text())

        # The options are what the network under test is, and the difference between two
        # runs of a test case is often one of them. The ones this class fills in are left
        # out, since they say nothing about the network.
        log.info("Starting an access point on %s (%s) with %s", self.ifname, self.radio.phy, self.options)
        with WiFiFixture.connect(HOSTAPD) as control:
            if self.ifname in control.request("INTERFACES").split():
                raise WiFiFixtureError(
                    f"hostapd is already running an access point on {self.ifname}. A test case that claims a radio "
                    "has to give it back; a test class has that done for it by WiFiFixtureMixin.")
            # bss_config= sets the interface up and enables the BSS in this one command,
            # where `ADD <ifname> config=` leaves it registered but disabled.
            control.request_ok(f"ADD bss_config={self.radio.phy}:{config_path}", timeout=ADD_TIMEOUT)
        self._started = True

        try:
            self._control = self._connect_to_bss()
            # Before the status check, so that nothing a client does is missed: a station
            # cannot join before the BSS is enabled, but it can while we are asking.
            self._control.attach()
            state = self.status().get("state")
            if state != "ENABLED":
                raise WiFiFixtureError(f"The access point on {self.ifname} is in state {state} rather than ENABLED")
        except Exception:
            self.close()
            raise

    def stop(self) -> None:
        """Takes the access point down and releases the radio, if it is still up.

        Fails if hostapd does not give the radio up, since the next test case to ask for
        one would then get a radio it cannot use. Use close on a cleanup path, which has
        something more worth reporting than this.
        """
        if not self._started:
            return
        self._started = False

        if self._control is not None:
            # Whatever hostapd said that the test case never waited for, so that it reaches
            # the log rather than going with the connection.
            self._control.drain_events()
            self._control.close()
            self._control = None

        log.info("Stopping the access point on %s", self.ifname)
        with WiFiFixture.connect(HOSTAPD) as control:
            control.request_ok(f"REMOVE {self.ifname}")
            if self.ifname in control.request("INTERFACES").split():
                raise WiFiFixtureError(f"hostapd still holds {self.ifname} after the access point on it was removed")

    def close(self) -> None:
        """Takes the access point down if it is still up, without failing if it cannot."""
        try:
            self.stop()
        except WiFiFixtureError as error:
            # The radio is released either way when the fixture next restarts hostapd.
            log.warning("Could not take the access point on %s down: %s", self.ifname, error)

    def request(self, command: str, timeout: float = DEFAULT_TIMEOUT) -> str:
        """Sends a command to the control socket of this access point."""
        if self._control is None:
            raise WiFiFixtureError(f"There is no access point running on {self.ifname}")
        return self._control.request(command, timeout)

    def request_ok(self, command: str, timeout: float = DEFAULT_TIMEOUT) -> None:
        """Sends a command that hostapd answers with OK, and fails on anything else."""
        if self._control is None:
            raise WiFiFixtureError(f"There is no access point running on {self.ifname}")
        self._control.request_ok(command, timeout)

    def reload(self) -> None:
        """Re-reads the configuration file and applies it, without taking the BSS down.

        hostapd reads the files a configuration refers to, an EAP user list or a certificate
        among them, only when it reads the configuration itself, so this is the only way to
        change one of those while the network stays up. Note that every station is
        deauthenticated in the process, because a configuration carrying no config_id= is
        taken to be a different configuration than the one it replaces.
        """
        log.info("Reloading the configuration of the access point on %s", self.ifname)
        self.request_ok("RELOAD_CONFIG", timeout=RELOAD_TIMEOUT)

    def next_event(self, timeout: float = DEFAULT_TIMEOUT) -> Event | None:
        """Returns the next event hostapd has sent about this access point, if any."""
        if self._control is None:
            raise WiFiFixtureError(f"There is no access point running on {self.ifname}")
        return self._control.next_event(timeout)

    def status(self) -> dict[str, str]:
        """Returns what hostapd reports about this access point, as key value pairs."""
        return _parse_properties(self.request("STATUS"))

    def stations(self) -> dict[str, Station]:
        """Returns the stations hostapd has for this access point, keyed by MAC address."""
        stations: dict[str, Station] = {}
        reply = self.request("STA-FIRST")
        while reply and not reply.startswith("FAIL"):
            station = _parse_station(reply)
            stations[station.address] = station
            reply = self.request(f"STA-NEXT {station.address}")
        return stations

    def station(self, address: str) -> Station | None:
        """Returns what hostapd has for one station, or None if it has nothing."""
        reply = self.request(f"STA {address}")
        if not reply or reply.startswith("FAIL"):
            return None
        return _parse_station(reply)

    def wait_for_station(self, timeout: float = STATION_TIMEOUT) -> Station:
        """Waits for a station to join the network and returns it.

        Joining means the four-way handshake has completed, which is the fixture's own
        account of a client having got onto the network: an association alone says nothing
        about whether the client had the credentials for it. The state is checked once
        before waiting, so that a station that joined earlier still counts, and whatever
        else hostapd reports in the meantime ends up in the failure message -- a client
        that tried and was turned away is worth telling apart from one that never came.
        """
        for station in self.stations().values():
            if station.authorized:
                return station

        deadline = time.monotonic() + timeout
        seen: list[str] = []
        while True:
            event = self.next_event(max(deadline - time.monotonic(), 0))
            if event is None:
                raise WiFiFixtureError(
                    f"No station joined the access point on {self.ifname} within {timeout} seconds"
                    + (f", hostapd reported: {'; '.join(seen)}" if seen else ""))
            if event.name == AP_STA_CONNECTED:
                # The address hostapd puts first, ahead of the optional keyid and the rest.
                address = event.arguments.split()[0]
                station = self.station(address)
                if station is None:
                    raise WiFiFixtureError(f"hostapd reported {address} as connected to {self.ifname}, "
                                           "and then knew nothing about it")
                return station
            seen.append(str(event))

    def _config_text(self) -> str:
        options = {
            "interface": self.ifname,
            "driver": "nl80211",
            # Where hostapd creates the control socket of the BSS, named after the
            # interface. The same directory its global socket is in.
            "ctrl_interface": str(WiFiFixture.directory(HOSTAPD)),
            **self.options,
        }

        lines = [f"# Written for {self.radio.phy} by wifi_fixture.py"]
        for key, value in options.items():
            text = str(value)
            if "\n" in text:
                raise WiFiFixtureError(f"The value of the hostapd option {key} spans more than one line")
            lines.append(f"{key}={text}")
        return "\n".join(lines) + "\n"

    def _connect_to_bss(self, timeout: float = DEFAULT_TIMEOUT) -> ControlConnection:
        # ADD only returns once the BSS is set up, so this is a formality, but it beats
        # reporting a missing socket as an unreachable fixture.
        socket_path = WiFiFixture.directory(HOSTAPD) / self.ifname
        deadline = time.monotonic() + timeout
        while not socket_path.exists():
            if time.monotonic() > deadline:
                raise WiFiFixtureError(f"hostapd added the access point on {self.ifname} without a control socket")
            time.sleep(POLL_INTERVAL)
        return WiFiFixture.connect(HOSTAPD, self.ifname)


class WiFiFixtureMixin:
    """Gives a test class the Wi-Fi fixture, and hands the radios back after every test case.

    A test case reaches the fixture through self.wifi_fixture and drives a radio through a
    role class such as AccessPointFixture, and does not have to take anything down itself:
    whatever it left running is released when the fixture is reset here. Name this ahead of
    the test base class, `class TC_FOO_1_2(WiFiFixtureMixin, MatterBaseTest)`, so that the
    reset runs before the framework's own teardown. A test class that overrides
    teardown_test has to call super().teardown_test() for it to run at all.
    """

    @property
    def wifi_fixture(self) -> type[WiFiFixture]:
        """The Wi-Fi fixture of the harness. There is one, so this is the class itself."""
        return WiFiFixture

    def teardown_test(self) -> None:
        # Restarting the daemons is the fixture's only reset, so this is worth skipping for
        # the test cases of a class that do not all use Wi-Fi.
        WiFiFixture.reset_if_used()
        super().teardown_test()
