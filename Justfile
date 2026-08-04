# Justfile for the CDT++ maintenance workflow.
# Usage: just <recipe> or just --list

set minimum-version := "1.58.0"
set shell := ["bash", "-euo", "pipefail", "-c"]

just_version := "1.58.0"
uv_version := "0.12.1"
python_version := "3.14.6"
git_cliff_version := "2.13.1"
actionlint_version := "1.7.12"
pinact_version := "4.1.1"
pinact_module := "github.com/suzuki-shunsuke/pinact/v4/cmd/pinact@v" + pinact_version
typos_version := "1.49.0"
llvm_version := "22"
cmake_minimum_version := "4.4.0"
cmake_version := "4.4.1"
ninja_version := "1.13.2"
ninja_windows_wheel_version := "1.13.0"
ccache_version := "4.13.6"
doxygen_version := "1.17.0"
graphviz_version := "15.1.0"
zizmor_version := "1.28.0"
primary_binary := if os_family() == "windows" { "out/build/reference/src/cdt.exe" } else { "out/build/reference/src/cdt" }
rng_benchmark_binary := if os_family() == "windows" { "out/build/reference/tests/CDT_rng_benchmark.exe" } else { "out/build/reference/tests/CDT_rng_benchmark" }
cgal_benchmark_binary := if os_family() == "windows" { "out/build/reference/tests/CDT_cgal_benchmark.exe" } else { "out/build/reference/tests/CDT_cgal_benchmark" }
parallel_cgal_benchmark_binary := if os_family() == "windows" { "out/build/parallel/tests/CDT_cgal_benchmark.exe" } else { "out/build/parallel/tests/CDT_cgal_benchmark" }
reference_fixture_binary := if os_family() == "windows" { "out/build/reference/tests/CDT_reference_fixture.exe" } else { "out/build/reference/tests/CDT_reference_fixture" }

# Build the supported configuration through the repository build script.
[group('workflows')]
build:
    {{ if os_family() == "windows" { "cmd.exe //d //c 'scripts\\build.bat'" } else { "just _build-unix" } }}

# Build and test the opt-in CGAL/oneTBB configuration.
[group('workflows')]
build-parallel:
    {{ if os_family() == "windows" { "cmd.exe //d //c 'scripts\\build.bat parallel'" } else { "just _build-parallel-unix" } }}

# Build production targets in Debug mode and run the supported CLI integration tests.
[group('workflows')]
build-debug:
    {{ if os_family() == "windows" { "cmd.exe //d //c 'scripts\\build.bat debug'" } else { "just _build-debug-unix" } }}

# Run fast, non-mutating local validation.
[group('workflows')]
check: _justfile-check _format-check _yaml-check _action-lint _zizmor _whitespace-check _cmake-check release-check python-check reference-check semgrep semgrep-test spell-check
    @echo "Checks complete."

# Run the comprehensive pre-commit/pre-push validation gate.
[group('workflows')]
ci: check _pinact-check reference-generated-check python-package-check
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

# Build with GNU coverage instrumentation and generate LCOV and HTML reports.
[group('workflows')]
coverage:
    #!/usr/bin/env bash
    set -euo pipefail
    if command -v pkgx >/dev/null; then
      exec pkgx \
        "+cmake.org@{{ cmake_version }}" \
        "+ninja-build.org@{{ ninja_version }}" \
        -- ./scripts/coverage.sh
    fi
    exec ./scripts/coverage.sh

# Measure run-owned PCG sampling against the removed entropy-per-draw design.
[group('workflows')]
benchmark-rng draws='10000': build
    {{ rng_benchmark_binary }} {{ draws }}

# Measure the deterministic sequential CGAL reference configuration.
[group('workflows')]
benchmark-cgal simplices='640' repetitions='5' moves='50' warmups='1': build
    {{ cgal_benchmark_binary }} {{ simplices }} {{ repetitions }} {{ moves }} 1 {{ warmups }}

# Record matched one-thread and increasing-thread CGAL/oneTBB scaling samples.
[group('workflows')]
benchmark-cgal-parallel threads='1 2 4' simplices='640' repetitions='5' moves='50' warmups='1': build-parallel
    #!/usr/bin/env bash
    set -euo pipefail
    read -r -a thread_counts <<< {{ quote(threads) }}
    [[ "${#thread_counts[@]}" -gt 0 ]] || {
      echo "At least one thread count is required." >&2
      exit 2
    }
    for thread_count in "${thread_counts[@]}"; do
      {{ parallel_cgal_benchmark_binary }} \
        {{ quote(simplices) }} {{ quote(repetitions) }} {{ quote(moves) }} \
        "${thread_count}" {{ quote(warmups) }}
    done

# Print the raw deterministic C++ oracle for the versioned reference package.
[group('workflows')]
reference-fixtures: build
    {{ reference_fixture_binary }}

# Regenerate every raw reference artifact and manifest from one clean commit.
[group('workflows')]
reference-regenerate: _sync-python-dev
    uv run --no-sync python scripts/generate_reference_fixtures.py --check-clean
    just build
    just build-parallel
    uv run --no-sync python scripts/generate_reference_fixtures.py
    just reference-check
    just reference-archive-check

# Validate schemas, exact topology, numerical oracles, and provenance.
[group('workflows')]
reference-check: _sync-python-dev
    uv run --no-sync python scripts/validate_reference_fixtures.py

# Require one clean source revision before an archival tag or publication.
[group('workflows')]
reference-archive-check: _sync-python-dev
    uv run --no-sync python scripts/validate_reference_fixtures.py \
        --provenance-only

# Rebuild the C++ fixture producer and compare its canonical scientific payload.
[group('workflows')]
reference-generated-check: build _sync-python-dev
    uv run --no-sync python scripts/validate_reference_fixtures.py \
        --generated-only --fixture-binary {{ quote(reference_fixture_binary) }}

# Apply safe automatic formatting to C++/Python source and the Justfile.
[group('workflows')]
fix: _format-fix python-fix
    just --fmt
    @echo "Fixes applied."

# Run Clang-Tidy with the pinned LLVM toolchain.
[group('workflows')]
clang-tidy:
    #!/usr/bin/env bash
    set -euo pipefail
    if ! command -v pkgx >/dev/null 2>&1; then
        exec ./scripts/clang-tidy.sh
    fi
    exec pkgx \
      +git-scm.org@2.55.0 \
      "+just.systems@{{ just_version }}" \
      "+llvm.org@{{ llvm_version }}" \
      "+cmake.org@{{ cmake_version }}" \
      "+ninja-build.org@{{ ninja_version }}" \
      "+python.org@{{ python_version }}" \
      +gnu.org/m4@1.4.21 \
      +gnu.org/autoconf@2.73.0 \
      +gnu.org/autoconf-archive@2024.10.16 \
      +gnu.org/automake@1.18.1 \
      +gnu.org/libtool@2.5.4 \
      +gnu.org/texinfo@7.3.0 \
      +freedesktop.org/pkg-config@0.29.2 \
      -- ./scripts/clang-tidy.sh

# Validate release metadata, citation fields, and version synchronization.
[group('workflows')]
release-check: _sync-python-dev
    uv run --no-sync python scripts/release_check.py

# Generate the changelog as though the requested release tag already exists.
[group('release')]
changelog-unreleased version: _ensure-git-cliff _sync-python-dev
    uv run --no-sync python scripts/generate_changelog.py {{ quote(version) }}

# Validate and preview an annotated release tag without creating it.
[group('release')]
tag-check version: _sync-python-dev
    uv run --no-sync cdt-tag-release {{ quote(version) }} --dry-run

# Create an annotated release tag from the matching CHANGELOG.md section.
[group('release')]
tag version: _sync-python-dev
    uv run --no-sync cdt-tag-release {{ quote(version) }}

# Scan production and correctness-test sources for repository-owned policies.
[group('workflows')]
semgrep: _sync-python-dev
    #!/usr/bin/env bash
    set -euo pipefail
    state_dir="$(mktemp -d "${TMPDIR:-/tmp}/cdt-semgrep-state.XXXXXX")"
    trap 'rm -rf "$state_dir"' EXIT
    SEMGREP_ENABLE_VERSION_CHECK=0 SEMGREP_LOG_FILE="$state_dir/semgrep.log" SEMGREP_SEND_METRICS=off \
        SEMGREP_SETTINGS_FILE="$state_dir/settings.yml" SEMGREP_VERSION_CACHE_PATH="$state_dir/version-cache" \
        uv run --no-sync semgrep scan --error --strict --timeout 120 --no-git-ignore \
            --config semgrep.yaml \
            --exclude .cache --exclude .venv --exclude venv \
            --exclude 'build*' --exclude 'cmake-build*' --exclude cov-int --exclude coverage \
            --exclude html --exclude out --exclude Testing --exclude vcpkg_installed \
            --exclude tests/semgrep .

# Test repository-owned Semgrep rules against annotated positive and negative fixtures.
[group('workflows')]
semgrep-test: _sync-python-dev
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
        uv run --no-sync python scripts/semgrep_fixture_config.py "$fixture" "$PWD/semgrep.yaml" "$config_path"
        SEMGREP_LOG_FILE="$state_dir/semgrep.log" SEMGREP_SEND_METRICS=off \
            SEMGREP_SETTINGS_FILE="$state_dir/settings.yml" SEMGREP_VERSION_CACHE_PATH="$state_dir/version-cache" \
            uv run --no-sync semgrep scan --test --strict --config "$config_path" "$fixture"
    done < <(find tests/semgrep -type f ! -name '*.fixed' -print0)

# Check repository text and identifiers for common spelling mistakes.
[group('workflows')]
spell-check: _ensure-typos
    typos

# Build and exercise one supported Linux sanitizer configuration.
[group('workflows')]
sanitize kind:
    #!/usr/bin/env bash
    set -euo pipefail
    if command -v pkgx >/dev/null; then
      exec pkgx \
        "+llvm.org@{{ llvm_version }}" \
        "+cmake.org@{{ cmake_version }}" \
        "+ninja-build.org@{{ ninja_version }}" \
        -- ./scripts/sanitizer.sh "{{ kind }}"
    fi
    exec ./scripts/sanitizer.sh "{{ kind }}"

# Run every non-mutating Python source check.
[group('workflows')]
python-check: python-format-check python-lint python-typecheck python-support-test python-entrypoint-test
    @echo "Python source checks complete."

# Install, type-check, and exercise the heavyweight PyTorch/Comet surface without networked services or datasets.
[group('workflows')]
python-experiment-check: _sync-python-experiments
    uv run --no-sync ty check scripts/mnist_experiment.py scripts/optimize_initialize.py scripts/experiment_tests/*.py --error all
    MPLCONFIGDIR="${TMPDIR:-/tmp}/cdt-matplotlib-cache" uv run --no-sync python -c "import comet_ml; import torch; import torchvision; print(comet_ml.__version__, torch.__version__, torchvision.__version__)"
    MPLCONFIGDIR="${TMPDIR:-/tmp}/cdt-matplotlib-cache" uv run --no-sync python -m unittest scripts.experiment_tests.test_comet_pytorch
    MPLCONFIGDIR="${TMPDIR:-/tmp}/cdt-matplotlib-cache" uv run --no-sync python -m unittest scripts.experiment_tests.test_mnist_training

# Build both Python artifacts and exercise every installed entry point outside the checkout.
[group('workflows')]
python-package-check: _sync-python-dev
    #!/usr/bin/env bash
    set -euo pipefail
    artifact_directory="$(mktemp -d "${TMPDIR:-/tmp}/cdt-python-artifacts.XXXXXX")"
    consumer_directory="$(mktemp -d "${TMPDIR:-/tmp}/cdt-python-consumer.XXXXXX")"
    cleanup() {
      rm -rf "$artifact_directory" "$consumer_directory"
    }
    trap cleanup EXIT

    uv build --out-dir "$artifact_directory"
    wheel="$(find "$artifact_directory" -maxdepth 1 -name '*.whl' -print -quit)"
    [[ -n "$wheel" ]] || { echo "uv build did not produce a wheel." >&2; exit 1; }
    uv venv --python {{ python_version }} "$consumer_directory/.venv"
    uv pip install --python "$consumer_directory/.venv" --no-build "$wheel"

    if [[ -x "$consumer_directory/.venv/bin/python" ]]; then
      python="$consumer_directory/.venv/bin/python"
      scripts_directory="$consumer_directory/.venv/bin"
    else
      python="$consumer_directory/.venv/Scripts/python.exe"
      scripts_directory="$consumer_directory/.venv/Scripts"
    fi
    (
      cd "$consumer_directory"
      "$python" -c "import scripts"
      "$scripts_directory/cdt-bootstrap-vcpkg" --help >/dev/null
      "$scripts_directory/cdt-optimize-initialize" --help >/dev/null
      "$scripts_directory/cdt-mnist-experiment" --help >/dev/null
      "$scripts_directory/cdt-tag-release" --help >/dev/null
    )

# Apply Ruff lint fixes and formatting to Python source.
[group('workflows')]
python-fix: _sync-python-dev
    uv run --no-sync ruff check scripts/ --fix
    uv run --no-sync ruff format scripts/

# Check Python formatting with Ruff.
[group('workflows')]
python-format-check: _sync-python-dev
    uv run --no-sync ruff format --check scripts/

# Lint Python source with Ruff.
[group('workflows')]
python-lint: _sync-python-dev
    uv run --no-sync ruff check scripts/

# Test repository-owned Python support scripts.
[group('workflows')]
python-support-test: _sync-python-dev
    uv run --no-sync python -m unittest discover -s scripts/tests -p 'test_*.py'

# Smoke-test installed entry points without loading optional experiment dependencies.
[group('workflows')]
python-entrypoint-test: _sync-python-dev
    uv run --no-sync cdt-bootstrap-vcpkg --help >/dev/null
    uv run --no-sync cdt-optimize-initialize --help >/dev/null
    uv run --no-sync cdt-mnist-experiment --help >/dev/null
    uv run --no-sync cdt-tag-release --help >/dev/null
    uv run --no-sync python scripts/sync_vcpkg_tool_pins.py --help >/dev/null

# Synchronize the lightweight Python development environment from the lockfile.
[group('workflows')]
python-sync: _sync-python-dev
    @echo "Python development environment synchronized."

# Synchronize dependencies required by the optional experiment scripts.
[group('workflows')]
python-sync-experiments: _sync-python-experiments
    @echo "Python experiment environment synchronized."

# Type-check Python support code with ty.
[group('workflows')]
python-typecheck: _sync-python-dev
    uv run --no-sync ty check scripts/*.py scripts/tests/*.py --error all

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

# Synchronize the trusted vcpkg tool release and Windows hashes with the manifest baseline.
[group('workflows')]
sync-vcpkg-tool-pins: _sync-python-dev
    uv run --no-sync python scripts/sync_vcpkg_tool_pins.py

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
      [[ -f "$file" ]] && files+=("$file")
    done < <(git ls-files -co --exclude-standard -z -- '.github/workflows/*.yml' '.github/workflows/*.yaml')
    if command -v actionlint >/dev/null; then
      actual_version="$(actionlint --version | head -n 1)"
      if [[ "$actual_version" != "{{ actionlint_version }}" ]]; then
        echo "actionlint {{ actionlint_version }} is required; found $actual_version." >&2
        exit 1
      fi
      exec actionlint "${files[@]}"
    fi
    if command -v pkgx >/dev/null; then
      exec pkgx "actionlint@{{ actionlint_version }}" "${files[@]}"
    fi
    if command -v go >/dev/null; then
      exec go run "github.com/rhysd/actionlint/cmd/actionlint@v{{ actionlint_version }}" "${files[@]}"
    fi
    echo "actionlint {{ actionlint_version }} is required; install it, Go, or pkgx." >&2
    exit 1

[private]
_build-unix:
    #!/usr/bin/env bash
    set -euo pipefail
    if command -v pkgx >/dev/null; then
      exec ./scripts/pkgx-build.sh
    fi
    exec ./scripts/build.sh

[private]
_build-parallel-unix:
    #!/usr/bin/env bash
    set -euo pipefail
    if command -v pkgx >/dev/null; then
      exec ./scripts/pkgx-build.sh --preset parallel
    fi
    exec ./scripts/build.sh parallel

[private]
_build-debug-unix:
    #!/usr/bin/env bash
    set -euo pipefail
    if command -v pkgx >/dev/null; then
      exec ./scripts/pkgx-build.sh --preset debug
    fi
    exec ./scripts/build.sh debug

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
    minimum="{{ cmake_minimum_version }}"
    pinned="{{ cmake_version }}"
    if command -v cmake >/dev/null; then
      installed="$(cmake --version | awk 'NR == 1 { print $3 }')"
      installed="${installed%%-*}"
      IFS=. read -r installed_major installed_minor installed_patch <<< "$installed"
      IFS=. read -r minimum_major minimum_minor minimum_patch <<< "$minimum"
      if (( installed_major > minimum_major ||
            (installed_major == minimum_major &&
             (installed_minor > minimum_minor ||
              (installed_minor == minimum_minor &&
               installed_patch >= minimum_patch))) )); then
        exec cmake --list-presets=all >/dev/null
      fi
    fi
    if command -v pkgx >/dev/null; then
      exec pkgx "+cmake.org@$pinned" cmake --list-presets=all >/dev/null
    fi
    echo "CMake $minimum or newer is required; install it or install pkgx for the tested $pinned toolchain." >&2
    exit 1

[private]
_ensure-git-cliff:
    #!/usr/bin/env bash
    set -euo pipefail
    command -v git-cliff >/dev/null || {
      echo "git-cliff {{ git_cliff_version }} is required." >&2
      echo "Install it with: cargo install git-cliff --version {{ git_cliff_version }} --locked" >&2
      exit 1
    }
    actual_version="$(git-cliff --version)"
    if [[ "$actual_version" != "git-cliff {{ git_cliff_version }}" ]]; then
      echo "git-cliff {{ git_cliff_version }} is required; found $actual_version." >&2
      exit 1
    fi

[private]
_ensure-typos:
    #!/usr/bin/env bash
    set -euo pipefail
    command -v typos >/dev/null || {
      echo "typos-cli {{ typos_version }} is required." >&2
      echo "Install it with: cargo install typos-cli --version {{ typos_version }} --locked" >&2
      exit 1
    }
    actual_version="$(typos --version | awk '{print $2}')"
    if [[ "$actual_version" != "{{ typos_version }}" ]]; then
      echo "typos-cli {{ typos_version }} is required; found $actual_version." >&2
      exit 1
    fi

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
_sync-python-dev: _ensure-uv
    uv sync --locked --no-build --no-install-project --group dev
    uv sync --locked --only-install-project --inexact --group dev

[private]
_sync-python-experiments: _ensure-uv
    uv sync --locked --no-build --no-install-project --group dev --group experiments
    uv sync --locked --only-install-project --inexact --group dev --group experiments

[private]
_format-check: _sync-python-dev
    #!/usr/bin/env bash
    set -euo pipefail
    uv run --no-sync clang-format --version | grep -Eq 'clang-format version {{ llvm_version }}([.]|$)'
    files=()
    while IFS= read -r -d '' file; do
      [[ -f "$file" ]] && files+=("$file")
    done < <(git ls-files -co --exclude-standard -z -- '*.c' '*.cc' '*.cpp' '*.h' '*.hpp')
    if [[ "${#files[@]}" -gt 0 ]]; then
      uv run --no-sync clang-format --dry-run --Werror "${files[@]}"
    fi

[private]
_format-fix: _sync-python-dev
    #!/usr/bin/env bash
    set -euo pipefail
    uv run --no-sync clang-format --version | grep -Eq 'clang-format version {{ llvm_version }}([.]|$)'
    files=()
    while IFS= read -r -d '' file; do
      [[ -f "$file" ]] && files+=("$file")
    done < <(git ls-files -co --exclude-standard -z -- '*.c' '*.cc' '*.cpp' '*.h' '*.hpp')
    if [[ "${#files[@]}" -gt 0 ]]; then
      uv run --no-sync clang-format -i "${files[@]}"
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
_yaml-check: _sync-python-dev
    #!/usr/bin/env bash
    set -euo pipefail
    files=(.clang-format)
    while IFS= read -r -d '' file; do
      [[ -f "$file" ]] && files+=("$file")
    done < <(git ls-files -co --exclude-standard -z -- '*.yml' '*.yaml')
    uv run --no-sync yamllint "${files[@]}"

[private]
_zizmor: _ensure-uv
    uvx --no-build --from "zizmor=={{ zizmor_version }}" zizmor .github
