# Config file format guide

## General
Our configuration file format is very much inspired by the [NGINX config format](http://nginx.org/en/docs/beginners_guide.html#conf_structure).

It consists of 'simple directives' and 'block directives':

Simple directives are single- or multiline declarations of key value pairs ending in a semicolon.
The key is separated from the value by white spaces, and the values include anything up to the following semicolon.
If a closing semicolon is missing, the configuration file is considered invalid.

A typical simple directive could be defined as:
`identifier-key <value> [more values...];`

Block directives are directives that can contain directives. This means they may contain simple directives, or
more block directives. A block directive that contains other block directives is called a 'context'. Simple directives or block directives that are not contained by an explicit block directive are considered to be part of the 'main context'.
Except for the main context, the scope of all block directives must be bounded by curly braces ('{' and '}'). A mismatch of curly braces will render the configuration file invalid.

A typical block directive could be defined as:
```
block_type <additional params, such as the block's name> {
    ... more directives
}
```

By default, the configuration file is very permissive. E.g. if the user forgets to close a simple directive with a semicolon, the following simple directive will be considered part of the previous' value - this allows for multiline simple directives. If the user enters a key that is not used in the setting up of the server, it will simply be ignored rather than flagged as invalid. This design choice lets us easily add more functionality in future, and allows for a degree of compatibility with standard NGINX configs (though a number of values have been implemented differently where necessary).
As a trade-off, this permissiveness does require extra diligence from the user in the formatting of custom configuration files.

## Identifiers

### Server block
The server block is where you define the server.

`server_name <hostname(s)>` - the hostnames of this particular server. ex.
`server_name www.example.org example.org;`

`listen <port>` - this server will listen at a specifed port TCP on IPv4.
ex. `listen 8080;`


### Location
The location block looks like this: 
```
location <location> {
    ... more directives
}
```

It will specify the behaviour specific to the location specified.

All of the following directives can also be placed in the server block. If a property isn't specified in a location block, then it will look in the parent server block for a value. If there none is found, the default value is used.

`error_page <HTTP error status code> <filename to host>` - serve custom error page on said error. Default none.
ex. `error_page 404 not_found.html;`

`root <root directory to serve from>` - from this directory the root is served. Defaults to `html`.
ex. `root html;` will serve from the server from the `html` directory(relative from the webserv's directory)

`autoindex <on|off>` if no index is found, webserv will generate a HTML index with all the files in the folder. Default is `off`.

`index <filename>` - this is the default index for this location. This file will be served when the location is requested without a filename. When omitted, defaults to `index.html`
ex. `index hello.html;`

`redirect <location to redirect to>` - when this location is hit, it will do a 301 Moved Permanently to the URL specified.
ex. `redirect https://www.youtube.com/watch?v=dQw4w9WgXcQ;`

`cgi <file extensions>` - when in this location a file with the ending file extentions is called, it won't serve it normally but instead will execute it using [Common Gateway Interface](https://en.wikipedia.org/wiki/Common_Gateway_Interface).
ex. `cgi pl;` will use `.pl` files for CGI.

`limit_except <HTTP methods>` - specifies which HTTP methods are allowed for this route.
ex. `limit_except GET POST;`

`client_max_body_size <max body size in bytes>` - defines the maximum body size a client request can have. Defaults to unlimited.
ex. `client_max_body_size 1024;`

Example:
```
location /cgi-bin/ {
    autoindex on;
    cgi pl;
    limit_except GET;
}
```
