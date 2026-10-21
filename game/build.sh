#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
: "${DEVKITPRO:?Set DEVKITPRO to a prefix with devkitARM and patched libnx32}"
export PATH="$DEVKITPRO/devkitARM/bin:$DEVKITPRO/tools/bin:$PATH"
python3 runtime/tools/gen_imports.py --port game --check
exec make -f Game.mk -j"${JOBS:-4}" "$@"
