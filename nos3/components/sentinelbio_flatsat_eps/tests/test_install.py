"""Exercise installer on a supplied disposable baseline (never your working checkout)."""
import argparse, importlib.util, tempfile, shutil
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('baseline',type=Path);a=p.parse_args()
pkg=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('installer',pkg/'tools/install_into_nos3.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
with tempfile.TemporaryDirectory() as d:
 root=Path(d)/'nos3';root.mkdir()
 try:m.install(root,pkg)
 except m.InstallError:pass
 else:raise AssertionError('Incomplete checkout accepted')
 assert not list(root.iterdir()),'Preflight failure mutated checkout'
 # Copy just source/config files needed for integration, avoiding build outputs/backups.
 for folder in ('cfg','scripts','components','sims','gsw/cosmos/config'):
  src=a.baseline/folder
  if src.exists():shutil.copytree(src,root/folder,ignore=shutil.ignore_patterns('build','sentinelbio_flatsat_eps','__pycache__'),dirs_exist_ok=True)
 shutil.copy2(a.baseline/'Makefile',root/'Makefile')
 sentinel=root/'components/sentinelbio_flatsat_eps/user_notes.txt';sentinel.parent.mkdir(exist_ok=True);sentinel.write_text('keep')
 planned=m.install(root,pkg,dry_run=True);assert planned;assert sentinel.read_text()=='keep'
 m.install(root,pkg);assert not m.install(root,pkg),'Not idempotent';assert sentinel.read_text()=='keep'
 assert list((root/'sentinelbio_eps_backups').glob('*/manifest.json'))
 assert 'EPS BATTERY_VOLTAGE' not in (root/'cfg/nos3_defs/tables/lc_def_wdt.c').read_text()
 print('PASS: preflight no mutation, dry-run, install, backups, idempotence, unrelated file preservation')
