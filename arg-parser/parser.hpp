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

//Native C Libraries
#include <cstdlib>  //Contains 'exit(1)'

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
            template <typename ...Args>     //Variadic constructor
            Parser(Args&&...);

            //Functions
            void parse(int, char*[]);
            void validate_args() const;
            void print_arg_info() const;

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
        static_assert((std::is_same_v<Args, Argument> and ...), "[[ARGPARSER]] all arguments of the Parser constructor must be \'Argument\' objects!");

        //Insert all the arguments into the map
        (arg_map.insert({args.name, std::move(args)}), ...);   //move arguments since they own resources (and are temp objects)
    }

    //Parse the commandline arguments (toggle proper values in the map)
    void Parser::parse(int argc, char* argv[])
    {
        //Current commandline argument + regex pattern for arguments
        std::string current_arg;
        const std::regex arg_pattern(R"((-|--)[a-zA-Z-]+)");  //Any series of characters or hyphens that follows a '-' or '--'

        //Process all the arguments
        for(int i=1; i < argc; ++i)   //safe to use 'int' over 'std::size_t' because max value of 'argc' is INT_MAX
        {
            //If parsing a commandline argument and not a parameter
            if (std::regex_match(argv[i], arg_pattern))
            {
                if (arg_map.find(argv[i]) != arg_map.end())  //second condition prevents injection of new args
                {
                    current_arg = argv[i];                  //Set the current argument
                    arg_map[current_arg].set = true;       //Mark the argument as seen
                }
                else  //unrecognized arguments are cause for exiting
                {
                    std::cout << "[[ARGPARSER]] **ERROR**: unrecognized argument: " << argv[i] << '\n';
                    exit(1);
                }
            }
            else  //add the parameter follows to the parameters of the current argument
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
    void Parser::validate_args() const
    {
        //List of errors (required + disincluded args/params) for the arguments and parameters
        std::string arg_errors;
        std::string param_errors;

        //Ensure all required arguments were provided
        std::for_each(arg_map.cbegin(), arg_map.cend(), [&arg_errors, &param_errors](auto arg)   //Equal to [&](auto arg)
                                                        { 
                                                            if (arg.second.required and not arg.second.set) 
                                                                arg_errors += arg.second.name + ", ";

                                                            if (arg.second.set and arg.second.params.size() < arg.second.num_of_params) 
                                                                param_errors += arg.second.name + ": " + std::to_string(arg.second.num_of_params) + " parameters, ";
                                                        });

        //Valdiate that all required arguments were included
        if (arg_errors.length() != 0)  //if (error_list.length())
        {
            std::clog << "[[ARGPARSER]] **Error**: required arguments not included: " << arg_errors << '\n';
            exit(1);
        }


        //Validate that all required parameters for all arguments were included
        if (param_errors.length() != 0)  //if (error_list.length())
        {
            std::clog << "[[ARGPARSER]] **Error**: required parameters not included: " << param_errors << '\n';
            exit(1);
        }
    }

    //Print the information about all the arguments -- useful for writing help menus
    void Parser::print_arg_info() const
    {
        for(auto itr=arg_map.begin(); itr != arg_map.end(); ++itr)
        {
            std::cout << itr->second.info() << '\n';
        }
    }
    
    


    // FOR DEBUGGING //
    void Parser::print_map() const
    {
        std::cout << std::boolalpha;

        for(const auto& map_entry : arg_map)
        {
            std::cout << map_entry.second.info();
            std::cout << "Parameters: ";

            //Print all the parameters
            for(const auto& param : map_entry.second.params)  //const std::string_view&
            {
                std::cout << param << " ";
            }
            std::cout << '\n';
        }
    }
}

