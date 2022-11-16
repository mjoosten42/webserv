# Config file format guide

## General
A typical directive is typically defined as:
`identifier <value> [more values...];`

This config format is very much inspired by the [NGINX config format](http://nginx.org/en/docs/beginners_guide.html#conf_structure).

## Identifiers

### Server block
The server block is where you define the server.

`server_name <hostname(s)>` - the hostnames of this particular server. ex.
`server_name www.example.org example.org;`

`listen <port>` - this server will listen at a specifed port TCP on IPv4.
ex. `listen 8080;`

`error_page <HTTP error status code> <filename to host>` - serve custom error page on said error.
ex. `error_page 404 not_found.html;`

`root <root directory to serve from>` - from this directory the root is served. Defaults to `html`.
ex. `root html;` will serve from the server from the `html` directory(relative from the webserv's directory)

`autoindex <on|off>` if no index is found, webserv will generate a HTML index with all the files in the folder.

### Location
The location block looks like this:
```
location <location> {
    ... more directives
}
```

It will specify the behaviour specific to the location specified.

`index <filename>` - this is the default index for this location. This file will be served when the location is requested without a filename. When omitted, defaults to `index.html`;
ex. `index hello.html`;

`redirect <location to redirect to>` - when this location is hit, it will do a 301 Moved Permanently to the URL specified.
ex. `redirect https://www.youtube.com/watch?v=dQw4w9WgXcQ;`

`cgi <file extensions>` - when in this location a file with the ending file extentions is called, it won't serve it normally but instead will execute it using [Common Gateway Interface](https://en.wikipedia.org/wiki/Common_Gateway_Interface).
ex. `cgi pl;` will use `.pl` files for CGI.

Example:
```
location /cgi-bin/ {
    autoindex on;
    cgi pl;
}
```
