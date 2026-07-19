#!/usr/bin/env bash

# Keep pkgx-provided build tools on PATH without allowing their transitive
# headers and libraries to affect compiler detection or dependency resolution.
prepare_reference_environment()
{
  unset CMAKE_PREFIX_PATH
  unset CPATH
  unset C_INCLUDE_PATH
  unset CPLUS_INCLUDE_PATH
  unset CFLAGS
  unset CPPFLAGS
  unset CXXFLAGS
  unset LDFLAGS
  unset LIBRARY_PATH
  unset PKG_CONFIG_PATH

  if [[ "$(uname -s)" == "Darwin" ]] && command -v xcode-select >/dev/null && command -v xcrun >/dev/null; then
    unset DEVELOPER_DIR
    DEVELOPER_DIR="$(xcode-select -p)"
    export DEVELOPER_DIR
    SDKROOT="$(xcrun --sdk macosx --show-sdk-path)"
    CC=/usr/bin/cc
    CXX=/usr/bin/c++
    unset ARCHFLAGS
    unset MACOSX_DEPLOYMENT_TARGET
    export CC CXX SDKROOT
  fi
}

# Source this file and call prepare_vcpkg with the repository root.
prepare_vcpkg()
{
  local repository_root="${1:?repository root is required}"
  local pinned_vcpkg_root="${CDT_VCPKG_CACHE_DIR:-${repository_root}/.cache/vcpkg}"
  local bootstrap_script="${repository_root}/scripts/bootstrap-vcpkg.sh"

  if [[ -n "${VCPKG_ROOT:-}" ]] &&
     CDT_VCPKG_CACHE_DIR="${VCPKG_ROOT}" "${bootstrap_script}" --check 2>/dev/null; then
    VCPKG_ROOT="$(cd -- "${VCPKG_ROOT}" && pwd -P)"
    export VCPKG_ROOT
    return
  fi

  if [[ -n "${VCPKG_ROOT:-}" ]]; then
    printf 'Ignoring VCPKG_ROOT=%s; using the repository-pinned checkout instead.\n' \
      "${VCPKG_ROOT}" >&2
  fi

  export VCPKG_ROOT="${pinned_vcpkg_root}"
  "${bootstrap_script}"
  VCPKG_ROOT="$(cd -- "${VCPKG_ROOT}" && pwd -P)"
  export VCPKG_ROOT
}

# Keep repeat builds incremental, but discard configuration state when the
# selected vcpkg toolchain actually changes. CMake cannot safely replace a
# cached toolchain after project() has enabled a language.
prepare_cmake_cache()
{
  local build_directory="${1:?build directory is required}"
  local cache_file="${build_directory}/CMakeCache.txt"
  local expected_toolchain="${VCPKG_ROOT:?VCPKG_ROOT must be set}/scripts/buildsystems/vcpkg.cmake"
  local expected_installed_dir="${build_directory}/vcpkg_installed"
  local cached_toolchain
  local cached_installed_dir

  [[ -f "${cache_file}" ]] || return 0

  cached_toolchain="$(sed -n 's/^CMAKE_TOOLCHAIN_FILE:[^=]*=//p' "${cache_file}" | head -n 1)"
  cached_installed_dir="$(sed -n 's/^VCPKG_INSTALLED_DIR:[^=]*=//p' "${cache_file}" | head -n 1)"
  if [[ -n "${cached_toolchain}" && "${cached_toolchain}" != "${expected_toolchain}" ]] ||
     [[ -n "${cached_installed_dir}" && "${cached_installed_dir}" != "${expected_installed_dir}" ]]; then
    printf 'vcpkg configuration changed; refreshing CMake configuration state.\n' >&2
    cmake -E rm -f "${cache_file}"
    cmake -E rm -rf "${build_directory}/CMakeFiles"
  fi
}
