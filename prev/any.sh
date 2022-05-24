if exists exiftool; then
	exiftool "$f"
else
	cat "$f"
fi
