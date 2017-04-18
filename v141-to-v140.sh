#!/bin/sh

find . -type f -iname "*vcxproj" | xargs perl -pi.bak -e 's/v141/v140/;'
