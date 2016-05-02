#!/bin/sh

find . -type f -iname "*vcxproj" | xargs perl -pi.bak -e 's/v140/v120/;'
