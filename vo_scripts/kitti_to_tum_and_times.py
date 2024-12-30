import pandas as pd
import os
import numpy as np
from scipy.spatial.transform import Rotation as R
import argparse
from evo.core.trajectory import PoseTrajectory3D
from evo.tools import file_interface


def kitti_poses_and_timestamps_to_trajectory(poses_file, timestamp_file):
    pose_path = file_interface.read_kitti_poses_file(poses_file)
    raw_timestamps_mat = file_interface.csv_read_matrix(timestamp_file)
    error_msg = ("timestamp file must have one column of timestamps and same number of rows as the KITTI poses file")
    if len(raw_timestamps_mat) > 0 and len(raw_timestamps_mat[0]) != 1 or len(raw_timestamps_mat) != pose_path.num_poses:
        raise file_interface.FileInterfaceException(error_msg)
    try:
        timestamps_mat = np.array(raw_timestamps_mat).astype(float)
    except ValueError:
        raise file_interface.FileInterfaceException(error_msg)
    return PoseTrajectory3D(poses_se3=pose_path.poses_se3, timestamps=timestamps_mat)

parser = argparse.ArgumentParser()
parser.add_argument("--dir", required=True, help="Folder in which you have GTs")
parser.add_argument("--sequence", required=True, help="KITTI sequence")
parser.add_argument("--times_dir", required=True, help="Timestamp files directory")
args = parser.parse_args()

dataset_path = args.dir
seq = args.sequence
times_dir = args.times_dir
complete_sequence_path = os.path.join(dataset_path, seq)

# names = ['tx', 'ty', 'tz', 'qx', 'qy', 'qz', 'qw']
# new_gt = pd.DataFrame()
#
# times = pd.read_csv(os.path.join(times_dir, seq, 'times.txt'), names=['timestamp'], header=0)
# old_gt = pd.read_csv(os.path.join(complete_sequence_path, seq + '.txt'), sep=' ')

traj = kitti_poses_and_timestamps_to_trajectory(
    os.path.join(complete_sequence_path, seq + '.txt'), os.path.join(times_dir, seq, 'times.txt'))

file_interface.write_tum_trajectory_file(os.path.join(complete_sequence_path, 'gt_tumFormat.txt'), traj)

# for i in range(len(old_gt)):
#     temp_np = np.array(old_gt.iloc[i])
#     # print(temp_np)
#     trasl = temp_np[9:]
#     rot = temp_np[:9].reshape(3, 3)
#     # print(trasl)
#     # print(rot)
#     r_mat = R.from_matrix(rot)
#     # print(r_mat.as_quat())
#     new_arr = np.concatenate((trasl, r_mat.as_quat()))
#     # print(new_arr)
#     new_gt = pd.concat((new_gt, pd.DataFrame(new_arr.reshape(1, 7), columns=names)))
#
# final_gt = pd.concat((times, new_gt.reset_index(drop=True)), axis=1)
# final_gt.to_csv(os.path.join(complete_sequence_path, 'gt_tumFormat.txt'), index=None, sep=' ', header=0)

print("NEW KITTI GT SAVED IN: ", os.path.join(complete_sequence_path, 'gt_tumFormat.txt'))
