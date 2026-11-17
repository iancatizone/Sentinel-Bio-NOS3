# Template for table configuration

cfetables: staging/cpu1/cf/to_config.tbl

staging/cpu1/cf/to_config.tbl: CFE_TABLE_CPUNAME   := cpu1
staging/cpu1/cf/to_config.tbl: CFE_TABLE_APPNAME   := to
staging/cpu1/cf/to_config.tbl: CFE_TABLE_BASENAME  := to_config

# Rules to build staging/cpu1/cf/to_config.tbl
elf/cpu1/to_config.c.o: /home/jstar/Desktop/github-nos3/fsw/build/amd64-nos3/default_cpu1/apps/to/libtblobj_cpu1_to.tbleac82570f087d67bb30ff30c503eedbe1523d28a.a
staging/cpu1/cf/to_config.tbl: elf/cpu1/to_config.c.o


