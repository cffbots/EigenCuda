#include <memory_manager.hpp>
#include <sstream>

namespace eigencuda {

Index count_available_gpus() {
  int count;
  cudaError_t err = cudaGetDeviceCount(&count);
  return 0 ? (err != cudaSuccess) : Index(count);
}

Unique_ptr_to_GPU_data alloc_tensor_in_gpu(size_t size_tensor) {
  double *dtensor;
  throw_if_not_enough_memory_in_gpu(size_tensor);
  checkCuda(cudaMalloc(&dtensor, size_tensor));
  Unique_ptr_to_GPU_data dev_ptr(dtensor,
                                 [](double *x) { checkCuda(cudaFree(x)); });
  return dev_ptr;
}

void throw_if_not_enough_memory_in_gpu(size_t requested_memory) {
  size_t free, total;
  checkCuda(cudaMemGetInfo(&free, &total));

  std::stringstream stream;
  stream << "There were requested : " << requested_memory
         << "bytes Index the device\n";
  stream << "Device Free memory (bytes): " << free
         << "\nDevice total Memory (bytes): " << total << "\n";

  // Raise an error if there is not enough total or free memory in the device
  if (requested_memory > free) {
    stream << "There is not enough memory in the Device!\n";
    throw std::runtime_error(stream.str());
  }
}

}  // namespace eigencuda
