# Justfile for the CDT++ maintenance workflow.
# Usage: just <recipe> or just --list

set minimum-version := "1.57.0"
set shell := ["bash", "-euo", "pipefail", "-c"]

just_version := "1.57.0"
uv_version := "0.11.29"
pinact_version := "4.1.0"
pinact_module := "github.com/suzuki-shunsuke/pinact/v4/cmd/pinact@v" + pinact_version
llvm_version := "22"
cmake_version := "4.4.0"
ninja_version := "1.13.2"
doxygen_version := "1.17.0"
graphviz_version := "15.1.0"
zizmor_version := "1.26.1"
primary_binary := if os_family() == "windows" { "out/build/reference/src/cdt.exe" } else { "out/build/reference/src/cdt" }
rng_benchmark_binary := if os_family() == "windows" { "out/build/reference/tests/CDT_rng_benchmark.exe" } else { "out/build/reference/tests/CDT_rng_benchmark" }

# Build the supported configuration through the repository build script.
[group('workflows')]
build:
    {{ if os_family() == "windows" { "cmd.exe //d //c 'scripts\\build.bat'" } else { "just _build-unix" } }}

# Run fast, non-mutating local validation.
[group('workflows')]
check: _justfile-check _format-check _yaml-check _action-lint _zizmor _whitespace-check _cmake-check release-check python-check semgrep semgrep-test
    @echo "Checks complete."

# Run the comprehensive pre-commit/pre-push validation gate.
[group('workflows')]
ci: check _pinact-check build
    @echo "CI validation complete."

# Configure dependencies before CodeQL begins tracing the C++ build.
[group('workflows')]
codeql-prepare:
    just _codeql-phase prepare

# Compile only project-owned production targets for CodeQL extraction.
[group('workflows')]
codeql-build:
    just _codeql-phase build

# Validate the generated API documentation without modifying the worktree.
[group('workflows')]
docs-check:
    ./scripts/doxygen.sh check "{{ doxygen_version }}" "{{ graphviz_version }}"

# Generate the API documentation in docs/html for local inspection or publishing.
[group('workflows')]
docs:
    ./scripts/doxygen.sh build "{{ doxygen_version }}" "{{ graphviz_version }}"

# Measure run-owned PCG sampling against the removed entropy-per-draw design.
[group('workflows')]
benchmark-rng draws='10000': build
    {{ rng_benchmark_binary }} {{ draws }}

# Apply safe automatic formatting to C++/Python source and the Justfile.
[group('workflows')]
fix: _format-fix python-fix
    just --fmt
    @echo "Fixes applied."

# Run Clang-Tidy with the pinned LLVM toolchain.
[group('workflows')]
clang-tidy:
    ./scripts/clang-tidy.sh

# Validate release metadata, citation fields, and version synchronization.
[group('workflows')]
release-check: _ensure-uv
    uv run --locked python scripts/release_check.py

# Scan production and correctness-test sources for repository-owned policies.
[group('workflows')]
semgrep: _ensure-uv
    #!/usr/bin/env bash
    set -euo pipefail
    state_dir="$(mktemp -d "${TMPDIR:-/tmp}/cdt-semgrep-state.XXXXXX")"
    trap 'rm -rf "$state_dir"' EXIT
    SEMGREP_LOG_FILE="$state_dir/semgrep.log" SEMGREP_SEND_METRICS=off \
        SEMGREP_SETTINGS_FILE="$state_dir/settings.yml" SEMGREP_VERSION_CACHE_PATH="$state_dir/version-cache" \
        uv run --locked semgrep scan --error --strict --timeout 120 --no-git-ignore \
            --config semgrep.yaml --exclude tests/semgrep include src tests

# Test repository-owned Semgrep rules against annotated positive and negative fixtures.
[group('workflows')]
semgrep-test: _ensure-uv
    #!/usr/bin/env bash
    set -euo pipefail
    config_dir="$(mktemp -d "${TMPDIR:-/tmp}/cdt-semgrep-config.XXXXXX")"
    state_root="$(mktemp -d "${TMPDIR:-/tmp}/cdt-semgrep-state.XXXXXX")"
    cleanup() {
        rm -rf "$config_dir" "$state_root"
    }
    trap cleanup EXIT

    while IFS= read -r -d '' fixture; do
        rel="${fixture#tests/semgrep/}"
        config_path="$config_dir/${rel%.*}.yaml"
        state_dir="$state_root/${rel%.*}"
        mkdir -p "$(dirname "$config_path")" "$state_dir"
        uv run --locked python scripts/semgrep_fixture_config.py "$fixture" "$PWD/semgrep.yaml" "$config_path"
        SEMGREP_LOG_FILE="$state_dir/semgrep.log" SEMGREP_SEND_METRICS=off \
            SEMGREP_SETTINGS_FILE="$state_dir/settings.yml" SEMGREP_VERSION_CACHE_PATH="$state_dir/version-cache" \
            uv run --locked semgrep scan --test --strict --config "$config_path" "$fixture"
    done < <(find tests/semgrep -type f ! -name '*.fixed' -print0)

# Build and exercise one supported Linux sanitizer configuration.
[group('workflows')]
sanitize kind:
    ./scripts/sanitizer.sh {{ kind }}

# Run every non-mutating Python source check.
[group('workflows')]
python-check: python-format-check python-lint python-typecheck python-support-test python-entrypoint-test
    @echo "Python source checks complete."

# Apply Ruff lint fixes and formatting to Python source.
[group('workflows')]
python-fix: _ensure-uv
    uv run --locked ruff check scripts/ --fix
    uv run --locked ruff format scripts/

# Check Python formatting with Ruff.
[group('workflows')]
python-format-check: _ensure-uv
    uv run --locked ruff format --check scripts/

# Lint Python source with Ruff.
[group('workflows')]
python-lint: _ensure-uv
    uv run --locked ruff check scripts/

# Test repository-owned Python support scripts.
[group('workflows')]
python-support-test: _ensure-uv
    uv run --locked python -m unittest discover -s scripts/tests -p 'test_*.py'

# Smoke-test installed entry points without loading optional experiment dependencies.
[group('workflows')]
python-entrypoint-test: _ensure-uv
    uv run --locked cdt-bootstrap-vcpkg --help >/dev/null
    uv run --locked cdt-optimize-initialize --help >/dev/null
    uv run --locked cdt-mnist-experiment --help >/dev/null

# Synchronize the lightweight Python development environment from the lockfile.
[group('workflows')]
python-sync: _ensure-uv
    uv sync --locked --group dev

# Synchronize dependencies required by the optional experiment scripts.
[group('workflows')]
python-sync-experiments: _ensure-uv
    uv sync --locked --group dev --group experiments

# Type-check Python support code with ty.
[group('workflows')]
python-typecheck: _ensure-uv
    uv run --locked ty check scripts/*.py scripts/tests/*.py --error all

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
_action-lint: _ensure-uv
    #!/usr/bin/env bash
    set -euo pipefail
    files=()
    while IFS= read -r -d '' file; do
      [[ -f "$file" ]] && files+=("$file")
    done < <(git ls-files -co --exclude-standard -z -- '.github/workflows/*.yml' '.github/workflows/*.yaml')
    uv run --locked actionlint "${files[@]}"

[private]
_build-unix:
    #!/usr/bin/env bash
    set -euo pipefail
    if command -v pkgx >/dev/null; then
      exec ./scripts/pkgx-build.sh
    fi
    exec ./scripts/build.sh

[private]
_codeql-phase phase:
    #!/usr/bin/env bash
    set -euo pipefail
    if command -v pkgx >/dev/null; then
      exec ./scripts/pkgx-build.sh --codeql {{ phase }}
    fi
    exec ./scripts/codeql-build.sh {{ phase }}

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
_ensure-uv:
    #!/usr/bin/env bash
    set -euo pipefail
    command -v uv >/dev/null || { echo "uv {{ uv_version }} is required." >&2; exit 1; }
    actual_version="$(uv --version | awk '{print $2}')"
    if [[ "$actual_version" != "{{ uv_version }}" ]]; then
      echo "uv {{ uv_version }} is required; found $actual_version." >&2
      exit 1
    fi

[private]
_format-check: _ensure-uv
    #!/usr/bin/env bash
    set -euo pipefail
    uv run --locked clang-format --version | grep -Eq 'clang-format version {{ llvm_version }}([.]|$)'
    files=()
    while IFS= read -r -d '' file; do
      [[ -f "$file" ]] && files+=("$file")
    done < <(git ls-files -co --exclude-standard -z -- '*.c' '*.cc' '*.cpp' '*.h' '*.hpp')
    if [[ "${#files[@]}" -gt 0 ]]; then
      uv run --locked clang-format --dry-run --Werror "${files[@]}"
    fi

[private]
_format-fix: _ensure-uv
    #!/usr/bin/env bash
    set -euo pipefail
    uv run --locked clang-format --version | grep -Eq 'clang-format version {{ llvm_version }}([.]|$)'
    files=()
    while IFS= read -r -d '' file; do
      [[ -f "$file" ]] && files+=("$file")
    done < <(git ls-files -co --exclude-standard -z -- '*.c' '*.cc' '*.cpp' '*.h' '*.hpp')
    if [[ "${#files[@]}" -gt 0 ]]; then
      uv run --locked clang-format -i "${files[@]}"
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
    if command -v go >/dev/null; then
      exec go run "{{ pinact_module }}" {{ args }}
    fi
    echo "pinact {{ pinact_version }} is required; install it, Go, or pkgx." >&2
    exit 1

[private]
_pinact-check:
    just _pinact run -fix=false -no-api

[private]
_whitespace-check:
    #!/usr/bin/env bash
    set -euo pipefail
    set +e
    git --no-pager grep -nI -E '[[:blank:]]+$' -- .
    status=$?
    set -e
    if [[ "$status" -eq 0 ]]; then
      echo "Trailing whitespace found in tracked files." >&2
      exit 1
    fi
    [[ "$status" -eq 1 ]] || exit "$status"

[private]
_yaml-check: _ensure-uv
    #!/usr/bin/env bash
    set -euo pipefail
    files=(.clang-format)
    while IFS= read -r -d '' file; do
      [[ -f "$file" ]] && files+=("$file")
    done < <(git ls-files -co --exclude-standard -z -- '*.yml' '*.yaml')
    uv run --locked yamllint "${files[@]}"

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
