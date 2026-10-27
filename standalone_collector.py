"""SPDX-License-Identifier: GPL-2.0-or-later
Read-only, dependency-free Linux collector bundled in Hardware Sync AI.
No RGB access, hardware identifiers, account, model names or remote service required.
"""
import csv
import json
import math
import subprocess
import sys
import threading
import time
import urllib.request
from pathlib import Path


def finite(value, low=0, high=1e12):
    try:
        value = float(value)
    except (ValueError, TypeError):
        return None
    return value if math.isfinite(value) and low <= value <= high else None


def models_state(models):
    flags = {'gpu_model': False, 'cpu_model': False, 'ram_model': False}
    for row in models:
        size, vram = finite(row.get('size')), finite(row.get('size_vram'))
        if size is None or vram is None or size <= 0:
            raise ValueError('Unknown model allocation')
        host = max(0, size - vram)
        flags['gpu_model'] |= vram > 0
        flags['cpu_model'] |= vram == 0 or host > max(64 * 1024**2, size * .02)
        flags['ram_model'] |= host > 0
    return dict(flags, model_count=len(models))


def cpu_snapshot():
    values = list(map(int, Path('/proc/stat').read_text().splitlines()[0].split()[1:9]))
    return sum(values), values[3] + values[4]


def cpu_usage(previous, current):
    total, idle = current[0] - previous[0], current[1] - previous[1]
    return max(0, min(100, 100 * (total - idle) / total)) if total > 0 else None


def ram_usage():
    memory = {line.split(':')[0]: int(line.split()[1])
              for line in Path('/proc/meminfo').read_text().splitlines()}
    return 100 * (1 - memory['MemAvailable'] / memory['MemTotal'])


def temperatures():
    values = []
    for hwmon in Path('/sys/class/hwmon').glob('hwmon*'):
        try:
            name = (hwmon / 'name').read_text().strip()
            if name not in ('coretemp', 'k10temp', 'zenpower', 'cpu_thermal'):
                continue
            for sensor in hwmon.glob('temp*_input'):
                value = finite(float(sensor.read_text()) / 1000, 0, 130)
                if value is not None:
                    values.append(value)
        except (OSError, ValueError):
            continue
    return max(values) if values else None


def gpu_metrics():
    # A multi-GPU system needs an explicit selection rather than an arbitrary first GPU.
    fields = 'temperature.gpu,power.draw,power.limit,utilization.gpu,memory.used,memory.total,fan.speed'
    output = subprocess.run(['nvidia-smi', '--query-gpu=' + fields,
                             '--format=csv,noheader,nounits'],
                            capture_output=True, text=True, timeout=2, check=True)
    rows = list(csv.reader(output.stdout.splitlines()))
    if len(rows) != 1 or len(rows[0]) != 7:
        raise ValueError('GPU unavailable or ambiguous')
    names = ('gpu_temp_c', 'gpu_power_w', 'gpu_power_limit_w', 'gpu_usage_percent',
             'gpu_memory_mib', 'gpu_memory_total_mib', 'gpu_fan_percent')
    result = {name: finite(value.strip()) for name, value in zip(names, rows[0])}
    power, limit = result['gpu_power_w'], result['gpu_power_limit_w']
    result['gpu_power_percent'] = min(100, 100 * power / limit) if power is not None and limit else None
    return result


def ollama_metrics():
    # Loopback only; system proxy settings must not send this request off the host.
    opener = urllib.request.build_opener(urllib.request.ProxyHandler({}))
    with opener.open('http://127.0.0.1:11434/api/ps', timeout=1) as response:
        raw = response.read(1_000_001)
    if len(raw) > 1_000_000:
        raise ValueError('Model response too large')
    return models_state(json.loads(raw)['models'])


def main():
    stop = threading.Event()
    def commands():
        for line in sys.stdin:
            if line.strip() == 'stop':
                break
        stop.set()
    threading.Thread(target=commands, daemon=True).start()
    previous = None
    while not stop.is_set():
        result = {}
        try:
            current = cpu_snapshot()
            result['cpu_usage_percent'] = cpu_usage(previous, current) if previous else None
            result['cpu_demand_percent'] = result['cpu_usage_percent']
            previous = current
            result['ram_usage_percent'] = ram_usage()
            result['cpu_temp_c'] = temperatures()
        except (OSError, ValueError, KeyError):
            pass
        try:
            result.update(gpu_metrics())
        except (OSError, ValueError, subprocess.SubprocessError):
            pass
        try:
            result.update(ollama_metrics())
        except (OSError, ValueError, KeyError, TypeError):
            result.update(gpu_model=None, cpu_model=None, ram_model=None, model_count=None)
        cpu, gpu = result.get('cpu_temp_c'), result.get('gpu_temp_c')
        result['case_thermal_percent'] = min(100, max(0, (max(cpu, gpu) - 30) / .6)) if cpu is not None and gpu is not None else None
        print(json.dumps({'schema': 1, 'telemetry': result}, allow_nan=False), flush=True)
        stop.wait(1)


if __name__ == '__main__':
    main()
