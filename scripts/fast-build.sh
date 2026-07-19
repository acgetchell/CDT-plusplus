#!/usr/bin/env bash

# Before running this script, make sure $VCPKG_ROOT names the pinned checkout.

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "${script_dir}/.." && pwd)"

cd -- "${repo_root}"
cmake --preset reference
cmake --build --preset reference --target cdt
