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

cmake --build "${BUILD_DIR}"
ctest --test-dir "${BUILD_DIR}" --output-on-failure

CDT_EXE="${BUILD_DIR}/src/cdt"
if [[ ! -x "${CDT_EXE}" && -x "${BUILD_DIR}/src/Debug/cdt" ]]; then
  CDT_EXE="${BUILD_DIR}/src/Debug/cdt"
fi
if [[ ! -x "${CDT_EXE}" && -x "${BUILD_DIR}/src/Release/cdt" ]]; then
  CDT_EXE="${BUILD_DIR}/src/Release/cdt"
fi
if [[ ! -x "${CDT_EXE}" && -x "${BUILD_DIR}/src/RelWithDebInfo/cdt" ]]; then
  CDT_EXE="${BUILD_DIR}/src/RelWithDebInfo/cdt"
fi

for point in ${COUPLING_POINTS}; do
  IFS=',' read -r KAPPA0 KAPPA4 DELTA <<< "${point}"
  for target in ${TARGET_VOLUMES}; do
    for chain in $(seq 1 "${CHAINS}"); do
      seed=$((BASE_SEED + chain))
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
