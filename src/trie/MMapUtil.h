/* Created by jianingy */

#ifndef MMAPUTIL_H
#define MMAPUTIL_H

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>

#include <cstring>
#include <cassert>
#include <stdexcept>
#include "common/common.h"

class MMapUtil {

protected:
    void        *m_start;
    int         m_size;

public:
    MMapUtil()
        :m_start(NULL), m_size(0) 
    {
    }

    MMapUtil(const char *filename)
        :m_start(NULL), m_size(0)
    {
        struct stat         sb;
        int                 fd;
        int                 retval;

        if ((fd = open(filename, O_RDONLY)) < 0)
            THROW_RUNTIME_ERROR(string(strerror(errno))+":"+filename);

        if (fstat(fd, &sb) < 0)
            THROW_RUNTIME_ERROR(strerror(errno));

        m_start = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

        if (m_start == MAP_FAILED)
            THROW_RUNTIME_ERROR(strerror(errno));

        while (retval = close(fd), retval == -1 && errno == EINTR)
            /* empty loop */;

        m_size = sb.st_size;
    }

    ~MMapUtil()
    {
        if (m_start) {
            if (munmap(m_start, m_size) < 0) 
                THROW_RUNTIME_ERROR(strerror(errno));
        }
    }

    void *start(int offset=0)
    {
        return (void *)((char *)m_start + offset);
    }

    bool isMapped()
    {
        return m_start;
    }
};

#endif // - MMAPUTIL_H

// vim: ts=4 sw=4 cindent et
