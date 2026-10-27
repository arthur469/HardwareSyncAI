"""Read-only sensor bridge for Hardware Sync AI. Never imports the RGB SDK/engine."""
import argparse,json,sys,threading,time
from pathlib import Path

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--library',required=True);args=parser.parse_args()
    sys.path.insert(0,str(Path(args.library).resolve()))
    from rgb_telemetry.telemetry import Collector,metrics,finite
    from rgb_telemetry import settings
    config=settings.load();collector=Collector(config).start();stopping=threading.Event()
    def commands():
        for line in sys.stdin:
            if line.strip()=='stop':break
        stopping.set()
    threading.Thread(target=commands,daemon=True).start()
    try:
        while not stopping.is_set():
            value=metrics(collector.snapshot());e=config['effects']
            watts,limit=value.get('gpu_power_w'),value.get('gpu_power_limit_w')
            value['gpu_power_percent']=min(100,max(0,100*watts/limit)) if watts is not None and limit else None
            power=value.get('cpu_power_w');value['cpu_demand_percent']=min(100,100*power/e['cpu_power_reference_w']) if power is not None else value.get('cpu_usage_percent')
            cpu,gpu=value.get('cpu_temp_c'),value.get('gpu_temp_c')
            norm=lambda x,r:min(100,max(0,100*(x-r[0])/(r[1]-r[0])))
            value['case_thermal_percent']=max(norm(cpu,e['cpu_temp_range_c']),norm(gpu,e['gpu_temp_range_c'])) if cpu is not None and gpu is not None else None
            value['model_count']=len(value.get('models',[])) if value.get('gpu_model') is not None else None
            # Only actual collector freshness, never a timestamp invented for stale sensors.
            print(json.dumps({'schema':1,'telemetry':value},allow_nan=False),flush=True)
            stopping.wait(1)
    finally:collector.close()
if __name__=='__main__':main()
