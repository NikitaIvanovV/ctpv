#!/bin/sh

table="$(mktemp XXXXXX)"
trap 'rm "$table"' EXIT

cat > "$table"

sed -i "
	/TABLESTART/,/TABLEEND/ {
		/TABLESTART/ {
			r $table
			b
		}
		/TABLEEND/!d
	}
" "$1"
