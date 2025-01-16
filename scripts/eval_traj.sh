#!/bin/bash

DATASETS_DIR=$1
OUTPUT_DIR=$2
SEQUENCE=$3

SCRIPTS_DIR="/root/LFFS/scripts"
DATASET="euroc" # UniversityTUM"  # euroc

apt update -y
apt install python3-pip -y
pip install evo
apt install python3-tk -y

cd /root/LFFS/
mkdir -p $OUTPUT_DIR/$DATASET/$SEQUENCE/

if [[ $DATASET == *"TUM"* ]]; then
  cp ./KeyFrameTrajectory.txt $OUTPUT_DIR/$DATASET/$SEQUENCE/CameraTrajectory.txt
  cp "$DATASETS_DIR"/"$DATASET"/"$SEQUENCE"/*"GT"* $OUTPUT_DIR/$DATASET/$SEQUENCE/gt.txt
  python3 $SCRIPTS_DIR/gt_to_tumFormat.py --dir $OUTPUT_DIR --dataset $DATASET --sequence $SEQUENCE
  python3 $SCRIPTS_DIR/orb_slam3_mono_vis.py --dir $OUTPUT_DIR --dataset $DATASET --sequence $SEQUENCE

  # evo translation
  evo_ape tum $OUTPUT_DIR/$DATASET/$SEQUENCE/gt_tumFormat.txt $OUTPUT_DIR/$DATASET/$SEQUENCE/traj_rotated.txt -as --save_results $OUTPUT_DIR/$DATASET/$SEQUENCE/trans_results.zip
  evo_res $OUTPUT_DIR/$DATASET/$SEQUENCE/trans_results.zip --save_table $OUTPUT_DIR/$DATASET/$SEQUENCE/trans_table.csv

  # evo rotation
  evo_ape tum $OUTPUT_DIR/$DATASET/$SEQUENCE/gt_tumFormat.txt $OUTPUT_DIR/$DATASET/$SEQUENCE/traj_rotated.txt -as --pose_relation angle_deg --save_results $OUTPUT_DIR/$DATASET/$SEQUENCE/rotat_results.zip
  evo_res $OUTPUT_DIR/$DATASET/$SEQUENCE/rotat_results.zip --save_table $OUTPUT_DIR/$DATASET/$SEQUENCE/rotat_table.csv

  # evo trajectories length
  evo_traj tum $OUTPUT_DIR/$DATASET/$SEQUENCE/traj_rotated.txt --ref $OUTPUT_DIR/$DATASET/$SEQUENCE/gt_tumFormat.txt -as --save_as_tum
  mv traj_rotated.tum $OUTPUT_DIR/$DATASET/$SEQUENCE/traj_rotated.tum
  mv gt_tumFormat.tum $OUTPUT_DIR/$DATASET/$SEQUENCE/gt_tumFormat.tum
  python3 $SCRIPTS_DIR/compute_traj_length.py --dir $OUTPUT_DIR/$DATASET/$SEQUENCE
fi

if [[ $DATASET == *"euroc"* ]]; then
  cp ./CameraTrajectory.txt $OUTPUT_DIR/$DATASET/$SEQUENCE/CameraTrajectory.txt
  cp "$DATASETS_DIR"/"$DATASET"/"$SEQUENCE"/"mav0"/"state_groundtruth_estimate0"/"data.csv" $OUTPUT_DIR/$DATASET/$SEQUENCE/data.csv
  python3 $SCRIPTS_DIR/gt_to_tumFormat.py --dir $OUTPUT_DIR --dataset $DATASET --sequence $SEQUENCE
  python3 $SCRIPTS_DIR/orb_slam3_mono_vis.py --dir $OUTPUT_DIR --dataset $DATASET --sequence $SEQUENCE

  # evo translation
  evo_ape tum $OUTPUT_DIR/$DATASET/$SEQUENCE/gt_tumFormat.txt $OUTPUT_DIR/$DATASET/$SEQUENCE/traj_rotated.txt -as --save_results $OUTPUT_DIR/$DATASET/$SEQUENCE/trans_results.zip
  evo_res $OUTPUT_DIR/$DATASET/$SEQUENCE/trans_results.zip --save_table $OUTPUT_DIR/$DATASET/$SEQUENCE/trans_table.csv

  # evo rotation
  evo_ape tum $OUTPUT_DIR/$DATASET/$SEQUENCE/gt_tumFormat.txt $OUTPUT_DIR/$DATASET/$SEQUENCE/traj_rotated.txt -as --pose_relation angle_deg --save_results $OUTPUT_DIR/$DATASET/$SEQUENCE/rotat_results.zip
  evo_res $OUTPUT_DIR/$DATASET/$SEQUENCE/rotat_results.zip --save_table $OUTPUT_DIR/$DATASET/$SEQUENCE/rotat_table.csv

  # evo trajectories length
  evo_traj tum $OUTPUT_DIR/$DATASET/$SEQUENCE/traj_rotated.txt --ref $OUTPUT_DIR/$DATASET/$SEQUENCE/gt_tumFormat.txt -as --save_as_tum
  mv traj_rotated.tum $OUTPUT_DIR/$DATASET/$SEQUENCE/traj_rotated.tum
  mv gt_tumFormat.tum $OUTPUT_DIR/$DATASET/$SEQUENCE/gt_tumFormat.tum
  python3 $SCRIPTS_DIR/compute_traj_length.py --dir $OUTPUT_DIR/$DATASET/$SEQUENCE
 fi
