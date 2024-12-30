#!/bin/bash

DATASET_PATH=$1
DATASET=$2

DEST_DIR=$3

for SEQUENCE_PATH in $DATASET_PATH$DATASET/*; do
	SEQUENCE=${SEQUENCE_PATH##*/}
	if [ "$SEQUENCE" = "download_euroc.sh" ]; then
		break
	fi
	if [ "$DATASET" = "euroc" ]; then
		#echo $SEQUENCE_PATH/mav0/state_groundtruth_estimate0/data.csv
		#echo $DEST_DIR$DATASET/$SEQUENCE
		#echo
		cp $SEQUENCE_PATH/mav0/state_groundtruth_estimate0/data.csv $DEST_DIR$DATASET/$SEQUENCE/data.csv
	fi
	if [ "$DATASET" = "kitti" ]; then
		readarray -d . -t strarr <<< "$SEQUENCE"
	  	SPLIT_SEQ="${strarr[0]}"
		echo $SEQUENCE
		echo $DEST_DIR/$DATASET/$SPLIT_SEQ
		cp $SEQUENCE_PATH $DEST_DIR/$DATASET/$SPLIT_SEQ/$SEQUENCE
	fi
	
done
