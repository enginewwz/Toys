#!/usr/bin/env bash
set -euo pipefail

# Usage:
#   ./icx_build.sh <output_name> <file1.c> [file2.c ...]
#
# Output:
#   <dir-of-first-source>/build/<output_name>

if [[ $# -lt 2 ]]; then
  echo "Usage: $0 <output_name> <file1.c> [file2.c ...]" >&2
  exit 1
fi

OUT_NAME="$1"
shift

FIRST_SRC="$1"
FIRST_DIR="$(dirname -- "$FIRST_SRC")"
BUILD_DIR="${FIRST_DIR}/build"
mkdir -p "$BUILD_DIR"

ELF="${BUILD_DIR}/${OUT_NAME}"

# Add Python embed flags if any source includes Python.h.
PY_EMBED_FLAGS=""
for SRC in "$@"; do
  if grep -q "Python.h" "$SRC" 2>/dev/null; then
    if command -v python3.12-config >/dev/null 2>&1; then
      PY_EMBED_FLAGS="$(python3.12-config --embed --cflags --ldflags)"
    elif command -v python3-config >/dev/null 2>&1; then
      PY_EMBED_FLAGS="$(python3-config --embed --cflags --ldflags)"
    else
      echo "Python config tool not found; linking may fail." >&2
    fi
    break
  fi
done

# Compile & link with required flags
icx -O3 -xHost -fp-model=fast -qopt-zmm-usage=high -qopenmp -o "$ELF" "$@" $PY_EMBED_FLAGS

echo "Built: $ELF"