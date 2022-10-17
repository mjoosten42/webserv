#!/bin/bash

echo -en "GET / HTTP/1.1\r\n" >> GET 
echo -en "GET / HTTP/1.1\r\n" >> GET 

nc localhost 8080 < GET
rm GET
