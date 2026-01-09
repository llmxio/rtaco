# Examples and Tests

This file contains example prompts and expected outputs to help validate the `humanize` skill. Use these for manual checks or to create automated validation scripts.

## Example 1 — Bug report

Input:

```text
We shipped an update and now a few customers report that the app crashes. We're seeing reports across browsers and OSes, but it's probably something to do with the new auth changes. Can someone look into it and prioritize? Thx!
```

Expected output:

```text
Summary: App crashes after recent auth change; affect multiple browsers and OSes.
Repro: Unknown — customers report crashes across browsers/OS.
Impact: Multiple customers affected. Prioritize investigation.
Suggested next steps:
1. Roll back auth change if rollback is feasible.
2. Collect crash logs and release a hotfix if rollback isn't possible.
```

## Example 2 — Release note (marketing → engineer)

Input:

```text
We're excited to announce a new feature that will significantly enhance user engagement by leveraging our state-of-the-art recommendation engine. This will allow users to have a more personalized experience and open up new possibilities for cross-selling.
```

Expected output:

```text
Summary: Added personalized recommendations using the recommender service.
Behavior: Recs computed in offline batch every 6 hours; UI displays top 5 results.
Impact: Slight increase in CPU during batch window; no user-facing downtime expected.
```

## Example 3 — Command preservation

Input:

```text
Run `db-migrate --to v12 --dry-run` and check for failing migrations. If anything fails, run `db-migrate --to v12`.
```

Expected output:

```text
Run `db-migrate --to v12 --dry-run` and check for failing migrations. If anything fails, run `db-migrate --to v12`.
```

## Example 4 — Ambiguity flagging

Input:

```text
We might want to change the cache TTL to improve performance.
```

Expected output:

```text
Summary: Proposal to change cache TTL to improve performance.
Open question: Which TTL value (seconds) should we use and what workload was measured?
```

## Quick validation script idea

Create a small script that loads `examples.md`, runs the skill on each `Input`, and verifies the output matches `Expected output` exactly (or within configurable fuzziness). This can be used as a smoke test when packaging or updating the skill.

### Running the smoke tests

A validation script is provided at `scripts/validate_examples.py`. It expects a runner command that reads the example input from stdin and writes the skill output to stdout.

Example (local mock runner):

```bash
python scripts/validate_examples.py --examples references/examples.md --runner "python scripts/mock_runner.py"
```

If you have a real skill runner (the runtime that executes the skill), pass it as `--runner` instead of the mock runner. The script exits with a non-zero code if any example fails.
