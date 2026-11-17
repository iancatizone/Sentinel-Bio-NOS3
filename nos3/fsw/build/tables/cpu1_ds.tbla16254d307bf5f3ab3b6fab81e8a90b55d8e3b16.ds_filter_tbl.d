# Template for table configuration

cfetables: staging/cpu1/cf/ds_filter_tbl.tbl

staging/cpu1/cf/ds_filter_tbl.tbl: CFE_TABLE_CPUNAME   := cpu1
staging/cpu1/cf/ds_filter_tbl.tbl: CFE_TABLE_APPNAME   := ds
staging/cpu1/cf/ds_filter_tbl.tbl: CFE_TABLE_BASENAME  := ds_filter_tbl

# Rules to build staging/cpu1/cf/ds_filter_tbl.tbl
elf/cpu1/ds_filter_tbl.c.o: /home/jstar/Desktop/github-nos3/fsw/build/amd64-nos3/default_cpu1/apps/ds/libtblobj_cpu1_ds.tbla16254d307bf5f3ab3b6fab81e8a90b55d8e3b16.a
staging/cpu1/cf/ds_filter_tbl.tbl: elf/cpu1/ds_filter_tbl.c.o


