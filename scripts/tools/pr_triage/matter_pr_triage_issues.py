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
"""matter-pr-triage-issues: the issues tied to one pull request, and what closing it does to them.

Sibling of matter_pr_triage.py. It imports that script's GitHub plumbing and reads its cache for
the pull request's verdict; nothing flows the other way. Works one pull request at a time so the
cost is one pull request's shortlist, never a sweep the user did not ask for.

Three tiers, each with a different worker:
  linked / referenced_by / mentioned  - mechanical; the link is in the data
  inferred                            - the script hunts candidates by content, the model judges
  duplicates                          - the model, among the issues that surfaced

Like its sibling it never posts, comments, closes, labels or changes anything on GitHub.
"""

import argparse
import json
import math
import os
import re
import shutil
import subprocess
import sys
import time
from datetime import datetime, timedelta, timezone
from pathlib import Path

# The sibling lives in the same folder, also when this file is reached through a symlink.
sys.path.insert(0, str(Path(__file__).resolve().parent))
import matter_pr_triage as triage  # noqa: E402 isort:skip

TOOL_VERSION = "1.0.0"          # the bundle ships as one version; check_sync keeps the two scripts equal
DOSSIER_VERSION = 1          # bumped when pr-<n>.json changes shape; report refuses older files
DEFAULT_CANDIDATES = 8       # inferred issues per pull request; the model reads each one
SEARCH_HITS = 10             # per anchor search
MAX_TEST_ID_ANCHORS = 4
MAX_FILE_ANCHORS = 3
HUB_ATTACHED_PRS = 4         # more pull requests than this attached to one issue: a tracking issue
ISSUE_CACHE_FRESH_HOURS = 1  # an issue can close at any moment; a cached record is trusted this long
SIBLING_MATCHES = 5          # content matches shown per sibling repository, as context, never judged
RELATIONS = ("linked", "referenced_by", "mentioned", "inferred")
VERDICTS = ("close", "leave", "related", "already-closed", "unrelated", "unclear")
CONFIDENCES = ("high", "medium", "low")
EFFORT_LEVELS = ("low", "medium", "high", "xhigh", "max")
GENERIC_BASENAMES = {"build", "readme", "cmakelists", "init", "main", "test", "tests", "utils",
                     "index", "makefile", "conftest", "setup", "config", "common", "helpers"}

DISCLAIMER = [
    "> ## Disclaimer",
    ">",
    "> This is a best-effort report, meant to inform decision making about the issues related to a pull request, not to make those decisions. Every entry is machine-assisted and provisional. Read each issue before acting on it.",
    "",
]


# ---------------------------------------------------------------- paths


def issues_root(owner, name):
    return triage.cache_root(owner, name) / "issues"


def dossier_path(root, number):
    return root / f"pr-{number}.json"


def judgment_path(root, number):
    return root / f"pr-{number}.judgment.json"


def issue_cache_path(root, owner, name, number):
    return root / "issue" / owner / name / f"{number}.json"


def checked_number(value):
    """A pull request number from what people paste: 40191, #40191, or the pull request URL."""
    text = str(value).strip()
    m = re.fullmatch(r"(?:https?://)?(?:www\.)?github\.com/([^/\s]+)/([^/\s]+)/pull/(\d+)/?.*", text)
    if m:
        return int(m.group(3)), (m.group(1), m.group(2))
    m = re.fullmatch(r"#?(\d+)", text)
    if not m:
        raise RuntimeError(f"{value!r} is not a pull request number or URL")
    return int(m.group(1)), None


def resolve_repo(checkout, repo, pr_refs):
    """owner/name from --repo, else from a pull request URL among the refs, else from the clone."""
    if repo:
        return triage.split_repo(repo)
    for ref in pr_refs or []:
        _, from_url = checked_number(ref)
        if from_url:
            return from_url
    return triage.infer_repo(Path(checkout).resolve())


# ---------------------------------------------------------------- mechanical tiers


MENTION_SAME = re.compile(r"(?<![\w/#])#(\d{2,7})\b")
MENTION_CROSS = re.compile(r"(?<![\w/])([A-Za-z0-9_.-]+)/([A-Za-z0-9_.-]+)#(\d+)\b")
MENTION_URL = re.compile(r"github\.com/([A-Za-z0-9_.-]+)/([A-Za-z0-9_.-]+)/(?:issues|pull)/(\d+)")


def extract_mentions(text, owner, name, own_number):
    """Numbers a pull request's text points at, split into this repository and others.

    Same-repository numbers still need their type resolved: issues and pull requests share one
    number space, so a bare #12345 can be either. Cross-repository ones are keyed by repository.
    """
    same, cross = set(), {}
    for m in MENTION_URL.finditer(text or ""):
        o, n, num = m.group(1), m.group(2), int(m.group(3))
        if (o.lower(), n.lower()) == (owner.lower(), name.lower()):
            same.add(num)
        else:
            cross.setdefault((o, n), set()).add(num)
    stripped = MENTION_URL.sub(" ", text or "")
    for m in MENTION_CROSS.finditer(stripped):
        o, n, num = m.group(1), m.group(2), int(m.group(3))
        if (o.lower(), n.lower()) == (owner.lower(), name.lower()):
            same.add(num)
        else:
            cross.setdefault((o, n), set()).add(num)
    stripped = MENTION_CROSS.sub(" ", stripped)
    # A bare #N that the same text also writes with a repository qualifier is the author's shorthand
    # for that reference, not a second one in this repository.
    qualified_elsewhere = {n for nums in cross.values() for n in nums}
    same |= {int(x) for x in MENTION_SAME.findall(stripped)} - qualified_elsewhere
    same.discard(own_number)
    return {"same": same, "cross": cross}


TEST_ID = re.compile(r"\bTC[-_]([A-Z][A-Z0-9]{1,})[-_](\d+)(?:[._](\d+))?\b")


def test_ids(*texts):
    """Matter test identifiers in their canonical form, TC-XXX-n.m, from titles, bodies or paths.

    A filename spells the same test TC_XXX_n_m; both forms reduce to one anchor so the search
    for the issue finds it whichever way the issue template spelt it.
    """
    found = set()
    for text in texts:
        for m in TEST_ID.finditer(text or ""):
            ident = f"TC-{m.group(1)}-{m.group(2)}"
            if m.group(3):
                ident += f".{m.group(3)}"
            found.add(ident)
    return found


def distinctive_basenames(paths):
    """Filenames worth searching for, with their extension: a stem alone is often a component name
    that every issue in that area carries as a tag, while "ReadHandler.cpp" names the file."""
    out = []
    for p in paths or []:
        filename = Path(p).name
        stem = re.sub(r"\..*$", "", filename)
        if len(stem) < 9 or stem.lower() in GENERIC_BASENAMES or TEST_ID.search(stem):
            continue                                   # test ids are their own anchor
        if filename not in out:
            out.append(filename)
    return out


def anchors_for(title, body, changed_paths):
    """What to search open issues for. Ordered so the most specific anchors come first.

    Each anchor becomes one search; several aliases go in one API call. A test identifier is the
    strongest anchor this repository has: a certification failure issue and the pull request that
    fixes it both carry it. Title words are the weakest, so they are tried in two strengths.
    """
    anchors = []
    ids = sorted(test_ids(title, body, " ".join(changed_paths or [])))
    for ident in ids[:MAX_TEST_ID_ANCHORS]:
        anchors.append({"kind": "test_id", "terms": f'"{ident}"'})
    for base in distinctive_basenames(changed_paths)[:MAX_FILE_ANCHORS]:
        anchors.append({"kind": "file", "terms": f'"{base}"'})
    # Title words are searched in issue titles only. Over bodies and comments two ordinary words
    # match half the tracker; an issue whose title carries the same words is a real lead.
    tokens = sorted(triage.title_tokens(title), key=lambda w: (-len(w), w))
    if len(tokens) >= 3:
        anchors.append({"kind": "title", "terms": " ".join(tokens[:3]) + " in:title"})
    if len(tokens) >= 2:
        anchors.append({"kind": "title", "terms": " ".join(tokens[:2]) + " in:title"})
    return anchors


def jaccard(a, b):
    return len(a & b) / len(a | b) if (a or b) else 0.0


def rank_candidates(title, hits_by_anchor, exclude, limit):
    """Score every issue the anchor searches returned; the model sees only the top few.

    Score is how many anchors found it plus title similarity, so an issue matched by a test id and
    by title beats one that came up once on a common word. Ties go to the newer issue.
    """
    scored = {}
    own = triage.title_tokens(title)
    for anchor, hits in hits_by_anchor:
        for hit in hits:
            number = hit["number"]
            if number in exclude:
                continue
            entry = scored.setdefault(number, {"number": number, "title": hit.get("title"),
                                               "state": hit.get("state"), "anchors": [], "score": 0.0})
            entry["anchors"].append(anchor["kind"])
            entry["score"] += 1.0
    kept = []
    for entry in scored.values():
        similarity = jaccard(own, triage.title_tokens(entry["title"]))
        entry["score"] = round(entry["score"] + similarity, 3)
        # A single file-name hit with no words in common is a component sweep, not a lead: every
        # issue tagged with that component's name would qualify. Anything else has earned a read.
        if entry["anchors"] == ["file"] and similarity == 0:
            continue
        kept.append(entry)
    ranked = sorted(kept, key=lambda e: (-e["score"], -e["number"]))
    return ranked[:limit]


# ---------------------------------------------------------------- GitHub reads


PR_QUERY = """
query($owner: String!, $repo: String!, $number: Int!) {
  repository(owner: $owner, name: $repo) {
    pullRequest(number: $number) {
      number title url body state isDraft createdAt updatedAt closedAt merged
      author { login }
      baseRefName
      labels(first: 12) { nodes { name } }
      commits(first: 60) { nodes { commit { message } } }
      files(first: 100) { totalCount nodes { path } }
      closingIssuesReferences(first: 20) { nodes { number title state stateReason } }
      comments(first: 60) { totalCount nodes { author { login } body } }
      timelineItems(itemTypes: [CROSS_REFERENCED_EVENT], first: 50) {
        nodes { ... on CrossReferencedEvent {
          source { __typename
                   ... on Issue { number title state repository { nameWithOwner } }
                   ... on PullRequest { number title state repository { nameWithOwner } } } } }
      }
    }
  }
}
"""

ISSUE_FIELDS = """
  number title url state stateReason createdAt closedAt body
  author { login }
  labels(first: 12) { nodes { name } }
  comments { totalCount }
  closedByPullRequestsReferences(first: 10) { nodes { number state title } }
  timelineItems(itemTypes: [CROSS_REFERENCED_EVENT], first: 40) {
    nodes { ... on CrossReferencedEvent {
      source { __typename ... on PullRequest { number state title repository { nameWithOwner } } } } }
  }
"""


def gql_string(text):
    """A GraphQL string literal. JSON escaping is a superset of what GraphQL needs here."""
    return json.dumps(text)


def fetch_pr(owner, name, number):
    data = triage.gh_graphql(PR_QUERY, {"owner": owner, "repo": name, "number": number})
    pr = (data.get("repository") or {}).get("pullRequest")
    if not pr:
        raise RuntimeError(f"{owner}/{name}#{number} is not a pull request, or cannot be read")
    return pr


def resolve_types(owner, name, numbers):
    """Issue or PullRequest for each number, in batches of aliased lookups."""
    types = {}
    numbers = sorted(numbers)
    for i in range(0, len(numbers), 40):
        chunk = numbers[i:i + 40]
        fields = " ".join(f"n{n}: issueOrPullRequest(number: {n}) {{ __typename "
                          f"... on Issue {{ number title state }} ... on PullRequest {{ number title state }} }}"
                          for n in chunk)
        query = f"{{ repository(owner: {gql_string(owner)}, name: {gql_string(name)}) {{ {fields} }} }}"
        try:
            data = triage.gh_graphql(query)
        except RuntimeError as e:
            # One unknown number fails the whole batch; fall back to one call per number.
            if len(chunk) == 1:
                types[chunk[0]] = {"type": None, "error": str(e)[:200]}
                continue
            for n in chunk:
                types.update(resolve_types(owner, name, [n]))
            continue
        repo_data = data.get("repository") or {}
        for n in chunk:
            node = repo_data.get(f"n{n}")
            types[n] = ({"type": node["__typename"], "title": node.get("title"), "state": node.get("state")}
                        if node else {"type": None})
    return types


def search_anchors(owner, name, anchors):
    """Every anchor search in one call. Returns [(anchor, hits)] in anchor order."""
    if not anchors:
        return []
    parts = []
    for i, anchor in enumerate(anchors):
        q = f"repo:{owner}/{name} is:issue is:open {anchor['terms']}"
        parts.append(f"a{i}: search(query: {gql_string(q)}, type: ISSUE, first: {SEARCH_HITS}) "
                     f"{{ nodes {{ ... on Issue {{ number title state }} }} }}")
    data = triage.gh_graphql("{ " + " ".join(parts) + " }")
    out = []
    for i, anchor in enumerate(anchors):
        nodes = [n for n in ((data.get(f"a{i}") or {}).get("nodes") or []) if n and n.get("number")]
        out.append((anchor, nodes))
    return out


def compact_issue(node, owner, name):
    """The issue as the judge reads it: the ask, its standing, and which pull requests hang off it."""
    body, body_note = triage.truncate(node.get("body") or "", 2500)
    attached = {}
    for ref in (node.get("closedByPullRequestsReferences") or {}).get("nodes") or []:
        attached[ref["number"]] = {"number": ref["number"], "state": ref.get("state"),
                                   "title": ref.get("title"), "how": "closing_reference"}
    for item in (node.get("timelineItems") or {}).get("nodes") or []:
        src = (item or {}).get("source") or {}
        if src.get("__typename") != "PullRequest" or not src.get("number"):
            continue
        if (src.get("repository") or {}).get("nameWithOwner", f"{owner}/{name}") != f"{owner}/{name}":
            continue
        attached.setdefault(src["number"], {"number": src["number"], "state": src.get("state"),
                                            "title": src.get("title"), "how": "mentions_issue"})
    labels = [x["name"] for x in (node.get("labels") or {}).get("nodes") or []]
    tracking = len(attached) > HUB_ATTACHED_PRS or any(
        re.search(r"epic|tracking|umbrella", lbl, re.I) for lbl in labels)
    return {
        "number": node["number"], "title": node.get("title"), "url": node.get("url"),
        "tracking_issue_hint": tracking,
        "state": node.get("state"), "state_reason": node.get("stateReason"),
        "created_at": node.get("createdAt"), "closed_at": node.get("closedAt"),
        "author": (node.get("author") or {}).get("login"),
        "labels": labels,
        "body": body, "body_truncated": body_note,
        "comments": (node.get("comments") or {}).get("totalCount"),
        "attached_pull_requests": sorted(attached.values(), key=lambda a: a["number"]),
        "repository": f"{owner}/{name}",
        "fetched_at": datetime.now(timezone.utc).isoformat(timespec="seconds"),
    }


def cache_is_fresh(fetched_at):
    """A cached issue record is reused only while its state can be trusted."""
    try:
        age = datetime.now(timezone.utc) - datetime.fromisoformat(str(fetched_at).replace("Z", "+00:00"))
    except (TypeError, ValueError):
        return False
    return timedelta(0) <= age < timedelta(hours=ISSUE_CACHE_FRESH_HOURS)


def fetch_issues(root, owner, name, numbers, refetch=False):
    """Issue details, from the per-issue cache where present. Returns {number: details|error}."""
    out, missing = {}, []
    for n in sorted(set(numbers)):
        cached = None if refetch else triage.read_json(issue_cache_path(root, owner, name, n))
        if cached and cache_is_fresh(cached.get("fetched_at")):
            out[n] = cached
        else:
            missing.append(n)
    for i in range(0, len(missing), 25):
        chunk = missing[i:i + 25]
        fields = " ".join(f"i{n}: issue(number: {n}) {{ {ISSUE_FIELDS} }}" for n in chunk)
        query = f"{{ repository(owner: {gql_string(owner)}, name: {gql_string(name)}) {{ {fields} }} }}"
        try:
            data = triage.gh_graphql(query)
        except RuntimeError as e:
            if len(chunk) == 1:
                out[chunk[0]] = {"number": chunk[0], "error": str(e)[:300]}
                continue
            for n in chunk:
                out.update(fetch_issues(root, owner, name, [n], refetch))
            continue
        repo_data = data.get("repository") or {}
        for n in chunk:
            node = repo_data.get(f"i{n}")
            if not node:
                out[n] = {"number": n, "error": "not an issue in this repository, or not readable"}
                continue
            details = compact_issue(node, owner, name)
            triage.write_json(issue_cache_path(root, owner, name, n), details)
            out[n] = details
    return out


# ---------------------------------------------------------------- issue corpus


CORPUS_CLOSED_MONTHS = 18     # closed issues kept this far back: duplicates and "closed as" chains
CORPUS_PAGE = 100
LONGLIST = 60                 # titles the judge skims; the read list is the top few of these
BODY_KEEP = 4000
QUERY_TERMS = 40              # most distinctive PR terms used for text similarity
SIGNAL_WEIGHTS = {"exact": 1.0, "text": 2.0, "title": 2.0, "refers": 3.0, "paths": 1.0,
                  "author": 1.0, "time": 0.5, "labels": 0.5, "graph": 2.0}
CONTENT_SIGNALS = ("exact", "text", "title", "refers", "paths", "graph")   # boosters alone never make a lead
EXACT_CAP = 5.0
IDENTIFIER_CAP = 2.0          # camel-case names from a log excerpt say less than a test id or a file
TITLE_IDF_FULL = 8.0          # shared title words summing to this much rarity count as a full match
READ_CAPS = {"graph": 3, "text": 3, "paths": 2}   # per lead kind, in the read list; the rest stay skimmable
# Platform names as the repository spells them in titles, so "nordic board" meets "[nrfconnect]".
ALIASES = {"nordic": "nrfconnect", "nrf": "nrfconnect", "nrf52": "nrfconnect", "nrf53": "nrfconnect",
           "espressif": "esp32", "esp32c3": "esp32", "esp32s3": "esp32", "esp32h2": "esp32",
           "macos": "darwin", "ios": "darwin", "apple": "darwin", "xcode": "darwin",
           "efr32": "silabs", "siliconlabs": "silabs", "silicon": "silabs", "mgm24": "silabs",
           "nxp": "nxp", "k32w": "nxp", "mcxw": "nxp", "rw61x": "nxp", "rt1060": "nxp",
           "telink": "telink", "tlsr": "telink", "bouffalo": "bouffalolab", "bl602": "bouffalolab",
           "raspberry": "linux", "raspberrypi": "linux", "ubuntu": "linux", "rpi": "linux"}
COMMON_TERM_DF = 50           # an identifier in more issues than this is log noise, not a pointer
TEXT_FLOOR = 0.35             # below this, wording alone is not a lead
STEM_SUFFIXES = ("ing", "ed", "es", "s")
TWO_LETTER_STOP = set("an as at be by do go he if in is it me my no of on or so to up us we".split())
PATH_RE = re.compile(r"\b(?:src|examples|scripts|docs|third_party|config|integrations|build_overrides|data_model)/[\w./-]+")

STOPWORDS = set("""
the and for with from into when new use using support test tests this that these those there their
then than also only just like will would should could can may might must shall about after before
during while between within without across against along around behind below beneath beside beyond
under over above being been are was were has have had having does did doing done not but nor yet
where which who whom whose what why how all any both each few more most other some such own same
very via per one two three first second next last some any every none here more less many much
issue issues bug pull request pr prs fix fixes fixed fixing add adds added adding update updates
updated updating change changes changed remove removes removed make makes made need needs needed
please thanks thank hello error errors fail fails failed failure problem problems expected actual
behavior behaviour steps reproduce reproduction version platform description summary note notes
matter chip sdk github com https http www file files line lines code https
""".split())
STOPWORDS |= triage.TITLE_NOISE

TOKEN_RE = re.compile(r"[a-z0-9]+(?:[._:/_-][a-z0-9]+)*")
CAMEL_RE = re.compile(r"\b(?:[A-Z][a-z0-9]+){2,}[A-Za-z0-9]*\b|\b[a-z]+(?:[A-Z][a-z0-9]+)+\b")
ERROR_CODE_RE = re.compile(r"\b0x[0-9A-Fa-f]{4,}\b|\bCHIP_ERROR_[A-Z0-9_]+\b|\bCHIP_[A-Z0-9_]{4,}\b")
BACKTICK_RE = re.compile(r"`([^`\n]{4,80})`")
SCOPED_RE = re.compile(r"\b[A-Za-z_][A-Za-z0-9_]*(?:::[A-Za-z_][A-Za-z0-9_]*)+\b")


def corpus_dir(root):
    return root / "corpus"


def stem(word):
    """Just enough stemming for titles to meet: uploaded meets upload, segments meets segment."""
    if not word.isalpha() or len(word) <= 4:
        return word
    for suffix in STEM_SUFFIXES:
        if word.endswith(suffix) and len(word) - len(suffix) >= 3:
            return word[:-len(suffix)]
    return word


def tokenize(text):
    """Lower-case tokens, keeping joined identifiers whole: tc-jfds-2.3, readhandler.cpp,
    0x00000032, chip::app. Their parts are added too, so a title word still meets a compound;
    camel case is split the same way, so ZoneManagement meets "Zone Management"; a test id is
    added in its canonical form whichever way it was spelt; two-letter acronyms such as JF stay."""
    text = text or ""
    out = [t.lower() for t in test_ids(text)]
    spaced = re.sub(r"([a-z0-9])([A-Z])", r"\1 \2", text)
    spaced = re.sub(r"([A-Z]+)([A-Z][a-z])", r"\1 \2", spaced)
    for tok in TOKEN_RE.findall((text + "\n" + spaced).lower()):
        parts = [p for p in re.split(r"[._:/_-]", tok) if p]
        for cand in ([tok] if len(parts) == 1 else [tok] + parts):
            if len(cand) < 2 or cand in STOPWORDS or (len(cand) == 2 and cand in TWO_LETTER_STOP):
                continue
            out.append(stem(ALIASES.get(cand, cand)))
    return out


def exact_anchors(title, body, changed_paths):
    """Strings specific enough to match on their own, with a weight each.

    A test identifier or a file name pins the topic; an error code or a scoped symbol from a log
    excerpt pins the symptom; camel-case identifiers and back-ticked names pin the code."""
    anchors = {}
    for ident in test_ids(title, body, " ".join(changed_paths or [])):
        anchors[ident.lower()] = ("test_id", 3.0)
    for filename in distinctive_basenames(changed_paths):
        anchors[filename.lower()] = ("file", 2.5)
    text = f"{title or ''}\n{body or ''}"
    for m in ERROR_CODE_RE.findall(text):
        anchors.setdefault(m.lower(), ("error_code", 2.5))
    for m in SCOPED_RE.findall(text):
        anchors.setdefault(m.lower(), ("symbol", 1.5))
    for m in BACKTICK_RE.findall(text):
        if re.fullmatch(r"[A-Za-z0-9_.:/-]+", m.strip()) and len(m.strip()) >= 6:
            anchors.setdefault(m.strip().lower(), ("symbol", 1.0))
    for m in CAMEL_RE.findall(text):
        if len(m) >= 8:
            anchors.setdefault(m.lower(), ("identifier", 0.75))
    return anchors


def corpus_record(node, owner, name):
    body = triage.truncate(node.get("body") or "", BODY_KEEP)[0]
    return {
        "number": node["number"], "title": node.get("title") or "", "state": node.get("state"),
        "state_reason": node.get("stateReason"), "created_at": node.get("createdAt"),
        "updated_at": node.get("updatedAt"), "closed_at": node.get("closedAt"),
        "author": (node.get("author") or {}).get("login"),
        "labels": [x["name"] for x in (node.get("labels") or {}).get("nodes") or []],
        "parent": ((node.get("parent") or {}).get("number")),
        "sub_issues": [x["number"] for x in (node.get("subIssues") or {}).get("nodes") or []],
        "refs": sorted(extract_mentions(body, owner, name, node["number"])["same"]),
        "paths": sorted({m.rstrip(".,:;)") for m in PATH_RE.findall(body)})[:40],
        "body": body,
    }


ISSUES_PAGE_QUERY = """
query($owner: String!, $repo: String!, $after: String) {
  repository(owner: $owner, name: $repo) {
    issues(first: %d, states: [%s], orderBy: {field: UPDATED_AT, direction: DESC}, after: $after) {
      pageInfo { hasNextPage endCursor }
      nodes {
        number title state stateReason createdAt updatedAt closedAt body
        author { login }
        labels(first: 12) { nodes { name } }
        parent { number }
        subIssues(first: 20) { nodes { number } }
      }
    }
  }
}
"""


FAMILY_FIELDS = "        parent { number }\n        subIssues(first: 20) { nodes { number } }\n"
SUPPORTS_FAMILY = {"value": True}     # flipped off for the process when the API rejects the fields


def page_issues(owner, name, state, after):
    variables = {"owner": owner, "repo": name}
    if after:
        variables["after"] = after
    query = ISSUES_PAGE_QUERY % (CORPUS_PAGE, state)
    if not SUPPORTS_FAMILY["value"]:
        query = query.replace(FAMILY_FIELDS, "")
    try:
        data = triage.gh_graphql(query, variables)
    except RuntimeError as e:
        # A GitHub without sub-issues rejects the two family fields; the corpus is still worth having.
        if SUPPORTS_FAMILY["value"] and ("parent" in str(e) or "subIssues" in str(e)):
            SUPPORTS_FAMILY["value"] = False
            return page_issues(owner, name, state, after)
        raise
    block = (data.get("repository") or {}).get("issues") or {}
    return [n for n in block.get("nodes") or [] if n], block.get("pageInfo") or {}


def merge_records(existing, fetched):
    """Upsert by number. Returns (records, added, updated)."""
    by_number = {r["number"]: r for r in existing}
    added = updated = 0
    for r in fetched:
        if r["number"] in by_number:
            if by_number[r["number"]] != r:
                updated += 1
        else:
            added += 1
        by_number[r["number"]] = r
    return [by_number[n] for n in sorted(by_number)], added, updated


def load_corpus(root):
    path = corpus_dir(root) / "issues.jsonl"
    if not path.exists():
        return None
    records = []
    with path.open(encoding="utf-8") as fh:
        for line in fh:
            line = line.strip()
            if line:
                records.append(json.loads(line))
    return records


def save_corpus(root, records, meta):
    path = corpus_dir(root) / "issues.jsonl"
    path.parent.mkdir(parents=True, exist_ok=True)
    tmp = path.with_suffix(".jsonl.tmp")
    with tmp.open("w", encoding="utf-8") as fh:
        for r in records:
            fh.write(json.dumps(r, ensure_ascii=False) + "\n")
    tmp.replace(path)
    triage.write_json(corpus_dir(root) / "meta.json", meta)


def sibling_repos(root, extra=()):
    """Repositories whose issue corpora are consulted as context: the ones recorded by an earlier
    sync, the ones named in MATTER_PR_TRIAGE_SIBLINGS, and any given now."""
    listed = triage.read_json(root / "siblings.json", []) or []
    env = [x for x in os.environ.get("MATTER_PR_TRIAGE_SIBLINGS", "").split(":") if x]
    out = []
    for item in list(listed) + env + list(extra):
        # The same two ways the main repository is named: a clone you have, or owner/name.
        try:
            path = Path(str(item)).expanduser()
            o, n = triage.infer_repo(path.resolve()) if path.is_dir() else triage.split_repo(item)
        except RuntimeError:
            continue
        full = f"{o}/{n}"
        if full not in out:
            out.append(full)
    return out


def sync_corpus(owner, name, full, closed_months):
    """Pull every open issue and the recently closed ones of one repository into its corpus,
    incrementally. Pages come newest-updated first, so a page older than the last sync ends the
    incremental pass, and for closed issues a page older than the window ends the full pass."""
    root = issues_root(owner, name)
    meta = triage.read_json(corpus_dir(root) / "meta.json", {}) or {}
    existing = (load_corpus(root) or []) if not full else []
    since = None if (full or not existing) else meta.get("synced_at")
    started = datetime.now(timezone.utc)
    window = (started - timedelta(days=30 * closed_months)).isoformat(timespec="seconds")
    fetched, pages = [], 0
    for state in ("OPEN", "CLOSED"):
        after, more = None, True
        while more:
            nodes, info = page_issues(owner, name, state, after)
            pages += 1
            for node in nodes:
                fetched.append(corpus_record(node, owner, name))
            oldest = nodes[-1]["updatedAt"] if nodes else None
            stop = (oldest and since and oldest < since) or (oldest and state == "CLOSED" and oldest < window)
            more = bool(info.get("hasNextPage")) and not stop
            after = info.get("endCursor")
    records, added, updated = merge_records(existing, fetched)
    cutoff = window[:10]
    # A full pass rebuilds from scratch, so closed issues that fell out of the window go with it;
    # an incremental pass keeps what it has, since nothing about those issues changed.
    if full:
        records = [r for r in records if r["state"] == "OPEN" or (r.get("updated_at") or "") >= window]
    meta = {"synced_at": started.isoformat(timespec="seconds"), "repo": f"{owner}/{name}",
            "issues": len(records), "open": sum(1 for r in records if r["state"] == "OPEN"),
            "closed_window_months": closed_months, "closed_since": cutoff,
            "last_pass": "full" if since is None else "incremental", "pages": pages}
    save_corpus(root, records, meta)
    return {"repo": f"{owner}/{name}", "corpus": str(corpus_dir(root) / "issues.jsonl"), "pass": meta["last_pass"],
            "fetched": len(fetched), "added": added, "updated": updated,
            "issues": len(records), "open": meta["open"], "pages": pages}


def sync(checkout, repo, full, closed_months, siblings=()):
    """The repository's corpus, then the corpora of its sibling repositories, such as the one that
    holds its test plans, so that their issues surface as context on every pull request."""
    owner, name = resolve_repo(checkout, repo, [])
    root = issues_root(owner, name)
    # A sibling that names nothing must not vanish quietly: the user would wait for matches that never come.
    bad = [item for item in siblings if not sibling_repos(issues_root("none", "none"), [item])]
    if bad:
        print(json.dumps({"error": "bad_sibling", "given": bad,
                          "fix": "name each sibling as owner/name, a github.com repository URL, or the path of a clone"}, indent=2))
        sys.exit(2)
    out = {"primary": sync_corpus(owner, name, full, closed_months), "siblings": []}
    wanted = sibling_repos(root, siblings)
    for full_name in wanted:
        so, sn = full_name.split("/", 1)
        try:
            out["siblings"].append(sync_corpus(so, sn, full, closed_months))
        except RuntimeError as e:
            out["siblings"].append({"repo": full_name, "error": str(e)[:300]})
    if wanted:
        triage.write_json(root / "siblings.json", wanted)
    print(json.dumps(out, indent=2))


class Index:
    """Term statistics over the corpus, for BM25 text similarity and exact-anchor lookups."""

    K1, B = 1.5, 0.75

    def __init__(self, records):
        self.records = records
        self.docs = {}          # number -> {term: tf} over title and body
        self.titles = {}        # number -> set of title terms
        self.lengths = {}
        self.df = {}
        for r in records:
            title_toks = tokenize(r["title"])
            toks = title_toks + tokenize(r.get("body") or "")
            tf = {}
            for t in toks:
                tf[t] = tf.get(t, 0) + 1
            self.docs[r["number"]] = tf
            self.titles[r["number"]] = set(title_toks)
            self.lengths[r["number"]] = len(toks)
            for t in tf:
                self.df[t] = self.df.get(t, 0) + 1
        self.n = max(1, len(records))
        self.avg_len = (sum(self.lengths.values()) / self.n) if records else 1.0

    def idf(self, term):
        df = self.df.get(term, 0)
        return math.log(1 + (self.n - df + 0.5) / (df + 0.5))

    def query_terms(self, title, rest, limit=QUERY_TERMS):
        """Every title term, then the most distinctive terms of the rest, so a long pull request
        body with many file names never crowds the title out of the query."""
        terms = []
        for t in tokenize(title):
            if t not in terms and self.df.get(t, 0) > 0:
                terms.append(t)
        seen = {}
        for t in tokenize(rest):
            if t not in terms and self.df.get(t, 0) > 0:
                seen[t] = self.idf(t)
        ranked = sorted(seen.items(), key=lambda kv: -kv[1])
        return terms + [t for t, _ in ranked[:max(0, limit - len(terms))]]

    def bm25(self, terms, number):
        """Text similarity with title hits counted again, and nothing for a single shared word:
        one word in common with a long issue is coincidence, two start to mean something."""
        tf = self.docs.get(number) or {}
        if not tf:
            return 0.0
        length = self.lengths[number]
        titles = self.titles.get(number) or set()
        score, matched = 0.0, 0
        for t in terms:
            f = tf.get(t)
            if not f:
                continue
            matched += 1
            part = self.idf(t) * (f * (self.K1 + 1)) / (f + self.K1 * (1 - self.B + self.B * length / self.avg_len))
            score += part * (2.0 if t in titles else 1.0)
        return score if matched >= 2 else 0.0

    def has(self, number, term):
        return term in (self.docs.get(number) or {})


def parse_date(text):
    if not text:
        return None
    return datetime.fromisoformat(text.replace("Z", "+00:00"))


def graph_hops(records, seeds, max_hops=2):
    """Distance from the seed issues over parent, sub-issue and body-reference edges."""
    by_number = {r["number"]: r for r in records}
    edges = {}
    for r in records:
        n = r["number"]
        for m in [r.get("parent")] + list(r.get("sub_issues") or []) + list(r.get("refs") or []):
            if m and m in by_number:
                edges.setdefault(n, set()).add(m)
                edges.setdefault(m, set()).add(n)
    dist = {s: 0 for s in seeds if s in by_number}
    frontier = list(dist)
    for hop in range(1, max_hops + 1):
        nxt = []
        for n in frontier:
            for m in edges.get(n, ()):
                if m not in dist:
                    dist[m] = hop
                    nxt.append(m)
        frontier = nxt
    return dist


def score_candidates(index, pr, seeds, exclude, longlist=LONGLIST, cross_repo=False):
    """Every corpus issue scored against the pull request; the top of the list is what the judge
    sees. Each entry says which signals fired, so a reader can weigh the lead the way its finder
    would have to."""
    title, body, paths = pr.get("title") or "", pr.get("body") or "", pr.get("changed_paths") or []
    number_of_pr = pr.get("number")
    anchors = {}
    for a, (kind, w) in exact_anchors(title, body, paths).items():
        # A pinned anchor that half the tracker's logs contain pins nothing.
        if index.df.get(a, 0) <= COMMON_TERM_DF or kind in ("test_id", "file"):
            anchors[a] = (kind, w)
    terms = index.query_terms(title, f"{body}\n{' '.join(paths)}")
    title_terms = set(tokenize(title))
    pr_created = parse_date(pr.get("created_at"))
    pr_labels = set(pr.get("labels") or [])
    pr_dirs = {"/".join(p.split("/")[:-1]) for p in paths if "/" in p}
    hops = graph_hops(index.records, seeds) if seeds else {}
    raw = []
    for r in index.records:
        n = r["number"]
        if n in exclude:
            continue
        signals, why = {}, []
        hits = [(a, kind, w) for a, (kind, w) in anchors.items() if index.has(n, a)]
        if hits:
            pinned = sum(w for _, kind, w in hits if kind != "identifier")
            named = min(IDENTIFIER_CAP, sum(w for _, kind, w in hits if kind == "identifier"))
            signals["exact"] = min(EXACT_CAP, pinned + named)
            why.append("shares " + ", ".join(sorted({f"{kind} {a}" for a, kind, _ in hits}))[:120])
        text = index.bm25(terms, n)
        if text:
            signals["text"] = text
        shared_title = title_terms & (index.titles.get(n) or set())
        if shared_title:
            rarity = sum(index.idf(t) for t in shared_title)
            signals["title"] = min(1.0, rarity / TITLE_IDF_FULL)
            why.append("title shares " + ", ".join(sorted(shared_title))[:60])
        if not cross_repo and number_of_pr and number_of_pr in (r.get("refs") or []):
            signals["refers"] = 1.0
            why.append("names this pull request")
        issue_paths = set() if cross_repo else set(r.get("paths") or [])
        same_file = {p for p in paths if p in issue_paths or Path(p).name in {Path(q).name for q in issue_paths}}
        deep_dirs = {d for d in pr_dirs if d.count("/") >= 2} & {"/".join(q.split("/")[:-1]) for q in issue_paths}
        if same_file:
            signals["paths"] = 1.0
            why.append("names " + Path(sorted(same_file)[0]).name)
        elif deep_dirs:
            signals["paths"] = 0.5
            why.append("names a file in " + sorted(deep_dirs)[0])
        created = parse_date(r.get("created_at"))
        if pr_created and created:
            days = abs((created - pr_created).days)
            if r.get("author") and r.get("author") == pr.get("author") and days <= 180:
                signals["author"] = 1.0
                why.append(f"same author, {days} days apart")
            if days <= 60:
                signals["time"] = 1.0 - days / 60.0
        overlap = pr_labels & set(r.get("labels") or [])
        if overlap:
            signals["labels"] = min(1.0, 0.4 * len(overlap))
            why.append("labels " + ", ".join(sorted(overlap)))
        if n in hops and hops[n] > 0:
            signals["graph"] = 1.0 if hops[n] == 1 else 0.5
            why.append(f"{hops[n]} hop{'s' if hops[n] > 1 else ''} from a linked issue")
        if signals:
            raw.append((n, r, signals, why))
    top_text = max((s.get("text", 0.0) for _, _, s, _ in raw), default=0.0) or 1.0
    scored = []
    for n, r, signals, why in raw:
        if "text" in signals:
            signals["text"] = signals["text"] / top_text
            if signals["text"] < TEXT_FLOOR:
                del signals["text"]
            elif signals["text"] >= 0.5:
                why.append("similar wording")
        # Same author, same week, same label: each says "could be", none says "is". Without a
        # content signal the entry is a hunch about people, not about the issue, and is dropped.
        if not any(k in signals for k in CONTENT_SIGNALS):
            continue
        total = sum(SIGNAL_WEIGHTS[k] * v for k, v in signals.items())
        scored.append({"number": n, "title": r["title"], "state": r["state"], "score": round(total, 3),
                       "signals": {k: round(v, 3) for k, v in signals.items()}, "why": "; ".join(why)})
    scored.sort(key=lambda e: (-e["score"], -e["number"]))
    return scored[:longlist]


def lead_kind(entry):
    """What made this entry a lead: its content signals, or the single one when there is one."""
    kinds = [k for k in CONTENT_SIGNALS if k in entry.get("signals", {})]
    return kinds[0] if len(kinds) == 1 else "mixed"


def read_list(longlist, limit):
    """The top of the longlist with a cap per lead kind, so one family of sibling issues or a run
    of look-alike wording cannot fill every read; the rest stay on the longlist to be skimmed."""
    taken, counts = [], {}
    for entry in longlist:
        kind = lead_kind(entry)
        cap = READ_CAPS.get(kind)
        if cap is not None and counts.get(kind, 0) >= cap:
            continue
        counts[kind] = counts.get(kind, 0) + 1
        taken.append(entry)
        if len(taken) >= limit:
            break
    return taken


def mask_numbers(text, numbers):
    """Remove references to these numbers from a text, so a benchmark measures content alone."""
    out = text or ""
    for n in numbers:
        out = re.sub(rf"(?<![\w/])#{n}\b", " ", out)
        out = re.sub(rf"github\.com/[^\s/]+/[^\s/]+/(?:issues|pull)/{n}\b", " ", out)
        out = re.sub(rf"[A-Za-z0-9_.-]+/[A-Za-z0-9_.-]+#{n}\b", " ", out)
    return out


def pr_for_scoring(pr_node):
    """The fields scoring reads, from the GraphQL pull request node."""
    return {"number": pr_node["number"], "title": pr_node.get("title") or "", "body": pr_node.get("body") or "",
            "author": (pr_node.get("author") or {}).get("login"), "created_at": pr_node.get("createdAt"),
            "labels": [x["name"] for x in (pr_node.get("labels") or {}).get("nodes") or []],
            "changed_paths": [f["path"] for f in (pr_node.get("files") or {}).get("nodes") or []],
            "commit_messages": [((c.get("commit") or {}).get("message") or "")
                                for c in (pr_node.get("commits") or {}).get("nodes") or []]}


def recall_metrics(rows, k):
    """rows: [{"truth": [...], "ranks": {issue: rank or None}}]. Per-issue recall and MRR."""
    total = found_k = found_long = 0
    rr = []
    for row in rows:
        best = None
        for n in row["truth"]:
            total += 1
            rank = row["ranks"].get(n)
            if rank is not None:
                found_long += 1
                if rank <= k:
                    found_k += 1
                best = rank if best is None else min(best, rank)
        rr.append(1.0 / best if best else 0.0)
    return {"issues": total, f"recall_at_{k}": round(found_k / total, 3) if total else None,
            f"recall_at_{LONGLIST}": round(found_long / total, 3) if total else None,
            "mrr": round(sum(rr) / len(rr), 3) if rr else None}


def benchmark(checkout, repo, sample, seed, k, keep_mentions):
    """How often the inferred tier would have found an issue on its own.

    Pull requests with formal closing links are free ground truth: hide the link, score the corpus
    from the pull request's content alone, and see where the linked issue lands."""
    import random
    owner, name = resolve_repo(checkout, repo, [])
    root = issues_root(owner, name)
    records = load_corpus(root)
    if not records:
        raise RuntimeError("no corpus yet. Run: sync")
    index = Index(records)
    known = {r["number"] for r in records}
    listing = corpus_dir(root) / "linked_prs.json"
    cached_list = triage.read_json(listing)
    if cached_list and (datetime.now(timezone.utc) - parse_date(cached_list["at"])).days < 1:
        nodes, total = cached_list["nodes"], cached_list["total"]
    else:
        nodes, total = triage.search_page(f"repo:{owner}/{name} is:pr linked:issue", cap_pages=10)
        triage.write_json(listing, {"at": datetime.now(timezone.utc).isoformat(timespec="seconds"),
                                    "nodes": nodes, "total": total})
    nodes = [n for n in nodes if not triage.is_mechanical((n.get("author") or {}).get("login"))]
    rng = random.Random(seed)
    picked = sorted(rng.sample(nodes, min(sample, len(nodes))), key=lambda n: -n["number"])
    rows, skipped, signal_hits = [], 0, {}
    cache = corpus_dir(root) / "prs"
    for node in picked:
        number = node["number"]
        cached = triage.read_json(cache / f"{number}.json")
        if not cached:
            try:
                cached = fetch_pr(owner, name, number)
            except RuntimeError:
                skipped += 1
                continue
            triage.write_json(cache / f"{number}.json", cached)
        truth = sorted({i["number"] for i in (cached.get("closingIssuesReferences") or {}).get("nodes") or []} & known)
        if not truth:
            skipped += 1
            continue
        pr = pr_for_scoring(cached)
        if not keep_mentions:
            pr["body"] = mask_numbers(pr["body"], truth)
            pr["title"] = mask_numbers(pr["title"], truth)
            pr["commit_messages"] = [mask_numbers(m, truth) for m in pr["commit_messages"]]
        pr["body"] = pr["body"] + "\n" + "\n".join(pr["commit_messages"])
        ranked = score_candidates(index, pr, seeds=set(), exclude={number})
        reads = [e["number"] for e in read_list(ranked, k)]
        # Rank is the position in the read list where it made it, else its longlist position past k.
        ranks = {e["number"]: (reads.index(e["number"]) + 1 if e["number"] in reads else max(k + 1, i + 1))
                 for i, e in enumerate(ranked)}
        row = {"pr": number, "truth": truth, "ranks": {n: ranks.get(n) for n in truth}}
        for n in truth:
            if ranks.get(n):
                for s in next(e for e in ranked if e["number"] == n)["signals"]:
                    signal_hits[s] = signal_hits.get(s, 0) + 1
        rows.append(row)
    metrics = recall_metrics(rows, k)
    misses = [{"pr": r["pr"], "missed": [n for n in r["truth"] if r["ranks"].get(n) is None]}
              for r in rows if any(r["ranks"].get(n) is None for n in r["truth"])]
    out = {"repo": f"{owner}/{name}", "corpus_issues": len(records), "linked_prs_available": total,
           "sampled": len(picked), "scored": len(rows), "skipped": skipped, "seed": seed, "k": k,
           "mentions_masked": not keep_mentions, **metrics,
           "signals_on_found_truths": signal_hits, "misses": misses[:40]}
    stamp = datetime.now(timezone.utc).strftime("%Y%m%d-%H%M%S")
    triage.write_json(corpus_dir(root) / f"benchmark-{stamp}.json", {**out, "rows": rows})
    print(json.dumps(out, indent=2))


# ---------------------------------------------------------------- the sibling's verdict


def triage_record(owner, name, number):
    """What matter_pr_triage decided about this pull request, if it ever saw it.

    The verdict and its reason are the coverage proof: an issue closes with the pull request only
    when the pull request itself is closing because its work is already on the base branch.
    """
    root = triage.cache_root(owner, name)
    state = triage.read_json(root / "state.json", {}) or {}
    recorded = state.get(str(number))
    dossier = triage.read_json(root / "pr" / f"{number}.json") or {}
    if not recorded:
        return {"triaged": False, "changed_paths": dossier.get("changed_paths") or []}
    verdict_entry = {}
    judgment = triage.read_json(root / "runs" / recorded.get("run", "") / "judgment.json") or {}
    for v in judgment.get("verdicts", []):
        if int(str(v.get("pr")).lstrip("#")) == number:
            verdict_entry = v
            break
    return {
        "triaged": True,
        "verdict": recorded.get("verdict"),
        "coverage": verdict_entry.get("coverage"),
        "reason": verdict_entry.get("evidence") or verdict_entry.get("reason"),
        "confidence": verdict_entry.get("confidence"),
        "run": recorded.get("run"),
        "judged_on": recorded.get("at"),
        "master_sha": dossier.get("master_sha"),
        "changed_paths": dossier.get("changed_paths") or [],
        "coverage_hint": ((dossier.get("signals") or {}).get("coverage_hint")),
    }


def from_triage_numbers(owner, name, verdicts):
    """Pull requests the sibling's report gave one of these verdicts, newest first."""
    state = triage.read_json(triage.cache_root(owner, name) / "state.json", {}) or {}
    return sorted((int(n) for n, v in state.items() if v.get("verdict") in verdicts), reverse=True)


def gather_one(root, owner, name, number, candidates, refetch, no_search, reads=()):
    pr = fetch_pr(owner, name, number)
    record = triage_record(owner, name, number)
    paths = [f["path"] for f in (pr.get("files") or {}).get("nodes") or []] or record.get("changed_paths") or []
    human_comments = [c.get("body") or "" for c in (pr.get("comments") or {}).get("nodes") or []
                      if not triage.is_mechanical((c.get("author") or {}).get("login"))]
    # Commit messages carry "Fixes #123" far more often than a body that was never filled in.
    commit_messages = [((c.get("commit") or {}).get("message") or "")
                       for c in (pr.get("commits") or {}).get("nodes") or []]
    text = "\n".join([pr.get("title") or "", pr.get("body") or ""] + human_comments + commit_messages)

    # Tier 1: the link is in the data.
    linked = {i["number"]: {"number": i["number"], "title": i.get("title"), "state": i.get("state"),
                            "relation": "linked"}
              for i in (pr.get("closingIssuesReferences") or {}).get("nodes") or []}
    referenced_by, mentioned_prs, cross_refs = {}, {}, []
    for item in (pr.get("timelineItems") or {}).get("nodes") or []:
        src = (item or {}).get("source") or {}
        if not src.get("number"):
            continue
        repo_full = (src.get("repository") or {}).get("nameWithOwner", f"{owner}/{name}")
        if repo_full != f"{owner}/{name}":
            cross_refs.append({"repository": repo_full, "number": src["number"], "type": src.get("__typename"),
                               "title": src.get("title"), "state": src.get("state"), "how": "references this pull request"})
            continue
        if src.get("__typename") == "Issue" and src["number"] not in linked:
            referenced_by[src["number"]] = {"number": src["number"], "title": src.get("title"),
                                            "state": src.get("state"), "relation": "referenced_by"}
        elif src.get("__typename") == "PullRequest":
            mentioned_prs.setdefault(src["number"], {"number": src["number"], "title": src.get("title"),
                                                     "state": src.get("state"), "how": "references this pull request"})

    mentions = extract_mentions(text, owner, name, number)
    known = set(linked) | set(referenced_by)
    to_resolve = sorted(mentions["same"] - known)
    mentioned = {}
    types = resolve_types(owner, name, to_resolve) if to_resolve else {}
    for n, t in types.items():
        if t.get("type") == "Issue":
            mentioned[n] = {"number": n, "title": t.get("title"), "state": t.get("state"), "relation": "mentioned"}
        elif t.get("type") == "PullRequest":
            mentioned_prs.setdefault(n, {"number": n, "title": t.get("title"), "state": t.get("state"),
                                         "how": "mentioned by this pull request"})
    skipped = []
    for (o, n_), nums in sorted(mentions["cross"].items()):
        for num in sorted(nums):
            try:
                t = resolve_types(o, n_, [num]).get(num) or {}
            except RuntimeError as e:
                t = {"type": None, "error": str(e)[:200]}
            if t.get("type") == "Issue":
                cross_refs.append({"repository": f"{o}/{n_}", "number": num, "type": "Issue",
                                   "title": t.get("title"), "state": t.get("state"), "how": "mentioned by this pull request"})
            elif t.get("type") == "PullRequest":
                cross_refs.append({"repository": f"{o}/{n_}", "number": num, "type": "PullRequest",
                                   "title": t.get("title"), "state": t.get("state"), "how": "mentioned by this pull request"})
            else:
                skipped.append({"repository": f"{o}/{n_}", "number": num,
                                "why": t.get("error") or "not readable with this token"})

    # Tier 2: candidates by content. The model decides; the script only shortlists. With a local
    # corpus every open issue is scored on several signals and the judge skims a longlist; without
    # one, a few anchor searches through the API stand in.
    anchors, inferred, longlist, source = [], [], [], "none"
    exclude = set(linked) | set(referenced_by) | set(mentioned) | {number}
    records = None if no_search else load_corpus(root)
    if records:
        source = "corpus"
        scoring_pr = pr_for_scoring(pr)
        scoring_pr["changed_paths"] = paths
        scoring_pr["body"] = scoring_pr["body"] + "\n" + "\n".join(commit_messages)
        seeds = set(linked) | set(referenced_by) | set(mentioned)
        longlist = score_candidates(Index(records), scoring_pr, seeds, exclude)
        inferred = [dict(e) for e in read_list(longlist, candidates)]
    elif not no_search:
        source = "search"
        anchors = anchors_for(pr.get("title"), pr.get("body"), paths)
        try:
            hits = search_anchors(owner, name, anchors)
        except RuntimeError as e:
            hits, skipped = [], skipped + [{"search": "anchor searches", "why": str(e)[:200]}]
        inferred = rank_candidates(pr.get("title"), hits, exclude, candidates)
    for entry in inferred:
        entry["relation"] = "inferred"
    # Sibling repositories, such as the one holding the test plans: their issues that match this pull
    # request by content are context for the reader, never candidates for a verdict here.
    sibling_matches = []
    if not no_search:
        for full_name in sibling_repos(root):
            so, sn = full_name.split("/", 1)
            recs = load_corpus(issues_root(so, sn))
            if not recs:
                continue
            spr = pr_for_scoring(pr)
            spr["changed_paths"] = paths
            spr["body"] = spr["body"] + "\n" + "\n".join(commit_messages)
            for hit in score_candidates(Index(recs), spr, set(), set(), longlist=SIBLING_MATCHES, cross_repo=True):
                sibling_matches.append({"repository": full_name, "number": hit["number"], "title": hit["title"],
                                        "state": hit["state"], "score": hit["score"], "why": hit["why"],
                                        "how": "found by content"})
    # The judge's own picks from the longlist, or any issue it names, read in full.
    for pick in reads:
        if pick in exclude or any(e["number"] == pick for e in inferred):
            continue
        from_long = next((e for e in longlist if e["number"] == pick), None)
        entry = dict(from_long) if from_long else {"number": pick, "title": None,
                                                   "state": None, "score": None, "signals": {}, "why": ""}
        entry["why"] = ("judge pick; " + entry.get("why", "")).rstrip("; ")
        entry["relation"] = "inferred"
        inferred.append(entry)

    numbers = set(linked) | set(referenced_by) | set(mentioned) | {e["number"] for e in inferred}
    details = fetch_issues(root, owner, name, numbers, refetch) if numbers else {}
    for table in (linked, referenced_by, mentioned):
        for n, entry in table.items():
            d = details.get(n) or {}
            entry["state"] = d.get("state", entry.get("state"))
            entry["title"] = d.get("title", entry.get("title"))
    for entry in inferred:
        d = details.get(entry["number"]) or {}
        entry["state"] = d.get("state", entry.get("state"))
        entry["title"] = d.get("title", entry.get("title"))

    dossier = {
        "dossier_version": DOSSIER_VERSION, "tool_version": TOOL_VERSION,
        "repository": f"{owner}/{name}",
        "gathered_at": datetime.now(timezone.utc).isoformat(timespec="seconds"),
        "pr": {
            "number": number, "title": pr.get("title"), "url": pr.get("url"),
            "state": pr.get("state"), "merged": pr.get("merged"), "draft": pr.get("isDraft"),
            "created_at": pr.get("createdAt"), "closed_at": pr.get("closedAt"),
            "author": (pr.get("author") or {}).get("login"), "base_ref": pr.get("baseRefName"),
            "body": triage.truncate(pr.get("body") or "", 3000)[0],
            "changed_paths": paths,
            "triage": record,
        },
        "linked": [linked[n] for n in sorted(linked)],
        "referenced_by": [referenced_by[n] for n in sorted(referenced_by)],
        "mentioned": [mentioned[n] for n in sorted(mentioned)],
        "inferred": inferred,
        "inferred_from": source,
        "longlist": [{k: e[k] for k in ("number", "title", "state", "score", "why")} for e in longlist],
        "anchors": anchors,
        "related_pull_requests": [mentioned_prs[n] for n in sorted(mentioned_prs)],
        "cross_repository": cross_refs,
        "sibling_matches": sibling_matches,
        "skipped": skipped,
        "issues": {str(n): details[n] for n in sorted(details)},
    }
    triage.write_json(dossier_path(root, number), dossier)
    return dossier


def judgment_status(dossier, judgment):
    """How an existing judgment fits a freshly gathered dossier: entries whose issue is no longer in
    front of the judge, and issues in front of the judge with no entry yet."""
    wanted = set(gathered_relations(dossier))
    judged = {triage.as_number(v.get("issue")) for v in judgment.get("issues", [])}
    judged |= {triage.as_number(u.get("issue") if isinstance(u, dict) else u) for u in judgment.get("unassessed", [])}
    judged.discard(None)
    return {"stale_entries": sorted(judged - wanted), "new_to_judge": sorted(wanted - judged)}


def gather(checkout, repo, pr, candidates, refetch, no_search, reads=()):
    if not pr:
        raise RuntimeError("name the pull request: gather --pr <n>")
    owner, name = resolve_repo(checkout, repo, [pr])
    root = issues_root(owner, name)
    number = checked_number(pr)[0]
    picks = [checked_number(r)[0] for r in reads or []]
    d = gather_one(root, owner, name, number, candidates, refetch, no_search, picks)
    tri = d["pr"]["triage"]
    out = {
        "repo": f"{owner}/{name}", "pr": number, "title": d["pr"]["title"], "pr_state": d["pr"]["state"],
        "dossier": str(dossier_path(root, number)),
        "triage": ({"verdict": tri.get("verdict"), "coverage": tri.get("coverage")} if tri.get("triaged")
                   else "not triaged: whether an issue is resolved by landed work cannot be said until it is"),
        "linked": [e["number"] for e in d["linked"]],
        "referenced_by": [e["number"] for e in d["referenced_by"]],
        "mentioned": [e["number"] for e in d["mentioned"]],
        "inferred": [{"issue": e["number"], "score": e["score"]} for e in d["inferred"]],
        "inferred_from": d["inferred_from"],
        "longlist": len(d["longlist"]),
        "related_pull_requests": [e["number"] for e in d["related_pull_requests"]],
        "cross_repository": len(d["cross_repository"]),
        "sibling_matches": [f"{m['repository']}#{m['number']}" for m in d["sibling_matches"]],
        "skipped": d["skipped"],
        "issues_to_judge": len(d["issues"]),
        "next": f"write {judgment_path(root, number)}, then: report --pr {number}",
    }
    if not no_search and not (corpus_dir(root) / "issues.jsonl").exists():
        out["note"] = ("no issue corpus for this repository yet, so the inferred tier used a few API searches. "
                       "Run `sync` once, about forty calls, to score every open issue on several signals instead.")
    existing = triage.read_json(judgment_path(root, number))
    if existing:
        # A second gather can move the read list; the judgment on disk then needs entries added or
        # dropped, and it is better said now than refused by report later.
        status = judgment_status(d, existing)
        if status["stale_entries"] or status["new_to_judge"]:
            status["next"] = "update the judgment: drop the stale entries, judge the new issues, then report"
        else:
            status["next"] = "the judgment still covers this dossier; report can run as is"
        out["existing_judgment"] = status
    print(json.dumps(out, indent=2))


# ---------------------------------------------------------------- judgment


def gathered_relations(dossier):
    """{issue number: relation} for every issue the dossier put in front of the judge."""
    out = {}
    for relation in RELATIONS:
        for entry in dossier.get(relation) or []:
            out.setdefault(entry["number"], relation)
    return out


def validate_issue_judgment(judgment, dossier):
    problems = []
    if dossier.get("dossier_version") != DOSSIER_VERSION:
        problems.append(f"dossier written by another version ({dossier.get('dossier_version')}); re-run gather")
        return problems
    number = dossier["pr"]["number"]
    if judgment.get("pr") is not None and int(str(judgment["pr"]).lstrip("#")) != number:
        problems.append(f"judgment says pr {judgment['pr']} but the dossier is #{number}")
    wanted = gathered_relations(dossier)
    pr_closes_by_coverage = (dossier["pr"]["triage"].get("triaged")
                             and dossier["pr"]["triage"].get("verdict") == "close")
    seen = set()
    for v in judgment.get("issues", []):
        if not isinstance(v, dict) or triage.as_number(v.get("issue")) is None:
            problems.append(f"entry {v!r} must be an object naming an issue")
            continue
        n = triage.as_number(v["issue"])
        if n in seen:
            problems.append(f"#{n}: appears more than once in issues; one entry per issue")
            continue
        seen.add(n)
        if n not in wanted:
            in_skim = any(e.get("number") == n for e in dossier.get("longlist") or [])
            problems.append(f"#{n}: judged but not in this dossier"
                            + (f"; it is on the skim list, so bring it in first: gather --pr {number} --read {n}" if in_skim else ""))
            continue
        details = (dossier.get("issues") or {}).get(str(n)) or {}
        if details.get("error"):
            problems.append(f"#{n}: could not be read ({details['error']}); list it under unassessed with that as the reason")
            continue
        verdict = v.get("verdict")
        if verdict not in VERDICTS:
            problems.append(f"#{n}: verdict {verdict!r} is not one of {list(VERDICTS)}")
        if verdict == "close" and not pr_closes_by_coverage and not v.get("disagreement_reason"):
            problems.append(f"#{n}: close needs the pull request itself to be closing as covered on the base branch; "
                            f"its triage verdict is {dossier['pr']['triage'].get('verdict') or 'missing'}. "
                            f"Use leave or unclear, or give a disagreement_reason.")
        if verdict in ("close", "leave", "related") and details.get("state") == "CLOSED":
            problems.append(f"#{n}: is already closed; the verdict for that is already-closed")
        if verdict == "already-closed" and details.get("state") == "OPEN":
            problems.append(f"#{n}: is open, so already-closed does not apply")
        if not (v.get("reason") or "").strip():
            problems.append(f"#{n}: no reason given. Every entry carries one.")
        if v.get("confidence") not in CONFIDENCES:
            problems.append(f"#{n}: confidence {v.get('confidence')!r} is not one of {list(CONFIDENCES)}")
        elif v["confidence"] != "high" and not v.get("falsifier"):
            problems.append(f"#{n}: confidence {v['confidence']} without a falsifier")
        for d in (v.get("duplicates") or []):
            if triage.as_number(d) is None:
                problems.append(f"#{n}: duplicate entry {d!r} does not name an issue")
            elif triage.as_number(d) == n:
                problems.append(f"#{n}: lists itself as a duplicate")
    unassessed = set()
    for u in judgment.get("unassessed", []):
        # A bare number would satisfy completeness and then vanish from the report; the entry has
        # to say which issue and why, so the report can carry it under Could not determine.
        if not isinstance(u, dict) or triage.as_number(u.get("issue")) is None or not (u.get("reason") or "").strip():
            problems.append(f"unassessed entry {u!r} must be an object with an issue number and a reason")
            continue
        unassessed.add(triage.as_number(u["issue"]))
    for n in sorted(set(wanted) - seen - unassessed):
        problems.append(f"#{n}: in the dossier but has no verdict and is not listed unassessed")
    return problems


TOOL_TALK = re.compile(
    r"\b(dossiers?|corpus|longlist|skim list|anchors?|signals?|tiers?|the tool|inferred tier|candidates?)\b", re.I)


def lint_issue_judgment(judgment):
    """Soft findings: reasons that talk about the tool instead of the issue."""
    found = []
    for v in judgment.get("issues", []):
        m = TOOL_TALK.search(v.get("reason") or "")
        if m:
            found.append(f"#{v.get('issue')}: {m.group(0)!r}")
    return found


def judged_prs(root):
    out = []
    for path in sorted(root.glob("pr-*.judgment.json")):
        number = int(re.match(r"pr-(\d+)\.judgment\.json", path.name).group(1))
        dossier, judgment = triage.read_json(dossier_path(root, number)), triage.read_json(path)
        if dossier and judgment:
            out.append((number, dossier, judgment))
    return out


def fold_state(root):
    """issues/state.json as a projection of every judgment on disk: newest finding per issue."""
    state = {}
    for number, dossier, judgment in sorted(judged_prs(root), key=lambda t: t[1].get("gathered_at", "")):
        relations = gathered_relations(dossier)
        for v in judgment.get("issues", []):
            n = triage.as_number(v.get("issue"))
            if n is None:
                continue
            state[str(n)] = {"verdict": v.get("verdict"), "via_pr": number,
                             "relation": relations.get(n), "at": dossier.get("gathered_at", "")[:10],
                             "judged_by": judgment.get("judged_by")}
    with triage.locked(root / "state.json"):
        triage.write_json(root / "state.json", state)
    return state


def issue_link(n, details, full):
    url = (details or {}).get("url") or f"https://github.com/{full}/issues/{n}"
    return f"[#{n}]({url})"


UNLINKED_MARK = "\U0001F50D"      # a magnifier: found by content, not by a GitHub link


def linkify(text, full):
    """Every bare #number in a reason becomes a link. GitHub redirects /issues/N to the pull request
    when N is one, so one form serves both. Numbers already inside a link are left alone."""
    return re.sub(r"(?<![\w/\[(])#(\d{2,7})\b", lambda m: f"[#{m.group(1)}](https://github.com/{full}/issues/{m.group(1)})", text or "")


def entry_line(n, details, reason, full, unlinked=False):
    title = (details.get("title") or "").strip()
    state = details.get("state") or "?"
    if details.get("state_reason"):
        state += f", {details['state_reason'].lower().replace('_', ' ')}"
    mark = f"{UNLINKED_MARK} " if unlinked else ""
    head = f"- {mark}{issue_link(n, details, full)} **{title}** ({state})" if title else f"- {mark}{issue_link(n, details, full)} ({state})"
    return f"{head}: {linkify(reason.strip(), full)}" if reason.strip() else head


def plural(count, one, many):
    return one if count == 1 else many


def render(root, full, number):
    """The report for one pull request the triage found redundant: the issues that should be closed with
    it. Its own state does not matter to that; the header only records it. Rendering never rewrites dossiers."""
    dossier, judgment = triage.read_json(dossier_path(root, number)), triage.read_json(judgment_path(root, number))
    if not (dossier and judgment):
        raise RuntimeError(f"#{number} is not judged yet. Gather it and write its judgment first.")
    relations = gathered_relations(dossier)
    details_all = dossier.get("issues") or {}
    pr, tri = dossier["pr"], dossier["pr"].get("triage") or {}
    url = pr.get("url") or f"https://github.com/{full}/pull/{number}"
    effort = (judgment.get("judged_effort") or "").strip().lower()

    heading_title = re.sub(r"([\[\]])", r"\\\1", (pr.get("title") or "").strip())
    closing = bool(tri.get("triaged") and tri.get("verdict") == "close")
    lines = [f"# Issues Related to PR [#{number} {heading_title}]({url})", "",
             ("*These issues are related to the pull request and can be safely closed.*" if closing else
              "*These issues are related to the pull request: what each asks for, and where the pull request's work stands against it.*"), "",
             "*The results also cover related issues that are not internally linked on GitHub.*", ""]
    # Provenance only. The pull request's own verdict and state drive no action here: every entry
    # below carries its reason, and the verdict was verified before this report was asked for.
    lines += ["| | |", "|---|---|", f"| **Run date** | {datetime.now(timezone.utc).strftime('%Y-%m-%d')} |"]
    if judgment.get("judged_by"):
        lines.append(f"| **Judged by** | `{judgment['judged_by']}`, {effort if effort in EFFORT_LEVELS else 'unknown'} effort |")
    lines.append("")
    lines += DISCLAIMER

    counts, to_close, leave, closed, unclear, rejected = {}, [], [], [], [], 0
    for v in judgment.get("issues", []):
        n = triage.as_number(v.get("issue"))
        if n is None or n not in relations:
            continue
        counts[v.get("verdict")] = counts.get(v.get("verdict"), 0) + 1
        item = (n, details_all.get(str(n)) or {"title": "", "state": "?"}, v, relations[n])
        verdict = v.get("verdict")
        if verdict == "close":
            to_close.append(item)
        elif verdict in ("leave", "related"):
            leave.append(item)
        elif verdict == "already-closed":
            closed.append(item)
        elif verdict == "unclear":
            unclear.append(item)
        elif relations[n] == "inferred":
            rejected += 1
    for u in judgment.get("unassessed", []):
        n = triage.as_number(u.get("issue")) if isinstance(u, dict) else None
        if n is not None and n in relations:
            counts["unassessed"] = counts.get("unassessed", 0) + 1
            unclear.append((n, details_all.get(str(n)) or {"title": "", "state": "?"},
                            {"reason": f"could not be assessed: {(u.get('reason') or '').strip()}"}, relations[n]))
    counts["inferred_rejected"] = rejected

    if closing or to_close:
        lines += ["### Safe to close", ""]
        if not to_close:
            lines += ["No issues addressed by this pull request were found.", ""]
    any_unlinked = False
    for n, d, v, relation in sorted(to_close, key=lambda t: t[0]):
        reason = (v.get("reason") or "").strip()
        dups = [triage.as_number(x) for x in (v.get("duplicates") or []) if triage.as_number(x) is not None]
        if dups:
            reason += ("; " if reason else "") + "same ask as " + \
                ", ".join(issue_link(x, details_all.get(str(x)), full) for x in sorted(dups))
        any_unlinked |= relation == "inferred"
        lines.append(entry_line(n, d, reason, full, unlinked=relation == "inferred"))
    if to_close:
        lines.append("")

    related_title = "Related but not resolved by the PR" if closing else "Related"
    related_line = ("Related to this pull request, on GitHub or by content, but what they ask for isn't resolved by the PR."
                    if closing else "Related to this pull request, on GitHub or by content. Each line says what the issue asks for and where the pull request's work stands.")
    for title, standfirst, items in (
            (related_title, related_line, leave),
            ("Already closed", None, closed),
            ("Could not determine", "Each names the check a person would do next.", unclear)):
        if not items and not (title == related_title and not closing):
            continue
        if not items:
            lines += [f"### {title}", "", "No related issues were found.", ""]
            continue
        lines += [f"### {title}", ""]
        if standfirst:
            lines += [f"*{standfirst}*", ""]
        for n, d, v, relation in sorted(items, key=lambda t: t[0]):
            any_unlinked |= relation == "inferred"
            lines.append(entry_line(n, d, "" if title == "Already closed" else (
                v.get("reason") or ""), full, unlinked=relation == "inferred"))
        lines.append("")
    elsewhere, seen_elsewhere = [], set()
    for ref in list(dossier.get("cross_repository") or []) + list(dossier.get("sibling_matches") or []):
        key = (ref.get("repository"), ref.get("number"))
        if key in seen_elsewhere or not all(key):
            continue
        seen_elsewhere.add(key)
        elsewhere.append(ref)
    if elsewhere:
        # Not judged: a test-plan or specification issue is resolved in its own repository, but the
        # reader should know the pull request points there, or that the same test is discussed there.
        lines += ["### Referenced elsewhere", "",
                  "*Issues and pull requests in other repositories that this pull request cites, that cite it, or that match it by content. Listed for context, not judged here.*", ""]
        for ref in sorted(elsewhere, key=lambda r: (r["repository"], r["number"])):
            kind = "pull request" if ref.get("type") == "PullRequest" else "issue"
            title = (ref.get("title") or "").strip()
            state = (ref.get("state") or "?").lower()
            url = f"https://github.com/{ref['repository']}/issues/{ref['number']}"
            by_content = ref.get("how") == "found by content"
            any_unlinked |= by_content
            mark = f"{UNLINKED_MARK} " if by_content else ""
            lines.append(f"- {mark}[{ref['repository']}#{ref['number']}]({url}) **{title}** ({kind}, {state})" if title
                         else f"- {mark}[{ref['repository']}#{ref['number']}]({url}) ({kind}, {state})")
        lines.append("")
    if any_unlinked:
        lines += [f"*{UNLINKED_MARK} marks an issue that is not internally linked on GitHub and was found by content.*", ""]
    text = "\n".join(lines) + "\n"
    return text, {"pr": number, "counts": counts, "to_close": [n for n, _, _, _ in to_close]}


def report_path(owner, name, number):
    """The report, in the shared reports folder, named for what it holds: the issues related to a PR."""
    return triage.reports_root(owner, name) / f"pr-{number}-related-issues.md"


def report(checkout, repo, pr, strict, out):
    if not pr:
        raise RuntimeError("name the pull request: report --pr <n>")
    owner, name = resolve_repo(checkout, repo, [pr])
    root = issues_root(owner, name)
    full = f"{owner}/{name}"
    number = checked_number(pr)[0]
    dossier, judgment = triage.read_json(dossier_path(root, number)), triage.read_json(judgment_path(root, number))
    if not dossier:
        raise RuntimeError(f"no dossier for #{number}. Run: gather --pr {number}")
    if not judgment:
        raise RuntimeError(f"no judgment for #{number}. Write {judgment_path(root, number)} first.")
    problems = validate_issue_judgment(judgment, dossier)
    if problems and strict:
        print(json.dumps({"error": "invalid_judgment", "problems": problems}, indent=2))
        sys.exit(1)
    if not problems:
        fold_state(root)
    text, stats = render(root, full, number)
    path = report_path(owner, name, number)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8")
    saved = triage.save_copy(out, path.name, text)
    out_json = {"repo": full, "pull_request": number, "rendered": saved or str(path), "kept_at": str(path),
                "to_close": stats["to_close"], "judged": stats["counts"], "validation_problems": problems}
    lint = lint_issue_judgment(judgment)
    if lint:
        out_json["reasons_mention_tooling"] = lint
    print(json.dumps(out_json, indent=2))


def list_prs(checkout, repo, from_triage):
    """What has been gathered and judged, and, with --from-triage, what the triage report offers."""
    owner, name = resolve_repo(checkout, repo, [])
    root = issues_root(owner, name)
    rows = {}
    for path in root.glob("pr-*.json"):
        m = re.match(r"pr-(\d+)\.json", path.name)
        if m:
            n = int(m.group(1))
            rows[n] = {"pr": n, "gathered": True, "judged": judgment_path(root, n).exists(),
                       "report": str(report_path(owner, name, n)) if report_path(owner, name, n).exists() else None}
    offered = []
    if from_triage:
        for n in from_triage_numbers(owner, name, from_triage):
            offered.append({"pr": n, "gathered": n in rows, "judged": rows.get(n, {}).get("judged", False)})
    out = {"repo": f"{owner}/{name}", "done": [rows[n] for n in sorted(rows)],
           "corpus": str(corpus_dir(root) / "issues.jsonl") if (corpus_dir(root) / "issues.jsonl").exists() else None,
           "siblings": sibling_repos(root)}
    if from_triage:
        out["from_triage"] = {"verdicts": list(from_triage), "pull_requests": offered,
                              "not_yet_done": [o["pr"] for o in offered if not o["judged"]]}
    print(json.dumps(out, indent=2))


def forget(checkout, repo, pr, yes):
    if not pr:
        raise RuntimeError("name the pull request to forget: forget --pr <n>")
    owner, name = resolve_repo(checkout, repo, [pr])
    root = issues_root(owner, name)
    number = checked_number(pr)[0]
    doomed = [p for p in (dossier_path(root, number), judgment_path(root, number), report_path(owner, name, number)) if p.exists()]
    if not yes:
        print(json.dumps({"would_remove": [str(p) for p in doomed], "issue_cache_kept": True,
                          "next": "Re-run with --yes to do it."}, indent=2))
        return
    for p in doomed:
        p.unlink()
    state = fold_state(root)
    print(json.dumps({"removed": [str(p) for p in doomed], "issues_in_state": len(state)}, indent=2))


def cost(checkout, repo, pr):
    """Time and API calls per command, from the sibling's own log. Never in the report."""
    owner, name = resolve_repo(checkout, repo, [pr] if pr else [])
    root = issues_root(owner, name)
    path = root / "cost.jsonl"
    rows = []
    if path.exists():
        with path.open(encoding="utf-8") as fh:
            for line in fh:
                line = line.strip()
                if line:
                    try:
                        rows.append(json.loads(line))
                    except json.JSONDecodeError:
                        continue
    if pr:
        want = f"pr-{checked_number(pr)[0]}"
        rows = [r for r in rows if r.get("run") == want]
    by_command = {}
    for r in rows:
        c = by_command.setdefault(r.get("command"), {"runs": 0, "seconds": 0.0,
                                  "graphql_calls": 0, "rest_calls": 0, "rate_limit_waits": 0})
        c["runs"] += 1
        c["seconds"] = round(c["seconds"] + (r.get("seconds") or 0), 1)
        for k in ("graphql_calls", "rest_calls", "rate_limit_waits"):
            c[k] += r.get(k) or 0
    total = {"seconds": round(sum(r.get("seconds") or 0 for r in rows), 1),
             "graphql_calls": sum(r.get("graphql_calls") or 0 for r in rows),
             "rest_calls": sum(r.get("rest_calls") or 0 for r in rows)}
    print(json.dumps({"repo": f"{owner}/{name}", "pull_request": checked_number(pr)[0] if pr else None,
                      "entries": len(rows), "total": total, "by_command": by_command,
                      "log": str(path)}, indent=2))


def preflight(checkout, repo):
    problems = []
    if not shutil.which("gh"):
        problems.append({"item": "gh", "fix": "install the GitHub CLI, then: gh auth login"})
    else:
        try:
            triage.run(["gh", "auth", "status"], check=True)
        except RuntimeError as e:
            problems.append({"item": "gh auth", "fix": "gh auth login", "detail": str(e)[:200]})
    owner = name = None
    try:
        owner, name = resolve_repo(checkout, repo, [])
    except RuntimeError as e:
        problems.append({"item": "repository", "fix": "run inside a clone, or pass --repo owner/name", "detail": str(e)})
    triage_cache = None
    if owner:
        state = triage.cache_root(owner, name) / "state.json"
        triage_cache = str(state) if state.exists() else None
    rate = None
    if not problems:
        try:
            limits = triage.gh_rest("/rate_limit").get("resources") or {}
            rate = {k: (limits.get(k) or {}).get("remaining") for k in ("graphql", "search", "core")}
            low = [k for k, v in rate.items() if v is not None and v < (10 if k == "search" else 200)]
            if low:
                problems.append({"item": "rate limit", "fix": f"wait for the {', '.join(low)} limit to reset", "detail": rate})
        except RuntimeError as e:
            rate = {"error": str(e)[:200]}
    out = {"ok": not problems, "repo": f"{owner}/{name}" if owner else None,
           "triage_state": triage_cache, "rate_remaining": rate,
           "note": (None if triage_cache else
                    "matter-pr-triage has not triaged this repository on this machine; the linked and inferred "
                    "tiers still work, but no issue can be marked as closing with its pull request until it has"),
           "problems": problems, "version": TOOL_VERSION, "pr_triage_version": triage.TOOL_VERSION}
    print(json.dumps(out, indent=2))
    sys.exit(0 if not problems else 1)


# ---------------------------------------------------------------- cli


def build_parser():
    p = argparse.ArgumentParser(prog="matter_pr_triage_issues.py",
                                description="The issues tied to a pull request, and what closing it does to them.")
    p.add_argument("--version", action="version", version=f"matter_pr_triage_issues {TOOL_VERSION}")
    sub = p.add_subparsers(dest="command", required=True)

    def common(sp, with_prs=True):
        sp.add_argument("--checkout", default=".", help="Clone of the target repository, for inferring it.")
        sp.add_argument("--repo", default=None, help="owner/name or a github.com URL. Inferred from the clone otherwise.")
        if with_prs:
            sp.add_argument("--pr", dest="pr", default=None, metavar="N",
                            help="The pull request: 40191, #40191 or its URL.")
        return sp

    common(sub.add_parser("preflight", help="Check gh, access and whether the repository was triaged."), with_prs=False)
    g = common(sub.add_parser("gather", help="Collect the issues tied to one pull request."))
    g.add_argument("--candidates", type=int, default=DEFAULT_CANDIDATES,
                   help=f"How many inferred candidates to keep per pull request. Default {DEFAULT_CANDIDATES}.")
    g.add_argument("--refetch", action="store_true", help="Ignore the per-issue cache and fetch every issue again.")
    g.add_argument("--no-search", dest="no_search", action="store_true",
                   help="Skip the inferred tier: only issues linked, referencing or mentioned are gathered.")
    g.add_argument("--read", dest="reads", action="append", default=[], metavar="ISSUE",
                   help="An issue to read in full as an inferred candidate, a judge's pick from the skim list. Repeatable.")
    sy = common(sub.add_parser("sync", help="Build or refresh the repository's issue corpus."), with_prs=False)
    sy.add_argument("--full", action="store_true", help="Rebuild from scratch instead of fetching what changed since the last sync.")
    sy.add_argument("--sibling", dest="siblings", action="append", default=[], metavar="OWNER/REPO",
                    help="Also keep a corpus of this repository's issues, for example the one holding the test plans, named as owner/name or as the path of a clone; its matches appear as context. Remembered for later syncs. Repeatable.")
    sy.add_argument("--closed-months", dest="closed_months", type=int, default=CORPUS_CLOSED_MONTHS,
                    help=f"How far back closed issues are kept. Default {CORPUS_CLOSED_MONTHS}.")
    b = common(sub.add_parser("benchmark", help="Measure the inferred tier's recall against pull requests with formal issue links."), with_prs=False)
    b.add_argument("--sample", type=int, default=60, help="How many linked pull requests to score. Default 60.")
    b.add_argument("--seed", type=int, default=1, help="Sampling seed, so a run can be repeated. Default 1.")
    b.add_argument("--k", type=int, default=DEFAULT_CANDIDATES,
                   help=f"The read-list size recall is measured at. Default {DEFAULT_CANDIDATES}.")
    b.add_argument("--keep-mentions", dest="keep_mentions", action="store_true",
                   help="Leave the linked issue numbers in the pull request text instead of masking them.")
    r = common(sub.add_parser("report", help="Validate the judgment and render the pull request's issue report."))
    r.add_argument("--out", default=None, metavar="PATH", help="Also save a copy here, a file or a directory.")
    r.add_argument("--no-strict", dest="strict", action="store_false", default=True,
                   help="Render even when a judgment fails validation.")
    li = common(sub.add_parser("list", help="What has been gathered and judged."), with_prs=False)
    li.add_argument("--from-triage", dest="from_triage", action="append", default=[], metavar="VERDICT",
                    help="Also list the pull requests the triage report gave this verdict and which are not done yet.")
    common(sub.add_parser("cost", help="What the commands spent: time and API calls, from the log; --pr narrows to one pull request."))
    f = common(sub.add_parser("forget", help="Discard the findings for one pull request."))
    f.add_argument("--yes", action="store_true", help="Actually remove. Without it, only says what would go.")
    return p


def main():
    args = build_parser().parse_args()
    kwargs = {k: v for k, v in vars(args).items() if k != "command"}
    handler = {"preflight": preflight, "gather": gather, "report": report,
               "list": list_prs, "forget": forget, "sync": sync, "benchmark": benchmark, "cost": cost}[args.command]
    started = time.time()
    try:
        handler(**kwargs)
    except RuntimeError as e:
        print(json.dumps({"error": str(e)}, indent=2))
        sys.exit(1)
    except (OSError, subprocess.SubprocessError) as e:
        print(json.dumps({"error": f"{type(e).__name__}: {e}"}, indent=2))
        sys.exit(1)
    except KeyboardInterrupt:
        print(json.dumps({"error": "interrupted. Re-run the same command to resume."}, indent=2))
        sys.exit(130)
    finally:
        try:
            pr = kwargs.get("pr")
            owner, name = resolve_repo(kwargs.get("checkout") or ".", kwargs.get("repo"), [pr] if pr else [])
            triage.log_cost(issues_root(owner, name), f"issues {args.command}",
                            f"pr-{checked_number(pr)[0]}" if pr else None, time.time() - started)
        except Exception:
            pass                       # bookkeeping never breaks a command


if __name__ == "__main__":
    main()
