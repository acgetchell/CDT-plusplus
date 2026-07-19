# Justfile for the CDT++ maintenance workflow.
# Usage: just <recipe> or just --list

set minimum-version := "1.56.0"

pinact_version := "4.1.0"
pinact_module := "github.com/suzuki-shunsuke/pinact/v4/cmd/pinact@v" + pinact_version
clang_format_version := "18"
primary_binary := if os_family() == "windows" { "out/build/reference/src/cdt.exe" } else { "out/build/reference/src/cdt" }

# Build the supported configuration through the repository build script.
[group('workflows')]
build:
    {{ if os_family() == "windows" { "scripts\\build.bat" } else { "just _build-unix" } }}

# Run fast, non-mutating local validation.
[group('workflows')]
check: _justfile-check _format-check _yaml-check _action-lint _zizmor _whitespace-check _cmake-check
    @echo "Checks complete."

# Run the comprehensive pre-commit/pre-push validation gate.
[group('workflows')]
ci: check _pinact-check build
    @echo "CI validation complete."

# Apply safe automatic formatting to the Justfile and changed C++ lines.
[group('workflows')]
fix: _format-fix
    just --fmt
    @echo "Fixes applied."

# Build as needed and run the primary CDT++ executable.
[group('workflows')]
run *args: build
    {{ primary_binary }} {{ args }}

# Update and repin GitHub Actions, then validate the resulting workflows.
[group('workflows')]
update-actions:
    just _pinact run -update
    just _yaml-check
    just _action-lint
    just _zizmor

[default]
[private]
default:
    @just --list

[private]
_action-lint:
    #!/usr/bin/env bash
    set -euo pipefail
    files=()
    while IFS= read -r -d '' file; do
      files+=("$file")
    done < <(git ls-files -co --exclude-standard -z -- '.github/workflows/*.yml' '.github/workflows/*.yaml')
    if command -v actionlint >/dev/null; then
      actionlint "${files[@]}"
    elif command -v pkgx >/dev/null; then
      pkgx actionlint "${files[@]}"
    else
      echo "actionlint is required; install it or install pkgx." >&2
      exit 1
    fi

[private]
_build-unix:
    #!/usr/bin/env bash
    set -euo pipefail
    if [[ "${CDT_PKGX_ACTIVE:-0}" != 1 ]] && command -v pkgx >/dev/null; then
      exec ./scripts/pkgx-build.sh
    fi
    exec ./scripts/build.sh

[private]
_cmake-check:
    #!/usr/bin/env bash
    set -euo pipefail
    if command -v cmake >/dev/null; then
      exec cmake --list-presets=all >/dev/null
    fi
    if command -v pkgx >/dev/null; then
      exec pkgx +cmake.org cmake --list-presets=all >/dev/null
    fi
    echo "CMake is required; install it or install pkgx." >&2
    exit 1

[private]
_format-check:
    #!/usr/bin/env bash
    set -euo pipefail
    clang_format="$(command -v clang-format-{{ clang_format_version }} || command -v clang-format || true)"
    if [[ -n "$clang_format" ]] && ! "$clang_format" --version | grep -Eq 'clang-format version {{ clang_format_version }}([.]|$)'; then
      clang_format=""
    fi
    if [[ -z "$clang_format" ]] && command -v pkgx >/dev/null; then
      exec pkgx +llvm.org@{{ clang_format_version }} +python.org -- just _format-check
    fi
    [[ -n "$clang_format" ]] || { echo "clang-format {{ clang_format_version }} is required; install it or install pkgx." >&2; exit 1; }
    clang_format_prefix="$(cd -- "$(dirname -- "$clang_format")/.." && pwd)"
    clang_format_diff=""
    for candidate in \
      "$(command -v clang-format-diff-{{ clang_format_version }} || true)" \
      "${clang_format_prefix}/share/clang/clang-format-diff.py"; do
      if [[ -x "$candidate" ]]; then
        clang_format_diff="$candidate"
        break
      fi
    done
    [[ -n "$clang_format_diff" ]] || { echo "clang-format-diff.py from LLVM {{ clang_format_version }} is required." >&2; exit 1; }
    diff_output="$(git diff -U0 --no-color HEAD -- '*.c' '*.cc' '*.cpp' '*.h' '*.hpp' | "$clang_format_diff" -p1 -style file -binary "$clang_format")"
    if [[ -n "$diff_output" ]]; then
      printf '%s\n' "$diff_output"
      exit 1
    fi
    untracked=()
    while IFS= read -r -d '' file; do
      untracked+=("$file")
    done < <(git ls-files --others --exclude-standard -z -- '*.c' '*.cc' '*.cpp' '*.h' '*.hpp')
    if [[ "${#untracked[@]}" -gt 0 ]]; then
      "$clang_format" --dry-run --Werror "${untracked[@]}"
    fi

[private]
_format-fix:
    #!/usr/bin/env bash
    set -euo pipefail
    clang_format="$(command -v clang-format-{{ clang_format_version }} || command -v clang-format || true)"
    if [[ -n "$clang_format" ]] && ! "$clang_format" --version | grep -Eq 'clang-format version {{ clang_format_version }}([.]|$)'; then
      clang_format=""
    fi
    if [[ -z "$clang_format" ]] && command -v pkgx >/dev/null; then
      exec pkgx +llvm.org@{{ clang_format_version }} +python.org -- just _format-fix
    fi
    [[ -n "$clang_format" ]] || { echo "clang-format {{ clang_format_version }} is required; install it or install pkgx." >&2; exit 1; }
    clang_format_prefix="$(cd -- "$(dirname -- "$clang_format")/.." && pwd)"
    clang_format_diff=""
    for candidate in \
      "$(command -v clang-format-diff-{{ clang_format_version }} || true)" \
      "${clang_format_prefix}/share/clang/clang-format-diff.py"; do
      if [[ -x "$candidate" ]]; then
        clang_format_diff="$candidate"
        break
      fi
    done
    [[ -n "$clang_format_diff" ]] || { echo "clang-format-diff.py from LLVM {{ clang_format_version }} is required." >&2; exit 1; }
    git diff -U0 --no-color HEAD -- '*.c' '*.cc' '*.cpp' '*.h' '*.hpp' | "$clang_format_diff" -p1 -i -style file -binary "$clang_format"
    untracked=()
    while IFS= read -r -d '' file; do
      untracked+=("$file")
    done < <(git ls-files --others --exclude-standard -z -- '*.c' '*.cc' '*.cpp' '*.h' '*.hpp')
    if [[ "${#untracked[@]}" -gt 0 ]]; then
      "$clang_format" -i "${untracked[@]}"
    fi

[private]
_justfile-check:
    just --fmt --check

[private]
_pinact *args:
    #!/usr/bin/env bash
    set -euo pipefail
    if [[ -z "${PINACT_GITHUB_TOKEN:-}" && -z "${GITHUB_TOKEN:-}" ]] && command -v gh >/dev/null && gh auth token >/dev/null 2>&1; then
      export PINACT_GITHUB_TOKEN="$(gh auth token)"
    fi
    if command -v pinact >/dev/null; then
      exec pinact {{ args }}
    fi
    if command -v pkgx >/dev/null; then
      exec pkgx go run "{{ pinact_module }}" {{ args }}
    fi
    echo "pinact {{ pinact_version }} is required; install it with Homebrew or install pkgx." >&2
    exit 1

[private]
_pinact-check:
    just _pinact run -fix=false -no-api

[private]
_whitespace-check:
    git diff --check HEAD

[private]
_yaml-check:
    #!/usr/bin/env bash
    set -euo pipefail
    files=(.clang-format)
    while IFS= read -r -d '' file; do
      files+=("$file")
    done < <(git ls-files -co --exclude-standard -z -- '*.yml' '*.yaml')
    if command -v yamllint >/dev/null; then
      yamllint "${files[@]}"
    elif command -v pkgx >/dev/null; then
      pkgx yamllint "${files[@]}"
    else
      echo "yamllint is required; install it or install pkgx." >&2
      exit 1
    fi

[private]
_zizmor:
    #!/usr/bin/env bash
    set -euo pipefail
    if command -v zizmor >/dev/null; then
      zizmor .github
    elif command -v pkgx >/dev/null; then
      pkgx zizmor .github
    else
      echo "zizmor is required; install it or install pkgx." >&2
      exit 1
    fi
