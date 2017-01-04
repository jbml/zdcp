// Created by jianingy <detrox@gmail.com>

#ifndef TRIEBASE_H
#define TRIEBASE_H

#include <stdint.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cassert>

#include <vector>
#include <map>
#include <string>
#include <exception>
#include <stdexcept>
#include <iostream>

#include "MMapUtil.h"

typedef void (*OnExploreFinish)(const char*, int, void *);

class TrieBase {
    friend class TrieDebugger;

public: 
    static const int        m_alphabetSize   = 257;
    static const int        m_terminator     = 1;
    static const int        m_magicSize      = 36;
    static const int        m_searchQueueSize= 4096;

    TrieBase(int n=m_defaultStateSize);
    TrieBase(const char *filename);
    virtual ~TrieBase()
    {
        if (m_mmap) {
            delete m_mmap;
        } else {
            delete m_header;
            free(m_state);
        }
    }

    void explore(OnExploreFinish cb, void *arg)
    {
        onExplore(cb, arg, 1, 0);
    }

    int getMaxValue()
    {
        return m_header->maxValue;
    }

    int getMinValue()
    {
        return m_header->minValue;
    }

    int64_t getSumValue()
    {
        return m_header->sumValue;
    }

    void insert(const char *key, int val);
    int search(const char *key);
    int prefixSearch(const char *key, std::map<std::string, int> &map);
    void save(const char *filename);

protected:

#pragma pack(push, 8)
    // alignment by 8 bytes
    typedef struct {
        // Use magicSize to make sure this struct is aligned by 8 bytes.
        char            magic[m_magicSize];
        int             size;
        int             maxValue;
        int             minValue;
        int64_t         sumValue;
    } Header;

    typedef struct {
        int base;
        int check;
    } State;

#pragma pack(pop)

    static const int        m_defaultStateSize   = 1024;
    static const int        m_exploreBufferSize  = 1024;
    int                     m_lastAvailState;
    static char             m_magic[m_magicSize];

    Header                  *m_header;
    State                   *m_state;
    MMapUtil                *m_mmap;
    char                    m_exploreBuffer[m_exploreBufferSize];

    std::vector<std::pair<int, int> >    m_searchQueue;

    int key2state(int ch)
    {
        return (unsigned int)ch + 1;
    }

    int state2key(int s)
    {
        return s - 1;
    }

    int getBase(int s)
    {
        return m_state[s].base;
    }

    int getCheck(int s)
    {
        return m_state[s].check;
    }

    void setBase(int s, int val)
    {
        m_state[s].base = val;
    }

    void setCheck(int s, int val)
    {
        m_state[s].check = val;
    }

    int getNext(int s, int ch)
    {
        int in = key2state(ch);

        if (s + in >= m_header->size)
            inflate(in);

        return getBase(s) + in;
    }

    int getForward(int s, int ch)
    {
        int t = getNext(s, ch);

        return (t > 0 && t < m_header->size && getCheck(t) == s)?t:0;
    }

    void inflate(int expandSize)
    {
        int newSize;

        newSize = (((m_header->size + expandSize) >> 12) + 1) << 12; // align for 4096

        if (!(m_state = (State *)realloc(m_state, newSize * sizeof(State))))
            throw std::bad_alloc();

        memset(m_state  + m_header->size, 0, (newSize - m_header->size) * sizeof(State));
        m_header->size = newSize;
    }

    int findAccepts(int s, int *inputs, int *max, int *min)
    {
        int ch;
        int *p;

        for (ch = 0, p = inputs; ch < m_alphabetSize; ch++) {
            if (getForward(s, ch)) {
                *(p++) = ch;
                if (max && (ch > *max || *max == 0)) *max = ch;
                if (min && (ch < *min || *min == 0)) *min = ch;
            }
        }

        *p = -1;

        return p - inputs;
    }

    void updateStat(int val)
    {
        m_header->maxValue = (val > m_header->maxValue)?val:m_header->maxValue;
        m_header->minValue = (val < m_header->minValue)?val:m_header->minValue;
        m_header->sumValue += val;
    }

    virtual void
    onExploreFinish(OnExploreFinish cb, void *arg, int s, int off) 
    {
        cb(m_exploreBuffer, getBase(s), arg);
    }
    virtual void
    onPrefixSearchFinish(const char *kp, char *buffer, int start, int s,
                         std::map<std::string, int> &map)
    {
        buffer[start] = '\0';
        map[buffer] = getBase(s);
    }

    int findBase(int *key, int max, int min);
    int relocate(int stand, int s, int *key, int max, int min);
    int createTransition(int s, int ch);
    void onExplore(OnExploreFinish cb, void *arg, int s, int off);

private:
    TrieBase(TrieBase &) 
    {
    
    }

    TrieBase &operator=(TrieBase &)
    {
        return *this;
    }
};

#endif // TRIEBASE_H

// vim: ts=4 sw=4 cindent et
