#!/usr/bin/env bash
set -euo pipefail

REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
LOG_DIR="${REPO_DIR}/logs"
mkdir -p "${LOG_DIR}"
RUN_LOG="${LOG_DIR}/sync_cann_master_to_github_main.log"
SUMMARY_FILE="${LOG_DIR}/sync_cann_master_to_github_main.summary"

mkdir -p "${HOME}/.cache/pto-isa"
LOCK_FILE="${HOME}/.cache/pto-isa/sync_cann_to_github_main.lock"
exec 9>"${LOCK_FILE}"
if ! flock -n 9; then
  echo "[$(date -Is)] Another sync is running; exiting." | tee -a "${RUN_LOG}"
  exit 0
fi

touch "${RUN_LOG}"
exec > >(tee -a "${RUN_LOG}") 2>&1

SYNC_STATUS="ERROR"
SYNC_MODE="none"
START_TS="$(date -Is)"
HEAD_BEFORE=""
HEAD_AFTER=""
ORIGIN_SHA=""
CANN_SHA=""
BASE_SHA=""
DETAILS=""

log() {
  echo "[$(date -Is)] $*"
}

restore_github_docs_surface() {
  local source_ref="$1"
  shift || true
  local preserve_paths=("$@")
  if [[ ${#preserve_paths[@]} -eq 0 ]]; then
    return 0
  fi

  if git diff --quiet "${source_ref}" -- "${preserve_paths[@]}"; then
    log "GitHub-owned docs surface unchanged; nothing to restore"
    return 0
  fi

  log "restoring GitHub-owned docs surface from ${source_ref}"
  git checkout "${source_ref}" -- "${preserve_paths[@]}"
}

write_summary() {
  cat > "${SUMMARY_FILE}" <<EOF
status=${SYNC_STATUS}
mode=${SYNC_MODE}
start=${START_TS}
end=$(date -Is)
head_before=${HEAD_BEFORE}
head_after=${HEAD_AFTER}
origin_main=${ORIGIN_SHA}
cann_master=${CANN_SHA}
merge_base=${BASE_SHA}
details=${DETAILS}
EOF
}

finish() {
  write_summary
}
trap finish EXIT

die() {
  DETAILS="$*"
  log "ERROR: $*"
  exit 1
}

require_remote() {
  local name="$1"
  git remote get-url "$name" >/dev/null 2>&1 || die "remote '$name' not found"
}

cd "${REPO_DIR}"

if [[ -n "$(git status --porcelain=v1)" ]]; then
  git status --porcelain=v1 >&2
  die "working tree not clean; refusing to sync"
fi

for r in cann origin; do
  require_remote "$r"
done

ORIGIN_URL="$(git remote get-url origin)"
CANN_URL="$(git remote get-url cann)"
log "origin=${ORIGIN_URL}"
log "cann=${CANN_URL}"

case "${ORIGIN_URL}" in
  *github.com:PTO-ISA/pto-isa.git|*github.com/PTO-ISA/pto-isa.git) ;;
  *) die "origin must point at GitHub PTO-ISA/pto-isa.git for direct org sync; got: ${ORIGIN_URL}" ;;
esac

case "${CANN_URL}" in
  *gitcode.com:cann/pto-isa.git|*gitcode.com/cann/pto-isa.git) ;;
  *) die "cann must point at gitcode cann/pto-isa.git; got: ${CANN_URL}" ;;
esac

log "fetching remotes"
git fetch cann --prune
git fetch origin --prune

if git show-ref --verify --quiet refs/heads/main; then
  CURRENT_BRANCH="$(git rev-parse --abbrev-ref HEAD)"
  if [[ "${CURRENT_BRANCH}" != "main" ]]; then
    log "checking out main"
    git checkout main
  fi
else
  die "local branch 'main' does not exist"
fi

if git show-ref --verify --quiet refs/remotes/origin/main; then
  log "fast-forwarding local main to origin/main"
  git merge --ff-only origin/main || die "cannot fast-forward local main to origin/main"
else
  die "origin/main not found after fetch"
fi

if ! git show-ref --verify --quiet refs/remotes/cann/master; then
  die "cann/master not found after fetch"
fi

HEAD_BEFORE="$(git rev-parse HEAD)"
ORIGIN_SHA="$(git rev-parse origin/main)"
CANN_SHA="$(git rev-parse cann/master)"
BASE_SHA="$(git merge-base HEAD cann/master || true)"

log "main=${HEAD_BEFORE}"
log "origin/main=${ORIGIN_SHA}"
log "cann/master=${CANN_SHA}"
log "merge-base=${BASE_SHA}"

if [[ "${HEAD_BEFORE}" != "${ORIGIN_SHA}" ]]; then
  die "local main diverged from origin/main after ff-only step"
fi

if [[ "${HEAD_BEFORE}" == "${CANN_SHA}" ]]; then
  SYNC_STATUS="OK"
  SYNC_MODE="noop"
  HEAD_AFTER="${HEAD_BEFORE}"
  DETAILS="main already matches cann/master"
  log "main already matches cann/master; nothing to do"
  exit 0
fi

if [[ -n "${BASE_SHA}" && "${BASE_SHA}" == "${HEAD_BEFORE}" ]]; then
  SYNC_MODE="fast-forward"
  DETAILS="fast-forward main to cann/master"
  log "cann/master is ahead of main; attempting ff-only update"
  git merge --ff-only cann/master || die "expected fast-forward from main to cann/master, but ff-only failed"
elif [[ -n "${BASE_SHA}" && "${BASE_SHA}" == "${CANN_SHA}" ]]; then
  SYNC_STATUS="OK"
  SYNC_MODE="noop-behind"
  HEAD_AFTER="${HEAD_BEFORE}"
  DETAILS="cann/master is behind main; nothing to merge"
  log "cann/master is behind main; nothing to merge"
  exit 0
else
  SYNC_MODE="merge"
  DETAILS="merge divergent histories to preserve both sides"
  MSG="sync: merge cann/master into main ($(date +%Y-%m-%d))"
  log "histories diverged; creating merge commit to preserve both histories"
  git merge --no-ff --no-edit -m "${MSG}" cann/master || {
    git merge --abort || true
    die "merge conflict while merging cann/master into main"
  }
fi

DOCS_PRESERVE_PATHS=(
  docs/mkdocs/src/assets
  docs/figures/pto_logo.svg
)
restore_github_docs_surface "${HEAD_BEFORE}" "${DOCS_PRESERVE_PATHS[@]}"
if [[ -n "$(git status --porcelain=v1 -- "${DOCS_PRESERVE_PATHS[@]}")" ]]; then
  log "committing preserved GitHub docs surface"
  git add -- "${DOCS_PRESERVE_PATHS[@]}"
  git commit -m "Preserve GitHub-owned docs chrome during GitCode sync" -m "The GitCode sync is allowed to update shared source content, but the GitHub docs chrome and branding assets remain GitHub-owned and must not be clobbered by upstream sync merges. Restore those assets from the pre-sync GitHub main tip before pushing the sync result."
fi

HEAD_AFTER="$(git rev-parse HEAD)"
log "pushing ${HEAD_AFTER} to origin main"
git push origin HEAD:main
SYNC_STATUS="OK"
DETAILS="sync completed successfully"
log "DONE. main moved ${HEAD_BEFORE} -> ${HEAD_AFTER} (${SYNC_MODE})"
log "SUMMARY: status=${SYNC_STATUS} mode=${SYNC_MODE} before=${HEAD_BEFORE} after=${HEAD_AFTER} origin=${ORIGIN_SHA} cann=${CANN_SHA}"
