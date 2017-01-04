// Created by jianingy <detrox@gmail.com>

#include "TailTrie.h"

void TailTrie::insertTail(int s, const char *key, int val)
{
    const char *p;

    setBase(s, -(m_tailHeader->last));
    
    for (p = key;*p; p++) {
        
        if (m_tailHeader->last + 32 >= m_tailHeader->size)
            inflateTail(32);

        m_splitTail[m_tailHeader->last++] = (unsigned char)*p;
    }

    m_splitTail[m_tailHeader->last++] = 0;
    m_splitTail[m_tailHeader->last++] = val;
    updateStat(val);
}

void TailTrie::branch(int s, const char *suffix, int val)
{
    int i, j, t, *key, start, base, max, min;

    key = m_tail.data;
    start = -(getBase(s));

    for (i = 0, max = 0, min = 0;; i++) {

        /* expand by 4k */
        if (i + 2 >= m_tail.length) {
        
            /* +2 for trailing marks*/
            int newSize = (((m_tail.length + i + 2) >> 8) + 1) << 8;
            m_tail.data = (int *)realloc(m_tail.data, newSize * sizeof(int));
            m_tail.length = newSize;
            key = m_tail.data;
        
        }

        key[i] = (unsigned char)suffix[i];
        
        if (key[i] > max || max == 0)
            max = key[i];
        if (key[i] < min || min == 0)
            min = key[i];

        if (*(m_splitTail + start + i) != key[i] || key[i] == 0)
            break;
    }

    key[i + 1] = -1;
    
    if (*(m_splitTail + start + i)  == key[i]) {
        *(m_splitTail + start + i + 1) = val;
        return;
    }

    base = findBase(key, max, min);
    setBase(s, base);
    
    for (j = 0, t = s; j < i; j++)
        t = createTransition(t, key[j]);
    
    // create twig
    s = createTransition(t, *(m_splitTail + start + i));
    setBase(s, -(start + i + 1));
    
    s = createTransition(t, key[i]);

    if (key[i] == 0) {
        setBase(s, val);
        updateStat(val);
    } else {
        insertTail(s, suffix + i + 1, val);
    }

}

void TailTrie::insert(const char *key, int val)
{
    int             i;
    int             s;
    int             t;
    const char      *p;

    for (p = key, i = 0, s = 1;; p++, i++) {
        
        if (getBase(s) < 0) {
            branch(s, p, val);
            return;
        }
        
        t = getForward(s, (unsigned char)*p);
        if (t == 0) {
            
            t = createTransition(s, (unsigned char)*p);

            if (*p == 0) {
                setBase(t, val);
                updateStat(val);
            } else {
                insertTail(t, p + 1, val);
            }

            return;

        }

        s = t;
        if (*p == '\0') break;
    }

    setBase(s, val);

}

int TailTrie::search(const char *key)
{
    int             s;
    int             t;
    int             *q;
    const char      *p;

    for (s = 1, p = key;; p++) {

        if (getBase(s) < 0) {

            for (q = m_splitTail - getBase(s);; q++, p++) {
                if (*q != (unsigned char)*p)
                    return 0;
                if (*q == 0) 
                    break;
            }

            return *(q + 1);
        
        }
        
        t = getForward(s, (unsigned char)*p);
        
        if (t == 0)
            return 0;

        s = t;
        if (*p == 0)
            break;

    }

    t = getBase(s);
    
    return (t < 0)?*(m_splitTail - t):getBase(s);
}

void TailTrie::save(const char *filename)
{
    FILE *fp = NULL;

    fp = fopen(filename, "w+");
    if (fp == NULL)
        THROW_RUNTIME_ERROR("Can not write to file");

    if (fwrite(m_header, sizeof(Header), 1, fp) != 1)
        THROW_RUNTIME_ERROR("TailTrie: Write Error");

    if ((int)fwrite(m_state, sizeof(State), m_header->size, fp)
        != m_header->size)
        THROW_RUNTIME_ERROR("TailTrie: Write Error");

    if ((int)fwrite(m_tailHeader, sizeof(TailHeader), 1, fp) != 1)
        THROW_RUNTIME_ERROR("TailTrie: Write Error");
    if ((int)fwrite(m_splitTail, sizeof(int), m_tailHeader->size, fp) 
        != m_tailHeader->size)
        THROW_RUNTIME_ERROR("TailTrie: Write Error");

    fclose(fp);
}

// vim: ts=4 sw=4 cindent et
