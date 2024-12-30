import os
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("--dataset", required=True, help="Dataset experiments that you want to delete")
args = parser.parse_args()
dataset = args.dataset

exp_path = "/root/Archive/exp_res"

dataset_path = os.path.join(exp_path, dataset)

for elem in os.listdir(dataset_path):
    for file in os.listdir(os.path.join(dataset_path, elem)):
        if file == 'data.csv' or file.startswith('0') or file.startswith('1'):
            continue
        os.remove(os.path.join(dataset_path, elem, file))
