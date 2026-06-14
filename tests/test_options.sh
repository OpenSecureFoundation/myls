#!/bin/sh

set -u

ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)

exec sh "$ROOT/tests/test_myls.sh"
