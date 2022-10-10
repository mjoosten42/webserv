#!/bin/bash

echo "GET / HTTP/1.1"  >> GET 
echo "HOST: localhost:8080" >> GET
echo "Connection: keep-alive" >> GET
echo -en "\r\n" >> GET
echo "GET / HTTP/1.1"  >> GET 
echo "HOST: localhost:8080" >> GET
echo "Connection: close" >> GET
echo -en "\r\n" >> GET

nc localhost 8080 < GET
rm GET
