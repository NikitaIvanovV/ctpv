# office: libreoffice

doc() {
	# File produced by libreoffice
	jpg="$(printf '%s\n' "$f" | sed 's|^.*/||; s|\..*$||')"

	libreoffice \
		--headless \
		--convert-to jpg \
		--outdir "$cache_d" \
		"$f" >/dev/null &&
		mv -- "$cache_d/$jpg.jpg" "$cache_f"
}

convert_and_show_image doc
