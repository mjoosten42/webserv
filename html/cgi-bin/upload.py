#!/usr/bin/env python3

import cgi, os

def printAndExit(code, msg):
	print("Content-Type: text/plain")
	print("Status: " + str(code))
	print()
	print(msg)
	exit()

method = os.environ["REQUEST_METHOD"]
if method != "POST":
	printAndExit(405, "Request method is " +  method + ", must be POST")

upload_dir = os.environ["UPLOAD_DIR"]

if not os.path.isdir(upload_dir):
	try:
		os.makedirs(upload_dir)
	except OSError as error:
		printAndExit(403, error)

form = cgi.FieldStorage()

try:
	fileitem = form["file"]
except:
	printAndExit(400, "No key names \"file\"")

message = ""
if fileitem.filename:
	fn = os.path.basename(fileitem.filename)
	try:
		f = open(upload_dir + "/" + fn, 'wb')
		f.write(fileitem.file.read())
		f.close()
	except OSError as error:
		printAndExit(403, error)
	message = 'The file "' + fn + '" was uploaded successfully'

print("Content-Type: text/html")
print("Status: 201")
print()
print ("""\
<html>
   <body>
      <p>%s</p>
   </body>
</html>
""" % (message,))
