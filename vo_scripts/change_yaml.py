from evo.tools import file_interface
import pandas as pd
import argparse
import os
import yaml

parser = argparse.ArgumentParser()
parser.add_argument("--dir", required=True, help="Directory path in which you Algo project")
parser.add_argument("--n_feat", required=True, help="New features number")
parser.add_argument("--pyr_lvl", required=True, help="New pyramid levels")

args = parser.parse_args()
dir_path = args.dir
n_feat = args.n_feat
pyr_lvl = args.pyr_lvl
print('Setting yaml files. Num features: {}\tPyramid levels: {}'.format(n_feat, pyr_lvl))

skip_lines = 2

# Euroc section
with open(os.path.join(dir_path, 'Examples', 'Monocular', 'EuRoC.yaml.orig')) as f:
    for i in range(skip_lines):
        _ = f.readline()
    doc = yaml.safe_load(f)

doc['ORBextractor.nFeatures'] = int(n_feat)
doc['ORBextractor.nLevels'] = int(pyr_lvl)

with open(os.path.join(dir_path, 'Examples', 'Monocular', 'EuRoC.yaml'), 'w') as f:
    yaml.dump(doc, f)

with open(os.path.join(dir_path, 'Examples', 'Monocular', 'EuRoC.yaml'), 'r') as f:
    old_yaml = f.read()

    with open(os.path.join(dir_path, 'Examples', 'Monocular', 'EuRoC.yaml'), 'w') as f_final:
        f_final.write('%YAML:1.0\n' + old_yaml)

# KITTI section
kitti_yaml = ["KITTI00-02.yaml", "KITTI03.yaml", "KITTI04-12.yaml"]

for cfg_file in kitti_yaml:
    with open(os.path.join(dir_path, 'Examples', 'Monocular', cfg_file + '.orig')) as f:
        for i in range(skip_lines):
            _ = f.readline()
        doc = yaml.safe_load(f)

    doc['ORBextractor.nFeatures'] = int(n_feat)
    doc['ORBextractor.nLevels'] = int(pyr_lvl)

    with open(os.path.join(dir_path, 'Examples', 'Monocular', cfg_file), 'w') as f:
        yaml.dump(doc, f)

    with open(os.path.join(dir_path, 'Examples', 'Monocular', cfg_file), 'r') as old:
        old_yaml = old.read()

        with open(os.path.join(dir_path, 'Examples', 'Monocular', cfg_file), 'w') as f_final:
            f_final.write('%YAML:1.0\n' + old_yaml)
