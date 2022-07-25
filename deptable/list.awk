#!/usr/bin/gawk -f

function perr(str) {
    printf "%s: %s\n", ARGV[0], str > "/dev/stderr"
}

function join(arr, sep,   res, s) {
    for (i in arr) {
        res = res s arr[i]
        if (s == "")
            s = sep
    }

    return res
}

function process_file(file,   i, line, arr, t, p, p_len) {
    i = getline line < file

    if (i == -1) {
        perr(ERRNO ": " file)
        exit
    } else if (i == 0) {
        return
    }

    if (match(line, /^#\s*([a-zA-Z0-9_-]+):\s*(.*)/, arr) == 0)
        return

    t = arr[1]
    p_len = split(arr[2], p, /\s+/)
    for (i = 1; i <= p_len; i++)
        types[t][types_len[t]++] = p[i]
}

BEGIN {
    for (i = 1; i < ARGC; i++)
        process_file(ARGV[i])

    for (t in types)
        s = s sprintf("%s\t%s\n", t, join(types[t], " "))

    printf "%s", s | "sort"
}
