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
"""Unit tests for the parts of matter_pr_triage that need no network and no checkout.

Run with:  python3 -m unittest discover scripts/tools/pr_triage/tests
"""
import json
import os
import pathlib
import sys
import tempfile
import unittest

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent.parent))
import matter_pr_triage as m  # noqa: E402 isort:skip


class Derive(unittest.TestCase):
    def test_coverage_dominates_band(self):
        for band in m.BANDS:
            self.assertEqual(m.derive("covered_master", band), "close")
            self.assertEqual(m.derive("covered_in_flight", band), "in-flight")
            self.assertEqual(m.derive("unknown", band), "unclear")

    def test_band_decides_when_not_covered(self):
        self.assertEqual(m.derive("not_covered", "moved"), "rewrite")
        self.assertEqual(m.derive("not_covered", "unowned"), "unowned")
        self.assertEqual(m.derive("not_covered", "cheap"), "keep")
        self.assertEqual(m.derive("not_covered", "conflicts"), "keep")


class TrimDossier(unittest.TestCase):
    def dossier(self):
        return {"number": 1, "comments": [
            {"by": "github-actions", "body": "size table " * 200},
            {"by": "human", "body": "x" * 1000},
            {"by": "gemini-code-assist", "body": "review"}],
            "review_threads": [{"resolved": False, "comments": [{"by": "copilot-pull-request-reviewer", "body": "nit"}]},
                               {"resolved": True, "comments": [{"by": "cecille", "body": "superseded by #1"}]}]}

    def test_drops_bots_and_caps_bodies(self):
        d = m.trim_dossier(self.dossier())
        self.assertEqual([c["by"] for c in d["comments"]], ["human"])
        self.assertTrue(d["comments"][0]["body"].endswith(" [...]"))
        self.assertLessEqual(len(d["comments"][0]["body"]), m.COMMENT_BUDGET + 6)
        self.assertEqual(len(d["review_threads"]), 1)               # bot-only thread removed
        self.assertEqual(d["comments_dropped"], 3)

    def test_idempotent(self):
        once = m.trim_dossier(self.dossier())
        twice = m.trim_dossier(json.loads(json.dumps(once)))
        self.assertEqual(once, twice)

    def test_untouched_when_nothing_to_trim(self):
        self.assertEqual(m.trim_dossier({"number": 2, "errors": [{"code": "x"}]}), {"number": 2, "errors": [{"code": "x"}]})


class RunIds(unittest.TestCase):
    def test_rejects_escapes(self):
        for bad in ("", ".", "..", "a/b", "a\\b", "../../etc"):
            with self.assertRaises(RuntimeError):
                m.checked_run_id(bad)

    def test_accepts_normal(self):
        self.assertEqual(m.checked_run_id("20260918-051451-clean-slate"), "20260918-051451-clean-slate")


class Judgment(unittest.TestCase):
    sel = {"prs": [{"number": 1}, {"number": 2}]}

    def verdict(self, **kw):
        base = {"pr": 1, "coverage": "not_covered", "revival": "cheap", "verdict": "keep", "evidence": "the guard is absent"}
        base.update(kw)
        return base

    def test_clean_judgment_passes(self):
        j = {"verdicts": [self.verdict(), self.verdict(pr=2)]}
        self.assertEqual(m.validate_judgment(j, self.sel), [])

    def test_missing_selected_pr(self):
        j = {"verdicts": [self.verdict()]}
        self.assertTrue(any("#2" in p and "no verdict" in p for p in m.validate_judgment(j, self.sel)))

    def test_unselected_pr_flagged(self):
        j = {"verdicts": [self.verdict(), self.verdict(pr=2), self.verdict(pr=99)]}
        self.assertTrue(any("#99" in p and "not in this run" in p for p in m.validate_judgment(j, self.sel)))

    def test_enum_typo_flagged(self):
        j = {"verdicts": [self.verdict(coverage="not_coverd"), self.verdict(pr=2, verdict="keeep")]}
        problems = m.validate_judgment(j, self.sel)
        self.assertTrue(any("coverage" in p for p in problems) and any("verdict" in p for p in problems))

    def test_disagreement_needs_reason(self):
        j = {"verdicts": [self.verdict(verdict="close"), self.verdict(pr=2)]}
        self.assertTrue(any("disagreement_reason" in p for p in m.validate_judgment(j, self.sel)))
        j["verdicts"][0]["disagreement_reason"] = "checked by hand"
        self.assertEqual(m.validate_judgment(j, self.sel), [])

    def test_lint_flags_tool_talk(self):
        j = {"verdicts": [self.verdict(evidence="blame shows it untouched"), self.verdict(pr=2)]}
        self.assertEqual(len(m.lint_judgment(j)), 1)


class Misc(unittest.TestCase):
    def test_date_field_inference(self):
        self.assertEqual(m.date_field_of({"date_field": "activity"}), "activity")
        self.assertEqual(m.date_field_of({"query": "repo:x is:pr created:<2025-01-01"}), "created")
        self.assertEqual(m.date_field_of({"query": "repo:x is:pr updated:<2025-01-01"}), "activity")

    def test_is_mechanical(self):
        self.assertTrue(m.is_mechanical("github-actions"))
        self.assertTrue(m.is_mechanical("foo[bot]"))
        self.assertFalse(m.is_mechanical("cecille"))
        self.assertFalse(m.is_mechanical(None))

    def test_save_copy_file_and_dir(self):
        with tempfile.TemporaryDirectory() as d:
            f = m.save_copy(os.path.join(d, "out.md"), "x.md", "hi\n")
            self.assertTrue(f.endswith("out.md"))
            self.assertEqual(pathlib.Path(f).read_text(), "hi\n")
            g = m.save_copy(os.path.join(d, "sub") + os.sep, "named.md", "hi\n")
            self.assertTrue(g.endswith(os.path.join("sub", "named.md")))
            self.assertIsNone(m.save_copy(None, "x.md", "hi"))

    def test_measured_tokens_sums_iterations(self):
        with tempfile.TemporaryDirectory() as d:
            p = os.path.join(d, "t.jsonl")
            rows = [{"timestamp": "2026-01-01T00:00:01Z", "message": {"usage": {"iterations": [
                {"input_tokens": 10, "cache_creation_input_tokens": 5, "cache_read_input_tokens": 100, "output_tokens": 7}]}}},
                {"timestamp": "2026-01-01T00:00:02Z", "message": {"usage": {"input_tokens": 1,
                                                                            "cache_creation_input_tokens": 0, "cache_read_input_tokens": 0, "output_tokens": 2}}},
                {"timestamp": "2026-01-01T00:00:09Z", "message": {"usage": {"input_tokens": 999, "output_tokens": 999}}}]
            pathlib.Path(p).write_text("\n".join(json.dumps(r) for r in rows) + "\n")
            t = m.measured_tokens(p, "2026-01-01T00:00:00", "2026-01-01T00:00:05")
            self.assertEqual((t["model_turns"], t["fresh_input_tokens"],
                             t["output_tokens"], t["cache_read_tokens"]), (2, 16, 9, 100))

    def test_cost_summary_on_empty_root(self):
        with tempfile.TemporaryDirectory() as d:
            root = pathlib.Path(d)
            (root / "pr").mkdir()
            c = m.cost_summary(root)
            self.assertEqual((c["wall_clock_seconds"], c["api_calls"], c["pull_requests"],
                             c["judging_tokens_estimate"]), (0, 0, 0, 0))
            self.assertFalse(c["gathering_timed"])

    def test_render_body_links_missing_dossier(self):
        v = {7: {"verdict": "keep", "evidence": "still absent from master, see #8"}}
        text = "\n".join(m.render_body(v, {}, "o/r"))
        self.assertIn("[#7](https://github.com/o/r/pull/7)", text)
        self.assertIn("[#8](https://github.com/o/r/pull/8)", text)
        self.assertIn("### STILL NEEDED (1)", text)


if __name__ == "__main__":
    unittest.main()


class CrossBatch(unittest.TestCase):
    def test_relate_reaches_across_batches_and_pairs_surface(self):
        with tempfile.TemporaryDirectory() as d:
            root = pathlib.Path(d)
            (root / "pr").mkdir()
            for n, title in ((1, "Fix memory leak in function A"), (2, "Fix memory leak in function B"), (3, "Unrelated docs tweak")):
                (root / "pr" / f"{n}.json").write_text(json.dumps({"number": n, "title": title,
                                                                   "changed_paths": ["src/x.cpp"] if n < 3 else ["docs/a.md"]}))
            m.relate_batch(root, [1])                     # batch of one; 2 sits in the cache from another batch
            cands = json.loads((root / "pr" / "1.json").read_text())["signals"]["related_candidates"]
            self.assertEqual([c["pr"] for c in cands], [2])
            verdicts = {1: {"verdict": "keep"}, 2: {"verdict": "keep"}, 3: {"verdict": "close"}}
            pairs = m.related_pairs_unreviewed(root, verdicts)
            self.assertEqual([p["prs"] for p in pairs], [[1, 2]])
            verdicts[1]["related"] = [{"pr": 2, "kind": "related", "why": "sweep"}]
            self.assertEqual(m.related_pairs_unreviewed(root, verdicts), [])

    def test_hub_files_do_not_relate_and_unrelated_is_reviewed(self):
        with tempfile.TemporaryDirectory() as d:
            root = pathlib.Path(d)
            (root / "pr").mkdir()
            for n in range(1, 7):        # six pull requests all touching one workflow file
                (root / "pr" / f"{n}.json").write_text(json.dumps({"number": n, "title": ["Alpha", "Bravo", "Charlie", "Delta", "Echo", "Foxtrot"][n - 1] + " work",
                                                                   "changed_paths": [".github/workflows/tests.yaml"] + (["src/only.cpp"] if n in (5, 6) else [])}))
            m.relate_batch(root, list(range(1, 7)))

            def cands(n):
                return [c["pr"] for c in json.loads((root / "pr" / f"{n}.json").read_text())["signals"]["related_candidates"]]
            self.assertEqual(cands(1), [])              # the hub alone relates nothing
            self.assertEqual(cands(5), [6])             # the rare file still does
            verdicts = {n: {"verdict": "keep"} for n in range(1, 7)}
            self.assertEqual([p["prs"] for p in m.related_pairs_unreviewed(root, verdicts)], [[5, 6]])
            verdicts[5]["unrelated"] = [6]
            self.assertEqual(m.related_pairs_unreviewed(root, verdicts), [])


class Windows(unittest.TestCase):
    def test_campaign_filename(self):
        self.assertEqual(m.campaign_filename({"cutoff": "2025-09-18", "date_field": "created"},
                         "2026-09-18"), "pr-triage-created-before-2025-09-18-as-of-2026-09-18.md")
        self.assertEqual(m.campaign_filename({"cutoff": "2025-12-02", "query": "repo:x is:pr updated:<2025-12-02"},
                         "2026-01-05"), "pr-triage-active-before-2025-12-02-as-of-2026-01-05.md")
        self.assertEqual(m.campaign_filename({"query": "repo:x is:pr"}), "pr-triage-repo-x-is-pr.md")

    def test_reports_root_is_a_visible_folder_apart_from_the_cache(self):
        os.environ.pop("MATTER_PR_TRIAGE_REPORTS", None)
        self.assertEqual(m.reports_root("o", "r"), pathlib.Path.home() / "matter-pr-triage-reports" / "o" / "r")
        os.environ["MATTER_PR_TRIAGE_REPORTS"] = "/tmp/elsewhere"
        try:
            self.assertEqual(m.reports_root("o", "r"), pathlib.Path("/tmp/elsewhere/o/r"))
        finally:
            os.environ.pop("MATTER_PR_TRIAGE_REPORTS", None)

    def test_one_file_per_window_and_no_complete_line(self):
        with tempfile.TemporaryDirectory() as d:
            root = pathlib.Path(d)
            (root / "pr").mkdir()

            def run(rid, query, cutoff, prs, created, left=0):
                (root / "runs" / rid).mkdir(parents=True)
                (root / "runs" / rid / "manifest.json").write_text(json.dumps(
                    {"query": query, "cutoff": cutoff, "date_field": "created", "created_at": created, "not_examined": left}))
                (root / "runs" / rid / "judgment.json").write_text(json.dumps(
                    {"verdicts": [{"pr": p, "coverage": "not_covered", "revival": "cheap", "verdict": "keep", "evidence": "absent"} for p in prs]}))
            run("a1", "q1", "2025-09-18", [1, 2], "2026-01-01T00:00:00")
            run("a2", "q1", "2025-09-18", [3], "2026-01-02T00:00:00")
            run("b1", "q2", "2025-06-01", [4], "2026-01-03T00:00:00", left=5)
            out = root / "reports"
            rendered = m.render_all_campaigns(root, "o/r", out_dir=out)
            names = sorted(r["path"].name for r in rendered)
            # the file carries the date of the window's newest batch, not today's
            self.assertEqual(names, ["pr-triage-created-before-2025-06-01-as-of-2026-01-03.md",
                             "pr-triage-created-before-2025-09-18-as-of-2026-01-02.md"])
            self.assertTrue(all(r["path"].parent == out for r in rendered))                # reports live in the reports folder
            # nothing readable left in the cache root
            self.assertFalse(list(root.glob("*.md")))
            q1 = next(r for r in rendered if r["query"] == "q1")
            q2 = next(r for r in rendered if r["query"] == "q2")
            self.assertEqual(q1["verdicts"], 3)
            self.assertNotIn("Still to triage", q1["text"])
            self.assertNotIn("batch", q1["text"].split("## Disclaimer")[0])
            self.assertIn("5 pull requests matching the window have not been looked at", q2["text"])
            self.assertTrue(q2["newest"])
            # a repeat of q1 on a later day gets its own file and the earlier day's file stays
            run("a3", "q1", "2025-09-18", [5], "2026-02-10T00:00:00")
            rendered = m.render_all_campaigns(root, "o/r", out_dir=out)
            self.assertTrue((out / "pr-triage-created-before-2025-09-18-as-of-2026-02-10.md").exists())
            self.assertTrue((out / "pr-triage-created-before-2025-09-18-as-of-2026-01-02.md").exists())
            index = json.loads((root / "reports.json").read_text())
            self.assertEqual(len(index), 3)


class PassFindings(unittest.TestCase):
    def test_rendering_does_not_rewrite_dossiers(self):
        with tempfile.TemporaryDirectory() as d:
            root = pathlib.Path(d)
            (root / "pr").mkdir()
            for n in (1, 2):
                (root / "pr" / f"{n}.json").write_text(json.dumps({"number": n, "title": f"T{n}", "changed_paths": ["a.cpp"]}))
            before = {n: (root / "pr" / f"{n}.json").stat().st_mtime_ns for n in (1, 2)}
            found = m.relate_batch(root, [1, 2], write=False)
            self.assertEqual([c["pr"] for c in found[1]], [2])
            self.assertEqual({n: (root / "pr" / f"{n}.json").stat().st_mtime_ns for n in (1, 2)}, before)

    def test_bad_related_entries_are_validation_problems_not_crashes(self):
        sel = {"prs": [{"number": 1}]}
        j = {"verdicts": [{"pr": 1, "coverage": "not_covered", "revival": "cheap", "verdict": "keep", "evidence": "x",
                           "related": [{"pr": "40007a"}], "unrelated": ["#7", "seven"]}]}
        problems = m.validate_judgment(j, sel)
        self.assertEqual(sum("must be an object naming a pull request" in p for p in problems), 1)
        self.assertEqual(sum("does not name a pull request" in p for p in problems), 1)
        self.assertEqual(m.related_pairs_unreviewed(pathlib.Path("/nonexistent"), {1: j["verdicts"][0]}, {1: []}), [])

    def test_colliding_windows_get_distinct_files_and_index_cleans_only_its_own(self):
        with tempfile.TemporaryDirectory() as d:
            root = pathlib.Path(d)
            (root / "pr").mkdir()

            def run(rid, query, prs, created):
                (root / "runs" / rid).mkdir(parents=True)
                (root / "runs" / rid / "manifest.json").write_text(json.dumps(
                    {"query": query, "cutoff": "2025-09-18", "date_field": "created", "created_at": created}))
                (root / "runs" / rid / "judgment.json").write_text(json.dumps(
                    {"verdicts": [{"pr": p, "coverage": "not_covered", "revival": "cheap", "verdict": "keep", "evidence": "absent"} for p in prs]}))
            run("a", "q created:<2025-09-18", [1], "2026-01-01T00:00:00")
            run("b", "q created:<2025-09-18 label:x", [2], "2026-01-01T00:00:00")
            out = root / "reports"
            out.mkdir()
            (out / "pr-triage-created-before-2025-09-18-mine.md").write_text("a user's own file\n")
            (root / "report-created-before-2025-09-18.md").write_text("a file from before the reports folder\n")
            (root / "reports.json").write_text(json.dumps({"report-created-before-2025-09-18.md": "q created:<2025-09-18"}))
            rendered = m.render_all_campaigns(root, "o/r", out_dir=out)
            names = {r["path"].name for r in rendered}
            plain = "pr-triage-created-before-2025-09-18-as-of-2026-01-01.md"
            self.assertEqual(len(names), 2)
            self.assertIn(plain, names)
            hashed = (names - {plain}).pop()
            self.assertTrue(hashed.startswith("pr-triage-created-before-2025-09-18-as-of-2026-01-01-"))
            self.assertTrue((out / "pr-triage-created-before-2025-09-18-mine.md").exists())   # not ours, untouched
            self.assertFalse((root / "report-created-before-2025-09-18.md").exists()
                             )          # the pre-folder file is cleaned up once
            import shutil
            shutil.rmtree(root / "runs" / "b")
            m.render_all_campaigns(root, "o/r", out_dir=out)
            self.assertFalse((out / hashed).exists())                                          # ours, window gone
            self.assertTrue((out / plain).exists())
            self.assertTrue((out / "pr-triage-created-before-2025-09-18-mine.md").exists())

    def test_related_shape_is_validated_before_render(self):
        sel = {"prs": [{"number": 1}]}
        base = {"pr": 1, "coverage": "not_covered", "revival": "cheap", "verdict": "keep", "evidence": "x"}
        bare = m.validate_judgment({"verdicts": [dict(base, related=[40007])]}, sel)
        self.assertTrue(any("must be an object" in p for p in bare))
        kind = m.validate_judgment({"verdicts": [dict(base, related=[{"pr": 40007, "kind": "twin"}])]}, sel)
        self.assertTrue(any("not duplicate or related" in p for p in kind))
        self.assertEqual(m.validate_judgment(
            {"verdicts": [dict(base, related=[{"pr": "#40007", "kind": "related", "why": "sweep"}])]}, sel), [])


class AddedCodeAndMode(unittest.TestCase):
    class FakeSource:
        """Files at the base revision, and no repo-wide search, like remote mode."""

        def __init__(self, files, grep=None): self.files, self._grep = files, grep
        def tree(self): return list(self.files)
        def file_text(self, path): return self.files.get(path)
        def grep_literal(self, text, limit=3): return None

    LINE = "VerifyOrReturnError(mExchangeCtx->HasSessionHandle(), CHIP_ERROR_INCORRECT_STATE);"

    def test_own_files_decide_and_unrelated_matches_are_not_leads(self):
        # absent from its own file, and the only other copy is nowhere near it
        src = self.FakeSource({"src/app/ReadHandler.cpp": "no guard here\n",
                               "src/test/Other.cpp": self.LINE})
        got = m.added_code_present(src, [self.LINE], ["src/app/ReadHandler.cpp"])
        self.assertEqual(got[0]["found_in"], [])     # decides: absent
        self.assertIsNone(got[0]["nearby"])          # a distant file is not a relocation lead

    def test_a_sibling_or_renamed_file_is_a_lead(self):
        src = self.FakeSource({"src/app/ReadHandler.cpp": "no guard here\n",
                               "src/app/ReadHandlerImpl.cpp": self.LINE})
        got = m.added_code_present(src, [self.LINE], ["src/app/ReadHandler.cpp"])
        self.assertEqual(got[0]["found_in"], [])
        self.assertEqual(got[0]["nearby"], "src/app/ReadHandlerImpl.cpp")

    def test_present_in_its_own_file(self):
        src = self.FakeSource({"a.cpp": "before\n" + self.LINE + "\nafter\n"})
        got = m.added_code_present(src, [self.LINE], ["a.cpp"])
        self.assertEqual(got[0]["found_in"], ["a.cpp"])

    def test_works_without_any_repo_wide_search(self):
        src = self.FakeSource({"a.cpp": self.LINE})          # grep unavailable, as in remote mode
        got = m.added_code_present(src, [self.LINE], ["a.cpp"])
        self.assertIsNotNone(got)
        self.assertEqual(got[0]["found_in"], ["a.cpp"])

    def test_unreadable_and_unsearchable_is_none(self):
        self.assertIsNone(m.added_code_present(self.FakeSource({}), [self.LINE], ["gone.cpp"]))

    def test_resolve_mode_prefers_a_matching_clone_and_falls_back(self):
        self.assertEqual(m.resolve_mode("/tmp", "owner/repo", False)[0], True)     # no clone -> API
        self.assertEqual(m.resolve_mode("/tmp", "owner/repo", True)[0], True)      # forced

    def test_binaries_are_never_relocation_candidates(self):
        tree = ["docs/logo.png", "src/app/Handler.cpp", "src/app/Handler.h", "src/app/logo.png"]
        got = m.relocation_candidates("src/app/Handler.cpp", tree)
        self.assertNotIn("src/app/logo.png", got)
        self.assertIn("src/app/Handler.h", got)


class LeavesWorkAlone(unittest.TestCase):
    def test_detects_each_in_progress_operation(self):
        for marker, expected in (("rebase-merge", "a rebase"), ("MERGE_HEAD", "a merge"),
                                 ("CHERRY_PICK_HEAD", "a cherry-pick"), ("BISECT_LOG", "a bisect"),
                                 ("index.lock", "another git command")):
            with tempfile.TemporaryDirectory() as d:
                git = pathlib.Path(d) / ".git"
                git.mkdir()
                self.assertIsNone(m.work_in_progress(d))
                (git / marker).mkdir() if marker == "rebase-merge" else (git / marker).write_text("x")
                self.assertEqual(m.work_in_progress(d), expected)

    def test_fixes_refuse_while_busy(self):
        with tempfile.TemporaryDirectory() as d:
            git = pathlib.Path(d) / ".git"
            git.mkdir()
            (git / "MERGE_HEAD").write_text("x")
            out = m.apply_fixes(d, [{"item": "stale clone", "autofix": True, "fix": "git fetch upstream master"}])
            self.assertEqual(len(out), 1)
            self.assertFalse(out[0]["ok"])
            self.assertIn("a merge is in progress", out[0]["detail"])


class PreflightIsNotVacuous(unittest.TestCase):
    def test_remote_without_a_repo_is_not_ok(self):
        missing = m.preflight_checks("/tmp", None, remote=True)
        self.assertTrue(any(i["item"] == "no repository" for i in missing))
        self.assertFalse(any(i.get("degrades") for i in missing if i["item"] == "no repository"))

    def test_remote_with_a_repo_does_not_raise_it(self):
        missing = m.preflight_checks("/tmp", "owner/name", remote=True)
        self.assertFalse(any(i["item"] == "no repository" for i in missing))


class RepoArgument(unittest.TestCase):
    def test_bad_repo_values_give_a_message_not_a_traceback(self):
        for bad in ("connectedhomeip", "", "a/b/c", "/name", "owner/"):
            with self.assertRaises(RuntimeError) as e:
                m.split_repo(bad)
            self.assertIn("owner/name", str(e.exception))

    def test_good_repo_value(self):
        self.assertEqual(m.split_repo("project-chip/connectedhomeip"), ("project-chip", "connectedhomeip"))


class StaleCloneFix(unittest.TestCase):
    def test_fix_uses_the_structured_remote_and_branch(self):
        # the human-readable fix string is not parsed; a refused fetch still names it verbatim
        with tempfile.TemporaryDirectory() as d:
            git = pathlib.Path(d) / ".git"
            git.mkdir()
            (git / "MERGE_HEAD").write_text("x")
            item = {"item": "stale clone", "autofix": True, "fetch": ["upstream", "master"],
                    "fix": "git fetch upstream master"}
            out = m.apply_fixes(d, [item])
            self.assertIn("git fetch upstream master", out[0]["detail"])


class MissingBinary(unittest.TestCase):
    def test_run_without_the_binary_degrades_or_explains(self):
        self.assertEqual(m.run(["definitely-not-a-binary-xyz", "--v"], check=False), "")
        with self.assertRaises(RuntimeError) as e:
            m.run(["definitely-not-a-binary-xyz", "--v"])
        self.assertIn("is not installed", str(e.exception))


class RepoUrls(unittest.TestCase):
    def test_pasted_forms_reduce_to_owner_name(self):
        for form in ("project-chip/connectedhomeip",
                     "https://github.com/project-chip/connectedhomeip",
                     "https://github.com/project-chip/connectedhomeip/",
                     "https://github.com/project-chip/connectedhomeip.git",
                     "http://www.github.com/project-chip/connectedhomeip",
                     "github.com/project-chip/connectedhomeip",
                     "git@github.com:project-chip/connectedhomeip.git",
                     "  project-chip/connectedhomeip  "):
            self.assertEqual(m.split_repo(form), ("project-chip", "connectedhomeip"), form)

    def test_dot_only_halves_are_refused(self):
        # both halves become cache directory names, so "." or ".." would point outside the cache
        for bad in ("../x", "x/..", "./x", "x/.", "../.."):
            with self.assertRaises(RuntimeError):
                m.split_repo(bad)
        self.assertEqual(m.split_repo("a.b/c.d"), ("a.b", "c.d"))

    def test_a_pull_request_url_is_not_a_repository(self):
        with self.assertRaises(RuntimeError):
            m.split_repo("https://github.com/project-chip/connectedhomeip/pull/40445")
