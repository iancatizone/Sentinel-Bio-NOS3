# Template for table configuration

cfetables: staging/cpu1/cf/spacecop_command_monitor_table.tbl

staging/cpu1/cf/spacecop_command_monitor_table.tbl: CFE_TABLE_CPUNAME   := cpu1
staging/cpu1/cf/spacecop_command_monitor_table.tbl: CFE_TABLE_APPNAME   := spacecop
staging/cpu1/cf/spacecop_command_monitor_table.tbl: CFE_TABLE_BASENAME  := spacecop_command_monitor_table

# Rules to build staging/cpu1/cf/spacecop_command_monitor_table.tbl
elf/cpu1/spacecop_command_monitor_table.c.o: /home/jstar/Desktop/github-nos3/fsw/build/amd64-nos3/default_cpu1/apps/spacecop/fsw/cfs/libtblobj_cpu1_spacecop.tbl08203f6f0cbdf075dd052e25937c3e63bcfb58d7.a
staging/cpu1/cf/spacecop_command_monitor_table.tbl: elf/cpu1/spacecop_command_monitor_table.c.o


