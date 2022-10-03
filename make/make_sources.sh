#!/bin/bash

echo -n "SOURCES =" > make/sources.mk;
find src -type f -name *.cpp | awk '{print "\t" $0 " \\"}' >> make/sources.mk ;

echo -n "DEPS =" > make/dependecies.mk;
find obj -type f -name *.d | awk '{print "\t" $0 " \\"}' >> make/dependecies.mk ;
