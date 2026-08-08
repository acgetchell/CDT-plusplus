#!/usr/bin/env bash

set -euo pipefail

usage() {
    echo "Usage: $0 <check|build> <doxygen-version> <graphviz-version> <python-version>" >&2
    exit 2
}

[[ $# -eq 4 ]] || usage

mode="$1"
doxygen_version="$2"
graphviz_version="$3"
python_version="$4"
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

python_matches() {
    command -v python3 >/dev/null 2>&1 &&
        [[ "$(python3 -c 'import platform; print(platform.python_version())')" == "$python_version" ]]
}

if ! doxygen_matches || ! graphviz_matches || ! python_matches; then
    if command -v pkgx >/dev/null 2>&1 && [[ -z "${CDT_DOXYGEN_TOOLCHAIN_ACTIVE:-}" ]]; then
        export CDT_DOXYGEN_TOOLCHAIN_ACTIVE=1
        exec pkgx \
            "+doxygen.nl@$doxygen_version" \
            "+graphviz.org@$graphviz_version" \
            "+python.org@$python_version" \
            -- "$0" "$mode" "$doxygen_version" "$graphviz_version" "$python_version"
    fi

    echo "Doxygen $doxygen_version, Graphviz $graphviz_version, and Python $python_version are required." >&2
    echo "Install those versions or install pkgx so the repository can provide them ephemerally." >&2
    exit 1
fi

temporary_output="$(mktemp -d "${TMPDIR:-/tmp}/cdt-doxygen.XXXXXX")"
cleanup() {
    rm -rf "$temporary_output"
}
trap cleanup EXIT

config_output="$temporary_output"
warning_log="$temporary_output/doxygen-warnings.log"
config_warning_log="$warning_log"
if command -v cygpath >/dev/null 2>&1; then
    config_output="$(cygpath -m "$temporary_output")"
    config_warning_log="$(cygpath -m "$warning_log")"
fi

{
    cat docs/Doxyfile
    printf '\nOUTPUT_DIRECTORY = "%s"\n' "$config_output"
    printf 'WARN_AS_ERROR = NO\n'
    printf 'WARN_LOGFILE = "%s"\n' "$config_warning_log"
} | doxygen -

generated_html="$temporary_output/html"
if [[ ! -f "$generated_html/index.html" ]]; then
    echo "Doxygen completed without generating html/index.html." >&2
    exit 1
fi

python3 scripts/validate_generated_site.py \
    --warning-log "$warning_log" \
    "$generated_html"

if [[ "$mode" == "build" ]]; then
    published_html="$repo_root/docs/html"
    rm -rf "$published_html"
    mv "$generated_html" "$published_html"
    touch "$published_html/.nojekyll"
    echo "Documentation generated in docs/html/."
else
    echo "Documentation validation complete."
fi
