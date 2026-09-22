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
"""Unit tests for matter_pr_triage_issues: everything that needs no network.

Run with:  python3 -m unittest discover scripts/tools/pr_triage/tests
"""
import contextlib
import io
import json
import os
import pathlib
import sys
import tempfile
import unittest

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent.parent))
import matter_pr_triage_issues as mi  # noqa: E402 isort:skip


OWNER, NAME = "project-chip", "connectedhomeip"


def dossier(pr=40191, verdict="close", issues=None, tiers=None):
    """A dossier as gather writes it, with the parts the judge and the renderer read."""
    issues = issues or {}
    tiers = tiers or {}
    return {
        "dossier_version": mi.DOSSIER_VERSION, "tool_version": mi.TOOL_VERSION,
        "repository": f"{OWNER}/{NAME}", "gathered_at": "2026-09-21T10:00:00+00:00",
        "pr": {"number": pr, "title": "Fix crash", "url": f"https://github.com/{OWNER}/{NAME}/pull/{pr}",
               "state": "CLOSED", "changed_paths": ["src/app/ReadHandler.cpp"],
               "triage": ({"triaged": True, "verdict": verdict, "coverage": "covered_master", "reason": "landed"}
                          if verdict else {"triaged": False})},
        "linked": tiers.get("linked", []), "referenced_by": tiers.get("referenced_by", []),
        "mentioned": tiers.get("mentioned", []), "inferred": tiers.get("inferred", []),
        "anchors": [], "related_pull_requests": [], "cross_repository": [], "skipped": [],
        "issues": {str(k): v for k, v in issues.items()},
    }


def issue(n, state="OPEN", title="An issue", reason=None):
    return {"number": n, "title": title, "state": state, "state_reason": reason, "labels": [],
            "attached_pull_requests": [], "tracking_issue_hint": False}


def entry(n, verdict, reason="because", confidence="high", **extra):
    e = {"issue": n, "verdict": verdict, "reason": reason, "confidence": confidence}
    e.update(extra)
    return e


class Numbers(unittest.TestCase):
    def test_pasted_forms(self):
        self.assertEqual(mi.checked_number("40191"), (40191, None))
        self.assertEqual(mi.checked_number("#40191"), (40191, None))
        self.assertEqual(mi.checked_number(f"https://github.com/{OWNER}/{NAME}/pull/40191"), (40191, (OWNER, NAME)))
        self.assertEqual(mi.checked_number(f"https://github.com/{OWNER}/{NAME}/pull/40191/files"), (40191, (OWNER, NAME)))

    def test_rejects_the_rest(self):
        for bad in ("", "abc", "40191/40192", f"https://github.com/{OWNER}/{NAME}/issues/40190"):
            with self.assertRaises(RuntimeError):
                mi.checked_number(bad)

    def test_repo_from_pr_url_when_not_given(self):
        self.assertEqual(mi.resolve_repo(".", None, [f"https://github.com/{OWNER}/{NAME}/pull/1"]), (OWNER, NAME))
        self.assertEqual(mi.resolve_repo(".", "a/b", [f"https://github.com/{OWNER}/{NAME}/pull/1"]), ("a", "b"))


class Mentions(unittest.TestCase):
    def test_same_repo_forms_and_own_number_dropped(self):
        text = (f"Fixes #40190, see #40191 and https://github.com/{OWNER}/{NAME}/issues/38040 "
                f"and {OWNER}/{NAME}#12345 but not a#1 or path/#2")
        m = mi.extract_mentions(text, OWNER, NAME, own_number=40191)
        self.assertEqual(m["same"], {40190, 38040, 12345})
        self.assertEqual(m["cross"], {})

    def test_cross_repo_kept_apart(self):
        text = "spec issue project-chip/connectedhomeip-spec#123 and https://github.com/CHIP-Specifications/chip-test-plans/pull/77"
        m = mi.extract_mentions(text, OWNER, NAME, own_number=1)
        self.assertEqual(m["same"], set())
        self.assertEqual(m["cross"], {("project-chip", "connectedhomeip-spec"): {123},
                                      ("CHIP-Specifications", "chip-test-plans"): {77}})

    def test_repo_match_ignores_case(self):
        m = mi.extract_mentions("https://github.com/Project-Chip/ConnectedHomeIP/issues/5", OWNER, NAME, 1)
        self.assertEqual(m["same"], {5})


class Anchors(unittest.TestCase):
    def test_test_ids_normalise_both_spellings(self):
        ids = mi.test_ids("[E2E] JFDS Tests 2.3", "covers TC-JFDS-2.3 and TC_JFDS_2_4", "src/python_testing/TC_DGGEN_2_1.py")
        self.assertEqual(ids, {"TC-JFDS-2.3", "TC-JFDS-2.4", "TC-DGGEN-2.1"})

    def test_basenames_keep_extension_and_skip_generic(self):
        paths = ["src/app/ReadHandler.cpp", "BUILD.gn", "a/README.md", "x/short.py",
                 "src/python_testing/TC_DGGEN_2_1.py", "src/app/ReadHandler.cpp"]
        self.assertEqual(mi.distinctive_basenames(paths), ["ReadHandler.cpp"])

    def test_anchor_order_and_title_search_is_title_only(self):
        anchors = mi.anchors_for("Fix crash when session is replaced before priming reports are sent",
                                 "Fixes TC-ACL-2.3", ["src/app/ReadHandler.cpp"])
        kinds = [a["kind"] for a in anchors]
        self.assertEqual(kinds, ["test_id", "file", "title", "title"])
        self.assertEqual(anchors[0]["terms"], '"TC-ACL-2.3"')
        self.assertEqual(anchors[1]["terms"], '"ReadHandler.cpp"')
        for a in anchors[2:]:
            self.assertTrue(a["terms"].endswith(" in:title"))
        self.assertLess(len(anchors[3]["terms"].split()), len(anchors[2]["terms"].split()))

    def test_short_titles_get_fewer_anchors(self):
        self.assertEqual([a["kind"] for a in mi.anchors_for("Fix", "", [])], [])
        self.assertEqual([a["kind"] for a in mi.anchors_for("Priming reports", "", [])], ["title"])


class Ranking(unittest.TestCase):
    def hits(self):
        file_anchor, title_anchor = {"kind": "file"}, {"kind": "title"}
        return [
            (file_anchor, [{"number": 1, "title": "[ReadHandler] Reduce RAM usage"},
                           {"number": 2, "title": "Crash when session replaced during priming"},
                           {"number": 9, "title": "Excluded"}]),
            (title_anchor, [{"number": 2, "title": "Crash when session replaced during priming"},
                            {"number": 3, "title": "Session replaced too early"}]),
        ]

    def test_component_sweep_is_dropped_and_multi_anchor_ranks_first(self):
        ranked = mi.rank_candidates("Fix crash when session is replaced before priming reports", self.hits(), {9}, 8)
        numbers = [r["number"] for r in ranked]
        self.assertEqual(numbers[0], 2)                   # two anchors plus shared words
        self.assertIn(3, numbers)                          # a title hit always earns a read
        self.assertNotIn(1, numbers)                       # file-only hit with no words in common
        self.assertNotIn(9, numbers)                       # excluded (linked already)

    def test_limit(self):
        ranked = mi.rank_candidates("Fix crash when session is replaced before priming reports", self.hits(), set(), 1)
        self.assertEqual([r["number"] for r in ranked], [2])


class Relations(unittest.TestCase):
    def test_first_tier_wins_when_an_issue_is_in_two(self):
        d = dossier(tiers={"linked": [{"number": 5}], "inferred": [{"number": 5}, {"number": 6}]})
        self.assertEqual(mi.gathered_relations(d), {5: "linked", 6: "inferred"})


class CompactIssue(unittest.TestCase):
    def node(self, attached_open=0, labels=(), refs=()):
        return {"number": 7, "title": "T", "url": "u", "state": "OPEN", "stateReason": None,
                "createdAt": "2025-01-01T00:00:00Z", "closedAt": None, "body": "b" * 3000,
                "author": {"login": "x"}, "labels": {"nodes": [{"name": name} for name in labels]},
                "comments": {"totalCount": 3},
                "closedByPullRequestsReferences": {"nodes": [{"number": 100 + i, "state": "OPEN", "title": "p"}
                                                             for i in range(attached_open)]},
                "timelineItems": {"nodes": [{"source": {"__typename": "PullRequest", "number": n, "state": "MERGED",
                                                        "title": "q", "repository": {"nameWithOwner": f"{OWNER}/{NAME}"}}}
                                            for n in refs]}}

    def test_attached_from_both_sources_deduped_and_body_capped(self):
        c = mi.compact_issue(self.node(attached_open=1, refs=(100, 200)), OWNER, NAME)
        self.assertEqual([(a["number"], a["how"]) for a in c["attached_pull_requests"]],
                         [(100, "closing_reference"), (200, "mentions_issue")])
        self.assertTrue(c["body_truncated"])
        self.assertFalse(c["tracking_issue_hint"])

    def test_tracking_hint_by_count_and_by_label(self):
        self.assertTrue(mi.compact_issue(self.node(attached_open=mi.HUB_ATTACHED_PRS + 1), OWNER, NAME)["tracking_issue_hint"])
        self.assertTrue(mi.compact_issue(self.node(labels=("Epic",)), OWNER, NAME)["tracking_issue_hint"])

    def test_other_repository_references_are_not_attached(self):
        node = self.node()
        node["timelineItems"]["nodes"] = [{"source": {"__typename": "PullRequest", "number": 1, "state": "OPEN",
                                                      "title": "q", "repository": {"nameWithOwner": "a/b"}}}]
        self.assertEqual(mi.compact_issue(node, OWNER, NAME)["attached_pull_requests"], [])


class Validation(unittest.TestCase):
    def setUp(self):
        self.d = dossier(issues={40190: issue(40190), 37233: issue(37233, "CLOSED", reason="COMPLETED"), 8: issue(8)},
                         tiers={"linked": [{"number": 40190}, {"number": 37233}], "inferred": [{"number": 8}]})

    def judgment(self, *entries, unassessed=()):
        return {"pr": 40191, "issues": list(entries), "unassessed": list(unassessed)}

    def test_clean_passes(self):
        j = self.judgment(entry(40190, "close"), entry(37233, "already-closed"),
                          entry(8, "unrelated", confidence="medium", falsifier="f"))
        self.assertEqual(mi.validate_issue_judgment(j, self.d), [])
        j = self.judgment(entry(40190, "leave"), entry(37233, "already-closed"),
                          entry(8, "unclear", confidence="low", falsifier="f"))
        self.assertEqual(mi.validate_issue_judgment(j, self.d), [])

    def test_every_gathered_issue_accounted_for(self):
        problems = mi.validate_issue_judgment(self.judgment(entry(40190, "close")), self.d)
        self.assertTrue(any("#37233" in p and "no verdict" in p for p in problems))
        self.assertTrue(any("#8" in p for p in problems))
        ok = self.judgment(entry(40190, "close"), entry(37233, "already-closed"), unassessed=[{"issue": 8, "reason": "r"}])
        self.assertEqual(mi.validate_issue_judgment(ok, self.d), [])

    def test_unknown_issue_and_bad_enums(self):
        problems = mi.validate_issue_judgment(self.judgment(entry(999, "close")), self.d)
        self.assertTrue(any("#999" in p and "not in this dossier" in p for p in problems))
        problems = mi.validate_issue_judgment(self.judgment(entry(40190, "closed")), self.d)
        self.assertTrue(any("verdict 'closed'" in p for p in problems))
        problems = mi.validate_issue_judgment(self.judgment(entry(40190, "close", confidence="sure")), self.d)
        self.assertTrue(any("confidence 'sure'" in p for p in problems))

    def test_close_needs_the_pr_to_close_by_coverage(self):
        d = dossier(verdict="keep", issues={40190: issue(40190)}, tiers={"linked": [{"number": 40190}]})
        problems = mi.validate_issue_judgment(self.judgment(entry(40190, "close")), d)
        self.assertTrue(any("close needs the pull request" in p for p in problems))
        with_reason = self.judgment(entry(40190, "close", disagreement_reason="author confirmed it landed"))
        self.assertEqual(mi.validate_issue_judgment(with_reason, d), [])
        untriaged = dossier(verdict=None, issues={40190: issue(40190)}, tiers={"linked": [{"number": 40190}]})
        problems = mi.validate_issue_judgment(self.judgment(entry(40190, "close")), untriaged)
        self.assertTrue(any("missing" in p for p in problems))

    def test_closed_state_and_verdict_must_agree(self):
        problems = mi.validate_issue_judgment(self.judgment(entry(37233, "close"), entry(40190, "already-closed"),
                                                            entry(8, "unrelated")), self.d)
        self.assertTrue(any("#37233" in p and "already closed" in p for p in problems))
        self.assertTrue(any("#40190" in p and "is open" in p for p in problems))

    def test_reason_falsifier_and_duplicates(self):
        j = self.judgment(entry(40190, "close", reason="  "), entry(37233, "already-closed", confidence="low"),
                          entry(8, "unrelated", duplicates=[8, "x"]))
        problems = mi.validate_issue_judgment(j, self.d)
        self.assertTrue(any("#40190" in p and "no reason" in p for p in problems))
        self.assertTrue(any("#37233" in p and "without a falsifier" in p for p in problems))
        self.assertTrue(any("lists itself" in p for p in problems))
        self.assertTrue(any("duplicate entry 'x'" in p for p in problems))

    def test_dossier_version_mismatch_refuses(self):
        d = dict(self.d, dossier_version=mi.DOSSIER_VERSION + 1)
        problems = mi.validate_issue_judgment(self.judgment(), d)
        self.assertEqual(len(problems), 1)
        self.assertIn("re-run gather", problems[0])

    def test_wrong_pr_in_judgment(self):
        j = {"pr": 1, "issues": [entry(40190, "close"), entry(37233, "already-closed"), entry(8, "unrelated")]}
        self.assertTrue(any("judgment says pr 1" in p for p in mi.validate_issue_judgment(j, self.d)))


class RenderAndState(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        os.environ["MATTER_PR_TRIAGE_CACHE"] = self.tmp.name
        os.environ["MATTER_PR_TRIAGE_REPORTS"] = os.path.join(self.tmp.name, "reports")
        self.root = mi.issues_root(OWNER, NAME)
        self.root.mkdir(parents=True)

    def tearDown(self):
        os.environ.pop("MATTER_PR_TRIAGE_CACHE", None)
        os.environ.pop("MATTER_PR_TRIAGE_REPORTS", None)
        self.tmp.cleanup()

    def write(self, pr, d, j):
        mi.triage.write_json(mi.dossier_path(self.root, pr), d)
        mi.triage.write_json(mi.judgment_path(self.root, pr), j)

    def two_prs(self):
        d1 = dossier(40191, issues={40190: issue(40190, title="Crash"), 5: issue(5, title="Sweep hit"),
                                    6: issue(6, title="Same crash, other words")},
                     tiers={"linked": [{"number": 40190}], "inferred": [{"number": 5}, {"number": 6}]})
        j1 = {"pr": 40191, "judged_by": "model-a", "judged_effort": "high", "issues": [
            entry(40190, "close", "guarded on master", duplicates=[6]),
            entry(5, "unrelated", "asks for RAM savings"),
            entry(6, "close", "same crash", confidence="medium", falsifier="f", duplicates=[40190])]}
        d2 = dossier(39685, issues={38040: issue(38040, title="Coverage opener"), 37233: issue(37233, "CLOSED", "Epic", "COMPLETED"),
                                    77: issue(77, title="Has another PR")},
                     tiers={"linked": [{"number": 38040}, {"number": 37233}], "mentioned": [{"number": 77}]})
        d2["gathered_at"] = "2026-09-21T11:00:00+00:00"
        j2 = {"pr": 39685, "judged_by": "model-a", "judged_effort": "nonsense", "issues": [
            entry(38040, "leave", "asks for more than the tests that landed"),
            entry(37233, "already-closed", "done"),
            entry(77, "leave", "another open pull request is attached")]}
        return (40191, d1, j1), (39685, d2, j2)

    def test_render_one_pull_request_is_a_list_of_issues_to_close(self):
        (pr1, d1, j1), (pr2, d2, j2) = self.two_prs()
        d1["pr"]["state"], d1["pr"]["closed_at"] = "CLOSED", "2026-09-21T10:00:00Z"
        self.write(pr1, d1, j1)
        self.write(pr2, d2, j2)
        text, stats = mi.render(self.root, f"{OWNER}/{NAME}", 40191)
        self.assertTrue(text.startswith(
            f"# Issues Related to PR [#40191 Fix crash](https://github.com/{OWNER}/{NAME}/pull/40191)\n"))
        self.assertNotIn("Triage verdict", text)                         # provenance only in the header
        self.assertNotIn("closed on 2026-09-21", text)
        self.assertIn("`model-a`, high effort", text)
        self.assertNotIn("#38040", text)                                 # the other pull request stays out
        body = text.split("### Safe to close")[1]
        self.assertIn(
            f"- [#40190](https://github.com/{OWNER}/{NAME}/issues/40190) **Crash** (OPEN): guarded on master; same ask as [#6](https://github.com/{OWNER}/{NAME}/issues/6)\n", body)
        self.assertIn("**Same crash, other words** (OPEN): same crash; same ask as [#40190]", body)
        self.assertIn(f"- {mi.UNLINKED_MARK} [#6](", body)               # the inferred one is marked, in the same list
        self.assertNotIn(f"{mi.UNLINKED_MARK} [#40190](", body)           # the linked one is not
        self.assertIn(f"*{mi.UNLINKED_MARK} marks an issue that is not internally linked on GitHub and was found by content.*", text)
        self.assertNotIn("found by content", body.split("*" + mi.UNLINKED_MARK)
                         [0].replace("found by content.*", ""))  # no trailing phrase on the entry
        self.assertNotIn("#5", body)                                     # rejected candidates leave no trace
        self.assertEqual(stats, {"pr": 40191, "counts": {"close": 2, "unrelated": 1,
                         "inferred_rejected": 1}, "to_close": [40190, 6]})
        text2, stats2 = mi.render(self.root, f"{OWNER}/{NAME}", 39685)
        self.assertNotIn(mi.UNLINKED_MARK, text2)                         # no legend when nothing is unlinked
        self.assertIn("No issues addressed by this pull request were found.", text2)
        linked = text2.split("### Linked but not resolved by the PR")[1].split("### Already closed")[0]
        self.assertIn("[#77](", linked)
        self.assertIn("(OPEN): asks for more than the tests that landed\n", linked)   # each with its reason
        self.assertIn("### Already closed\n\n- [#37233](", text2)
        self.assertIn("**Epic** (CLOSED, completed)\n", text2)
        self.assertIn("`model-a`, unknown effort", text2)                # a made-up effort prints as unknown
        self.assertEqual(stats2["to_close"], [])

    def test_no_list_when_the_pull_request_was_not_closable(self):
        d = dossier(7, verdict="keep", issues={1: issue(1)}, tiers={"linked": [{"number": 1}]})
        j = {"pr": 7, "issues": [entry(1, "leave", "the fix never landed")]}
        self.write(7, d, j)
        text, _ = mi.render(self.root, f"{OWNER}/{NAME}", 7)
        self.assertIn("Nothing can be closed on this pull request's strength", text)
        self.assertIn("### Linked but not resolved by the PR", text)

    def test_render_refuses_an_unjudged_pull_request(self):
        (pr1, d1, j1), _ = self.two_prs()
        mi.triage.write_json(mi.dossier_path(self.root, pr1), d1)
        with self.assertRaises(RuntimeError):
            mi.render(self.root, f"{OWNER}/{NAME}", pr1)

    def test_numbers_in_reasons_become_links_once(self):
        d = dossier(issues={1: issue(1, title="T")}, tiers={"linked": [{"number": 1}]})
        j = {"pr": 40191, "issues": [
            entry(1, "close", "delivered by #39941 and [#40693](https://github.com/o/n/pull/40693); see (#7)")]}
        self.write(40191, d, j)
        text, _ = mi.render(self.root, f"{OWNER}/{NAME}", 40191)
        self.assertIn(
            f"delivered by [#39941](https://github.com/{OWNER}/{NAME}/issues/39941) and [#40693](https://github.com/o/n/pull/40693); see (#7)", text)
        self.assertEqual(mi.linkify("nothing here", "o/n"), "nothing here")

    def test_no_hard_wrapped_prose(self):
        (pr1, d1, j1), _ = self.two_prs()
        self.write(pr1, d1, j1)
        text, _ = mi.render(self.root, f"{OWNER}/{NAME}", pr1)
        for line in text.splitlines():
            if line.startswith("- "):
                self.assertNotIn("\n", line)
        self.assertIn("(OPEN): guarded on master;", text)               # the reason follows on the same line

    def test_state_is_a_projection_and_forget_rebuilds_it(self):
        for pr, d, j in self.two_prs():
            self.write(pr, d, j)
        state = mi.fold_state(self.root)
        self.assertEqual(state["40190"]["verdict"], "close")
        self.assertEqual(state["38040"], {"verdict": "leave", "via_pr": 39685, "relation": "linked",
                                          "at": "2026-09-21", "judged_by": "model-a"})
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            mi.forget(".", f"{OWNER}/{NAME}", "#40191", yes=False)
        self.assertTrue(mi.judgment_path(self.root, 40191).exists())      # dry run removes nothing
        self.assertIn("would_remove", out.getvalue())
        with contextlib.redirect_stdout(io.StringIO()):
            mi.report(".", f"{OWNER}/{NAME}", "40191", strict=True, out=None)
        self.assertTrue(mi.report_path(OWNER, NAME, 40191).exists())
        with contextlib.redirect_stdout(io.StringIO()):
            mi.forget(".", f"{OWNER}/{NAME}", "40191", yes=True)
        self.assertFalse(mi.dossier_path(self.root, 40191).exists())
        self.assertFalse(mi.report_path(OWNER, NAME, 40191).exists())       # the report goes with the judgment
        state = mi.triage.read_json(self.root / "state.json")
        self.assertNotIn("40190", state)
        self.assertIn("38040", state)                                     # the other pull request's findings survive

    def test_report_refuses_invalid_and_renders_valid(self):
        (pr, d, j), _ = self.two_prs()
        j["issues"][0]["reason"] = ""
        self.write(pr, d, j)
        with contextlib.redirect_stdout(io.StringIO()) as out, self.assertRaises(SystemExit):
            mi.report(".", f"{OWNER}/{NAME}", "40191", strict=True, out=None)
        self.assertIn("invalid_judgment", out.getvalue())
        j["issues"][0]["reason"] = "fine"
        self.write(pr, d, j)
        target = pathlib.Path(self.tmp.name) / "copies"
        target.mkdir()
        with contextlib.redirect_stdout(io.StringIO()) as out:
            mi.report(".", f"{OWNER}/{NAME}", "#40191", strict=True, out=str(target))
        result = json.loads(out.getvalue())
        self.assertEqual(result["pull_request"], 40191)
        self.assertTrue(pathlib.Path(result["rendered"]).exists())
        self.assertEqual(pathlib.Path(result["kept_at"]).name, "pr-40191-related-issues.md")
        self.assertEqual(pathlib.Path(result["kept_at"]).parent, pathlib.Path(self.tmp.name) / "reports" / OWNER / NAME)
        self.assertTrue((target / "pr-40191-related-issues.md").exists())
        with self.assertRaises(RuntimeError):
            mi.report(".", f"{OWNER}/{NAME}", None, strict=True, out=None)

    def test_rendering_does_not_rewrite_dossiers(self):
        for pr, d, j in self.two_prs():
            self.write(pr, d, j)
        before = {p.name: p.read_text() for p in self.root.glob("pr-*.json")}
        mi.render(self.root, f"{OWNER}/{NAME}", 40191)
        after = {p.name: p.read_text() for p in self.root.glob("pr-*.json")}
        self.assertEqual(before, after)

    def test_from_triage_numbers_reads_the_sibling_state(self):
        sibling_root = mi.triage.cache_root(OWNER, NAME)
        mi.triage.write_json(sibling_root / "state.json", {"1": {"verdict": "close"}, "2": {"verdict": "keep"},
                                                           "3": {"verdict": "close"}})
        self.assertEqual(mi.from_triage_numbers(OWNER, NAME, ["close"]), [3, 1])
        self.assertEqual(mi.from_triage_numbers(OWNER, NAME, ["rewrite"]), [])
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            mi.list_prs(".", f"{OWNER}/{NAME}", ["close"])
        listed = json.loads(out.getvalue())
        self.assertEqual(listed["from_triage"]["not_yet_done"], [3, 1])

    def test_triage_record_without_a_sibling_cache(self):
        self.assertEqual(mi.triage_record(OWNER, NAME, 1), {"triaged": False, "changed_paths": []})


if __name__ == "__main__":
    unittest.main()


class Corpus(unittest.TestCase):
    """The local corpus: tokens that meet, scoring signals, the read list and the recall maths."""

    def records(self):
        return [
            {"number": 1, "title": "Decouple `Actions` cluster", "body": "", "state": "OPEN", "author": "ann",
             "created_at": "2026-01-10T00:00:00Z", "labels": ["cluster-decoupling"], "parent": None, "sub_issues": [], "refs": [], "paths": []},
            {"number": 2, "title": "[BUG] Pairing over thread failing on nordic board CHIP Error 0x00000050", "body": "Fails with CHIP Error 0x00000050 on the nordic board",
             "state": "OPEN", "author": "bob", "created_at": "2026-01-08T00:00:00Z", "labels": ["bug"], "parent": None, "sub_issues": [], "refs": [], "paths": []},
            {"number": 3, "title": "[TC_AVSM_2_14] Test case description mismatch", "body": "In TC_AVSM_2_14 the class name differs",
             "state": "OPEN", "author": "cid", "created_at": "2025-07-10T00:00:00Z", "labels": [], "parent": None, "sub_issues": [], "refs": [], "paths": ["src/python_testing/TC_AVSM_2_14.py"]},
            {"number": 4, "title": "Flaky darwin test", "body": "There is a PR https://github.com/o/n/pull/500 for that issue",
             "state": "OPEN", "author": "dee", "created_at": "2026-01-01T00:00:00Z", "labels": [], "parent": 9, "sub_issues": [], "refs": [500, 9], "paths": ["src/messaging/tests/TestReliableMessageProtocol.cpp"]},
            {"number": 5, "title": "Something else entirely about lighting", "body": "on off level control " * 50,
             "state": "OPEN", "author": "ann", "created_at": "2026-01-11T00:00:00Z", "labels": [], "parent": None, "sub_issues": [], "refs": [], "paths": []},
            {"number": 9, "title": "Coverage umbrella", "body": "", "state": "CLOSED", "author": "eve",
             "created_at": "2025-01-01T00:00:00Z", "labels": ["epic"], "parent": None, "sub_issues": [4], "refs": [], "paths": []},
        ]

    def test_tokens_meet_across_spellings(self):
        self.assertIn("tc-avsm-2.14", mi.tokenize("TC_AVSM_2_14 python script"))
        self.assertIn("tc-avsm-2.14", mi.tokenize("the TC-AVSM-2.14 test"))
        toks = mi.tokenize("ZoneManagement refactor")
        self.assertTrue({"zonemanagement", "zone", "management"} <= set(toks))
        self.assertIn("jf", mi.tokenize("[E2E][JF] XML fixes"))          # two-letter acronyms survive
        self.assertNotIn("to", mi.tokenize("how to do it"))              # two-letter stopwords do not
        self.assertEqual(mi.stem("uploaded"), mi.stem("upload"))
        self.assertEqual(mi.stem("segments"), "segment")
        self.assertEqual(mi.stem("0x0032"), "0x0032")                    # identifiers are never stemmed
        self.assertIn("nrfconnect", mi.tokenize("nordic board"))         # platform aliases
        self.assertNotIn("the", mi.tokenize("the and for"))

    def test_query_keeps_every_title_term(self):
        index = mi.Index(self.records())
        long_body = " ".join(f"distinctword{i}" for i in range(200)) + " actions cluster nordic"
        terms = index.query_terms("Convert Actions cluster", long_body)
        self.assertEqual(terms[:2], ["convert", "action"][:0] +
                         [t for t in mi.tokenize("Convert Actions cluster") if index.df.get(t)][:2])
        self.assertLessEqual(len(terms), mi.QUERY_TERMS)

    def test_bm25_needs_two_shared_terms_and_prefers_titles(self):
        index = mi.Index(self.records())
        self.assertEqual(index.bm25(["nordic"], 2), 0.0)                 # one word in common is nothing
        self.assertGreater(index.bm25(mi.tokenize("nordic board pairing"), 2), 0.0)
        # the same two terms score higher against an issue carrying them in its title
        recs = self.records()
        recs.append({"number": 6, "title": "unrelated", "body": "pairing over thread on the nordic board", "state": "OPEN",
                     "author": "x", "created_at": "2026-01-01T00:00:00Z", "labels": [], "parent": None, "sub_issues": [], "refs": [], "paths": []})
        index = mi.Index(recs)
        q = mi.tokenize("pairing nordic")
        self.assertGreater(index.bm25(q, 2), index.bm25(q, 6))

    def test_signals_and_boosters(self):
        recs = self.records()
        index = mi.Index(recs)
        pr = {"number": 500, "title": "[CodeDriven] Convert Actions cluster to be code-driven", "body": "Migrate the cluster",
              "author": "ann", "created_at": "2026-01-12T00:00:00Z", "labels": ["cluster-decoupling"], "changed_paths": ["src/app/clusters/actions-server/ActionsCluster.cpp"]}
        ranked = mi.score_candidates(index, pr, seeds=set(), exclude={500})
        by = {e["number"]: e for e in ranked}
        self.assertIn(1, by)
        self.assertIn("title", by[1]["signals"])                          # "Actions cluster" met "Actions cluster"
        self.assertIn("author", by[1]["signals"])
        self.assertIn("labels", by[1]["signals"])
        self.assertIn(4, by)
        self.assertEqual(by[4]["signals"].get("refers"), 1.0)            # the issue names this pull request
        self.assertIn("names this pull request", by[4]["why"])
        # same author, one day apart, but nothing in common: a hunch about people, not a lead
        self.assertNotIn(5, by)

    def test_paths_signal_needs_the_file_or_a_deep_folder(self):
        index = mi.Index(self.records())
        pr = {"number": 7, "title": "Fix MRP delays", "body": "", "author": "z", "created_at": "2026-01-01T00:00:00Z",
              "labels": [], "changed_paths": ["src/messaging/tests/TestReliableMessageProtocol.cpp"]}
        by = {e["number"]: e for e in mi.score_candidates(index, pr, set(), {7})}
        self.assertEqual(by[4]["signals"].get("paths"), 1.0)             # names the very file
        pr["changed_paths"] = ["src/messaging/ReliableMessageMgr.cpp"]
        by = {e["number"]: e for e in mi.score_candidates(index, pr, set(), {7})}
        self.assertNotIn(4, by)                                           # src/messaging is not deep enough on its own

    def test_graph_hops_and_test_id_anchor(self):
        recs = self.records()
        hops = mi.graph_hops(recs, {9})
        self.assertEqual(hops[4], 1)
        index = mi.Index(recs)
        pr = {"number": 8, "title": "Fix description typo in AVSM test case number", "body": "", "author": "q",
              "created_at": "2025-08-26T00:00:00Z", "labels": [], "changed_paths": ["src/python_testing/TC_AVSM_2_14.py"]}
        by = {e["number"]: e for e in mi.score_candidates(index, pr, set(), {8})}
        self.assertIn("test_id tc-avsm-2.14", by[3]["why"])
        self.assertEqual(by[3]["signals"].get("paths"), 1.0)

    def test_read_list_caps_one_kind(self):
        longlist = [{"number": n, "signals": {"graph": 1.0}} for n in range(10)]
        longlist.append({"number": 99, "signals": {"exact": 3.0}})
        picked = [e["number"] for e in mi.read_list(longlist, 8)]
        self.assertEqual(picked.count(99), 1)
        self.assertEqual(len([n for n in picked if n < 10]), mi.READ_CAPS["graph"])
        self.assertEqual(mi.lead_kind({"signals": {"text": 0.9, "exact": 1.0}}), "mixed")

    def test_mask_numbers_and_recall_maths(self):
        text = "Fixes #123 and https://github.com/o/n/issues/123 and o/n#123 but keeps #124"
        masked = mi.mask_numbers(text, [123])
        self.assertNotIn("123", masked)
        self.assertIn("#124", masked)
        rows = [{"truth": [1, 2], "ranks": {1: 1, 2: 30}}, {"truth": [3], "ranks": {3: None}}]
        m = mi.recall_metrics(rows, 8)
        self.assertEqual(m["issues"], 3)
        self.assertAlmostEqual(m["recall_at_8"], 1 / 3, places=3)
        self.assertAlmostEqual(m[f"recall_at_{mi.LONGLIST}"], 2 / 3, places=3)
        self.assertAlmostEqual(m["mrr"], 0.5, places=3)

    def test_merge_and_record(self):
        old = [{"number": 1, "title": "a"}, {"number": 2, "title": "b"}]
        new = [{"number": 2, "title": "b2"}, {"number": 3, "title": "c"}]
        merged, added, updated = mi.merge_records(old, new)
        self.assertEqual([r["number"] for r in merged], [1, 2, 3])
        self.assertEqual((added, updated), (1, 1))
        node = {"number": 7, "title": "T", "state": "OPEN", "body": "see #12 and src/app/ReadHandler.cpp.", "author": {"login": "x"},
                "labels": {"nodes": [{"name": "bug"}]}, "parent": {"number": 3}, "subIssues": {"nodes": [{"number": 8}]}}
        rec = mi.corpus_record(node, OWNER, NAME)
        self.assertEqual(rec["refs"], [12])
        self.assertEqual(rec["paths"], ["src/app/ReadHandler.cpp"])
        self.assertEqual((rec["parent"], rec["sub_issues"]), (3, [8]))

    def test_pr_for_scoring_reads_commit_messages(self):
        node = {"number": 1, "title": "t", "body": "b", "author": {"login": "a"}, "createdAt": "2026-01-01T00:00:00Z",
                "labels": {"nodes": []}, "files": {"nodes": [{"path": "x/y.cpp"}]},
                "commits": {"nodes": [{"commit": {"message": "Fixes #9"}}]}}
        pr = mi.pr_for_scoring(node)
        self.assertEqual(pr["commit_messages"], ["Fixes #9"])
        self.assertEqual(pr["changed_paths"], ["x/y.cpp"])


class SeniorDevPass(unittest.TestCase):
    """Fixes from the adversarial read, each proven to fire."""

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        os.environ["MATTER_PR_TRIAGE_CACHE"] = self.tmp.name
        os.environ["MATTER_PR_TRIAGE_REPORTS"] = os.path.join(self.tmp.name, "reports")
        self.root = mi.issues_root(OWNER, NAME)
        self.root.mkdir(parents=True)

    def tearDown(self):
        os.environ.pop("MATTER_PR_TRIAGE_CACHE", None)
        os.environ.pop("MATTER_PR_TRIAGE_REPORTS", None)
        self.tmp.cleanup()

    def test_a_verdict_for_a_skim_entry_says_how_to_bring_it_in(self):
        d = dossier(issues={1: issue(1)}, tiers={"linked": [{"number": 1}]})
        d["longlist"] = [{"number": 55, "title": "on the skim list", "state": "OPEN", "score": 2.0, "why": "x"}]
        problems = mi.validate_issue_judgment({"pr": 40191, "issues": [entry(1, "close"), entry(55, "close")]}, d)
        self.assertTrue(any("gather --pr 40191 --read 55" in p for p in problems))
        problems = mi.validate_issue_judgment({"pr": 40191, "issues": [entry(1, "close"), entry(56, "close")]}, d)
        self.assertTrue(any("#56: judged but not in this dossier" in p and "--read" not in p for p in problems))

    def test_reasons_that_talk_about_the_tool_are_flagged(self):
        j = {"issues": [entry(1, "close", "the dossier shows the fix"), entry(2, "close", "master carries the guard"),
                        entry(3, "unrelated", "a candidate from the corpus")]}
        found = mi.lint_issue_judgment(j)
        self.assertEqual(len(found), 2)
        self.assertTrue(found[0].startswith("#1:") and found[1].startswith("#3:"))

    def test_heading_escapes_brackets_in_the_title(self):
        d = dossier(issues={1: issue(1)}, tiers={"linked": [{"number": 1}]})
        d["pr"]["title"] = "[BUG] Crash [app]"
        mi.triage.write_json(mi.dossier_path(self.root, 40191), d)
        mi.triage.write_json(mi.judgment_path(self.root, 40191), {"pr": 40191, "issues": [entry(1, "close")]})
        text, _ = mi.render(self.root, f"{OWNER}/{NAME}", 40191)
        self.assertTrue(text.startswith(r"# Issues Related to PR [#40191 \[BUG\] Crash \[app\]](https://github.com/"))

    def test_sync_pages_degrade_without_family_fields(self):
        calls = []

        def fake_graphql(query, variables=None, retries=4):
            calls.append(query)
            if "subIssues" in query:
                raise RuntimeError("graphql: Field 'subIssues' doesn't exist on type 'Issue'")
            return {"repository": {"issues": {"pageInfo": {"hasNextPage": False, "endCursor": None},
                                              "nodes": [{"number": 1, "title": "t", "state": "OPEN", "updatedAt": "2026-01-01T00:00:00Z"}]}}}
        real, mi.triage.gh_graphql = mi.triage.gh_graphql, fake_graphql
        mi.SUPPORTS_FAMILY["value"] = True
        try:
            nodes, info = mi.page_issues(OWNER, NAME, "OPEN", None)
        finally:
            mi.triage.gh_graphql = real
            mi.SUPPORTS_FAMILY["value"] = True
        self.assertEqual([n["number"] for n in nodes], [1])
        self.assertEqual(len(calls), 2)                                  # rejected once, then retried without the fields
        self.assertNotIn("subIssues", calls[1])
        self.assertIn("labels(first: 12)", calls[1])                      # the rest of the query is intact

    def test_cost_sums_the_log_and_narrows_to_a_pull_request(self):
        rows = [{"command": "issues gather", "run": "pr-40191", "seconds": 4.0, "graphql_calls": 3, "rest_calls": 0, "rate_limit_waits": 0},
                {"command": "issues gather", "run": "pr-40453", "seconds": 6.0,
                    "graphql_calls": 5, "rest_calls": 1, "rate_limit_waits": 0},
                {"command": "issues report", "run": "pr-40191", "seconds": 0.5, "graphql_calls": 0, "rest_calls": 0, "rate_limit_waits": 0}]
        (self.root / "cost.jsonl").write_text("\n".join(json.dumps(r) for r in rows) + "\nnot json\n")
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            mi.cost(".", f"{OWNER}/{NAME}", None)
        summary = json.loads(out.getvalue())
        self.assertEqual(summary["total"], {"seconds": 10.5, "graphql_calls": 8, "rest_calls": 1})
        self.assertEqual(summary["by_command"]["issues gather"]["runs"], 2)
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            mi.cost(".", f"{OWNER}/{NAME}", "#40191")
        narrowed = json.loads(out.getvalue())
        self.assertEqual(narrowed["entries"], 2)
        self.assertEqual(narrowed["total"]["graphql_calls"], 3)

    def test_a_regathered_dossier_reports_how_the_judgment_fits(self):
        d = dossier(issues={1: issue(1), 2: issue(2)}, tiers={"linked": [{"number": 1}], "inferred": [{"number": 2}]})
        j = {"pr": 40191, "issues": [entry(1, "close"), entry(9, "unrelated")], "unassessed": [{"issue": 8, "reason": "r"}]}
        self.assertEqual(mi.judgment_status(d, j), {"stale_entries": [8, 9], "new_to_judge": [2]})
        j = {"pr": 40191, "issues": [entry(1, "close"), entry(2, "unrelated")]}
        self.assertEqual(mi.judgment_status(d, j), {"stale_entries": [], "new_to_judge": []})

    def test_repo_halves_made_of_dots_are_refused(self):
        for bad in ("../x", "x/..", "./x", "x/.", "../.."):
            with self.assertRaises(RuntimeError):
                mi.triage.split_repo(bad)
        self.assertEqual(mi.triage.split_repo("a.b/c.d"), ("a.b", "c.d"))   # dots inside a name are fine
