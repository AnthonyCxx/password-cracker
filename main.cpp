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
#include <algorithm>       //The cardinal sin in an algorithms class 

//Native C Libraries
#include <cstdlib>      //Contains exit()

//External Libraries (dependencies)
#include "hashlib++/hashlibpp.h"  //Contains implmentations of MD5 and SHA-family hashing algorithms

//Typedefs
typedef std::unordered_map<std::string, std::optional<std::string>> hashmap; 


//Function prototypes
void validateCmdArgs(int argc);                               //Ensure that there was a file to read from
void loadHashes(hashmap& hashes, std::string filename);      //Load the hashes from the file
void crackHashes(hashmap& hashes, std::string filename);    //(Attempt to) crack all the hashes
void printHashTable(const hashmap& hashes);                //Print all the hashes + cracked passwords as a table


// DRIVER CODE //
int main(int argc, char* argv[])
{
    //Variables
    hashmap hashes;    //map of all the hashes to crack (password hash -> optional<cracked password value>)

    validateCmdArgs(argc);                                       //Validate the commandline arguments (check that a file WAS provided)
    loadHashes(hashes, argv[1]);                                //Load in all the hashes from the file
    crackHashes(hashes, "top-10-million-passwords.txt");      //Attempt to crack all the hashes
    printHashTable(hashes);                                   //Print all the hashes and their cracked equivalents as a table

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
        std::clog << "Non-fatal error: too many arguments -- only filename (first arg) accepted.\n"; 
    }
}


//Load the hashes from the given file
void loadHashes(hashmap& hashes, std::string filename)
{
    //Infile to read in hashed passwords from + temp str to store individual passwords
    std::ifstream hashed_passwords(filename);
    std::string password;

    //Error-handling
    if (!hashed_passwords.good())
    {
        std::clog << "***FATAL ERROR***: the file " << std::quoted(filename) << " could not be found. Exiting...\n";
        exit(2);
    }
    
    //Until you reach the end of the file
    while (std::getline(hashed_passwords, password))   //implicit std::noskipws
    {
        //Move the hashed password into the map, along with an empty std::optional<> object
        hashes.insert({std::move(password), std::nullopt});
    }
}


//(Attemp to) crack all the passwords
void crackHashes(hashmap& hashes, std::string filename)
{   
    //Variables
    auto md5hasher = std::make_unique<md5wrapper>();       //MD5 Hash Generator
    std::ifstream dictionary(filename);                   //File containing the password for the dictionary attack
    std::string password;                                //Temp string to store a given password from the dictionary
    unsigned long long counter = 0;                     //Counter -- how many passwords it's gone through

    //Validate dictionary file
    if (!dictionary.good())
    {
        std::clog << "***FATAL ERROR***: the file " << std::quoted(filename) << " could not be found. Exiting...\n";
        exit(3);
    }

    //Try every password in the password list
    while (std::getline(dictionary, password))
    {
        std::cout << "Progress: " << counter++ << '\r';

        std::string password_hash = std::move(md5hasher->getHashFromString(password));

        if (hashes.find(password_hash) != hashes.end())
            hashes[password_hash] = password;
    }
    std::cout << '\n';
}

//Print a the map of the hashed passwords and the cracked passwords as a table
void printHashTable(const hashmap& hashes)
{
    //Table header
    std::cout << std::setw(16) << "******* PASSWORD HASHES ********" << " ***** CRACKED PASSWORDS *****\n"
                               << "================================" << " =============================\n";
    
    //Print all the password hashes + cracked password (if successful, else '// NOT FOUND //'
    for(const auto& map_entry : hashes)
    {
        std::cout << map_entry.first << " " << (map_entry.second.has_value() ? map_entry.second.value() : "") << '\n';
    }

    //Table end
    std::cout << "***** END OF PASSWORD LIST *****\n";
}
