"""Regression coverage for the changed-line clang-format check."""

import importlib.util
import subprocess
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch


spec = importlib.util.spec_from_file_location(
    "check_format", Path(__file__).with_name("check-format.py")
)
check_format = importlib.util.module_from_spec(spec)
spec.loader.exec_module(check_format)


DIFF = """diff --git a/src/a.cc b/src/a.cc
--- a/src/a.cc
+++ b/src/a.cc
@@ -3,0 +4,2 @@
+int added_two_lines;
+int and_another;
@@ -20 +22 @@
-int old_line;
+int replaced_one_line;
diff --git a/src/b.h b/src/b.h
--- a/src/b.h
+++ b/src/b.h
@@ -7,3 +8,0 @@
-int deleted_only;
"""


class ChangedLineRangeTests(unittest.TestCase):
    def ranges_for(self, diff):
        completed = subprocess.CompletedProcess([], 0, stdout=diff, stderr="")
        with patch.object(check_format.subprocess, "run", return_value=completed):
            return check_format.changed_line_ranges("HEAD", ["src"])

    def test_collects_ranges_from_post_image(self):
        # A hunk without a count covers exactly one line.
        self.assertEqual(self.ranges_for(DIFF)["src/a.cc"], [(4, 5), (22, 22)])

    def test_ignores_pure_deletions(self):
        # "+8,0" removes lines, so there is nothing to format in the result.
        self.assertNotIn("src/b.h", self.ranges_for(DIFF))

    def test_ignores_removed_files(self):
        diff = "--- a/src/gone.cc\n+++ /dev/null\n@@ -1,2 +0,0 @@\n-int gone;\n"
        self.assertEqual(self.ranges_for(diff), {})


class MainTests(unittest.TestCase):
    def run_main(self, contents, formatted_output):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "sample.cc"
            path.write_text(contents)
            ranges = {str(path): [(1, 1)]}
            with patch.object(
                check_format, "changed_line_ranges", return_value=ranges
            ), patch.object(
                check_format, "formatted", return_value=formatted_output.encode()
            ):
                return check_format.main(["HEAD", str(path)])

    def test_passes_when_already_formatted(self):
        self.assertEqual(self.run_main("int x;\n", "int x;\n"), 0)

    def test_fails_when_reformatting_would_change_the_file(self):
        self.assertEqual(self.run_main("int  x ;\n", "int x;\n"), 1)


if __name__ == "__main__":
    unittest.main()
