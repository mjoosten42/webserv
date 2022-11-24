#!/usr/bin/env python3

import cgi, os
from os import path

def printAndExit(code, msg):
	print("Content-Type: text/plain")
	print("Status: " + str(code))
	print()
	print(msg)
	exit()

from urllib.parse import parse_qs

method = os.environ["REQUEST_METHOD"]
if method != "DELETE":
	printAndExit(405, "Request method is " +  method + ", must be DELETE")

qs = os.environ["QUERY_STRING"]

qs_list = parse_qs(qs) # return list of strings

if "file" not in qs_list:
	printAndExit(400, "Missing \"file\" key. Query string: " + str(qs_list))

file = qs_list["file"][0]
if not path.isfile(file):
	printAndExit(404, "Couldn't find \"" + file)

try:	
	os.unlink(file)
except OSError as error:
	printAndExit(404, "unlink: " + str(error))

print("Content-Type: text/html")
print("Status: 200")
print()
print ("""\
<html>
   <body>
      <p>File %s successfully deleted</p>
   </body>
</html>
""" % (file,))

