#!/bin/bash

DATASETS_DIR=$1
OUTPUT_DIR=$2
ALGO=$3
SCRIPTS_DIR=$4

DATASET_LIST="euroc"
NUM_FEATURES="1800"
PYRAMID_LEVELS="4"

folder="/root/Archive/${ALGO}"

if [ ! -d "$folder" ]; then
    echo "Folder '$folder' does not exist..."
    mkdir "${folder}"
    echo "Folder '$folder' created."
else
    echo "Folder '$folder' exists, skipping creation"

fi

if [[ $ALGO == *"super"* ]]; then
  echo "Chosen Algorithm: SUPERSLAM3"
  cd /root/Programs/SuperSlam3Enhanced/
elif [[ $ALGO == *"orb"* ]]; then
  echo "Chosen Algorithm: ORBSLAM3"
  cd /root/Programs/ORB_SLAM3/
fi

# Activate Conda environment
eval "$(conda shell.bash hook)"
conda activate vo_env

for N_FEAT in $NUM_FEATURES; do

  for PYR_LEV in $PYRAMID_LEVELS; do

    # SeT new pyramid level and new max features
    python $SCRIPTS_DIR/change_yaml.py --dir $(pwd) --n_feat $N_FEAT --pyr_lvl $PYR_LEV

    for DATASET_NAME in $DATASET_LIST; do

      SEQUENCES_PATH=$DATASETS_DIR/$DATASET_NAME

      # EuRoC section
      if [[ $DATASET_NAME == "euroc" ]]; then
        for SEQUENCE_PATH in $SEQUENCES_PATH/*; do
          SEQUENCE=${SEQUENCE_PATH##*/}
          readarray -d _ -t strarr <<< "$SEQUENCE"
          SPLIT_SEQ="${strarr[0]}${strarr[1]}"

          ./Examples/Monocular/mono_euroc ./Vocabulary/ORBvoc.txt	./Examples/Monocular/EuRoC.yaml	"$DATASETS_DIR"/"$DATASET_NAME"/"$SEQUENCE" ./Examples/Monocular/EuRoC_TimeStamps/"$SPLIT_SEQ".txt
          echo "DONE EXPERIMENT WITH " $ALGO " ON " $SEQUENCE_PATH
          cp ./CameraTrajectory.txt $OUTPUT_DIR/$DATASET_NAME/$SEQUENCE/CameraTrajectory.txt
          python $SCRIPTS_DIR/gt_to_tumFormat.py --dir $OUTPUT_DIR --dataset $DATASET_NAME --sequence $SEQUENCE
          python $SCRIPTS_DIR/orb_slam3_mono_vis.py --dir $OUTPUT_DIR --dataset $DATASET_NAME --sequence $SEQUENCE

          # evo translation
          evo_ape tum $OUTPUT_DIR/$DATASET_NAME/$SEQUENCE/gt_tumFormat.txt $OUTPUT_DIR/$DATASET_NAME/$SEQUENCE/traj_rotated.txt -as --save_results $OUTPUT_DIR/$DATASET_NAME/$SEQUENCE/trans_results.zip
          evo_res $OUTPUT_DIR/$DATASET_NAME/$SEQUENCE/trans_results.zip --save_table $OUTPUT_DIR/$DATASET_NAME/$SEQUENCE/trans_table.csv

          # evo rotation
          evo_ape tum $OUTPUT_DIR/$DATASET_NAME/$SEQUENCE/gt_tumFormat.txt $OUTPUT_DIR/$DATASET_NAME/$SEQUENCE/traj_rotated.txt -as --pose_relation angle_deg --save_results $OUTPUT_DIR/$DATASET_NAME/$SEQUENCE/rotat_results.zip
          evo_res $OUTPUT_DIR/$DATASET_NAME/$SEQUENCE/rotat_results.zip --save_table $OUTPUT_DIR/$DATASET_NAME/$SEQUENCE/rotat_table.csv

          # evo trajectories length
          evo_traj tum $OUTPUT_DIR/$DATASET_NAME/$SEQUENCE/traj_rotated.txt --ref $OUTPUT_DIR/$DATASET_NAME/$SEQUENCE/gt_tumFormat.txt -as --save_as_tum
          mv traj_rotated.tum $OUTPUT_DIR/$DATASET_NAME/$SEQUENCE/traj_rotated.tum
          mv gt_tumFormat.tum $OUTPUT_DIR/$DATASET_NAME/$SEQUENCE/gt_tumFormat.tum
          python $SCRIPTS_DIR/compute_traj_length.py --dir $OUTPUT_DIR/$DATASET_NAME/$SEQUENCE

        done
      fi


      # KITTI section
      if [[ $DATASET_NAME == "kitti" ]]; then
        for SEQUENCE_PATH in $SEQUENCES_PATH/*; do
          SEQUENCE=${SEQUENCE_PATH##*/}
          readarray -d _ -t strarr <<< "$SEQUENCE"
          SPLIT_SEQ="${strarr[0]}${strarr[1]}"

          if [[ $SEQUENCE == '00' || $SEQUENCE == '01' || $SEQUENCE == '02' ]]; then
            YAML_FILE="KITTI00-02.yaml"
          elif [ $SEQUENCE == '03' ]; then
            YAML_FILE="KITTI03.yaml"
          else
            YAML_FILE="KITTI04-12.yaml"
          fi

          ./Examples/Monocular/mono_kitti ./Vocabulary/ORBvoc.txt ./Examples/Monocular/$YAML_FILE /root/Archive/Dataset/"$DATASET_NAME"/"$SEQUENCE"
          echo "DONE EXPERIMENT WITH " $ALGO " ON " $SEQUENCE_PATH
          cp ./KeyFrameTrajectory.txt $OUTPUT_DIR/$DATASET_NAME/$SEQUENCE/CameraTrajectory.txt

          python $SCRIPTS_DIR/kitti_to_tum_and_times.py --dir $OUTPUT_DIR/$DATASET_NAME --sequence $SEQUENCE --times_dir $DATASETS_DIR/$DATASET_NAME
          python $SCRIPTS_DIR/orb_slam3_mono_vis.py --dir $OUTPUT_DIR --dataset $DATASET_NAME --sequence $SEQUENCE

          # evo translation
          evo_ape tum $OUTPUT_DIR/$DATASET_NAME/$SEQUENCE/gt_tumFormat.txt $OUTPUT_DIR/$DATASET_NAME/$SEQUENCE/traj_rotated.txt -as --save_results $OUTPUT_DIR/$DATASET_NAME/$SEQUENCE/trans_results.zip
          evo_res $OUTPUT_DIR/$DATASET_NAME/$SEQUENCE/trans_results.zip --save_table $OUTPUT_DIR/$DATASET_NAME/$SEQUENCE/trans_table.csv

          # evo rotation
          evo_ape tum $OUTPUT_DIR/$DATASET_NAME/$SEQUENCE/gt_tumFormat.txt $OUTPUT_DIR/$DATASET_NAME/$SEQUENCE/traj_rotated.txt -as --pose_relation angle_deg --save_results $OUTPUT_DIR/$DATASET_NAME/$SEQUENCE/rotat_results.zip
          evo_res $OUTPUT_DIR/$DATASET_NAME/$SEQUENCE/rotat_results.zip --save_table $OUTPUT_DIR/$DATASET_NAME/$SEQUENCE/rotat_table.csv

          # evo trajectories length
          evo_traj tum $OUTPUT_DIR/$DATASET_NAME/$SEQUENCE/traj_rotated.txt --ref $OUTPUT_DIR/$DATASET_NAME/$SEQUENCE/gt_tumFormat.txt -as --save_as_tum
          mv traj_rotated.tum $OUTPUT_DIR/$DATASET_NAME/$SEQUENCE/traj_rotated.tum
          mv gt_tumFormat.tum $OUTPUT_DIR/$DATASET_NAME/$SEQUENCE/gt_tumFormat.tum
          python $SCRIPTS_DIR/compute_traj_length.py --dir $OUTPUT_DIR/$DATASET_NAME/$SEQUENCE
        done
      fi

    done

    cp -r "$OUTPUT_DIR"/ /root/Archive/$ALGO/"$N_FEAT"_"$PYR_LEV"/
    echo "Experiments saved in: " /root/Archive/$ALGO/"$N_FEAT"_"$PYR_LEV"

    for DATASET_NAME in $DATASET_LIST; do
      python $SCRIPTS_DIR/delete_previous_exp.py --dataset $DATASET_NAME
    done

  done
done



conda deactivate
