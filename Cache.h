

class Cache
{
    class Line         
    {
        public:
            int64_t tag;  //address tag
            bool valid;   //is tag valid
            Line();
    };
    private:
    Line ** lines;           //cache is a 2D array of lines
    int32_t numSets;         //number of cache sets (number of rows)
    int32_t associativity;   //set associativity (number of columns)
    int32_t blockOffsetBits; //number of bits of the address for block offset
    int32_t setIndexBits;    //number of bits of the address for set index
    bool verbose;            //verbose mode of execution
    public:
    Cache(int32_t associativity, int32_t blockOffsetBits, 
            int32_t setIndexBits, bool verbose);
    ~Cache();
    uint64_t getBits(uint64_t source, int32_t low, int32_t high);
    bool presentInCache(int64_t set, int64_t tag);
    bool isFull(int64_t set);
    void updateCache(int64_t set, int64_t tag);
    void updateLRU(int64_t set);
    int64_t getTag(uint64_t source);
    int64_t getSet(uint64_t source);
    void printAccess(std::string line);
    void updateLRUHit(int64_t set, int64_t tag); 

};
