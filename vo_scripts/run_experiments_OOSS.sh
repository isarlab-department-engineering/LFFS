#!/bin/bash

: '
echo "Executing script: "
bash run_experiments_OS_Euroc.sh /root/Archive/Dataset /root/Archive/exp_res orbslam /root/Programs/vo_scripts &&

echo "Executing script: "
bash run_experiments_OS_kitti.sh /root/Archive/Dataset /root/Archive/exp_res orbslam /root/Programs/vo_scripts &&

echo "Executing script: "
bash run_experiments_SS_Euroc.sh /root/Archive/Dataset /root/Archive/exp_res superslam /root/Programs/vo_scripts &&

echo "Executing script: "
bash run_experiments_SS_kitti.sh /root/Archive/Dataset /root/Archive/exp_res superslam /root/Programs/vo_scripts &&

echo "Superslam with non-normalized params"
bash run_experiments_SS_kitti_notnorm.sh /root/Archive/Dataset /root/Archive/exp_res superslam /root/Programs/vo_scripts &&

echo "Orbslam with non-normalized params"
bash run_experiments_OS_Euroc_notnorm.sh /root/Archive/Dataset /root/Archive/exp_res orbslam /root/Programs/vo_scripts
'


echo "Executing script: "
bash run_experiments_OS_Euroc.sh /root/Archive/Dataset /root/Archive/exp_res orbslam /root/Programs/vo_scripts &&

echo "Superslam with non-normalized params"
bash run_experiments_SS_Euroc.sh /root/Archive/Dataset /root/Archive/exp_res superslam /root/Programs/vo_scripts
