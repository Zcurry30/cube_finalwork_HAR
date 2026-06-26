#!/usr/bin/env python3
"""Generate test IMU data for HAR model testing via serial."""
import math
import random

def generate_standing(n=128):
    """Standing still — very stable, gravity only on Z, tiny noise."""
    lines = []
    for i in range(n):
        ax = 0.02 + random.uniform(-0.03, 0.03)
        ay = 0.01 + random.uniform(-0.03, 0.03)
        az = -1.01 + random.uniform(-0.02, 0.02)  # gravity
        gx = random.uniform(-0.02, 0.02)
        gy = random.uniform(-0.02, 0.02)
        gz = random.uniform(-0.02, 0.02)
        lines.append(f"{ax:.3f},{ay:.3f},{az:.3f},{gx:.3f},{gy:.3f},{gz:.3f}")
    return lines

def generate_walking(n=128):
    """Walking — rhythmic accel oscillation, moderate gyro."""
    lines = []
    step_period = 20  # samples per step
    for i in range(n):
        phase = (i % step_period) / step_period
        # Simulate heel strike pattern
        impact = max(0, math.sin(phase * 2 * math.pi)) * 0.4
        ax = 0.15 * math.sin(phase * 2 * math.pi) + random.uniform(-0.05, 0.05)
        ay = 0.08 * math.cos(phase * 2 * math.pi) + random.uniform(-0.04, 0.04)
        az = -1.01 + impact * 0.3 + random.uniform(-0.03, 0.03)
        gx = 0.5 * math.sin(phase * 2 * math.pi) + random.uniform(-0.1, 0.1)
        gy = 0.3 * math.cos(phase * 2 * math.pi) + random.uniform(-0.1, 0.1)
        gz = 0.2 * math.sin(phase * 3 * math.pi) + random.uniform(-0.1, 0.1)
        lines.append(f"{ax:.3f},{ay:.3f},{az:.3f},{gx:.3f},{gy:.3f},{gz:.3f}")
    return lines

def generate_running(n=128):
    """Running — larger amplitudes, faster rhythm."""
    lines = []
    step_period = 12
    for i in range(n):
        phase = (i % step_period) / step_period
        impact = max(0, math.sin(phase * 2 * math.pi)) * 1.2
        ax = 0.5 * math.sin(phase * 2 * math.pi) + random.uniform(-0.1, 0.1)
        ay = 0.2 * math.cos(phase * 2 * math.pi) + random.uniform(-0.08, 0.08)
        az = -1.01 + impact * 0.5 + random.uniform(-0.05, 0.05)
        gx = 1.5 * math.sin(phase * 2 * math.pi) + random.uniform(-0.3, 0.3)
        gy = 0.8 * math.cos(phase * 2 * math.pi) + random.uniform(-0.2, 0.2)
        gz = 0.5 * math.sin(phase * 2 * math.pi) + random.uniform(-0.2, 0.2)
        lines.append(f"{ax:.3f},{ay:.3f},{az:.3f},{gx:.3f},{gy:.3f},{gz:.3f}")
    return lines

def generate_upstairs(n=128):
    """Upstairs — upward movement, positive az delta, gyro_z positive."""
    lines = []
    step_period = 25
    for i in range(n):
        phase = (i % step_period) / step_period
        lift = max(0, math.sin(phase * math.pi)) * 0.5
        ax = 0.2 * math.sin(phase * 2 * math.pi) + random.uniform(-0.05, 0.05)
        ay = 0.1 * math.cos(phase * 2 * math.pi) + random.uniform(-0.05, 0.05)
        az = -0.9 + lift + random.uniform(-0.03, 0.03)
        gx = 0.3 * math.sin(phase * 2 * math.pi) + random.uniform(-0.1, 0.1)
        gy = 0.2 * math.cos(phase * 2 * math.pi) + random.uniform(-0.1, 0.1)
        gz = 0.6 * math.sin(phase * math.pi) + random.uniform(-0.15, 0.15)
        lines.append(f"{ax:.3f},{ay:.3f},{az:.3f},{gx:.3f},{gy:.3f},{gz:.3f}")
    return lines

def generate_downstairs(n=128):
    """Downstairs — downward movement, negative az delta, gyro_z negative."""
    lines = []
    step_period = 25
    for i in range(n):
        phase = (i % step_period) / step_period
        drop = max(0, math.sin(phase * math.pi)) * 0.4
        ax = 0.2 * math.sin(phase * 2 * math.pi) + random.uniform(-0.05, 0.05)
        ay = 0.1 * math.cos(phase * 2 * math.pi) + random.uniform(-0.05, 0.05)
        az = -1.1 - drop + random.uniform(-0.03, 0.03)
        gx = 0.3 * math.sin(phase * 2 * math.pi) + random.uniform(-0.1, 0.1)
        gy = 0.2 * math.cos(phase * 2 * math.pi) + random.uniform(-0.1, 0.1)
        gz = -0.5 * math.sin(phase * math.pi) + random.uniform(-0.15, 0.15)
        lines.append(f"{ax:.3f},{ay:.3f},{az:.3f},{gx:.3f},{gy:.3f},{gz:.3f}")
    return lines

# Print test data
import sys
activity = sys.argv[1] if len(sys.argv) > 1 else "walking"

generators = {
    "standing": generate_standing,
    "walking": generate_walking,
    "running": generate_running,
    "upstairs": generate_upstairs,
    "downstairs": generate_downstairs,
}

gen = generators.get(activity, generate_walking)
lines = gen(256)  # 256 samples to cover 2 windows + stride

# Print header
print(f"# {activity.upper()} test data (256 samples, 10ms interval)")
print(f"# Copy these lines to XCOM with timed send at 10ms")
print()
for line in lines:
    print(line)
