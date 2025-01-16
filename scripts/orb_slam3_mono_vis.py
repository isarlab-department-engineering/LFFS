import pandas as pd
import argparse
import os
import numpy as np
from scipy.spatial.transform import Rotation as R


parser = argparse.ArgumentParser()
parser.add_argument("--dir", required=True, help="--dir <path of directory in which you have trajectories output>")
parser.add_argument("--dataset", required=True, help="Dataset name", type=str)
parser.add_argument("--sequence", required=True, help="Sequence number", type=str)

args = parser.parse_args()
absolute_path = args.dir
dataset_name = args.dataset
seq_numb = args.sequence

traj_orig = os.path.join(absolute_path, dataset_name, seq_numb, 'CameraTrajectory.txt')
traj_final = os.path.join(absolute_path, dataset_name, seq_numb, 'traj_rotated.txt')

names = ['timestamp', 'tx', 'ty', 'tz', 'qx', 'qy', 'qz', 'qw']

df_result = pd.read_csv(traj_orig, sep=' ', header=None, index_col=False, names=names)
df_result['timestamp'] *= 1e-9

x_rot, y_rot, z_rot = 0, 0, 0
if "tum" in dataset_name.lower():
    x_rot = 90
    y_rot = 270
elif "euroc" in dataset_name.lower():
    z_rot = 270
elif "kitti" in dataset_name.lower():
    df_result['timestamp'] *= 1e9
    df_result.to_csv(traj_final, sep=' ', header=False, index=False)
    print("OS3/SuperSLAM3 output trajectory rotated: ", traj_final)
    import sys
    sys.exit(100)

rot_matrix = R.from_euler("xyz", [x_rot, y_rot, z_rot], degrees=True).as_matrix()

for i in range(len(df_result)):
    curr_matrix = R.from_quat(
        [df_result.iloc[i]['qx'], df_result.iloc[i]['qy'], df_result.iloc[i]['qz'],
         df_result.iloc[i]['qw']]).as_matrix()
    new_matrix = np.dot(curr_matrix, rot_matrix)
    new_quat = R.from_matrix(new_matrix).as_quat()
    df_result.iloc[i, df_result.columns.get_loc('qx')] = new_quat[0]
    df_result.iloc[i, df_result.columns.get_loc('qy')] = new_quat[1]
    df_result.iloc[i, df_result.columns.get_loc('qz')] = new_quat[2]
    df_result.iloc[i, df_result.columns.get_loc('qw')] = new_quat[3]


df_result.to_csv(traj_final, sep=' ', header=False, index=False)
print("OS3/SuperSLAM3 output trajectory rotated: ", traj_final)
