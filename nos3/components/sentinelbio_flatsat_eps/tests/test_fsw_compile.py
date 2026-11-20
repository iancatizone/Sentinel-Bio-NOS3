"""Syntax-check the C sources against real, configured NOS3 headers (no link/run)."""
from pathlib import Path
import argparse, tempfile, subprocess, shlex
p=argparse.ArgumentParser();p.add_argument('nos3_root',type=Path);a=p.parse_args()
nos=a.nos3_root.resolve(); pkg=Path(__file__).resolve().parents[1]
flags=nos/'fsw/build/amd64-nos3/default_cpu1/apps/sentinelbio_flatsat_eps/fsw/cfs/CMakeFiles/sentinelbio_eps.dir/flags.make'
if not flags.is_file(): raise SystemExit('Need a configured NOS3 cFS build with EPS compile flags')
s=flags.read_text(); incs=next(x.split(' = ',1)[1] for x in s.splitlines() if x.startswith('C_INCLUDES ='))
old='/home/jstar/Desktop/github-nos3'
incs=incs.replace(old+'/components/sentinelbio_flatsat_eps',str(pkg)).replace(old,str(nos))
with tempfile.TemporaryDirectory() as tmp:
 t=Path(tmp)
 # EPS uses no CAN functions; suppress only this unrelated external header.
 (t/"libsocketcan.h").write_text("/* Empty CAN dependency shim; EPS does not use CAN. */\n")
 for base in [nos/'fsw/build/inc',nos/'fsw/build/amd64-nos3/default_cpu1/inc',nos/'fsw/build/amd64-nos3/default_cpu1/osal/inc']:
  for file in base.glob('*.h'):
   (t/file.name).write_text(file.read_text().replace(old,str(nos)))
 for src in [pkg/'fsw/cfs/src/sentinelbio_flatsat_eps_app.c',pkg/'fsw/shared/sentinelbio_flatsat_eps_device.c']:
  subprocess.run(['gcc','-fsyntax-only','-std=c99','-Wall','-Wextra','-Werror','-Wformat=2','-Wno-unused-parameter','-D_DEFAULT_SOURCE','-DBYTE_ORDER_LE','-D_LINUX_OS_','-D_NOS_OS_','-I'+str(t),*shlex.split(incs),str(src)],check=True)
print('PASS: both FSW sources compile against actual NOS3 headers, warnings as errors')
