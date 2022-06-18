#!/bin/sh

echo "| File types | Programs |"
echo "| ---- | ---- |"

sort | sed 's/ /` `/g; s/^/| /; s/\t/ | `/; s/$/` |/'
echo
