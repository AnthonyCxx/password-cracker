# Password Cracker in C++
An MD5 password cracker built in C++17.

# Compilatition and Execution
- Compilation: `g17 main.cpp ./hashlib++/*.cpp`
- Execution: `./a.out hashes.txt`

# External Libraries
| Library | Author | Used for |
| ------- | ------ | -------- |
| [hashlib++](http://hashlib2plus.sourceforge.net/) | Benjamin Grüdelbach | MD5 hashing algorithm | 
| [cmdline-arg-parser](https://github.com/EthanC2/cmdline-arg-parser) | me | parsing commandline arguments/including program options |

# Output and Piping
Output goes to the console, which also means it can be redirected to a file. The output table is designed to be friendly for piping, so a simple `./a.out hashes.txt | awk 'NR > 3'` skips the progress counter and table header, giving you just the 
original hashes and cracked passwords separated by a space. If a hash was not cracked, the space under the column _CRACKED PASSWORDS_ should be empty.

# Process
The process for cracking the passwords is pretty straight-forward.
1. Load all the hashes from the file into a map, associating them with an `std::optional<std::string>`, which is the cracked password
2. Attempt to crack the passwords by hashing every password in the given dictionary (here: top-10-million-passwords.txt)
3. Print all the password hashes and the uncovered passwords (where `std::optional<std::string>` has a value)

# License
This project is available under an MIT license; by using this password cracker, you agree to take full responsiblity for how you use it.
