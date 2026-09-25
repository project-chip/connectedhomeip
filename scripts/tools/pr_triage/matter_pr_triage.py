#!/usr/bin/env python3
# ruff: noqa: UP017  -- timezone.utc rather than datetime.UTC keeps Python 3.8 to 3.10 working
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
"""matter-pr-triage backend.

Deterministic half of the PR backlog triage tool: select, collect, signals, report.
The judgment step between `signals` and `report` is done by a model and writes judgment.json.

Never writes to GitHub. Never prompts. JSON is the source of truth; markdown renders from it.
Exit codes: 0 ok, 1 error, 2 preflight failed.
"""

import argparse
import contextlib
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
import time
from datetime import datetime, timedelta, timezone
from pathlib import Path

try:
    import fcntl  # POSIX; absent on Windows, where the lock degrades to none
except ImportError:                   # pragma: no cover
    fcntl = None

TOOL_VERSION = "1.0.0"
DEFAULT_CAP = 25      # pull requests per batch; measured, see BENCHMARKS.md
SEARCH_RETRIEVAL_LIMIT = 1000  # GitHub will not page past this many search results
# Authors whose pull requests --include-bots controls. Distinct from MECHANICAL_AUTHORS below,
# which is about who wrote a comment: dependabot opens pull requests but rarely comments, and
# CLAassistant is the reverse.
BOT_AUTHORS = {"dependabot", "restyled-io", "github-actions", "renovate"}

# ---------------------------------------------------------------- cost

# What a run costs in time and calls. Not money: the script buys nothing. Tokens are the agent's,
# not the script's, so the only honest number here is the size of the material it hands over.
COST = {"graphql_calls": 0, "rest_calls": 0, "rate_limit_waits": 0}

# The fields a judging pass actually reads. The rest of a dossier is mostly bot comments.
JUDGING_KEYS = ("number", "title", "body", "changed_paths", "diffstat", "signals",
                "linked_issues", "review_decision", "draft", "created_at", "updated_at", "base_ref")


def log_cost(root, command, run_id, seconds):
    """Append one line per phase. A log rather than a field so re-running a phase adds to the bill
    instead of hiding the earlier attempt."""
    try:
        with (root / "cost.jsonl").open("a", encoding="utf-8") as fh:
            fh.write(json.dumps({"at": datetime.now(timezone.utc).isoformat(timespec="seconds"),
                                 "command": command, "run": run_id, "seconds": round(seconds, 1),
                                 "graphql_calls": COST["graphql_calls"],
                                 "rest_calls": COST["rest_calls"],
                                 "rate_limit_waits": COST["rate_limit_waits"]}) + "\n")
    except OSError:
        pass                          # telemetry must never fail a run


# ---------------------------------------------------------------- shell helpers


def run(cmd, cwd=None, check=True, timeout=120):
    """Run a command and return stdout. Raises RuntimeError on failure when check."""
    # errors="replace": a repository contains binaries, and reading one must degrade to unusable
    # text rather than killing the run with a UnicodeDecodeError.
    try:
        p = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True, errors="replace",
                           timeout=timeout)
    except FileNotFoundError:
        # The binary itself is missing. A traceback helps nobody; say which one, or report
        # failure the way a non-zero exit would.
        if check:
            raise RuntimeError(f"{cmd[0]} is not installed")
        return ""
    if check and p.returncode != 0:
        raise RuntimeError(f"{' '.join(cmd[:3])}... failed ({p.returncode}): {p.stderr.strip()[:400]}")
    return p.stdout


def gh_graphql(query, variables=None, retries=4):
    """Call the GraphQL API through gh. Pauses and retries on rate limiting."""
    COST["graphql_calls"] += 1
    cmd = ["gh", "api", "graphql", "-f", f"query={query}"]
    for key, value in (variables or {}).items():
        # -F converts types, which is right for an Int variable and wrong for every String one.
        cmd += (["-F", f"{key}={value}"] if isinstance(value, int)
                else ["-f", f"{key}={value}"])
    for attempt in range(retries):
        p = subprocess.run(cmd, capture_output=True, text=True, timeout=180)
        if p.returncode == 0:
            data = json.loads(p.stdout)
            if "errors" in data and data["errors"]:
                msg = json.dumps(data["errors"])[:300]
                if "RATE_LIMITED" in msg or "secondary rate" in msg.lower():
                    COST["rate_limit_waits"] += 1
                    time.sleep(20 * (attempt + 1))
                    continue
                raise RuntimeError(f"graphql: {msg}")
            return data["data"]
        if "rate limit" in p.stderr.lower():
            COST["rate_limit_waits"] += 1
            time.sleep(20 * (attempt + 1))
            continue
        raise RuntimeError(f"gh graphql failed: {p.stderr.strip()[:400]}")
    raise RuntimeError("gh graphql: rate limited after retries")


def gh_rest(path, retries=4):
    COST["rest_calls"] += 1
    for attempt in range(retries):
        p = subprocess.run(["gh", "api", path], capture_output=True, text=True, timeout=180)
        if p.returncode == 0:
            return json.loads(p.stdout)
        if "rate limit" in p.stderr.lower():
            COST["rate_limit_waits"] += 1
            time.sleep(20 * (attempt + 1))
            continue
        raise RuntimeError(f"gh api {path} failed: {p.stderr.strip()[:300]}")
    raise RuntimeError(f"gh api {path}: rate limited after retries")


# ---------------------------------------------------------------- paths and state


def cache_root(owner, repo):
    base = os.environ.get("MATTER_PR_TRIAGE_CACHE")
    root = Path(base) if base else Path.home() / ".cache" / "matter-pr-triage"
    return root / owner / repo


def reports_root(owner, repo):
    """Where reports land: a visible folder a person opens, apart from the cache nobody reads.
    The sibling tool writes its reports here too."""
    base = os.environ.get("MATTER_PR_TRIAGE_REPORTS")
    root = Path(base) if base else Path.home() / "matter-pr-triage-reports"
    return root / owner / repo


@contextlib.contextmanager
def locked(path):
    """Advisory lock around a read-modify-write of a shared file.

    Two agents rendering reports at once, which happens on a shared cache, would otherwise each
    read state.json, add their verdicts and write, and the second write would drop the first's.
    POSIX only; where fcntl is missing the block runs unlocked rather than failing.
    """
    lock_path = Path(str(path) + ".lock")
    lock_path.parent.mkdir(parents=True, exist_ok=True)
    with lock_path.open("a") as fh:
        if fcntl:
            fcntl.flock(fh, fcntl.LOCK_EX)
        try:
            yield
        finally:
            if fcntl:
                fcntl.flock(fh, fcntl.LOCK_UN)


def write_json(path, obj):
    """Write atomically. A run killed mid-write must not leave unreadable JSON behind."""
    path.parent.mkdir(parents=True, exist_ok=True)
    tmp = path.with_suffix(path.suffix + ".tmp")
    tmp.write_text(json.dumps(obj, indent=2, sort_keys=False) + "\n", encoding="utf-8")
    tmp.replace(path)


def read_json(path, default=None):
    if not path.exists():
        return default
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except json.JSONDecodeError as e:
        raise RuntimeError(f"{path} is not valid JSON ({e}). Delete it and re-run that phase.")


def split_repo(repo_arg):
    """owner/name from a --repo value, or a message a person can act on.

    Accepts what people paste: owner/name, a github.com URL with or without scheme or .git, or an
    SSH remote. A pull request URL is refused, because its extra path is not a repository. Every
    caller goes through here rather than splitting on "/" and reaching the user as a traceback.
    """
    text = re.sub(r"^(?:https?://)?(?:www\.)?(?:git@)?github\.com[/:]", "", (repo_arg or "").strip())
    text = re.sub(r"\.git$", "", text.rstrip("/"))
    owner, _, name = text.partition("/")
    # Both halves become directory names under the cache, so a dot-only half would point outside it.
    if (not owner or not name or "/" in name or any(c in text for c in " :@?#")
            or owner.strip(".") == "" or name.strip(".") == ""):
        raise RuntimeError(f"--repo takes owner/name or a github.com repository URL, not {repo_arg!r}")
    return owner, name


def infer_repo(checkout):
    """owner/repo for the upstream of this checkout, preferring an 'upstream' remote."""
    remotes = run(["git", "-C", str(checkout), "remote"], check=False).split()
    for name in ("upstream", "origin"):
        if name in remotes:
            url = run(["git", "-C", str(checkout), "remote", "get-url", name], check=False).strip()
            m = re.search(r"[:/]([^/:]+)/([^/]+?)(?:\.git)?$", url)
            if m:
                return m.group(1), m.group(2)
    raise RuntimeError("could not infer owner/repo from the checkout remotes")


def default_branch(checkout):
    """The upstream default branch. Not every repo calls it master."""
    for remote in ("upstream", "origin"):
        out = run(["git", "-C", str(checkout), "symbolic-ref", "--quiet",
                   f"refs/remotes/{remote}/HEAD"], check=False).strip()
        if out:
            return out.rsplit("/", 1)[-1]
    for name in ("main", "master"):
        if resolve_commit(checkout, name)[0]:
            return name
    raise RuntimeError("cannot determine the default branch. Pass --base-ref.")


def resolve_commit(checkout, ref):
    """Resolve a ref to a commit, or (None, None).

    `git rev-parse` echoes the ref back on stdout when it cannot resolve it, so testing for
    non-empty output silently accepts garbage. `--verify` with a commit peel is the honest form.
    """
    p = subprocess.run(["git", "-C", str(checkout), "rev-parse", "--verify", "--quiet",
                        f"{ref}^{{commit}}"], capture_output=True, text=True)
    sha = p.stdout.strip()
    return (sha, ref) if p.returncode == 0 and sha else (None, None)


def remote_base_commit(owner, name, base_ref=None):
    """Resolve the base branch and its head through the API, for a run with no checkout."""
    if not base_ref:
        base_ref = gh_rest(f"/repos/{owner}/{name}")["default_branch"]
    sha = gh_rest(f"/repos/{owner}/{name}/commits/{base_ref}")["sha"]
    return sha, base_ref


def base_commit(checkout, base_ref):
    """The commit to judge coverage against, preferring a remote-tracking ref over a local one."""
    for ref in (f"upstream/{base_ref}", f"origin/{base_ref}", base_ref):
        sha, found = resolve_commit(checkout, ref)
        if sha:
            return sha, found
    raise RuntimeError(f"cannot resolve {base_ref} in the checkout. "
                       f"Fetch it, or pass --base-ref with a ref that exists.")


# ---------------------------------------------------------------- preflight


def work_in_progress(checkout):
    """What the user has half-finished in this clone, so a fetch never lands in the middle of it.

    Reading the object database cannot disturb anything, but `--fix` writes: it fetches. A fetch
    during a rebase or a merge does not corrupt them, yet it moves refs under someone mid-task and
    can block on an index lock, so the honest move is to refuse and say what is in the way.
    """
    git = Path(checkout) / ".git"
    if git.is_file():                      # a worktree: .git is a file pointing at the real dir
        try:
            git = Path(git.read_text(encoding="utf-8").split(":", 1)[1].strip())
        except (OSError, IndexError):
            return None
    for marker, what in (("rebase-merge", "a rebase"), ("rebase-apply", "a rebase or am"),
                         ("MERGE_HEAD", "a merge"), ("CHERRY_PICK_HEAD", "a cherry-pick"),
                         ("REVERT_HEAD", "a revert"), ("BISECT_LOG", "a bisect"),
                         ("index.lock", "another git command")):
        if (git / marker).exists():
            return what
    return None


def resolve_mode(checkout, repo_arg, remote):
    """Whether to read evidence from a clone or from the API.

    `--remote` forces the API. Otherwise a clone is used when one is here and is the repository
    being triaged, because local reads are faster and spend no API calls. When it is not, the run
    falls back to the API rather than failing: cloning a large repository to triage it is a poor
    trade, and every check but the repo-wide literal search works either way.
    """
    # Validate before the try below, or a malformed --repo is swallowed as "no clone here" and the
    # run proceeds against nothing.
    named = split_repo(repo_arg) if repo_arg else None
    if remote:
        return True, "asked for with --remote"
    if not shutil.which("git"):
        return True, "git is not installed, so the API is used"
    try:
        owner, name = named or infer_repo(Path(checkout))
    except RuntimeError:
        return True, "no clone here to infer a repository from"
    sha, _ = resolve_commit(Path(checkout), "HEAD")
    if not sha:
        return True, "no git history here"
    try:
        here_owner, here_name = infer_repo(Path(checkout))
    except RuntimeError:
        return True, "this directory is not a clone"
    if (here_owner.lower(), here_name.lower()) != (owner.lower(), name.lower()):
        return True, f"this clone is {here_owner}/{here_name}, not {owner}/{name}"
    return False, "using the clone here"


def preflight_checks(checkout, repo_arg, remote=False, base_ref_for_check=None):
    missing = []
    needed = [("gh", "all GitHub access goes through it", "https://cli.github.com")]
    inferable = True
    if remote and not repo_arg:
        try:
            infer_repo(checkout)
        except RuntimeError:
            inferable = False
    if remote and not repo_arg and not inferable:
        # Nothing was checked against a repository, so "ok" would mean only that gh runs. That
        # reads as success and is not one: say what is missing instead of passing vacuously.
        missing.append({
            "item": "no repository",
            "why": "no clone here to infer one from and no --repo given, so nothing was checked "
                   "against a repository at all",
            "fix": "name it with --repo owner/name, or run from a clone of it",
        })
    if not remote:
        # Remote mode reads everything through the API, so git is not involved at all.
        needed.append(("git", "blame and file lookups run against a local checkout",
                       "https://git-scm.com/downloads"))
    for binary, why, fix in needed:
        if not shutil.which(binary):
            missing.append({"item": binary, "why": why, "fix": fix})

    if shutil.which("gh"):
        p = subprocess.run(["gh", "auth", "status"], capture_output=True, text=True)
        if p.returncode != 0:
            missing.append({"item": "gh auth", "why": "queries need an authenticated gh",
                            "fix": "gh auth login"})

    if shutil.which("git") and not remote:
        inside = run(["git", "-C", str(checkout), "rev-parse", "--is-inside-work-tree"],
                     check=False).strip()
        if inside != "true":
            missing.append({"item": "checkout", "why": f"{checkout} is not a git checkout",
                            "fix": "run from the target repo, or pass --checkout"})
        else:
            shallow = run(["git", "-C", str(checkout), "rev-parse", "--is-shallow-repository"],
                          check=False).strip()
            if shallow == "true":
                missing.append({
                    "item": "shallow clone",
                    "degrades": True,
                    "why": "blame cannot see past the graft commit, so pull requests that modify "
                           "existing code come back undetermined. Pull requests that add files or "
                           "lines are unaffected, because those tests need no history.",
                    "fix": "git fetch --unshallow --filter=blob:none",
                    "autofix": True,
                })
        if inside == "true" and shutil.which("gh"):
            # A clone is pinned to whatever its remote-tracking ref holds, and nothing fetches it.
            # Judging a year-old backlog against a base that is weeks behind is a quiet wrong
            # answer, so say so; remote mode always reads the branch as it is now.
            try:
                base = base_ref_for_check or default_branch(checkout)
                local_sha, ref = base_commit(checkout, base)
                owner, repo = infer_repo(checkout)
                head = gh_rest(f"/repos/{owner}/{repo}/commits/{base}")["sha"]
                if local_sha != head:
                    missing.append({
                        "item": "stale clone",
                        "degrades": True,
                        "why": f"{ref} here is {local_sha[:9]} but {base} is {head[:9]}; verdicts "
                        "would be judged against a base that has moved on",
                        "fix": f"git fetch {ref.split('/')[0]} {base}",
                        "fetch": [ref.split("/")[0], base],
                        "alternative": "--remote reads the branch as it is now and touches "
                                       "nothing in this clone",
                        "autofix": True,
                    })
            except (RuntimeError, KeyError, TypeError):
                pass          # a repo that cannot be resolved is reported by the checks below
        if inside == "true" and repo_arg:
            try:
                owner, repo = infer_repo(checkout)
                if f"{owner}/{repo}".lower() != repo_arg.lower():
                    missing.append({
                        "item": "checkout/repo mismatch",
                        "why": f"checkout is {owner}/{repo} but --repo is {repo_arg}; "
                        "blame and probe results would be meaningless",
                        "fix": "point --checkout at a clone of the target repo",
                    })
            except RuntimeError as e:
                missing.append({"item": "remote", "why": str(e), "fix": "add an origin or upstream remote"})

    return missing


# ---------------------------------------------------------------- selection


SEARCH_QUERY = """
query($q: String!, $after: String) {
  search(query: $q, type: ISSUE, first: 100, after: $after) {
    issueCount
    pageInfo { hasNextPage endCursor }
    nodes {
      ... on PullRequest {
        number title url isDraft createdAt updatedAt headRefOid
        author { login }
        labels(first: 20) { nodes { name } }
      }
    }
  }
}
"""


def build_query(owner, repo, state, cutoff, date_field, labels, exclude_labels,
                authors, exclude_authors, drafts, review):
    parts = [f"repo:{owner}/{repo}", "is:pr", f"is:{state}"]
    parts.append(f"{'updated' if date_field == 'activity' else 'created'}:<{cutoff}")
    parts += [f'label:"{x}"' for x in labels]
    parts += [f'-label:"{x}"' for x in exclude_labels]
    parts += [f"author:{x}" for x in authors]
    parts += [f"-author:{x}" for x in exclude_authors]
    if drafts == "only":
        parts.append("draft:true")
    elif drafts == "exclude":
        parts.append("draft:false")
    if review:
        parts.append(f"review:{review}")
    return " ".join(parts)


def search_page(query_string, cap_pages=10):
    """Page through search results. Returns (nodes, issue_count)."""
    nodes, after, count = [], None, 0
    for _ in range(cap_pages):
        variables = {"q": query_string}
        if after:
            variables["after"] = after
        data = gh_graphql(SEARCH_QUERY, variables)["search"]
        count = data["issueCount"]
        nodes += [n for n in data["nodes"] if n]
        if not data["pageInfo"]["hasNextPage"]:
            break
        after = data["pageInfo"]["endCursor"]
    return nodes, count


def resolve_run(root, ref):
    """A run directory by id, or the most recent one when given 'latest'."""
    runs = root / "runs"
    if ref != "latest":
        return runs / ref
    existing = sorted(d for d in runs.glob("*") if (d / "manifest.json").exists())
    if not existing:
        raise RuntimeError("no previous runs to continue from")
    return existing[-1]


# ---------------------------------------------------------------- cli


def apply_fixes(checkout, missing):
    """Run the remedies the tool is allowed to apply. Only additive, local git operations."""
    applied = []
    busy = work_in_progress(checkout)
    if busy:
        # Every fix here writes to the user's repository. Not while they are in the middle of
        # something: report it and let them choose when.
        return [{"item": m["item"], "ok": False,
                 "detail": f"refused: {busy} is in progress in this clone. Finish or abort it, "
                 f"then re-run, or run this yourself: {m['fix']}"}
                for m in missing if m.get("autofix")]
    for item in missing:
        if not item.get("autofix"):
            continue
        if item["item"] == "stale clone":
            remote_name, branch = item["fetch"]
            p = subprocess.run(["git", "-C", str(checkout), "fetch", remote_name, branch],
                               capture_output=True, text=True, errors="replace", timeout=3600)
            applied.append({"item": item["item"], "ok": p.returncode == 0,
                            "detail": "done" if p.returncode == 0 else p.stderr.strip()[-200:]})
        if item["item"] == "shallow clone":
            # blob:none keeps full commit history, which is all blame needs, and skips the file
            # contents, which is most of the download. Fall back if the server refuses filtering.
            p = subprocess.run(["git", "-C", str(checkout), "fetch", "--unshallow",
                                "--filter=blob:none"],
                               capture_output=True, text=True, timeout=3600)
            if p.returncode != 0 and "filter" in p.stderr.lower():
                p = subprocess.run(["git", "-C", str(checkout), "fetch", "--unshallow"],
                                   capture_output=True, text=True, timeout=3600)
            ok = p.returncode == 0 or "on a complete repository" in p.stderr
            applied.append({"item": item["item"], "ok": ok,
                            "detail": (p.stderr.strip()[-200:] or "done") if not ok else "done"})
    return applied


def preflight(checkout, repo, fix, remote):
    """Check dependencies and access. Never installs, never prompts."""
    checkout = Path(checkout).resolve()
    remote, why = resolve_mode(checkout, repo, remote)
    missing = preflight_checks(checkout, repo, remote)
    applied = []
    if fix:
        applied = apply_fixes(checkout, missing)
        if applied:
            missing = preflight_checks(checkout, repo, remote)   # re-check after fixing
    blocking = [m for m in missing if not m.get("degrades")]
    payload = {"ok": not blocking, "missing": missing,
               "mode": "remote" if remote else "local", "mode_because": why}
    if shutil.which("gh"):
        try:
            core = gh_rest("/rate_limit")["resources"]["core"]
            resets = datetime.fromtimestamp(core["reset"], timezone.utc).isoformat(timespec="seconds")
            payload["rate_limit"] = {"remaining": core["remaining"], "resets_at": resets}
            if core["remaining"] < 120:
                payload["warning"] = (f"only {core['remaining']} API calls left until {resets}; "
                                      "a batch of 25 takes roughly 40 to 60")
        except Exception:
            # A failed rate query is not a preflight failure; the run will just find out later.
            pass
    if fix:
        payload["fixed"] = applied
    if any(m.get("autofix") for m in missing):
        payload["hint"] = "re-run with --fix to apply the remedies marked autofix"
    print(json.dumps(payload, indent=2))
    sys.exit(2 if blocking else 0)


def select(checkout, repo, state, older_than, updated_before, date_field, labels,
           exclude_labels, authors, exclude_authors, drafts, review, cap, include_bots,
           skip_triaged, continue_from, base_ref, depth, label, remote, revisit):
    """Pick the batch. Deterministic: least recently updated first, first N taken."""
    checkout = Path(checkout).resolve()
    if remote and not repo:
        raise RuntimeError("--remote needs --repo owner/name: there is no checkout to infer it from")
    owner, name = split_repo(repo) if repo else infer_repo(checkout)

    if updated_before:
        cutoff = updated_before
    else:
        m = re.fullmatch(r"(\d+)(d|w|mo|y)", older_than)
        if not m:
            raise RuntimeError("--older-than must look like 365d, 12w, 18mo or 2y")
        n, unit = int(m.group(1)), m.group(2)
        days = {"d": 1, "w": 7, "mo": 30, "y": 365}[unit] * n
        cutoff = (datetime.now(timezone.utc) - timedelta(days=days)).strftime("%Y-%m-%d")

    narrowed, continues, truncated = [], None, False
    root_early = cache_root(owner, name)
    if continue_from:
        source = resolve_run(root_early, continue_from)
        prior = read_json(source / "manifest.json")
        if not prior:
            raise RuntimeError(f"no run to continue from at {source}")
        # Reuse the query verbatim: same window, next batch.
        query_string = prior["query"]
        cutoff = prior["cutoff"]
        continues = prior["run"]
        if cap is None:
            cap = prior.get("cap", DEFAULT_CAP)
        depth = prior.get("depth", depth)
    else:
        query_string = build_query(owner, name, state, cutoff, date_field, list(labels),
                                   list(exclude_labels), list(authors), list(exclude_authors),
                                   drafts, review)
    nodes, total = search_page(query_string)

    # The API will not page past 1000 results. Narrow the window until the population fits.
    while total > SEARCH_RETRIEVAL_LIMIT and not continue_from:
        cutoff_dt = datetime.strptime(cutoff, "%Y-%m-%d")
        older = (cutoff_dt - timedelta(days=180)).strftime("%Y-%m-%d")
        narrowed.append({"from": cutoff, "to": older, "population_was": total})
        cutoff = older
        query_string = build_query(owner, name, state, cutoff, date_field, list(labels),
                                   list(exclude_labels), list(authors), list(exclude_authors),
                                   drafts, review)
        nodes, total = search_page(query_string)
        if len(narrowed) > 8:
            break
    truncated = total > SEARCH_RETRIEVAL_LIMIT

    if not include_bots:
        nodes = [n for n in nodes
                 if (n.get("author") or {}).get("login", "").lower().replace("[bot]", "") not in BOT_AUTHORS]

    nodes.sort(key=lambda n: n["updatedAt"])  # least recently updated first
    root = cache_root(owner, name)
    state_file = read_json(root / "state.json", {}) or {}
    for n in nodes:
        recorded = state_file.get(str(n["number"]))
        n["recorded_verdict"] = (recorded or {}).get("verdict")
        n["already_triaged"] = bool(recorded) and n["recorded_verdict"] not in revisit

    remaining = [n for n in nodes if not (skip_triaged and n["already_triaged"])]
    already = len(nodes) - len(remaining)
    skipped_by_verdict = {}
    for n in nodes:
        if skip_triaged and n["already_triaged"]:
            key = n["recorded_verdict"] or "unrecorded"
            skipped_by_verdict[key] = skipped_by_verdict.get(key, 0) + 1

    # Nothing left under this query means the set is finished. Say so instead of writing an
    # empty run that a later continuation would have to step over.
    if not remaining:
        undecided = skipped_by_verdict.get("unclear", 0)
        if not total:
            # Nothing matched and nothing left to do are different answers, and telling them apart
            # is how the activity-date trap gets caught: where a bulk update has touched every
            # pull request, an activity window matches zero while a creation one matches hundreds.
            message = ("no pull request matched this window at all. If a bulk update has touched "
                       "every pull request in this repository, last-activity dates are useless "
                       "here: re-run with --date-field created.")
        else:
            message = "every pull request matching this query has been triaged. Widen the window, or "
            message += ("use --include-triaged to go over them again."
                        if not undecided else
                        f"bring back the {undecided} nothing could be decided about with "
                        "--revisit unclear.")
        print(json.dumps({
            "complete": True,
            "repo": f"{owner}/{name}",
            "matched": total,
            "already_triaged": already,
            "skipped_by_verdict": skipped_by_verdict,
            "message": message,
        }, indent=2))
        return

    selected = remaining[:cap]
    remote, mode_why = resolve_mode(checkout, f"{owner}/{name}", remote)
    if remote:
        sha, ref = remote_base_commit(owner, name, base_ref)
    else:
        sha, ref = base_commit(checkout, base_ref or default_branch(checkout))
    # Timestamp first so runs sort chronologically and "latest" stays meaningful. An optional
    # label follows it, which is what makes a report findable by name months later.
    slug = re.sub(r"[^a-z0-9]+", "-", (label or "").lower()).strip("-")[:40]
    stamp = datetime.now(timezone.utc).strftime("%Y%m%d-%H%M%S")
    run_id = f"{stamp}-{slug}" if slug else stamp
    base_id, suffix = run_id, 1
    run_dir = root / "runs" / run_id
    while run_dir.exists():          # same-second runs must not overwrite each other
        suffix += 1
        run_id = f"{base_id}-{suffix}"
        run_dir = root / "runs" / run_id

    COST["run"] = run_id

    by_age, now = {}, datetime.now(timezone.utc)
    for n in nodes:
        age = (now - datetime.fromisoformat(n["updatedAt"].replace("Z", "+00:00"))).days
        bucket = "<1y" if age < 365 else "1-2y" if age < 730 else "2y+"
        by_age[bucket] = by_age.get(bucket, 0) + 1

    write_json(run_dir / "manifest.json", {
        "run": run_id, "tool_version": TOOL_VERSION,
        "repo": f"{owner}/{name}", "checkout": str(checkout),
        "master_ref": ref, "master_sha": sha, "mode": "remote" if remote else "local",
        "query": query_string, "cutoff": cutoff, "cap": cap, "continues": continues,
        "date_field": "created" if "created:" in query_string else "activity",
        "narrowed": narrowed, "truncated": truncated, "depth": depth,
        "matched": total, "retrieved": len(nodes), "selected": len(selected),
        "already_triaged_skipped": already, "not_examined": len(remaining) - len(selected),
        "created_at": datetime.now(timezone.utc).isoformat(),
    })
    write_json(run_dir / "selection.json", {"run": run_id, "prs": [
        {"number": n["number"], "title": n["title"], "url": n["url"],
         "updated_at": n["updatedAt"], "draft": n["isDraft"],
         "head_sha": n.get("headRefOid"),
         "author": (n.get("author") or {}).get("login"),
         "already_triaged": n["already_triaged"]}
        for n in selected]})

    print(json.dumps({
        "run": run_id, "run_dir": str(run_dir), "repo": f"{owner}/{name}",
        # What the arguments resolved to, in words, so nobody has to infer it from the query.
        "window": f"{state} pull requests "
        f"{'created' if 'created:' in query_string else 'last active'} "
        f"before {cutoff}",
        "cap": cap, "depth": depth,
        "matched": total, "retrieved": len(nodes), "selected": len(selected),
        "mode": "remote" if remote else "local", "mode_because": mode_why,
        "already_triaged_skipped": already,
        "skipped_by_verdict": skipped_by_verdict,
        "remaining_after_this_batch": len(remaining) - len(selected),
        "complete": len(remaining) == len(selected),
        "drafts": sum(1 for n in selected if n["isDraft"]),
        "by_age": by_age, "narrowed": narrowed, "continues": continues,
        "truncated": truncated,
        "warning": (f"more than {SEARCH_RETRIEVAL_LIMIT} match and search will not page further, "
                    "so this batch is drawn from a partial list. Narrow the window.")
        if truncated else None,
    }, indent=2))


PR_QUERY = """
query($owner: String!, $repo: String!, $number: Int!) {
  repository(owner: $owner, name: $repo) {
    pullRequest(number: $number) {
      number title url body isDraft state createdAt updatedAt
      author { login }
      baseRefName baseRefOid headRefOid
      mergeable
      reviewDecision
      changedFiles
      additions deletions
      labels(first: 20) { nodes { name } }
      files(first: 100) { totalCount nodes { path additions deletions } }
      latestReviews(first: 20) { nodes { state submittedAt commit { oid } author { login } } }
      commits(last: 1) { nodes { commit {
        oid committedDate
        statusCheckRollup { state }
      } } }
      comments(first: 30) { totalCount nodes { author { login } createdAt body } }
      reviewThreads(first: 20) { totalCount nodes {
        isResolved
        comments(first: 5) { nodes { author { login } createdAt body } }
      } }
      closingIssuesReferences(first: 10) { nodes { number title state body } }
    }
  }
}
"""


def truncate(text, limit=4000):
    if text and len(text) > limit:
        return text[:limit], f"kept {limit} of {len(text)} chars"
    return text, None


def list_runs(checkout, repo, limit):
    """What has been triaged, grouped by window.

    Batches are how the work gets done, not something to report: several batches of one window are
    one piece of work with one report. The batch ids stay, under each window, because
    `--continue-from`, `forget --run` and `cost --run` all take one.
    """
    checkout = Path(checkout).resolve()
    owner, name = split_repo(repo) if repo else infer_repo(checkout)
    root = cache_root(owner, name)
    reports = {}
    for path, q in sorted((read_json(root / "reports.json", {}) or {}).items()):
        reports[q] = path          # sorted, so the newest as-of date wins for a window with several

    windows = {}
    for d in sorted((root / "runs").glob("*")):
        m = read_json(d / "manifest.json")
        if not m:
            continue
        judged = read_json(d / "judgment.json", {}) or {}
        w = windows.setdefault(m.get("query", ""), {
            "window": None, "first_batch": m["created_at"][:16], "last_batch": None,
            "selected": 0, "judged": 0, "still_to_triage": None,
            "report": None, "rendered": False, "batches": []})
        field = "created" if date_field_of(m) == "created" else "last active"
        w["window"] = f"{field} before {m['cutoff']}" if m.get("cutoff") else m.get("query", "")
        w["last_batch"] = m["created_at"][:16]
        w["selected"] += m["selected"]
        w["judged"] += len(judged.get("verdicts", []))
        w["still_to_triage"] = m.get("not_examined")
        # The path always shows, even before anything has been rendered: a window whose batches are
        # not all judged yet still has a filename it will land under, and null tells nobody where.
        path_ = reports.get(m.get("query", ""))
        if path_:
            path_ = Path(path_) if ("/" in path_ or "\\" in path_) else root / path_
        else:
            path_ = reports_root(owner, name) / campaign_filename(m, m["created_at"][:10])
        w["report"] = str(path_)
        w["rendered"] = path_.exists()
        w["batches"].append({"run": m["run"], "selected": m["selected"],
                             "judged": len(judged.get("verdicts", []))})
    out = sorted(windows.values(), key=lambda w: w["first_batch"])[-limit:]
    print(json.dumps({"repo": f"{owner}/{name}", "windows": out}, indent=2))


# Bots that post mechanically on every pull request. Their output is long, uniform and says
# nothing about whether a goal has been met, so carrying it costs the judging agent tokens it can
# never spend. Measured on connectedhomeip: 96% of all comment text, and half of a whole dossier.
MECHANICAL_AUTHORS = {"github-actions", "CLAassistant", "codecov", "restyled-io", "mergify",
                      "codecov-commenter", "sonarcloud", "netlify",
                      # Automated reviewers. They read the diff and nothing else, so they cannot
                      # know whether the goal already landed on the base branch, which is the only
                      # question here. Human reviewers are kept: "superseded by #123" settles a case.
                      "gemini-code-assist", "copilot-pull-request-reviewer", "coderabbitai",
                      "sourcery-ai", "deepsource-autofix"}
HUB_FILE_THRESHOLD = 3    # a path shared by more dossiers than this is infrastructure, not a link
COMMENT_BUDGET = 600      # enough for "superseded by #123, closing" and any abandonment note


def is_mechanical(author):
    author = author or ""
    return author in MECHANICAL_AUTHORS or author.endswith("[bot]")


def trim_dossier(pr):
    """Drop what a verdict can never rest on. Idempotent, so re-running a phase re-applies it.

    Only comment text is touched. Everything a verdict does rest on, the diff, the signals, the
    linked issues and the pull request's own body, is left exactly as it was.
    """
    dropped = pr.get("comments_dropped") or 0
    comments = pr.get("comments")
    if isinstance(comments, list):
        kept = []
        for comment in comments:
            if is_mechanical(comment.get("by")):
                dropped += 1
                continue
            body = comment.get("body") or ""
            if len(body) > COMMENT_BUDGET:
                comment = {**comment, "body": body[:COMMENT_BUDGET] + " [...]"}
            kept.append(comment)
        pr["comments"] = kept
    if dropped:
        pr["comments_dropped"] = dropped

    threads = pr.get("review_threads")
    if isinstance(threads, list):
        kept_threads = []
        for thread in threads:
            comments = []
            for comment in thread.get("comments") or []:
                if is_mechanical(comment.get("by")):
                    dropped += 1
                    continue
                body = comment.get("body") or ""
                if len(body) > COMMENT_BUDGET:
                    comment = {**comment, "body": body[:COMMENT_BUDGET] + " [...]"}
                comments.append(comment)
            # A thread that was only ever a bot leaves nothing behind worth carrying.
            if comments:
                kept_threads.append({**thread, "comments": comments})
        pr["review_threads"] = kept_threads
    if dropped:
        pr["comments_dropped"] = dropped
    return pr


def collect(run_id, checkout, repo, refetch):
    run_id = checked_run_id(run_id)
    """Fetch raw facts per PR. Cached across runs, keyed by PR head and master SHA."""
    checkout = Path(checkout).resolve()
    owner, name = split_repo(repo) if repo else infer_repo(checkout)
    root = cache_root(owner, name)
    run_dir = root / "runs" / run_id
    manifest = read_json(run_dir / "manifest.json")
    selection = read_json(run_dir / "selection.json")
    if not manifest or not selection:
        raise RuntimeError(f"run {run_id} not found under {run_dir}. List runs with: matter_pr_triage.py runs")

    # Selection recorded each head at search time; a PR that moved since must be refetched.
    heads = {p["number"]: p.get("head_sha") for p in selection["prs"]}
    done, skipped, failed = 0, 0, 0
    kept = []
    for entry in selection["prs"]:
        number = entry["number"]
        cache_file = root / "pr" / f"{number}.json"
        cached = read_json(cache_file)
        head_now = heads.get(number)
        if (cached and not refetch
                and cached.get("master_sha") == manifest["master_sha"]
                and (head_now is None or cached.get("head_sha") == head_now)):
            kept.append(number)
            skipped += 1
            continue
        try:
            pr = gh_graphql(PR_QUERY, {"owner": owner, "repo": name, "number": number})
            pr = pr["repository"]["pullRequest"]
        except RuntimeError as e:
            write_json(cache_file, {"number": number, "errors": [{"code": "fetch_failed", "detail": str(e)[:300]}]})
            failed += 1
            continue

        paths = [f["path"] for f in pr["files"]["nodes"]]
        body, body_trunc = truncate(pr.get("body") or "")
        all_comments = pr["comments"]["nodes"]
        human = [c for c in all_comments if not is_mechanical((c.get("author") or {}).get("login"))]
        comments, comments_dropped = human[-20:], len(all_comments) - len(human)
        commit = (pr["commits"]["nodes"] or [{}])[0].get("commit", {})
        approvals = [r for r in pr["latestReviews"]["nodes"] if r["state"] == "APPROVED"]

        write_json(cache_file, trim_dossier({
            "number": number,
            "comments_dropped": comments_dropped,
            "url": pr["url"],
            "title": pr["title"],
            "body": body,
            "author": (pr.get("author") or {}).get("login"),
            "state": pr["state"],
            "draft": pr["isDraft"],
            "created_at": pr["createdAt"],
            "updated_at": pr["updatedAt"],
            "base_ref": pr["baseRefName"],
            "base_sha": pr["baseRefOid"],
            "head_sha": pr["headRefOid"],
            "master_sha": manifest["master_sha"],
            "mergeable": pr["mergeable"],
            "review_decision": pr["reviewDecision"],
            "approvals": [{"by": (a.get("author") or {}).get("login"),
                           "at": a["submittedAt"],
                           "commit": (a.get("commit") or {}).get("oid"),
                           "current_head": (a.get("commit") or {}).get("oid") == pr["headRefOid"]}
                          for a in approvals],
            "checks": (commit.get("statusCheckRollup") or {}).get("state"),
            "labels": [x["name"] for x in pr["labels"]["nodes"]],
            "diffstat": {"files": pr["changedFiles"], "additions": pr["additions"],
                         "deletions": pr["deletions"]},
            "changed_paths": paths,
            "linked_issues": [{"number": i["number"], "title": i["title"], "state": i["state"],
                               "body": truncate(i.get("body") or "", 1500)[0]}
                              for i in pr["closingIssuesReferences"]["nodes"]],
            "comments": [{"by": (c.get("author") or {}).get("login"), "at": c["createdAt"],
                          "body": truncate(c["body"], 1200)[0]} for c in comments],
            "review_threads": [{"resolved": t["isResolved"],
                                "comments": [{"by": (c.get("author") or {}).get("login"),
                                              "at": c["createdAt"],
                                              "body": truncate(c["body"], 800)[0]}
                                             for c in t["comments"]["nodes"]]}
                               for t in pr["reviewThreads"]["nodes"]],
            "truncation": {
                "body": body_trunc,
                "comments": (f"kept 20 of {pr['comments']['totalCount']}"
                             if pr["comments"]["totalCount"] > 20 else None),
                "files": (f"listed 100 of {pr['files']['totalCount']}"
                          if pr["files"]["totalCount"] > 100 else None),
            },
            "errors": [],
        }))
        kept.append(number)
        done += 1

    print(json.dumps({"run": run_id, "fetched": done, "cached_or_filtered": skipped,
                      "failed": failed, "in_batch": len(kept)}, indent=2))


# ---------------------------------------------------------------- signals

HUNK_RE = re.compile(r"^@@ -(\d+)(?:,(\d+))? \+(\d+)(?:,(\d+))? @@", re.M)


def classify_patch(patch):
    """Split a unified patch into hunks tagged add / modify / delete, with context."""
    hunks = []
    if not patch:
        return hunks
    positions = [m.start() for m in HUNK_RE.finditer(patch)]
    for i, start in enumerate(positions):
        end = positions[i + 1] if i + 1 < len(positions) else len(patch)
        body = patch[start:end].split("\n")[1:]
        removed = [ln[1:] for ln in body if ln.startswith("-")]
        added = [ln[1:] for ln in body if ln.startswith("+")]
        context = [ln[1:] for ln in body if ln.startswith(" ")]
        kind = "modify" if removed and added else "delete" if removed else "add"
        hunks.append({"kind": kind, "removed": removed, "added": added, "context": context})
    return hunks


class LocalSource:
    """Evidence from a checkout. Accurate, needs a clone with history for blame."""

    kind = "local"

    def __init__(self, checkout, rev):
        self.checkout, self.rev, self._tree = checkout, rev, None
        self.tree_partial = False

    def file_text(self, path):
        return run(["git", "-C", str(self.checkout), "show", f"{self.rev}:{path}"],
                   check=False) or None

    def blame_newest(self, path, start, end):
        out = run(["git", "-C", str(self.checkout), "blame", "-L", f"{start},{end}",
                   "-p", self.rev, "--", path], check=False)
        stamps = [int(s) for s in re.findall(r"^author-time (\d+)$", out, re.M)]
        return datetime.fromtimestamp(max(stamps), timezone.utc) if stamps else None

    def tree(self):
        if self._tree is None:
            # Blobs only. --name-only would also list submodule entries, which are not files and
            # which the remote source does not report, so the two modes would disagree.
            out = run(["git", "-C", str(self.checkout), "ls-tree", "-r", self.rev], check=False)
            self._tree = [line.split("\t", 1)[1] for line in out.split("\n")
                          if "\t" in line and line.split()[1] == "blob"]
        return self._tree

    def grep_literal(self, text, limit=3):
        out = run(["git", "-C", str(self.checkout), "grep", "-F", "-l", text, self.rev],
                  check=False)
        return [f.split(":", 1)[-1] for f in out.split("\n") if f][:limit]


BLAME_QUERY = """
query($owner: String!, $repo: String!, $rev: String!, $expr: String!, $path: String!) {
  repository(owner: $owner, name: $repo) {
    blob: object(expression: $expr) { ... on Blob { text isTruncated } }
    commit: object(expression: $rev) {
      ... on Commit {
        blame(path: $path) { ranges { startingLine endingLine commit { committedDate } } }
      }
    }
  }
}
"""


class RemoteSource:
    """Evidence from the API. No clone needed, but literal search is not available.

    GitHub code search tokenizes rather than matching substrings, only indexes the default branch
    and is heavily rate limited, so the added-line test cannot be done honestly here. It reports
    unsupported instead of guessing.
    """

    kind = "remote"

    def __init__(self, owner, name, rev):
        self.owner, self.name, self.rev, self._tree, self._blame = owner, name, rev, None, {}
        self.tree_partial = False

    def _fetch(self, path):
        if path not in self._blame:
            data = gh_graphql(BLAME_QUERY, {"owner": self.owner, "repo": self.name,
                                            "rev": self.rev, "expr": f"{self.rev}:{path}",
                                            "path": path})["repository"]
            blob = data.get("blob") or {}
            # A truncated blob would match content at the wrong offset and blame the wrong lines.
            # Unavailable is the honest answer; the caller reports unlocatable.
            text = None if blob.get("isTruncated") else blob.get("text")
            ranges = ((data.get("commit") or {}).get("blame") or {}).get("ranges") or []
            self._blame[path] = (text, ranges)
        return self._blame[path]

    def file_text(self, path):
        return self._fetch(path)[0]

    def blame_newest(self, path, start, end):
        _, ranges = self._fetch(path)
        covering = [r for r in ranges
                    if not (r["endingLine"] < start or r["startingLine"] > end)]
        if not covering:
            return None
        newest = max(r["commit"]["committedDate"] for r in covering)
        return datetime.fromisoformat(newest.replace("Z", "+00:00"))

    def tree(self):
        if self._tree is None:
            data = gh_rest(f"/repos/{self.owner}/{self.name}/git/trees/{self.rev}?recursive=1")
            self._tree = [e["path"] for e in data.get("tree", []) if e.get("type") == "blob"]
            # A very large repository can exceed what one tree call returns. Say so rather than
            # letting a short list read as "the file is not there".
            self.tree_partial = bool(data.get("truncated"))
        return self._tree

    def grep_literal(self, text, limit=3):
        return None     # unsupported, and saying so beats a weak answer


def path_exists(src, path):
    """True when the path exists at the revision.

    A complete listing settles it on its own. Only a truncated one needs a read, which costs a
    request per path and is why it is not the default route.
    """
    if path in src.tree():
        return True
    if not getattr(src, "tree_partial", False):
        return False
    return src.file_text(path) is not None


def locate_and_blame(src, path, hunk, since_iso):
    """Find the hunk's pre-image at the revision by content and blame it.

    Returns a dict with status untouched | touched | unlocatable.
    """
    anchor = [ln for ln in (hunk["removed"] or hunk["context"]) if ln.strip()]
    if not anchor:
        return {"status": "unlocatable", "why": "no anchor lines"}
    current = src.file_text(path)
    if not current:
        return {"status": "unlocatable", "why": "path not at this revision"}

    lines = current.split("\n")
    if lines and lines[-1] == "":
        lines.pop()          # trailing newline, not a line
    needle = anchor[0].strip()
    matches = [i for i, ln in enumerate(lines) if ln.strip() == needle]
    if len(anchor) > 1:
        second = anchor[1].strip()
        matches = [i for i in matches
                   if any(lines[j].strip() == second for j in range(i + 1, min(i + 6, len(lines))))]
    if len(matches) != 1:
        return {"status": "unlocatable", "why": f"{len(matches)} content matches"}

    start = matches[0] + 1
    end = min(start + max(len(anchor), 1) - 1, len(lines))
    newest = src.blame_newest(path, start, end)
    if newest is None:
        return {"status": "unlocatable", "why": "blame produced nothing"}
    # Parse both sides: "…Z" and "…+00:00" do not compare correctly as strings.
    touched = newest > datetime.fromisoformat(since_iso.replace("Z", "+00:00"))
    return {"status": "touched" if touched else "untouched",
            "range": f"{path}#L{start}-{end}", "newest_commit": newest.isoformat()}


def new_file_check(src, paths):
    """For files a PR creates, the decisive question is whether the revision has them.

    Checks the exact path first, then the basename anywhere, which catches a file that landed
    somewhere else. This is the cheapest coverage test there is, and for a new test script it is
    close to conclusive.
    """
    tree = src.tree()
    partial = getattr(src, "tree_partial", False)
    results = []
    for path in paths:
        base = path.rsplit("/", 1)[-1]
        # None, not [], when the listing is incomplete: absent and unknown are different answers.
        elsewhere = (None if partial
                     else [p for p in tree if p.rsplit("/", 1)[-1] == base and p != path][:3])
        results.append({"path": path, "on_master": path_exists(src, path),
                        "same_name_elsewhere": elsewhere})
    return results


MAX_TEXT_FETCHES = 25      # a sprawling pull request must not turn into a hundred API calls
RELOCATION_LOOKS = 6       # likely destinations to read when a line is missing from its own file


def relocation_candidates(path, tree):
    """Where a file's contents could have gone: the same name elsewhere, then its siblings.

    The tree is already in hand in both modes, so the places worth reading can be named without
    searching the repository. This is the same technique new_file_check uses for a moved file,
    applied to a moved hunk.
    """
    base = path.rsplit("/", 1)[-1]
    folder = path.rsplit("/", 1)[0] if "/" in path else ""
    prefix = folder + "/" if folder else ""

    def textual(p):
        return not p.lower().endswith((".png", ".jpg", ".jpeg", ".gif", ".ico", ".pdf", ".zip",
                                       ".gz", ".jar", ".bin", ".so", ".dll", ".woff", ".woff2",
                                       ".ttf", ".otf", ".mp4", ".webm", ".der", ".pem"))
    same_name = [p for p in tree if p != path and p.rsplit("/", 1)[-1] == base and textual(p)]
    siblings = [p for p in tree
                if p != path and p.startswith(prefix) and "/" not in p[len(prefix):]
                and textual(p)]
    ordered, seen = [], set()
    for p in same_name + siblings:
        if p not in seen:
            seen.add(p)
            ordered.append(p)
    return ordered[:RELOCATION_LOOKS]


def added_code_present(src, added_lines, paths=(), limit=3):
    """For a PR that adds code, blame says nothing. Ask whether the revision already has it.

    Samples the most distinctive added lines and looks for them verbatim in the pull request's own
    files at the base revision. Its own files are what the question means: a line found somewhere
    unrelated is the documented false positive, not evidence the goal landed, and reading those
    files needs only file contents, so this works with or without a clone.

    When a line is missing from its own file, the likely destinations named by the tree are read
    too, and a hit is recorded as `nearby`: a lead about where the change went, never coverage on
    its own. Returns None only when nothing could be read at all, which is not finding nothing.
    """
    candidates = [ln.strip() for ln in added_lines
                  if len(ln.strip()) > 25
                  and not ln.strip().startswith(("//", "#", "*", "/*", "--"))]
    candidates.sort(key=len, reverse=True)

    texts = {}
    for path in list(paths)[:MAX_TEXT_FETCHES]:
        text = src.file_text(path)
        if text:
            texts[path] = text

    # Only read further when a line is missing from its own file, which is when "where did it go"
    # is a question at all. The tree names the places worth reading, so no search is needed and
    # both modes answer identically, against the commit the run pinned.
    tree = None if getattr(src, "tree_partial", False) else src.tree()
    nearby, budget, candidates_for = {}, RELOCATION_LOOKS, {}

    def look_nearby(line):
        nonlocal budget
        if tree is None:
            return None
        for path in list(paths)[:MAX_TEXT_FETCHES]:
            if path not in candidates_for:          # one scan of the tree per file, not per line
                candidates_for[path] = relocation_candidates(path, tree)
            for cand in candidates_for[path]:
                if cand not in nearby:
                    if budget <= 0:
                        continue
                    budget -= 1
                    nearby[cand] = src.file_text(cand) or ""
                if line in nearby[cand]:
                    return cand
        return None

    results = []
    for line in candidates[:limit]:
        found_in = sorted(p for p, t in texts.items() if line in t)
        results.append({"line": line[:120], "found_in": found_in,
                        "nearby": None if found_in else look_nearby(line)})
    if not texts and not any(r["nearby"] for r in results):
        return None
    return results


def probe_terms(pr):
    """Identifiers likely to survive a refactor, strongest first."""
    terms = []
    text = f"{pr['title']} {' '.join(pr['changed_paths'])}"
    terms += re.findall(r"\bTC[-_][A-Z]+[-_]\d+[._]\d+\b", text)
    terms += re.findall(r"\b[A-Z][A-Za-z]{4,}(?:Cluster|Server|Manager|Delegate)\b", text)
    terms += re.findall(r"\b[A-Z]{3,}\.[SC]\.[A-Za-z0-9]+\b", text)
    seen, ordered = set(), []
    for t in terms:
        if t not in seen:
            seen.add(t)
            ordered.append(t)
    return ordered[:6]


def probe(src, terms):
    hits = []
    for term in terms:
        found = src.grep_literal(term, limit=5)
        if found is None:
            return None
        hits.append({"term": term, "files": found})
    return hits


ABANDON_PHRASES = (
    "feel free to close", "ok to close", "okay to close", "can be closed", "should be closed",
    "closing this", "i'll close", "will close this", "no longer needed", "no longer relevant",
    "abandon", "superseded by", "replaced by", "obsolete now", "not going to finish",
    "won't have time", "wont have time", "no longer working on",
)


def abandonment(pr):
    """A comment saying the work is finished with. Cheap, and often decisive.

    Returns the latest matching comment rather than all of them, since the last word on a pull
    request is the one that counts. The author saying it is near conclusive; anyone else saying it
    is a lead for the model to weigh.
    """
    everywhere = list(pr.get("comments", []))
    for thread in pr.get("review_threads", []):
        everywhere += thread.get("comments", [])
    everywhere.sort(key=lambda c: c.get("at") or "")
    hits = []
    for c in everywhere:
        body = (c.get("body") or "").lower()
        for phrase in ABANDON_PHRASES:
            if phrase in body:
                hits.append({"by": c.get("by"), "at": c.get("at"), "phrase": phrase,
                             "by_author": c.get("by") == pr.get("author"),
                             "text": (c.get("body") or "")[:200]})
                break
    return hits[-1] if hits else None


# Words that say nothing about what a pull request is for. Without these removed, "Update the
# docs" and "Update the tests" look half identical.
TITLE_NOISE = {"add", "adds", "update", "updates", "fix", "fixes", "the", "and", "for", "with",
               "from", "into", "when", "new", "use", "using", "support", "test", "tests"}


def title_tokens(title):
    """Comparable words from a title, with bracketed tags and filler words removed."""
    cleaned = re.sub(r"\[[^\]]*\]", " ", title or "").lower()
    return {w for w in re.split(r"[^a-z0-9]+", cleaned) if len(w) > 2} - TITLE_NOISE


def resolve_mergeability(owner, name, root, numbers, wait=5):
    """Fill in mergeable for the pull requests GitHub had not computed yet.

    The first collect request triggers the computation server side and returns UNKNOWN. Waiting
    briefly and asking again turns most of them into a real answer, which is what makes the
    conflicts band reachable at all.
    """
    pending = {}
    for n in numbers:
        pr = read_json(root / "pr" / f"{n}.json")
        if pr and pr.get("mergeable") == "UNKNOWN":
            pending[n] = pr
    if not pending:
        return 0
    time.sleep(wait)
    aliases = "\n".join(f'  p{n}: pullRequest(number: {n}) {{ mergeable }}' for n in pending)
    query = ("query($owner: String!, $repo: String!) {\n"
             "  repository(owner: $owner, name: $repo) {\n" + aliases + "\n  }\n}")
    try:
        data = gh_graphql(query, {"owner": owner, "repo": name})["repository"]
    except RuntimeError:
        return 0        # the caller reports the count, so a zero here is visible rather than silent
    resolved = 0
    for n, pr in pending.items():
        value = (data.get(f"p{n}") or {}).get("mergeable")
        if value and value != "UNKNOWN":
            pr["mergeable"] = value
            write_json(root / "pr" / f"{n}.json", trim_dossier(pr))
            resolved += 1
    return resolved


def relate_batch(root, numbers, write=True):
    """Cross-PR signals: two pull requests touching one file often mean one unaware duplicate.

    Deterministic and free, since every changed path is already collected. It seeds relatedness
    rather than deciding it: the model confirms whether the goals actually coincide.

    The batch is paired against every dossier in the cache, not only against itself. Batches are
    an artefact of how much a model can judge at once; a duplicate does not care which batch its
    twin landed in, and pairing within the batch alone left them blind to each other.
    """
    loaded = {}
    for n in numbers:
        pr = read_json(root / "pr" / f"{n}.json")
        if pr and not pr.get("errors"):
            loaded[n] = pr
    pool = dict(loaded)
    for f in (root / "pr").glob("*.json"):
        if f.stem.isdigit() and int(f.stem) not in pool:
            opr = read_json(f)
            if opr and not opr.get("errors"):
                pool[int(f.stem)] = opr
    # A file touched by many pull requests, PICS values, a workflow, a BUILD.gn, relates none of
    # them: sharing it says they live in the same repository, not that they chase the same goal.
    touched_by = {}
    for opr in pool.values():
        for path in set(opr.get("changed_paths") or []):
            touched_by[path] = touched_by.get(path, 0) + 1
    hubs = {path for path, count in touched_by.items() if count > HUB_FILE_THRESHOLD}
    found = {}
    for n, pr in loaded.items():
        mine, my_title = set(pr.get("changed_paths") or []) - hubs, title_tokens(pr.get("title"))
        candidates = []
        for other, opr in pool.items():
            if other == n:
                continue
            shared = mine & (set(opr.get("changed_paths") or []) - hubs)
            other_title = title_tokens(opr.get("title"))
            union = my_title | other_title
            similarity = round(len(my_title & other_title) / len(union), 2) if union else 0.0
            if shared or similarity >= 0.5:
                smaller = min(len(mine), len(set(opr.get("changed_paths") or []))) or 1
                candidates.append({"pr": other, "title": opr.get("title"),
                                   "shared_files": sorted(shared)[:5],
                                   "overlap": round(len(shared) / smaller, 2),
                                   "title_similarity": similarity})
        candidates.sort(key=lambda c: (-len(c["shared_files"]), -c["title_similarity"]))
        pr.setdefault("signals", {})["related_candidates"] = candidates[:5]
        found[n] = candidates[:5]
        if write:
            write_json(root / "pr" / f"{n}.json", trim_dossier(pr))
    return found


def supersede_candidates(owner, name, pr):
    """The replacement is usually active, so it is never in a stale batch. Query for it."""
    found = []
    for issue in pr.get("linked_issues", []):
        try:
            timeline = gh_rest(f"/repos/{owner}/{name}/issues/{issue['number']}/timeline?per_page=100")
        except RuntimeError:
            continue
        for item in timeline:
            src = (item.get("source") or {}).get("issue") or {}
            if src.get("number") and src["number"] != pr["number"] and src.get("pull_request"):
                found.append({"number": src["number"], "state": src.get("state"),
                              "title": src.get("title"), "via": f"issue#{issue['number']}"})
    return found[:10]


def signals(run_id, checkout, repo, remote):
    """Structural coverage and revival signals. No model."""
    run_id = checked_run_id(run_id)
    checkout = Path(checkout).resolve()
    remote, mode_why = resolve_mode(checkout, repo, remote)
    owner, name = split_repo(repo) if repo else infer_repo(checkout)
    root = cache_root(owner, name)
    run_dir = root / "runs" / run_id
    selection = read_json(run_dir / "selection.json")
    manifest = read_json(run_dir / "manifest.json")
    if not selection or not manifest:
        raise RuntimeError(f"run {run_id} not found. List runs with: matter_pr_triage.py runs")
    # Before anything reads mergeable: the band depends on it, so resolving afterwards would
    # leave the first pass deciding from a value GitHub had not computed yet.
    resolved = resolve_mergeability(owner, name, root, [e["number"] for e in selection["prs"]])

    rev = manifest["master_sha"]   # judge against the base branch, never the checked-out one
    default_ref = (manifest.get("master_ref") or "master").rsplit("/", 1)[-1]
    src = RemoteSource(owner, name, rev) if remote else LocalSource(checkout, rev)

    processed, errors = 0, 0
    for entry in selection["prs"]:
        number = entry["number"]
        cache_file = root / "pr" / f"{number}.json"
        pr = read_json(cache_file)
        if not pr or pr.get("errors"):
            errors += 1
            continue

        try:
            files = gh_rest(f"/repos/{owner}/{name}/pulls/{number}/files?per_page=100")
        except RuntimeError as e:
            pr["errors"] = [{"code": "fetch_failed", "detail": str(e)[:200]}]
            write_json(cache_file, trim_dossier(pr))
            errors += 1
            continue

        FILE_LIMIT = 40
        examined = files[:FILE_LIMIT]
        per_file, kinds, added_lines, new_files = [], set(), [], []
        for f in examined:
            hunks = classify_patch(f.get("patch"))
            for h in hunks:
                if h["kind"] == "add":
                    added_lines += h["added"]
            file_kinds = {h["kind"] for h in hunks} or {"unknown"}
            kinds |= file_kinds
            record = {"path": f["filename"], "status": f["status"],
                      "kinds": sorted(file_kinds), "blame": None}
            if f["status"] == "added":
                new_files.append(f["filename"])
            mods = [h for h in hunks if h["kind"] in ("modify", "delete")]
            if mods:
                record["blame"] = locate_and_blame(src, f["filename"], mods[0],
                                                   pr["created_at"])
            per_file.append(record)

        missing_paths = []
        for f in examined:
            if f["status"] == "added":
                continue   # the PR creates it, so master not having it is expected
            if not path_exists(src, f["filename"]):
                missing_paths.append(f["filename"])

        new_check = new_file_check(src, new_files) if new_files else []
        added_check = (added_code_present(src, added_lines, [f["filename"] for f in examined])
                       if added_lines else [])
        added_unchecked = added_check is None      # nothing could be read or searched at all
        added_check = added_check or []
        added_all_present = bool(added_check) and all(a["found_in"] for a in added_check)
        added_none_present = bool(added_check) and not any(a["found_in"] for a in added_check)

        blames = [f["blame"]["status"] for f in per_file if f.get("blame")]
        if new_check and all(n["on_master"] for n in new_check):
            coverage_hint = "new_files_already_on_master"
        elif (new_check and all(n["same_name_elsewhere"] is not None for n in new_check)
              and not any(n["on_master"] or n["same_name_elsewhere"] for n in new_check)):
            coverage_hint = "new_files_absent"
        elif added_all_present and not blames:
            coverage_hint = "added_code_already_on_master"
        elif not blames and added_none_present:
            coverage_hint = "added_code_absent"
        elif not blames and added_unchecked:
            coverage_hint = "added_code_unchecked"
        elif not blames:
            coverage_hint = "no_blameable_lines"
        elif all(b == "untouched" for b in blames):
            coverage_hint = "targets_untouched"
        elif any(b == "touched" for b in blames):
            coverage_hint = "targets_touched"
        else:
            coverage_hint = "unlocatable"

        author_idle_days = (datetime.now(timezone.utc) -
                            datetime.fromisoformat(pr["updated_at"].replace("Z", "+00:00"))).days
        # Most specific obstacle wins. Moved code outranks a quiet author, because no amount of
        # ownership fixes a target that is gone.
        if missing_paths:
            band = "moved"
        elif author_idle_days > 365:
            band = "unowned"
        elif pr.get("mergeable") == "CONFLICTING":
            band = "conflicts"
        else:
            band = "cheap"

        terms = probe_terms(pr)
        pr["signals"] = {
            "change_kinds": sorted(kinds),
            "files": per_file,
            "coverage_hint": coverage_hint,
            "probes": (probe(src, terms) or []) if terms else [],
            "added_code_check": added_check,
            "new_file_check": new_check,
            "linked_issue_states": [{"number": i["number"], "state": i["state"]}
                                    for i in pr.get("linked_issues", [])],
            "supersede_candidates": supersede_candidates(owner, name, pr),
            "quick_wins": {
                "linked_issue_closed": [i["number"] for i in pr.get("linked_issues", [])
                                        if i.get("state") == "CLOSED"],
                "base_ref": (pr.get("base_ref")
                             if pr.get("base_ref") != default_ref else None),
                "abandonment": abandonment(pr),
            },
            "revival": {"mergeable": pr.get("mergeable"), "missing_paths": missing_paths,
                        "author_idle_days": author_idle_days, "band": band},
            "computed_against": {"master_sha": pr["master_sha"], "head_sha": pr["head_sha"],
                                 "evidence_source": src.kind},
            "truncation": (f"examined {FILE_LIMIT} of {len(files)} files"
                           if len(files) > FILE_LIMIT else None),
        }
        write_json(cache_file, trim_dossier(pr))
        processed += 1

    relate_batch(root, [e["number"] for e in selection["prs"]])
    print(json.dumps({"run": run_id, "mode": "remote" if remote else "local",
                      "mode_because": mode_why, "processed": processed, "errors": errors,
                      "mergeability_resolved": resolved}, indent=2))


# ---------------------------------------------------------------- report

def derive(coverage, band):
    """The verdict follows from coverage and the obstacle, so wording cannot decide it."""
    if coverage == "covered_master":
        return "close"
    if coverage == "covered_in_flight":
        return "in-flight"
    if coverage == "unknown":
        return "unclear"
    return {"moved": "rewrite", "unowned": "unowned"}.get(band, "keep")


COVERAGES = ("covered_master", "covered_in_flight", "not_covered", "unknown")
VERDICTS = ("close", "in-flight", "unclear", "rewrite", "unowned", "keep")
BANDS = ("cheap", "conflicts", "moved", "unowned")
EFFORT_LEVELS = ("low", "medium", "high", "xhigh", "max")
TOOL_TALK = re.compile(r"\b(blame|signals?|probes?|dossiers?|manifest|the tool|coverage hint)\b", re.I)


def lint_judgment(judgment):
    """Soft findings: evidence that talks about the tool instead of the pull request."""
    return [f"#{v.get('pr')}: {TOOL_TALK.search(v.get('evidence') or '').group(0)!r}"
            for v in judgment.get("verdicts", []) if TOOL_TALK.search(v.get("evidence") or "")]


def validate_judgment(judgment, selection):
    problems = []
    wanted = {p["number"] for p in selection["prs"]}
    seen = set()
    for v in judgment.get("verdicts", []):
        number = int(str(v["pr"]).lstrip("#"))
        seen.add(number)
        if number not in wanted:
            problems.append(f"#{number}: has a verdict but was not in this run's selection")
        # A typo in any of these would otherwise derive quietly into keep and render as such.
        for field, allowed in (("coverage", COVERAGES), ("verdict", VERDICTS), ("revival", BANDS)):
            if v.get(field) not in allowed:
                problems.append(f"#{number}: {field} {v.get(field)!r} is not one of {list(allowed)}")
        for r in (v.get("related") or []):
            # The renderer reads r["pr"] and r["kind"], so a bare number or a stray kind would
            # pass here and crash there; refuse the shape up front, where the message can help.
            if not isinstance(r, dict) or as_number(r.get("pr")) is None:
                problems.append(f"#{number}: related entry {r!r} must be an object naming a pull request")
            elif r.get("kind") not in ("duplicate", "related"):
                problems.append(f"#{number}: related entry for #{r.get('pr')} has kind {r.get('kind')!r}, not duplicate or related")
        for x in (v.get("unrelated") or []):
            if as_number(x) is None:
                problems.append(f"#{number}: unrelated entry {x!r} does not name a pull request")
        derived = derive(v.get("coverage"), v.get("revival"))
        if derived and v.get("verdict") != derived and not v.get("disagreement_reason"):
            problems.append(f"#{number}: verdict {v.get('verdict')} differs from derived "
                            f"{derived} with no disagreement_reason")
        if not (v.get("evidence") or "").strip():
            problems.append(f"#{number}: no reason given. Every entry carries one.")
        if v.get("confidence") in ("medium", "low") and not v.get("falsifier"):
            problems.append(f"#{number}: confidence {v['confidence']} without a falsifier")
    unaccounted = wanted - seen - {int(str(x).lstrip('#')) for x in judgment.get("unassessed", [])}
    for number in sorted(unaccounted):
        problems.append(f"#{number}: selected but has no verdict and is not listed unassessed")
    return problems


# Stated once, before anything that reads like a conclusion.
def date_field_of(manifest):
    """Which date selected a run. Manifests written before --date-field existed lack the key."""
    return manifest.get("date_field") or ("created" if "created:" in manifest.get("query", "")
                                          else "activity")


DISCLAIMER = [
    "> ## Disclaimer",
    ">",
    "> This is a best-effort report, meant to inform decision making regarding a backlog of old pull requests, not to make those decisions. Every verdict is machine-assisted and provisional. Review each pull request before acting on it.",
    "",
]


def render_body(verdicts, prs, repo, unassessed=()):
    """The sections themselves. Shared so a per-run report and a combined one cannot drift."""
    def pr_url(number):
        """A pull request outside the batch has no dossier, but its URL is still knowable."""
        cached = prs.get(number, {}).get("url")
        return cached or f"https://github.com/{repo}/pull/{number}"

    def linkify(text):
        """Any #1234 written in prose becomes a link, not just the ones the renderer places."""
        # Any number of digits: a small repository has pull requests below #100, and the lookbehind
        # already keeps C#5-style text and already-linked [#N] out of it.
        return re.sub(r"(?<![\[\w/#])#(\d+)\b",
                      lambda m: f"[#{m.group(1)}]({pr_url(int(m.group(1)))})", text or "")

    def entry(number, v, show_related=False):
        pr = prs.get(number, {})
        # Titles routinely contain [brackets], which would break the link text.
        # Trailing spaces are common in real titles and would sit inside the link text.
        title = pr.get("title", "").strip().replace("[", "\\[").replace("]", "\\]")
        # A dossier can be absent (cache pruned, run carried over), and the docs promise every
        # number is a link, so fall back to the URL the number itself implies.
        url = pr.get("url") or pr_url(number)
        link = f"[#{number} {title}]({url})" if title else f"[#{number}]({url})"
        head = f"**{link}**"
        out = [head]
        created, active = (pr.get("created_at") or "")[:10], (pr.get("updated_at") or "")[:10]
        dates = (f"`Created {created}, last active {active}`" if created and active
                 else f"`Created {created}`" if created else "")
        # Related only helps where the pull request stays open. On a close it is noise:
        # the decision is already made.
        everything = v.get("related") or []
        duplicates = [r for r in everything if r.get("kind") == "duplicate"]
        related = [r for r in everything if r.get("kind") != "duplicate"] if show_related else []
        reason = v.get("evidence", "").strip()
        for prefix in ("Already handled:", "Not handled:", "Not established:"):
            if reason.lower().startswith(prefix.lower()):
                reason = reason[len(prefix):].strip()
                break
        if reason:
            sentence = reason[0].upper() + reason[1:] + ("" if reason.endswith(".") else ".")
            out.append(linkify(sentence))
        # The citation stays in judgment.json for traceability but is not printed: the reason
        # already names the location, so printing it again is a third restatement.
        if dates:
            out.append(dates)
        for r in duplicates:
            number_d = int(str(r["pr"]).lstrip("#"))
            why_d = linkify((r.get("why") or "").strip())
            out.append(f"**Duplicate of [#{number_d}]({pr_url(number_d)})**"
                       + (f", {why_d}" if why_d else ""))
        if related:
            parts = []
            for r in related:
                number_r = int(str(r["pr"]).lstrip("#"))
                link_r = f"[#{number_r}]({pr_url(number_r)})"
                why = linkify((r.get("why") or "").strip())
                parts.append(f"{link_r} ({why})" if why else link_r)
            out.append("***Related:*** " + ", ".join(parts))
        # disagreement_reason stays in judgment.json for traceability and is not printed: the
        # reader needs the conclusion, which the reason line carries, not the bookkeeping behind it.
        return "  \n".join(x for x in out if x) + "\n"

    # Two sections only: what needs doing, and what does not. Each opens with the one thing
    # worth saying across its members, which is where a shared cause belongs.
    SECTION_TEXT = {
        "action": "These need a decision from you. The goal has already been met, the work has moved on, or nobody is driving it any more.",
        "no_action": "These stay open. Either the goal is confirmed unmet and the pull request still stands, or the check was inconclusive and the pull request needs to be manually reviewed.",
    }
    # Headings stand alone. A subtitle under them only restates the heading or the entry.
    ACTIONS = (
        ("close", "CLOSE", "The goal is already met on master, so there is nothing left for the pull request to do. Confirm each one, then close it."),
        ("rewrite", "STALE FILE REFERENCES", "The files it targets are gone from master, so this branch cannot be rebased. Closing it is the likely outcome; the open question is whether the work gets redone elsewhere."),
        ("in-flight", "SUPERSEDED", "A live pull request covers the same goal. Decide which of "
                                    "the two survives."),
        ("unowned", "NEEDS AN OWNER", "It still applies cleanly, but nobody has moved it in over "
                                      "a year. Find a driver or close it."),
    )
    PARKED = (
        ("keep", "STILL NEEDED", "Unmet, and the code it targets is intact."),
        ("unclear", "COULD NOT DETERMINE", "Nothing gathered settles whether the goal is already met. Each entry names the check that would."),
    )

    def section(heading, kinds, blurb_key, show_related=False):
        members = {k: sorted(n for n, v in verdicts.items() if v["verdict"] == k)
                   for k, _, _ in kinds}
        total = sum(len(v) for v in members.values())
        if not total:
            return []
        out = [f"## {heading} ({total})", "", SECTION_TEXT[blurb_key], ""]
        for kind, label, blurb in kinds:
            if not members[kind]:
                continue
            out += [f"### {label} ({len(members[kind])})", blurb, ""]
            out += [entry(n, verdicts[n], show_related=show_related) for n in members[kind]]
        return out

    lines = section("DECIDE", ACTIONS, "action")
    lines += section("LEAVE OPEN", PARKED, "no_action", show_related=True)

    if unassessed:
        lines += [f"## Could not assess ({len(unassessed)})", ""]
        for u in unassessed:
            if isinstance(u, dict):
                lines.append(f"#{u['pr']}  {u.get('reason', '')}")
            else:
                lines.append(f"#{u}")
        lines += [""]
    return lines


def as_number(value):
    """A pull request reference as written by a person: 40007, "40007" or "#40007". None otherwise."""
    try:
        return int(str(value).strip().lstrip("#"))
    except (TypeError, ValueError):
        return None


def related_pairs_unreviewed(root, verdicts, candidates=None):
    """Pairs of judged pull requests that the signals relate but no judgment mentions.

    Each batch is judged on its own, so a pair split across two batches is only ever seen by the
    later one, if at all. Recomputed over everything judged, this is the list the final pass has to
    look at: the signals say the two touch the same ground, and nobody has yet said whether that
    makes them duplicates, siblings, or nothing.
    """
    def numbers_in(v):
        out = []
        for r in (v.get("related") or []):
            out.append(as_number(r.get("pr") if isinstance(r, dict) else r))
        out += [as_number(x) for x in (v.get("unrelated") or [])]
        return {x for x in out if x is not None}

    def mentions(v, other):
        return other in numbers_in(v)
    pairs, seen = [], set()
    for n, v in verdicts.items():
        if candidates is not None:
            cands = candidates.get(n, [])
        else:
            cands = ((read_json(root / "pr" / f"{n}.json") or {}).get("signals") or {}).get("related_candidates") or []
        for c in cands:
            other = c.get("pr")
            if other not in verdicts or other == n:
                continue
            key = tuple(sorted((n, other)))
            if key in seen or mentions(v, other) or mentions(verdicts[other], n):
                continue
            seen.add(key)
            pairs.append({"prs": list(key), "shared_files": c.get("shared_files", []),
                          "overlap": c.get("overlap"), "title_similarity": c.get("title_similarity"),
                          "verdicts": {str(key[0]): verdicts[key[0]]["verdict"],
                                       str(key[1]): verdicts[key[1]]["verdict"]}})
    return pairs


def campaign_filename(manifest, as_of=None):
    """One report per window per day, named by what selected it and when it was judged.

    The window is what stays the same across the batches of one triage and differs between two
    triages worth keeping apart. The date keeps a report someone acted on from being rewritten by
    a repeat weeks later: batches on the same day share a file, a later day gets its own.
    """
    cutoff = manifest.get("cutoff")
    if cutoff:
        field = "created" if date_field_of(manifest) == "created" else "active"
        stem = f"pr-triage-{field}-before-{cutoff}"
    else:
        slug = re.sub(r"[^a-z0-9]+", "-", (manifest.get("query") or "").lower()).strip("-")[:60]
        stem = f"pr-triage-{slug or 'all'}"
    return f"{stem}-as-of-{as_of}.md" if as_of else f"{stem}.md"


def render_campaign(root, full, group, filename, with_cost=False, out_dir=None):
    """Render one window's report over every judged run that belongs to it.

    A pull request can be judged more than once (a later run revisits it), so the newest verdict
    wins. Concatenating runs would double-count and would keep a superseded verdict alive.
    """
    verdicts, unassessed, superseded = {}, [], 0
    for _, run_id, _manifest, judgment in group:
        for v in judgment["verdicts"]:
            number = int(str(v["pr"]).lstrip("#"))
            if number in verdicts:
                superseded += 1
            verdicts[number] = v
        unassessed += judgment.get("unassessed", [])
    # An entry judged in a later run is no longer unassessed, and one carried by two runs is one.
    seen_unassessed, deduped = set(), []
    for u in unassessed:
        number = int(str(u["pr"] if isinstance(u, dict) else u).lstrip("#"))
        if number in verdicts or number in seen_unassessed:
            continue
        seen_unassessed.add(number)
        deduped.append(u)

    # Every judged pull request against every cached dossier, so relationships that straddle two
    # batches exist in the signals before the report is rendered.
    candidates = relate_batch(root, sorted(verdicts), write=False)
    prs = {n: read_json(root / "pr" / f"{n}.json") or {} for n in verdicts}
    pairs = related_pairs_unreviewed(root, verdicts, candidates)

    newest = group[-1][2]
    field = date_field_of(newest)
    key = "created_at" if field == "created" else "updated_at"
    verb = {"created": "created", "activity": "last active"}.get(field)
    cutoff = newest.get("cutoff")
    dates = sorted(pr[key][:10] for pr in prs.values() if pr.get(key))

    # The title identifies the document the way the filename does, so a forwarded or printed copy
    # still says which window it covers. A one-sentence lead says what the reader is holding; the
    # table carries the facts that let a verdict be re-checked later: base commit, date, judge.
    window = f"{verb} before {cutoff}" if (cutoff and verb) else (f"before {cutoff}" if cutoff else "")
    lines = [f"# PR triage - {full}" + (f" - {window}" if window else ""), "",
             "*Old pull requests read against the base branch: what is already done, what has moved on, and what still stands.*", ""]
    judged_on = (group[-1][2].get("created_at") or "")[:10]
    lines += ["| | |", "|---|---|"]
    if window:
        lines.append(f"| **Window** | open pull requests {window} |")
    result = f"{len(verdicts)} open pull requests"
    if dates:
        by = "" if field == "created" else " by last activity"
        result += f" spanning from {dates[0]} to {dates[-1]}{by}"
    lines.append(f"| **Result** | {result} |")
    if judged_on:
        lines.append(f"| **Run date** | {judged_on} |")
    if newest.get("master_sha"):
        lines.append(f"| **Judged against** | `{newest.get('master_ref') or 'master'}` at "
                     f"`{newest['master_sha'][:9]}` |")
    judged = sorted({(j.get("judged_by"), (j.get("judged_effort") or "").strip().lower())
                     for _, _, _, j in group if j.get("judged_by")})
    if judged:
        # The model is an identifier; the effort is always stated, as a named level when the judge
        # knew it and as "unknown" otherwise, so its absence is never mistaken for a default.
        def styled(model, effort):
            return f"`{model}`, {effort if effort in EFFORT_LEVELS else 'unknown'} effort"
        lines.append(f"| **Judged by** | {'; '.join(styled(m, e) for m, e in judged)} |")
    # Batches run to completion, so a finished window is the normal case and needs no row. These
    # two appear only when they change what the numbers above mean.
    left = newest.get("not_examined")
    if left:
        lines.append(
            f"| **Still to triage** | {left} pull requests matching the window have not been looked at; the run stopped part-way |")
    if any(m.get("truncated") for _, _, m, _ in group):
        lines.append("| **Search limit** | the search hit its retrieval limit, so the window may hold more than were counted |")
    lines.append("")
    lines += DISCLAIMER
    lines += render_body(verdicts, prs, full, deduped)
    if with_cost:
        lines += cost_footer(root)

    text = "\n".join(lines) + "\n"
    out_dir = out_dir or reports_root(*full.split("/", 1))
    out_dir.mkdir(parents=True, exist_ok=True)
    path = out_dir / filename
    path.write_text(text, encoding="utf-8")
    return {"path": path, "text": text, "verdicts": len(verdicts), "superseded": superseded,
            "pairs": pairs, "runs": [r[1] for r in group]}


def judged_runs(root):
    runs = []
    for d in sorted((root / "runs").glob("*")):
        manifest, judgment = read_json(d / "manifest.json"), read_json(d / "judgment.json")
        if manifest and judgment and judgment.get("verdicts"):
            runs.append((manifest.get("created_at", ""), d.name, manifest, judgment))
    runs.sort()
    return runs


def render_all_campaigns(root, full, label=None, with_cost=False, out_dir=None):
    """One report file per window per day, under the reports folder. Runs sharing a query, including
    its continuations and revisits, are one window; the file carries the date of the window's newest
    batch, so a repeat on a later day gets its own file and the earlier one stays as the record of
    what was acted on. Files this tool wrote for a window that has lost every run are removed; a
    file named with --name is left alone, since only its owner knows its purpose."""
    runs = judged_runs(root)
    if not runs:
        raise RuntimeError("no judged runs yet. Triage a batch first, then render.")
    out_dir = out_dir or reports_root(*full.split("/", 1))
    campaigns = {}
    for r in runs:
        campaigns.setdefault(r[2].get("query", ""), []).append(r)
    newest_query = runs[-1][2].get("query", "")
    # Two windows can share a cutoff and date field yet differ in some other filter; the second
    # then gets a short hash of its query appended rather than overwriting the first.
    names, taken = {}, set()
    for query, group in campaigns.items():
        as_of = (group[-1][2].get("created_at") or "")[:10] or None
        if label and query == newest_query:
            name = f"pr-triage-{label}-as-of-{as_of}.md" if as_of else f"pr-triage-{label}.md"
        else:
            name = campaign_filename(group[-1][2], as_of)
        if name in taken:
            name = name[:-3] + "-" + hashlib.sha1(query.encode()).hexdigest()[:6] + ".md"
        names[query] = name
        taken.add(name)
    rendered = []
    for query, group in campaigns.items():
        result = render_campaign(root, full, group, names[query], with_cost, out_dir)
        result["query"], result["newest"] = query, (query == newest_query)
        rendered.append(result)
    # The index remembers every file this tool wrote, as a full path, with the window it belongs
    # to. A file whose window has no run left is removed; earlier days of a living window stay; a
    # user's own file is never in the index, so it is never mistaken for a stale one. Entries from
    # before the reports folder existed are bare names inside the cache and are cleaned up once.
    index_file = root / "reports.json"
    previous = read_json(index_file, {}) or {}
    index = {}
    for key, query in previous.items():
        path = Path(key) if "/" in key or "\\" in key else root / key
        if path.parent == root or query not in campaigns:
            if path.exists():
                path.unlink()
            continue
        index[str(path)] = query
    legacy = root / "report.md"
    if legacy.exists():
        legacy.unlink()
    for r in rendered:
        index[str(r["path"])] = r["query"]
    write_json(index_file, index)
    return rendered


def combined(checkout, repo, label=None, out=None, with_cost=False):
    """The report over every batch of the newest window, plus one file per other window.

    Batching is an execution detail, not a finding: the user gets one document per window.
    """
    checkout = Path(checkout).resolve()
    owner, name = split_repo(repo) if repo else infer_repo(checkout)
    root = cache_root(owner, name)
    full = f"{owner}/{name}"
    # The label becomes part of a filename, so it gets the same slug select gives a run name.
    label = re.sub(r"[^a-z0-9]+", "-", (label or "").lower()).strip("-")[:40] or None

    rendered = render_all_campaigns(root, full, label, with_cost)
    newest = next(r for r in rendered if r["newest"])
    saved = save_copy(out, newest["path"].name, newest["text"])
    out_json = {"repo": full, "rendered": saved or str(newest["path"]),
                "kept_at": str(newest["path"]), "verdicts": newest["verdicts"],
                "runs": newest["runs"], "superseded": newest["superseded"]}
    others = [str(r["path"]) for r in rendered if not r["newest"]]
    if others:
        out_json["other_windows"] = others
    if newest["pairs"]:
        # For the judging agent, not the reader: each pair needs a decision (duplicate, related,
        # or nothing) written into the judgments, after which it stops appearing here.
        out_json["related_pairs_unreviewed"] = newest["pairs"]
    print(json.dumps(out_json, indent=2))


def save_copy(out, default_name, text):
    """Write the report where the user asked for it, on top of the cache copy.

    The cache copy always stays, because `runs` and the combined report find reports by that path.
    `out` may be a directory or a file name.
    """
    if not out:
        return None
    target = Path(out).expanduser()
    if target.is_dir() or str(out).endswith(("/", os.sep)):
        target = target / default_name
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(text, encoding="utf-8")
    return str(target.resolve())


def checked_run_id(run_id):
    """A run id becomes a path, and `forget` deletes that path, so it may not escape the runs
    directory. `..` alone would resolve to the repository cache and take every dossier with it."""
    if not run_id or run_id in (".", "..") or "/" in run_id or "\\" in run_id:
        raise RuntimeError(f"{run_id!r} is not a run id. List them with: matter_pr_triage.py runs")
    return run_id


def measured_tokens(session, lo=None, hi=None):
    """Real token usage from a Claude Code session transcript, for the window [lo, hi].

    Claude Code writes one JSONL line per message under ~/.claude/projects/<slug>/<id>.jsonl, each
    carrying the usage the API reported. That is measured, unlike the payload estimate, and it is
    the number to quote when someone asks what the tool costs on a sponsored account.
    """
    path = Path(session).expanduser()
    if not path.exists():
        raise RuntimeError(f"no transcript at {path}. Claude Code keeps them under "
                           f"~/.claude/projects/<project>/<session-id>.jsonl")
    totals = {"input_tokens": 0, "cache_creation_input_tokens": 0,
              "cache_read_input_tokens": 0, "output_tokens": 0}
    turns = 0
    with path.open(encoding="utf-8", errors="replace") as fh:
        for line in fh:
            if '"usage"' not in line:
                continue
            try:
                entry = json.loads(line)
            except ValueError:
                continue
            # Transcripts end a stamp with Z, manifests with +00:00, and those sort against
            # each other wrongly. Comparing to second precision sidesteps the suffix entirely.
            stamp = (entry.get("timestamp") or "")[:19]
            if (lo and stamp < lo[:19]) or (hi and stamp > hi[:19]):
                continue
            usage = (entry.get("message") or {}).get("usage")
            if not usage:
                continue
            turns += 1
            # A turn may report per-iteration usage; the top level is zeroed when it does.
            for part in (usage.get("iterations") or [usage]):
                for key in totals:
                    totals[key] += part.get(key) or 0
    fresh = totals["input_tokens"] + totals["cache_creation_input_tokens"]
    return {"model_turns": turns, "fresh_input_tokens": fresh,
            "cache_read_tokens": totals["cache_read_input_tokens"],
            "output_tokens": totals["output_tokens"],
            "window": {"from": lo, "to": hi}}


def run_window(root, run_id):
    """When a run's work started and when its report first appeared, for slicing a transcript."""
    manifest = read_json(root / "runs" / checked_run_id(run_id) / "manifest.json")
    if not manifest:
        raise RuntimeError(f"run {run_id} not found. List runs with: matter_pr_triage.py runs")
    started = manifest.get("created_at")
    # mtime moves when a report is re-rendered, so it is an upper bound, not the judging moment.
    # A run that was judged but never rendered still has a judgment file to bound it.
    ended = None
    for name in ("report.md", "judgment.json"):
        f = root / "runs" / run_id / name
        if f.exists():
            ended = datetime.fromtimestamp(f.stat().st_mtime, timezone.utc).isoformat()
            break
    return started, ended


def cost_summary(root, run_id=None):
    """What a run cost: wall clock, API calls, and the size of the material judging has to read.

    Tokens are spent by the agent doing the judging, not by this script, so the only honest figure
    here is the payload handed over. It is a floor: investigating a pull request costs more.
    """
    rows = []
    log = root / "cost.jsonl"
    if log.exists():
        for line in log.read_text(encoding="utf-8").splitlines():
            try:
                row = json.loads(line)
            except ValueError:
                continue
            if run_id is None or row.get("run") == run_id:
                rows.append(row)

    phases, seconds, calls, waits = {}, 0.0, 0, 0
    for row in rows:
        phases[row["command"]] = round(phases.get(row["command"], 0) + row.get("seconds", 0), 1)
        seconds += row.get("seconds", 0)
        calls += row.get("graphql_calls", 0) + row.get("rest_calls", 0)
        waits += row.get("rate_limit_waits", 0)

    # The judging payload, measured rather than guessed.
    numbers = []
    if run_id:
        selection = read_json(root / "runs" / checked_run_id(run_id) / "selection.json")
        numbers = [e["number"] for e in selection["prs"]] if selection else []
    else:
        numbers = [int(f.stem) for f in (root / "pr").glob("*.json") if f.stem.isdigit()]
    chars = 0
    for number in numbers:
        dossier = read_json(root / "pr" / f"{number}.json")
        if dossier:
            chars += len(json.dumps({k: dossier.get(k) for k in JUDGING_KEYS}))
    per_pr = chars // max(len(numbers), 1)

    return {
        "run": run_id or "all runs",
        "wall_clock_seconds": round(seconds, 1),
        "by_phase_seconds": phases,
        "phases_recorded": sorted(phases),
        "gathering_timed": any(k in phases for k in ("select", "collect", "signals")),
        "api_calls": calls,
        "rate_limit_waits": waits,
        "pull_requests": len(numbers),
        "judging_payload_chars": chars,
        "judging_tokens_estimate": chars // 4,
        "judging_tokens_estimate_per_pr": per_pr // 4,
        "note": "Token figures estimate the material handed to the judging agent, at roughly four "
                "characters per token. They are a floor: reading master to settle a verdict costs "
                "more, and that part is not measurable from here.",
    }


def cost(checkout, repo, run_id=None, session=None, since=None, until=None):
    """Print what a run cost. With --session, measured token usage instead of an estimate."""
    checkout = Path(checkout).resolve()
    owner, name = split_repo(repo) if repo else infer_repo(checkout)
    root = cache_root(owner, name)
    if run_id and not (root / "runs" / checked_run_id(run_id)).exists():
        raise RuntimeError(f"run {run_id} not found. List runs with: matter_pr_triage.py runs")
    out = {"repo": f"{owner}/{name}", **cost_summary(root, run_id)}

    if session:
        lo, hi = since, until
        derived = False
        if run_id and not (since or until):
            lo, hi = run_window(root, run_id)
            derived = True
        measured = measured_tokens(session, lo, hi)
        if hi is None:
            measured["window"]["to"] = "end of transcript"
            measured["window"]["note"] = ("No end could be derived for this run, so everything "
                                          "after the start is counted. Pass --until to bound it.")
        if derived and hi is not None:
            measured["window"]["to_is_upper_bound"] = (
                "Taken from when the report file was last written. Re-rendering a report moves "
                "that, so anything done in between is counted too. Pass --until to pin it.")
        prs = out["pull_requests"] or 1
        measured["per_pull_request"] = {
            "fresh_input_tokens": measured["fresh_input_tokens"] // prs,
            "cache_read_tokens": measured["cache_read_tokens"] // prs,
            "output_tokens": measured["output_tokens"] // prs,
        }
        measured["note"] = (
            "Measured from the session transcript, so it counts everything the agent did in the "
            "window, including reading the base branch. Cache reads are the bulk of the volume and "
            "are billed at a fraction of fresh input; they scale with how long the conversation "
            "already is, not with the triage, so a run in its own session reads far less than one "
            "at the end of a long day. Quote fresh input and output for a figure that travels.")
        out["measured"] = measured
    print(json.dumps(out, indent=2))


def cost_footer(root, run_id=None):
    """One line under the report, for when the reader is the person who ran it.

    Off unless asked: a maintainer deciding on pull requests is not helped by knowing what the
    gathering cost, and it would be the last thing they read before acting.
    """
    c = cost_summary(root, run_id)
    seconds = c["wall_clock_seconds"]
    # Only claim a time when the phases that spend it were actually timed. A log that starts after
    # the gathering would otherwise report seconds for work that took minutes.
    if c["gathering_timed"]:
        spent = (f"{int(seconds // 60)}m {int(seconds % 60)}s" if seconds >= 60
                 else f"{seconds:.0f}s")
        calls = f" and {c['api_calls']} API calls" if c["api_calls"] else ""
        effort = f"{spent} of tool time{calls}"
    else:
        effort = "gathering not timed"
    return ["", "---", "",
            f"*Triage effort: {c['pull_requests']} pull requests, {effort}. "
            f"Judging read roughly {c['judging_tokens_estimate']:,} tokens of gathered material, "
            f"about {c['judging_tokens_estimate_per_pr']:,} per pull request. That is a floor: "
            f"reading the base branch to settle a verdict costs more and is not counted here.*", ""]


def forget(checkout, repo, run_id=None, everything=False, yes=False):
    """Discard runs so their pull requests can be triaged again.

    State is rebuilt from the runs that survive rather than patched, so a forgotten verdict falls
    back to the newest earlier one instead of vanishing.
    """
    checkout = Path(checkout).resolve()
    owner, name = split_repo(repo) if repo else infer_repo(checkout)
    root = cache_root(owner, name)
    runs_dir = root / "runs"

    if not run_id and not everything:
        raise RuntimeError("name a run with --run, or pass --all to discard every run. "
                           "List them with: matter_pr_triage.py runs")
    if everything:
        doomed = sorted(d for d in runs_dir.glob("*") if d.is_dir())
    else:
        target = runs_dir / checked_run_id(run_id)
        # Belt and braces for the one command that deletes: the id has passed the name check, and
        # the resolved path must still be a direct child of runs/ before anything is removed.
        if target.resolve().parent != runs_dir.resolve():
            raise RuntimeError(f"{run_id!r} does not name a run directory under {runs_dir}")
        if not target.exists():
            raise RuntimeError(f"run {run_id} not found. List runs with: matter_pr_triage.py runs")
        doomed = [target]

    def judged_in(dirs):
        return {int(str(v["pr"]).lstrip("#"))
                for d in dirs
                for v in (read_json(d / "judgment.json") or {}).get("verdicts", [])}

    # A pull request judged again in a surviving run keeps that verdict, so it is not freed.
    doomed_set = {d.name for d in doomed}
    surviving = [d for d in runs_dir.glob("*") if d.is_dir() and d.name not in doomed_set]
    discarded = judged_in(doomed)
    freed = sorted(discarded - judged_in(surviving))
    if not yes:
        print(json.dumps({
            "would_remove": [d.name for d in doomed],
            "verdicts_discarded": len(discarded),
            "pull_requests_freed": freed,
            "dossiers_kept": True,
            "next": "Re-run with --yes to do it. The cached facts stay, so re-selecting these is "
                    "cheap; only the judgments are lost.",
        }, indent=2))
        return

    if not doomed:
        print(json.dumps({"removed": [], "note": "no runs to discard; state left alone."},
                         indent=2))
        return

    for d in doomed:
        shutil.rmtree(d)

    # Rebuild state as a projection of the surviving runs: newest verdict per pull request.
    state, survivors = {}, []
    for d in sorted(runs_dir.glob("*")):
        judgment, manifest = read_json(d / "judgment.json"), read_json(d / "manifest.json")
        if not (judgment and manifest):
            continue
        survivors.append(d.name)
        for v in judgment.get("verdicts", []):
            number = int(str(v["pr"]).lstrip("#"))
            dossier = read_json(root / "pr" / f"{number}.json") or {}
            state[str(number)] = {"verdict": v["verdict"], "run": d.name,
                                  "at": manifest.get("created_at", "")[:10],
                                  "head_sha": dossier.get("head_sha")}
    with locked(root / "state.json"):
        write_json(root / "state.json", state)

    # Reports follow the runs: windows that lost every run lose their file, the rest re-render.
    reports = []
    if judged_runs(root):
        reports = [str(r["path"]) for r in render_all_campaigns(root, f"{owner}/{name}")]
    else:
        index_file = root / "reports.json"
        for stale in (read_json(index_file, {}) or {}):
            path = Path(stale) if ("/" in stale or "\\" in stale) else root / stale
            if path.exists():
                path.unlink()
        for f in (index_file, root / "report.md"):
            if f.exists():
                f.unlink()

    print(json.dumps({"removed": [d.name for d in doomed], "runs_remaining": survivors,
                      "pull_requests_freed": freed, "triaged_now": len(state), "reports": reports,
                      "next": "Those pull requests will be selected again on the next run."},
                     indent=2))


def report(run_id, checkout, repo, strict, out=None, with_cost=False):
    """Validate judgment.json and render the markdown report."""
    run_id = checked_run_id(run_id)
    checkout = Path(checkout).resolve()
    owner, name = split_repo(repo) if repo else infer_repo(checkout)
    root = cache_root(owner, name)
    run_dir = root / "runs" / run_id
    selection = read_json(run_dir / "selection.json")
    manifest = read_json(run_dir / "manifest.json")
    judgment = read_json(run_dir / "judgment.json")
    if not selection:
        raise RuntimeError(f"run {run_id} not found. List runs with: matter_pr_triage.py runs")

    prs = {}
    for entry in selection["prs"]:
        pr = read_json(root / "pr" / f"{entry['number']}.json")
        if pr:
            prs[entry["number"]] = pr

    lines = [f"# PR triage - {manifest['repo']} - run {run_id}", ""]

    # The span must use the same field that selected the batch, or it contradicts the cutoff.
    field = date_field_of(manifest)
    key = "created_at" if field == "created" else "updated_at"
    verb = "created" if field == "created" else "last active"
    dates = sorted(pr[key][:10] for pr in prs.values() if pr.get(key))
    lines += [f"{len(prs)} pull requests triaged.", ""]
    # One line for the rule that selected them, one for what actually turned up.
    lines.append(f"- Selected: {verb} before {manifest['cutoff']}")
    if dates:
        lines.append(f"- Spanning: {dates[0]} to {dates[-1]}")
    # A report gets forwarded on its own, so it has to say whether it is the whole window or one
    # slice of it. Without this a reader cannot tell 15 of 15 from 15 of 42.
    matched, left = manifest.get("matched"), manifest.get("not_examined")
    if matched is not None and left is not None:
        earlier = manifest.get("already_triaged_skipped") or 0
        parts = [f"{len(prs)} of the {matched} pull requests matching it are in this report"]
        if earlier:
            parts.append(f"{earlier} were triaged in earlier runs")
        parts.append(f"{left} have not been looked at" if left else "none are left to triage")
        tail = parts[0] if len(parts) == 1 else ", ".join(parts[:-1]) + ", and " + parts[-1]
        lines.append(f"- Window: {'incomplete' if left else 'complete'}. {tail}.")
    if manifest.get("continues"):
        lines.append(f"- Continues run {manifest['continues']}")
    if manifest.get("truncated"):
        lines.append("- The search hit its retrieval limit, so the match count above is a floor rather than the true size of this window.")
    if judgment and judgment.get("judged_by"):
        # Results vary with the model and effort, so a report should say which produced it.
        lines.append(f"- Judged by: {judgment['judged_by']}")
    lines.append("")
    lines += DISCLAIMER

    if not judgment:
        lines += ["## Signals only (no judgment yet)", ""]
        for number, pr in sorted(prs.items()):
            sig = pr.get("signals", {})
            lines.append(f"**#{number} {pr['title']}**  \n{pr['url']}  \n"
                         f"{sig.get('coverage_hint', 'no signals')}, "
                         f"{sig.get('revival', {}).get('band', '?')} to revive\n")
        text = "\n".join(lines) + "\n"
        (run_dir / "report.md").write_text(text, encoding="utf-8")
        saved = save_copy(out, f"pr-triage-{run_id}.md", text)
        print(json.dumps({"run": run_id, "rendered": saved or str(run_dir / "report.md"),
                          "kept_at": str(run_dir / "report.md"),
                          "mode": "signals_only"}, indent=2))
        return

    problems = validate_judgment(judgment, selection)
    if problems and strict:
        print(json.dumps({"error": "invalid_judgment", "problems": problems}, indent=2))
        sys.exit(1)

    verdicts = {int(str(v["pr"]).lstrip("#")): v for v in judgment.get("verdicts", [])}

    lines += render_body(verdicts, prs, manifest["repo"],
                         judgment.get("unassessed", []))
    if with_cost:
        lines += cost_footer(root, run_id)

    text = "\n".join(lines) + "\n"
    (run_dir / "report.md").write_text(text, encoding="utf-8")
    saved = save_copy(out, f"pr-triage-{run_id}.md", text)

    state_file = root / "state.json"
    with locked(state_file):
        update_state(state_file, verdicts, prs, manifest, run_id)

    mismatch = sorted(n for n, pr in prs.items()
                      if (pr.get("signals") or {}).get("computed_against", {}).get("master_sha")
                      not in (None, manifest.get("master_sha")))
    lint = lint_judgment(judgment)
    out = {"run": run_id, "rendered": saved or str(run_dir / "report.md"),
           "kept_at": str(run_dir / "report.md"),
           "verdicts": len(verdicts), "validation_problems": problems}
    if mismatch:
        # Another run re-computed these dossiers against a different base since this run's
        # signals were taken, so the judge may have read newer evidence than the manifest says.
        out["signals_base_mismatch"] = mismatch
    if lint:
        out["evidence_mentions_tooling"] = lint
    print(json.dumps(out, indent=2))


def update_state(state_file, verdicts, prs, manifest, run_id):
    """Fold a run's verdicts into state.json without rewinding anything a later run decided."""
    state = read_json(state_file, {}) or {}
    for number, v in verdicts.items():
        pr = prs.get(number, {})
        # Re-rendering an older run must not rewind a verdict a later run already revised, or a
        # revisit would keep re-selecting pull requests that have since been decided. Run ids are
        # timestamp-prefixed, so they order directly.
        recorded = state.get(str(number), {})
        this_at, prev_at = manifest.get("created_at") or "", recorded.get("run_at")
        # Prefer the recorded creation times; fall back to id order for entries written before
        # run_at existed. Ids are timestamp-prefixed, so that order is right except within one second.
        if (prev_at and this_at and prev_at > this_at) or \
           (not prev_at and (recorded.get("run") or "") > run_id):
            continue
        state[str(number)] = {"verdict": v["verdict"], "run": run_id, "run_at": this_at,
                              "at": datetime.now(timezone.utc).strftime("%Y-%m-%d"),
                              "head_sha": pr.get("head_sha")}
    write_json(state_file, state)


def build_parser():
    p = argparse.ArgumentParser(
        prog="matter_pr_triage",
        description="PR backlog triage. Deterministic evidence gathering; the judge step is a model.")
    p.add_argument("--version", action="version", version=f"matter_pr_triage {TOOL_VERSION}")
    sub = p.add_subparsers(dest="command", required=True)

    def common(sp):
        sp.add_argument("--checkout", default=".", help="Checkout of the target repo.")
        sp.add_argument("--repo", default=None, help="owner/repo. Defaults to the checkout upstream.")
        return sp

    pre = common(sub.add_parser("preflight", help="Check dependencies and access."))
    pre.add_argument("--remote", action="store_true",
                     help="Checking for a run without a checkout: skip the checkout checks.")
    pre.add_argument("--fix", action="store_true",
                     help="Apply the remedies marked autofix. Currently: unshallow the checkout, "
                          "which is a large download.")

    sel = common(sub.add_parser("select", help="Pick the batch."))
    sel.add_argument("--state", choices=["open", "closed", "merged", "all"], default="open")
    sel.add_argument("--older-than", "--inactive-for", dest="older_than", default="365d",
                     metavar="AGE", help="e.g. 365d, 18mo, 2y. Measured against --date-field.")
    sel.add_argument("--updated-before", dest="updated_before", default=None,
                     help="Absolute cutoff YYYY-MM-DD. Overrides --inactive-for.")
    sel.add_argument("--date-field", dest="date_field", choices=["activity", "created"],
                     default="activity")
    sel.add_argument("--label", dest="labels", action="append", default=[])
    sel.add_argument("--exclude-label", dest="exclude_labels", action="append", default=[])
    sel.add_argument("--author", dest="authors", action="append", default=[])
    sel.add_argument("--exclude-author", dest="exclude_authors", action="append", default=[])
    sel.add_argument("--draft", dest="drafts", choices=["include", "exclude", "only"],
                     default="include")
    sel.add_argument("--review", default=None, help="none|required|approved|changes_requested")
    sel.add_argument("--cap", type=int, default=None,
                     help=f"How many pull requests to judge in one batch. Default {DEFAULT_CAP}, "
                     "which is the largest size measured end to end.")
    sel.add_argument("--include-bots", dest="include_bots", action="store_true")
    sel.add_argument("--revisit", action="append", default=[],
                     choices=["close", "rewrite", "in-flight", "unowned", "keep", "unclear"],
                     help="Re-include pull requests recorded with this verdict. Repeatable. "
                          "'unclear' brings back the ones nothing could be decided about.")
    sel.add_argument("--include-triaged", dest="skip_triaged", action="store_false", default=True,
                     help="Re-select PRs already recorded in state.")
    sel.add_argument("--remote", action="store_true",
                     help="Work without a checkout. Needs --repo, and signals must also use "
                          "--remote.")
    # --label is already the GitHub label filter above, so the run name is --name.
    sel.add_argument("--name", dest="label", default=None, metavar="NAME",
                     help="Name this run. Appended to the run id, so the report path carries it.")
    sel.add_argument("--depth", choices=["quick", "standard"], default="standard",
                     help="quick runs no model and reports signals only.")
    sel.add_argument("--base-ref", dest="base_ref", default=None,
                     help="Branch to judge coverage against. Defaults to the repo's default branch.")
    sel.add_argument("--continue-from", dest="continue_from", default=None, metavar="RUN_ID",
                     help="Continue a previous run: reuse its query and filters. "
                          "Accepts a run id or 'latest'.")

    runs = common(sub.add_parser("runs", help="List recent runs and their ids."))
    runs.add_argument("--limit", type=int, default=10)

    col = common(sub.add_parser("collect", help="Fetch raw facts per PR."))
    col.add_argument("--run", dest="run_id", required=True)
    col.add_argument("--refetch", action="store_true", help="Discard cached PR files.")

    sig = common(sub.add_parser("signals", help="Structural coverage and revival signals."))
    sig.add_argument("--run", dest="run_id", required=True)
    sig.add_argument("--remote", action="store_true",
                     help="Read evidence from the API instead of a checkout. No clone needed; the "
                          "added-line test is unavailable.")

    rep = common(sub.add_parser("report", help="Render the report. With --run, that one batch; "
                                "without it, every batch in one document."))
    rep.add_argument("--run", dest="run_id", default=None,
                     help="Render one batch. Omit to combine every judged run into a single "
                          "report, newest verdict per pull request.")
    rep.add_argument("--name", dest="label", default=None,
                     help="Name the window report, so it lands as pr-triage-<name>-as-of-<date>.md.")
    rep.add_argument("--with-cost", dest="with_cost", action="store_true",
                     help="Add a footer saying what the triage cost in time, calls and tokens. "
                          "Off by default: it is about the tool, not the pull requests.")
    rep.add_argument("--out", default=None, metavar="PATH",
                     help="Also save the report here, a file or a directory. The copy under "
                          "~/.cache/matter-pr-triage/<owner>/<repo>/ is kept either way, because "
                          "that is where runs and the combined report look for it.")

    cst = common(sub.add_parser("cost", help="What a run cost: time, API calls, and the size of "
                                "the material judging has to read."))
    cst.add_argument("--run", dest="run_id", default=None,
                     help="One run. Omit for every run on this repository.")
    cst.add_argument("--session", default=None, metavar="PATH",
                     help="A Claude Code transcript (~/.claude/projects/<project>/<id>.jsonl) to "
                          "read real token usage from, rather than estimating it. With --run the "
                          "window is taken from that run.")
    cst.add_argument("--since", default=None, metavar="ISO8601",
                     help="Start of the transcript window, ISO 8601 to the second, overriding the run's own.")
    cst.add_argument("--until", default=None, metavar="ISO8601",
                     help="End of the transcript window, ISO 8601 to the second, overriding the run's own.")

    fgt = common(sub.add_parser("forget", help="Discard a run so its pull requests can be "
                                               "triaged again. Cached facts are kept."))
    fgt.add_argument("--run", dest="run_id", default=None)
    fgt.add_argument("--all", dest="everything", action="store_true",
                     help="Discard every run and start the window over.")
    fgt.add_argument("--yes", action="store_true",
                     help="Actually do it. Without this it only reports what would go.")
    rep.add_argument("--no-strict", dest="strict", action="store_false", default=True,
                     help="Render even when the judgment fails validation.")
    return p


def log_phase_cost(command, kwargs, seconds):
    """Record what the phase cost. Best effort: never let bookkeeping break a command."""
    try:
        checkout = Path(kwargs.get("checkout") or ".").resolve()
        repo = kwargs.get("repo")
        owner, name = split_repo(repo) if repo else infer_repo(checkout)
        log_cost(cache_root(owner, name), command,
                 kwargs.get("run_id") or COST.get("run"), seconds)
    except Exception:
        pass


def main():
    args = build_parser().parse_args()
    kwargs = {k: v for k, v in vars(args).items() if k != "command"}
    if args.command == "select" and kwargs.get("cap") is None and not kwargs.get("continue_from"):
        # 25, not a round 50: judging a batch of 24 was measured at up to 112 model turns and
        # 560k tokens for the most thorough tier, so doubling it risks one batch outgrowing the
        # judging agent's context. Raise it with --cap once `cost` says what your repo costs.
        kwargs["cap"] = DEFAULT_CAP
    command = args.command
    # `report` with no --run is the whole window rather than one batch: batching is how the work
    # gets done, not something a reader of the result should have to reassemble.
    if command == "report" and not kwargs.get("run_id"):
        command = "combined"
        kwargs = {k: kwargs[k] for k in ("checkout", "repo", "label", "out", "with_cost")
                  if k in kwargs}
    elif command == "report":
        # --name belongs to the combined report; the per-run one takes its name from the run.
        # Popping it for every command would strip select's own --name, which select requires.
        kwargs.pop("label", None)
    handler = {"preflight": preflight, "select": select, "runs": list_runs,
               "collect": collect, "signals": signals, "report": report,
               "combined": combined, "cost": cost, "forget": forget}[command]
    started = time.time()
    try:
        handler(**kwargs)
    except RuntimeError as e:
        # Deliberate, explained failures: the message is the whole story.
        print(json.dumps({"error": str(e)}, indent=2))
        sys.exit(1)
    except (OSError, subprocess.SubprocessError) as e:
        # Unexpected: the type is part of the diagnosis.
        print(json.dumps({"error": f"{type(e).__name__}: {e}"}, indent=2))
        sys.exit(1)
    except KeyboardInterrupt:
        print(json.dumps({"error": "interrupted. Re-run the same phase to resume."}, indent=2))
        sys.exit(130)
    finally:
        # A phase that exits early or fails still spent the calls it made, so bill it either way.
        # preflight exits through sys.exit even when it passes, which a trailing call would miss.
        log_phase_cost(command, kwargs, time.time() - started)


if __name__ == "__main__":
    main()
