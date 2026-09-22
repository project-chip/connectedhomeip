# Command-line reference

Every flag both scripts accept, for the agent driving the phases or a person
doing it by hand. Each tool's `SKILL.md` under `.agents/skills/` maps plain
English to the common ones; this lists every flag the scripts accept and what it
means, grouped by the command that takes it.

## Every command

-   `--version` - print the tool version and exit.
-   `--checkout PATH` - the clone to read from when one is used. Defaults to the
    current directory; when that is not a clone of the repository, the API is
    used instead.
-   `--repo owner/name` - the repository. A github.com URL is accepted too and
    reduced to `owner/name`. Inferred from the clone's remotes when omitted, so
    it is needed only where there is no clone to infer it from.

## preflight

-   `--remote` - force the API even when a clone is here. Without it the mode is
    chosen for you: a clone of the repository being triaged is used when there
    is one, the API otherwise. Also the way to read the branch as it is now when
    the clone has fallen behind and you would rather not fetch.
-   `--fix` - apply the items preflight marked `autofix`: fetching a base branch
    that has fallen behind, and restoring a shallow clone's full history. Both
    write to your repository, so neither happens on its own.

## select

-   `--older-than 18mo` - the window, as an age: `365d`, `18mo`, `2y`. Measured
    from last activity unless `--date-field created` is given.
-   `--updated-before 2025-01-01` - the window as an absolute date instead of an
    age, applied to whichever date field is in use.
-   `--date-field created` - measure the window from when a pull request was
    opened rather than from its last activity. Needed where a bulk update has
    touched every pull request; the zero-match message says so when it happens.
-   `--cap N` - how many to judge in one batch. Default 25, the largest size
    measured end to end. Gathering a pull request is expensive once; anything
    re-run over the same pull requests reuses the cache.
-   `--name X` - labels the run, so its directory and its internal per-batch
    report carry the name rather than only a timestamp. It does not name the
    window report; `--name` on `report` does that.
-   `--continue-from RUN_ID` or `latest` - the next batch, reusing that run's
    query, window and cap. Already-triaged pull requests are skipped, which is
    what makes batches advance.
-   `--revisit unclear` - bring back pull requests recorded with that verdict.
    `unclear` means deferred rather than resolved, so those would otherwise
    never resurface. Accepts any verdict; repeatable.
-   `--include-triaged` - re-select everything already recorded in state,
    decided or not.
-   `--base-ref REF` - the branch coverage is judged against, for a repository
    whose default branch cannot be detected.
-   `--depth quick` - record that the run is signals-only. The report renders
    the signals whenever no judgment has been written, so a quick run is one
    that stops before judging.
-   `--remote` - force the API; pair with `--repo`. Falling back to the API
    happens on its own when there is no usable clone.
-   Narrower selection: `--state`, `--draft`, `--label`, `--exclude-label`,
    `--author`, `--exclude-author`, `--review`, `--include-bots`. Note that
    `--label` is the GitHub label filter; the flag that names a run is `--name`.

## collect

-   `--run RUN_ID` - which run.
-   `--refetch` - throw the cached facts away and pull them again. Re-judging
    needs no flag: the facts are cached, so rewriting the judgment and rendering
    again is enough.

## signals

-   `--run RUN_ID` - which run.
-   `--remote` - read evidence through the API instead of a checkout. Blame,
    file contents, file existence and the tree all work, so every check works
    and the two modes reach the same answers. A clone is only faster and cheaper
    in API calls.

## report

-   `--run RUN_ID` - render that one batch, which is internal and stays in the
    cache. Omit it to render the window reports into the reports folder: one
    file per window per day, over every judged batch of that window, newest
    verdict per pull request.
-   `--name X` - with no `--run`, names the newest window's report
    `pr-triage-X-as-of-<date>.md` instead of the automatic
    `pr-triage-<field>-before-<cutoff>-as-of-<date>.md`.
-   `--out PATH` - also save a copy at that path, a file or a directory. The
    copy in the reports folder is kept either way.
-   `--with-cost` - add a one-line cost footer. Off by default: it is about the
    tool, not the pull requests.
-   `--no-strict` - render even when the judgment fails validation.

## cost

-   `--run RUN_ID` - one run; omit for everything on the repository.
-   `--session PATH` - a Claude Code transcript to read measured token usage
    from, instead of estimating it. With `--run`, the window is taken from that
    run.
-   `--since`, `--until` - the transcript window, ISO 8601 to the second,
    overriding the run's own.

## runs

-   `--limit N` - how many recent runs to list. Default 10.

## forget

-   `--run RUN_ID` - the run to discard, so its pull requests can be triaged
    again. Cached facts are kept.
-   `--all` - discard every run for the repository.
-   `--yes` - actually do it. Without it, `forget` only reports what would go.

The cache is per user, not per agent. Every run under
`~/.cache/matter-pr-triage/<owner>/<repo>/` belongs to whoever runs the command,
including runs another agent made on the same machine, so `forget --all` removes
those too. Prefer `--run` when more than one agent shares a machine.

## Environment

-   `MATTER_PR_TRIAGE_CACHE` - where the cache lives, instead of
    `~/.cache/matter-pr-triage`. Give each agent its own value to keep their
    runs and state apart.
-   `MATTER_PR_TRIAGE_REPORTS` - where reports land, instead of
    `~/matter-pr-triage-reports`. Both tools write there, one subfolder per
    repository.

# matter-pr-triage-issues

The sibling's flags. Its commands take `--checkout` and `--repo` exactly as
above, and most take `--pr`.

## Every command

-   `--version` - print the tool version and exit.
-   `--checkout PATH` - the clone the repository is inferred from. Defaults to
    the current directory.
-   `--repo owner/name` - the repository, as `owner/name` or a github.com URL.
    Inferred from the clone, or from a pull request URL given to `--pr`, when
    omitted.
-   `--pr N` - the pull request: `40191`, `#40191` or its URL. One per
    invocation. Taken by `gather`, `report` and `forget`.

## gather

-   `--candidates N` - how many inferred candidates to keep per pull request.
    Default 8; each one costs the judge a read.
-   `--refetch` - ignore the per-issue cache and fetch every issue again.
-   `--no-search` - skip the inferred tier, so only issues linked, referencing
    or mentioned are gathered.
-   `--read ISSUE` - an issue to read in full as an inferred candidate, the
    judge's pick from the skim list. Repeatable.

## sync

-   `--full` - rebuild the corpus from scratch instead of fetching what changed
    since the last sync.
-   `--closed-months N` - how far back closed issues are kept. Default 18.

## benchmark

-   `--sample N` - how many pull requests with formal issue links to score.
    Default 60.
-   `--seed N` - the sampling seed, so a run can be repeated or a fresh sample
    drawn. Default 1.
-   `--k N` - the read-list size recall is measured at. Default 8, the read
    list's default size.
-   `--keep-mentions` - leave the linked issue numbers in the pull request text
    instead of masking them, to measure the whole pipeline rather than inference
    alone.

## report

-   `--out PATH` - also save a copy at that path, a file or a directory. The
    copy in the reports folder is kept either way.
-   `--no-strict` - render even when a judgment fails validation.

## list

-   `--from-triage VERDICT` - also list the pull requests the triage report gave
    this verdict, `close` for the ones recommended for closing, marking which
    are not done yet. Repeatable.

## cost

-   `--pr N` - narrow the summary to one pull request's entries. Without it,
    every command run against the repository.

## forget

-   `--yes` - actually remove the pull request's dossier and judgment. Without
    it, `forget` only reports what would go. The per-issue cache is kept.
