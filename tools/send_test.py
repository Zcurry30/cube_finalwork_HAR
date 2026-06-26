#!/usr/bin/env python3
"""Send HAR test data over serial to COM8."""
import sys, time, serial, math, random

def gen_walking(n=256):
    lines = []
    for i in range(n):
        p = (i % 20) / 20.0
        ax=round(0.15*math.sin(p*6.28)+random.uniform(-.05,.05), 3)
        ay=round(0.08*math.cos(p*6.28)+random.uniform(-.04,.04), 3)
        az=round(-1.01+max(0,math.sin(p*6.28))*.3+random.uniform(-.03,.03), 3)
        gx=round(0.5*math.sin(p*6.28)+random.uniform(-.1,.1), 3)
        gy=round(0.3*math.cos(p*6.28)+random.uniform(-.1,.1), 3)
        gz=round(0.2*math.sin(p*9.42)+random.uniform(-.1,.1), 3)
        lines.append(f"{ax:.3f},{ay:.3f},{az:.3f},{gx:.3f},{gy:.3f},{gz:.3f}")
    return lines

def gen_standing(n=256):
    lines = []
    for i in range(n):
        ax=round(random.uniform(-.03,.03),3)
        ay=round(random.uniform(-.03,.03),3)
        az=round(-1.01+random.uniform(-.02,.02),3)
        gx=round(random.uniform(-.02,.02),3)
        gy=round(random.uniform(-.02,.02),3)
        gz=round(random.uniform(-.02,.02),3)
        lines.append(f"{ax:.3f},{ay:.3f},{az:.3f},{gx:.3f},{gy:.3f},{gz:.3f}")
    return lines

ACTIVITIES = {"walking": gen_walking, "standing": gen_standing}

def main():
    activity = sys.argv[1] if len(sys.argv) > 1 else "walking"
    interval_ms = int(sys.argv[2]) if len(sys.argv) > 2 else 10
    gen = ACTIVITIES.get(activity, gen_walking)
    lines = gen()

    print(f"Opening COM8 ...", flush=True)
    ser = serial.Serial('COM8', 115200, timeout=1)
    time.sleep(0.3)
    ser.read(ser.in_waiting)  # flush stale

    print(f"Sending {activity} ({len(lines)} lines, {interval_ms}ms) ...", flush=True)
    print(f"Expected: {activity}", flush=True)
    print("-" * 40, flush=True)

    results = []
    for idx, line in enumerate(lines):
        ser.write((line + "\r\n").encode())
        ser.flush()
        time.sleep(interval_ms / 1000.0)

        # Check for responses
        while ser.in_waiting:
            try:
                resp = ser.readline().decode('ascii', errors='replace').strip()
                if resp:
                    if 'HAR' in resp:
                        results.append(resp)
                        print(f"  >>> {resp}", flush=True)
                    elif 'ERR' in resp:
                        print(f"  ERR: {resp}", flush=True)
            except:
                pass

        # Progress every 32 lines
        if (idx + 1) % 32 == 0:
            print(f"  [{idx+1}/{len(lines)}] sent", flush=True)

    # Wait for remaining responses
    print("Waiting for final responses ...", flush=True)
    time.sleep(2)
    while ser.in_waiting:
        try:
            resp = ser.readline().decode('ascii', errors='replace').strip()
            if resp:
                if 'HAR' in resp:
                    results.append(resp)
                    print(f"  >>> {resp}", flush=True)
                elif 'ERR' in resp:
                    print(f"  ERR: {resp}", flush=True)
                else:
                    print(f"  RX: {resp}", flush=True)
        except:
            pass

    ser.close()
    print("-" * 40, flush=True)
    if results:
        print(f"RESULTS: {len(results)} inferences", flush=True)
        for r in results:
            print(f"  {r}", flush=True)
    else:
        print("NO HAR RESULTS. Board may not be running user program.", flush=True)
        print("Check: BOOT0=GND, power on, RESET, XCOM shows HAR ready.", flush=True)

if __name__ == "__main__":
    main()
