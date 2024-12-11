// headers/SharedMemoryService.h
#ifndef SHARED_MEMORY_SERVICE_H
#define SHARED_MEMORY_SERVICE_H

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <opencv2/cudaarithm.hpp>


using namespace boost::interprocess;
using namespace cv;
using namespace std;

namespace SharedMemory
{
    class SharedMemoryService
    {
    public:
        SharedMemoryService(char *name);
        ~SharedMemoryService();

        bool initSharedMemory();
        void writeToSharedMemory(const cv::Mat &processedCpuFrame);
        std::vector<unsigned char> readFromSharedMemory();

    private:
        char *m_name;
    };
}

#endif