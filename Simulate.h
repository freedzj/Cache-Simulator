
class Simulate
{
    private:
        Cache * cacheP;      //pointer to the cache object
        int32_t hits;        //number of hits
        int32_t misses;      //number of misses
        int32_t evictions;   //number of evictions
        bool verbose;        //for verbose output
    public:
        Simulate(int32_t associativity, int32_t blockOffsetBits, 
                int32_t setIndexBits, bool verbose);
        ~Simulate();
        void printSummary();
        void run(std::string filename);
        void loadData(int64_t tag, int64_t set);
};
