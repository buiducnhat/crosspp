#!/usr/bin/env python3
"""
CrossPP Automated Release Script

Builds production firmware assets and publishes/updates a GitHub release
matching the standard 1.0.0 / 1.0.1 / 1.0.3 specification:
  - Title: <version> (e.g. 1.0.4)
  - Assets: firmware.bin, bootloader.bin, partitions.bin
  - Release Notes: ## CrossPP <version>, ### What's New, ### Downloads, ### How to Install

Usage:
  python3 scripts/release.py <version> [--notes "custom markdown notes"]
"""

import argparse
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
BUILD_DIR = REPO_ROOT / ".pio" / "build" / "gh_release"
FIRMWARE_BIN = BUILD_DIR / "firmware.bin"
BOOTLOADER_BIN = BUILD_DIR / "bootloader.bin"
PARTITIONS_BIN = BUILD_DIR / "partitions.bin"
DEFAULT_REPO = "buiducnhat/crosspp"


def find_pio() -> str:
    pio_path = shutil.which("pio")
    if pio_path:
        return pio_path
    candidate = Path.home() / ".platformio" / "penv" / "bin" / "pio"
    if candidate.exists() and os.access(candidate, os.X_OK):
        return str(candidate)
    print("Error: PlatformIO 'pio' command not found in PATH or ~/.platformio/penv/bin/pio", file=sys.stderr)
    sys.exit(1)


def check_git_status():
    res = subprocess.run(["git", "status", "--porcelain"], cwd=REPO_ROOT, capture_output=True, text=True)
    tracked_changes = [line for line in res.stdout.strip().splitlines() if not line.startswith("??")]
    if tracked_changes:
        print("Warning: uncommitted changes detected in tracked files:", file=sys.stderr)
        for line in tracked_changes:
            print(f"  {line}", file=sys.stderr)
        ans = input("Continue anyway? [y/N]: ").strip().lower()
        if ans != "y":
            sys.exit(1)


def verify_platformio_ini_version(version: str):
    ini_path = REPO_ROOT / "platformio.ini"
    content = ini_path.read_text(encoding="utf-8")
    m = re.search(r"\[crosspoint\]\s*\n\s*version\s*=\s*([^\s\n]+)", content)
    if not m:
        print(f"Warning: Could not find [crosspoint] version in {ini_path}", file=sys.stderr)
        return
    ini_ver = m.group(1)
    if ini_ver != version:
        print(f"Version mismatch: platformio.ini has '{ini_ver}', but target release is '{version}'", file=sys.stderr)
        ans = input(f"Update platformio.ini to {version}? [Y/n]: ").strip().lower()
        if ans not in ("", "y", "yes"):
            sys.exit(1)
        new_content = re.sub(r"(\[crosspoint\]\s*\n\s*version\s*=\s*)[^\s\n]+", rf"\g<1>{version}", content)
        ini_path.write_text(new_content, encoding="utf-8")
        print(f"Updated platformio.ini version to {version}")


def build_release_binaries(pio: str):
    print(f"\n==> Building production firmware (pio run -e gh_release)...")
    res = subprocess.run([pio, "run", "-e", "gh_release"], cwd=REPO_ROOT)
    if res.returncode != 0:
        print("Error: PlatformIO build failed", file=sys.stderr)
        sys.exit(res.returncode)

    for path, name in [(FIRMWARE_BIN, "firmware.bin"), (BOOTLOADER_BIN, "bootloader.bin"), (PARTITIONS_BIN, "partitions.bin")]:
        if not path.exists() or path.stat().st_size == 0:
            print(f"Error: Missing or empty artifact {name} at {path}", file=sys.stderr)
            sys.exit(1)
        print(f"  ✓ {name}: {path.stat().st_size / 1024:.1f} KB")


def generate_default_notes(version: str, notes_body: str = "") -> str:
    if not notes_body:
        notes_body = """### What's New

- Performance, stability, and feature updates. Refer to commit log for details."""

    return f"""## CrossPP {version}

{notes_body}

### Downloads
- **`firmware.bin`**: Production firmware binary for ESP32-C3 based devices (Xteink X3 and Xteink X4).
- **`bootloader.bin`**: Bootloader binary.
- **`partitions.bin`**: Partition table binary.

### How to Install
1. Download `firmware.bin`.
2. Connect your device via USB-C.
3. Flash via web flasher (e.g. at [crosspointreader.com/#flash-tools](https://crosspointreader.com/#flash-tools) with Custom .bin) or using `esptool.py`.
"""


def ensure_git_tag(version: str):
    res = subprocess.run(["git", "tag", "-l", version], cwd=REPO_ROOT, capture_output=True, text=True)
    if version not in res.stdout.splitlines():
        print(f"\n==> Creating git tag {version}...")
        subprocess.run(["git", "tag", "-a", version, "-m", f"Release {version}"], cwd=REPO_ROOT, check=True)
    else:
        print(f"  ✓ Git tag {version} already exists locally")


def push_to_remote(version: str, repo: str):
    ans = input(f"\nPush branch and tag '{version}' to remote origin? [Y/n]: ").strip().lower()
    if ans not in ("", "y", "yes"):
        print("Skipping push to remote.")
        return False
    subprocess.run(["git", "push", "origin", "develop", version], cwd=REPO_ROOT, check=True)
    return True


def publish_github_release(version: str, repo: str, notes: str):
    gh = shutil.which("gh")
    if not gh:
        print("Warning: 'gh' CLI not found. Binaries are built, but GitHub release must be created manually.", file=sys.stderr)
        return

    print(f"\n==> Publishing release to GitHub ({repo})...")
    # Check if release exists
    res = subprocess.run([gh, "release", "view", version, "--repo", repo], cwd=REPO_ROOT, capture_output=True, text=True)
    if res.returncode == 0:
        print(f"Release {version} exists. Updating title, notes, and uploading assets...")
        subprocess.run([gh, "release", "edit", version, "--repo", repo, "--title", version, "--notes", notes], cwd=REPO_ROOT, check=True)
        subprocess.run([gh, "release", "upload", version, str(FIRMWARE_BIN), str(BOOTLOADER_BIN), str(PARTITIONS_BIN),
                        "--repo", repo, "--clobber"], cwd=REPO_ROOT, check=True)
    else:
        print(f"Creating new GitHub release {version} with assets...")
        subprocess.run([
            gh, "release", "create", version,
            str(FIRMWARE_BIN), str(BOOTLOADER_BIN), str(PARTITIONS_BIN),
            "--repo", repo,
            "--title", version,
            "--notes", notes,
        ], cwd=REPO_ROOT, check=True)

    print(f"\n✓ Successfully published: https://github.com/{repo}/releases/tag/{version}")


def main():
    parser = argparse.ArgumentParser(description="CrossPP Release Automation")
    parser.add_argument("version", help="Release version string, e.g. 1.0.4")
    parser.add_argument("--notes", help="Custom markdown body for release notes", default="")
    parser.add_argument("--repo", help="Target GitHub repository", default=DEFAULT_REPO)
    parser.add_argument("--skip-build", help="Skip PlatformIO build step", action="store_true")
    parser.add_argument("--no-push", help="Skip pushing to git remote", action="store_true")
    args = parser.parse_args()

    version = args.version.lstrip("v")  # Standard is without leading 'v'
    print(f"==================================================")
    print(f" CrossPP Release Process for Version: {version}")
    print(f"==================================================")

    check_git_status()
    verify_platformio_ini_version(version)

    pio = find_pio()
    if not args.skip_build:
        build_release_binaries(pio)

    ensure_git_tag(version)

    pushed = False
    if not args.no_push:
        pushed = push_to_remote(version, args.repo)

    notes = generate_default_notes(version, args.notes)
    if pushed or args.no_push:
        publish_github_release(version, args.repo, notes)

    print("\nRelease workflow completed successfully!")


if __name__ == "__main__":
    main()
