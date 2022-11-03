#!/usr/bin/env python3

import cgi, os

upload_dir = os.environ["UPLOAD_DIR"]

try:
	os.makedirs(upload_dir)
except:
	exit(1)

form = cgi.FieldStorage()

try:
	fileitem = form["userfile"]
except:
	exit(1)

message = 'No file was uploaded'
if fileitem.filename:
	fn = os.path.basename(fileitem.filename)
	try:
		f = open(upload_dir + "/" + fn, 'wb')
		f.write(fileitem.file.read())
		f.close()
		message = 'The file "' + fn + '" was uploaded successfully'
	except OSError:
		print("open: " + fn)

print("Content-Type: text/html")
print("Location: " + "/uploads/" + fileitem.filename)
print()

   
print ("""\
<html>
   <body>
      <p>%s</p>
   </body>
</html>
""" % (message,))