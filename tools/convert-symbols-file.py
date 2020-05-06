#!/usr/bin/env python3

import re
import sys
import argparse


class BsnesPlus:
    # Spec: https://github.com/BenjaminSchulte/fma-snes65816/blob/master/docs/symbols.adoc

    def __init__(self):
        self.symbols = list()


    def add_symbol(self, addr, name):
        self.symbols.append(f"{addr >> 16:02x}:{addr & 0xffff:04x} {name} ANY 1\n")


    def write(self, fp):
        fp.write("#SNES65816\n")

        fp.write("\n[SYMBOL]\n")
        fp.writelines(self.symbols)



class MesenS:
    # Spec: https://www.mesen.ca/docs/debugging/debuggerintegration.html#mesen-label-files-mlb

    def __init__(self, is_hirom):
        self.is_hirom = is_hirom
        self.lines = list()


    def mapper(self, addr):
        bank = addr >> 16

        if bank == 0x7e or bank == 0x7f:
            return 'WORK', addr - 0x7e0000

        if bank & 0x7f <= 0x3f:
            if addr & 0xffff < 0x8000:
                a = addr & 0xfffff
                if a < 0x2000:
                    return 'WORK', a
                else:
                    return 'REG', a

        if self.is_hirom:
            return 'PRG', addr & 0x3fffff
        else:
            # lorom
            return 'PRG', (addr & 0x3f0000) >> 1 | (addr & 0x7fff)


    def add_symbol(self, addr, name):
        region, address = self.mapper(addr)

        name = name.replace('.', '_')

        self.lines.append(f"{region}:{address:04x}:{name}\n")


    def write(self, fp):
        fp.writelines(self.lines)



def parse_arguments():
    parser = argparse.ArgumentParser(
                description='bass-untech symbol file converter')


    mapping_group = parser.add_mutually_exclusive_group(required=True)

    mapping_group.add_argument('--hirom',
                               action='store_true',
                               help='HiROM mpping')
    mapping_group.add_argument('--lorom',
                               action='store_true',
                               help='LoROM mpping')


    format_group = parser.add_mutually_exclusive_group(required=True)

    format_group.add_argument('-b', '--bsnes-plus',
                              action='store_true',
                              help='output bsnes plus symbol file')
    format_group.add_argument('-m', '--mesen-s',
                              action='store_true',
                              help='output mesen-s label file')


    parser.add_argument('-o', '--output',
                        required=True,
                        help='output file')

    parser.add_argument('input_file',
                        help='input symbol file')

    return parser.parse_args()



def main():
    args = parse_arguments()

    if args.bsnes_plus:
        exporter = BsnesPlus()
    elif args.mesen_s:
        exporter = MesenS(args.hirom)
    else:
        raise RuntimeError("Unknown output format")


    regex = re.compile('^([a-f0-9]+) (.+)$')

    with open(args.input_file, 'r') as f:
        for line in f:
            if line:
                m = regex.match(line)
                if not m:
                    raise RuntimeError(f"Unknown line format: {line}")

                exporter.add_symbol(int(m.group(1), 16), m.group(2))


    with open(args.output, 'w') as f:
        exporter.write(f)



if __name__ == '__main__':
    main()

