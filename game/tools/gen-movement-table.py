#!/usr/bin/env python3

import argparse

from io import StringIO
from typing import Callable, Final, NamedTuple, TextIO


def _parse_u16_hex(s: str) -> int:
    assert s.startswith("$")

    i = int(s[1:], 16)
    if i < 0 or i > 0xFFFF:
        raise ValueError(f"Number is out of u16 range: {s}")
    return i


def _parse_u16_fp_float(s: str, scale: int) -> int:
    i = int(round(float(s) * scale))

    if i < 0 or i > 0xFFFF:
        m = 0xFFFF / scale
        raise ValueError(f"Number is out of range: {s} (min: 0.0, max: {m})")
    return i


class TableValue(NamedTuple):
    value: int


def parse_u8_8_fp(s: str) -> TableValue:
    if s.startswith("$"):
        return TableValue(_parse_u16_hex(s))
    else:
        return TableValue(_parse_u16_fp_float(s, 256))


def parse_u0_16_fp(s: str) -> TableValue:
    if s.startswith("$"):
        return TableValue(_parse_u16_hex(s))
    else:
        return TableValue(_parse_u16_fp_float(s, 256 * 256))


class TableColumn(NamedTuple):
    name: str
    format_comment: str
    parser: Callable[[str], TableValue]


TABLE_COLUMNS: Final = (
    TableColumn(
        "MoveAccelerationTable",
        "0:16 fixed point px/frame/frame",
        parse_u0_16_fp,
    ),
    TableColumn(
        "TurnDecelerationTable",
        "0:16 fixed point px/frame/frame",
        parse_u0_16_fp,
    ),
    TableColumn(
        "BreakDecelerationTable",
        "0:16 fixed point px/frame/frame",
        parse_u0_16_fp,
    ),
    TableColumn(
        "MaxMomentum",
        "8:8 fixed point px/frame",
        parse_u8_8_fp,
    ),
    TableColumn(
        "JumpVelocity",
        "8:8 fixed point px/frame",
        parse_u8_8_fp,
    ),
)


class TableRow(NamedTuple):
    name: str
    values: list[TableValue]


def parse_csv_file(fp: TextIO) -> list[TableRow]:
    out: list[TableRow] = list()

    N_CSV_COLUMNS = len(TABLE_COLUMNS) + 1

    for line_no, line in enumerate(fp, 1):
        line = line.strip()

        if not line or line.startswith("#"):
            continue

        data = [x.strip() for x in line.split(",")]

        if len(data) != N_CSV_COLUMNS:
            raise ValueError(f"line {line_no} requires {N_CSV_COLUMNS} values")

        it = iter(data)

        out.append(TableRow(next(it), [c.parser(next(it)) for c in TABLE_COLUMNS]))

        assert next(it, None) is None, "iterator not empty"

    return out


def generate_inc_file(table: list[TableRow]) -> str:
    with StringIO() as out:
        out.write(
            """// Momentum SoA tables

namespace MovementTableIndex {
"""
        )
        for i, row in enumerate(table):
            out.write(f"  constant {row.name} = {i * 2}\n")

        out.write(
            """
}

rodata()
namespace MovementTable {
"""
        )

        for ci, c in enumerate(TABLE_COLUMNS):
            out.write(f"// {c.format_comment}\n")
            out.write(f"{c.name}:\n  dw")
            for row in table:
                out.write(f" ${row.values[ci].value:04x},")  # type: ignore[union-attr]
            out.write("\n\n")

        out.write(
            f"assert(pc() - MoveAccelerationTable == {2 * len(table) * len(TABLE_COLUMNS)})\n"
        )
        out.write("}\n\n")

        return out.getvalue()


def parse_arguments():
    parser = argparse.ArgumentParser(description="movement table generator")

    parser.add_argument("-o", "--output", required=True, help="output file")

    parser.add_argument("input_file", help="input symbol file")

    return parser.parse_args()


def main():
    args = parse_arguments()

    with open(args.input_file, "r") as f:
        table = parse_csv_file(f)

    inc_file = generate_inc_file(table)

    with open(args.output, "w") as f:
        f.write(inc_file)


if __name__ == "__main__":
    main()
