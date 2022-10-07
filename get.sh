#!/bin/bash

echo "GET / HTTP/1.1"  >> GET 
echo "HOST: localhost:8080" >> GET
echo "host: localhost:8080" >> GET

echo -en "/r\n" >> GET

echo "amogus" >> GET

nc localhost 8080 < GET
rm GET
