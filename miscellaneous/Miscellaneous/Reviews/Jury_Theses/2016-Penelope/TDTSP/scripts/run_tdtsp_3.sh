#!/bin/bash

INSTANCES=InstancesTDTSP
NEWINSTANCES=InstancesTDTSP/NewTWinstances
MATRIX=InstancesTDTSP/tdtsp_bench_255_6h00_12h30_mirrored.txt
MATRIX1=InstancesTDTSP/tdtsp_bench_255_6h00_12h30_10p_ti_mirrored.txt
MATRIX2=InstancesTDTSP/tdtsp_bench_255_6h00_12h30_20p_ti_mirrored.txt

run_tdtsp_next(){
  local instance=$1
  # local matrix_type=$2
	echo "Running tdtsp_next.cpp on ${instance}.txt";
  rm -f ${instance}.log
 ./tdtsp_next.exe ${MATRIX1} ${instance}.txt > ${instance}_NEXTDFS_10p_ti.log;
 ./tdtsp_next.exe ${MATRIX2} ${instance}.txt > ${instance}_NEXTDFS_20p_ti.log;
 ./tdtsp_next.exe ${MATRIX} ${instance}.txt > ${instance}_NEXTDFS.log;
 # ./tdtsp_next.exe ${MATRIX1} ${instance}.txt > ${instance}_NEXTRESTART_10p_ti.log;
 # ./tdtsp_next.exe ${MATRIX2} ${instance}.txt > ${instance}_NEXTRESTART_20p_ti.log;
 # ./tdtsp_next.exe ${MATRIX} ${instance}.txt > ${instance}_NEXTRESTART.log;

}



run_tdtsp_tdnooverlap_DFS(){

	local instance=$1
  echo "Running tdtsp_tdnooverlap_DFS.cpp on ${instance}.dat";
  rm -f ${instance}.log
 ./tdtsp_tdnooverlap_DFS.exe ${MATRIX1} ${instance}.txt > ${instance}_TDNOOVERDFS_10p_ti.log;
 ./tdtsp_tdnooverlap_DFS.exe ${MATRIX2} ${instance}.txt > ${instance}_TDNOOVERDFS_20p_ti.log;
 ./tdtsp_tdnooverlap_DFS.exe ${MATRIX} ${instance}.txt > ${instance}_TDNOOVERDFS.log;

}
run_tdtsp_sched_DFS(){

	local instance=$1
  local matrix_type=$2
	echo "Running tdtsp_sched3.cpp on ${instance}.dat";
  rm -f ${instance}.log
 ./tdtsp_sched3.exe  ${MATRIX}${matrix_type}.txt ${instance}.txt > ${instance}_SCHEDDFS${matrix_type}.log;

}
run_tdtsp_tdnooverlap_RESTART(){

	local instance=$1
  echo "Running tdtsp_tdnooverlap_Restart.cpp on ${instance}.txt";
  rm -f ${instance}.log
 ./tdtsp_tdnooverlap_Restart.exe ${MATRIX} ${instance}.txt > ${instance}_TDNOOVERRESTART.log;
 ./tdtsp_tdnooverlap_Restart.exe ${MATRIX1} ${instance}.txt > ${instance}_TDNOOVERRESTART_10p_ti.log;
 ./tdtsp_tdnooverlap_Restart.exe ${MATRIX2} ${instance}.txt > ${instance}_TDNOOVERRESTART_20p_ti.log;
 # ./tdtsp_tdnooverlap_Restart.exe ${MATRIX}${matrix_type}_mirrored.txt ${instance}.txt > ${instance}_TDNOOVERRESTART${matrix_type}.log;

}
run_tsp_nooverlap_median(){

	local instance=$1
  # local matrix_type=$2
  echo "Running tsp_nooverlap.cpp on ${instance}.txt";
  rm -f ${instance}.log
 ./tsp_nooverlap.exe ${MATRIX1} ${instance}.txt > ${instance}_TSPMEDIAN_10p_ti.log;
 ./tsp_nooverlap.exe ${MATRIX2} ${instance}.txt > ${instance}_TSPMEDIAN_20p_ti.log;
 ./tsp_nooverlap.exe ${MATRIX} ${instance}.txt > ${instance}_TSPMEDIAN.log;

}
run_tw_instances_gen(){
  local instance=$1
  local seed=$2
	echo "Running tdtsp_tw_instances_generator.cpp on ${instance}.txt";
  rm -f ${instance}.log
 ./tdtsp_tw_instances_generator.exe ${MATRIX1} ${instance}.txt ${seed} > ${instance}_TW.txt;
 
}
run_tw_instances_gen_tsp(){
  local instance=$1
  local seed=$2
	echo "Running tdtsp_tw_instances_generator.cpp on ${instance}.txt";
  rm -f ${instance}.log
 ./tdtsp_tw_instances_generator_tsp.exe ${MATRIX1} ${instance}.txt ${seed} > ${instance}_TW.txt;
 
}
run_solChecker(){
  local instance=$1
  rm -f ${instance}.log
 ./tdtsp_solChecker.exe ${MATRIX1} ${instance}_1TW.txt ${instance}_TDNOOVERRESTART_10p_ti.log; 
}
run_tw_testing(){
  local instance=$1
  rm -f ${instance}.log
 ./tdtsp_tw_testing.exe ${MATRIX1} ${instance}_1TW.txt ${instance}_TDNOOVERRESTART_10p_ti.log; 
}

run_print_instance(){
  local instance=$1
  rm -f ${instance}.log
 ./tdtsp_print_sol.exe ${MATRIX1} ${instance}.txt; 
}

run_print_sol(){
  local instance=$1
  local algo=$2
  rm -f ${instance}.log
 ./tdtsp_print_sol.exe ${MATRIX} ${instance}.txt ${instance}_${algo}.log; 
 ./tdtsp_print_sol.exe ${MATRIX1} ${instance}.txt ${instance}_${algo}_10p_ti.log; 
 ./tdtsp_print_sol.exe ${MATRIX2} ${instance}.txt ${instance}_${algo}_20p_ti.log; 
}


# for (( j=1; j<=10; j++))
# do
  # run_tw_instances_gen_tsp ${NEWINSTANCES}/inst_10_${j} $((${j}))
  # run_tw_instances_gen_tsp ${NEWINSTANCES}/inst_100_${j} $((200 + ${j}))
# done
# for (( j=11; j<=20; j++))
# do
  # run_tw_instances_gen ${NEWINSTANCES}/inst_10_${j} $((${j}))
  # run_tw_instances_gen ${NEWINSTANCES}/inst_100_${j} $((200 + ${j}))
# done

for (( j=1; j<=10; j++))
do
  # run_tw_instances_gen ${NEWINSTANCES}/inst_20_${j} $((20 + ${j}))
  # run_tw_instances_gen ${NEWINSTANCES}/inst_30_${j} $((60+${j}))
  # run_tw_instances_gen ${NEWINSTANCES}/inst_50_${j} $((100+${j}))
  # run_tw_instances_gen ${NEWINSTANCES}/inst_100_${j} $((200+${j}))
  # run_solChecker ${INSTANCES}/20/${NAMEINSTANCE}_20_${j}
  # run_tw_testing ${INSTANCES}/100/${NAMEINSTANCE}_100_${j}
  # run_tdtsp_next ${INSTANCES}/10/${NAMEINSTANCE}_10_${j}
  # run_tdtsp_tdnooverlap_DFS ${NEWINSTANCES}/inst_10_${j}_TW
  # run_tdtsp_tdnooverlap_RESTART ${NEWINSTANCES}/inst_10_${j}_TW
  run_tdtsp_next ${NEWINSTANCES}/inst_10_${j}_TW
done

# for (( j=1; j<=2; j++))
# do
  # run_print_instance ${NEWINSTANCES}/inst_100_${j}_TW
  # run_print_instance ${NEWINSTANCES}/inst_20_${j}_TW
  # run_print_sol ${NEWINSTANCES}/inst_100_${j}_TW TDNOOVERRESTART
# done

# for (( j=6; j<=7; j++))
# do
  # run_print_instance ${NEWINSTANCES}/inst_100_${j}_TW
  # run_print_instance ${NEWINSTANCES}/inst_20_${j}_TW
  # run_print_sol ${NEWINSTANCES}/inst_100_${j}_TW TDNOOVERRESTART
# done

