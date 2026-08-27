"""
Copyright (c) 2026-, Zeph Leggett.

This file is part of zoompilot and is licensed under the MIT License.
See the LICENSE.md file in the root directory for more details.
"""
import pytest

from openpilot.common.params import Params
from openpilot.sunnypilot.system import params_migration
from openpilot.sunnypilot.system.params_migration import (
  LEGACY_BRANCHES,
  ZOOMPILOT_BRANCH,
  ZOOMPILOT_ORIGIN,
  _migrate_zoompilot_channel,
)

LEGACY_HTTPS = "https://github.com/zephleggett/openpilot.git"
LEGACY_SSH = "git@github.com:zephleggett/openpilot.git"


def _setup(monkeypatch, origin, branch):
  """Stub the two git reads and capture any `git remote set-url`."""
  def fake_default(cmd, default="", cwd=None, env=None):
    if "remote.origin.url" in cmd:
      return origin
    if "--abbrev-ref" in cmd:
      return branch
    return default

  set_url_calls = []
  monkeypatch.setattr(params_migration, "run_cmd_default", fake_default)
  monkeypatch.setattr(params_migration, "run_cmd", lambda cmd, cwd=None, env=None: set_url_calls.append(cmd))
  return set_url_calls


@pytest.mark.parametrize(("origin", "branch"), [
  (LEGACY_HTTPS, "mazda-dev"),
  (LEGACY_HTTPS, "zoompilot"),
  (LEGACY_SSH, "mazda-dev"),
])
def test_repoints_legacy_origin_and_targets_main(monkeypatch, origin, branch):
  params = Params()
  params.remove("UpdaterTargetBranch")
  calls = _setup(monkeypatch, origin, branch)

  try:
    _migrate_zoompilot_channel(params)
    assert calls == [["git", "remote", "set-url", "origin", ZOOMPILOT_ORIGIN]]
    assert params.get("UpdaterTargetBranch") == ZOOMPILOT_BRANCH
  finally:
    params.remove("UpdaterTargetBranch")


def test_targets_main_without_rewriting_an_already_migrated_origin(monkeypatch):
  """Origin already repointed but the update has not been installed yet."""
  params = Params()
  params.remove("UpdaterTargetBranch")
  calls = _setup(monkeypatch, ZOOMPILOT_ORIGIN, "mazda-dev")

  try:
    _migrate_zoompilot_channel(params)
    assert calls == []
    assert params.get("UpdaterTargetBranch") == ZOOMPILOT_BRANCH
  finally:
    params.remove("UpdaterTargetBranch")


@pytest.mark.parametrize("branch", ["develop", "develop-prebuilt"])
@pytest.mark.parametrize("origin", [ZOOMPILOT_ORIGIN, LEGACY_HTTPS, LEGACY_SSH])
def test_never_moves_anyone_off_develop(monkeypatch, origin, branch):
  """develop is its own install channel. Whatever the origin looks like, a device
  sitting on it must keep both its remote and its target branch."""
  params = Params()
  params.remove("UpdaterTargetBranch")
  calls = _setup(monkeypatch, origin, branch)

  try:
    _migrate_zoompilot_channel(params)
    assert calls == [], "develop device had its origin rewritten"
    assert params.get("UpdaterTargetBranch") is None, "develop device was retargeted"
  finally:
    params.remove("UpdaterTargetBranch")


def test_published_channels_are_not_migration_sources():
  """Guard against anyone widening LEGACY_BRANCHES into a live install channel."""
  for protected in ("develop", "main", "main-prebuilt", "develop-prebuilt"):
    assert protected not in LEGACY_BRANCHES


@pytest.mark.parametrize(("origin", "branch"), [
  (ZOOMPILOT_ORIGIN, ZOOMPILOT_BRANCH),   # migration complete, must go quiet
  (ZOOMPILOT_ORIGIN, "main-prebuilt"),    # prebuilt channel is left alone
  (LEGACY_HTTPS, "master"),               # someone else's branch on the fork
  (LEGACY_HTTPS, "dev-c3-new"),
  (ZOOMPILOT_ORIGIN, "HEAD"),             # detached HEAD reads back as "HEAD"
  ("https://github.com/commaai/openpilot.git", "master"),
  ("", "mazda-dev"),                      # git read failed
])
def test_leaves_everything_else_alone(monkeypatch, origin, branch):
  params = Params()
  params.remove("UpdaterTargetBranch")
  calls = _setup(monkeypatch, origin, branch)

  try:
    _migrate_zoompilot_channel(params)
    assert calls == []
    assert params.get("UpdaterTargetBranch") is None
  finally:
    params.remove("UpdaterTargetBranch")
