#!/usr/bin/env python3
"""Create a project from project_a."""

from __future__ import annotations

import argparse
import json
import re
import shutil
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
PROJECTS_ROOT = ROOT / "projects"
PLATFORM_ROOT = ROOT / "platform"
APP_ROOT = ROOT / "app"


def valid_name(value: str) -> bool:
    return re.fullmatch(r"[A-Za-z][A-Za-z0-9_]*", value) is not None


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--name", required=True)
    parser.add_argument("--platform", required=True)
    parser.add_argument("--template", default="project_a")
    args = parser.parse_args(argv)

    if not valid_name(args.name):
        print("[ERROR] invalid project name")
        return 1
    if not (PLATFORM_ROOT / args.platform).is_dir():
        print(f"[ERROR] platform not found: {args.platform}")
        return 1

    template_project_dir = PROJECTS_ROOT / args.template
    template_app_dir = APP_ROOT / args.template
    project_dir = PROJECTS_ROOT / args.name
    app_dir = APP_ROOT / args.name
    if not template_project_dir.is_dir():
        print(f"[ERROR] project template not found: {template_project_dir}")
        return 1
    if not template_app_dir.is_dir():
        print(f"[ERROR] app template not found: {template_app_dir}")
        return 1
    if project_dir.exists():
        print(f"[ERROR] project already exists: {project_dir}")
        return 1
    if app_dir.exists():
        print(f"[ERROR] app already exists: {app_dir}")
        return 1

    project_dir.mkdir(parents=True)
    for name in ("project.json", "project_config.h", "README.md"):
        shutil.copy2(template_project_dir / name, project_dir / name)

    shutil.copytree(template_app_dir, app_dir)
    for path in sorted(app_dir.rglob("*"), key=lambda item: len(item.parts), reverse=True):
        if args.template not in path.name:
            continue
        renamed = path.with_name(path.name.replace(args.template, args.name))
        path.rename(renamed)

    for path in list(project_dir.rglob("*")) + list(app_dir.rglob("*")):
        if not path.is_file():
            continue
        if path.suffix.lower() not in {".c", ".h", ".json", ".md"}:
            continue
        text = path.read_text(encoding="utf-8")
        text = text.replace(args.template, args.name)
        text = text.replace(args.template.upper(), args.name.upper())
        text = text.replace(
            f"app_{args.template}",
            f"app_{args.name}",
        )
        with path.open("w", encoding="utf-8", newline="\n") as handle:
            handle.write(text)

    manifest_path = project_dir / "project.json"
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    manifest["name"] = args.name
    manifest["platform"] = args.platform
    manifest["app"] = args.name
    manifest_path.write_text(
        json.dumps(manifest, indent=2, ensure_ascii=False) + "\n",
        encoding="utf-8",
    )

    print(f"[OK] created project: {project_dir.relative_to(ROOT)}")
    print(f"[OK] created app: {app_dir.relative_to(ROOT)}")
    print(f"[NEXT] python scripts/sync_keil.py {args.name}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
