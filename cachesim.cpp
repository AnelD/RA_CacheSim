#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <string>
#include <sstream>
#include <ctime>

#define TIME_TYPE std::chrono::high_resolution_clock::time_point

// TODO
// Pass cache parameters through console [X]
// Clean up Code []
// 
// Macros for long typename -> timestamp [X]
// Have Output not be overwritten [X]
// Timestamp when benchmark was performed and used arguments  [X]
// FIX crafty_mem.trace not working -> out of range exception @stoul() [?]
// -> changing stoul() to stoull() fixes it but the address is atleast 33 bits long so simulation might give wrong results 
// 
// Write allocate [X]

// Struct that keeps track of all relevant information of a single cache block
struct CacheBlock
{
    unsigned int tag;
    bool valid;
    
    // Timestamp for evicition eviction_policy
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
// eviction_policy: 0 == LRU,  1 == FIFO,  2 == Random
void store( CacheBlock** cache, unsigned int address, int tagBits, 
            int offsetBits, int associativity, int eviction_policy, int &evictions, 
            int &accesses, int &read_hit, int &read_miss, int &write_hit, int &write_miss, 
            int writing_policy, bool call_by_load)
{
    unsigned int tag = adr_to_tag(address, tagBits);
    unsigned int index = adr_to_index(address, tagBits, offsetBits);

    // Check if data is already in cache
    // If not decide with write policy to fetch it or not 
    // 0 == No allocate 1 == Write allocate
    bool tag_not_found = true;
    
    for (int i = 0; i < associativity; i++)
    {
        if (cache[index][i].tag == tag)
        {  
            // Write hit
            write_hit++;
            accesses++;
            tag_not_found = false;
            if(eviction_policy == 0)
                cache[index][i].timestamp = getTimestamp();
        }
       
    } 

    // Write miss
    if(tag_not_found)
    {
        write_miss++;
        accesses++;
    }
    
    // If No allocate as write policy and function was called by load function
    // We don't fetch the memory 
    if((writing_policy == 0) && call_by_load)
        return ;
    

    // Check if all blocks in a set are already filled (valid = true)
    // If one is found that isnt filled fill it
    bool set_full = false;

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
    // If all are filled decide which to overwrite using the chosen eviction eviction_policy

    // Overwrite using LRU eviction_policy
    if(set_full && (eviction_policy == 0))
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

    // Overwrite using FIFO eviction_policy
    else if(set_full && (eviction_policy == 1))
    {
        TIME_TYPE lru_t = cache[index][0].timestamp;
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

    else if(set_full && (eviction_policy == 2))
    {   
        int rand_index = rand() % associativity;

        cache[index][rand_index].tag = tag;
        cache[index][rand_index].valid = true;
        cache[index][rand_index].timestamp = getTimestamp();
        evictions++;
    }
}


// Function to read a cache entry 
// if entry exists hit++ else miss++ and load that entry according to eviction eviction_policy
void load(CacheBlock** cache, unsigned int address, int tagBits, 
            int offsetBits, int associativity, int eviction_policy, 
            int &read_miss, int &read_hit, int &write_miss, int &write_hit, int &evictions, int &accesses, int writing_policy)
{
    

    unsigned int tag = adr_to_tag(address, tagBits);
    unsigned int index = adr_to_index(address, tagBits, offsetBits);

    // Search for tag in the set
    // If tag is in set hit++ else miss++ and load tag into set

    bool tag_not_found = true;
    
    for (int i = 0; i < associativity; i++)
    {
        if (cache[index][i].tag == tag)
        {  
            accesses++;
            read_hit++;
            tag_not_found = false;
            if(eviction_policy == 0)
                cache[index][i].timestamp = getTimestamp();
        }
       
    } 
    
    if(tag_not_found)
    {
        accesses++;
        read_miss++;
        store(  cache, address, tagBits, offsetBits, associativity, 
                eviction_policy, evictions, accesses, read_hit, read_miss, write_hit, write_miss, writing_policy, 0);
    }

}

// Function to read a trace file and extract the relevant information
// [#][Load==0/Store==1][32-Bit Adress][Irrelevant]
// Needs to read 2 numbers after # in every line
// First number chooses which function gets called 
// Second number gets split up into tag and index  
void read(  std::string filename, int tagBits, int offsetBits, CacheBlock** cache, 
            int associativity, int eviction_policy, int &accesses, int &read_hit, 
            int &read_miss, int& write_hit, int& write_miss, int &evictions, int writing_policy)
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
                // stoul converts to unsigned int but seems to work fine
                // out of range excepetion with crafty_mem.trace
                address = std::stoull(hexNum, nullptr, 16); 

                //std::cout << "L/S: " << load_store << ", Adress: " << address << ", Number 3: " << ignoredNum << std::endl;
                
                // Extract tag and index from adress ?
               
                line_nr++;

            }


            // Loads address from cache
            if(load_store == 0)
            {
                load(   cache, address, tagBits, offsetBits, associativity, 
                        eviction_policy, read_miss, read_hit, write_miss, write_hit, 
                        evictions, accesses, writing_policy);
            }


            // Stores address into cache
            else if (load_store == 1)
            {
                store(  cache, address, tagBits, offsetBits, associativity, 
                        eviction_policy, evictions, accesses, read_hit, read_miss, write_hit, write_miss, writing_policy, 1);
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
void write( std::string filename, int read_hit, int read_miss, int write_hit, 
            int write_miss, int accesses, int evictions, 
            int cache_block_num, int cache_block_size, int associativity, 
            int eviction_policy, std::string inputfile, int writing_policy)
{
    // Timestamping
    // Get the current system time
    auto currentTime = std::chrono::system_clock::now();

    // Convert the current system time to a time_t object
    std::time_t time = std::chrono::system_clock::to_time_t(currentTime);

    // Convert the time_t object to a tm struct
    std::tm* timeInfo = std::localtime(&time);

    // Extract the day, hour, and month from the tm struct
    int min = timeInfo->tm_min;
    int day = timeInfo->tm_mday;
    int hour = timeInfo->tm_hour;
    int month = timeInfo->tm_mon + 1; // tm_mon is zero-based, so add 1
       
    float miss_rate = 100.0/accesses*(write_miss+read_miss);

    std::ofstream outputFile;

    outputFile.open(filename, std::ios::app); // Append data without overwriting

    if (outputFile.is_open()) 
    {
        // Timestamp
        outputFile << "--------------------------------------------";
        outputFile << "--------------------------------------------" << std::endl;
        outputFile << month << "/" << day << " @" << hour; 
        outputFile << ":" << min << " Input File: " << inputfile << std::endl << std::endl;

        // Write data to the file
        outputFile << "Cache evictions: " << evictions << std::endl;
        outputFile << "Cache accesses: " << accesses << std::endl;
        outputFile << "Cache read hits: " << read_hit << " | Cache write hits: " << write_hit << std::endl;
        outputFile << "Cache read miss: " << read_miss << " | Cache write miss: " << write_miss << std::endl;
        outputFile << "Cache miss total: " << read_miss+write_miss << " | Cache hit total: " << read_hit+write_hit << std::endl;
        outputFile << "Cache miss rate: " << miss_rate << " %" << std::endl;

        // Parameters used
        outputFile << "Number of cache blocks: " << cache_block_num;
        outputFile << ", Size of blocks: " << cache_block_size;
        outputFile << ", Associativity: " << associativity;
        outputFile << ", Eviction policy: " << eviction_policy; 
        outputFile << ", Writing policy: " << writing_policy << std::endl;

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
    // RNG seed
    std::srand(std::time(0));


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
    int eviction_policy = 0;    // eviction policy: 0 == LRU,  1 == FIFO,  2 == Random
    int writing_policy = 0; // writing policy: 0 == No allocate,    1 == Write allocate

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
            if(nextArg == "NoAlloc")
                writing_policy = 0;
            else if (nextArg == "Alloc")
                writing_policy = 1;
        }
        
    }

    int cache_set_num = cache_block_num/cache_associativity; // Number of sets


    int offsetBits = log2(cache_block_size);  // Number of offset bits

    int indexBits = log2(cache_set_num);  // Number of index bits

    int addressBits = 32;  
    int tagBits = addressBits - (offsetBits + indexBits);


    int write_hit = 0;
    int write_miss = 0;
    int read_miss = 0;
    int read_hit = 0;
    int evictions = 0;
    int accesses = 0;  // Keeps track of all cache acceses, hits, misses and evicitons

    
    
    CacheBlock** cache = new CacheBlock*[cache_set_num];
    for (int i = 0; i < cache_set_num; i++)
        cache[i] = new CacheBlock[cache_associativity];

    read(   input_filename, tagBits, offsetBits, cache, cache_associativity, 
            eviction_policy, accesses, read_hit, read_miss, write_hit, write_miss, evictions, writing_policy);
    
    write(  output_filename, read_hit, read_miss, write_hit, write_miss, accesses, evictions, cache_block_num, 
            cache_block_size, cache_associativity, eviction_policy, input_filename, writing_policy);

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