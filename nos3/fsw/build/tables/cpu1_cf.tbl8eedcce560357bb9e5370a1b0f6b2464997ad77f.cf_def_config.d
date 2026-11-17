# Template for table configuration

cfetables: staging/cpu1/cf/cf_def_config.tbl

staging/cpu1/cf/cf_def_config.tbl: CFE_TABLE_CPUNAME   := cpu1
staging/cpu1/cf/cf_def_config.tbl: CFE_TABLE_APPNAME   := cf
staging/cpu1/cf/cf_def_config.tbl: CFE_TABLE_BASENAME  := cf_def_config

# Rules to build staging/cpu1/cf/cf_def_config.tbl
elf/cpu1/cf_def_config.c.o: /home/jstar/Desktop/github-nos3/fsw/build/amd64-nos3/default_cpu1/apps/cf/libtblobj_cpu1_cf.tbl8eedcce560357bb9e5370a1b0f6b2464997ad77f.a
staging/cpu1/cf/cf_def_config.tbl: elf/cpu1/cf_def_config.c.o


