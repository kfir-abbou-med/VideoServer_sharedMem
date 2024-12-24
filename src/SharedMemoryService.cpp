#include "headers/SharedMemoryService.h"
#include <iostream>
#include <QDebug>

using namespace std;

namespace SharedMemory
{
    SharedMemoryService::SharedMemoryService(char *name) : m_name(name)
    {
        // shared_memory_object shm(open_only, m_shmName.c_str(), read_only);
    }

    SharedMemoryService::~SharedMemoryService()
    {
    }

    bool SharedMemoryService::initSharedMemory()
    {
        try
        {
            cout << "[SharedMemoryService::initSharedMemory]" << endl;
            // Calculate exact frame size - TBD read from config
            const int width = 640;
            const int height = 480;
            const size_t channelSize = width * height;
            const size_t totalSize = channelSize * 3; // RGB channels

            // Create shared memory with precise size
            m_shm = boost::interprocess::shared_memory_object(
                boost::interprocess::open_or_create,
                m_name,
                boost::interprocess::read_write);

            // Truncate to exact frame size
            m_shm.truncate(totalSize);

            boost::interprocess::mapped_region m_shmRegion = boost::interprocess::mapped_region(m_shm, boost::interprocess::read_write);
            m_sharedMemory = m_shmRegion.get_address();
            return true;
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
            return false;
        }
    }

    void SharedMemoryService::writeToSharedMemory(const cv::Mat &processedCpuFrame)
    {
        size_t frameSize = processedCpuFrame.total() * processedCpuFrame.elemSize();
        if (frameSize <= m_shmRegion.get_size())
        {
            std::memcpy(m_sharedMemory, processedCpuFrame.data, frameSize);
        }
        else
        {
            qDebug() << "Frame size exceeds shared memory size!";
        }
    }

    std::vector<unsigned char> SharedMemoryService::readFromSharedMemory()
    {
    }

}