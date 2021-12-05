#pragma once

//Native C++ Libraries
#include <string>
#include <string_view>
#include <vector>

namespace arg_parser
{
    //Class 'Argument' represents a commandline argument and its properties
    class Argument final 
    {
        private:
            //Data members
            std::string name;                              //The name (identifier) of the argument
            std::string desc;                             //Description of the argument
            bool required;                               //Whether or not the argument is required
            bool set;                                   //Whether or not the argument was used in the cmd args
            std::vector<std::string_view> params;      //List of the parameters (view of the strings in char* argv[])
            unsigned short num_of_params;             //Amount of parameters (minimum number; max. not enforced)

        public:
            //Special methods
            Argument(std::string = "", unsigned short = 0, bool = false, std::string = "");    //Constructor (default +  parameterized)

            //General methods
            [[nodiscard]] bool is_set() const noexcept;     
            [[nodiscard]] bool is_required() const noexcept; 
            [[nodiscard]] std::size_t param_count() const noexcept;
            [[nodiscard]] std::string info() const noexcept;

            //Overloaded operators
            std::string_view operator[](std::size_t) const;

            //Friends
            friend class Parser;   //Allow direct access for efficient parsing
    };


    // ***** SPECIAL METHODS ***** //

    //Constructor (default + parameterized)
    Argument::Argument(std::string in_name, unsigned short param_count, bool in_required, std::string description)
    {
        name = in_name;
        num_of_params = param_count;
        desc = description;
        required = in_required;
        set = false;
    }


    // ***** GENERAL METHODS ***** //
    
    //Return whether or not the argument was toggled at runtime
    [[nodiscard]] bool Argument::is_set() const noexcept
    {
        return set;
    }

    //Return whether or not the argument is required
    [[nodiscard]] bool Argument::is_required() const noexcept
    {
        return required;
    }

    //Return the number of parameters passed at runtime
    [[nodiscard]] std::size_t Argument::param_count() const noexcept
    {
        return params.size();
    }
    
    //Return a summary of the info of the argument
    [[nodiscard]] std::string Argument::info() const noexcept
    {
        //I will not use something as slow as stringstream and that is a hill I will die on.
        return  "Name: " + name + '\n' 
             +  "Description: " + desc + '\n'
             +  "Required: " + (required ? "true\n" : "false\n")
             +  "Number of Parameters: " + std::to_string(num_of_params) + '\n';
    }


    // ***** OVERLOADED OPERATORS ***** //

    //[] operator grants access to the list of parameters
    std::string_view Argument::operator[](std::size_t idx) const
    {
        //Safely return params[idx]
        return params.at(idx);   //CAN THROW std::out_of_range
    }
}
