#!/bin/bash

#PBS -V
#PBS -N PCST
#PBS -q batch
#PBS -l nodes=1:bigmem:ppn=10,walltime=5:25:00
#PBS -l mem=240gb
#PBS -e /data/wudb/users/sguan/pcst/Optimize_index/result/fig6c_er_err3.txt
#PBS -o /data/wudb/users/sguan/pcst/Optimize_index/result/fig6c_er_out3.txt
#PBS -M sheng.guan@wsu.edu
#PBS -m abe

### -e: error log
### -o: output log stdout

module load compilers/gcc/6.2.0

echo 'START runPCST.sh'
cd /data/wudb/users/sguan/pcst/Optimize_index/
./PCSTStaticV5 \
/data/wudb/users/sguan/pcst/data/graph_3000_e25m_pl.txt \
/data/wudb/users/sguan/pcst/data/graph_3000_e25m_pl-magfitf-2attr \
/data/wudb/users/sguan/pcst/data/_3000_2m_pl_250nodes.penalties.txt \
/data/wudb/users/sguan/pcst/data/_3000_2m_pl_250nodes.penalties.txt \
2 \

echo 'Done The result is shown in pcst'
