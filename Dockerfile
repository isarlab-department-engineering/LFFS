FROM nvidia/cuda:11.8.0-devel-ubuntu20.04

# Arguments required: Compute Capability COMPUTE_CAP=$(nvidia-smi --query-gpu=compute_cap --format=csv,noheader)
ARG CCAP


# ------------------------------------------------------------------------------------
# CUDNN
# ------------------------------------------------------------------------------------
RUN apt-get update && \
    apt-get install -y wget && \
    rm -rf /var/lib/apt/lists/* \
WORKDIR /root

ENV CUDNN_TAR_FILE="cudnn-linux-x86_64-8.7.0.84_cuda11-archive.tar.xz"
RUN wget https://developer.download.nvidia.com/compute/redist/cudnn/v8.7.0/local_installers/11.8/${CUDNN_TAR_FILE}
RUN tar -xvf ${CUDNN_TAR_FILE}
RUN mv cudnn-linux-x86_64-8.7.0.84_cuda11-archive cudnn

# copy the following files into the cuda toolkit directory.
RUN cp -P cudnn/include/cudnn.h /usr/local/cuda-11.8/include && \
 cp -P cudnn/lib/libcudnn* /usr/local/cuda-11.8/lib64/ && \
 chmod a+r /usr/local/cuda-11.8/lib64/libcudnn*

# Remove the folder
RUN rm -r cudnn && rm ${CUDNN_TAR_FILE}

RUN echo "Nvidia CUDA compiler version:" && nvcc --version


# --------------
# MISC PREREQUISITES
# --------------

ENV DEBIAN_FRONTEND noninteractive
RUN apt-get update && \
    apt-get install -y tzdata coreutils && \
    rm -rf /var/lib/apt/lists/*

# ------------------------------------------------------------------------------------
# OPENCV
# ------------------------------------------------------------------------------------

# Setting Up the Folder
WORKDIR /root/opencv_build

# Install prerequisites Video/Audio Libs - FFMPEG, GSTREAMER, x264 etc.
RUN apt-get update && \
    apt-get install -y \
        libdc1394-22 libdc1394-22-dev libxine2-dev libv4l-dev v4l-utils \
        libfaac-dev libmp3lame-dev libvorbis-dev \
        libxvidcore-dev x264 libx264-dev libfaac-dev libmp3lame-dev libtheora-dev \
        libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev \
        libavcodec-dev libavformat-dev libswscale-dev libavresample-dev && \
    rm -rf /var/lib/apt/lists/*


#Parallelism library C++ for CPU and optimization libraries for OpenCV
RUN apt-get update && \
    apt-get install -y libtbb-dev \
    libatlas-base-dev \
    gfortran openexr libtbb2 && \
    rm -rf /var/lib/apt/lists/*

# Additional libraries
RUN apt-get update && \
    apt-get install -y libprotobuf-dev protobuf-compiler \
        libgoogle-glog-dev libgflags-dev libpng-dev libjpeg-dev \
        libgphoto2-dev libeigen3-dev libhdf5-dev libtiff-dev \
        doxygen build-essential cmake git libgtk-3-dev make g++ && \
    rm -rf /var/lib/apt/lists/*

# Additional libraries for python3
RUN apt-get update && \
    apt-get install -y python3 python3-dev python3-numpy && \
    rm -rf /var/lib/apt/lists/*


# Clone the repositories
ENV OPENCV_CONTRIB_HASH "f852576142dec4c99fbeb89902129192cda3e7b6"
RUN git clone https://github.com/opencv/opencv_contrib.git && \
    cd opencv_contrib && \
    git checkout ${OPENCV_CONTRIB_HASH}

ENV OPENCV_HASH "b1cf5501233405de3ea5926d1d688e421b337458"
RUN git clone https://github.com/opencv/opencv.git && \
    cd opencv && \
    git checkout ${OPENCV_HASH}

RUN cd opencv &&  \
    mkdir build && \
    cd build && \
    cmake -D CMAKE_BUILD_TYPE=RELEASE \
    -D CMAKE_INSTALL_PREFIX=/usr/local \
    -D INSTALL_C_EXAMPLES=OFF \
    -D INSTALL_PYTHON_EXAMPLES=OFF \
    -D OPENCV_GENERATE_PKGCONFIG=ON \
    -D WITH_TBB=ON \
    -D ENABLE_FAST_MATH=1 \
    -D CUDA_FAST_MATH=1 \
    -D WITH_CUBLAS=1 \
    -D WITH_CUDA=ON \
    -D BUILD_opencv_cudacodec=OFF \
    -D WITH_CUDNN=ON \
    -D OPENCV_DNN_CUDA=ON \
    -D CUDA_ARCH_BIN=${CCAP} \
    -D WITH_V4L=ON \
    -D WITH_QT=OFF \
    -D WITH_OPENGL=ON \
    -D WITH_GSTREAMER=ON \
    -D OPENCV_ENABLE_NONFREE=ON \
    -D OPENCV_EXTRA_MODULES_PATH=/root/opencv_build/opencv_contrib/modules \
    -D BUILD_opencv_python2=OFF \
    -D BUILD_opencv_python3=ON \
    -D PYTHON_VERSION=38 \
    -D PYTHON3_EXECUTABLE=/usr/bin/python3.8 \
    -D PYTHON3_PACKAGES_PATH=/usr/lib/python3/dist-packages \
    -D PYTHON3_INCLUDE_DIR=/usr/include/python3.8 \
    -D OPENCV_PYTHON3_INSTALL_PATH=/usr/local/lib/python3.8/dist-packages \
    -D PYTHON3_NUMPY_INCLUDE_DIRS=/usr/local/lib/python3.8/dist-packages/numpy/core/include \
    -D BUILD_EXAMPLES=OFF \
    -D INSTALL_PYTHON_EXAMPLES=OFF \
    -D INSTALL_C_EXAMPLES=OFF \
    ..

# Build OpenCv
RUN cd opencv/build && make -j7
# Install OpenCv
RUN cd opencv/build && make install

# ------------------------------------------------------------------------------------
# LIBTORCH
# ------------------------------------------------------------------------------------
WORKDIR /root/

# Additional libraries
RUN apt-get update && \
    apt-get install -y unzip libboost-all-dev && \
    rm -rf /var/lib/apt/lists/*

# Download libtorch 2.0.0 with for cuda 11.x
RUN wget https://download.pytorch.org/libtorch/cu118/libtorch-cxx11-abi-shared-with-deps-2.0.0%2Bcu118.zip --output-document=libt.zip && \
    unzip libt.zip && rm libt.zip

# ------------------------------------------------------------------------------------
# PANGOLIN
# ------------------------------------------------------------------------------------
WORKDIR /root/

RUN apt-get update && \
    apt-get install -y sudo python3-setuptools python3-wheel libglew-dev  && \
    rm -rf /var/lib/apt/lists/* \

ENV PANGOLIN_HASH "dd801d244db3a8e27b7fe8020cd751404aa818fd"
RUN git clone --recursive https://github.com/stevenlovegrove/Pangolin.git && \
    cd Pangolin && \
    git checkout ${PANGOLIN_HASH}
RUN cd Pangolin && cmake -B build
RUN cd Pangolin/build && make -j4 && make install



# ------------------------------------------------------------------------------------
# LFFS
# ------------------------------------------------------------------------------------

# Additional libraries for downloading
RUN apt-get update && \
    apt-get install -y unzip libboost-all-dev libssl-dev && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /root/
RUN mkdir -p /root/Archive
RUN git clone https://github.com/isarlab-department-engineering/LFFS
WORKDIR /root/LFFS

# IntstAll
RUN sh build.sh


