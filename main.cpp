/*
    Students: Michael Gain and Ethan Cox
    Date: 12/2/2021
    Class: Data Structures and Algorithms (CSC-2710-01)
    Project: Hashing Implementation Project > Password Cracker
    C++ Version: C++17

    Compilation Instructions: 
        > Windows: g++ -std=c++17 *.cpp .\hashlib++\*.cpp
        > Linux: g++ -std=c++17 *.cpp ./hashlib++/*.cpp

    Description: this program is a password cracker for md5 hashes. Is it realistic? No, but it's still a good exercise.
*/

//Native C++ Libraries
#include <iostream>              //For input and output operations
#include <iomanip>              //For formatting output as a table
#include <fstream>             //For reading in hashes from a file provided a cmdline argument
#include <unordered_map>      //For storing the map of hashes to their cracked equivalent
#include <optional>          //For optional values (in the map)
#include <memory>           //For smart pointers

//Native C Libraries
#include <cstdlib>      //Contains exit()

//External Libraries (dependencies)
#include "hashlib++/hashlibpp.h"  //Contains implmentations of MD5 and SHA-family hashing algorithms

//Typedefs
typedef std::unordered_map<std::string, std::optional<std::string>> hashmap; 


//Function prototypes
void validateCmdArgs(int argc);
void loadHashes(hashmap hashes, std::string filename);


// DRIVER CODE //
int main(int argc, char* argv[])
{
    //MD5 hash generator
    auto md5hasher = std::make_unique<hashwrapper>();

    //Variables
    hashmap hashes;    //Set of all the hashes to crack


    std::cout << md5hasher->getHashFromString("Hello world") << '\n';

    //validateCmdArgs(argc);                   //Validate the commandline arguments (check that a file WAS provided)
    //loadHashes(hashes, argv[1]);            //Load in all the hashes from the file

    return 0;
}




  // =============================================== //
 // *********** FUNTION IMPLEMENTATIONS **********  //
// =============================================== //

//Validates the commandline arguments (so 'loadHashes' doesn't have it)
inline void validateCmdArgs(int argc)
{
    if (argc < 2)
    {
        std::clog << "usage: ./a.out <file>\n";
        exit(1);
    }
    
    if (argc > 2)
    {
        std::clog << "Non-fatal error: too many arguments.\n"; 
    }
}


//Load the hashes from the given file
void loadHashes(hashmap hashes, std::string filename)
{
    //Infile to read in hashed passwords from + temp str to store individual passwords
    std::fstream inFile(filename);
    std::string password;

    //Error-handling
    if (!inFile.good())
    {
        std::clog << "The file " << std::quoted(filename) << " could not be found. Exiting...\n";
        exit(2);
    }
    
    //Until you reach the end of the file
    while (std::getline(inFile, password))   //implicit std::noskipws
    {
        //Move the hashed password into the map, along with an empty std::optional<> object
        hashes.insert({std::move(password), std::nullopt});
    }
}
