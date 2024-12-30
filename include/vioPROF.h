#pragma once

#define vioPROF_enable

#ifdef vioPROF_enable

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <cuda_runtime.h>
#include <map>


/*

// show memory usage of GPU

        size_t free_byte ;

        size_t total_byte ;

        cuda_status = cudaMemGetInfo( &free_byte, &total_byte ) ;

        if ( cudaSuccess != cuda_status ){

            printf("Error: cudaMemGetInfo fails, %s \n", cudaGetErrorString(cuda_status) );

            exit(1);

        }

double free_db = (double)free_byte ;

        double total_db = (double)total_byte ;

        double used_db = total_db - free_db ;

        printf("GPU memory usage: used = %f, free = %f MB, total = %f MB\n",

            used_db/1024.0/1024.0, free_db/1024.0/1024.0, total_db/1024.0/1024.0);
*/
namespace vio { 

class Profiler {

    uint64_t startTs = 0;
    uint64_t endTs = 0;
    size_t start_mem, end_mem;
    cudaEvent_t startCu, stopCu;

    std::string csv_times_path = ".";
    std::string csv_gpu_path = ".";
    std::string csv_gpu_CUDA_path = ".";
    std::ofstream csv_file_times;
    std::ofstream csv_file_gpu_stats;
    std::ofstream csv_file_gpu_CUDA_stats;

    public:

    Profiler(const std::string &name) {
        // saved on RAM to speedup
        csv_times_path = "/tmp/vioProf_" + name +  "_TIMESSTAT.csv"; 
        csv_gpu_path = "/tmp/vioProf_" + name + "_GPUSTATS.csv"; 
        csv_gpu_CUDA_path = "/tmp/vioProf_" + name + "_CUDASTATS.csv"; 
    }

    void checkCudaError(cudaError_t error) {
        if (error != cudaSuccess) {
            std::cerr << "CUDA Error: " << cudaGetErrorString(error) << std::endl;
            // exit(-1);
        }
    }

    void gpuTic() {

        start_mem = 0;
        end_mem = 0;
        // Record initial GPU memory usage
        checkCudaError(cudaMemGetInfo(&start_mem, nullptr));
    
        // Create CUDA events for timing
        checkCudaError(cudaEventCreate(&startCu));
        checkCudaError(cudaEventCreate(&stopCu));
    
        // Record start event
        checkCudaError(cudaEventRecord(startCu, 0));
    }  

    void gpuToc(char const * caller_name,char const * caller_file) {
        // Record stop event
        checkCudaError(cudaEventRecord(stopCu, 0));
        checkCudaError(cudaEventSynchronize(stopCu));
    
        // Record GPU memory usage after executing forward pass
        checkCudaError(cudaMemGetInfo(&end_mem, nullptr));
    
        // Calculate elapsed time
        float elapsedTime = 0;
        checkCudaError(cudaEventElapsedTime(&elapsedTime, startCu, stopCu));
        double used_mem = 0;
        if(start_mem > end_mem)
        {
            used_mem = (double)start_mem - (double)end_mem;
        }
        // std::cout << "GPU end_mem " << end_mem << " bytes" << std::endl;
        // std::cout << "GPU start_mem: " << start_mem << " bytes" << std::endl;
        // std::cout << "GPU Memory Used: " << used_mem << " bytes" << std::endl;
        // std::cout << "GPU Time Used: " << elapsedTime << " ms" << std::endl;

        if(!csv_file_gpu_CUDA_stats.is_open()) {
            csv_file_gpu_CUDA_stats.open(csv_gpu_CUDA_path);
            csv_file_gpu_CUDA_stats<<"#caller_method,gpu_memory_free(MB),diff_gpu_memory_used(MB),GPU Time Used(ms)\n";
        }
        csv_file_gpu_CUDA_stats<<"["<<caller_name<<"@"<<caller_file<<"] "<<end_mem/1024.0/1024.0<<","<<used_mem/1024.0/1024.0<<","<<elapsedTime<<"\n";
        csv_file_gpu_CUDA_stats.flush();
    
        // Clean up events
        checkCudaError(cudaEventDestroy(startCu));
        checkCudaError(cudaEventDestroy(stopCu));
    } 

    void saveGpuStats(char const * caller_name,char const * caller_file) {
        if(!csv_file_gpu_stats.is_open()) {
            csv_file_gpu_stats.open(csv_gpu_path);
            csv_file_gpu_stats<<"#caller_method,gpu_memory_used(MB),gpu_memory_free(MB),gpu_memory_total(MB)\n";
        }

        size_t free_byte ;
        size_t total_byte ;

        cudaError_t cuda_status = cudaSetDevice(0); // Assuming device 0
        cuda_status = cudaMemGetInfo( &free_byte, &total_byte ) ;

        if ( cudaSuccess != cuda_status ){
            printf("Error: cudaMemGetInfo fails, %s \n", cudaGetErrorString(cuda_status) );
            // exit(1);
        }

        double free_db = (double)free_byte ;
        double total_db = (double)total_byte ;
        double used_db = total_db - free_db ;

        csv_file_gpu_stats<<"["<<caller_name<<"@"<<caller_file<<"] "<<used_db/1024.0/1024.0<<","<<free_db/1024.0/1024.0<<","<<total_db/1024.0/1024.0<<"\n";
        csv_file_gpu_stats.flush();
        // printf("GPU memory usage: used = %f, free = %f MB, total = %f MB\n",
        //     used_db/1024.0/1024.0, free_db/1024.0/1024.0, total_db/1024.0/1024.0);
    }

    void tic() {
        struct timespec tp;
        clock_gettime(CLOCK_REALTIME, &tp);
        startTs = ((uint64_t)tp.tv_sec)*1e9 + tp.tv_nsec;  
    }

    void toc(char const * caller_name,char const * caller_file) {
        struct timespec tp;
        clock_gettime(CLOCK_REALTIME, &tp);
        uint64_t ts = ((uint64_t)tp.tv_sec)*1e9 + tp.tv_nsec; 
        if(startTs == 0) {
            std::cerr<<"must tic() before toc()\n";
            exit(-1);
        }
        endTs = ts;

        if(!csv_file_times.is_open()) {
            csv_file_times.open(csv_times_path);
            csv_file_times<<"#caller_method,start_timestamp(ns),end_timestamp(ns),diff_timestamp(ns)\n";
        }
        csv_file_times<<"["<<caller_name<<"@"<<caller_file<<"] "<<startTs<<","<<endTs<<","<<endTs-startTs<<"\n";
        csv_file_times.flush();
    }

};

}

#define vioPROF_tic(name) static vio::Profiler prof_##name(#name); prof_##name.tic();
#define vioPROF_toc(name) prof_##name.toc();

#else
#define vioPROF_tic(name) ;
#define vioPROF_toc(name) ;
#endif