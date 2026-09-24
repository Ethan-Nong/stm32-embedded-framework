#!/usr/bin/env python3
"""Create or update one Keil project from a project manifest."""

from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
PLATFORM_ROOT = ROOT / "platform"
PROJECTS_ROOT = ROOT / "projects"
SHARED_ROOT = ROOT / "shared"
APP_ROOT = ROOT / "app"


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="ignore")


def write_text(path: Path, text: str) -> None:
    with path.open("w", encoding="utf-8", newline="\n") as handle:
        handle.write(text)


def load_manifest(project: str) -> dict:
    path = PROJECTS_ROOT / project / "project.json"
    if not path.is_file():
        raise ValueError(f"project manifest not found: {path}")
    return json.loads(path.read_text(encoding="utf-8"))


def find_base_project(platform: str, cube_dir: Path) -> Path:
    mdk_dir = cube_dir / "MDK-ARM"
    preferred = mdk_dir / f"{platform}.uvprojx"
    if preferred.is_file():
        return preferred

    projects = sorted(mdk_dir.glob("*.uvprojx"))
    if not projects:
        raise ValueError(f"base Keil project not found: {mdk_dir}")
    return projects[0]


def relative_to_mdk(path: Path, platform_dir: Path) -> str:
    mdk_dir = platform_dir / "cube" / "MDK-ARM"
    return os.path.relpath(path, mdk_dir).replace("\\", "/")


def selected_paths(manifest: dict, key: str, root: Path) -> list[Path]:
    paths = []
    for item in manifest.get(key, []):
        path = root / str(item)
        if not path.is_dir():
            raise ValueError(f"selected {key} directory not found: {path}")
        paths.append(path)
    return paths


def source_paths(directory: Path) -> list[Path]:
    source_root = directory / "src"
    if source_root.is_dir():
        return sorted(source_root.rglob("*.c"))
    return sorted(directory.rglob("*.c"))


def include_dirs(directory: Path) -> list[Path]:
    paths = [directory]
    include_dir = directory / "include"
    if include_dir.is_dir():
        paths.append(include_dir)
    return paths


def include_paths(manifest: dict, platform_dir: Path) -> list[str]:
    project = str(manifest["name"])
    app_name = str(manifest["app"])
    project_dir = PROJECTS_ROOT / project
    app_dir = APP_ROOT / app_name
    required = [
        "../Core/Inc",
        "../Drivers/STM32F4xx_HAL_Driver/Inc",
        "../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy",
        "../Drivers/CMSIS/Device/ST/STM32F4xx/Include",
        "../Drivers/CMSIS/Include",
        relative_to_mdk(platform_dir, platform_dir),
        relative_to_mdk(platform_dir / "overrides", platform_dir),
        relative_to_mdk(SHARED_ROOT / "board", platform_dir),
        relative_to_mdk(SHARED_ROOT / "bsp" / "include", platform_dir),
        relative_to_mdk(SHARED_ROOT / "common" / "include", platform_dir),
        relative_to_mdk(APP_ROOT / "main", platform_dir),
        relative_to_mdk(project_dir, platform_dir),
    ]

    for directory in [app_dir]:
        for path in include_dirs(directory):
            required.append(relative_to_mdk(path, platform_dir))

    for key, root in (
        ("modules", SHARED_ROOT / "modules"),
        ("drivers", SHARED_ROOT / "drivers"),
        ("components", SHARED_ROOT / "components"),
    ):
        for directory in selected_paths(manifest, key, root):
            for path in include_dirs(directory):
                required.append(relative_to_mdk(path, platform_dir))

    return list(dict.fromkeys(required))


def source_files(manifest: dict, platform_dir: Path) -> list[tuple[str, str]]:
    project = str(manifest["name"])
    app_name = str(manifest["app"])
    project_dir = PROJECTS_ROOT / project
    app_dir = APP_ROOT / app_name

    paths = [
        APP_ROOT / "main" / "app_main.c",
        SHARED_ROOT / "board" / "board.c",
    ]
    paths.extend(source_paths(app_dir))
    paths.extend(source_paths(project_dir))
    paths.extend(source_paths(SHARED_ROOT / "common"))

    for directory, prefix in (
        (SHARED_ROOT / "bsp" / "port", None),
        (platform_dir / "overrides", None),
    ):
        for path in sorted(directory.glob("*.c")):
            paths.append(path)

    for key, root in (
        ("modules", SHARED_ROOT / "modules"),
        ("drivers", SHARED_ROOT / "drivers"),
        ("components", SHARED_ROOT / "components"),
    ):
        for directory in selected_paths(manifest, key, root):
            paths.extend(source_paths(directory))

    unique_paths = list(dict.fromkeys(paths))
    return [
        (path.name, relative_to_mdk(path, platform_dir))
        for path in unique_paths
    ]


def set_project_identity(text: str, project: str) -> str:
    text = re.sub(
        r"<TargetName>[^<]*</TargetName>",
        f"<TargetName>{project}</TargetName>",
        text,
        count=1,
    )
    text = re.sub(
        r"<OutputDirectory>[^<]*</OutputDirectory>",
        f"<OutputDirectory>{project}\\</OutputDirectory>",
        text,
        count=1,
    )
    text = re.sub(
        r"<OutputName>[^<]*</OutputName>",
        f"<OutputName>{project}</OutputName>",
        text,
        count=1,
    )
    return text


def set_includes(text: str, required: list[str]) -> str:
    def replace(match: re.Match[str]) -> str:
        content = match.group(1) or ""
        if "../Core/Inc" not in content and "../Drivers/" not in content:
            return match.group(0)
        return "<IncludePath>" + ";".join(required) + "</IncludePath>"

    return re.sub(r"<IncludePath>(.*?)</IncludePath>", replace, text)


def add_defines(text: str, defines: list[str]) -> str:
    if not defines:
        return text

    match = re.search(r"<Define>(.*?)</Define>", text, re.DOTALL)
    if match is None or "USE_HAL_DRIVER" not in match.group(1):
        return text

    current = [item.strip() for item in match.group(1).split(",") if item.strip()]
    for item in defines:
        if item not in current:
            current.append(item)
    replacement = "<Define>" + ",".join(current) + "</Define>"
    return text[: match.start()] + replacement + text[match.end() :]


def remove_source_nodes(text: str, names: set[str]) -> str:
    for name in names:
        pattern = re.compile(
            rf"\s*<File>\s*<FileName>{re.escape(name)}</FileName>.*?</File>",
            re.DOTALL,
        )
        text = pattern.sub("", text)
    return text


def file_node(name: str, path: str) -> str:
    return (
        "            <File>\n"
        f"              <FileName>{name}</FileName>\n"
        "              <FileType>1</FileType>\n"
        f"              <FilePath>{path}</FilePath>\n"
        "            </File>\n"
    )


def add_sources(text: str, sources: list[tuple[str, str]]) -> str:
    nodes = "".join(file_node(name, path) for name, path in sources)
    pattern = re.compile(
        r"(?P<head>\s*<Group>\s*<GroupName>Application/User(?:/Core)?</GroupName>\s*<Files>\s*)"
        r"(?P<body>.*?)"
        r"(?P<tail>\s*</Files>\s*</Group>)",
        re.DOTALL,
    )
    match = pattern.search(text)
    if match is None:
        raise ValueError("Application/User/Core group not found in uvprojx")
    return text[: match.start("body")] + match.group("body") + nodes + text[match.start("tail") :]


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("project", help="directory name under projects/")
    parser.add_argument("--rebuild-from-base", action="store_true")
    args = parser.parse_args(argv)

    try:
        manifest = load_manifest(args.project)
        platform = str(manifest["platform"])
        platform_dir = PLATFORM_ROOT / platform
        cube_dir = platform_dir / "cube"
        base_project = find_base_project(platform, cube_dir)
        project_file = cube_dir / "MDK-ARM" / f"{args.project}.uvprojx"

        if args.rebuild_from_base or not project_file.is_file():
            shutil.copy2(base_project, project_file)

        text = read_text(project_file)
        text = set_project_identity(text, args.project)
        text = set_includes(text, include_paths(manifest, platform_dir))
        text = add_defines(text, [str(item) for item in manifest.get("defines", [])])

        sources = source_files(manifest, platform_dir)
        managed_names = {name for name, _ in sources}
        managed_names.update(
            {
                "bsp_uart_demo.c",
                "bsp_gpio_demo.c",
                "bsp_adc_demo.c",
                "bsp_soft_i2c_demo.c",
            }
        )
        text = remove_source_nodes(text, managed_names)
        text = add_sources(text, sources)
        write_text(project_file, text)
    except (KeyError, ValueError, OSError, json.JSONDecodeError) as exc:
        print(f"[ERROR] {exc}")
        return 1

    print(f"[OK] Keil project: {project_file.relative_to(ROOT)}")
    print(f"[OK] platform: {platform}")
    print(f"[OK] sources: {len(sources)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
