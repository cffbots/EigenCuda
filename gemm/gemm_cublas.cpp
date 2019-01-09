#include <cstdlib>
#include <Eigen/Dense>
#include <Eigen/Core>
#include <stdlib.h>
#include <iostream>
#include <cublas_v2.h>
#include <chrono>
#include <curand.h>
#include <cxxopts.hpp>

//col Major for CUDA
typedef Eigen::Matrix<float,Eigen::Dynamic,Eigen::Dynamic,Eigen::ColMajor> Mat;

Mat cublas_gemm(Mat A, Mat B)
{

    const float alpha = 1.;
    const float beta = 0.;
    const float *pa = &alpha;
    const float *pb = &beta;

    int size = A.cols();
    Mat C = Mat::Zero(size,size);

    // and their pointers
    float *hA = A.data();
    float *hB = B.data();
    float *hC = C.data();

    // alloc memory on the GPU
    float *dA, *dB, *dC;
    cudaMalloc(&dA,size*size*sizeof(float));
    cudaMalloc(&dB,size*size*sizeof(float));
    cudaMalloc(&dC,size*size*sizeof(float));

    // cuda handle
    cublasHandle_t handle;
    cublasCreate(&handle);

    // Transfer data to GPU
    cudaMemcpy(dA,hA,size*size*sizeof(float),cudaMemcpyHostToDevice);
    cudaMemcpy(dB,hB,size*size*sizeof(float),cudaMemcpyHostToDevice);

    // process on GPU
    cublasSgemm(handle,CUBLAS_OP_N, CUBLAS_OP_N,size,size,size,pa,dA,size,dB,size,pb,dC,size);
    //gpu_blas_gemm(handle,dA,dB,dC,size);

    // send data back to CPU
    cudaMemcpy(hC,dC,size*size*sizeof(float),cudaMemcpyDeviceToHost);
    
    // create an eigen matrix
    C = Eigen::Map<Mat>(hC,size,size);

    // free memory
    cublasDestroy(handle);
    cudaFree(dA);
    cudaFree(dB);
    cudaFree(dC);

    return C;

}

int main(int argc, char *argv[]) {
    
    // parse the input
    cxxopts::Options options(argv[0],  "gemm example using eigen");
    options.positional_help("[optional args]").show_positional_help();
    options.add_options()
        ("size", "dimension of the matrix", cxxopts::value<std::string>()->default_value("100"));

    auto result = options.parse(argc,argv);
    int size = std::stoi(result["size"].as<std::string>(),nullptr);

    // Create CPU matrices
    Mat A = Mat::Random(size,size);
    Mat B = Mat::Random(size,size);

    // chrono    
    std::chrono::time_point<std::chrono::system_clock> start, end;

    start = std::chrono::system_clock::now();
    Mat C = cublas_gemm(A, B);

    // outputs
    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_time = end-start;
    std::cout << "Run time    : " << elapsed_time.count() << " secs" <<  std::endl;

    return 0;
}