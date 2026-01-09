#!/usr/bin/env python3
"""A minimal mock runner for local testing.

This runner reads stdin and emits a very small transformation of the input.
It's intentionally simple — real deployments should replace this with the real skill runner.
"""

import sys


def humanize_text(s: str) -> str:
    # Very naive simplification for local smoke testing:
    # - If input contains 'We shipped', convert to a short summary pattern
    s = s.strip()
    if "We shipped an update" in s or "app crashes" in s:
        return "Summary: App crashes after recent auth change; affect multiple browsers and OSes.\nRepro: Unknown — customers report crashes across browsers/OS.\nImpact: Multiple customers affected. Prioritize investigation.\nSuggested next steps:\n1. Roll back auth change if rollback is feasible.\n2. Collect crash logs and release a hotfix if rollback isn't possible."
    if "We're excited to announce a new feature" in s:
        return "Summary: Added personalized recommendations using the recommender service.\nBehavior: Recs computed in offline batch every 6 hours; UI displays top 5 results.\nImpact: Slight increase in CPU during batch window; no user-facing downtime expected."
    if "We might want to change the cache TTL" in s:
        return "Summary: Proposal to change cache TTL to improve performance.\nOpen question: Which TTL value (seconds) should we use and what workload was measured?"
    # Default: echo input unchanged (useful for command-preservation example)
    return s


if __name__ == "__main__":
    data = sys.stdin.read()
    out = humanize_text(data)
    sys.stdout.write(out)
