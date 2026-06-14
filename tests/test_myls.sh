#!/bin/sh

set -u

ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
BIN="$ROOT/myls"
TMP=$(mktemp -d)
PASS=0
FAIL=0

cleanup() {
	rm -rf "$TMP"
}
trap cleanup EXIT INT TERM

make_fixture() {
	mkdir -p "$TMP/dossier/sub"
	printf "alpha\n" > "$TMP/dossier/file1"
	printf "beta\n" > "$TMP/dossier/file2.o"
	printf "hidden\n" > "$TMP/dossier/.hidden"
	: > "$TMP/dossier/empty"
	: > "$TMP/dossier/backup~"
	: > "$TMP/dossier/space file"
	printf "#!/bin/sh\nexit 0\n" > "$TMP/dossier/executable"
	chmod +x "$TMP/dossier/executable"
	ln -s file1 "$TMP/dossier/link"
}

pass() {
	printf "PASS: %s\n" "$1"
	PASS=$((PASS + 1))
}

fail() {
	printf "FAIL: %s\n" "$1"
	printf "  %s\n" "$2"
	FAIL=$((FAIL + 1))
}

compare_exact() {
	desc="$1"
	ls_cmd="$2"
	myls_cmd="$3"
	ls_out="$TMP/ls.out"
	myls_out="$TMP/myls.out"

	( cd "$TMP/dossier" && LC_ALL=C eval "$ls_cmd" > "$ls_out" 2>&1 )
	ls_code=$?
	( cd "$TMP/dossier" && LC_ALL=C eval "$myls_cmd" > "$myls_out" 2>&1 )
	myls_code=$?
	if [ "$ls_code" -eq "$myls_code" ] && cmp -s "$ls_out" "$myls_out"; then
		pass "$desc"
	else
		fail "$desc" "outputs differ"
		diff -u "$ls_out" "$myls_out" || true
	fi
}

check_contains() {
	desc="$1"
	cmd="$2"
	expected="$3"
	out="$TMP/check.out"

	( cd "$TMP/dossier" && LC_ALL=C eval "$cmd" > "$out" 2>&1 )
	code=$?
	if [ "$code" -eq 0 ] && grep -F "$expected" "$out" >/dev/null 2>&1; then
		pass "$desc"
	else
		fail "$desc" "expected '$expected'"
		cat "$out"
	fi
}

check_nonzero() {
	desc="$1"
	cmd="$2"
	out="$TMP/error.out"

	( cd "$TMP" && LC_ALL=C eval "$cmd" > "$out" 2>&1 )
	code=$?
	if [ "$code" -ne 0 ]; then
		pass "$desc"
	else
		fail "$desc" "command unexpectedly succeeded"
		cat "$out"
	fi
}

make_fixture

compare_exact "./myls -1" "ls -1 --color=never" "$BIN -1 --color=never"
compare_exact "./myls -a" "ls -a -1 --color=never" "$BIN -a -1 --color=never"
compare_exact "./myls -A" "ls -A -1 --color=never" "$BIN -A -1 --color=never"
compare_exact "./myls -d dossier" "ls -d -1 --color=never ." "$BIN -d -1 --color=never ."
compare_exact "./myls -t" "ls -t -1 --color=never" "$BIN -t -1 --color=never"
compare_exact "./myls -tr" "ls -tr -1 --color=never" "$BIN -tr -1 --color=never"
compare_exact "./myls -S" "ls -S -1 --color=never" "$BIN -S -1 --color=never"
compare_exact "./myls -X" "ls -X -1 --color=never" "$BIN -X -1 --color=never"
compare_exact "./myls -r" "ls -r -1 --color=never" "$BIN -r -1 --color=never"
compare_exact "./myls --sort=time" "ls --sort=time -1 --color=never" "$BIN --sort=time -1 --color=never"
compare_exact "./myls --sort=size" "ls --sort=size -1 --color=never" "$BIN --sort=size -1 --color=never"
compare_exact "./myls --sort=extension" "ls --sort=extension -1 --color=never" "$BIN --sort=extension -1 --color=never"
compare_exact "./myls --hide" "ls --hide='*.o' -1 --color=never" "$BIN --hide='*.o' -1 --color=never"
compare_exact "./myls --ignore" "ls --ignore='*.o' -1 --color=never" "$BIN --ignore='*.o' -1 --color=never"
compare_exact "./myls -B" "ls -B -1 --color=never" "$BIN -B -1 --color=never"

check_contains "./myls" "$BIN --color=never" "file1"
check_contains "./myls -l" "$BIN -l --color=never" "total "
check_contains "./myls -l link" "$BIN -l --color=never link" "link -> file1"
check_contains "./myls -la" "$BIN -la --color=never" ".hidden"
check_contains "./myls -lh" "$BIN -lh --color=never" "file1"
check_contains "./myls -n" "$BIN -n --color=never" "file1"
check_contains "./myls -i" "$BIN -i --color=never" "file1"
check_contains "./myls -s" "$BIN -s --color=never" "file1"
check_contains "./myls -R dossier" "$BIN -R -1 --color=never ." "./sub:"
check_contains "./myls -m" "$BIN -m --color=never" ", "
check_contains "./myls -F" "$BIN -F -1 --color=never" "executable*"
check_contains "./myls -p" "$BIN -p -1 --color=never" "sub/"
check_contains "./myls --color=never" "$BIN --color=never" "file1"
check_nonzero "./myls dossier_inexistant" "$BIN dossier_inexistant"

printf "\nResults: %d PASS / %d FAIL\n" "$PASS" "$FAIL"
[ "$FAIL" -eq 0 ]
