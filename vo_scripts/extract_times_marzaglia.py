import pandas as pd
import os

seq = 'sequence_3'
abs_path = '/root/Archive/Dataset/Marzaglia'
dir_to_save = 'orb_slam3_mono_vis_configs'
times_path = os.path.join(abs_path, seq)

old_times = pd.read_csv(os.path.join(times_path, 'data.csv'))

old_times['#timestamp [ns]'] = old_times['#timestamp [ns]'].str.replace(',', '.').astype(float) * 1e-9

old_times['#timestamp [ns]'].to_csv(os.path.join(times_path, dir_to_save, 'times.txt'), index=None, header=None)

## TEST MARZAGLIA WITH EUROC SECTION
old_times['#timestamp [ns]'] = old_times['#timestamp [ns]'] * 1e9
old_times['#timestamp [ns]'] = old_times['#timestamp [ns]'].astype(str).str.zfill(9)
for i in range(len(old_times)):
    old_times.iloc[i, 0] = f'{float(old_times.iloc[i, 0]):.20f}'
# print(old_times)

rename = True
final_time_df = pd.DataFrame()

if rename:
    # rename images:
    for img_name in os.listdir(os.path.join(abs_path, seq, 'mav0', 'cam0', 'data')):
        # print('x'*20)
        # print(img_name)
        corresp_time = old_times.loc[old_times['filename'] == img_name]['#timestamp [ns]']
        final_time = corresp_time.values[0].split('.')[0]
        # print(final_time)
        # print('x'*20)
        os.rename(os.path.join(abs_path, seq, 'mav0', 'cam0', 'data', img_name),
                  os.path.join(abs_path, seq, 'mav0', 'cam0', 'data', final_time + '.png'))
    #old_times.to_csv(os.path.join(times_path, dir_to_save, 'frames_and_times.csv'), index=None, header=None)

for img_name in os.listdir(os.path.join(abs_path, seq, 'mav0', 'cam0', 'data')):
    final_time_df = final_time_df._append(pd.Series(img_name.split('.')[0]), ignore_index=True)
# print(final_time_df)
final_time_df = final_time_df.sort_values(by=[0])
final_time_df.to_csv(os.path.join(times_path, dir_to_save, 'times_without_comma.txt'), index=None, header=None)