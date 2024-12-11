#include "headers/SharedMemoryService.h"

namespace SharedMemory
{
    SharedMemoryService::SharedMemoryService(char *name) : m_name(name)
    {
        // shared_memory_object shm(open_only, m_shmName.c_str(), read_only);
    }

    SharedMemoryService::~SharedMemoryService()
    {
    }
}