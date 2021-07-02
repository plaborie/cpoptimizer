# MATRICES: matrix00.tdtsp, matrix10.tdtsp, matrix20.tdtsp
# INSTANCES: inst_10_a_1.txt

#!/bin/bash

SIZE=50
ID=0

rename(){
  local instance=$1
  echo "Renaming ${instance} as ALL/inst_${SIZE}_${ID}.txt"
  ((ID++))
  cp ${instance} ALL/inst_${SIZE}_${ID}.txt
}

rm -rf ALL
mkdir ALL
  
for f in *.txt;
do 
  rename $f;
done;

# for f in Small/*.txt;
# do 
  # rename $f;
# done;

# for f in Zero/*.txt;
# do 
  # rename $f;
# done;

echo "DONE: ${ID} INSTANCES WERE RENAMED."