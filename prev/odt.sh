# libreoffice

office() {
	# File produced by libreoffice
	jpg="$(printf '%s\n' "$f" | sed 's|^.*/||; s|\..*$||')"

	libreoffice               \
		--headless            \
		--convert-to jpg "$f" \
		--outdir "$cache_d" >/dev/null &&
		mv "$cache_d/$jpg.jpg" "$cache_f"
}

convert_and_show_image office
