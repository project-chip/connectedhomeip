import shlex
import subprocess

from mobly.utils import stop_standing_subprocess


class BashRunner:
    """
    Uses subprocess to execute bash commands
    Intended to be instantiated and then only interacted with through instance methods
    """

    def __init__(self, command: str, sync: bool = False,
                 capture_output: bool = False) -> None:
        """
        Run a bash command as a sub process
        :param command: Command to run
        :param sync: If True, wait for command to terminate
        :param capture_output: Only applies to sync; if True, store stdout and stderr
        """
        self.command: str = command
        self.sync = sync
        self.capture_output = capture_output

        self.args: list[str] = []
        self._init_args()
        self.proc: subprocess.Popen[bytes] | subprocess.CompletedProcess[bytes] | None = None

        if self.sync:
            self.proc = subprocess.run(
                self.args, capture_output=capture_output)
        else:
            self.proc = subprocess.Popen(self.args)

    def _init_args(self) -> None:
        """Escape quotes, call bash, and prep command for subprocess args"""
        command_escaped = self.command.replace('"', '\"')
        self.args = shlex.split(f'/bin/bash -c "{command_escaped}"')

    def command_is_running(self) -> bool:
        """Check if subproc is still running"""
        return self.proc is not None and self.proc.poll() is None

    def get_captured_output(self) -> str:
        """Return captured output when the relevant instance var is set"""
        if not self.capture_output or not self.sync:
            return ""
        return self.proc.stdout.decode().strip()

    def stop_command(self, soft: bool = False) -> None:
        # TODO: Make this uniform
        if self.command_is_running():
            if soft:
                self.proc.terminate()
                if self.proc.stdout:
                    self.proc.stdout.close()
                if self.proc.stderr:
                    self.proc.stderr.close()
                self.proc.wait()
            else:
                stop_standing_subprocess(self.proc)
        else:
            print(f'WARNING {self.command} stop requested while not running')
        self.proc = None
