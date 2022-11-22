#!/usr/bin/env python3

import cgi, os

print("Content-Type: text/html")
print()

from urllib.parse import parse_qs

method = os.environ["REQUEST_METHOD"]

if method != "DELETE":
	exit(1)

qs = os.environ["QUERY_STRING"]

file = parse_qs(qs)

print("qs: " + qs)
print(file)
print(file["file"])

try:
	os.unlink(file["file"])
except OSError as error:
	print(error)
	exit(1)
