#!/bin/sh
echo -ne '\033c\033]0;FishBytes\a'
base_path="$(dirname "$(realpath "$0")")"
"$base_path/FishBytes.x86_64" "$@"
