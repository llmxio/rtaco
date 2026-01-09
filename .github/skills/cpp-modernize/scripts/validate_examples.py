#!/usr/bin/env python3
"""Smoke-test runner for the `cpp-modernize` skill examples.

Usage:
  python validate_examples.py --examples ../references/examples.md --runner "python mock_runner.py"

The script expects a runner command that reads text from stdin and writes the skill's output to stdout.
Exit code: 0 if all tests pass, non-zero if any test fails.
"""

import argparse
import re
import subprocess
import sys
from difflib import unified_diff

RE_INPUT = re.compile(r"Input:\s*```text\n(.*?)\n```", re.S)
RE_EXPECTED = re.compile(r"Expected output:\s*```text\n(.*?)\n```", re.S)

# type: ignore


def parse_examples(path):
    txt = open(path, "r", encoding="utf-8").read()
    inputs = RE_INPUT.findall(txt)
    expecteds = RE_EXPECTED.findall(txt)
    if len(inputs) != len(expecteds):
        raise SystemExit(
            f"Error parsing examples: found {len(inputs)} inputs but {len(expecteds)} expected outputs"
        )
    examples = []
    for i, (inp, exp) in enumerate(zip(inputs, expecteds), start=1):
        examples.append({"id": i, "input": inp.strip(), "expected": exp.strip()})
    return examples


def run_runner(cmd, input_text, timeout=10):
    proc = subprocess.run(
        cmd,
        input=input_text.encode("utf-8"),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        shell=True,
        timeout=timeout,
    )
    stdout = proc.stdout.decode("utf-8").strip()
    stderr = proc.stderr.decode("utf-8").strip()
    return proc.returncode, stdout, stderr


def normalize(text):
    # Basic normalization: strip trailing spaces on lines and remove extra blank lines
    lines = [line.rstrip() for line in text.strip().splitlines()]
    # Remove leading/trailing blank lines
    while lines and lines[0] == "":
        lines.pop(0)
    while lines and lines[-1] == "":
        lines.pop()
    return "\n".join(lines)


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--examples", required=True, help="Path to examples.md")
    p.add_argument(
        "--runner",
        required=True,
        help='Runner command (reads stdin, writes stdout). e.g. "python mock_runner.py"',
    )
    p.add_argument("--timeout", type=int, default=10)
    p.add_argument(
        "--fuzzy",
        action="store_true",
        help="Allow fuzzy comparison (ignore repeated whitespace)",
    )
    args = p.parse_args()

    examples = parse_examples(args.examples)

    failures = 0
    for ex in examples:
        print(f"--- Test #{ex['id']} ---")
        rc, out, err = run_runner(args.runner, ex["input"], timeout=args.timeout)
        if err:
            print(f"Runner stderr:\n{err}\n")
        if rc != 0:
            print(f"Runner exited with code {rc}")
        expected = ex["expected"]
        actual = out
        if args.fuzzy:
            # collapse whitespace
            expected_c = " ".join(expected.split())
            actual_c = " ".join(actual.split())
        else:
            expected_c = normalize(expected)
            actual_c = normalize(actual)

        if expected_c == actual_c:
            print("PASS\n")
        else:
            print("FAIL")
            print("--- Expected ---")
            print(expected)
            print("--- Actual ---")
            print(actual)
            print("--- Diff ---")
            for line in unified_diff(
                expected.splitlines(), actual.splitlines(), lineterm=""
            ):
                print(line)
            print("")
            failures += 1

    if failures:
        print(f"{failures} test(s) failed")
        sys.exit(2)
    else:
        print("All tests passed")
        sys.exit(0)


if __name__ == "__main__":
    main()
