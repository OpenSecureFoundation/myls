#!/bin/sh

set -u

ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
BIN="$ROOT/myls"
TMP=$(mktemp -d)

cleanup() {
	rm -rf "$TMP"
}
trap cleanup EXIT INT TERM

if ! command -v valgrind >/dev/null 2>&1; then
	echo "SKIP: valgrind not installed"
	exit 0
fi

mkdir -p "$TMP/dossier/sub"
printf "alpha\n" > "$TMP/dossier/file1"
printf "hidden\n" > "$TMP/dossier/.hidden"
ln -s file1 "$TMP/dossier/link"

valgrind --leak-check=full --error-exitcode=99 "$BIN" -la "$TMP/dossier"
valgrind --leak-check=full --error-exitcode=99 "$BIN" -R "$TMP/dossier"
