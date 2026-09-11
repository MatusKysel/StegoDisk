#!/usr/bin/env python3
"""Report advisory findings while rejecting incomplete or failed analysis."""

import argparse
import os
from collections import Counter
from pathlib import Path
import sys
import xml.etree.ElementTree as ET

import yaml


def summarize(build_dir, analyzer):
    counts = {}
    failures = []
    if analyzer == "clang-tidy":
        try:
            content = (build_dir / "clang-tidy.yaml").read_text()
            # run-clang-tidy-18 creates a zero-byte export when no diagnostics
            # are merged. The workflow gates analyzer failures separately.
            report = {"Diagnostics": []} if content == "" else yaml.safe_load(content)
            if not isinstance(report, dict) or not isinstance(report.get("Diagnostics"), list):
                raise ValueError("expected a clang-tidy Diagnostics list")
            diagnostics = report["Diagnostics"]
            # A header can be diagnosed from several translation units.
            unique = {
                (
                    item["DiagnosticName"],
                    item["DiagnosticMessage"].get("FilePath", ""),
                    item["DiagnosticMessage"].get("FileOffset", 0),
                    item["DiagnosticMessage"]["Message"],
                )
                for item in diagnostics
            }
            counts["clang-tidy"] = Counter(item[0] for item in unique)
            if counts["clang-tidy"]["clang-diagnostic-error"]:
                failures.append("clang-tidy could not parse one or more source files.")
        except (OSError, ValueError, KeyError, TypeError, yaml.YAMLError) as error:
            failures.append(f"clang-tidy report is missing or invalid: {error}")

    if analyzer == "cppcheck":
        try:
            report = ET.parse(build_dir / "cppcheck.xml").getroot()
            if report.tag != "results" or report.find("errors") is None:
                raise ValueError("expected a complete cppcheck results document")
            diagnostics = report.findall("errors/error")
            unique = {
                (
                    item.attrib["id"],
                    tuple(
                        (loc.get("file", ""), loc.get("line", "0"))
                        for loc in item.findall("location")
                    ),
                    item.attrib["msg"],
                )
                for item in diagnostics
                if item.get("severity") != "information"
            }
            counts["cppcheck"] = Counter(item[0] for item in unique)
            # These indicate incomplete analysis, rather than a source-code finding.
            fatal_ids = {
                "cppcheckError", "internalError", "internalAstError",
                "instantiationError", "preprocessorErrorDirective",
                "syntaxError", "unknownMacro",
            }
            for item in diagnostics:
                if item.get("id") in fatal_ids:
                    failures.append(f"cppcheck {item.get('id')}: {item.get('msg')}")
                location = item.find("location")
                position = "" if location is None else (
                    f"{location.get('file')}:{location.get('line')}: "
                )
                print(f"{position}cppcheck [{item.get('id')}]: {item.get('msg')}")
        except (OSError, ValueError, KeyError, ET.ParseError) as error:
            failures.append(f"cppcheck report is missing or invalid: {error}")

    lines = [
        f"## {analyzer} analysis", "",
        "Findings are **advisory** while the existing backlog is reviewed. "
        "A successful job does not mean the source is free of findings. "
        "Tool, configuration and parsing failures fail the job.", "",
        "Scope: Linux Debug project sources and their headers; bundled `lib/` "
        "translation units and tests are excluded.", "",
        "| Analyzer | Unique findings |", "| --- | ---: |",
    ]
    checks = counts.get(analyzer)
    count = str(sum(checks.values())) if checks is not None else "report unavailable"
    lines.append(f"| {analyzer} | {count} |")
    if checks:
        lines.extend(["", "### Findings by check", ""])
        lines.extend(f"- `{check}`: {count}" for check, count in sorted(checks.items()))
    if failures:
        lines.extend(["", "### Analysis failures", ""])
        lines.extend(f"- {failure}" for failure in failures)
    lines.extend(["", f"Full diagnostics are in the `{analyzer}-reports` artifact.", ""])
    summary = "\n".join(lines)
    (build_dir / "summary.md").write_text(summary)
    if os.environ.get("GITHUB_STEP_SUMMARY"):
        with open(os.environ["GITHUB_STEP_SUMMARY"], "a") as output:
            output.write(summary)
    print(summary)
    return bool(failures)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("build_dir", type=Path)
    parser.add_argument("analyzer", choices=("clang-tidy", "cppcheck"))
    arguments = parser.parse_args()
    sys.exit(summarize(arguments.build_dir, arguments.analyzer))
