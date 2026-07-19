#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "${script_dir}/.." && pwd)"
manifest="${repo_root}/vcpkg.json"
vcpkg_dir="${CDT_VCPKG_CACHE_DIR:-${repo_root}/.cache/vcpkg}"
mode="${1:-bootstrap}"

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
  if [[ -n "$(git -C "${vcpkg_dir}" status --short)" ]]; then
    printf 'Refusing to reuse a modified vcpkg checkout at %s.\n' "${vcpkg_dir}" >&2
    exit 1
  fi
  if [[ -x "${vcpkg_dir}/vcpkg" ]] &&
     [[ "$(git -C "${vcpkg_dir}" rev-parse HEAD 2>/dev/null || true)" == "${baseline}" ]]; then
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
"${vcpkg_dir}/bootstrap-vcpkg.sh" -disableMetrics

printf 'Bootstrapped vcpkg at %s (%s)\n' "${vcpkg_dir}" "${baseline}"
