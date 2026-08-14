#!/usr/bin/env bash
# Build openpilot natively and publish a device-installable prebuilt branch.
# Usage: BRANCH=<target-branch> [BUILD_DIR=...] release/build_prebuilt_ci.sh
set -euo pipefail

BRANCH="${BRANCH:?set BRANCH to the prebuilt target branch}"
SOURCE_DIR="$(git rev-parse --show-toplevel)"
BUILD_DIR="${BUILD_DIR:-/tmp/openpilot-prebuilt}"
PUB_DIR="${PUB_DIR:-/tmp/openpilot-publish}"
SCONS_CACHE_DIR="${SCONS_CACHE_DIR:-$HOME/.scons_cache}"

cd "$SOURCE_DIR"
GIT_HASH="$(git rev-parse HEAD)"
DATETIME="$(date '+%Y-%m-%dT%H:%M:%S')"
VERSION="$(awk -F'\"' '{print $2}' common/version.h || echo unknown)"

echo "[-] staging release files -> $BUILD_DIR"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
./release/release_files.py | sort -u | rsync -rRl --files-from=- . "$BUILD_DIR/"

# release_files.py excludes tinygrad_repo, but scons compiles models via
# tinygrad_repo/examples/openpilot/compile3.py — stage it for the build only
# (excluded again from the published branch, matching the upstream release shape).
rsync -a tinygrad_repo/ "$BUILD_DIR/tinygrad_repo/"

cd "$BUILD_DIR"
export PYTHONPATH="$BUILD_DIR"
# panda jungle import breaks headless CI builds (same workaround as sunnypilot)
sed -i '/from .board.jungle import PandaJungle, PandaJungleDFU/s/^/#/' panda/__init__.py || true

echo "[-] scons"
scons -j"$(nproc)" cache_dir="$SCONS_CACHE_DIR" --minimal
touch prebuilt

echo "[-] publishing $BRANCH"
ORIGIN_URL="$(git -C "$SOURCE_DIR" config --get remote.origin.url)"
rm -rf "$PUB_DIR"
if git ls-remote --exit-code "$ORIGIN_URL" "refs/heads/$BRANCH" >/dev/null 2>&1; then
  git clone --depth 1 --branch "$BRANCH" "$ORIGIN_URL" "$PUB_DIR"
else
  mkdir -p "$PUB_DIR"
  git -C "$PUB_DIR" init -q
  git -C "$PUB_DIR" checkout -q --orphan "$BRANCH"
fi
git -C "$PUB_DIR" remote remove origin 2>/dev/null || true
git -C "$PUB_DIR" remote add origin "$ORIGIN_URL"
cd "$PUB_DIR"
find . -maxdepth 1 -not -path './.git' -not -name '.' -exec rm -rf '{}' +
rsync -a \
  --exclude='.sconsign.dblite' --exclude='*.o' --exclude='*.os' --exclude='*.a' \
  --exclude='__pycache__/' --exclude='.scons_cache/' --exclude='tinygrad_repo/' \
  "$BUILD_DIR"/ .
BIG_FILES="$(find . -type f -not -path './.git/*' -size +95M)"
if [ -n "$BIG_FILES" ]; then
  # GitHub rejects >100MB blobs on push (no LFS: the on-device updater has no git-lfs).
  # Split into parts; launch_chffrplus.sh reassembles on the device.
  echo "$BIG_FILES" | while read -r f; do
    echo "splitting $f for GitHub's 100MB limit"
    split -b 95m "$f" "$f.part-" && rm "$f"
  done
fi
git config http.postBuffer 1048576000
PUSH_URL="origin"
if [ -n "${GITHUB_TOKEN:-}" ] && [ -n "${GITHUB_REPOSITORY:-}" ]; then
  PUSH_URL="https://x-access-token:${GITHUB_TOKEN}@github.com/${GITHUB_REPOSITORY}.git"
fi

# GitHub Smart HTTP 408s on giant single-commit packs (~first publish is ~2GB).
# Big blobs (>20M, incl. the .part-* chunks) ride their own commits so each push pack stays modest.
commit_push() {
  git diff --cached --quiet && return 0
  git -c user.name="github-actions[bot]" -c user.email="github-actions[bot]@users.noreply.github.com" commit -qm "$1"
  git push -q -f "$PUSH_URL" "HEAD:$BRANCH"
}

mapfile -t BIGS < <(find . -type f -not -path './.git/*' -size +20M)
git add -f .
if [ ${#BIGS[@]} -gt 0 ]; then
  if git rev-parse -q --verify HEAD >/dev/null; then
    git reset -q -- "${BIGS[@]}"        # re-run: keep HEAD's blobs, base commit carries only small-file delta
  else
    git rm -qf --cached -- "${BIGS[@]}" # first publish (unborn HEAD): exclude bigs from base commit
  fi
fi
commit_push "openpilot v$VERSION prebuilt

date: $DATETIME
source commit: $GIT_HASH"
for f in "${BIGS[@]}"; do
  git add -f -- "$f"
  commit_push "blob: $f"
done
echo "[-] done: $BRANCH @ source $GIT_HASH"
