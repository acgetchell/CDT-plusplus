#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "${script_dir}/.." && pwd)"
manifest="${repo_root}/vcpkg.json"
vcpkg_dir="${CDT_VCPKG_CACHE_DIR:-${repo_root}/.cache/vcpkg}"
mode="${1:-bootstrap}"
trusted_vcpkg_tool_tag="2026-07-13"

metadata_value()
{
  local key="${1}"
  sed -nE "s/^${key}=(.*)$/\\1/p" "${vcpkg_dir}/scripts/vcpkg-tool-metadata.txt"
}

compute_sha512()
{
  local file="${1}"
  if command -v sha512sum >/dev/null; then
    sha512sum "${file}" | awk '{print $1}'
  elif command -v shasum >/dev/null; then
    shasum -a 512 "${file}" | awk '{print $1}'
  else
    printf 'A SHA-512 utility is required to validate the cached vcpkg binary.\n' >&2
    return 1
  fi
}

validate_cached_vcpkg()
{
  local metadata="${vcpkg_dir}/scripts/vcpkg-tool-metadata.txt"
  local expected_sha_key=""
  local expected_tag=""
  local expected_sha=""
  local actual_sha=""

  [[ -f "${metadata}" && -x "${vcpkg_dir}/vcpkg" ]] || return 1

  expected_tag="$(metadata_value VCPKG_TOOL_RELEASE_TAG)"
  [[ "${expected_tag}" == "${trusted_vcpkg_tool_tag}" ]] || return 1

  case "$(uname -s):$(uname -m)" in
    Darwin:*)
      expected_sha_key="VCPKG_MACOS_SHA"
      ;;
    Linux:aarch64 | Linux:arm64)
      expected_sha_key="VCPKG_GLIBC_ARM64_SHA"
      ;;
    Linux:x86_64 | Linux:amd64)
      if ldd --version 2>&1 | grep -qi musl; then
        expected_sha_key="VCPKG_MUSLC_SHA"
      else
        expected_sha_key="VCPKG_GLIBC_SHA"
      fi
      ;;
    *)
      return 1
      ;;
  esac

  expected_sha="$(metadata_value "${expected_sha_key}")"
  [[ "${expected_sha}" =~ ^[0-9a-f]{128}$ ]] || return 1
  actual_sha="$(compute_sha512 "${vcpkg_dir}/vcpkg")" || return 1
  [[ "${actual_sha}" == "${expected_sha}" ]] || return 1

  "${vcpkg_dir}/vcpkg" version 2>/dev/null |
    grep -q "^vcpkg package management program version ${trusted_vcpkg_tool_tag}-"
}

if [[ "${mode}" != "bootstrap" && "${mode}" != "--check" ]]; then
  printf 'Usage: %s [--check]\n' "$0" >&2
  exit 2
fi

baseline="$(
  sed -nE 's/^[[:space:]]*"builtin-baseline":[[:space:]]*"([0-9a-f]{40})",?$/\1/p' \
    "${manifest}"
)"

if [[ ! "${baseline}" =~ ^[0-9a-f]{40}$ ]]; then
  printf 'Unable to read a 40-character builtin-baseline from %s\n' "${manifest}" >&2
  exit 1
fi

if [[ -d "${vcpkg_dir}/.git" ]]; then
  origin="$(git -C "${vcpkg_dir}" remote get-url origin 2>/dev/null || true)"
  if [[ "${origin}" != "https://github.com/microsoft/vcpkg.git" ]]; then
    printf 'Refusing to reuse %s because its origin is not microsoft/vcpkg.\n' "${vcpkg_dir}" >&2
    exit 1
  fi
  if [[ -n "$(git -C "${vcpkg_dir}" status --short --untracked-files=no)" ]]; then
    printf 'Refusing to reuse a modified vcpkg checkout at %s.\n' "${vcpkg_dir}" >&2
    exit 1
  fi
  if [[ -x "${vcpkg_dir}/vcpkg" ]] &&
     [[ "$(git -C "${vcpkg_dir}" rev-parse HEAD 2>/dev/null || true)" == "${baseline}" ]] &&
     validate_cached_vcpkg; then
    printf 'Using vcpkg at %s (%s)\n' "${vcpkg_dir}" "${baseline}"
    exit 0
  fi
elif [[ -d "${vcpkg_dir}" ]] &&
     [[ -n "$(find "${vcpkg_dir}" -mindepth 1 -maxdepth 1 -print -quit)" ]]; then
  printf 'Refusing to initialize vcpkg in non-empty directory %s.\n' "${vcpkg_dir}" >&2
  exit 1
fi

if [[ "${mode}" == "--check" ]]; then
  printf 'VCPKG_ROOT is not a clean official checkout at baseline %s: %s\n' \
    "${baseline}" "${vcpkg_dir}" >&2
  exit 1
fi

if [[ ! -d "${vcpkg_dir}/.git" ]]; then
  mkdir -p "${vcpkg_dir}"
  git -C "${vcpkg_dir}" init
  git -C "${vcpkg_dir}" remote add origin https://github.com/microsoft/vcpkg.git
fi

git -C "${vcpkg_dir}" fetch --depth 1 origin "${baseline}"
git -C "${vcpkg_dir}" checkout --detach "${baseline}"
rm -f -- "${vcpkg_dir}/vcpkg"
"${vcpkg_dir}/bootstrap-vcpkg.sh" -disableMetrics

if ! validate_cached_vcpkg; then
  printf 'Bootstrapped vcpkg does not match the tool release pinned by %s.\n' \
    "${baseline}" >&2
  exit 1
fi

printf 'Bootstrapped vcpkg at %s (%s)\n' "${vcpkg_dir}" "${baseline}"
