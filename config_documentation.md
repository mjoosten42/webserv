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

### General Caveats
By default, the configuration file is very permissive. 
As a trade-off, this permissiveness does require extra diligence from the user in the formatting of custom configuration files.

If the user forgets to close a simple directive with a semicolon, the following simple directive will be considered part of the previous' value - this allows for multiline simple directives. 

If the user enters a key that is not used in the setting up of the server, it will simply be ignored rather than flagged as invalid. This design choice lets us easily add more functionality in future, and allows for a degree of compatibility with standard NGINX configs (though a number of config options have been implemented differently from NGINX where necessary). This documentation will provide examples of where our implementation does differ from standard NGINX config implementation, so compatibility should not be taken as a hard guarantee.

Additionally, most values that are expected within a certain context are initialised by a default value (in line with the NGINX default), and merely overwritten if they are explicitly expressed in the config file.

## Identifiers

### Server block
The server block is a block directive where you define a server.
Listed below are the directives it may contain that can be recognised by the configuration parser.

`server_name <hostname(s)>` - The hostnames of this particular server. The expected value is one or more strings.
E.g. `server_name www.example.org example.org;`
When this value is not specified it defaults to ` ` (an empty string).

`listen <port>` - This server will listen at a specifed port TCP on IPv4.
E.g. `listen 8080;`
When this value is not specified it defaults to `8000`.
WARNING: NGINX difference - Nginx allows the user to parse the host from the listen option, however we standardly only accept localhost as host.

`root <root directory>` - The directory that this server is rooted to, and from where it will serve its files. This path is relative to the webserv's directory.
E.g. `root html;`
When this value is not specified it defaults to `html`.

`client_max_body_size <size>` - The maximum allowed size of the client request body. If zero, it will not be checked.
E.g. `client_max_body_size 1024;`
When this value is not specified it defaults to `0` (meaning it will not be checked).

`autoindex <on|off>` - When the user navigates to a directory, the server will serve an index page if it can find one in that directory. If it cannot, it will serve an automatically generated HTML index page listing all the files in that directory, or a 404 error, depending on whether autoindex is on or off respectively.
E.g. `autoindex on;`
When this value is not specified it defaults to `off`.

`index <file_name>` - When the user navigates to a directory, the server will serve an index page if it can find one in that directory. The filename it looks for to use as an index is the one specified by this directive. This takes precedence over auto-indexing, if this option is also enabled.
E.g. `index my_amazing_index.html;`
When this value is not specified it defaults to `index.html`.

`error_page <HTTP error status code> <filename to host> [<code> <page> ...]` - What page to serve in the event of an HTTP error matching the specified code.
E.g. `error_page 404 my_amazing_404_error_page.html;`
When this value is not specified it defaults to standard error pages for each of the encounterable error codes.
WARNING: NGINX difference - Nginx allows the user to specify multiple error codes per error page (e.g. `error_page 401 402 403 404 my_amazing_4XX_error_page.html;`). In our config they must be strictly alternating key value pairs, though can be written on multiple lines.


### Location block
The location block is a block directive that only exists within the context of a server block. Internally, it can be understood as a(n indirect) subdirectory of the server's root directory, but with its own set of rules and settings. These may overwrtie those inherited from the server context, resulting in location-specific behaviour. Whenever a user navigates to this part of the server, that specific location block's rules will apply. When the server tries to determine what location block the user is in, it looks for the longest possible match between the address the user navigated to, and the root + name of a location block it recognises from the config file.

In the configuration file it may look something like this: 
```
location </name_of_subdirectory> {
    ... more directives
}
```

The hierarchy for determining what rules apply to a location block is:
directives explicitly listed in that location block > 
directives explicitly listed in the parent server block >
default values for that directive.

As the location block inherits many of its directives from a parent server block, but can also overwrite them, the following directives may be specified in either a location block or a server block. To understand their individual usage, please refer to the above section on server blocks.

```
root
index
autoindex
error_page
client_max_body_size
```

`redirect <location to redirect to>` - when this location is hit, it will do a 301 Moved Permanently to the URL specified.
ex. `redirect https://www.youtube.com/watch?v=dQw4w9WgXcQ;`

`cgi <file extensions>` - when in this location a file with the ending file extentions is called, it won't serve it normally but instead will execute it using [Common Gateway Interface](https://en.wikipedia.org/wiki/Common_Gateway_Interface).
ex. `cgi pl;` will use `.pl` files for CGI.

`limit_except <HTTP methods>` - specifies which HTTP methods are allowed for this route.
ex. `limit_except GET POST;`

// write: upload

Example:
```
location /cgi-bin/ {
    autoindex on;
    cgi pl;
    limit_except GET;
}
```
