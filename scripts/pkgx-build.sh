#!/usr/bin/env -S pkgx bash>=4
# shellcheck shell=bash

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "${script_dir}/.." && pwd)"

pkgx_tools=(
  +git-scm.org
  +cmake.org
  +ninja-build.org
  +python.org
  "+just.systems>=1.56.0"
  +ccache.dev
  +gnu.org/m4
  +gnu.org/autoconf
  +gnu.org/autoconf-archive
  +gnu.org/automake
  +gnu.org/libtool@2.5.4
  +gnu.org/texinfo
  +freedesktop.org/pkg-config
)

cd -- "${repo_root}"
export CDT_PKGX_ACTIVE=1
exec pkgx "${pkgx_tools[@]}" just build
