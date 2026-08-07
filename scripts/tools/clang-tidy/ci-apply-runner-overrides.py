#!/usr/bin/env python3
"""Inject CI-only clang-tidy overrides into run-clang-tidy-on-compile-commands.py.

The shared runner exposes no flag for these, so the nightly whole-tree scan patches
the (ephemeral CI checkout of the) runner in place. This never lands in a commit of
the runner itself, so the PR clang-tidy job is unaffected. Rationale:

  -Wno-error               The compile commands carry -Werror. Warnings clang-tidy
                           emits but the original build does not
                           (clang-diagnostic-missing-format-attribute,
                           -thread-safety-attributes) become fatal, so ~200 TUs abort
                           with "Found compiler error(s)" before the check ever runs.
  --header-filter=src/     Frontend checks (bugprone-*) do NOT display a finding whose
                           location is a header unless this is set; much C++ code is
                           inline in headers/templates.
  --warnings-as-errors=-*  Override the repo .clang-tidy WarningsAsErrors:'*' so a TU
                           that merely HAS a finding exits 0 and is not miscounted as a
                           failed/aborted TU (keeps "Failed to process" honest).

Idempotent. Asserts the anchor still exists so a future runner rewrite fails loudly
(re-roll the overrides) instead of silently scanning ~all TUs broken again.
"""
import os
import sys

# Resolve the runner relative to this script so it works regardless of CWD.
RUNNER = os.path.normpath(
    os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "run-clang-tidy-on-compile-commands.py")
)
ANCHOR = "            self.clang_arguments = command_items[1:]\n"
INJECT = (
    '            self.clang_arguments.append("-Wno-error")\n'
    '            self.tidy_arguments.append("--header-filter=src/")\n'
    '            self.tidy_arguments.append("--warnings-as-errors=-*")\n'
)


def main() -> int:
    with open(RUNNER, encoding="utf-8") as fh:
        src = fh.read()
    if '"-Wno-error"' in src and "--header-filter=src/" in src:
        print("runner already patched; nothing to do")
        return 0
    if ANCHOR not in src:
        print(f"ERROR: anchor not found in {RUNNER}; re-roll the CI overrides", file=sys.stderr)
        return 1
    with open(RUNNER, "w", encoding="utf-8") as fh:
        fh.write(src.replace(ANCHOR, ANCHOR + INJECT, 1))
    print(f"patched {RUNNER} with CI clang-tidy overrides")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
