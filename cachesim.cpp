#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <cstdlib>


// Struct that keeps track of all relevant information of a single cache block
struct CacheBlock
{
    unsigned int tag;
    bool valid;
    
    // Timestamp for evicition policy
    std::chrono::high_resolution_clock::time_point timestamp; 

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
std::chrono::high_resolution_clock::time_point getTimestamp() 
{
    return std::chrono::high_resolution_clock::now();
}

// Function to store into cache
// policy: 0 == LRU,  1 == FIFO,  2 == Random
void store( CacheBlock** cache, unsigned int adress, int tagBits, 
            int offsetBits, int associativity, int policy, int evictions, int accesses)
{
    unsigned int tag = adr_to_tag(adress, tagBits);
    unsigned int index = adr_to_index(adress, tagBits, offsetBits);

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
        std::chrono::high_resolution_clock::time_point 
                    lru_t = cache[index][0].timestamp; // Ugly
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
void load(CacheBlock** cache, unsigned int adress, int tagBits, 
            int offsetBits, int associativity, int policy, int miss, int hit, int evictions, int accesses)
{
    accesses++;

    unsigned int tag = adr_to_tag(adress, tagBits);
    unsigned int index = adr_to_index(adress, tagBits, offsetBits);

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
        store(cache, adress, tagBits, offsetBits, associativity, policy, evictions, accesses);
    }

}




int main(int arg, char** argv)
{
    int cache_block_num = 1024; // == Number of cache blocks
    
    int cache_associtivity = 2; // Cache associativity 

    int cache_set_num = cache_block_num/cache_associtivity; // Number of sets

    int cache_block_size = 32; // Size of cache blocks in bytes

    int offsetBits = log2(cache_block_size);  // Number of offset bits

    int indexBits = log2(cache_set_num);  // Number of index bits

    int addressBits = 32;  
    int tagBits = addressBits - (offsetBits + indexBits);

    CacheBlock** cache = new CacheBlock*[cache_set_num];
    for (int i = 0; i < cache_set_num; i++)
        cache[i] = new CacheBlock[cache_associtivity];


    for (int i = 0; i < cache_set_num; ++i) 
        delete[] cache[i];
    
    delete[] cache;
}