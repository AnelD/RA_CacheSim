#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    std::string tTag = "-t"; // trace file
    std::string nTag = "-n"; // nr of cache blocks
    std::string gTag = "-g"; // size of cache block in bytes
    std::string aTag = "-a"; // associativity
    std::string rTag = "-r"; // eviction policy
    std::string wTag = "-w"; // writing policy
    std::string oTag = "-o"; // output file


    std::string inputFile;
    std::string outputFile;
    int n = 512;
    int g, a, r, w;

    for (int i = 1; i < argc - 1; ++i) 
    { // Start from index 1 to skip the program name
        std::string arg = argv[i];
        std::string nextArg = argv[i + 1];

        if (arg == tTag) 
        {
            inputFile = nextArg;
        } 
        else if (arg == oTag) 
        {
            outputFile = nextArg;
        }
        else if (arg == nTag) 
        {
            try 
            {
                n = std::stoi(nextArg);
            } 
            catch (const std::exception& e) {
                std::cout << "Invalid argument for " << nTag << ": " << nextArg << std::endl;
            }
        }
        else if (arg == gTag) 
        {
            try 
            {
                g = std::stoi(nextArg);
            } 
            catch (const std::exception& e) {
                std::cout << "Invalid argument for " << nTag << ": " << nextArg << std::endl;
            }
        }
        else if (arg == aTag) 
        {
            try 
            {
                a = std::stoi(nextArg);
            } 
            catch (const std::exception& e) {
                std::cout << "Invalid argument for " << nTag << ": " << nextArg << std::endl;
            }
        }
        else if (arg == rTag) 
        {
            try 
            {
                r = std::stoi(nextArg);
            } 
            catch (const std::exception& e) {
                std::cout << "Invalid argument for " << nTag << ": " << nextArg << std::endl;
            }
        }
        else if (arg == wTag) 
        {
        }
        
    }


    // Print the values assigned to variables
    std::cout << "Input file: " << inputFile << std::endl;
    std::cout << "Output file: " << outputFile << std::endl;
    std::cout << "n: " << n << std::endl;
    std::cout << "g: " << g << std::endl;
    std::cout << "a: " << a << std::endl;
    std::cout << "r: " << r << std::endl;
    

}
