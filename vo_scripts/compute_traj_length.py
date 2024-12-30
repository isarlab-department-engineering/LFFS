from evo.tools import file_interface
import pandas as pd
import argparse
import os

parser = argparse.ArgumentParser()
parser.add_argument("--dir", required=True, help="Directory path in which you have trajectories and gt")
args = parser.parse_args()
dir_path = args.dir

# Compute GT length
gt_file = os.path.join(dir_path, 'gt_tumFormat.tum')
gt_len = file_interface.read_tum_trajectory_file(gt_file).path_length

# Compute VO output length
vo_file = os.path.join(dir_path, 'traj_rotated.tum')
vo_len = file_interface.read_tum_trajectory_file(vo_file).path_length

# Trajectory ratio
tr = vo_len / gt_len

df = pd.read_csv(os.path.join(dir_path, 'trans_table.csv'))
df.insert(8, "traj_length", [vo_len])
df.insert(9, "trajectory_ratio", [tr])
df.to_csv(os.path.join(dir_path, 'trans_table.csv'))

print('Trajectory ratio saved in {}/trans_table.csv'.format(dir_path))