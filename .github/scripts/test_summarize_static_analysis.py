"""Regression coverage for successful empty and invalid analyzer reports."""

import contextlib
import importlib.util
import io
import os
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch


spec = importlib.util.spec_from_file_location(
    "static_analysis_summary", Path(__file__).with_name("summarize-static-analysis.py")
)
summary = importlib.util.module_from_spec(spec)
spec.loader.exec_module(summary)


class ReportTests(unittest.TestCase):
    def check_report(self, analyzer, content, expected_failure):
        with tempfile.TemporaryDirectory() as directory:
            build = Path(directory)
            filename = "clang-tidy.yaml" if analyzer == "clang-tidy" else "cppcheck.xml"
            if content is not None:
                (build / filename).write_text(content)
            with patch.dict(os.environ, {"GITHUB_STEP_SUMMARY": ""}):
                with contextlib.redirect_stdout(io.StringIO()):
                    failed = summary.summarize(build, analyzer)
            self.assertEqual(failed, expected_failure)
            report = (build / "summary.md").read_text()
            if expected_failure:
                self.assertIn("### Analysis failures", report)
            else:
                self.assertIn(f"| {analyzer} | 0 |", report)

    def test_zero_byte_clang_tidy_export_is_a_clean_report(self):
        # run-clang-tidy-18 writes zero bytes when no diagnostics are merged.
        self.check_report("clang-tidy", "", False)

    def test_empty_clang_tidy_diagnostics_list_is_a_clean_report(self):
        self.check_report("clang-tidy", "Diagnostics: []\n", False)

    def test_missing_clang_tidy_report_fails(self):
        self.check_report("clang-tidy", None, True)

    def test_nonempty_invalid_clang_tidy_reports_fail(self):
        for content in ("null\n", "[]\n", "{}\n", " \n", "Diagnostics: {}\n", "Diagnostics: ["):
            with self.subTest(content=content):
                self.check_report("clang-tidy", content, True)

    def test_clang_tidy_parse_errors_fail(self):
        self.check_report(
            "clang-tidy",
            "Diagnostics:\n- DiagnosticName: clang-diagnostic-error\n"
            "  DiagnosticMessage:\n    Message: failed to parse\n",
            True,
        )

    def test_missing_cppcheck_report_fails(self):
        self.check_report("cppcheck", None, True)

    def test_malformed_cppcheck_reports_fail(self):
        for content in ("", "<results><errors>", "<not-results />"):
            with self.subTest(content=content):
                self.check_report("cppcheck", content, True)


if __name__ == "__main__":
    unittest.main()
