#!/usr/bin/env python3
"""STM32 Flash via UART Bootloader — write .hex or .bin file."""

import sys, time, serial

BOOT_ACK = 0x79
BOOT_NACK = 0x1F

def sync(ser):
    for _ in range(10):
        ser.write(b'\x7F'); ser.flush()
        b = ser.read(1)
        if b and b[0] == BOOT_ACK: return True
    return False

def cmd(ser, byte):
    """Send command+complement together, wait for ACK."""
    ser.write(bytes([byte, 0xFF ^ byte])); ser.flush()
    time.sleep(0.01)
    b = ser.read(1)
    return b and b[0] == BOOT_ACK

def write_memory(ser, address, data):
    """Write up to 256 bytes to memory at address."""
    # Step 1: send 4-byte address (MSB first) + checksum
    addr_bytes = bytes([(address >> 24) & 0xFF, (address >> 16) & 0xFF,
                        (address >> 8) & 0xFF, address & 0xFF])
    addr_csum = addr_bytes[0] ^ addr_bytes[1] ^ addr_bytes[2] ^ addr_bytes[3]
    ser.write(addr_bytes + bytes([addr_csum & 0xFF])); ser.flush()
    time.sleep(0.02)
    b = ser.read(1)
    if not b or b[0] != BOOT_ACK:
        print(f"\n  Address 0x{address:08X} NACK'd", flush=True)
        return False

    # Step 2: send N-1 + data + checksum
    n = len(data) - 1  # bootloader uses N-1
    csum = n
    for d in data: csum ^= d
    payload = bytes([n]) + data + bytes([csum & 0xFF])

    ser.write(payload); ser.flush()
    time.sleep(0.05)
    b = ser.read(1)
    return b and b[0] == BOOT_ACK

def flash_hex(ser, hex_path):
    """Parse Intel HEX and write to flash."""
    print(f"Reading {hex_path} ...", flush=True)
    segments = {}

    ext_base = 0  # extended linear address
    with open(hex_path, 'r') as f:
        for line in f:
            line = line.strip()
            if not line.startswith(':'): continue
            byte_count = int(line[1:3], 16)
            address = int(line[3:7], 16)
            record_type = int(line[7:9], 16)

            if record_type == 0x00:  # Data
                full_addr = ext_base + address
                base = full_addr & 0xFFFF0000
                offset = full_addr & 0x0000FFFF
                seg = segments.setdefault(base, bytearray(0x10000))
                data = bytes.fromhex(line[9:9+byte_count*2])
                seg[offset:offset+len(data)] = data
            elif record_type == 0x04:  # Extended Linear Address
                ext_base = int(line[9:13], 16) << 16
            elif record_type == 0x01:  # EOF
                break

    total = sum(len(s) for s in segments.values())
    print(f"Total: {total} bytes in {len(segments)} segment(s)", flush=True)
    for base in sorted(segments):
        print(f"  0x{base:08X}: {len(segments[base])} bytes", flush=True)

    written = 0
    for base, seg in sorted(segments.items()):
        addr = base
        pos = 0
        while pos < len(seg):
            chunk = bytes(seg[pos:pos+256])
            if len(chunk) == 0: break

            ok = False
            for retry in range(5):
                if cmd(ser, 0x31) and write_memory(ser, addr, chunk):
                    ok = True
                    break
                time.sleep(0.05)

            if not ok:
                print(f"\n  SKIP at 0x{addr:08X} after 5 retries", flush=True)
                # skip this chunk, continue with next
                addr += len(chunk)
                pos += len(chunk)
                continue

            addr += len(chunk)
            pos += len(chunk)
            written += len(chunk)

            pct = written * 100 // total
            print(f"\r  {pct}% ({written}/{total})", end="", flush=True)

    print(f"\nFlash complete: {written} bytes", flush=True)
    return True

def main():
    port = sys.argv[1] if len(sys.argv) > 1 else "COM8"
    hex_file = sys.argv[2] if len(sys.argv) > 2 else "E:/Desktop/HAR/har_project/Debug/finalwork.hex"
    skip_erase = "--no-erase" in sys.argv

    print(f"Opening {port} ...", flush=True)
    ser = serial.Serial(port, 115200, serial.EIGHTBITS, serial.PARITY_EVEN, serial.STOPBITS_ONE, timeout=5)
    ser.dtr = False; ser.rts = False
    time.sleep(0.3)

    print("Syncing with bootloader...", flush=True)
    if not sync(ser):
        print("FAILED: no response. Power-cycle board.", flush=True)
        ser.close(); sys.exit(1)
    print("Synced!", flush=True)

    # Global Erase first (unless skipped)
    if not skip_erase:
        print("Erasing chip...", flush=True)
        if cmd(ser, 0x44):
            ser.timeout = 30
            ser.write(b'\xFF\xFF\x00'); ser.flush()
            b = ser.read(1)
            if b and b[0] == BOOT_ACK:
                print("Chip erased.", flush=True)
            else:
                print("Erase timeout (may be OK).", flush=True)
            ser.timeout = 5
        else:
            print("WARNING: Erase rejected — trying without erase.", flush=True)
    else:
        print("Skipping erase (--no-erase).", flush=True)

    # Flash
    if not flash_hex(ser, hex_file):
        ser.close(); sys.exit(1)

    print("\nDONE! Power off, set BOOT0 to GND, power on.", flush=True)
    ser.close()

if __name__ == "__main__":
    main()
