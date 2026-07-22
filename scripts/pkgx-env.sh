#!/usr/bin/env bash
# shellcheck shell=bash

# Source this file from a shell or IDE environment-file setting.
if [[ -n "${BASH_VERSION:-}" ]]; then
  script_file="${BASH_SOURCE[0]}"
elif [[ -n "${ZSH_VERSION:-}" ]]; then
  script_file="${(%):-%x}"
else
  printf 'pkgx-env.sh requires Bash or Zsh.\n' >&2
  return 1 2>/dev/null || exit 1
fi

script_dir="$(cd -- "$(dirname -- "${script_file}")" && pwd -P)"
repo_root="$(cd -- "${script_dir}/.." && pwd -P)"

if ! pkgx_environment="$(pkgx \
  +freedesktop.org/pkg-config@0.29.2 \
  +gnu.org/m4@1.4.21 \
  +gnu.org/autoconf@2.73.0 \
  +gnu.org/autoconf-archive@2024.10.16 \
  +gnu.org/automake@1.18.1 \
  +gnu.org/libtool@2.5.4 \
  +gnu.org/texinfo@7.3.0)"; then
  printf 'Unable to resolve the pkgx build environment.\n' >&2
  unset pkgx_environment repo_root script_dir script_file
  return 1 2>/dev/null || exit 1
fi

case "$-" in
  *a*) had_allexport=1 ;;
  *) had_allexport=0; set -a ;;
esac
eval "${pkgx_environment}"
if [[ "${had_allexport}" == 0 ]]; then
  set +a
fi

# Keep pkgx-provided build tools on PATH without exposing their transitive
# headers and libraries to compiler detection or vcpkg dependency builds.
unset CMAKE_PREFIX_PATH
unset CPATH
unset C_INCLUDE_PATH
unset CPLUS_INCLUDE_PATH
unset CC
unset CFLAGS
unset CPPFLAGS
unset CXX
unset CXXFLAGS
unset LDFLAGS
unset LIBRARY_PATH
unset PKG_CONFIG_PATH

if [[ "$(uname -s)" == "Darwin" ]] && command -v xcode-select >/dev/null && command -v xcrun >/dev/null; then
  unset DEVELOPER_DIR
  DEVELOPER_DIR="$(xcode-select -p)"
  SDKROOT="$(xcrun --sdk macosx --show-sdk-path)"
  CC=/usr/bin/cc
  CXX=/usr/bin/c++
  unset ARCHFLAGS
  unset MACOSX_DEPLOYMENT_TARGET
  export CC CXX DEVELOPER_DIR SDKROOT
fi

export VCPKG_ROOT="${repo_root}/.cache/vcpkg"

unset had_allexport pkgx_environment repo_root script_dir script_file
