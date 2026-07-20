#!/usr/bin/env bash

set -euo pipefail

usage() {
    echo "Usage: $0 <check|build> <doxygen-version> <graphviz-version>" >&2
    exit 2
}

[[ $# -eq 3 ]] || usage

mode="$1"
doxygen_version="$2"
graphviz_version="$3"
[[ "$mode" == "check" || "$mode" == "build" ]] || usage

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

doxygen_matches() {
    command -v doxygen >/dev/null 2>&1 &&
        [[ "$(doxygen --version)" == "$doxygen_version" ]]
}

graphviz_matches() {
    local installed_version
    command -v dot >/dev/null 2>&1 || return 1
    installed_version="$(dot -V 2>&1 | sed -nE 's/^dot - graphviz version ([^ ]+).*/\1/p')"
    [[ "$installed_version" == "$graphviz_version" ]]
}

if ! doxygen_matches || ! graphviz_matches; then
    if command -v pkgx >/dev/null 2>&1 && [[ -z "${CDT_DOXYGEN_TOOLCHAIN_ACTIVE:-}" ]]; then
        export CDT_DOXYGEN_TOOLCHAIN_ACTIVE=1
        exec pkgx "+doxygen.nl@$doxygen_version" "+graphviz.org@$graphviz_version" -- \
            "$0" "$mode" "$doxygen_version" "$graphviz_version"
    fi

    echo "Doxygen $doxygen_version and Graphviz $graphviz_version are required." >&2
    echo "Install those versions or install pkgx so the repository can provide them ephemerally." >&2
    exit 1
fi

temporary_output="$(mktemp -d "${TMPDIR:-/tmp}/cdt-doxygen.XXXXXX")"
cleanup() {
    rm -rf "$temporary_output"
}
trap cleanup EXIT

config_output="$temporary_output"
if command -v cygpath >/dev/null 2>&1; then
    config_output="$(cygpath -m "$temporary_output")"
fi

{
    cat docs/Doxyfile
    printf '\nOUTPUT_DIRECTORY = "%s"\n' "$config_output"
} | doxygen -

generated_html="$temporary_output/html"
if [[ ! -f "$generated_html/index.html" ]]; then
    echo "Doxygen completed without generating html/index.html." >&2
    exit 1
fi

if [[ "$mode" == "build" ]]; then
    published_html="$repo_root/docs/html"
    rm -rf "$published_html"
    mv "$generated_html" "$published_html"
    touch "$published_html/.nojekyll"
    echo "Documentation generated in docs/html/."
else
    echo "Documentation validation complete."
fi
