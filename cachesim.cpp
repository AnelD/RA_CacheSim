#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <string>
#include <sstream>

#define TIME_TYPE std::chrono::high_resolution_clock::time_point

// TODO
// Pass cache parameters through console []
// Clean up Code []
// Refactor code maybe split into different files []
// Macros for long typename -> timestamp [X]
// Have Output not be overwritten []
// Timestamp when benchmark was performed and used arguments  []

// Struct that keeps track of all relevant information of a single cache block
struct CacheBlock
{
    unsigned int tag;
    bool valid;
    
    // Timestamp for evicition policy
    TIME_TYPE timestamp; 

};

// Function that splits the 32-Bit adress into TAG bits
// Address:  [TAG][INDEX][OFFSET]

unsigned int adr_to_tag(unsigned int adr, int tagBits)
{
    // Number of bits in unsigned int, size of returns bytes so *8 for bits
    
    int size_of_uint = sizeof(unsigned int)*8;  

    int shift_amount = size_of_uint - tagBits;
    
    unsigned int r = adr >> shift_amount;

    return r;

}


// Function that splits the 32-Bit adress into INDEX bits
// Address:  [TAG][INDEX][OFFSET]

unsigned int adr_to_index(unsigned int adr, int tagBits, int offsetBits)
{

    // Number of bits in unsigned int, size of returns bytes so *8 for bits
    
    int size_of_uint = sizeof(unsigned int)*8;  

    int shift_left_amount = tagBits;

    int shift_right_amount = shift_left_amount + offsetBits;
    
    unsigned int r = adr << shift_left_amount;
    r = r >> shift_right_amount; 

    return r;


}

void adress_calculation_check(int tagBits, int offsetBits, int indexBits)
{
    
    std::cout << "Tag bits: " << tagBits << std::endl;
    std::cout << "Index bits: " << indexBits << std::endl;
    std::cout << "Offset bits: " << offsetBits << std::endl;


    unsigned int adr[] = {  0xffffffff, 0x10010000, 0x10010060, 0x10010030,
                            0x10010004, 0x10010064, 0x10010034  };

    for (int i = 0; i < 7; i++)
    {
        std::cout << "Adress[" << i << "]: Tag = " << adr_to_tag(adr[i], tagBits)
        << " , Index = " << adr_to_index(adr[i], tagBits, offsetBits) << std::endl;
    }
}

// Function to create timestamp for eviction policies
TIME_TYPE getTimestamp() 
{
    return std::chrono::high_resolution_clock::now();
}

// Function to store into cache
// policy: 0 == LRU,  1 == FIFO,  2 == Random
void store( CacheBlock** cache, unsigned int address, int tagBits, 
            int offsetBits, int associativity, int policy, int &evictions, int &accesses)
{
    unsigned int tag = adr_to_tag(address, tagBits);
    unsigned int index = adr_to_index(address, tagBits, offsetBits);

    accesses++;

    // Check if all blocks in a set are already filled (valid = true)
    // If one is found that isnt filled fill it

    bool set_full;

    for (int i = 0; i < associativity; i++)
    {
        if (cache[index][i].valid == false)
        {  
            cache[index][i].tag = tag;
            cache[index][i].valid = true;
            set_full = false;
        }
        else 
            set_full = true;
    }

    // If all are filled decide which to overwrite using the chosen eviction policy

    // Overwrite using LRU policy
    if(set_full && (policy == 0))
    {
        TIME_TYPE lru_t = cache[index][0].timestamp; 
        int lru_index = 0;
        
        for (int i = 1; i < associativity; i++)
        {
            if (cache[index][i].timestamp < lru_t)
                {
                    lru_t = cache[index][i].timestamp;
                    lru_index = i;
                }
        }

        cache[index][lru_index].tag = tag;
        cache[index][lru_index].valid = true;
        cache[index][lru_index].timestamp = getTimestamp();
        evictions++;
    }

    // Overwrite using FIFO policy
    else if(set_full && (policy == 1))
    {
        std::chrono::high_resolution_clock::time_point 
                    lru_t = cache[index][0].timestamp; // Ugly
        int lru_index = 0;
        
        for (int i = 1; i > associativity; i++)
        {
            if (cache[index][i].timestamp > lru_t)
                {
                    lru_t = cache[index][i].timestamp;
                    lru_index = i;
                }            
        }
       
        cache[index][lru_index].tag = tag;
        cache[index][lru_index].valid = true;
        cache[index][lru_index].timestamp = getTimestamp();
        evictions++;
    }

    else if(set_full && (policy == 2))
    {   
        int rand_index = rand() % associativity;

        cache[index][rand_index].tag = tag;
        cache[index][rand_index].valid = true;
        cache[index][rand_index].timestamp = getTimestamp();
        evictions++;
    }
}


// Function to read a cache entry 
// if entry exists hit++ else miss++ and load that entry according to eviction policy
void load(CacheBlock** cache, unsigned int address, int tagBits, 
            int offsetBits, int associativity, int policy, 
            int &miss, int &hit, int &evictions, int &accesses)
{
    accesses++;

    unsigned int tag = adr_to_tag(address, tagBits);
    unsigned int index = adr_to_index(address, tagBits, offsetBits);

    // Search for tag in the set
    // If tag is in set hit++ else miss++ and load tag into set

    bool tag_not_found = true;
    
    for (int i = 0; i < associativity; i++)
    {
        if (cache[index][i].tag == tag)
        {  
            hit++;
            tag_not_found = false;
        }
       
    } 
    
    if(tag_not_found)
    {
        miss++;
        store(cache, address, tagBits, offsetBits, associativity, policy, evictions, accesses);
    }

}

// Function to read a trace file and extract the relevant information
// [#][Load==0/Store==1][32-Bit Adress][Irrelevant]
// Needs to read 2 numbers after # in every line
// First number chooses which function gets called 
// Second number gets split up into tag and index  

void read(  std::string filename, int tagBits, int offsetBits, CacheBlock** cache, 
            int associativity, int policy, int &accesses, int &hit, 
            int &miss, int &evictions)
{
    int i = 0;
    std::ifstream inputFile(filename); // Tries to open the file

    if (inputFile.is_open()) 
    {
        std::string line;
    	int line_nr = 0;

        while (std::getline(inputFile, line)) 
        {
            // Reads every line as a string
            std::istringstream iss(line);
            char hashtag;
            std::string hexNum;
            int load_store, ignoredNum; // load == 0, store == 1
            unsigned int address;
            // Extracts the # and numbers from the string
            // # and the 3rd number get ignored, first number is saved to num1
            // 2nd number is saved as string to hexNum and gets converted with stoi
            if (iss >> hashtag >> load_store >> hexNum >> ignoredNum) 
            {
                // Convert the hexadecimal number to integer -> need to convert to unsigned
                // stoul converts to unsigned long but seems to work fine
                address = std::stoul(hexNum, nullptr, 16); 

                //std::cout << "L/S: " << load_store << ", Adress: " << address << ", Number 3: " << ignoredNum << std::endl;
                
                // Extract tag and index from adress ?
               
                line_nr++;

            }


            // Loads address from cache
            if(load_store == 0)
            {
                load(cache, address, tagBits, offsetBits, associativity, policy, miss, hit, evictions, accesses);
            }


            // Stores address into cache
            else if (load_store == 1)
            {
                store(cache, address, tagBits, offsetBits, associativity, policy, evictions, accesses);
            }


            // If neither works prints to console in which line it failed
            else
            {
                std::cout << "failed at line: " << line_nr << std::endl;
            }

            //Testing only first 20 lines to see if function works
            //i++;
            //if(i >= 20)
            //   break;
        }
    } 
    
    else 
    {
        std::cout << "Unable to open the file: " << filename << std::endl;
    }
}


//Function to write the results of the cache simulation to an output file

void write(std::string filename, int hit, int miss, int accesses, int evictions)
{
    float miss_rate = 100.0/accesses*miss;

    std::ofstream outputFile(filename);

    if (outputFile.is_open()) {
        // Write data to the file
        outputFile << "Cache accesses: " << accesses << std::endl;
        outputFile << "Cache hits: " << hit << std::endl;
        outputFile << "Cache evictions: " << evictions << std::endl;
        outputFile << "Cache misses: " << miss << std::endl;
        outputFile << "Cache miss rate: " << miss_rate << " %" << std::endl;

        // Close the file
        outputFile.close();

        std::cout << "Data written to the file: " << filename << std::endl;
    } else 
    {
        std::cout << "Unable to open the file: " << filename << std::endl;
    }
}


int main(int argc, char** argv)
{
    std::string input_filename;
    std::string output_filename;

    std::string tTag = "-t"; // Name of trace file with file extension, has to be in same folder as programm
    std::string nTag = "-n"; // Number of cache blocks
    std::string gTag = "-g"; // Size of cache blocks in bytes
    std::string aTag = "-a"; // Cache associativity
    std::string rTag = "-r"; // Eviction policy
    std::string wTag = "-w"; // Writing policy
    std::string oTag = "-o"; // Name of output file with, file extension has to bein same folder as programm

    // Console arguments, if not given initialised value == default value
    int cache_block_num = 1024; // == Number of cache blocks
    int cache_block_size = 32; // Size of cache blocks in bytes
    int cache_associativity = 2; // Cache associativity 
    int eviction_policy = 0;    //policy: 0 == LRU,  1 == FIFO,  2 == Random

    for (int i = 1; i < argc - 1; ++i) 
    { // Start from index 1 to skip the program name
        std::string arg = argv[i];
        std::string nextArg = argv[i + 1];

        if (arg == tTag) 
        {
            input_filename = nextArg;
        } 
        else if (arg == oTag) 
        {
            output_filename = nextArg;
        }
        else if (arg == nTag) 
        {
            try 
            {
                cache_block_num = std::stoi(nextArg);
            } 
            catch (const std::exception& e) {
                std::cout << "Invalid argument for " << nTag << ": " << nextArg << std::endl;
            }
        }
        else if (arg == gTag) 
        {
            try 
            {
                cache_block_size = std::stoi(nextArg);
            } 
            catch (const std::exception& e) {
                std::cout << "Invalid argument for " << nTag << ": " << nextArg << std::endl;
            }
        }
        else if (arg == aTag) 
        {
            try 
            {
                cache_associativity = std::stoi(nextArg);
            } 
            catch (const std::exception& e) {
                std::cout << "Invalid argument for " << nTag << ": " << nextArg << std::endl;
            }
        }
        else if (arg == rTag) 
        {
            if(nextArg == "LRU")
                eviction_policy = 0;
            else if (nextArg == "FIFO")
                eviction_policy = 1;
            else if (nextArg == "Random")
                eviction_policy = 2;
        }
        else if (arg == wTag) 
        {
        }
        
    }

    int cache_set_num = cache_block_num/cache_associativity; // Number of sets


    int offsetBits = log2(cache_block_size);  // Number of offset bits

    int indexBits = log2(cache_set_num);  // Number of index bits

    int addressBits = 32;  
    int tagBits = addressBits - (offsetBits + indexBits);

    int miss = 0;
    int hit = 0;
    int evictions = 0;
    int accesses = 0;  // Keeps track of all cache acceses, hits, misses and evicitons

    
    
    CacheBlock** cache = new CacheBlock*[cache_set_num];
    for (int i = 0; i < cache_set_num; i++)
        cache[i] = new CacheBlock[cache_associativity];

    read(input_filename, tagBits, offsetBits, cache, cache_associativity, eviction_policy, accesses, hit, miss, evictions);
    
    write(output_filename, hit, miss, accesses, evictions);
      
    for (int i = 0; i < cache_set_num; ++i) 
        delete[] cache[i];
    
    delete[] cache;


    std::cout << "in_filename: " << input_filename << std::endl; 
    std::cout << "out_filename: " << output_filename << std::endl;
    std::cout << "num of cache blocks: " << cache_block_num << std::endl;
    std::cout << "size of blocks: " << cache_block_size << std::endl;
    std::cout << "associativity: " << cache_associativity << std::endl;
    std::cout << "eviction policy: " << eviction_policy << std::endl;


}