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
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE)
    pout, perr = process.communicate()
    pout = pout.decode('utf-8')
    perr = perr.decode('utf-8')

    if process.returncode == 0:
        raise Exception(filename + ": bass did not fail")

    errorLine = searchErrorLine(filename)

    # have to test both stderr and stdout as bass-ARM9 and bass-v14.05
    # output errors to different streams.

    regex = re.compile(r"^\s*" + re.escape(filename) + r":(\d+):", re.MULTILINE)

    match = None
    for match in regex.finditer(pout):
        pass
    for match in regex.finditer(perr):
        pass

    if match is None or int(match.group(1)) != errorLine:
        raise Exception(filename + ": did not error out on marked line")


if __name__ == "__main__":
    for arg in sys.argv[1:]:
        doTest(arg)
