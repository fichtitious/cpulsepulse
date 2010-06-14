#!/bin/bash

set -e

if [ -d "test/testcpulsepulse" ]; then
    rm test/testcpulsepulse
fi

g++ src/cpulsepulse.cpp test/test.cpp -lpulse-simple -o test/testcpulsepulse
test/testcpulsepulse
