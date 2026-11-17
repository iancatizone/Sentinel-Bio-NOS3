# Template for table configuration

cfetables: staging/cpu1/cf/sbn_conf_tbl.tbl

staging/cpu1/cf/sbn_conf_tbl.tbl: CFE_TABLE_CPUNAME   := cpu1
staging/cpu1/cf/sbn_conf_tbl.tbl: CFE_TABLE_APPNAME   := sbn
staging/cpu1/cf/sbn_conf_tbl.tbl: CFE_TABLE_BASENAME  := sbn_conf_tbl

# Rules to build staging/cpu1/cf/sbn_conf_tbl.tbl
elf/cpu1/sbn_conf_tbl.c.o: /home/jstar/Desktop/github-nos3/fsw/build/amd64-nos3/default_cpu1/apps/sbn/libtblobj_cpu1_sbn.tbl48adf2b003ff9b69abafef910f0e8010b92a49e8.a
staging/cpu1/cf/sbn_conf_tbl.tbl: elf/cpu1/sbn_conf_tbl.c.o


