#!/bin/bash
INSTANCES=InstancesTDTSP
MATRIX=InstancesTDTSP/tdtsp_bench_255_6h00_12h30_mirrored.txt
MATRIX1=InstancesTDTSP/tdtsp_bench_255_6h00_12h30_10p_ti_mirrored.txt
MATRIX2=InstancesTDTSP/tdtsp_bench_255_6h00_12h30_20p_ti_mirrored.txt

run_tsp_nooverlap_RESTART(){
  local instance=$1
  echo "Running tsp_nooverlap.cpp on ${instance}.txt";
  rm -f ${instance}.log
 ./tsp_nooverlap ${MATRIX} ${instance}.txt  > ${instance}_NOOVERRESTART.log;
 ./tsp_nooverlap ${MATRIX1} ${instance}.txt > ${instance}_NOOVERRESTART_10p_ti.log;
 ./tsp_nooverlap ${MATRIX2} ${instance}.txt > ${instance}_NOOVERRESTART_20p_ti.log;
}

for (( j=6; j<11; j++))
do
  run_tsp_nooverlap_RESTART ${INSTANCES}/inst_20_${j}_TW
  run_tsp_nooverlap_RESTART ${INSTANCES}/inst_30_${j}_TW
  run_tsp_nooverlap_RESTART ${INSTANCES}/inst_50_${j}_TW
  run_tsp_nooverlap_RESTART ${INSTANCES}/inst_100_${j}_TW
done
