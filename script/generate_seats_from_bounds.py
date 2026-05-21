#!/usr/bin/env python3
"""Generate seat JSON and price-tier JSON from zone bounds JSON."""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path
from typing import Any


DEFAULT_SEAT_SIZE = 24
DEFAULT_SPACING = 5

# Curated palette: #AARRGGBB (opaque, vivid but balanced)
TIER_COLORS = [
    "#FF6366F1",  # indigo
    "#FF22D3EE",  # cyan
    "#FF34D399",  # emerald
    "#FFFBBF24",  # amber
    "#FFFB7185",  # rose
    "#FFA78BFA",  # violet
    "#FF38BDF8",  # sky
    "#FFF97316",  # orange
    "#FF4ADE80",  # green
    "#FFE879F9",  # fuchsia
]

# (zone_id_prefix, chunk_count, tier_names) — zones split evenly within each prefix group
PREFIX_GROUPS: list[tuple[str, int, list[str]]] = [
    ("10", 3, ["内场·普通", "内场·标准", "内场·优选"]),
    ("20", 1, ["看台·东"]),
    ("30", 1, ["看台·西"]),
    ("40", 3, ["上层·普通", "上层·优选", "上层·VIP"]),
]


def compute_axis_layout(
    region_length: float, seat_size: int, min_spacing: int
) -> tuple[int, float, float]:
    """Return seat count, step between seat origins, and leading offset along one axis."""
    min_step = seat_size + min_spacing
    if region_length < seat_size:
        return 0, 0.0, 0.0

    count = int((region_length - seat_size + min_spacing) // min_step)
    if count <= 0:
        return 0, 0.0, 0.0

    if count == 1:
        return 1, 0.0, (region_length - seat_size) / 2.0

    gap = (region_length - count * seat_size) / (count - 1)
    return count, seat_size + gap, 0.0


def chunk_list(items: list[str], chunk_count: int) -> list[list[str]]:
    if chunk_count <= 1 or len(items) <= 1:
        return [items]
    chunk_count = min(chunk_count, len(items))
    base_size = len(items) // chunk_count
    remainder = len(items) % chunk_count
    chunks: list[list[str]] = []
    index = 0
    for chunk_index in range(chunk_count):
        size = base_size + (1 if chunk_index < remainder else 0)
        chunks.append(items[index : index + size])
        index += size
    return chunks


def build_pricecode_tiers(zone_ids: list[str]) -> list[dict[str, Any]]:
    """Build price-tier list: one tier may bind multiple zones."""
    grouped: dict[str, list[str]] = {}
    for zone_id in sorted(zone_ids, key=int):
        prefix = zone_id[:2]
        grouped.setdefault(prefix, []).append(zone_id)

    tiers: list[dict[str, Any]] = []
    color_index = 0

    for prefix, chunk_count, tier_names in PREFIX_GROUPS:
        zones = grouped.get(prefix, [])
        if not zones:
            continue
        zone_chunks = chunk_list(zones, chunk_count)
        for chunk_index, zone_chunk in enumerate(zone_chunks):
            if not zone_chunk:
                continue
            tier_name = tier_names[chunk_index] if chunk_index < len(tier_names) else tier_names[-1]
            tiers.append(
                {
                    "color": TIER_COLORS[color_index % len(TIER_COLORS)],
                    "name": tier_name,
                    "code": str(len(tiers) + 1),
                    "zoneIds": zone_chunk,
                }
            )
            color_index += 1

    return tiers


def zone_to_pricecode_map(tiers: list[dict[str, Any]]) -> dict[str, str]:
    mapping: dict[str, str] = {}
    for tier in tiers:
        code = str(tier["code"])
        for zone_id in tier["zoneIds"]:
            mapping[str(zone_id)] = code
    return mapping


def load_json_array(path: Path) -> list[dict[str, Any]]:
    with path.open(encoding="utf-8") as file:
        data = json.load(file)
    if not isinstance(data, list):
        raise ValueError(f"{path} must be a top-level array")
    return data


def generate_seats_for_zone(
    zone_id: str,
    bounds: dict[str, float],
    seat_size: int,
    spacing: int,
    pricecode: str,
    global_seat_index: int,
    use_global_seat_id: bool,
) -> tuple[list[dict[str, Any]], int]:
    origin_x = bounds["x"]
    origin_y = bounds["y"]
    width = bounds["w"]
    height = bounds["h"]

    column_count, column_step, column_offset = compute_axis_layout(width, seat_size, spacing)
    row_count, row_step, row_offset = compute_axis_layout(height, seat_size, spacing)

    seats: list[dict[str, Any]] = []
    for row in range(row_count):
        for column in range(column_count):
            if use_global_seat_id:
                seat_id = str(global_seat_index)
                global_seat_index += 1
            else:
                seat_id = f"{zone_id}_seat_{row}_{column}"

            seats.append(
                {
                    "seatId": seat_id,
                    "pricecode": pricecode,
                    "y": int(round(origin_y + row_offset + row * row_step)),
                    "x": int(round(origin_x + column_offset + column * column_step)),
                }
            )

    return seats, global_seat_index


def generate_seat_data(
    zones: list[dict[str, Any]],
    seat_size: int,
    spacing: int,
    zone_pricecode: dict[str, str],
    default_pricecode: str,
    use_global_seat_id: bool,
) -> dict[str, list[dict[str, Any]]]:
    result: dict[str, list[dict[str, Any]]] = {}
    global_seat_index = 0

    for entry in zones:
        zone_id = str(entry["zoneId"])
        bounds = entry["bounds"]
        for key in ("x", "y", "w", "h"):
            if key not in bounds:
                raise ValueError(f"zone {zone_id} bounds missing '{key}'")

        pricecode = zone_pricecode.get(zone_id, default_pricecode)
        seats, global_seat_index = generate_seats_for_zone(
            zone_id=zone_id,
            bounds=bounds,
            seat_size=seat_size,
            spacing=spacing,
            pricecode=pricecode,
            global_seat_index=global_seat_index,
            use_global_seat_id=use_global_seat_id,
        )
        if seats:
            result[zone_id] = seats

    return result


def write_json(path: Path, data: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    text = json.dumps(data, indent=4, ensure_ascii=False) + "\n"
    path.write_text(text, encoding="utf-8")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Generate seat JSON and/or price-tier JSON from zone bounds."
    )
    parser.add_argument(
        "bounds_json",
        type=Path,
        help="Path to bounds JSON (array of {zoneId, bounds}).",
    )
    parser.add_argument(
        "-o",
        "--output",
        type=Path,
        default=None,
        help="Seat JSON output path. Prints to stdout if omitted.",
    )
    parser.add_argument(
        "--pricecode-output",
        type=Path,
        default=None,
        help="Write auto-generated price-tier JSON to this path.",
    )
    parser.add_argument(
        "--pricecode-json",
        type=Path,
        default=None,
        help="Existing price-tier JSON; maps zoneIds to seat pricecode.",
    )
    parser.add_argument(
        "--seat-size",
        type=int,
        default=DEFAULT_SEAT_SIZE,
        help=f"Seat size in pixels (default: {DEFAULT_SEAT_SIZE}).",
    )
    parser.add_argument(
        "--spacing",
        type=int,
        default=DEFAULT_SPACING,
        help=f"Gap between seats in pixels (default: {DEFAULT_SPACING}).",
    )
    parser.add_argument(
        "--default-pricecode",
        default="0",
        help="Fallback pricecode when a zone is not listed in price-tier JSON.",
    )
    parser.add_argument(
        "--global-seat-id",
        action="store_true",
        help="Use globally incrementing numeric seatId instead of zoneId_seat_row_col.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    bounds_path: Path = args.bounds_json

    if not bounds_path.is_file():
        print(f"error: file not found: {bounds_path}", file=sys.stderr)
        return 1

    if args.seat_size <= 0 or args.spacing < 0:
        print("error: seat-size must be > 0 and spacing must be >= 0", file=sys.stderr)
        return 1

    if args.output is None and args.pricecode_output is None:
        print(
            "error: specify --output and/or --pricecode-output",
            file=sys.stderr,
        )
        return 1

    try:
        zones = load_json_array(bounds_path)
        zone_ids = [str(entry["zoneId"]) for entry in zones]

        if args.pricecode_json is not None:
            tiers = load_json_array(args.pricecode_json)
        else:
            tiers = build_pricecode_tiers(zone_ids)

        zone_pricecode = zone_to_pricecode_map(tiers)

        if args.pricecode_output is not None:
            write_json(args.pricecode_output, tiers)
            print(
                f"wrote {len(tiers)} price tiers, {len(zone_ids)} zones -> {args.pricecode_output}",
                file=sys.stderr,
            )

        seat_data = generate_seat_data(
            zones=zones,
            seat_size=args.seat_size,
            spacing=args.spacing,
            zone_pricecode=zone_pricecode,
            default_pricecode=args.default_pricecode,
            use_global_seat_id=args.global_seat_id,
        )

        if args.output is not None:
            write_json(args.output, seat_data)
            total_seats = sum(len(seats) for seats in seat_data.values())
            print(
                f"wrote {total_seats} seats in {len(seat_data)} zones -> {args.output}",
                file=sys.stderr,
            )
        elif args.pricecode_output is not None:
            pass
        else:
            sys.stdout.write(json.dumps(seat_data, indent=4, ensure_ascii=False) + "\n")

    except (json.JSONDecodeError, ValueError, KeyError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
