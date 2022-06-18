#!/bin/sh

echo '.TS'
echo 'allbox;'
echo 'lb lb'
echo 'l li .'

printf '%s\t%s\n' 'File type' 'Programs'
sort | sed 's/\t/&T{\n/; s/$/\nT}/'

echo '.TE'
