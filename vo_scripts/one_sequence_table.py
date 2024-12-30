import os, argparse

import numpy as np
import pandas as pd

parser = argparse.ArgumentParser()
parser.add_argument("--dir", required=True, help="Directory path called ALGO_DATASET_PARAM1_PARAM2")

args = parser.parse_args()
abs_path: str = args.dir
dir_name = abs_path.split('/')[-1]

algo_name = dir_name.split('_')[0]
dataset = dir_name.split('_')[1]
if 'euroc' in dir_name:
    sequence = dir_name.split('_')[2] + '_' + dir_name.split('_')[3] + '_' + dir_name.split('_')[4]
elif 'arzaglia' in dir_name:
    sequence = dir_name.split('_')[2] + '_' + dir_name.split('_')[3]
else:
    sequence = dir_name.split('_')[2]

# print('algo: {}, dataset: {}, sequence: {}'.format(algo_name, dataset, sequence))

names = ['FEATURES NUMBER', 'PYRAMID LEVEL', 'ATE [m]', 'ARE [deg]', 'TR [%]']
df = pd.DataFrame(columns=names)

for directory in os.listdir(abs_path):
    n_feat = int(float(directory.split('_')[0]))
    pyr_lvl = int(directory.split('_')[1])
    # print(n_feat, ' and ', pyr_lvl)
    new_line = {'FEATURES NUMBER': n_feat, 'PYRAMID LEVEL': pyr_lvl, 'ATE [m]': None, 'ARE [deg]': None, 'TR [%]': None}
    for file in os.listdir(os.path.join(abs_path, directory)):
        if file == 'trans_table.csv':
            ate, tr = 0, 0
            try:
                tr_table = pd.read_csv(os.path.join(abs_path, directory, file))
                ate = np.round(tr_table['rmse'][0], 3)
                tr = np.round(tr_table['trajectory_ratio'][0], 3)
            except:
                NotADirectoryError
                # Trans table does not exist --> FAIL
                ate, tr = np.nan, np.nan

            new_line['ATE [m]'] = ate
            new_line['TR [%]'] = tr

            # print('tr table:')
            # print(tr_table)

        if file == 'rotat_table.csv':
            are = 0
            try:
                rot_table = pd.read_csv(os.path.join(abs_path, directory, file))
                are = np.round(rot_table['rmse'][0], 3)
            except:
                NotADirectoryError
                # Trans table does not exist --> FAIL
                are = np.nan

            new_line['ARE [deg]'] = are
            # print('rotat table:')
            # print(rot_table)

    # print(new_line)
    df = df._append(new_line, ignore_index=True)
    # with pd.option_context('display.max_rows', None, 'display.max_columns', None, 'display.precision', 3, ):
    #     print(df)

df = df.sort_values(by=['FEATURES NUMBER', 'PYRAMID LEVEL'])
# print(df)
df.to_csv(os.path.join(abs_path.removesuffix(abs_path.split('/')[-1])
                       , algo_name + '_' + dataset + '_' + sequence + '.csv'),
          index=None, sep=',')

# temp = pd.read_csv(os.path.join(abs_path.removesuffix(abs_path.split('/')[-1])
#                        , algo_name + '_' + dataset + '_' + sequence + '.csv'))
#
# print(temp['ATE [m]'])