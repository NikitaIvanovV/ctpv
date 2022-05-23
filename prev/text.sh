if exists bat; then
    batcmd=bat
elif exists batcat; then
    batcmd=batcat
else
    batcmd=
fi

if [ -n "$batcmd" ]; then
    "$batcmd" --color always        \
              --style plain         \
              --paging never        \
              --terminal-width "$w" \
              --wrap character      \
              -- "$f"
elif exists highlight; then
    highlight --replace-tabs=4 --out-format=ansi \
              --style='pablo' --force -- "$f"
elif exists source-highlight; then
    source-highlight --tab=4 --out-format=esc \
              --style=esc256.style --failsafe -i "$f"
else
    cat "$f"
fi
