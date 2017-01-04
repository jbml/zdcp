#ifndef SIMPLE_CONFIG_H
#define SIMPLE_CONFIG_H

#include <map>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <stdexcept>
#include <vector>
#include <ctype.h>
#include "common/common.h"

class SimpleConfig {

protected:
    size_t                                 m_deepParseLevel;
    std::map<std::string, std::string>     m_map;
    std::map<std::string, std::string>     m_storage;
    std::string                            m_key;
    std::string                            m_val;
    std::string                            m_real;
    std::string                            m_reserve;

    std::string& trim(std::string &s)
    {
        static const char       whitespace[] = " \t\r\n";
        std::string::size_type  begin;
        std::string::size_type  end;

        begin = s.find_first_not_of(whitespace);
        if (begin > 0 && begin < s.npos)
            s.erase(0, begin);

        end = s.find_last_not_of(whitespace);
        if (end + 1 < s.npos)
            s.erase(end + 1);

        return s;
    }

    void insert(std::string &s)
    {
        int split = s.find("=");

        m_key = s.substr(0, split);
        m_val = s.substr(split + 1);

        trim(m_key);
        trim(m_val);

        if (!m_key.empty())
            m_map[m_key] =m_val;
    }

    void parseValue(std::string &str, std::string &real)
    {
        const char          *pch;
        bool                opencb = false;
        enum {
            PS_UNKNOW = 0,
            PS_ESCAPE,
            PS_DOLLAR,
            PS_FETCHKEY,
            PS_REPLACE,
            PS_FIN,
            PS_REPFIN,
        } state;

        real.clear();
        m_key.clear();

#define isident(CH) (isalnum(CH) || (CH) == '_')

        for (pch = str.c_str(), state = PS_UNKNOW; ;pch++) {

            if (*pch == '\\')
                state = PS_ESCAPE;
            else if (*pch == '\0')
                state = (state == PS_FETCHKEY)?PS_REPFIN:PS_FIN;
            else if (*pch == '$' && state != PS_ESCAPE)
                state = PS_DOLLAR;
            else if (state == PS_DOLLAR)
                state = PS_FETCHKEY;
            else if (state == PS_FETCHKEY && !isident(*pch) && *pch != '{')
                state = PS_REPLACE;
            else if (state != PS_FETCHKEY)
                state = PS_UNKNOW;

            if (state == PS_UNKNOW) {

                real.append(pch, 1);

            } else if (state == PS_FETCHKEY) {

                if (isident(*pch))
                    m_key.append(pch, 1);
                if (*pch == '{')
                    opencb = true;

            } else if (state == PS_REPLACE || state == PS_REPFIN) {

                real.append(m_map[m_key]); 
                m_key.clear(); 

                if (!(opencb && *pch == '}'))
                    pch--;

            }

            if (state == PS_FIN || state == PS_REPFIN)
                break;
        }
    }

#undef isident

    std::string& deepParse(std::string &str, size_t level = 0)
    {
        size_t i;

        level = (level)?level:m_deepParseLevel;

        for(i = 0; i < level && str.find('$') < str.npos; i++) {
            parseValue(str,m_real);
            str =m_real;
        }

        return str;
    }

public:
    SimpleConfig()
        :m_deepParseLevel(3)
    {
    }

    SimpleConfig(const char *s)
        :m_deepParseLevel(3)
    {
        readFromFile(s);
    }

    SimpleConfig& operator<< (const char *s)
    {
        m_reserve = s;
        insert(m_reserve);
        return *this;
    }

    SimpleConfig& operator<< (std::string &s)
    {
        insert(s);
        return *this;
    }

    std::string &operator[] (std::string s)
    {
        return m_map[s];
    }

    void getValue(std::string key, int &val)
    {
        if (m_map.find(key) != m_map.end())
            val = atoi(deepParse(m_map[key]).c_str());
    }

    void getValue(std::string key, long &val)
    {
        if (m_map.find(key) != m_map.end())
            val = atol(deepParse(m_map[key]).c_str());
    }

    void getValue(std::string key, double &val)
    {
        if (m_map.find(key) != m_map.end())
            val = atof(deepParse(m_map[key]).c_str());
    }

    void getValue(std::string key, std::string &val)
    {
        if (m_map.find(key) != m_map.end())
            val = deepParse(m_map[key]);
    }

    void getValue(std::string key, std::vector<std::string> &val)
    {
        if (m_map.find(key) != m_map.end()) {
            std::string::size_type i, p;

            val.clear();
            m_reserve = deepParse(m_map[key]);

            for (p = 0; p <m_reserve.npos; p = i + 1) { 
                i = m_reserve.find(',', p);
                if (i == m_reserve.npos)
                    i = m_reserve.npos - 1;
                std::string s = m_reserve.substr(p, i - p);

                trim(s);

                if (!s.empty())
                    val.push_back(s);
            }
        }
    }

    void dump(std::string &s)
    {
        std::map<std::string, std::string>::iterator it;
        std::ostringstream oss;

        for (it =m_map.begin(); it !=m_map.end(); it++) {
            oss << it->first << " = " << it->second << std::endl;
        }
        s.assign(oss.str());
    }

    void readFromFile(const char *s)
    {
        std::ifstream ifs(s);
        if (ifs.is_open()) {
            while(!(std::getline(ifs,m_reserve).eof())) {
                trim(m_reserve);
                if (!m_reserve.empty() && m_reserve[0] != '#')
                    insert(m_reserve);
            }
        } else {
            THROW_RUNTIME_ERROR(std::string("can not open configuration ") + s);
        }
    }
};

#endif // SIMPLE_CONFIG_H
// vim: ts=4 sw=4 et ai cindent
