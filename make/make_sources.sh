#!/bin/bash

echo "SOURCES = \\" > make/sources.mk;
find src -type f -name *.cpp | awk '{print "\t" $0 " \\"}' >> make/sources.mk ;

echo "HEADERS = \\" > make/headers.mk;
find include -type f -name *.hpp | awk '{print "\t" $0 " \\"}' >> make/headers.mk ;

