# font: fontimage

font() {
	fontimage -o "$cache_f.png" "$f" 2> /dev/null &&
		mv -- "$cache_f.png" "$cache_f"
}

convert_and_show_image font
