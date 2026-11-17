# Template for table configuration

cfetables: staging/cpu1/cf/sch_def_schtbl.tbl

staging/cpu1/cf/sch_def_schtbl.tbl: CFE_TABLE_CPUNAME   := cpu1
staging/cpu1/cf/sch_def_schtbl.tbl: CFE_TABLE_APPNAME   := sch
staging/cpu1/cf/sch_def_schtbl.tbl: CFE_TABLE_BASENAME  := sch_def_schtbl

# Rules to build staging/cpu1/cf/sch_def_schtbl.tbl
elf/cpu1/sch_def_schtbl.c.o: /home/jstar/Desktop/github-nos3/fsw/build/amd64-nos3/default_cpu1/apps/sch/libtblobj_cpu1_sch.tbla3422bddc3b7962c333037be983f7563d3c495c3.a
staging/cpu1/cf/sch_def_schtbl.tbl: elf/cpu1/sch_def_schtbl.c.o


