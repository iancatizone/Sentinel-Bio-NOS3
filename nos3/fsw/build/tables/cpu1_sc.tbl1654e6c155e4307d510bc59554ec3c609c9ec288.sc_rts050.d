# Template for table configuration

cfetables: staging/cpu1/cf/sc_rts050.tbl

staging/cpu1/cf/sc_rts050.tbl: CFE_TABLE_CPUNAME   := cpu1
staging/cpu1/cf/sc_rts050.tbl: CFE_TABLE_APPNAME   := sc
staging/cpu1/cf/sc_rts050.tbl: CFE_TABLE_BASENAME  := sc_rts050

# Rules to build staging/cpu1/cf/sc_rts050.tbl
elf/cpu1/sc_rts050.c.o: /home/jstar/Desktop/github-nos3/fsw/build/amd64-nos3/default_cpu1/apps/sc/libtblobj_cpu1_sc.tbl1654e6c155e4307d510bc59554ec3c609c9ec288.a
staging/cpu1/cf/sc_rts050.tbl: elf/cpu1/sc_rts050.c.o


