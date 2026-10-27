from pathlib import Path
import json,sys,time,threading,argparse
parser=argparse.ArgumentParser();parser.add_argument('--library');args=parser.parse_args();stop=threading.Event()
def commands():
 for line in sys.stdin:
  if line.strip()=='stop':break
 stop.set()
threading.Thread(target=commands,daemon=True).start()
while not stop.is_set():
 value=json.loads((Path(args.library)/'fixture.json').read_text())
 print(json.dumps({'schema':1,'telemetry':value}),flush=True);stop.wait(.1)
