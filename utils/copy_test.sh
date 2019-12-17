#!/bin/sh

for i in test/*_gml.res; do
    name=$(echo $i | sed 's/...$/sol/')
    cp $i $name
done
