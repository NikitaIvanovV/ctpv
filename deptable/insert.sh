#!/bin/sh

set -e

tmp="$(mktemp XXXXXX)"
table="$(mktemp XXXXXX)"
trap 'rm "$table"' EXIT

cat > "$table"

sed "
	/TABLESTART/,/TABLEEND/ {
		/TABLESTART/ {
			r $table
			b
		}
		/TABLEEND/!d
	}
" "$1" > "$tmp"

mv -- "$tmp" "$1"
