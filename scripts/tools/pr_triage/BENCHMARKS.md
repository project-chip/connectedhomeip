# Benchmarks

What the judging costs and how well it does, measured. Everything here is one
repository, `project-chip/connectedhomeip`. The model comparisons all use one
batch of 24 pull requests, so read differences of a verdict or two as noise and
the shapes as the finding; the cost section adds two earlier batches of 37
and 15. The tool itself is described in [README.md](README.md).

## Model and effort

The gathering phases need no model at all. Judging splits in two: where the
gathered signals already point one way the verdict follows mechanically, and
where they do not, someone has to open the base branch and decide whether an
equivalent change is already there. That second half is what separates models
and, it turns out, what separates effort settings on the same model. Two
measured runs, both on the same 24 pull requests with the same evidence and the
same instructions, scored against verdicts checked by hand against the base
branch.

How both runs were set up, so they can be repeated or argued with:

|                                  |                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              |
| -------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Repository                       | `project-chip/connectedhomeip`, base branch `upstream/master` at `99a81bd32`. One agent in run 2 fetched part-way through, moving the branch name to `954268d5`, sixteen commits on; none of those touch any of the 143 files the 24 pull requests target, so no verdict could have changed, and the skill now tells agents to read the recorded commit rather than the branch name.                                                                                                                                                                                         |
| Selection                        | 24 open pull requests, `--older-than 290d` on the default activity date field, so last activity before 2025-12-02. Identical set in both runs.                                                                                                                                                                                                                                                                                                                                                                                                                               |
| Shared input                     | Gathering ran once per run. Every agent got the same `selection.json`, the same 24 dossiers and the same `SKILL.md` (then named JUDGE.md) Step 3 instructions, with nothing else in its context.                                                                                                                                                                                                                                                                                                                                                                             |
| Tools                            | Read access to the checkout and to `gh`, so any agent could open the base branch or a diff. None could write.                                                                                                                                                                                                                                                                                                                                                                                                                                                                |
| Isolation                        | Every agent ran on its own, blind to the others' answers, and returned its verdicts as structured output.                                                                                                                                                                                                                                                                                                                                                                                                                                                                    |
| Run 1, model tiers               | Run `20260918-051451-clean-slate`, 2026-09-18. One agent each of `claude-fable-5-1`, `claude-sonnet-5` and `claude-haiku-4-5` at their default effort. The Opus row is taken from run 2 at medium effort, which is the closest to a default.                                                                                                                                                                                                                                                                                                                                 |
| Run 2, Opus effort               | Run `20260918-060944-effort-sweep`, 2026-09-18. Five `claude-opus-5` agents at once, one per reasoning effort: low, medium, high, xhigh, max. Each transcript records the effort that was applied, and the thinking-token column below rises with it, so the setting demonstrably took hold.                                                                                                                                                                                                                                                                                 |
| Run 3, OpenAI models under Codex | Run `20260918-063128-codex-bench`, 2026-09-18, driven by Codex following the recipe at the end of this file, same 24 pull requests, base pinned to `99a81bd32`. One fresh Codex session per model and effort; eighteen completed before the account's token limit stopped it, and the remainder was not run, since the best family's score had not moved across four effort levels and the missing tiers could at most equal it. Tokens come from Codex's own accounting, full-rate input and output, which is the nearest equivalent to the basis used for the Claude rows. |

**Run 1: which model, at default effort.**

| Model               | Verdicts that held up | Undecided | Input tokens | Output tokens | Model turns | Wall clock |
| ------------------- | --------------------- | --------- | ------------ | ------------- | ----------- | ---------- |
| Opus, medium effort | 21 of 24              | 1         | 264,070      | 12,080        | 35          | 5m 20s     |
| Fable               | 23 of 24              | 0         | 510,627      | 48,680        | 112         | 11m 46s    |
| Sonnet              | 22 of 24              | 1         | 452,851      | 68,776        | 87          | 13m 40s    |
| Haiku               | 12 of 24              | 6         | 171,021      | 9,150         | 47          | 2m 40s     |

**Run 2: Opus, by reasoning effort.**

| Effort   | Verdicts that held up | Undecided | Input tokens | Output tokens | Thinking tokens | Model turns | Wall clock |
| -------- | --------------------- | --------- | ------------ | ------------- | --------------- | ----------- | ---------- |
| low      | 21 of 24              | 1         | 288,222      | 11,277        | 4,270           | 34          | 3m 41s     |
| medium   | 21 of 24              | 1         | 264,070      | 12,080        | 4,946           | 35          | 5m 20s     |
| **high** | **23 of 24**          | 0         | 307,788      | 21,766        | 9,771           | 84          | 6m 19s     |
| xhigh    | 22 of 24              | 0         | 342,970      | 29,160        | 12,913          | 102         | 8m 18s     |
| max      | 24 of 24              | 0         | 530,009      | 40,776        | 22,188          | 102         | 10m 12s    |

**Run 3: OpenAI models under Codex, by reasoning effort.**

| Model           | Effort | Verdicts that held up | Undecided | Full-rate input | Output tokens | Reasoning tokens | Model turns | Wall clock |
| --------------- | ------ | --------------------- | --------- | --------------- | ------------- | ---------------- | ----------- | ---------- |
| gpt-5.5         | low    | 12 of 24              | 6         | 40,754          | 3,958         | 596              | 5           | 1m 25s     |
| gpt-5.5         | medium | 19 of 24              | 0         | 108,067         | 10,220        | 2,779            | 20          | 4m 01s     |
| gpt-5.5         | high   | 16 of 24              | 1         | 115,924         | 5,604         | 1,875            | 8           | 11m 30s    |
| gpt-5.6-luna    | low    | 13 of 24              | 4         | 36,497          | 4,302         | 453              | 3           | 1m 28s     |
| gpt-5.6-luna    | medium | 19 of 24              | 0         | 85,749          | 9,265         | 3,356            | 20          | 4m 10s     |
| gpt-5.6-luna    | high   | 20 of 24              | 0         | 174,265         | 25,190        | 15,120           | 29          | 17m 13s    |
| gpt-5.6-sol     | low    | 19 of 24              | 0         | 65,547          | 4,858         | 1,137            | 10          | 1m 57s     |
| gpt-5.6-sol     | medium | 20 of 24              | 0         | 92,649          | 10,084        | 2,550            | 12          | 3m 46s     |
| gpt-5.6-sol     | high   | 20 of 24              | 0         | 107,623         | 10,602        | 4,826            | 15          | 4m 06s     |
| gpt-5.6-sol     | xhigh  | 20 of 24              | 0         | 167,793         | 15,187        | 5,836            | 36          | 6m 28s     |
| gpt-5.6-terra   | low    | 17 of 24              | 1         | 95,251          | 6,905         | 2,147            | 14          | 2m 44s     |
| gpt-5.6-terra   | medium | 17 of 24              | 3         | 93,701          | 7,828         | 3,511            | 11          | 2m 52s     |
| gpt-5.6-terra   | high   | 19 of 24              | 0         | 87,626          | 9,023         | 4,403            | 12          | 3m 15s     |
| gpt-5.6-terra   | xhigh  | 20 of 24              | 0         | 167,049         | 14,724        | 8,103            | 20          | 5m 15s     |
| **gpt-6-astra** | low    | **22 of 24**          | 0         | 107,228         | 4,747         | 340              | 15          | 3m 22s     |
| **gpt-6-astra** | medium | **22 of 24**          | 0         | 112,544         | 5,517         | 437              | 15          | 8m 52s     |
| **gpt-6-astra** | high   | **22 of 24**          | 0         | 149,692         | 9,891         | 3,027            | 20          | 9m 06s     |
| **gpt-6-astra** | xhigh  | **22 of 24**          | 0         | 145,599         | 12,716        | 3,258            | 20          | 35m 11s    |
| gpt-6-astra     | max    | failed, token limit   |           | 61,645          | 859           | 57               | 7           | 0m 40s     |

Not run: gpt-6-astra ultra; gpt-5.6-sol max and ultra; gpt-5.6-terra max and
ultra; gpt-5.6-luna max; and two families never started, gpt-reserve and
codex-auto-review. The account's token limit stopped the sweep there, and
finishing it was judged not worth the cost: gpt-6-astra had already scored 22 of
24 at every one of four efforts, so a fifth could only have matched it.

What each column means:

-   **Verdicts that held up** - the agent's verdict for that pull request
    matched the answer arrived at by opening the base branch and checking by
    hand. Out of the 24 every agent was given.
-   **Undecided** - verdicts of `unclear`, the ones that land under COULD NOT
    DETERMINE and leave the work to a person. Counted separately because they
    are not wrong answers, they are absent ones; the rest of the shortfall in
    each row is verdicts that were wrong.
-   **Input tokens** - input billed at the full rate, meaning new prompt content
    plus cache writes. Cache reads are excluded on purpose: they ran from 2M to
    15M across these agents and track how many turns an agent took and how long
    its conversation already was, rather than how hard the work is, so including
    them would compare the wrong thing.
-   **Output tokens** - everything the model generated, reasoning included.
-   **Thinking tokens** - the part of the output spent reasoning before
    answering. Shown for run 2 because it is the direct evidence that each
    effort setting applied.
-   **Model turns** - one request to the model. A reply is a turn, and every
    tool call plus reading its result costs another, so this is how much back
    and forth the judging took rather than a cost in itself.
-   **Wall clock** - start of judging to the finished verdicts, for one batch
    of 24. It tracks turns more than tokens, and it is time nobody has to sit
    and watch.
-   **Full-rate input** and **Reasoning tokens**, run 3 only - Codex's own
    accounting. Full-rate input is what was billed at the full rate, so it
    stands in for the input-tokens column; reasoning tokens are the part of the
    output spent thinking, as thinking tokens are for run 2.

Every figure in runs 1 and 2 is summed from that agent's own transcript by the
same script, which is what makes those rows comparable to one another; run 3 is
Codex's own accounting, as its column names say.

Read all of it with three caveats. It is one repository and one batch of 24, so
a one-verdict difference between rows is inside the noise. The answer key was
settled by hand only where agents disagreed, so a mistake every agent made in
the same way would not show up; run 2 demonstrated the risk in the other
direction, when all five efforts disagreed with the key on one pull request, the
key was re-checked and found wrong, and the scores in both tables moved by one
as a result. Unanimous disagreement between independent judges is worth acting
on; it is how the key was corrected, and it is the cheapest error check this
tool has. And the Opus row in run 1 is a stand-in: Opus was not run at a default
in run 1, so its medium-effort result from run 2 sits there instead.

What the two runs show together is the shape of the failures. A weaker model, or
the same model thinking less, mostly declines to reach a verdict rather than
inventing a wrong one, so its cost lands under COULD NOT DETERMINE for a person
to finish. Haiku also got six outright wrong, which is worse than abstaining,
and one of those was a wrong `close`. Effort moves the same dial as tier: low
and medium each left one undecided and missed the same two, high cleared the
undecided and missed one, and max missed nothing. Low's misses included a wrong
`close`; from high upward no setting produced one. The one pull request that
separated high from max was a live duplicate that only the max agent went
looking for, and it is the kind of miss that lands under LEAVE OPEN, where
nobody re-checks.

The OpenAI runs sharpen the picture, because every one of them missed the same
two pull requests. All eighteen called #41084 `keep`: not one went looking for
the live pull request that supersedes it, so the SUPERSEDED heading was reached
only by Claude models in this data. All eighteen called #40889 `rewrite`,
reading the diff literally as "disable workflows" rather than reading the goal
it served, which the base branch already carries. Beyond those two, the misses
cluster on the two traps the Claude runs had already exposed: fourteen of
eighteen fell for the call that exists in the file but not in the function that
matters (#40012), and thirteen missed the equivalent guard written with a
different error code (#40191). The best OpenAI model, gpt-6-astra, scored 22 of
24 at low, medium, high and xhigh alike, with the same two misses each time: for
that model, effort bought nothing, where for Opus it moved the score from 21
to 24. Its low setting is the cheapest 22 of 24 anywhere in these tables, at
about 107,000 full-rate input and 4,700 output tokens in under three and a half
minutes.

Codex also re-checked the answer key where its models disagreed with it, as the
recipe asks, and retained every entry it examined, with evidence saved beside
each: the seven it re-opened include both entries this file records as having
been corrected earlier, #40012 and #40191, and the live-duplicate finding on
#41084. That is an independent confirmation of the key by a different vendor's
models reading the same pinned tree.

On cost, effort changed output far more than input: from low to max, input rose
1.8 times while output rose 3.6 times and wall clock 2.8 times, because reading
the same dossiers and the same files costs about the same however hard the model
thinks about them. It is also why the cap is 25, for the reason set out under
Cost.

## Cost

Not money: the script buys nothing and calls no model.
`matter_pr_triage.py cost` reports what a run spent, for one `--run` or for
everything on the repository. Time, API calls and rate-limit waits are measured
directly. Tokens need more care, because the script spends none: the agent doing
the judging does.

By default `cost` estimates the token side from the payload it hands over, at
roughly four characters per token. That is portable, works under any agent, and
is a floor rather than a total. For a real figure, point it at a Claude Code
session transcript, which records the usage the API reported for every message:

```
matter_pr_triage.py cost --run <run-id> --session ~/.claude/projects/<project>/<session-id>.jsonl
```

Two real batches on connectedhomeip, both judged by Opus:

| Per pull request | 37 in one batch, judged from signals | 15 revisited, each read against master |
| ---------------- | ------------------------------------ | -------------------------------------- |
| Fresh input      | 1,577                                | 25,766                                 |
| Output           | 1,199                                | 8,369                                  |
| Cache read       | 717,000                              | 1,674,000                              |

The gap between the columns is the whole point: a verdict that follows from the
gathered signals is cheap, and one that needs a human-style investigation costs
roughly sixteen times more in fresh input. Depth drives cost, not the number of
pull requests.

Two things to know before quoting any of this. Cache reads dominate the volume
but are billed at a fraction of fresh input, and they scale with how long the
conversation already is rather than with the triage, so the same run in its own
fresh session reads far less than one at the end of a long day. Fresh input and
output are the figures that travel. And these are Opus numbers on one
repository; a repository with larger diffs will differ.

At the recommended setting, Opus at high effort, a full independent judging pass
came to about 12,800 input and 900 output tokens per pull request, so a batch of
25 is roughly 320,000 input, 23,000 output and six to seven minutes. Plan on
that, and see the effort table above for how it moves with the setting.

If you measure under Claude Code, take the figures from the transcript with
`cost --session`. The per-agent token total the harness prints when a delegated
agent finishes came out about three times lower than the transcript sum on every
run here, so whatever it aggregates, it is not input plus output and should not
be quoted as such.

The default cap is 25, and that number is now measured rather than guessed.
Judging a batch of 24 took up to 112 model turns and about 560,000 tokens of
input and output for the most thorough tier, so a batch of 50 would run to
roughly 230 turns and 1.2 million, which risks one batch outgrowing the judging
agent's context part-way through. 25 is the largest size seen to work end to end
on every tier. Raise it with `--cap` once `cost` tells you what your own
repository costs, which will be lower if its pull requests are smaller or if
most verdicts follow from the signals without anyone opening the base branch.

Dossiers carry only what a verdict can rest on. Comments from CI bots, CLA
checks, coverage bots and automated reviewers are dropped as they are gathered,
and the rest are capped; on connectedhomeip that is 55% less for the judging
agent to read, with no change to any verdict. Human review comments are kept,
because "superseded by #123" settles a case.

## Things the measurements taught

-   **Expect the first pass to carry a few wrong verdicts.** Both times a
    delivered batch was cross-checked, one verdict in it was wrong: a `close`
    for a call that did exist in the file but not in the function that mattered,
    and a `close` for a port that covered half of what the pull request added.
    That is a few percent, and both were the same shape, a real match in the
    wrong place. Confirming each DECIDE entry before acting catches these; STILL
    NEEDED is where the rest would hide.
-   **Matched boilerplate is not evidence.** A line found elsewhere on master
    may just be a shared assertion message, and a match outside the pull
    request's own files proves little. Open the pull request before acting on a
    close, which is why every title is a link.
-   **Same file plus similar title is usually a sweep, not a duplicate.** One
    author fixing several functions in one area produces the strongest possible
    signal while being nothing like a duplicate.
-   **Last-activity date is useless on connectedhomeip.** A bulk event reset it
    on every stale pull request, so selecting on activity matches almost
    nothing. Use creation date. The tool says so itself now: when nothing
    matches, `select` names the bulk-update case and points at
    `--date-field created`.

## Repeating the benchmark

The facts are cached, so re-judging costs only the judging. To run it on another
model or another agent:

1. `select --older-than 290d --include-triaged --name <label>` from the
   checkout, or with `--repo owner/name`, then `collect` and `signals` on the
   run it writes. Confirm the selection matches the 24 above.
2. Give the judging agent Step 3 of `.agents/skills/matter-pr-triage/SKILL.md`
   and only that, with read access to the checkout and `gh` and no way to write.
   Tell it to read the base branch at the commit in the run's `manifest.json`,
   `master_sha`, never at the branch name.
3. Have it return one verdict per pull request as structured output or a JSON
   file; do not let it run `report`, which would write state.
4. Score against the answer key, count undecided separately from wrong, and
   where several independent judges unanimously disagree with the key, re-check
   the key against the base commit. That is how the key above was corrected.
5. Take tokens from the agent's own transcript or accounting, input and output
   separately, and leave cache reads out.

Answer key for the 24, verified by hand against `99a81bd32`: #39510 keep, #39612
keep, #39805 keep, #39820 rewrite, #39824 keep, #39865 keep, #40007 close,
#40012 keep, #40191 close, #40266 keep, #40445 rewrite, #40528 keep, #40657
keep, #40887 close, #40889 close, #40914 keep, #41084 in-flight, #41108 keep,
#41165 rewrite, #41444 close, #41545 keep, #41555 keep, #41609 close, #41675
rewrite.

# matter-pr-triage-issues: the inferred tier

**Question.** When nothing on GitHub links an issue to a pull request, how often
does the corpus scoring put that issue in front of the judge? Ground truth is
free: pull requests with formal closing links. The link is hidden, mentions of
the linked number are masked out of the pull request's text, and the corpus is
scored from the pull request's title, body, commit messages, changed paths,
author, date and labels alone. Recall is per linked issue: at the read list of 8
the judge reads in full, and on the skim list of 60 titles.

**Corpus.** project-chip/connectedhomeip on 2026-09-21: 3,784 issues, 2,484 open
and 1,300 closed within 18 months, synced in 38 calls and 52 seconds. Bots
excluded from the pull request sample.

| Sample     | Pull requests scored | Linked issues | Recall at 8 | Recall at 60 | MRR  | Missed                                                                  |
| ---------- | -------------------- | ------------- | ----------- | ------------ | ---- | ----------------------------------------------------------------------- |
| seed 1, 60 | 43                   | 44            | 0.75        | 1.00         | 0.55 | none                                                                    |
| seed 2, 60 | 38                   | 40            | 0.80        | 0.975        | 0.59 | a flaky-test issue whose body is CI log links, fixed by a DNS-SD change |
| seed 3, 80 | 56                   | 59            | 0.83        | 0.98         | 0.60 | one                                                                     |

**Baseline, before tuning, seed 1:** recall at 8 was 0.66 and on the skim list,
then 40 titles, 0.75. What closed the gap, in order of effect: keeping every
title term in the query so a long body of file names does not crowd it out;
tokens that meet across spellings, test identifiers in canonical form, camel
case split into words, underscores as joiners, two-letter acronyms kept, light
stemming; a title-to-title overlap signal weighted by word rarity; an issue that
names the pull request by number or URL; a file named in the issue body that the
pull request touches; platform aliases so "nordic" meets "nrfconnect"; and a
longer skim list. What protected precision at the same time: same author, same
weeks and shared labels count only as boosters and never make a lead on their
own; camel-case identifiers common in stack traces are discounted; wording alone
needs two shared terms and a floor; the folder signal needs the file itself or a
folder at least three deep; and the read list is capped per kind of lead so one
family of sibling issues cannot fill it.

**Caveats.** Pull requests that link issues formally are the ones whose authors
write good descriptions, so the population is friendlier than the not-linked
case the tier exists for. The remaining misses are semantic, an issue described
only by symptoms and log links whose fix touched something else, and are out of
reach for lexical scoring. Repeat with
`matter_pr_triage_issues.py benchmark --sample 80 --seed 3` after `sync`, from a
clone or with `--repo owner/name`; a fresh seed draws a fresh sample.
