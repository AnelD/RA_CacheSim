#include <iostream>
#include <cmath>
#include <string>


unsigned long adr_to_tag(unsigned long adr, int tagBits)
{
    // Number of bits in unsigned long, size of returns bytes so *8 for bits
    
    int size_of_uint = sizeof(unsigned long)*8;  

    int shift_amount = size_of_uint - tagBits;
    
    unsigned long r = adr >> shift_amount;

    return r;

}


// Function that splits the 32-Bit adress into INDEX bits
// Address:  [TAG][INDEX][OFFSET]
unsigned long adr_to_index(unsigned long adr, int tagBits, int offsetBits)
{

    // Number of bits in unsigned long, size of returns bytes so *8 for bits
    
    int size_of_uint = sizeof(unsigned long)*8;  

    int shift_left_amount = tagBits;

    int shift_right_amount = shift_left_amount + offsetBits;
    
    unsigned long r = adr << shift_left_amount;
    r = r >> shift_right_amount; 

    return r;


}

int main()
{

    int offsetBits = log2(32);  // Number of offset bits

    int indexBits = log2(256);  // Number of index bits

    int addressBits = 32;  
    int tagBits = addressBits - (offsetBits + indexBits);

    std::string hexNum = "1400e8278";

    unsigned long adr = stoull(hexNum, nullptr, 16);

    int index = adr_to_index(adr, tagBits, offsetBits);
    int tag = adr_to_tag(adr, tagBits);

    std::cout << "Index: " << index << " Tag: " << tag;
    
}