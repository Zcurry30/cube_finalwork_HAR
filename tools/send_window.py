#!/usr/bin/env python3
"""Send a complete 128-sample UCI HAR window to STM32 via serial."""

import serial, time, sys, numpy as np

ACTIVITIES = {
    "walking":      1,
    "walking_up":   2,
    "walking_down": 3,
    "sitting":      4,
    "standing":     5,
    "laying":       6,
}

def main():
    activity = sys.argv[1] if len(sys.argv) > 1 else "walking"
    code = ACTIVITIES.get(activity, 1)

    base = 'E:/Desktop/HAR/uci_har/UCI HAR Dataset/test'
    y_test = np.loadtxt(f'{base}/y_test.txt', dtype=int)
    sig = f'{base}/Inertial Signals'
    axes = ['total_acc_x','total_acc_y','total_acc_z','body_gyro_x','body_gyro_y','body_gyro_z']

    indices = np.where(y_test == code)[0]
    idx = indices[len(indices)//2]  # middle window
    window = np.column_stack([np.loadtxt(f'{sig}/{ax}_test.txt')[idx] for ax in axes])

    ser = serial.Serial('COM8', 115200, timeout=2, dsrdtr=False)
    ser.dtr = False
    ser.rts = False
    time.sleep(1.0)  # let board settle
    ser.read(ser.in_waiting)  # flush any boot noise

    print(f"Sending {activity} window #{idx} (128 samples @ 10ms)...", flush=True)
    for row in window:
        line = ','.join(f'{v:.6f}' for v in row)
        ser.write((line + '\r\n').encode())
        ser.flush()
        time.sleep(0.01)  # 10ms = 100Hz

    print("Waiting for inference...", flush=True)
    # Wait long enough for 128-sample accumulation + CNN inference + output
    for _ in range(40):  # up to 12 seconds
        time.sleep(0.3)
        while ser.in_waiting:
            try:
                resp = ser.readline().decode('ascii', errors='replace').strip()
                if resp and 'HAR' in resp:
                    print(f"\n>>> {resp}", flush=True)
            except:
                pass

    ser.close()
    print("Done.", flush=True)

if __name__ == "__main__":
    main()
