/**
* Software License Agreement (BSD License)
*
*  Copyright (c) 2009, Willow Garage, Inc.
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above
*     copyright notice, this list of conditions and the following
*     disclaimer in the documentation and/or other materials provided
*     with the distribution.
*   * Neither the name of the Willow Garage nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*
*/

#include <SPDetector.hpp>

namespace SuperPointSLAM
{


void NMS(cv::Mat det, cv::Mat conf, cv::Mat desc, std::vector<cv::KeyPoint>& pts, cv::Mat& descriptors,
        int border, int dist_thresh, int img_width, int img_height);
void NMS2(std::vector<cv::KeyPoint> det, cv::Mat conf, std::vector<cv::KeyPoint>& pts,
            int border, int dist_thresh, int img_width, int img_height);

// SPDetector::SPDetector(std::shared_ptr<SuperPoint::SuperPoint> _model) : model(_model) 
// {
// }

SPDetector::SPDetector(std::string _weight_dir, bool _use_cuda)
    :   mDeviceType((_use_cuda) ? c10::kCUDA : c10::kCPU),
        mDevice(c10::Device(mDeviceType))
{   
    /* SuperPoint model loading */
    model = std::make_shared<SuperPoint>();
    torch::load(model, _weight_dir);
    
    /* This option should be done exactly as below */
    tensor_opts = c10::TensorOptions()
                        .dtype(torch::kFloat32)
                        .layout(c10::kStrided)
                        .requires_grad(false);

    // bool is_cuda_available = _use_cuda && torch::cuda::is_available();
    std::cout << "Running Learned Features For SLAM..." << std::endl;
    if (_use_cuda)
    {
        model->to(mDevice);
        std::cout << "Model loaded into GPU device!" << std::endl;
    
    }
    model->eval();
}
/*
void SPDetector::detect(cv::InputArray _image, std::vector<cv::KeyPoint>& _keypoints,
                      cv::Mat &_descriptors)
{
    cv::Mat img = _image.getMat();
    at::Tensor x = torch::from_blob((void*)img.clone().data, \
                                    {1, 1, img.rows, img.cols}, \
                                    tensor_opts).to(mDevice);
    
    // To avoid Error caused by division by zero.
    // "EPSILON" is mostly used for this purpose.
    x = (x + EPSILON) / 255.0; 

    model->forward(x, mProb, mDesc);
    mProb = mProb.squeeze(0);

    //Return a "CUDA bool type Tensor"
    // 1 if there is a featrue, and 0 otherwise 
    at::Tensor kpts = (mProb > mConfThres);  
    
    // Remove potential redundent features. 
    if(nms) 
    {   // Default=true
        SemiNMS(kpts);
    }

    // Prepare grid_sampler() 
    kpts = at::nonzero(kpts); // [N, 2] (y, x)               
    at::Tensor fkpts = kpts.to(kFloat);
    at::Tensor grid = torch::zeros({1, 1, kpts.size(0), 2}).to(mDevice); 
    // grid.print(); // [CUDAFloatType [1, 1, 225, 2]]

    // mProb size(1): W - cols - 320, size(0): H - rows - 240

    // Get each Keypoints' descriptor. 
    grid[0][0].slice(1, 0, 1) = (2.0 * (fkpts.slice(1, 1, 2) / mProb.size(1))) - 1; // x
    grid[0][0].slice(1, 1, 2) = (2.0 * (fkpts.slice(1, 0, 1) / mProb.size(0))) - 1; // y
    mDesc = at::grid_sampler(mDesc, grid, 0, 0, false);    // [1, 256, 1, n_keypoints]
    mDesc = mDesc.squeeze(0).squeeze(1);                  // [256, n_keypoints]

    // Normalize 1-Dimension with 2-Norm. 
    at::Tensor dn = at::norm(mDesc, 2, 1);          // [CUDAFloatType [256]]
    mDesc = at::div((mDesc + EPSILON), unsqueeze(dn, 1));
    //mDesc = mDesc.div(unsqueeze(dn, 1));          // [256, n_keypoints] <- unsqueeezed dn[CUDAFloatType [256, 1]]
    mDesc = mDesc.transpose(0, 1).contiguous();     // [CUDAFloatType [N, 256]]
    
    // After processing, back to CPU only descriptor
    if (mDeviceType == c10::kCUDA)
        mDesc = mDesc.to(kCPU);

    // Convert descriptor From at::Tensor To cv::Mat  
    cv::Size desc_size(mDesc.size(1), mDesc.size(0)); 
    n_keypoints = mDesc.size(0); 
    
    // [256, N], CV_32F
    _descriptors.create(n_keypoints, 256, CV_32FC1);
    memcpy((void*)_descriptors.data, mDesc.data_ptr(), sizeof(float) * mDesc.numel());
    // descriptors = cv::Mat(desc_size, CV_32FC1, mDesc.data_ptr<float>());
    // std::cout << _descriptors.row(0) << std::endl;

    // Convert Keypoint
    // From torch::Tensor   kpts(=keypoints)
    // To   cv::KeyPoint    keypoints_no_nms 
    _keypoints.clear();
    _keypoints.reserve(n_keypoints); 
    for (int i = 0; i < n_keypoints; i++)
    {
        float x = kpts[i][1].item<float>(), y = kpts[i][0].item<float>();
        float conf = mProb[kpts[i][0]][kpts[i][1]].item<float>();
        _keypoints.push_back(cv::KeyPoint(cv::Point((int)x, (int)y), 1.0, 0.0, conf));
    }
    
    mProb.reset();
    mDesc.reset();
}
*/
void SPDetector::SemiNMS(at::Tensor& kpts)
{
    if (mDeviceType == c10::kCUDA)
        kpts = kpts.to(kCPU);
    // std::cout << kpts.scalar_type() << sizeof(kpts.scalar_type()) << std::endl;
    // NMS alternative
    int rowlen = kpts.size(0);
    int collen = kpts.size(1);

    //auto accessor = kpts.accessor<bool,2>();
    auto pT1 = kpts.data_ptr<bool>();
    auto pT2 = pT1 + collen;
    // auto pT3 = pT2 + collen;

    for(int i = 0; i < rowlen; i++)
    {
        for(int j = 0 ; j < collen; j++)
        {
            if(*pT1 && (i < rowlen-1) && (j < collen-1))
            {
                *(pT1 + 1) = 0;             // *(pT1 + 2) = 0;
                *pT2 = 0; *(pT2 + 1) = 0;   // *(pT2 + 2) = 0; 
                //*pT3 = 0; *(pT3 + 1) = 0; *(pT3 + 2) = 0; 
            }
            pT1++;
            pT2++;
            // pT3++;
        }
    }

    if (mDeviceType == c10::kCUDA)
        kpts = kpts.to(kCUDA);
}


void SPDetector::detect(const cv::Mat &img, bool cuda)
{

    auto x = torch::from_blob(img.clone().data, {1, 1, img.rows, img.cols}, torch::kByte).clone();
    x = x.to(torch::kFloat) / 255;

    // bool use_cuda = cuda && torch::cuda::is_available();
    torch::DeviceType device_type;
    if (cuda)
        device_type = torch::kCUDA;
    else
        device_type = torch::kCPU;

    torch::Device device(device_type);

    // this->model->to(device);
    x = x.set_requires_grad(false);

    auto out = this->model->forward(x.to(device));
    
    torch::Device device_cpu(torch::kCPU);

    out[0] = out[0].to(device_cpu);
    out[1] = out[1].to(device_cpu);
    
    mProb = out[0].squeeze(0);  // [H, W]

    mDesc = out[1];             // [1, 256, H/8, W/8]

    // printf("%s \n" , __PRETTY_FUNCTION__);
    // std::cout << __PRETTY_FUNCTION__ << " IN " <<  x.sizes() << std::endl;

    // std::cout << __PRETTY_FUNCTION__ << " OUT " <<  out[0].sizes() << std::endl;
    // std::cout << __PRETTY_FUNCTION__ << " OUT " <<  out[1].sizes() << std::endl;

    // printf("%d, %d, %d \n", out[0].sizes()[0], mProb.sizes()[1], mProb.sizes()[2]);
    // printf("%d, %d, %d \n", mDesc.sizes()[0], mDesc.sizes()[1], mDesc.sizes()[2]);

}

// void SPDetector::extractFirstNPoints( ,int num_points)
// {

// }

void SPDetector::getKeyPoints(float threshold, int iniX, int maxX, int iniY, int maxY, std::vector<cv::KeyPoint> &keypoints, bool nms)
{
    //TIC
    auto prob = mProb.slice(0, iniY, maxY).slice(1, iniX, maxX);  // [h, w]
    // if(use_threshold)
    // {
    //     auto kpts = (prob > threshold);
    // }
    // else
    // {
    
    auto kpts = (prob > threshold);
        
    // }
    
    kpts = torch::nonzero(kpts);  // [n_keypoints, 2]  (y, x)

    std::vector<cv::KeyPoint> keypoints_no_nms;
    for (int i = 0; i < kpts.size(0); i++) {
        float response = prob[kpts[i][0]][kpts[i][1]].item<float>();
        keypoints_no_nms.push_back(cv::KeyPoint(kpts[i][1].item<float>(), kpts[i][0].item<float>(), 8, -1, response));
    }

    //TOC
    //TIC

    if (nms) {
        cv::Mat conf(keypoints_no_nms.size(), 1, CV_32F);
        for (size_t i = 0; i < keypoints_no_nms.size(); i++) {
            int x = keypoints_no_nms[i].pt.x;
            int y = keypoints_no_nms[i].pt.y;
            conf.at<float>(i, 0) = prob[y][x].item<float>();
        }

        // cv::Mat descriptors;

        int border = 0;
        int dist_thresh = 4;
        int height = maxY - iniY;
        int width = maxX - iniX;

        NMS2(keypoints_no_nms, conf, keypoints, border, dist_thresh, width, height);
    }
    else {
        keypoints = keypoints_no_nms;
    }

    //TOC
}

// void SPDetector::getKeyPoints(float threshold, int iniX, int maxX, int iniY, int maxY, const int& num_keypoints, std::vector<cv::KeyPoint> &keypoints, bool nms)
// {
//     TIC
//     auto prob = mProb.clone();  // [h, w]
//     TOC
//     prob = prob.slice(0, iniY, maxY).slice(1, iniX, maxX); 
//     auto res = torch::topk(prob.flatten(), num_keypoints);
//     // std::get<0>(res) = std::get<0>(res).to(torch::kCPU);
//     // std::get<0>(res) = std::get<1>(res).to(torch::kCPU);

//     TOC

//     // std::cout << std::get<0>(res) << "\n\n\n"; 
//     // std::cout << std::get<1>(res) << "\n\n\n"; 
//     // std::cout << prob.sizes() << "\n\n\n"; 
//     // std::cout << std::get<1>(res)[0].item<int>() << std::endl;

//     // Unravel index
//     // int rows = 0; 
//     // int cols = 0; 
//     int nrows = prob.size(0);
//     int ncols = prob.size(1);

//     auto rows = std::get<1>(res).floor_divide(ncols);
//     auto cols = std::get<1>(res) % ncols;

//     TOC

//     std::vector<cv::KeyPoint> keypoints_no_nms;
//     for (int i = 0; i < num_keypoints; i++) {
//         // rows = std::get<1>(res)[i].item<int>() / ncols;
//         // cols = std::get<1>(res)[i].item<int>() % ncols;
//         // std::cout << rows << " " << cols << std::endl;
//         // std::cout << "Keypoint value: " <<  << "Keypoint position" << std::endl;
//         // float response = prob[rows][cols].item<float>();
//         float response = std::get<0>(res)[i].item<float>();
//         keypoints_no_nms.push_back(cv::KeyPoint(cols[i].item<int>(), rows[i].item<int>(), 8, -1, response));
//     }

//     TOC
//     // TIC

//     if (nms) {
//         cv::Mat conf(keypoints_no_nms.size(), 1, CV_32F);
//         for (size_t i = 0; i < keypoints_no_nms.size(); i++) {
//             int x = keypoints_no_nms[i].pt.x;
//             int y = keypoints_no_nms[i].pt.y;
//             conf.at<float>(i, 0) = prob[y][x].item<float>();
//         }

//         // cv::Mat descriptors;

//         int border = 0;
//         int dist_thresh = 4;
//         int height = nrows;
//         int width = ncols;

//         NMS2(keypoints_no_nms, conf, keypoints, border, dist_thresh, width, height);
//     }
//     else {
//         keypoints = keypoints_no_nms;
//     }

//     // TOC
// }

void SPDetector::getKeyPoints(const int& num_keypoints, std::vector<cv::KeyPoint> &keypoints, bool nms)
{
    // TIC
    auto prob = mProb.clone();  // [h, w]

    auto res = torch::topk(prob.flatten(), num_keypoints);
    // std::get<0>(res) = std::get<0>(res).to(torch::kCPU);
    // std::get<0>(res) = std::get<1>(res).to(torch::kCPU);

    // TOC

    // std::cout << std::get<0>(res) << "\n\n\n"; 
    // std::cout << std::get<1>(res) << "\n\n\n"; 
    // std::cout << prob.sizes() << "\n\n\n"; 
    // std::cout << std::get<1>(res)[0].item<int>() << std::endl;

    // Unravel index
    // int rows = 0; 
    // int cols = 0; 
    int nrows = prob.size(0);
    int ncols = prob.size(1);

    auto rows = std::get<1>(res).floor_divide(ncols);
    auto cols = std::get<1>(res) % ncols;

    // TOC

    std::vector<cv::KeyPoint> keypoints_no_nms;
    for (int i = 0; i < num_keypoints; i++) {
        // rows = std::get<1>(res)[i].item<int>() / ncols;
        // cols = std::get<1>(res)[i].item<int>() % ncols;
        // std::cout << rows << " " << cols << std::endl;
        // std::cout << "Keypoint value: " <<  << "Keypoint position" << std::endl;
        // float response = prob[rows][cols].item<float>();
        float response = std::get<0>(res)[i].item<float>();
        keypoints_no_nms.push_back(cv::KeyPoint(cols[i].item<int>(), rows[i].item<int>(), 8, -1, response));
    }

    // TOC
    // TIC

    if (nms) {
        cv::Mat conf(keypoints_no_nms.size(), 1, CV_32F);
        for (size_t i = 0; i < keypoints_no_nms.size(); i++) {
            int x = keypoints_no_nms[i].pt.x;
            int y = keypoints_no_nms[i].pt.y;
            conf.at<float>(i, 0) = prob[y][x].item<float>();
        }

        // cv::Mat descriptors;

        int border = 0;
        int dist_thresh = 4;
        int height = nrows;
        int width = ncols;

        NMS2(keypoints_no_nms, conf, keypoints, border, dist_thresh, width, height);
    }
    else {
        keypoints = keypoints_no_nms;
    }

    // TOC
}

void SPDetector::computeDescriptors(const std::vector<cv::KeyPoint> &keypoints, cv::Mat &descriptors, bool use_cuda)
{

    // TIC 

    cv::Mat kpt_mat(keypoints.size(), 2, CV_32F);  // [n_keypoints, 2]  (y, x)

    for (size_t i = 0; i < keypoints.size(); i++) {
        kpt_mat.at<float>(i, 0) = (float)keypoints[i].pt.y;
        kpt_mat.at<float>(i, 1) = (float)keypoints[i].pt.x;
    }

    // auto fkpts = torch::from_blob(kpt_mat.data, {keypoints.size(), 2}, torch::kFloat);
    auto fkpts = torch::from_blob(kpt_mat.data, {static_cast<int64_t>(keypoints.size()), 2}, torch::kFloat);

    auto grid = torch::zeros({1, 1, fkpts.size(0), 2});  // [1, 1, n_keypoints, 2]

    torch::DeviceType device_type;
    if (torch::cuda::is_available() && use_cuda)
        device_type = torch::kCUDA;
    else
        device_type = torch::kCPU;

    torch::Device device(device_type);

    grid = grid.to(device);


    grid[0][0].slice(1, 0, 1) = 2.0 * fkpts.slice(1, 1, 2) / mProb.size(1) - 1;  // x
    grid[0][0].slice(1, 1, 2) = 2.0 * fkpts.slice(1, 0, 1) / mProb.size(0) - 1;  // y

    auto desc = torch::grid_sampler(mDesc, grid, 0, 0, false);  // [1, 256, 1, n_keypoints]
    desc = desc.squeeze(0).squeeze(1);  // [256, n_keypoints]

    // normalize to 1
    auto dn = torch::norm(desc, 2, 1);
    desc = desc.div(torch::unsqueeze(dn, 1));

    desc = desc.transpose(0, 1).contiguous();  // [n_keypoints, 256]
    desc = desc.to(torch::kCPU);

    cv::Mat desc_mat(cv::Size(desc.size(1), desc.size(0)), CV_32FC1, desc.data<float>());

    descriptors = desc_mat.clone();

    
    // printf("%s, Descriptors cols: %d, rows:%d; mDesc 0: %d, 1: %d, 2: %d \n",__PRETTY_FUNCTION__, descriptors.cols, descriptors.rows, mDesc.sizes()[0], mDesc.sizes()[1], mDesc.sizes()[2]);
}


void NMS2(std::vector<cv::KeyPoint> det, cv::Mat conf, std::vector<cv::KeyPoint>& pts,
            int border, int dist_thresh, int img_width, int img_height)
{

    std::vector<cv::Point2f> pts_raw;

    for (int i = 0; i < det.size(); i++){

        int u = (int) det[i].pt.x;
        int v = (int) det[i].pt.y;

        pts_raw.push_back(cv::Point2f(u, v));
    }

    cv::Mat grid = cv::Mat(cv::Size(img_width, img_height), CV_8UC1);
    cv::Mat inds = cv::Mat(cv::Size(img_width, img_height), CV_16UC1);

    cv::Mat confidence = cv::Mat(cv::Size(img_width, img_height), CV_32FC1);

    grid.setTo(0);
    inds.setTo(0);
    confidence.setTo(0);

    for (int i = 0; i < pts_raw.size(); i++)
    {   
        int uu = (int) pts_raw[i].x;
        int vv = (int) pts_raw[i].y;

        grid.at<char>(vv, uu) = 1;
        inds.at<unsigned short>(vv, uu) = i;

        confidence.at<float>(vv, uu) = conf.at<float>(i, 0);
    }
    
    cv::copyMakeBorder(grid, grid, dist_thresh, dist_thresh, dist_thresh, dist_thresh, cv::BORDER_CONSTANT, 0);

    for (int i = 0; i < pts_raw.size(); i++)
    {   
        int uu = (int) pts_raw[i].x + dist_thresh;
        int vv = (int) pts_raw[i].y + dist_thresh;

        if (grid.at<char>(vv, uu) != 1)
            continue;

        for(int k = -dist_thresh; k < (dist_thresh+1); k++)
            for(int j = -dist_thresh; j < (dist_thresh+1); j++)
            {
                if(j==0 && k==0) continue;

                if ( confidence.at<float>(vv + k, uu + j) < confidence.at<float>(vv, uu) )
                    grid.at<char>(vv + k, uu + j) = 0;
                
            }
        grid.at<char>(vv, uu) = 2;
    }

    size_t valid_cnt = 0;
    std::vector<int> select_indice;

    for (int v = 0; v < (img_height + dist_thresh); v++){
        for (int u = 0; u < (img_width + dist_thresh); u++)
        {
            if (u -dist_thresh>= (img_width - border) || u-dist_thresh < border || v-dist_thresh >= (img_height - border) || v-dist_thresh < border)
            continue;

            if (grid.at<char>(v,u) == 2)
            {
                int select_ind = (int) inds.at<unsigned short>(v-dist_thresh, u-dist_thresh);
                cv::Point2f p = pts_raw[select_ind];
                float response = conf.at<float>(select_ind, 0);
                pts.push_back(cv::KeyPoint(p, 8.0f, -1, response));

                select_indice.push_back(select_ind);
                valid_cnt++;
            }
        }
    }
    
    // descriptors.create(select_indice.size(), 256, CV_32F);

    // for (int i=0; i<select_indice.size(); i++)
    // {
    //     for (int j=0; j < 256; j++)
    //     {
    //         descriptors.at<float>(i, j) = desc.at<float>(select_indice[i], j);
    //     }
    // }
}

void NMS(cv::Mat det, cv::Mat conf, cv::Mat desc, std::vector<cv::KeyPoint>& pts, cv::Mat& descriptors,
        int border, int dist_thresh, int img_width, int img_height)
{

    std::vector<cv::Point2f> pts_raw;

    for (int i = 0; i < det.rows; i++){

        int u = (int) det.at<float>(i, 0);
        int v = (int) det.at<float>(i, 1);
        // float conf = det.at<float>(i, 2);

        pts_raw.push_back(cv::Point2f(u, v));
    }

    cv::Mat grid = cv::Mat(cv::Size(img_width, img_height), CV_8UC1);
    cv::Mat inds = cv::Mat(cv::Size(img_width, img_height), CV_16UC1);

    cv::Mat confidence = cv::Mat(cv::Size(img_width, img_height), CV_32FC1);

    grid.setTo(0);
    inds.setTo(0);
    confidence.setTo(0);

    for (int i = 0; i < pts_raw.size(); i++)
    {   
        int uu = (int) pts_raw[i].x;
        int vv = (int) pts_raw[i].y;

        grid.at<char>(vv, uu) = 1;
        inds.at<unsigned short>(vv, uu) = i;

        confidence.at<float>(vv, uu) = conf.at<float>(i, 0);
    }
    
    cv::copyMakeBorder(grid, grid, dist_thresh, dist_thresh, dist_thresh, dist_thresh, cv::BORDER_CONSTANT, 0);

    for (int i = 0; i < pts_raw.size(); i++)
    {   
        int uu = (int) pts_raw[i].x + dist_thresh;
        int vv = (int) pts_raw[i].y + dist_thresh;

        if (grid.at<char>(vv, uu) != 1)
            continue;

        for(int k = -dist_thresh; k < (dist_thresh+1); k++)
            for(int j = -dist_thresh; j < (dist_thresh+1); j++)
            {
                if(j==0 && k==0) continue;

                if ( conf.at<float>(vv + k, uu + j) < conf.at<float>(vv, uu) )
                    grid.at<char>(vv + k, uu + j) = 0;
                
            }
        grid.at<char>(vv, uu) = 2;
    }

    size_t valid_cnt = 0;
    std::vector<int> select_indice;

    for (int v = 0; v < (img_height + dist_thresh); v++){
        for (int u = 0; u < (img_width + dist_thresh); u++)
        {
            if (u -dist_thresh>= (img_width - border) || u-dist_thresh < border || v-dist_thresh >= (img_height - border) || v-dist_thresh < border)
            continue;

            if (grid.at<char>(v,u) == 2)
            {
                int select_ind = (int) inds.at<unsigned short>(v-dist_thresh, u-dist_thresh);
                pts.push_back(cv::KeyPoint(pts_raw[select_ind], 1.0f));

                select_indice.push_back(select_ind);
                valid_cnt++;
            }
        }
    }
    
    descriptors.create(select_indice.size(), 256, CV_32F);

    for (int i=0; i<select_indice.size(); i++)
    {
        for (int j=0; j < 256; j++)
        {
            descriptors.at<float>(i, j) = desc.at<float>(select_indice[i], j);
        }
    }
}


} //END namespace SuperPointSLAM
