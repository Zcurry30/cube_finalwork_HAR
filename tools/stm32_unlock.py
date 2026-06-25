#!/usr/bin/env python3
"""STM32 RDP Unlock via UART Bootloader — mass erase to clear read protection."""

import sys
import time
import serial

def main():
    port = sys.argv[1] if len(sys.argv) > 1 else "COM8"

    print(f"Opening {port} 115200 8E1 ...", flush=True)

    ser = serial.Serial(
        port=port,
        baudrate=115200,
        bytesize=serial.EIGHTBITS,
        parity=serial.PARITY_EVEN,
        stopbits=serial.STOPBITS_ONE,
        timeout=0.5,
        dsrdtr=False,
        rtscts=False,
    )
    # Don't let RTS/DTR toggle disturb the bootloader
    ser.dtr = False
    ser.rts = False
    time.sleep(0.5)

    print(f"OK. Syncing with bootloader...", flush=True)

    # Step 1: auto-baud sync
    synced = False
    for i in range(1, 300):
        ser.write(b'\x7F')
        ser.flush()
        b = ser.read(1)
        if b and b[0] == 0x79:
            print(f"  Synced on attempt {i}! (ACK=0x79)", flush=True)
            synced = True
            break
        time.sleep(0.01)

    if not synced:
        print("FAILED: bootloader not responding.", flush=True)
        print("  Power-cycle the board and try again.", flush=True)
        ser.close()
        sys.exit(1)

    # Step 2: send Extended Erase
    def cmd(byte):
        ser.write(bytes([byte])); ser.flush()
        ack = ser.read(1)
        if not ack or ack[0] != 0x79:
            print(f"  NACK/timeout on 0x{byte:02X}", flush=True)
            return False
        return True

    print("Sending Extended Erase (0x44)...", flush=True)
    if not cmd(0x44):
        ser.close(); sys.exit(1)
    if not cmd(0xBB):   # complement
        ser.close(); sys.exit(1)

    # Step 3: mass erase code 0xFFFF, checksum 0x00
    print("MASS ERASE (0xFFFF)...", flush=True)
    ser.write(b'\xFF\xFF\x00')
    ser.flush()

    ser.timeout = 30.0   # mass erase can take a while
    ack = ser.read(1)

    if ack and ack[0] == 0x79:
        print("\n===== SUCCESS! RDP cleared, flash empty. =====", flush=True)
    elif ack:
        print(f"\nUnexpected response: 0x{ack[0]:02X}", flush=True)
    else:
        print("\nTimeout — but erase may have completed anyway.", flush=True)
        print("Try connecting with CubeProgrammer now.", flush=True)

    ser.close()

if __name__ == "__main__":
    main()
