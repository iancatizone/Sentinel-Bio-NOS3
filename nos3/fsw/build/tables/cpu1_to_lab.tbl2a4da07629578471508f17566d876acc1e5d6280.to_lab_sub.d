# Template for table configuration

cfetables: staging/cpu1/cf/to_lab_sub.tbl

staging/cpu1/cf/to_lab_sub.tbl: CFE_TABLE_CPUNAME   := cpu1
staging/cpu1/cf/to_lab_sub.tbl: CFE_TABLE_APPNAME   := to_lab
staging/cpu1/cf/to_lab_sub.tbl: CFE_TABLE_BASENAME  := to_lab_sub

# Rules to build staging/cpu1/cf/to_lab_sub.tbl
elf/cpu1/to_lab_sub.c.o: /home/jstar/Desktop/github-nos3/fsw/build/amd64-nos3/default_cpu1/apps/to_lab/libtblobj_cpu1_to_lab.tbl2a4da07629578471508f17566d876acc1e5d6280.a
staging/cpu1/cf/to_lab_sub.tbl: elf/cpu1/to_lab_sub.c.o


