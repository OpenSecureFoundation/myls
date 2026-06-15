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
	printf "ten\n" > "$TMP/dossier/file10"
	printf "hidden\n" > "$TMP/dossier/.hidden"
	: > "$TMP/dossier/empty"
	: > "$TMP/dossier/-dash"
	: > "$TMP/dossier/backup~"
	: > "$TMP/dossier/space file"
	: > "$TMP/dossier/quote\"file"
	: > "$TMP/dossier/back\\slash"
	: > "$TMP/dossier/tab	file"
	printf "line\n" > "$TMP/dossier/newline
file"
	printf "#!/bin/sh\nexit 0\n" > "$TMP/dossier/executable"
	chmod +x "$TMP/dossier/executable"
	ln -s file1 "$TMP/dossier/link"
	ln -s sub "$TMP/dossier/linkdir"
	ln -s missing "$TMP/dossier/broken"
	mkfifo "$TMP/dossier/fifo"
	touch -t 202001010101 "$TMP/dossier/file1"
	touch -t 202001010102 "$TMP/dossier/file2.o"
	touch -t 202001010103 "$TMP/dossier/file10"
	touch -t 202001010104 "$TMP/dossier/executable"
	touch -t 202001010105 "$TMP/dossier/sub"
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
	if [ "$code" -eq 0 ] && grep -F -- "$expected" "$out" >/dev/null 2>&1; then
		pass "$desc"
	else
		fail "$desc" "expected '$expected'"
		cat "$out"
	fi
}

check_not_contains() {
	desc="$1"
	cmd="$2"
	unexpected="$3"
	out="$TMP/check_not.out"

	( cd "$TMP/dossier" && LC_ALL=C eval "$cmd" > "$out" 2>&1 )
	code=$?
	if [ "$code" -eq 0 ] && ! grep -F -- "$unexpected" "$out" >/dev/null 2>&1; then
		pass "$desc"
	else
		fail "$desc" "unexpected '$unexpected'"
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

check_exit_code() {
	desc="$1"
	cmd="$2"
	expected="$3"
	out="$TMP/code.out"

	( cd "$TMP/dossier" && LC_ALL=C eval "$cmd" > "$out" 2>&1 )
	code=$?
	if [ "$code" -eq "$expected" ]; then
		pass "$desc"
	else
		fail "$desc" "expected code $expected, got $code"
		cat "$out"
	fi
}

check_color() {
	desc="$1"
	cmd="$2"
	out="$TMP/color.out"
	esc=$(printf '\033')

	( cd "$TMP/dossier" && LC_ALL=C eval "$cmd" > "$out" 2>&1 )
	code=$?
	if [ "$code" -eq 0 ] && grep "$esc" "$out" >/dev/null 2>&1; then
		pass "$desc"
	else
		fail "$desc" "expected ANSI color escape"
		cat "$out"
	fi
}

check_sgr() {
	desc="$1"
	cmd="$2"
	expected="$3"
	out="$TMP/sgr.out"
	esc=$(printf '\033')

	( cd "$TMP/dossier" && LC_ALL=C eval "$cmd" > "$out" 2>&1 )
	code=$?
	if [ "$code" -eq 0 ] && grep -F -- "${esc}[${expected}m" "$out" >/dev/null 2>&1; then
		pass "$desc"
	else
		fail "$desc" "expected SGR $expected"
		od -An -t x1 "$out"
	fi
}

check_zero_output() {
	desc="$1"
	cmd="$2"
	out="$TMP/zero.out"

	( cd "$TMP/dossier" && LC_ALL=C eval "$cmd" > "$out" 2>&1 )
	code=$?
	if [ "$code" -eq 0 ] && od -An -t x1 "$out" | grep '00' >/dev/null 2>&1; then
		pass "$desc"
	else
		fail "$desc" "expected NUL-separated output"
		od -An -t x1 "$out"
	fi
}

check_no_color() {
	desc="$1"
	cmd="$2"
	out="$TMP/no_color.out"
	esc=$(printf '\033')

	( cd "$TMP/dossier" && LC_ALL=C eval "$cmd" > "$out" 2>&1 )
	code=$?
	if [ "$code" -eq 0 ] && ! grep "$esc" "$out" >/dev/null 2>&1; then
		pass "$desc"
	else
		fail "$desc" "unexpected ANSI color escape"
		cat "$out"
	fi
}

check_zero_literal() {
	desc="$1"
	cmd="$2"
	out="$TMP/zero_literal.out"

	( cd "$TMP/dossier" && LC_ALL=C eval "$cmd" > "$out" 2>&1 )
	code=$?
	if [ "$code" -eq 0 ] \
		&& od -An -t x1 "$out" | grep '00' >/dev/null 2>&1 \
		&& od -An -t x1 "$out" | grep '09' >/dev/null 2>&1; then
		pass "$desc"
	else
		fail "$desc" "expected literal NUL output without quoting"
		od -An -t x1 "$out"
	fi
}

check_capability_marker() {
	desc="$1"
	out="$TMP/cap.out"
	file="$TMP/dossier/capfile"

	cp "$BIN" "$file" 2>/dev/null || {
		pass "$desc (skipped)"
		return
	}
	if ! command -v setcap >/dev/null 2>&1 \
		|| ! setcap cap_net_bind_service=ep "$file" 2>/dev/null; then
		pass "$desc (skipped)"
		return
	fi
	( cd "$TMP/dossier" && LC_ALL=C "$BIN" -l --color=never capfile > "$out" 2>&1 )
	if grep -E '^-[-rwxstST]{9}[.] ' "$out" >/dev/null 2>&1; then
		pass "$desc"
	else
		fail "$desc" "expected capability marker '.'"
		cat "$out"
	fi
	setcap -r "$file" 2>/dev/null || true
}


make_fixture

compare_exact "./myls -1" "ls -1 --color=never" "$BIN -1 --color=never"
compare_exact "./myls -a" "ls -a -1 --color=never" "$BIN -a -1 --color=never"
compare_exact "./myls -A" "ls -A -1 --color=never" "$BIN -A -1 --color=never"
compare_exact "./myls -d dossier" "ls -d -1 --color=never ." "$BIN -d -1 --color=never ."
compare_exact "./myls -t" "ls -t -1 --color=never" "$BIN -t -1 --color=never"
compare_exact "./myls -tr" "ls -tr -1 --color=never" "$BIN -tr -1 --color=never"
compare_exact "./myls -u" "ls -u -1 --color=never" "$BIN -u -1 --color=never"
compare_exact "./myls -c" "ls -c -1 --color=never" "$BIN -c -1 --color=never"
compare_exact "./myls -lu" "ls -lu --time-style=long-iso --color=never file1 file2.o" "$BIN -lu --time-style=long-iso --color=never file1 file2.o"
compare_exact "./myls -lc" "ls -lc --time-style=long-iso --color=never file1 file2.o" "$BIN -lc --time-style=long-iso --color=never file1 file2.o"
compare_exact "./myls -l no acl marker" "ls -l --time-style=long-iso --color=never file1 file2.o" "$BIN -l --time-style=long-iso --color=never file1 file2.o"
compare_exact "./myls -S" "ls -S -1 --color=never" "$BIN -S -1 --color=never"
compare_exact "./myls -X" "ls -X -1 --color=never" "$BIN -X -1 --color=never"
compare_exact "./myls -r" "ls -r -1 --color=never" "$BIN -r -1 --color=never"
compare_exact "./myls --sort=time" "ls --sort=time -1 --color=never" "$BIN --sort=time -1 --color=never"
compare_exact "./myls --time=atime" "ls --time=atime -1 --color=never" "$BIN --time=atime -1 --color=never"
compare_exact "./myls --time=ctime" "ls --time=ctime -1 --color=never" "$BIN --time=ctime -1 --color=never"
compare_exact "./myls --sort=size" "ls --sort=size -1 --color=never" "$BIN --sort=size -1 --color=never"
compare_exact "./myls --sort=extension" "ls --sort=extension -1 --color=never" "$BIN --sort=extension -1 --color=never"
compare_exact "./myls --hide" "ls --hide='*.o' -1 --color=never" "$BIN --hide='*.o' -1 --color=never"
compare_exact "./myls --ignore" "ls --ignore='*.o' -1 --color=never" "$BIN --ignore='*.o' -1 --color=never"
compare_exact "./myls -a --ignore period" "ls -a --ignore='*' -1 --color=never" "$BIN -a --ignore='*' -1 --color=never"
compare_exact "./myls -A --ignore period" "ls -A --ignore='*' -1 --color=never" "$BIN -A --ignore='*' -1 --color=never"
compare_exact "./myls -a --hide overridden" "ls -a --hide='*' -1 --color=never" "$BIN -a --hide='*' -1 --color=never"
compare_exact "./myls -I" "ls -I '*.o' -1 --color=never" "$BIN -I '*.o' -1 --color=never"
compare_exact "./myls -B" "ls -B -1 --color=never" "$BIN -B -1 --color=never"
compare_exact "./myls --format=single-column" "ls --format=single-column --color=never" "$BIN --format=single-column --color=never"
compare_exact "./myls --group-directories-first" "ls --group-directories-first -1 --color=never" "$BIN --group-directories-first -1 --color=never"
compare_exact "./myls -b" "ls -b -1 --color=never" "$BIN -b -1 --color=never"
compare_exact "./myls -q" "ls -q -1 --color=never" "$BIN -q -1 --color=never"
compare_exact "./myls --show-control-chars" "ls -q --show-control-chars -1 --color=never" "$BIN -q --show-control-chars -1 --color=never"
compare_exact "./myls -N" "ls -N -1 --color=never" "$BIN -N -1 --color=never"
compare_exact "./myls -Q" "ls -Q -1 --color=never" "$BIN -Q -1 --color=never"
compare_exact "./myls quoting literal" "ls --quoting-style=literal -1 --color=never" "$BIN --quoting-style=literal -1 --color=never"
compare_exact "./myls quoting escape" "ls --quoting-style=escape -1 --color=never" "$BIN --quoting-style=escape -1 --color=never"
compare_exact "./myls quoting c" "ls --quoting-style=c -1 --color=never" "$BIN --quoting-style=c -1 --color=never"
compare_exact "./myls quoting locale" "ls --quoting-style=locale -1 --color=never" "$BIN --quoting-style=locale -1 --color=never"
compare_exact "./myls quoting shell" "ls --quoting-style=shell -1 --color=never" "$BIN --quoting-style=shell -1 --color=never"
compare_exact "./myls quoting shell-always" "ls --quoting-style=shell-always -1 --color=never" "$BIN --quoting-style=shell-always -1 --color=never"
compare_exact "./myls quoting shell-escape" "ls --quoting-style=shell-escape -1 --color=never" "$BIN --quoting-style=shell-escape -1 --color=never"
compare_exact "./myls quoting shell-escape-always" "ls --quoting-style=shell-escape-always -1 --color=never" "$BIN --quoting-style=shell-escape-always -1 --color=never"

check_contains "./myls" "$BIN --color=never" "file1"
check_contains "./myls -l" "$BIN -l --color=never" "total "
check_capability_marker "./myls capability marker"
check_contains "./myls -l link" "$BIN -l --color=never link" "link -> file1"
check_contains "./myls -l broken" "$BIN -l --color=never broken" "broken -> missing"
check_not_contains "./myls -lL link" "$BIN -lL --color=never link" "link -> file1"
check_contains "./myls -la" "$BIN -la --color=never" ".hidden"
check_contains "./myls -lh" "$BIN -lh --color=never" "file1"
check_contains "./myls -n" "$BIN -n --color=never" "file1"
check_contains "./myls -i" "$BIN -i --color=never" "file1"
check_contains "./myls -s" "$BIN -s --color=never" "file1"
check_contains "./myls -R dossier" "$BIN -R -1 --color=never ." "./sub:"
check_contains "./myls -m" "$BIN -m --color=never" ", "
check_contains "./myls -C" "$BIN -C --color=never" "file1"
check_contains "./myls -x" "$BIN -x --color=never" "file1"
check_contains "./myls -F" "$BIN -F -1 --color=never" "executable*"
check_contains "./myls -F fifo" "$BIN -F -1 --color=never fifo" "fifo|"
check_contains "./myls -p" "$BIN -p -1 --color=never" "sub/"
check_contains "./myls --file-type" "$BIN --file-type -1 --color=never" "link@"
check_not_contains "./myls --file-type executable" "$BIN --file-type -1 --color=never executable" "executable*"
check_contains "./myls --indicator-style=classify" "$BIN --indicator-style=classify -1 --color=never" "executable*"
check_contains "./myls --indicator-style=file-type" "$BIN --indicator-style=file-type -1 --color=never" "link@"
check_contains "./myls --indicator-style=slash" "$BIN --indicator-style=slash -1 --color=never" "sub/"
check_not_contains "./myls --indicator-style=none" "$BIN --indicator-style=none -1 --color=never executable" "*"
check_not_contains "./myls --classify=never" "$BIN --classify=never -1 --color=never executable" "*"
check_contains "./myls --color=never" "$BIN --color=never" "file1"
check_color "./myls --color=always" "$BIN --color=always -1"
check_sgr "./myls LS_COLORS directory" "LS_COLORS='di=31:ln=32:or=41;37:pi=36:ex=33:*.o=35' $BIN --color=always -F -1" "31"
check_sgr "./myls LS_COLORS symlink" "LS_COLORS='di=31:ln=32:or=41;37:pi=36:ex=33:*.o=35' $BIN --color=always -F -1" "32"
check_sgr "./myls LS_COLORS extension" "LS_COLORS='di=31:ln=32:or=41;37:pi=36:ex=33:*.o=35' $BIN --color=always -F -1" "35"
check_sgr "./myls LS_COLORS fifo" "LS_COLORS='di=31:ln=32:or=41;37:pi=36:ex=33:*.o=35' $BIN --color=always -F -1" "36"
check_sgr "./myls LS_COLORS broken" "LS_COLORS='di=31:ln=32:or=41;37:pi=36:ex=33:*.o=35' $BIN --color=always -F -1" "41;37"
check_zero_output "./myls --zero" "$BIN --zero"
check_no_color "./myls --zero disables color" "$BIN --zero --color=always"
check_zero_literal "./myls --zero literal" "$BIN --zero -Q -q"
check_contains "./myls --full-time" "$BIN --full-time --color=never file1" "2020-01-01 01:01:"
check_contains "./myls --time-style=long-iso" "$BIN -l --time-style=long-iso --color=never file1" "2020-01-01 01:01"
check_contains "./myls --time-style=full-iso" "$BIN -l --time-style=full-iso --color=never file1" "2020-01-01 01:01:"
check_contains "./myls --time-style=+%Y" "$BIN -l --time-style=+%Y --color=never file1" "2020"
check_contains "./myls --sort=version" "$BIN --sort=version -1 --color=never" "file10"
check_contains "./myls --format=commas" "$BIN --format=commas --color=never" ", "
check_contains "./myls --format=long" "$BIN --format=long --color=never file1" "file1"
check_contains "./myls mixed options" "$BIN file1 -l --color=never" "file1"
check_contains "./myls -- ends options" "$BIN -- -dash" "-dash"
check_nonzero "./myls dossier_inexistant" "$BIN dossier_inexistant"
check_exit_code "./myls --help" "$BIN --help" 0
check_exit_code "./myls --version" "$BIN --version" 0
check_exit_code "./myls -?" "$BIN -?" 2
check_exit_code "./myls --sort=bad" "$BIN --sort=bad" 2
check_exit_code "./myls -w missing" "$BIN -w" 2
check_exit_code "./myls -Z unsupported" "$BIN -Z" 2
check_exit_code "./myls --hyperlink unsupported" "$BIN --hyperlink=always" 2
check_exit_code "./myls --time=birth unsupported" "$BIN --time=birth" 2

printf "\nResults: %d PASS / %d FAIL\n" "$PASS" "$FAIL"
[ "$FAIL" -eq 0 ]
