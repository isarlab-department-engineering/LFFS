import os
import pandas as pd
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("--dir", required=True, help="Folder path in which you have datasets (and GTs)")
parser.add_argument("--dataset", required=True, help="Dataset name", type=str)
parser.add_argument("--sequence", required=True, help="Sequence number", type=str)
args = parser.parse_args()

path = args.dir
dataset = args.dataset
seq_num = args.sequence

gt_name = 'data.csv' if 'euroc' in dataset else 'gt.txt'  # gt.txt is the kitti case
final_name = 'gt_tumFormat.txt'

complete_path = os.path.join(path, dataset, seq_num, gt_name)
complete_final_gt_path = os.path.join(path, dataset, seq_num, final_name)
print('Experiments directory path: {}'.format(complete_path))

if 'euroc' in dataset:
    # Read gt
    names_euroc = ['#timestamp', ' p_RS_R_x [m]', ' p_RS_R_y [m]', ' p_RS_R_z [m]', ' q_RS_w []', ' q_RS_x []',
                   ' q_RS_y []', ' q_RS_z []', ' v_RS_R_x [m s^-1]', ' v_RS_R_y [m s^-1]', ' v_RS_R_z [m s^-1]',
                   ' b_w_RS_S_x [rad s^-1]', ' b_w_RS_S_y [rad s^-1]', ' b_w_RS_S_z [rad s^-1]', ' b_a_RS_S_x [m s^-2]',
                   ' b_a_RS_S_y [m s^-2]', ' b_a_RS_S_z [m s^-2]']
    temp_names = ['#timestamp', ' p_RS_R_x [m]', ' p_RS_R_y [m]', ' p_RS_R_z [m]', ' q_RS_x []', ' q_RS_y []',
                  ' q_RS_z []', ' q_RS_w []']

    gt_original = pd.read_csv(complete_path, header=0, names=names_euroc)
    gt_original = gt_original[temp_names]
    gt_original['#timestamp'] *= 1e-9
    gt_original.to_csv(complete_final_gt_path, sep=' ', header=False, index=False)

    print("GroundTruth saved in tum format in: ", complete_final_gt_path)

elif 'kitti' in dataset:

    names = ['timestamp', 'tx', 'ty', 'tz', 'qx', 'qy', 'qz', 'qw']
    gt_original = pd.read_csv(complete_path, ' ', header=None, index_col=False, names=names)
    df_result.drop(df_result.head(1).index,inplace=True)
    df_result['timestamp'] *= 1e-9

else:
    assert False, "The given dataset does not exist!"
