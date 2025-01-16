# LF²SLAM: Learning-based Features For visual SLAM

## Project Page
For a brief overview, a presentation video, and some qualitative results, please take a 
look at our [project page](https://marcoleg.github.io/lf2slam/) :)


[//]: <> (<div align="center">)
[//]: <> (  <img src="./overview.png" alt="Alt text" style="max-width: 50%; height: auto;">)
[//]: <> (</div>)

## Overview
Learned Features For SLAM (LF²SLAM) is a hybrid Visual Odometry (VO) approach that integrates robust data-driven 
feature extraction into a standard geometric VO pipeline. 
This repository provides the code and documentation for the LF²SLAM framework, which is designed to improve 
generalization capabilities and robustness in challenging real-world or simulated scenarios.

## License
LF<sup>2</sup>SLAM is released under [GPLv3 license](https://github.com/isarlab-department-engineering/LFFS/LICENSE), 
as hybrid approach, it integrates DL features within the original pipeline of ORBSLAM3. 
For any commercial use please also refer to the [ORBSLAM3 page](https://github.com/UZ-SLAMLab/ORB_SLAM3).

## Dependencies
- CUDA 11.8 + cudnn 8.7.0
- Ubuntu 20.0.4
- Docker
- OpenCV 3.4.6
- Libtorch 2.0.0
- For other dependencies, please refer to Dockerfile

## Usage with 
Our Docker image can be downloaded [HERE](www.google.com) (about 85GB) or you can use the Dockerfile provided in this
repository. Regarding the Dockerfile, build it by:
```
docker build -t lffs:latest --build-arg CCAP=<CCAP_version> .
```
where ```<CCAP_version>``` is GPU dependant (it must be compatible with CUDA11, and it can be seen with 
```nvidia-smi --query-gpu=compute_cap --format=csv,noheader``` or by looking for your GPU model 
[here](https://developer.nvidia.com/cuda-gpus)).
N.B. in our case ```CCAP=8.6```, therefore the provided Docker image can be used just with NVIDIA RTX 3060.

Run the Docker image with:
```
docker run -td -i --privileged --net=host --name=lffs -v /tmp.X11-unix -e DISPLAY=$DISPLAY -e "QT_X11_NO_MITSHM=1" -h $HOSTNAME -v $XAUTHORITY:/root/.Xauthority:rw --runtime=nvidia --gpus all --env="NVIDIA_DRIVER_CAPABILITIES=all" <imageID>
```
You have to insert your ```<imageID>``` (it can be seen by running ```docker image list```), then:
```
docker exec lffs -it bash
```

## Run LF²SLAM

### On a custom sequence from our dataset
The following commands download our dataset sequence and run LF²SLAM. We created a ```/root/Archive``` directory,
but you can use ```/root``` or create your own. 
```
cd /root/LFFS/scripts
bash ./run_unilab_seq.sh /root/Archive/Dataset Casi3
```
The output trajectory will be the file
```KeyFrameTrajectory.txt``` or ```CameraTrajectory.txt``` in the ```LFFS``` folder.

### On a sequence from EuRoC dataset
Please download a sequence from EuRoC dataset (*e.g.*, [MH_01_easy](http://robotics.ethz.ch/~asl-datasets/ijrr_euroc_mav_dataset/machine_hall/MH_01_easy/MH_01_easy.zip))),
then unzip the sequence and create (if it does not exist) a folder hierarchy as above:
```bash
-- Dataset
    |-- UniversityTUM
    |   `-- Casi3
    |       `-- images
    `-- euroc
        `-- MH_01_easy
            `-- mav0
```
The EuRoC folder name has to be the same as the one you have to insert in the ```run_unilab_seq.sh``` script file,
(line 7). Then run:
```
cd /root/LFFS/scripts
bash ./run_unilab_seq.sh /root/Archive/Dataset MH_01_easy
```
You can also use a TUM sequence or create your own by using the same formatting as the others.

## Trajectory Evaluation
We use [EVO](https://github.com/MichaelGrupp/evo) to evaluate translation and rotation errors. By running:
```
cd /root/LFFS/scripts
bash eval_traj.sh /root/Archive/Dataset /root/Archive/exp_res Casi3
```
in the ```exp_res``` directory, you can check two *.csv* files regarding the translational and rotational error respectively.
Then, if you run:
```
evo_ape tum /root/Archive/exp_res/UniversityTUM/Casi3/gt_tumFormat.txt /root/Archive/exp_res/UniversityTUM/Casi3/traj_rotated.txt  -as --plot_mode=xy --save_plot /root/Archive/ 
```
you can obtain the ground-truth VS estimated trajectory plot, which highlights the translational error.

To plot the rotational error over the trajectory, please add ```--pose_relation angle_deg``` to the above command.

## Cite
To cite this work, please use the BibTeX below:

```
@INPROCEEDINGS{10801935,
  author={Legittimo, Marco and Crocetti, Francesco and Fravolini, Mario Luca and Mollica, Giuseppe and Costante, Gabriele},
  booktitle={2024 IEEE/RSJ International Conference on Intelligent Robots and Systems (IROS)}, 
  title={LF2SLAM: Learning-based Features For visual SLAM}, 
  year={2024},
  volume={},
  number={},
  pages={5648-5655},
  keywords={Training;Visualization;Simultaneous localization and mapping;Pipelines;Pose estimation;Feature extraction;Robustness;Tuning;Standards;Optimization},
  doi={10.1109/IROS58592.2024.10801935}}
```