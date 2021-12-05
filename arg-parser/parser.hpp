#pragma once

//Native C++ Libraries
#include <iostream>                //I/O operations
#include <iomanip>                //I/O formatting (std::boolalpha)
#include <string>                //Representing commandline arguments
#include <string_view>          //Passing/viewing arguments without allocating new resources
#include <unordered_map>       //For querying arguments
#include <regex>              //Regular expressions for matching commandline arguments
#include <type_traits>       //std::is_same_v
#include <stdexcept>        //std::invalid_argument

//Custom Header files
#include "argument.hpp"

namespace arg_parser
{
    //Class 'Parser' represents a cmdline argument parser and its data
    class Parser final
    {
        private:
            //A map of all the commandline arguments, mapped to their values
            std::unordered_map<std::string, Argument> arg_map;

        public:
            //Special methods
            template <typename ...Args>
            Parser(Args&&...);

            //Functions
            void parse(int, char*[]);
            void throw_if_req_not_set() const;

            //Overloaded operators
            [[nodiscard]] const Argument& operator[](const std::string&) const;


            // *** AID: DEBUGGING *** //
            void print_map() const;
    };



    // ***** FUNCTION IMPLEMENTATION ***** //

    //Constructor
    template <typename ...Args>
    Parser::Parser(Args&&... args)
    {
        //Make sure all the arguments are 'Argument' objects (if using C++20, change this to a 'requires' clause)
        static_assert((std::is_same_v<Args, Argument> and ...), "whoops, not all the arguments were \'Argument\' objects :P");

        //Insert all the arguments into the map
        (arg_map.insert({args.name, std::move(args)}), ...);   //move arguments since they own resources
    }

    //Parse the commandline arguments (toggle proper values in the map)
    void Parser::parse(int argc, char* argv[])
    {
        //Regex for commandline arguments: matches with any alphabetic string that starts with '-' or '--' (also allows hyphens)
        //const static std::regex arg_pattern(R"((-|--)[a-zA-Z\-]+)");
        std::string current_arg;

        //Process all the arguments
        for(int i=1; i < argc; ++i)   //safe to use 'int' over 'std::size_t' because max is INT_MAX
        {
            if (arg_map.find(argv[i]) != arg_map.end())  //second condition prevents injection of new args
            {
                current_arg = argv[i];                  //Set the current argument
                arg_map[current_arg].set = true;       //Mark the argument as seen
            }
            else  //add whatever follows to the list of current arguments
            {
                if (arg_map.find(current_arg) != arg_map.end())      //Protect against stray input (parameters without arguments)
                    arg_map[current_arg].params.push_back(argv[i]);
            }
        }
    }


    //Return an argument object
    [[nodiscard]] const Argument& Parser::operator[](const std::string& arg) const
    {
        //Return the Argument object associated with the string -- CAN THROW std::out_of_range
        return arg_map.at(arg);
    }

    //Throw an error if required arguments are not set
    void Parser::throw_if_req_not_set() const
    {
        //List containing all the required arguments
        std::string error_list;

        //Ensure all required arguments were provided
        std::for_each(arg_map.cbegin(), arg_map.cend(), [&error_list](auto argument) 
                                                        { 
                                                            if (argument.second.required and not argument.second.set) 
                                                                error_list += argument.second.name + ", ";
                                                        });

        
        if (error_list.length() != 0)  //if (error_list.length())
         throw std::invalid_argument("Fatal error, required arguments not included: " + error_list);
    }

    // FOR DEBUGGING //
    void Parser::print_map() const
    {
        std::cout << std::boolalpha;

        for(const auto& map_entry : arg_map)
        {
            std::cout << map_entry.first << "=\n"
            << "> Required: " << map_entry.second.required << '\n'
            << "> Set: " << map_entry.second.set << '\n'
            << "> Params: ";

            //Print all the parameters
            for(const auto& param : map_entry.second.params)  //const std::string_view&
            {
                std::cout << param << " ";
            }
            std::cout << '\n';
        }
    }
}

