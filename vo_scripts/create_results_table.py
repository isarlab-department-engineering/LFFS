import pandas as pd
import numpy as np
import os
import sys
import argparse
from pathlib import Path
from datetime import datetime


######################################################################
# UTILITIES AND TOOLS (SELF CONTAINED)
######################################################################

def custom_sort_key_session(item):
    # Expected item
    parts = item.split('_')
    if len(parts) >= 2:
        # Extract the first number (e.g., '40' from '40_T3')
        kpts = int(parts[0])
        # Extract the "T" value (e.g., 'T3')
        levels = parts[1]
    else:
        msg = "The folder naming is not supported"
        print(msg)
        raise ValueError(msg)

    return kpts, levels


class FolderCreator:
    def __init__(self, path):
        self.path = os.path.abspath(path)
        try:
            os.mkdir(self.path)
        except Exception as e:
            msg = ''.join(["Error while creating the folder: ", str(self.path), "\n", str(e)])
            print(msg)
            sys.exit(1)

    def get_path(self):
        return self.path


def setup_logging():
    pass


def str2bool(v):
    if isinstance(v, bool):
        return v
    if v.lower() in ('yes', 'true', 't', 'y', '1'):
        return True
    elif v.lower() in ('no', 'false', 'f', 'n', '0'):
        return False
    else:
        raise argparse.ArgumentTypeError('Boolean value expected.')


def check_directory(path_str):
    if not os.path.isdir(Path(path_str)):
        message = "Folder {} NOT found".format(path_str)
        print(message)
        return False
    else:
        return True


def check_file(file_path_str):
    if not os.path.isfile(file_path_str):
        message = "File {} NOT found".format(file_path_str)
        print(message)
        return False
    else:
        return True


######################################################################
# SET CONFIGS
######################################################################

parser = argparse.ArgumentParser(description='Data configuration')

parser.add_argument('--dataset', type=str, required=True,
                    help='Dataset name, allowed names: kitty, euroc, tum, marzaglia')

parser.add_argument('--root_folder', type=str, required=True,
                    help="The folder containing results divided by folder")

parser.add_argument('--algorithm', type=str, required=True,
                    help="The algorithm name  corresponding to a the folder name: superpoint, kitty, tum")

args = parser.parse_args()

# Results Files
rotation_results = "rotat_table.csv"
translation_results = "trans_table.csv"

######################################################################
# EXTRACTING
######################################################################
# Parse the Argument
root_folder = args.root_folder
algorithm = args.algorithm
dataset_str_list = str(args.dataset)

# Check if dataset list is ok -
# TODO: MUST BE MOVED LATER
dataset_list = dataset_str_list.split(sep=",")
if len(dataset_list) >= 1:
    pass
else:
    raise ValueError("Invalid dataset list.")

# Create results folder (if not already created)
folder_eva_name = "Evaluations"

if not os.path.exists(os.path.join(root_folder, folder_eva_name)):
    evaluation_folder = FolderCreator(os.path.join(root_folder, folder_eva_name)).get_path()
else:
    evaluation_folder = os.path.join(root_folder, folder_eva_name)

# Create session folder
session_name = "".join(["eva", "_", str(algorithm), "_", datetime.now().strftime("%d_%m_%Y_%H_%M_%S")])
session_folder = FolderCreator(os.path.join(evaluation_folder, session_name))

# Define Hyperparameters folders
algorithm_results_path = str(os.path.join(root_folder, algorithm))
check_directory(algorithm_results_path)

# main loop iterating dataset names.
for dataset in dataset_list:

    # Create folder dataset@Algorithm
    dataset_session = "".join(["eva_", dataset])
    out_folder = FolderCreator(os.path.join(str(session_folder.get_path()), dataset_session))

    # Get list of hyperparms folder
    hyperparams_folders = os.listdir(algorithm_results_path)
    hyperparams_sorted = sorted(hyperparams_folders, key=custom_sort_key_session, reverse=False)

    # Create the header
    columns = ["ID", "HyperSet"]
    rot_header_des = ["rmse"]
    trans_header_des = ["rmse", "trajectory_ratio"]
    rot_header = []
    trans_header = []
    result_df = pd.DataFrame()
    headers_found = False
    processing_done = False


    # subsets_elem_sorted = subsets_elem
    while not processing_done:
        if headers_found:
            process_started = True
        else:
            process_started = False

        # The hyperparameters are the main folders of a dataset i.e. 500_1, 500_2 etc.
        for hyperparam_case in hyperparams_sorted:
            active_hyp_dir = os.path.join(algorithm_results_path, hyperparam_case).__str__()
            active_dataset_dir = os.path.join(active_hyp_dir, dataset).__str__()

            # Get list of Scenarios
            check_directory(active_dataset_dir)
            scenario_list = os.listdir(active_dataset_dir)
            scenario_list_sorted = sorted(scenario_list)

            # The Scenarios of a dataset, that are the same for each hyperparam folder
            # i.e:
            # - 00, 01 (kitty),
            # - VH1, MH1(euroc)
            # - etc..
            for scenario in scenario_list_sorted:
                active_scenario_directory = os.path.join(active_dataset_dir, scenario)
                ape_rot_results = os.path.join(active_scenario_directory, rotation_results)
                ape_tran_results = os.path.join(active_scenario_directory, translation_results)
                result = (check_file(ape_rot_results) and check_file(ape_tran_results))

                if not headers_found:  # Scan Documents
                    print("Scanning folder {} to check if there are results to instatiate header: {}".format(
                        active_scenario_directory, result))

                    if result:
                        # Filtering  headers
                        [rot_header.append("".join([column, "_ROTO"])) for column in pd.read_csv(ape_rot_results).columns.values.tolist() if
                         column in rot_header_des]
                        [trans_header.append("".join([column, "_TRANS"])) for column in pd.read_csv(ape_tran_results).columns.values.tolist()
                         if column in trans_header_des]
                        print("Selected original headers: \n\t-rot_header: {}\n\t-tran_header: {}".format(rot_header, trans_header))

                        # Control if all the desired columns are found, otherwise raise exception
                        if len(rot_header_des) == len(rot_header) and len(trans_header_des) == len(trans_header):
                            [columns.append(col) for col in trans_header]
                            [columns.append(col) for col in rot_header]
                            result_df = pd.DataFrame(columns=columns)
                            headers_found = True

                        else:
                            message = "Valid results found in {}, but not coherent length of headers".format(active_scenario_directory)
                            raise FileExistsError(message)

                # Inner loop processing: files creation
                elif headers_found and process_started:  # Headers already set
                    print("Now processing {}".format(os.path.join(active_dataset_dir, scenario)))
                    if result:
                        trans_res_list = pd.read_csv(ape_tran_results)[trans_header_des].iloc[0, :].tolist() # Assuming one single result
                        rot_res_list = pd.read_csv(ape_rot_results)[rot_header_des].iloc[0, :].tolist()
                        new_row = [scenario, hyperparam_case]
                        new_row.extend(trans_res_list)
                        new_row.extend(rot_res_list)

                    else:
                        new_row = [scenario, hyperparam_case]
                        [new_row.append(np.nan) for col in trans_header]
                        [new_row.append(np.nan) for col in rot_header]
                    result_df.loc[len(result_df)] = new_row
                    #result_df = result_df._append(new_row, ignore_index=True)

        # If after the first scan of all the scenarios fo the current dataset no valid results are found, we raise an
        # exception and stop the processing.
        if not headers_found and not process_started:
            raise FileNotFoundError("Unable to find a valid scenario for the all the hyperparameters, implying "
                                    " the Dataset {} was not processed correctly.".format(dataset))
        elif headers_found and not process_started:
            pass
        elif headers_found and process_started:
            print("Processing for dataset {} Done".format(dataset))
            processing_done = True
            df_name = "".join([dataset, "_results.csv"])
            out_file = os.path.join(out_folder.get_path(), df_name)
            result_df.to_csv(out_file)
        else:
            message = "BUG: inconsistent state while processing the dataset {}".format(dataset)
            raise SystemError(message)
