#!/usr/bin/env python3

import os
import re
import subprocess
import sys

def searchErrorLine(filename):
    lineNo = 0

    with open(filename, "r") as fp:
        for line in fp:
            lineNo += 1
            if "// ERROR" in line:
                return lineNo

    raise Exception(filename + ": Error token `// ERROR` not found")


def doTest(filename):
    process = subprocess.Popen(
        ("bass", "-strict", "-o", os.devnull, filename), 
        stdout=subprocess.PIPE)
    out = process.communicate()[0].decode('utf-8')

    if process.returncode == 0:
        raise Exception(filename + ": bass did not fail")

    errorLine = searchErrorLine(filename)

    lastError = out.splitlines()[-2]
    match = re.match("   " + re.escape(filename) + r":(\d+):", lastError)

    if match is None or int(match.group(1)) != errorLine:
        raise Exception(filename + ": did not error out on marked line")


if __name__ == "__main__":
    for arg in sys.argv[1:]:
        doTest(arg)
