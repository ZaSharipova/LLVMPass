#!/usr/bin/env bash
set -e

process_module() {
    local name="$1"
    shift
    local sources=("$@")

    echo -e "\n\033[34mWorking on: ${sources[*]} -> ${name}\033[0m"

    MYPASS_DOT_FILE="dots/${name}.dot"          \
    MYPASS_MAP_FILE="dots/${name}_mapping.txt"  \
    ./scripts/mypass-clang.sh                   \
        "${sources[@]}" build/obj/Runtime.o     \
        -o "build/bin/${name}_inst"

    MYPASS_LOG_FILE="dots/${name}_log.txt" \
    "./build/bin/${name}_inst"

    MYPASS_LOG_FILE="dots/${name}_log.txt"  \
    python3 scripts/Annotate.py             \
        "dots/${name}.dot"                  \
        "dots/${name}_annotated.dot"
}

if [ $# -eq 0 ]; then
    for src in tests/*.c; do
        name=$(basename "$src" .c)
        process_module "$name" "$src"
    done
else
    for arg in "$@"; do
        if [ -d "$arg" ]; then
            name=$(basename "$arg")
            sources=("$arg"/*.c)
            process_module "$name" "${sources[@]}"
        elif [ -f "$arg" ]; then
            name=$(basename "$arg" .c)
            process_module "$name" "$arg"
        else
            echo "Error: $arg is not a file or directory." >&2
            exit 1
        fi
    done
fi

echo -e "\n\n\033[32mDone! \033[0m"
