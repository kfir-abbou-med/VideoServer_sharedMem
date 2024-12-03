#include <opencv2/core.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <iostream>

int main() {
    std::cout << "CUDA Enabled: " << cv::cuda::getCudaEnabledDeviceCount() > 0 << std::endl;
    return 0;
}