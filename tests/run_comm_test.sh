#!/usr/bin/env bash
# Copyright (c) 2025 Huawei Technologies Co., Ltd.
# This program is free software, you can redistribute it and/or modify it under the terms and conditions of
# CANN Open Software License Agreement Version 2.0 (the "License").
# Please refer to the License for details. You may not use this file except in compliance with the License.
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
# See LICENSE in the root of the software repository for the full text of the License.

set -euo pipefail

# ============================================================================
# Usage
# ============================================================================
usage() {
  cat <<EOF
Usage: $(basename "$0") [-n NPU_COUNT] [-v VERSION] [-t TESTCASE] [-d]

Options:
  -n NPU_COUNT   Number of NPUs (devices) available: 2, 4, or 8 (default: 8)
                 Only test cases requiring <= NPU_COUNT ranks will run.
  -v VERSION     SoC version: a3 (Ascend910B, default) or a5 (Ascend910_9599).
  -t TESTCASE    Run only the specified testcase (e.g. tput, treduce).
                 Can be specified multiple times. Default: run all.
  -d             Enable debug mode (extra logging at each sync point).
  -h             Show this help message.

Examples:
  $(basename "$0")                   # Run all tests with 8 NPUs on a3
  $(basename "$0") -n 2              # Run only 2-rank tests
  $(basename "$0") -v a5 -n 2 -t tput  # Run tput on A5 with 2 NPUs
  $(basename "$0") -d -t tput        # Run tput with debug output
EOF
  exit 0
}

# ============================================================================
# Parse arguments
# ============================================================================
NPU_COUNT=8
SOC_VERSION="a3"
DEBUG_FLAG=""
declare -a SELECTED_TESTS=()

while getopts "n:v:t:dh" opt; do
  case "$opt" in
    n) NPU_COUNT="$OPTARG" ;;
    v) SOC_VERSION="$OPTARG" ;;
    t) SELECTED_TESTS+=("$OPTARG") ;;
    d) DEBUG_FLAG="-d" ;;
    h) usage ;;
    *) usage ;;
  esac
done

if [[ "$NPU_COUNT" != 2 && "$NPU_COUNT" != 4 && "$NPU_COUNT" != 8 ]]; then
  echo "[ERROR] -n must be 2, 4, or 8 (got: ${NPU_COUNT})" >&2
  exit 1
fi

if [[ "$SOC_VERSION" != "a3" && "$SOC_VERSION" != "a5" ]]; then
  echo "[ERROR] -v must be a3 or a5 (got: ${SOC_VERSION})" >&2
  exit 1
fi

# ============================================================================
# Build gtest filter for a specific rank count.
#
# Each test expects an exact MPI world size (ForkAndRunWithHcclRootInfo checks
# mpiSize == nRanks). The script runs the binary once per distinct rank count,
# using gtest filters to select only the matching tests each time.
#
# Tests following the *_NRanks / *_Nranks naming convention are matched by
# pattern. Tests without a rank suffix in their names are listed explicitly.
# ============================================================================

# Tests that need 4 ranks but lack "4Ranks"/"4ranks" in their name.
KNOWN_4RANK_TESTS_tput="TPut.Vec_FloatSmall:TPut.AtomicAdd_Int32"
KNOWN_4RANK_TESTS_tgather="TGather.FloatSmall"
KNOWN_4RANK_TESTS_tscatter="TScatter.FloatSmall"
KNOWN_4RANK_TESTS_treduce="TReduce.FloatSmall_Sum"
KNOWN_4RANK_TESTS_tbroadcast="TBroadCast.FloatSmallRoot0"

# Tests that need 8 ranks but lack "8Ranks"/"8ranks" in their name.
KNOWN_8RANK_TESTS_tput="TPut.Vec_Uint8Small"

get_known_tests() {
  local test_name="$1"
  local nranks="$2"
  local varname="KNOWN_${nranks}RANK_TESTS_${test_name}"
  echo "${!varname:-}"
}

get_gtest_filter_for_nranks() {
  local test_name="$1"
  local nranks="$2"

  local known4; known4="$(get_known_tests "$test_name" 4)"
  local known8; known8="$(get_known_tests "$test_name" 8)"

  case "$nranks" in
    2)
      # All tests EXCEPT those needing 4 or 8 ranks.
      local negative="*4Ranks*:*4ranks*:*8Ranks*:*8ranks*"
      [[ -n "$known4" ]] && negative="${negative}:${known4}"
      [[ -n "$known8" ]] && negative="${negative}:${known8}"
      echo "*-${negative}"
      ;;
    4)
      # Only tests needing exactly 4 ranks.
      local positive="*4Ranks*:*4ranks*"
      [[ -n "$known4" ]] && positive="${positive}:${known4}"
      echo "${positive}"
      ;;
    8)
      # Only tests needing exactly 8 ranks.
      local positive="*8Ranks*:*8ranks*"
      [[ -n "$known8" ]] && positive="${positive}:${known8}"
      echo "${positive}"
      ;;
  esac
}

# ============================================================================
# Discover testcases
# ============================================================================
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
if [[ "$SOC_VERSION" == "a5" ]]; then
  ST_DIR="${ROOT_DIR}/tests/npu/a5/comm/st/testcase"
else
  ST_DIR="${ROOT_DIR}/tests/npu/a2a3/comm/st/testcase"
fi

if [[ ! -d "${ST_DIR}" ]]; then
  echo "[ERROR] testcase dir not found: ${ST_DIR}" >&2
  exit 1
fi

declare -a tests=()
if [[ "${#SELECTED_TESTS[@]}" -gt 0 ]]; then
  tests=("${SELECTED_TESTS[@]}")
else
  while IFS= read -r -d '' dir; do
    tests+=("$(basename "${dir}")")
  done < <(find "${ST_DIR}" -maxdepth 1 -mindepth 1 -type d -print0 | sort -z)
fi

if [[ "${#tests[@]}" -eq 0 ]]; then
  echo "[ERROR] No testcase directories found under ${ST_DIR}" >&2
  exit 1
fi

echo "[INFO] NPU_COUNT=${NPU_COUNT}, SOC=${SOC_VERSION}, DEBUG=${DEBUG_FLAG:-(off)}, running ${#tests[@]} testcase(s): ${tests[*]}"

# ============================================================================
# Run
# ============================================================================
fail_count=0
total_runs=0

for t in "${tests[@]}"; do
  built=false
  for nranks in 2 4 8; do
    if (( nranks > NPU_COUNT )); then continue; fi

    gtest_filter="$(get_gtest_filter_for_nranks "$t" "$nranks")"
    [[ -z "$gtest_filter" ]] && continue

    echo "============================================================"
    echo "[INFO] Running testcase: ${t}  (nranks=${nranks}, GTEST_FILTER=${gtest_filter})"
    echo "============================================================"

    build_flag=""
    if $built; then
      build_flag="-w"
    fi
    built=true

    total_runs=$((total_runs + 1))
    if ! GTEST_FILTER="${gtest_filter}" \
         python3 "${ROOT_DIR}/tests/script/run_st.py" -r npu -v "${SOC_VERSION}" ${DEBUG_FLAG} ${build_flag} -n "${nranks}" -t "comm/${t}"; then
      echo "[ERROR] Testcase failed: ${t} (nranks=${nranks})" >&2
      fail_count=$((fail_count + 1))
    fi
  done
done

echo "============================================================"
if [[ "${fail_count}" -eq 0 ]]; then
  echo "[INFO] All ${total_runs} comm ST run(s) passed (NPU_COUNT=${NPU_COUNT}, SOC=${SOC_VERSION})."
  exit 0
else
  echo "[ERROR] ${fail_count}/${total_runs} run(s) failed."
  exit 1
fi
