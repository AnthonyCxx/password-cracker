#include <iostream>
#include "hashlib++/hashlibpp.h"

int main()
{
    //MD5 hash generator
    hashwrapper *myWrapper = new md5wrapper();
    
    try
    {
            std::cout << myWrapper->getHashFromString("Hello world") << std::endl;
    }
    catch(hlException &e)
    {
            //your error handling here
    }

    //Cleanup
    delete myWrapper;
    myWrapper = NULL;

    return 0;
}