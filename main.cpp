/*
    Students: Michael Gain and Ethan Cox
    Date: 12/2/2021
    Class: Data Structures and Algorithms (CSC-2710-01)
    Project: Hashing Implementation Project > Password Cracker
    C++ Version: C++17

    Compilation Instructions: 
        > Windows: g++ -std=c++17 *.cpp .\hashlib++_md5\*.cpp
        > Linux:   g++ -std=c++17 *.cpp ./hashlib++_md5/*.cpp

    Description: this program is a password cracker for md5 hashes. Is it realistic? No, but it's still a good exercise.
*/
 
//Native C libraries
#include <cstdlib>      //contains exit()

//Native C++ Libraries
#include <iostream>              //For input and output operations
#include <iomanip>              //For formatting output as a table
#include <fstream>             //For reading in hashes from a file provided a cmdline argument
#include <unordered_map>      //For storing the map of hashes to their cracked equivalent
#include <optional>          //For optional values (in the map)
#include <memory>           //For smart pointers
#include <algorithm>       //The cardinal sin in an algorithms class 
#include <vector>         //I know this is slow but im only using it for writing hashes to a file

//External Libraries (dependencies)
// #include "hashlib++/hashlibpp.h"  //Contains implmentations of MD5 and SHA-family hashing algorithms
#include "hashlib++_md5/hashlibpp.h" //this directory contains a modified version of the hashlib++ library which only contains the code for md5. hopefully this will speed up compilation

//Custom Libraries (by yours truly :D)
#include "arg-parser/parser.hpp"          //By Ethan
#include "permuter/permute.hpp"          //By Michael

//Typedefs
typedef std::unordered_map<std::string, std::optional<std::string>> passwd_hashmap; 


//Function prototypes
void process_args(int argc, arg_parser::Parser&);                     //Ensure that there was a file to read from
void load_hashes(passwd_hashmap& hashes, std::string filename);      //Load the hashes from the file
void crack_hashes(passwd_hashmap& hashes, std::string filename);    //(Attempt to) crack all the hashes
void print_hashes(const passwd_hashmap& hashes);                   //Print all the hashes + cracked passwords as a table
void crack_brute_hash(passwd_hashmap& hashes, const size_t& size = 5);   //(Attempt to) crack all the hashes with passwords of a given size 
                                                                 //(probably dont want to run larger than 5 or youll have time to discover the cure to cancer)                                     
void gen_hash_to_file(std::string& file_name, std::initializer_list<std::string> plaintext);         //Hashes plaintext and puts in a file

// DRIVER CODE //
int main(int argc, char* argv[])
{
    //Commandline argument parser + argument list
    arg_parser::Parser parser(
                                //Format:  cmd arg=string, # of parameters=uint, is_required=bool, description=string
                                arg_parser::Argument("--help", 0, false, "displays the help screen"),            
                                arg_parser::Argument("-h", 0, false, "displays the help screen"),                 
                                arg_parser::Argument("--hashfile", 1, true, "takes the list of hashed passwords"),   
                                arg_parser::Argument("--dict", 1, false, "source dictionary of passwords"),
				arg_parser::Argument("--brute", 1, false, "runs the brute force algorithm which does not require a dictionary. 1 arg: size of password")
                             );

    //Parse the commandline arguments
    parser.parse(argc, argv);
    process_args(argc, parser);                                    //Validate the commandline arguments (check that a file WAS provided)

    //Variables
    passwd_hashmap hashes;  //map of all the hashes to crack (password hash -> optional<cracked password value>)
    std::string dictionary = (parser["--dict"].is_set() ? parser["--dict"][0].data() : "top-10-million-passwords.txt");
    size_t size = (parser["--brute"].is_set() ? std::stoi(parser["--brute"][0].data()) : (size_t)5);

    load_hashes(hashes, parser["--hashfile"][0].data());          //Load in all the hashes from the file

	if(parser["--brute"].is_set())
		crack_brute_hash(hashes, size);
	else
    	crack_hashes(hashes, dictionary);                       //Attempt to crack all the hashes

    print_hashes(hashes);                                       //Print all the hashes and their cracked equivalents as a table

    return 0;
}




  // =============================================== //
 // *********** FUNTION IMPLEMENTATIONS **********  //
// =============================================== //

//Validates the commandline arguments (so 'load_hashes' doesn't have it)
inline void process_args(int argc, arg_parser::Parser& parser)
{
    //If no commandline arguments were provided OR the user asked for help
    if (argc == 1 or parser["-h"].is_set() or parser["--help"].is_set())   //Done in shortest order or evaluation for short-circuiting
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
                  << "========\n";


                  //Print the information for each of the arguments
           
                  parser.print_arg_info();

				  std::cout << "\nContributors: Ethan Cox and Michael Gain :)\n";

        exit(0);    //Exiting early via help is not erroneous
    }

    //Throw std::invalid_argument if not all the required arguments are set + parameters needed
    parser.validate_args();

    //Make sure the user provided a file
    if (not parser["--hashfile"].is_set())
    {
        std::clog << "usage: ./a.out <password_hashlist>\n";
        exit(1);
    }
}


//Load the hashes from the given file
void load_hashes(passwd_hashmap& hashes, std::string filename)
{
    //Infile to read in hashed passwords from + temp str to store individual passwords
    std::ifstream password_hashlist(filename);
    std::string password;

    //Error-handling
    if (not password_hashlist.good())
    {
        std::clog << "***FATAL ERROR***: the file " << std::quoted(filename) << " could not be found. Exiting with status code 2...\n";
        exit(2);
    }
    
    //Until you reach the end of the file
    while (std::getline(password_hashlist, password))   //implicit std::noskipws
    {
        //Move the hashed password into the map, along with an empty std::optional<> object
        hashes.insert({std::move(password), std::nullopt});
    }
	
    password_hashlist.close();
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
    if (not dictionary.good())
    {
        std::clog << "***FATAL ERROR***: the file " << std::quoted(filename) << " could not be found. Exiting with status code 2...\n";
        exit(2);
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

   dictionary.close();
}

void crack_brute_hash(passwd_hashmap& hashes, const size_t& size )
{   
    //Variables
    auto md5hasher = std::make_unique<md5wrapper>();       //MD5 Hash Generator
    std::string password;                                 //Temp string to store a given password from the dictionary
    unsigned long long counter = 0;                      //Counter -- how many passwords it's gone through
	const std::string endOfPermuter(size, '0');         //Stores the string that corresponds to the end of the permuter. this feels hacky but it works for now

    //Run through all combinations of N size strings and checking if they match the hash in hashes hashmap
    for(password = Permute::gen_brute_str(size); password != endOfPermuter; password = Permute::gen_brute_str(size))
    {
        std::cout << "Progress: " << ++counter << '\r';  // '\r' overwrites the current line, acting as a progress bar

        std::string password_hash = std::move(md5hasher->getHashFromString(password));

        if (hashes.find(password_hash) != hashes.end())
            hashes[password_hash] = password;
    }
	// this code is duplicated because once the permuter reaches the end it loops back to "0"*size so this duplicate is needed to check if that is the password
	std::cout << "Progress: " << ++counter << '\r';  // '\r' overwrites the current line, acting as a progress bar

	std::string password_hash = std::move(md5hasher->getHashFromString(password));

	if (hashes.find(password_hash) != hashes.end())
		hashes[password_hash] = password;

    std::cout << '\n';
}
                                      
void gen_hash_to_file(std::string& file_name, std::initializer_list<std::string> plaintext) { //Hashes plaintext and outputs to file

	auto md5hasher = std::make_unique<md5wrapper>();
	std::ofstream out_file(file_name);

	if(out_file.good()) {
		for(auto i : plaintext) {
			out_file << md5hasher->getHashFromString(i) << "\n";
		}
	}
	else
		std::cout << "hashes not written. file is not good" << std::endl;
	
	out_file.close();
}

//Print a the map of the hashed passwords and the cracked passwords as a table
void print_hashes(const passwd_hashmap& hashes)
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
