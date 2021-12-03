# Password Cracker in C++
An MD5 password cracker build in C++

# Compilatition and Execution

# Piping
The output table is designed to be friendly for piping, so a simple `./a.out hashes.txt | awk 'NR > 3'` skips the progress counter and  table header, giving you just the 
original hashes and cracked passwords separated by a space. If a hash was not cracked, the space under the column _CRACKED PASSWORDS_ should be empty.

# Process
