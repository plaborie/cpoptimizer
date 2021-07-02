#!/bin/bash
INSTANCES=InstancesTDTSP
MATRIX=InstancesTDTSP/tdtsp_bench_255_6h00_12h30_mirrored.txt
MATRIX1=InstancesTDTSP/tdtsp_bench_255_6h00_12h30_10p_ti_mirrored.txt
MATRIX2=InstancesTDTSP/tdtsp_bench_255_6h00_12h30_20p_ti_mirrored.txt


run_tdtsp_tdnooverlap_RESTART(){

	local instance=$1
  echo "Running tdtsp_tdnooverlap_Restart.cpp on ${instance}.txt";
  rm -f ${instance}.log
 ./tdtsp_tdnooverlap_Restart ${MATRIX} ${instance}.txt > ${instance}_TDNOOVERRESTART.log;
 ./tdtsp_tdnooverlap_Restart ${MATRIX1} ${instance}.txt > ${instance}_TDNOOVERRESTART_10p_ti.log;
 ./tdtsp_tdnooverlap_Restart ${MATRIX2} ${instance}.txt > ${instance}_TDNOOVERRESTART_20p_ti.log;

}

for (( j=1; j<6; j++))
do
  run_tdtsp_tdnooverlap_RESTART ${INSTANCES}/inst_20_${j}_TW
  run_tdtsp_tdnooverlap_RESTART ${INSTANCES}/inst_30_${j}_TW
  run_tdtsp_tdnooverlap_RESTART ${INSTANCES}/inst_50_${j}_TW
  run_tdtsp_tdnooverlap_RESTART ${INSTANCES}/inst_100_${j}_TW
 
done
