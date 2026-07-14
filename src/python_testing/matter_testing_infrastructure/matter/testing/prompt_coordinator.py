#
#    Copyright (c) 2025 Project CHIP Authors
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

"""Interactive timeout prompt coordinator for Matter testing.

Serializes timeout prompts across concurrent timeout sources (per-step and overall)
and tracks time spent in prompts so the overall supervisor clock does not tick
while the user is being questioned.
"""

import sys
import threading
import time
from typing import Callable, Optional


class PromptCoordinator:
    """Serializes interactive timeout prompts and tracks paused time.

    Two independent timeout sources — per-step callbacks (Component B) and the
    overall supervisor thread (Component A) — both route through this coordinator.
    A threading.Lock ensures only one prompt is displayed at a time.  Time spent
    waiting for the lock and time spent reading user input are both accumulated in
    ``total_prompt_seconds`` so Component A's effective elapsed calculation does not
    penalise the user for time already spent answering prompts.

    In non-interactive mode (no controlling TTY, i.e. CI), ``ask_user`` always
    returns False without printing anything.
    """

    def __init__(self):
        self._lock = threading.Lock()
        self._total_prompt_seconds = 0.0
        self._ps_lock = threading.Lock()

    @property
    def is_interactive(self) -> bool:
        return sys.stdin.isatty()

    @property
    def total_prompt_seconds(self) -> float:
        with self._ps_lock:
            return self._total_prompt_seconds

    def _add_prompt_time(self, seconds: float) -> None:
        with self._ps_lock:
            self._total_prompt_seconds += seconds

    def ask_user(
        self,
        description: str,
        elapsed_sec: float,
        extension_sec: float = 600.0,
        still_needed: Optional[Callable[[], bool]] = None,
    ) -> bool:
        """Prompt the user to extend or abort on a timeout.

        Args:
            description: Human-readable description of what timed out.
            elapsed_sec: Effective elapsed seconds at the time of the call (shown to user).
            extension_sec: How many seconds to add if the user says yes.
            still_needed: Optional callable with no arguments that returns True when the
                prompt is still warranted.  Called after acquiring the lock — if it returns
                False (another prompt already resolved the situation), the prompt is skipped
                and True is returned immediately.

        Returns:
            True if the user chose to extend (or the situation was resolved by another
            concurrent prompt). False to abort or in non-interactive mode.
        """
        if not self.is_interactive:
            return False

        t_wait_start = time.time()
        self._lock.acquire()
        wait_time = time.time() - t_wait_start
        self._add_prompt_time(wait_time)

        try:
            if still_needed is not None and not still_needed():
                # The timeout was resolved while we were waiting for the lock.
                return True

            prompt_start = time.time()
            print(
                f"\n⏱  Timeout: {description}\n"
                f"   Elapsed: {elapsed_sec:.0f}s — extend by {extension_sec:.0f}s? "
                f"[y]es / [n]o (abort): ",
                end="",
                flush=True,
            )
            answer = sys.stdin.readline().strip().lower()
            self._add_prompt_time(time.time() - prompt_start)

            return answer in ("y", "yes")
        finally:
            self._lock.release()
