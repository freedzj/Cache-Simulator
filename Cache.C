#include <iostream>
#include <string>
#include <assert.h>
#include "Cache.h"

/*
 * Cache constructor
 * Creates the twoD array of lines and initializes the
 * associativity, numSets, blockOffsetBits, setIndexBits,
 * and verbose data members.
 */
Cache::Cache(int32_t associativity, int32_t blockOffsetBits, 
        int32_t setIndexBits, bool verbose)
{
    //The cache needs to be a 2d array.
    //2^s by 2^e, or S by E (associativity)
    //No. sets is 2^setIndexBits
    numSets = 1 << setIndexBits;

    //initialize private members
    this->associativity = associativity;
    this->blockOffsetBits = blockOffsetBits;
    this->setIndexBits = setIndexBits;
    this->verbose = verbose;

    //Create the 2D array
    lines = new Line*[numSets];
    for(int i = 0; i < numSets; i++) 
    {
        lines[i] = new Line[associativity];
    }
}

/*
 * Cache destructor
 * Destroys the twoD array of lines
 */
Cache::~Cache()
{
    for(int i = 0; i < numSets; i++) 
    {
        delete [] lines[i];
    }
    delete [] lines;
}

/*
 * Line constructor
 * Sets the tag to 0 and the valid bit to false (tag not valid)
 */
Cache::Line::Line()
{
    //This constructor is complete.
    tag = 0;
    valid = false;
}

/*
 * getBits
 * Get the bits from a given source using the parameters low and high.
 *
 * low - the lower bounds of the selection
 * high - the higher bounds of the selection
 * source - the data used for extraction
 *
 * returns a uint64_t containing only the bits within the bounds
 */
uint64_t Cache::getBits(uint64_t source, int32_t low, int32_t high)
{
    //By this point, the source is converted from hex to base 10
    assert(low >= 0 && high >= 0 && low <= high && high <= 63); 
    //shifting works and has been tested on this input:
    // uint64_t n=395; //110001011
    // uint64_t bits = getBits(n,0,5);
    // cout << bits; OUTPUT: 11 ... as expected since 0-5 bits are 01011 = 11 base 10
    source = source << (63-high);
    source = source >> (63-high);
    source = source >> low;
    return source;   
}

/*
 * presentInCache
 * This function determines if the line is already present in the cache set
 *
 * setIndexBits - the set being searched
 * tagBits - the tag being looked for
 *
 * returns true if the tag was found, along with the valid bit being true
 */
bool Cache::presentInCache(int64_t set, int64_t tag)
{
    //Search the given set for a matching tag
    for(int i = 0; i < this->associativity; i++)
    {
        //The tag must match and the valid bit must be true
        if((lines[set][i].tag == tag) && 
                lines[set][i].valid == true)
        {
            return true;
        }
    }
    //It's not there; miss
    return false;
}

/*
 * isFull
 * This function determines if the given set is full
 * The set is full if the last index's valid member is true
 *
 * setNum - the set being searched
 *
 * returns true if the given set is full
 */
bool Cache::isFull(int64_t set)
{
    //complete
    return lines[set][associativity - 1].valid;
}

/*
 * updateCache
 * This function is used to update the cache, specifically
 * it updates the given set by adding the new tag to the first element
 * of the given set.
 *
 * setNum - the set being searched
 * tagBits - the tag being looked for
 *
 * returns true if an eviction happened
 */
void Cache::updateCache(int64_t set, int64_t tag)
{
    //prepare the cache set for an update by shifting the lru order
    updateLRU(set);

    //Update set index 0
    lines[set][0].tag = tag;
    lines[set][0].valid = true;	
}

/*
 * getTag
 * This returns the tag bits from a given data source
 *
 * source - the data the tag is being extracted from
 *
 * returns int32_t value containing the tag bits
 */
int64_t Cache::getTag(uint64_t source)
{
    //Get the tag bits ------ 63 (highest bit) to offset+set
    return getBits(source, (setIndexBits + blockOffsetBits), 63);
}

/*
 * getSet
 * This returns the set bits from a given data source
 *
 * source - the data the set is being extracted from
 *
 * returns an int32_t value containing the set bits
 */
int64_t Cache::getSet(uint64_t source)
{
    //Get the set bits and mod by the number of sets
    int64_t set = (getBits(source,  blockOffsetBits, (setIndexBits + blockOffsetBits) - 1));
}

/*
 * updateLRU
 * This updates the LRU ordering of a given set,
 * so lines[setNum][associativity - 1].tag is the least recently used tag
 *
 * setNum - the set that is being reordered
 *
 * returns int32_t value containing the set bits
 */
void Cache::updateLRU(int64_t set)
{
    //complete and tested
    for(int i = associativity - 1; i > 0; i--)
    {
        lines[set][i] = lines[set][i-1];
    }
    //this should shift all the elements right while discarding the last element
    //allowing us to overwrite the first element while pushing it backwards in the array
    // ex: "16 2 77 40 12071" becomes "16 16 2 77 40", so we can place the new one in front
}

/*
 * updateLRUHit
 * Update the cache by moving the tag that was hit
 * to the front of the array.
 *
 * set - the index in the cache containing the tag
 * tag - the tag itself; the tag we hit
 *
 */
void Cache::updateLRUHit(int64_t set, int64_t tag)
{
    if(lines[set][0].tag != tag) 
    {
        int index = -1;
        for(int i = 0; i < associativity && i != index ; i++)
        {
            if(lines[set][i].tag == tag) 
            {
                index = i;
                break;
            }
        }
        //index is now the location of the tag we need to move to front
        int32_t placeHolder = lines[set][index].tag;
        for(int j = index; j > 0; j--) 
        {
            lines[set][j] = lines[set][j-1];
        }	
        lines[set][0].tag = placeHolder;
    }
}


/*
 * printAccess
 * This prints the current line if verbose mode
 * was selected when execution began.
 *
 * line - the set that is being reordered
 */
void Cache::printAccess(std::string line)
{
    //complete and tested
    if(this->verbose)
    {
        //The lines come with a leading white space, we need to remove it
        std::string noLeadingSpace = line.substr(1,line.length());
        //Print the cleaned up string

        bool endOfLeadingZero = false;
        std::cout << noLeadingSpace[0];
        std::cout << noLeadingSpace[1];
        for(int i = 2; i < noLeadingSpace.size(); i++) 
        {
            if(noLeadingSpace[i] != '0')
            {
                endOfLeadingZero = true;
            }
            if(endOfLeadingZero)
            {
                std::cout << noLeadingSpace[i];
            }

        }	
        std::cout<< " ";
    }
}
