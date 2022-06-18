# pdf: pdftoppm

pdf() {
	pdftoppm -f 1 -l 1              \
		-scale-to-x 1920            \
		-scale-to-y -1              \
		-singlefile                 \
		-jpeg -tiffcompression jpeg \
		-- "$f" "$cache_f" && mv -- "$cache_f.jpg" "$cache_f"
}

convert_and_show_image pdf
