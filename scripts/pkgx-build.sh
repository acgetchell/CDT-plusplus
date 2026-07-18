#!/usr/bin/env -S pkgx +git +cmake +ninja +python bash>=4
# shellcheck shell=bash

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
exec "${script_dir}/build.sh"
