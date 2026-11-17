# Template for table configuration

cfetables: staging/cpu1/cf/fm_monitor.tbl

staging/cpu1/cf/fm_monitor.tbl: CFE_TABLE_CPUNAME   := cpu1
staging/cpu1/cf/fm_monitor.tbl: CFE_TABLE_APPNAME   := fm
staging/cpu1/cf/fm_monitor.tbl: CFE_TABLE_BASENAME  := fm_monitor

# Rules to build staging/cpu1/cf/fm_monitor.tbl
elf/cpu1/fm_monitor.c.o: /home/jstar/Desktop/github-nos3/fsw/build/amd64-nos3/default_cpu1/apps/fm/libtblobj_cpu1_fm.tbl3b997c1a8bf025a8a3486bc83a55a02021eb857a.a
staging/cpu1/cf/fm_monitor.tbl: elf/cpu1/fm_monitor.c.o


