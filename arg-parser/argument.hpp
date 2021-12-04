#pragma once

//Native C++ Libraries
#include <string>
#include <string_view>
#include <vector>

namespace arg_parser
{
    //Class 'Argument' represents a commandline argument and its properties
    struct Argument final 
    {
        //Data members
        std::string name;                             //The name (identifier) of the argument
        bool required;                               //Whether or not the argument is required
        bool set;                                   //Whether or not the argument was used in the cmd args
        std::vector<std::string_view> params;      //List of the parameters (view of the strings in char* argv[])

        //Constructor (default + parameterized)
        Argument(std::string in_name = "", bool in_required = false)
        {
            name = in_name;
            required = in_required;
            set = false;
        }
    };
}
