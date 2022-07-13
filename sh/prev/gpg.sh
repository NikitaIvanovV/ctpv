# gpg-encrypted: gpg

# "showgpg" option must be enabled for this preview to work
[ -z "$showgpg" ] && exit 127

gpg -d -- "$f" 2>&1
