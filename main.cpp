#include <iostream>
#include <chrono>
#include <fstream>
#include <thread>
#include "ckeysstorage.h"
#include "cCmdInterp.hpp"

#define KEY_SIZE 4096

using namespace std;

// usage
// chainsign --daemon name_of_instance out_dir
// chainsign --verify-chain 1st_pub_key --move out_dir
// chainsign --verify sig_file

// ./chainsign --daemon my_instance .
// ./chainsign --verify-chain my_instance-key1.pub --move good_keys
// ./chainsign --verify-file sig_file
int main(int argc, char* argv[])
{
	std::cout << "start main" << std::endl;
	
	if (std::string(argv[1]) == "--daemon")
	{
		cCmdInterp cmdInterp("fifo", argv[2]);
		cmdInterp.setOutDir(std::string(argv[3]));
		std::cout << "start loop" << std::endl;
		cmdInterp.cmdReadLoop();
	}
	
	else if (std::string(argv[1]) == "--verify-chain")
	{
		cCmdInterp cmdInterp;
		cmdInterp.setOutDir(std::string(argv[4]));
		unsigned int ret = cmdInterp.verify(std::string(argv[2]));
		if (ret == -1)
			return 2; // keys verification error
		std::cout << "OK" << std::endl;
	}
	
	else if (std::string(argv[1]) == "--verify-file")
	{
		cCmdInterp cmdInterp;
		return cmdInterp.verifyOneFile(std::string(argv[2]));
	}
    //std::cout << KEY_SIZE << std::endl;
	/*cKeysStorage keyStorage = cKeysStorage();
	keyStorage.RSAVerifyFile("test.txt.sig", "my_instance");*/
    //for (int i = 0; i < 1; ++i)
       // keyStorage.generate(std::to_string(i));

	//keyStorage.sign("test.txt", 0);
	//keyStorage.GenerateRSAKey(KEY_SIZE);
	//keyStorage.RSASignFile("test.txt", "test.sig", 0);

    return 0;
}

// return 0 - OK
// return 1 - other error (exception)
// return 2 - keys verification error
// return 3 - file verification error
