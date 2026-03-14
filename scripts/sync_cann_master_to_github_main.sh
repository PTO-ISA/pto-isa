#!/usr/bin/env bash
set -euo pipefail

REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
LOG_DIR="${REPO_DIR}/logs"
mkdir -p "${LOG_DIR}"

mkdir -p "${HOME}/.cache/pto-isa"
LOCK_FILE="${HOME}/.cache/pto-isa/sync_cann_to_github_main.lock"
exec 9>"${LOCK_FILE}"
if ! flock -n 9; then
  echo "[$(date -Is)] Another sync is running; exiting."
  exit 0
fi

log() {
  echo "[$(date -Is)] $*"
}

die() {
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

HEAD_SHA="$(git rev-parse HEAD)"
ORIGIN_SHA="$(git rev-parse origin/main)"
CANN_SHA="$(git rev-parse cann/master)"
BASE="$(git merge-base HEAD cann/master || true)"

log "main=${HEAD_SHA}"
log "origin/main=${ORIGIN_SHA}"
log "cann/master=${CANN_SHA}"

if [[ "${HEAD_SHA}" != "${ORIGIN_SHA}" ]]; then
  die "local main diverged from origin/main after ff-only step"
fi

if [[ "${HEAD_SHA}" == "${CANN_SHA}" ]]; then
  log "main already matches cann/master; nothing to do"
  exit 0
fi

if [[ -n "${BASE}" && "${BASE}" == "${HEAD_SHA}" ]]; then
  log "cann/master is ahead of main; attempting ff-only update"
  git merge --ff-only cann/master || die "expected fast-forward from main to cann/master, but ff-only failed"
elif [[ -n "${BASE}" && "${BASE}" == "${CANN_SHA}" ]]; then
  log "cann/master is behind main; nothing to merge"
  exit 0
else
  MSG="sync: merge cann/master into main ($(date +%Y-%m-%d))"
  log "histories diverged; creating merge commit to preserve both histories"
  git merge --no-ff --no-edit -m "${MSG}" cann/master || {
    git merge --abort || true
    die "merge conflict while merging cann/master into main"
  }
fi

NEW_SHA="$(git rev-parse HEAD)"
log "pushing ${NEW_SHA} to origin main"
git push origin HEAD:main
log "DONE. main is now at ${NEW_SHA}"
