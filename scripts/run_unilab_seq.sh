#!/bin/bash

DATASETS_DIR=$1
SEQUENCE=$2

SCRIPTS_DIR="/root/Programs/scripts"
DATASET="UniversityTUM"  # euroc

# Check if the folder exists
SEQUENCES_PATH=$DATASETS_DIR/$DATASET/$SEQUENCE
if [ ! -d "$SEQUENCES_PATH" ]; then
  echo "Sequence folder $SEQUENCES_PATH does not exist. Downloading it..."
  wget -P "$DATASETS_DIR" "http://sira.diei.unipg.it/supplementary/public/Datasets/LFFS/$DATASET.tar.gz"
  tar -xzvf "$DATASETS_DIR/$DATASET.tar.gz" -C "$DATASETS_DIR"
  rm -r "$DATASETS_DIR/$DATASET.tar.gz"
fi

cd /root/LFFS/

SEQUENCES_PATH=$DATASETS_DIR/$DATASET

# TUM and UNILAB section
if [[ $DATASET == *"TUM"* ]]; then
  echo "$DATASETS_DIR"/"$DATASET"/"$SEQUENCE"
  ./Examples/Monocular/mono_tum ./Vocabulary/ORBvoc.txt ./Examples/Monocular/bf_1012bC_2112_fisheye_noname.yaml "$DATASETS_DIR"/"$DATASET"/"$SEQUENCE"
fi

# EuRoC section
if [[ $DATASET == *"euroc"* ]]; then
  readarray -d _ -t strarr <<< "$SEQUENCE"
  SPLIT_SEQ="${strarr[0]}${strarr[1]}"
  ./Examples/Monocular/mono_euroc ./Vocabulary/ORBvoc.txt ./Examples/Monocular/EuRoC.yaml "$DATASETS_DIR"/"$DATASET"/"$SEQUENCE" ./Examples/Monocular/EuRoC_TimeStamps/"$SPLIT_SEQ".txt
fi
