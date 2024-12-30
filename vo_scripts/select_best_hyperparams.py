import os
import pandas as pd

# Change these three variables
algo = 'superslam'  # superslam orbslam
dataset = 'euroc'  # kitti euroc
sequence = 'MH_01_easy'  # 10 MH_01_easy

def perform_total_metric(ate, are, tr):
    new_tr = None
    if 0 < tr < 1:
        new_tr = 1/(tr ** 2)
    elif tr >= 1:
        new_tr = tr ** 2
    else:
        assert False, "Trajectory ratio equals (or lower) to 0! The sequence failed"

    return new_tr * (ate + are)


path_summary_tables = '/root/Archive/'
summary_table_path = os.path.join(path_summary_tables, algo + '_' + dataset + '_' + sequence + '.csv')

print('*' * 50, '\n', 'Compute overall metric R and choose best hyperparams on:\n', summary_table_path, '\n', '*' * 50)
names = ['FEATURES NUMBER', 'PYRAMID LEVEL', 'ATE [m]', 'ARE [deg]', 'TR [%]']
df = pd.read_csv(summary_table_path, sep=',', names=names, header=0)

best_R = 1e18
best_n_feat = None
best_pyr_lvl = None
best_ate = None
best_are = None
best_tr = None

USE_NORMALIZED_R = False



if USE_NORMALIZED_R:

    min_ate = df.min().iloc[2]
    max_ate = df.max().iloc[2]
    min_are = df.min().iloc[3]
    max_are = df.max().iloc[3]

    deno_ate = max_ate - min_ate
    deno_are = max_are - min_are

    for index, row in df.iterrows():
        normalized_ate = (row['ATE [m]'] - min_ate) / deno_ate
        normalized_are = (row['ARE [deg]'] - min_are) / deno_are
        R = perform_total_metric(ate=normalized_ate, are=normalized_are, tr=row['TR [%]'])
        if R < best_R:
            best_n_feat = int(row['FEATURES NUMBER'])
            best_pyr_lvl = int(row['PYRAMID LEVEL'])
            best_ate = row['ATE [m]']
            best_are = row['ARE [deg]']
            best_tr = row['TR [%]']
            best_R = R
else:
    for index, row in df.iterrows():
        R = perform_total_metric(ate=row['ATE [m]'], are=row['ARE [deg]'], tr=row['TR [%]'])
        if R < best_R:
            best_n_feat = int(row['FEATURES NUMBER'])
            best_pyr_lvl = int(row['PYRAMID LEVEL'])
            best_ate = row['ATE [m]']
            best_are = row['ARE [deg]']
            best_tr = row['TR [%]']
            best_R = R


print('Best Hyperparamters of Algorithm: {}, on Dataset: {}, Sequence: {}'.format(algo, dataset, sequence))
print('Number of features: {}, Pyramid Level: {} --> Summary Score R = {}'.format(best_n_feat, best_pyr_lvl, best_R))
print('The correspondant ATE is {}, ARE {}, TR {}'.format(best_ate, best_are, best_tr))
final_string = 'ARE' if USE_NORMALIZED_R else 'ARE NOT'
print('ATE, ARE, and R {} normalized on the sequence'.format(final_string))