#include <fstream>
#include <iostream>
#include <string>
#include <assert.h>
#include <regex>
#include "Cache.h"
#include "Simulate.h"

/* Simulate
 * Initializes the hits, misses, evictions, and verbose
 * data members.
 * Create the cache object that will be used in the
 * simulation.
 */
Simulate::Simulate(int32_t associativity, int32_t blockOffsetBits, 
        int32_t setIndexBits, bool verbose)
{
    //This method is complete
    hits = misses = evictions = 0;
    cacheP = new Cache(associativity, blockOffsetBits, setIndexBits, verbose);
    this->verbose = verbose;
}

/*
 * Simulate destructor
 * Destroys the cacheP object
 */
Simulate::~Simulate()
{
    delete cacheP;
}

/*
 * run
 * Opens the file of addresses and uses each address of a data item
 * to perform a cache access.  The addresses are formatted as follows:
 I 0400d7d4,8
 M 0421c7f0,4
 L 04f6b868,8
 S 7ff0005c8,8

 Each line denotes one or two memory accesses. The format of each line is

 [space]operation address,size

 The operation field denotes the type of memory access: 
 I denotes an instruction load, L a data load, S a data store, and M a 
 data modify (i.e., a data load  followed by a data store, thus two 
 sequential accesses to cache). There is never a space before an I. 
 There is always a space before each M, L, and S. The address field 
 specifies a 64-bit hexadecimal memory address. The size field specifies 
 the number of bytes accessed by the operation.  The size
 field and instruction (I) addresses can be ignored.

 This method will use an address to access the cache
 and update the hits, misses, and evictions data members.
 */
void Simulate::run(std::string filename)
{

    //Create ifstream to read the tracefile
    std::ifstream tracefile;
    tracefile.open(filename);

    //This is used to hold the current line
    std::string line;

    //Make sure we were able to open the file
    if(!tracefile.is_open()) 
    {
        std::cout << "Unable to open trace file " << filename << "\n";
        exit(EXIT_FAILURE);
    }

    //Continue to read the trace file while there is input left
    while(getline(tracefile, line)) 
    {
        //inside here the line variable is the input from the file

        //Check to see if the line is an instruction, if so ignore
        //and go to the next line
        if(line[0] == 'I')
        {
            continue;
        }


        //Extract the value after the identifier
        std::string hex = ""; 

        //Create char to hold the type of access L/M/S
        char memAccess = line[1]; //1 because the type is 2 characters into the input string

        //Start j at 0, since hex is a normal string
        int j = 0;

        //Start loop at index 3 since that is where
        //The numbers start within the format given
        for(int i = 3; line[i] != ','; i++, j++)
        { 
            hex[j] = line[i];
        }
        //Insure the strings terminate in the right location
        hex[j]='\0';
        //This is used to keep track of how many chars stoull processes
        std::string::size_type sz = 0; 

        //hex holds the hex string value of the input line, we need to convert it 
        //stoull takes the hex string, converts the numbers into decimal
        uint64_t source = std::stoul(hex, &sz, 16);

        //See if we need to print (verbose mode)
        cacheP->printAccess(line);

        //Send the address to be processed
        int64_t tag = cacheP->getTag(source);
        int64_t set = cacheP->getSet(source);

        //Determine what to do with the given memory access
        switch(memAccess) 
        {
            //L a data load
            case 'L' :	loadData(tag, set);
                        break;
                        //S a data store
            case 'S' :  loadData(tag, set);
                        break;			
                        //M a data load  followed by a data store
            case 'M' :  loadData(tag, set);
                        loadData(tag, set);
                        break;
            default:	std::cout << " Invalid memory access attempted \n";	
                        break;
        }
        if(verbose)
        {    
            std::cout<<"\n"; //Blank line so the next line doesn't run onto the previous
        }
    }
    tracefile.close();
}

/*
 * printSummary
 * Prints the number of hits, misses, and evictions both
 * to stdout and to a file to support automatic testing
 */
void Simulate::printSummary()
{
    //This method is complete
    std::fstream fs;
    printf("hits:%d misses:%d evictions:%d\n", hits, misses, evictions);
    fs.open(".csim_results", std::fstream::out);
    assert(fs.is_open());
    fs << hits << " " << misses << " " << evictions <<"\n";;
    fs.close();
}

/*
 * loadData
 * Load the data into the cache.
 *
 * setNum - the set being searched
 * tagBits - the tag being looked for
 */
void Simulate::loadData(int64_t tag, int64_t set)
{
    //Check to see if the data is in the cache
    if(cacheP->presentInCache(set, tag)) //hit
    {
        hits++;
        cacheP->updateLRUHit(set, tag); //shift the tag to the front, moving the others in front down
        if(verbose)
        {
            std::cout << "hit ";
        }
    }
    else //miss
    {
        bool full = false;
        misses++; 

        //check to see if we need to evict
        if(cacheP->isFull(set))
        {
            full = true;
            evictions++; 
        }

        //update the cache
        cacheP->updateCache(set, tag);

        //print if needed
        if(verbose)
        {
            std::cout << "miss ";
        }
        if(full && verbose)
        {
            std::cout << "eviction ";
        }
    }
}
