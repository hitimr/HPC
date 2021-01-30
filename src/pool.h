#ifndef POOL_H
#define POOL_H

#ifdef __cplusplus
extern "C"
{
#endif
    #include <vector>
    #include <iostream>

    class Pool
    {
        public:
            // TODO: check how inline impacts performance
            std::vector<int64_t>::iterator begin() { return data.begin()++; }
            std::vector<int64_t>::iterator end() { return data.end(); }
            int64_t at(int64_t n) { return data.at(n+1); }

            std::vector<int64_t> data;

    };

#ifdef __cplusplus
}
#endif

#endif