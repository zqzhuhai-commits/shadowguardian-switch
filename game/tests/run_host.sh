#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
mkdir -p build/host
cc=${CC:-cc}
common=(-std=c11 -g -Wall -Wextra -Werror -Igame/tests/host -Igame/source -Iruntime/source)
sanitize=(-fsanitize=address,undefined -fno-omit-frame-pointer)
export ASAN_OPTIONS=detect_leaks=0
"$cc" "${common[@]}" "${sanitize[@]}" game/source/sg_api.c game/tests/api_test.c -o build/host/api_test
build/host/api_test
"$cc" "${common[@]}" "${sanitize[@]}" game/source/sg_atc.c game/tests/atc_test.c -o build/host/atc_test
build/host/atc_test
"$cc" "${common[@]}" "${sanitize[@]}" game/source/sg_atc.c game/source/sg_gl.c game/tests/gl_test.c -o build/host/gl_test
build/host/gl_test
"$cc" "${common[@]}" "${sanitize[@]}" -pthread game/source/sg_audio.c game/tests/audio_test.c -o build/host/audio_test
build/host/audio_test
"$cc" "${common[@]}" "${sanitize[@]}" game/source/sg_swp.c game/tests/swp_test.c -o build/host/swp_test
build/host/swp_test
"$cc" "${common[@]}" "${sanitize[@]}" game/source/sg_input.c game/tests/input_test.c -o build/host/input_test
build/host/input_test
python3 runtime/tools/gen_imports.py --port game --check
