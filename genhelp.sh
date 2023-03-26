#!/bin/sh

export GROFF_NO_SGR=1

groff -t -man -mtty-char "$1" -Tascii |
	col -bx |
	sed '/OPTIONS/,/^[A-Z]/!d; /^[A-Z]/d'
