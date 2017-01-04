// Created by jianingy <detrox@gmail.com> 
#include <limits.h>
#include "TrieBase.h"

char TrieBase::m_magic[m_magicSize] = "TrieBase";

TrieBase::TrieBase(int n)
    :m_header(NULL), m_state(NULL), m_mmap(NULL)
{
    m_header = new Header;

    m_header->maxValue = INT_MIN;
    m_header->minValue = INT_MAX;
    m_header->sumValue = 0;
    m_header->size = 0;

    strcpy(m_header->magic, m_magic);

    inflate(n);
    m_lastAvailState = 1;
}

TrieBase::TrieBase(const char *filename)
    :m_header(NULL), m_state(NULL), m_mmap(NULL)
{
    m_mmap = new MMapUtil(filename);
    m_header = (Header *)m_mmap->start();
    m_state = (State *)m_mmap->start(sizeof(Header));

/*    
    if (strncmp(m_header->magic, m_magic, sizeof(m_magic)))
        THROW_RUNTIME_ERROR(
              std::string("TrieBase: Invalid Index Format! Should be ") 
              + m_magic);
*/
}

int TrieBase::findBase(int *key, int min, int max)
{
    bool            found;
    int             i;
    int             *p;

    for (i = m_lastAvailState, found = false;!found;) {
        i++;

        if (i + key2state(max) >= m_header->size) 
            inflate(key2state(max));

        if (   getCheck(i + key2state(min)) <= 0 
            && getCheck(i + key2state(max)) <= 0) 
        {

            found = true;

            for (p = key; *p > -1; p++) {
                if (getCheck(i + key2state(*p)) > 0) {
                    found = false;
                    break;
                }
            }

        } else {
            //i += min;
        }
    }

    m_lastAvailState = i;

    return i;
}

int TrieBase::relocate(int stand, int s, int *key, int max, int min)
{
    int         oldBase;
    int         newBase;
    int         i;
    int         *p;
    int         ancestor[m_alphabetSize] = {0};

    oldBase = getBase(s);
    newBase = findBase(key, max, min);

    for (i = 0; key[i] > -1; i++) {

        int t = key2state(key[i]);

        if (getCheck(oldBase + t) != s)
            continue;

        setBase(newBase + t, getBase(oldBase + t));
        setCheck(newBase + t, getCheck(oldBase + t));

        findAccepts(oldBase + t, ancestor, NULL, NULL);
        for (p = ancestor; *p > -1; p++) 
            setCheck(getBase(oldBase + t) + key2state(*p), newBase + t);

        if (stand == oldBase + t) stand = newBase + t; 
        setBase(oldBase + t, 0);
        setCheck(oldBase + t, 0);
    }   
    setBase(s, newBase);

    return stand;
}

int TrieBase::createTransition(int s, int ch)
{
    int         t;
    int         n;
    int         m;
    int         nAncestor[m_alphabetSize] = {0};
    int         mAncestor[m_alphabetSize] = {0};
    int         max[] = {0, 0};
    int         min[] = {0, 0};

    t = getNext(s, ch);

    if (getBase(s) > 0 && getCheck(t) <= 0) {
        // Do Nothing!
    } else {

        n = findAccepts(s, nAncestor, &max[0], &min[0]);
        m = getCheck(t)?
            findAccepts(getCheck(t), mAncestor, &max[1], &min[1]):0;

        if (m > 0 && n + 1 > m) {
        
            s = relocate(s, getCheck(t), mAncestor, max[1], min[1]); 

        } else {

            nAncestor[n++] = ch;
            nAncestor[n] = -1;

            if (ch > max[0] || !max[0])
                max[0] = ch;
            if (ch < min[0] || !min[0])
                min[0] = ch;

            s = relocate(s, s, nAncestor, max[0], min[0]);

        }

        t = getNext(s, ch);

    }

    setCheck(t, s);

    return t;
}


void TrieBase::insert(const char *key, int val)
{
    const char      *p;
    int             s;
    int             t;

    if (val < 0)
        THROW_RUNTIME_ERROR("TrieBase: Invalid value");

    if (!key)
        THROW_RUNTIME_ERROR("TrieBase: Empty Key");

    for (p = key, s = 1;; p++) {

        t = getForward(s, (unsigned char)*p);
        if (!t) {
            for (;; p++) {
                s = createTransition(s, (unsigned char)*p);
                if (*p == '\0') break;
            }
            break;
        }

        s = t; /* move to next state */
        if (*p == '\0') break;
    }

    setBase(s, val);
    updateStat(val);

}

int TrieBase::search(const char *key)
{
    const char  *p;
    int         s;
    
    for (p = key, s = 1;; *p++) {

        if (!(s = getForward(s, (unsigned char)*p)))
            return 0;

        if (*p == '\0') break;
    }

    return s?getBase(s):0;
}

int TrieBase::prefixSearch(const char *key, std::map<std::string, int> &map)
{
    const char          *p;
    int                 *q;
    int                 s;
    int              i;
    int                 accepts[m_alphabetSize];
    char                keyBuffer[4096];

    for (p = key, s = 1;; *p++) {

        if (*p == '\0' || getBase(s) < 0) 
            break;

        if (!(s = getForward(s, (unsigned char)*p)))
            return 0;

    }

    --p;
    i = p - key;
    strncpy(keyBuffer, key, i);

    if (getBase(s) < 0) {
        keyBuffer[i++] = *p;
        //std::cout << "buffer = " << keyBuffer << ", p = " << (int)*(p + 1) << std::endl;
        onPrefixSearchFinish(p + 1, keyBuffer, i, s, map);
        return 0;
    }

    m_searchQueue.clear();
    m_searchQueue.reserve(i * 2);
    m_searchQueue.push_back(std::make_pair(s, i));

    while (!m_searchQueue.empty()) {
        std::pair<int, int>     pair;

        pair = m_searchQueue.back();
        m_searchQueue.pop_back();

        s = pair.first;
        i = pair.second;
        
        keyBuffer[i++] = (char)(s - getBase(getCheck(s)) - 1); 

        if (findAccepts(s, accepts, NULL, NULL) > 0) {

            for (q = accepts; *q > -1; q++)
                m_searchQueue.push_back(std::make_pair(getNext(s, *q), i));

        } else {
           onPrefixSearchFinish(p + 1, keyBuffer, i, s, map);
        }
    }

    return 0;
}

void TrieBase::onExplore(OnExploreFinish cb, void *arg, int s, int off)
{
    int         *p; 
    int         key[m_alphabetSize];

    if (findAccepts(s, key, NULL, NULL) > 0) {
        for (p = key; *p > -1; p++) {
            m_exploreBuffer[off] = (char)*p;
            onExplore(cb, arg, getNext(s, *p), off + 1);
        }
    } else {
        onExploreFinish(cb, arg, s, off);
    }

}

void TrieBase::save(const char *filename)
{
    FILE *fp;

    if (!(fp = fopen(filename, "w+")))
        THROW_RUNTIME_ERROR("TrieBase: Can not write to file");

    if (fwrite(m_header, sizeof(Header), 1, fp) != 1)
        THROW_RUNTIME_ERROR("TrieBase: Write Error!");

    if ((int)fwrite(m_state, sizeof(State), m_header->size, fp)
        != m_header->size)
        THROW_RUNTIME_ERROR("TrieBase: Write Error!");

    fclose(fp);
}

// vim: ts=4 sw=4 cindent et

