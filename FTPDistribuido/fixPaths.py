#!/usr/bin/env python3

from functools import reduce
from sys import argv

if __name__ == "__main__":
    _, file, *patterns = argv

    content = ""
    with open(file) as f:
        content = f.read()

    content = reduce(
        lambda s, pattern: s.replace(f"/{pattern}", pattern), patterns, content
    )

    with open(file, "w") as f:
        f.write(content)
