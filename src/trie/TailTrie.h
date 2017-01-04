// Created by jianingy <detrox@gmail.com>

#ifndef TAILTRIE_H
#define TAILTRIE_H

#include "TrieBase.h"


class TailTrie: public TrieBase {
    friend class TrieDebugger;

private:
    TailTrie(TailTrie &trie)
    {
    }

protected:

#pragma pack(push, 8)
    typedef struct {
        int size;
        int last;
    } TailHeader;

    typedef struct {
        int *data;
        int length;
    } Tail;
#pragma pack(pop)

    TailHeader              *m_tailHeader;
    int                     *m_splitTail;
    Tail                    m_tail;

    void inflateTail(int expandSize)
    {
        int newSize;

        newSize = (((m_tailHeader->size + expandSize) >> 12) + 1) << 12; // align for 32 bits
        m_splitTail = (int *)realloc(m_splitTail, newSize * sizeof(int));
        
        if (m_splitTail == NULL)
            throw std::bad_alloc();
        
        memset(m_splitTail + m_tailHeader->size, 0, (newSize - m_tailHeader->size) * sizeof(int));
        m_tailHeader->size = newSize;
    }

    virtual void onExploreFinish(OnExploreFinish cb, void *arg, int s, int off) 
    {
        int p, val;

        if (m_exploreBuffer[off - 1] == '\0') {

            if (getBase(s) < 0)
                val = *(m_splitTail - getBase(s));
            else
                val = getBase(s);
            
        } else {

            for (p = -getBase(s); m_splitTail[p]; p++) 
                m_exploreBuffer[off++] = m_splitTail[p];

            m_exploreBuffer[off] = '\0';
            
            val = *(m_splitTail + p + 1);
        }

        cb(m_exploreBuffer, val, arg);
    }

    void insertTail(int s, const char *key, int val);
    void branch(int s, const char *key, int val);

public:
    TailTrie(int size=m_defaultStateSize)
        :TrieBase(size), m_tailHeader(NULL), m_splitTail(NULL)
    {
        m_tailHeader = new TailHeader;
        m_tailHeader->size = 0;
        m_tailHeader->last = 1;
        
        m_tail.data = NULL;
        m_tail.length = 0;
        
        inflateTail(size);

        strncpy(m_magic, "TailTrie", m_magicSize);
        strcpy(m_header->magic, m_magic);
    }

    TailTrie(const char *filename)
        :TrieBase(filename)
    {
        strncpy(m_magic, "TailTrie", m_magicSize);
    
        if (strncmp(m_header->magic, m_magic, sizeof(m_magic)))
            THROW_RUNTIME_ERROR(
                  std::string("TailTrie: Invalid Index Format! Should be ") 
                  + m_magic);

        int start = sizeof(Header) + m_header->size * sizeof(State);

        m_tailHeader = (TailHeader *)m_mmap->start(start);
        m_splitTail = (int *)m_mmap->start(start + sizeof(TailHeader));
        
        m_tail.data = NULL;
        m_tail.length = 0;
    }

    ~TailTrie()
    {
        if (!m_mmap) {
            delete m_tailHeader;
            free(m_splitTail);
            free(m_tail.data);
            m_tail.data = 0;
        }
    }

    virtual void
    onPrefixSearchFinish(const char *kp, char *buffer, int start, int s,
                         std::map<std::string, int> &map)
    {
        int        p;

        if (getBase(s) < 0) {
            /* search in tail */
            
            if (*kp) {
                /* search key not consumed */

                for (p = -getBase(s); ; p++) {
                    if ((char)m_splitTail[p] != *(kp++))
                        return;
                    buffer[start++] = m_splitTail[p];
                    if (!(*kp && m_splitTail[p]))
                        break;
                }

                buffer[start] = '\0';
                /* +1 is '\0', +2 is value */
                map[buffer] = *(m_splitTail + p + 2);

            } else {

                if (buffer[start - 1] == '\0') {
                    /* already got terminator */

                    map[buffer] = *(m_splitTail - getBase(s));

                } else {
                    /* haven't meet terminator yet */

                    for (p = -getBase(s); m_splitTail[p]; p++) 
                        buffer[start++] = m_splitTail[p];

                    buffer[start] = '\0';
                    map[buffer] = *(m_splitTail + p + 1);
                }

            }
        
        } else {
            /* no need to search in tail */
            buffer[start] = '\0';
            map[buffer] = getBase(s);
        }

    }

    void insert(const char *key, int val);
    int search(const char *key);
    void save(const char *filename);
};

#endif // TAILTRIE_H

// vim: ts=4 sw=4 cindent et
