#!/usr/bin/env python3
"""Check clang-format compliance of the lines a branch actually changed.

The tree predates .clang-format, so reformatting it wholesale would bury real
changes in noise. This restricts clang-format to the line ranges introduced by
the diff, which is what the upstream git-clang-format does; the pinned
clang-format wheel does not ship that script, so the line ranges are computed
here instead.
"""

import argparse
import re
import subprocess
import sys
from pathlib import Path

# "+++ b/src/foo.cc" or "+++ /dev/null" for a deletion.
FILE_RE = re.compile(r"^\+\+\+ (?:b/)?(.*)$")
# "@@ -12,3 +14,5 @@" where the second pair describes the post-image.
HUNK_RE = re.compile(r"^@@ -\d+(?:,\d+)? \+(\d+)(?:,(\d+))? @@")


def changed_line_ranges(base, paths):
    """Maps each changed file to the line ranges the diff introduced."""
    diff = subprocess.run(
        ["git", "diff", "-U0", "--no-color", "--diff-filter=d", base, "--", *paths],
        capture_output=True,
        text=True,
        check=True,
    ).stdout

    ranges = {}
    current = None
    for line in diff.splitlines():
        file_match = FILE_RE.match(line)
        if file_match:
            name = file_match.group(1)
            current = None if name == "/dev/null" else name
            continue
        hunk_match = HUNK_RE.match(line)
        if hunk_match and current:
            start = int(hunk_match.group(1))
            count = 1 if hunk_match.group(2) is None else int(hunk_match.group(2))
            # A zero-length post-image is a pure deletion: no lines to format.
            if count:
                ranges.setdefault(current, []).append((start, start + count - 1))
    return ranges


def formatted(binary, path, line_ranges):
    """Returns the file as clang-format would write those line ranges."""
    command = [binary, "--style=file", f"--assume-filename={path}"]
    command += [f"--lines={start}:{end}" for start, end in line_ranges]
    with open(path, "rb") as handle:
        return subprocess.run(
            command, stdin=handle, capture_output=True, check=True
        ).stdout


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("base", help="commit to diff against")
    parser.add_argument("paths", nargs="*", default=["src"], help="paths to check")
    parser.add_argument("--binary", default="clang-format")
    parser.add_argument(
        "--extensions",
        default="c,cc,cpp,h,hpp",
        help="comma separated list of extensions to check",
    )
    args = parser.parse_args(argv)

    suffixes = {"." + extension for extension in args.extensions.split(",")}
    ranges = changed_line_ranges(args.base, args.paths or ["src"])

    offenders = []
    for path, line_ranges in sorted(ranges.items()):
        if Path(path).suffix not in suffixes or not Path(path).exists():
            continue
        expected = formatted(args.binary, path, line_ranges)
        actual = Path(path).read_bytes()
        if expected != actual:
            offenders.append(path)
            diff = subprocess.run(
                ["diff", "-u", path, "-"],
                input=expected,
                capture_output=True,
            ).stdout.decode("utf-8", "replace")
            print(diff)

    if offenders:
        print(f"::error::{len(offenders)} file(s) have unformatted changed lines.")
        print("Reformat just those lines with:")
        print(f"  python3 {Path(__file__).name} {args.base} --binary {args.binary}")
        print("or format the whole file with: clang-format -i <file>")
        return 1

    print(f"Changed lines are correctly formatted ({len(ranges)} file(s) inspected).")
    return 0


if __name__ == "__main__":
    sys.exit(main())
