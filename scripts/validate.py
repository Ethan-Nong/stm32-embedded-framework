#!/usr/bin/env python3
"""Validate the simplified framework layout."""

from __future__ import annotations

import json
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]

REQUIRED_DIRS = [
    ROOT / "platform",
    ROOT / "projects",
    ROOT / "app" / "main",
    ROOT / "shared" / "common",
    ROOT / "shared" / "board",
    ROOT / "shared" / "bsp" / "include",
    ROOT / "shared" / "bsp" / "port",
    ROOT / "shared" / "drivers",
    ROOT / "shared" / "modules",
    ROOT / "scripts",
]

REQUIRED_PLATFORM_FILES = [
    "cube",
    "overrides",
    "platform_config.h",
    "board_config.h",
]

REQUIRED_PROJECT_FILES = [
    "project.json",
    "project_config.h",
    "README.md",
]


def main() -> int:
    errors: list[str] = []
    checks = 0

    for path in REQUIRED_DIRS:
        if path.is_dir():
            checks += 1
        else:
            errors.append(f"missing directory: {path.relative_to(ROOT)}")

    platforms = [path for path in (ROOT / "platform").iterdir() if path.is_dir()]
    for platform_dir in platforms:
        for item in REQUIRED_PLATFORM_FILES:
            if (platform_dir / item).exists():
                checks += 1
            else:
                errors.append(
                    f"missing platform item: {platform_dir.name}/{item.as_posix()}"
                )

    for project_dir in sorted((ROOT / "projects").iterdir()):
        if not project_dir.is_dir():
            continue
        for name in REQUIRED_PROJECT_FILES:
            if (project_dir / name).is_file():
                checks += 1
            else:
                errors.append(f"missing project file: {project_dir.name}/{name}")

        manifest_path = project_dir / "project.json"
        try:
            manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError) as exc:
            errors.append(f"invalid manifest: {manifest_path.relative_to(ROOT)}: {exc}")
            continue

        platform = manifest.get("platform")
        if not (ROOT / "platform" / str(platform)).is_dir():
            errors.append(f"{project_dir.name} references missing platform: {platform}")

        app = manifest.get("app")
        if not (ROOT / "app" / str(app)).is_dir():
            errors.append(f"{project_dir.name} references missing app: {app}")

    for item in errors:
        print(f"[ERROR] {item}")
    print(f"[SUMMARY] checks={checks} platforms={len(platforms)} errors={len(errors)}")
    return 1 if errors else 0


if __name__ == "__main__":
    raise SystemExit(main())
