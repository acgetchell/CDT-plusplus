#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${BUILD_DIR:-build}"
RESULTS_DIR="${RESULTS_DIR:-results}"
TARGET_VOLUMES="${TARGET_VOLUMES:-8000 16000 32000 64000}"
COUPLING_POINTS="${COUPLING_POINTS:-1.0,0.2,0.1}"
CHAINS="${CHAINS:-4}"
TIMESLICES="${TIMESLICES:-32}"
THERMALIZATION="${THERMALIZATION:-1000}"
MEASUREMENT_INTERVAL="${MEASUREMENT_INTERVAL:-10}"
PASSES="${PASSES:-2000}"
VOLUME_EPSILON="${VOLUME_EPSILON:-0.001}"
BASE_SEED="${BASE_SEED:-1000}"

set -f

cmake --build "${BUILD_DIR}"
ctest --test-dir "${BUILD_DIR}" --output-on-failure

CDT_EXE=""
for candidate in \
  "${BUILD_DIR}/src/cdt" \
  "${BUILD_DIR}/src/cdt.exe" \
  "${BUILD_DIR}/src/Debug/cdt" \
  "${BUILD_DIR}/src/Debug/cdt.exe" \
  "${BUILD_DIR}/src/Release/cdt" \
  "${BUILD_DIR}/src/Release/cdt.exe" \
  "${BUILD_DIR}/src/RelWithDebInfo/cdt" \
  "${BUILD_DIR}/src/RelWithDebInfo/cdt.exe"; do
  if [[ -x "${candidate}" ]]; then
    CDT_EXE="${candidate}"
    break
  fi
done
if [[ -z "${CDT_EXE}" ]]; then
  echo "Could not find cdt executable under ${BUILD_DIR}." >&2
  exit 1
fi

for point in ${COUPLING_POINTS}; do
  IFS=',' read -r KAPPA0 KAPPA4 DELTA <<< "${point}"
  if [[ -z "${KAPPA0}" || -z "${KAPPA4}" || -z "${DELTA}" ]]; then
    echo "Coupling point '${point}' must contain kappa0,kappa4,Delta." >&2
    exit 1
  fi
  for target in ${TARGET_VOLUMES}; do
    for chain in $(seq 1 "${CHAINS}"); do
      seed="$(printf '%s' "${BASE_SEED}|${point}|${target}|${chain}" | cksum | awk '{print $1}')"
      run_id="cds-k0_${KAPPA0}-k4_${KAPPA4}-d_${DELTA}-n4_${target}-chain_${chain}"
      "${CDT_EXE}" \
        --spherical \
        --dimensions 4 \
        --simplices "${target}" \
        --timeslices "${TIMESLICES}" \
        --kappa0 "${KAPPA0}" \
        --kappa4 "${KAPPA4}" \
        --Delta "${DELTA}" \
        --target-n4 "${target}" \
        --volume-epsilon "${VOLUME_EPSILON}" \
        --passes "${PASSES}" \
        --thermalization "${THERMALIZATION}" \
        --measurement-interval "${MEASUREMENT_INTERVAL}" \
        --seed "${seed}" \
        --chain-id "chain-${chain}" \
        --run-id "${run_id}" \
        --output-dir "${RESULTS_DIR}"
    done
  done
done
