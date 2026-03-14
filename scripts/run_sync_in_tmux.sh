#!/usr/bin/env bash
set -euo pipefail
SESSION_NAME="${1:-pto-isa-sync}"
REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
LOG_FILE="${REPO_DIR}/logs/sync_cann_master_to_github_main.log"
mkdir -p "${REPO_DIR}/logs"
if tmux has-session -t "${SESSION_NAME}" 2>/dev/null; then
  echo "tmux session '${SESSION_NAME}' already exists"
  exit 0
fi
tmux new-session -d -s "${SESSION_NAME}" "cd '${REPO_DIR}' && echo '[tmux] starting sync at '$(date -Is) | tee -a '${LOG_FILE}' && ./scripts/sync_cann_master_to_github_main.sh | tee -a '${LOG_FILE}'"
echo "started tmux session: ${SESSION_NAME}"
echo "attach with: tmux attach -t ${SESSION_NAME}"
