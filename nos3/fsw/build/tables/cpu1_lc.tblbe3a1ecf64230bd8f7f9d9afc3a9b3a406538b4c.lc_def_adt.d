# Template for table configuration

cfetables: staging/cpu1/cf/lc_def_adt.tbl

staging/cpu1/cf/lc_def_adt.tbl: CFE_TABLE_CPUNAME   := cpu1
staging/cpu1/cf/lc_def_adt.tbl: CFE_TABLE_APPNAME   := lc
staging/cpu1/cf/lc_def_adt.tbl: CFE_TABLE_BASENAME  := lc_def_adt

# Rules to build staging/cpu1/cf/lc_def_adt.tbl
elf/cpu1/lc_def_adt.c.o: /home/jstar/Desktop/github-nos3/fsw/build/amd64-nos3/default_cpu1/apps/lc/libtblobj_cpu1_lc.tblbe3a1ecf64230bd8f7f9d9afc3a9b3a406538b4c.a
staging/cpu1/cf/lc_def_adt.tbl: elf/cpu1/lc_def_adt.c.o


