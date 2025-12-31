#!/usr/bin/env bash
set -euo pipefail

# Ensure Meson subprojects are available before configuring the build.
# This script downloads missing wrap dependencies and reports any failures.

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

require_cmd() {
  local cmd="$1"
  if ! command -v "$cmd" >/dev/null 2>&1; then
    echo "Error: required command '$cmd' was not found in PATH." >&2
    exit 1
  fi
}

require_cmd meson
require_cmd ninja
require_cmd pkg-config

# Fetch all wrap-based dependencies up front so builds don't fail halfway
# through configuration or compilation.
meson subprojects download --sourcedir "$ROOT_DIR"

status_output=$(meson subprojects status --sourcedir "$ROOT_DIR")
missing=$(printf '%s\n' "$status_output" | grep -E "not downloaded|download error" || true)

if [[ -n "$missing" ]]; then
  echo "One or more subprojects failed to download:" >&2
  printf '%s\n' "$missing" >&2
  exit 1
fi

echo "All Meson subprojects are available." >&2
