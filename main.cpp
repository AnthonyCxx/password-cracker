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

//Custom Libraries (by yours truly :D)
#include "arg-parser/parser.hpp"

//External Libraries (dependencies)
#include "hashlib++/hashlibpp.h"  //Contains implmentations of MD5 and SHA-family hashing algorithms

//Typedefs
typedef std::unordered_map<std::string, std::optional<std::string>> passwd_hashmap; 


//Function prototypes
void process_args(int argc, arg_parser::Parser&);                     //Ensure that there was a file to read from
void load_hashes(passwd_hashmap& hashes, std::string filename);      //Load the hashes from the file
void crack_hashes(passwd_hashmap& hashes, std::string filename);    //(Attempt to) crack all the hashes
void print_hash_table(const passwd_hashmap& hashes);               //Print all the hashes + cracked passwords as a table


// DRIVER CODE //
int main(int argc, char* argv[])
{
    //Commandline argument parser + argument list
    arg_parser::Parser parser(
                                arg_parser::Argument("--help", false),          //required=false
                                arg_parser::Argument("-h", false),             //required=false
                                arg_parser::Argument("--hashfile", true)      //required=true
                             );

    //Variables
    passwd_hashmap hashes;    //map of all the hashes to crack (password hash -> optional<cracked password value>)

    //Parse the commandline arguments
    parser.parse(argc, argv);

    process_args(argc, parser);                                         //Validate the commandline arguments (check that a file WAS provided)
    load_hashes(hashes, parser["--hashfile"].params.at(0).data());     //Load in all the hashes from the file
    crack_hashes(hashes, "top-10-million-passwords.txt");             //Attempt to crack all the hashes
    print_hash_table(hashes);                                        //Print all the hashes and their cracked equivalents as a table

    return 0;
}




  // =============================================== //
 // *********** FUNTION IMPLEMENTATIONS **********  //
// =============================================== //

//Validates the commandline arguments (so 'load_hashes' doesn't have it)
inline void process_args(int argc, arg_parser::Parser& parser)
{
    //If no commandline arguments were provided OR the user asked for help
    if (argc == 1 or parser["-h"].set or parser["--help"].set)   //Done in shortest order or evaluation for short-circuiting
    {
        //It's dangerous to go alone! Take this help screen with you.
		std::cout << " ___  ________ _____   _   _           _       _____                _              \n"
			      << " |  \\/  |  _  \\  ___| | | | |         | |     /  __ \\              | |             \n"
	              << " | .  . | | | |___ \\  | |_| | __ _ ___| |__   | /  \\/_ __ __ _  ___| | _____ _ __  \n"
		          << " | |\\/| | | | |   \\ \\ |  _  |/ _` / __| '_ \\  | |   | '__/ _` |/ __| |/ / _ \\ '__| \n"
		          << " | |  | | |/ //\\__/ / | | | | (_| \\__ \\ | | | | \\__/\\ | | (_| | (__|   <  __/ |    \n"
	              << " \\_|  |_/___/ \\____/  \\_| |_/\\__,_|___/_| |_|  \\____/_|  \\__,_|\\___|_|\\_\\___|_|    \n"
 				  
				  << "\n\nWelcome to an MD5 Password Cracker.\n" 
                  << "To crack a list of MD5 Hashed Passwords, run \'./a.out --hashfile <password_hashlist>\'\n"
                  << "> There is an example list of hashes provided as \'Hashes.txt\'\n"

                  << "\n\nOptions: \n"
                  << "========\n"
                  
                  << "-h or --help:\n"
                  << "> Displays this help screen\n"
                  << "> Required=false\n\n"
                  
                  << "--hashfile + filename:\n"
                  << "> following filename is the list of password hashes to crack\n"
                  << "> Required=true\n"


				  << "\n\nContributors: Ethan Cox and Michael Gain :)\n";

        exit(0);    //Exiting early via help is not erroneous
    }

    //Throw std::invalid_argument if not all the required arguments are set
    parser.throw_if_req_not_set();

    //Make sure the user provided a file
    if (!parser["--hashfile"].set || parser["--hashfile"].params.size() == 0)
    {
        std::clog << "usage: ./a.out <password_hashlist>\n";
        exit(1);
    }
}


//Load the hashes from the given file
void load_hashes(passwd_hashmap& hashes, std::string filename)
{
    //Infile to read in hashed passwords from + temp str to store individual passwords
    std::ifstream hashed_passwords(filename);
    std::string password;

    //Error-handling
    if (!hashed_passwords.good())
    {
        std::clog << "***FATAL ERROR***: the file " << std::quoted(filename) << " could not be found. Exiting with status code 2...\n";
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
void crack_hashes(passwd_hashmap& hashes, std::string filename)
{   
    //Variables
    auto md5hasher = std::make_unique<md5wrapper>();       //MD5 Hash Generator
    std::ifstream dictionary(filename);                   //File containing the password for the dictionary attack
    std::string password;                                //Temp string to store a given password from the dictionary
    unsigned long long counter = 0;                     //Counter -- how many passwords it's gone through

    //Validate dictionary file
    if (!dictionary.good())
    {
        std::clog << "***FATAL ERROR***: the file " << std::quoted(filename) << " could not be found. Exiting with status code 3...\n";
        exit(3);
    }

    //Try every password in the password list
    while (std::getline(dictionary, password))
    {
        std::cout << "Progress: " << ++counter << '\r';  // '\r' overwrites the current line, acting as a progress bar

        std::string password_hash = std::move(md5hasher->getHashFromString(password));

        if (hashes.find(password_hash) != hashes.end())
            hashes[password_hash] = password;
    }
    std::cout << '\n';
}

//Print a the map of the hashed passwords and the cracked passwords as a table
void print_hash_table(const passwd_hashmap& hashes)
{
    //Table header
    std::cout << std::setw(16) << "******* PASSWORD HASHES ********" << " ***** CRACKED PASSWORDS *****\n"
                               << "================================" << " =============================\n";
    
    //Print all the password hashes + cracked password (if successful, else <empty str>)
    for(const auto& map_entry : hashes)
    {
        std::cout << map_entry.first << " " << (map_entry.second.has_value() ? map_entry.second.value() : "") << '\n';
    }
}
