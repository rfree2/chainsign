#include "ckeysstorage.h"

#include <memory>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <openssl/rsa.h>
#include <openssl/pem.h>

#include <crypto++/rsa.h>
#include <crypto++/osrng.h>
#include <crypto++/base64.h>
#include <crypto++/files.h>
#include <crypto++/hex.h>


cKeysStorage::cKeysStorage()
{
}

// http://www.codepool.biz/tech-frontier/how-to-use-openssl-generate-rsa-keys-cc.html
bool cKeysStorage::generate(std::string pFileName)
{
    int             ret = 0;
    RSA             *rsa = NULL;
    BIGNUM          *bne = NULL;
    BIO             *bp_public = NULL, *bp_private = NULL;

    int             bits = 4096;
    unsigned long   e = RSA_F4;

    // 1. generate rsa key
    bne = BN_new();
    ret = BN_set_word(bne, e);

    rsa = RSA_new();
    ret = RSA_generate_key_ex(rsa, bits, bne, NULL);

    // 2. save public key
    std::string mFileName = pFileName;
    mFileName += "_pub.pem";
    bp_public = BIO_new_file(mFileName.c_str(), "w+");
    ret = PEM_write_bio_RSAPublicKey(bp_public, rsa);

    // 3. save private key
    mFileName = pFileName;
    mFileName += "_prv.pem";
    bp_private = BIO_new_file(mFileName.c_str(), "w+");
    ret = PEM_write_bio_RSAPrivateKey(bp_private, rsa, NULL, NULL, 0, NULL, NULL);

    BIO_free_all(bp_public);
    BIO_free_all(bp_private);
    RSA_free(rsa);
    BN_free(bne);

    return (ret == 1);
}

bool cKeysStorage::sign(std::string pFileName, unsigned int pNumberOfKey)
{

    return true;
}

bool cKeysStorage::verify(std::string pFileName, unsigned int pNumberOfKey)
{

    return true;
}


RSA* cKeysStorage::createRSAWithFilename(const char * filename, bool pub)
{
	std::cout << "open key " << filename << std::endl;
    FILE * fp = fopen(filename,"rb");

    if(fp == NULL)
    {
		std::cout << "open key error" << std::endl;
        return NULL;
    }
    RSA *rsa= RSA_new();
    if (rsa != NULL)
		std::cout << "rsa address " << rsa << std::endl;

    if(pub)
    {
		std::cout << "rsa open pub key" << std::endl;
        rsa = PEM_read_RSA_PUBKEY(fp, &rsa, NULL, NULL);
    }
    else
    {
		std::cout << "rsa open prv key" << std::endl;
        rsa = PEM_read_RSAPrivateKey(fp, &rsa, NULL, NULL);
    }

    return rsa;
}

void cKeysStorage::GenerateRSAKey(unsigned int keyLength)
{
	using namespace CryptoPP;	
	AutoSeededRandomPool rng;
	// Generate Parameters
	InvertibleRSAFunction params;
	params.GenerateRandomWithKeySize(rng, keyLength);
	
	// Create Keys
	CryptoPP::RSA::PrivateKey privateKey(params);
	CryptoPP::RSA::PublicKey publicKey(params);
	mPrvKeys.push_back(privateKey);
	savePubFile(mPrvKeys.size() - 1, publicKey); // 0, 1, 2 ...
}

void cKeysStorage::RSAVerifyFile(const std::string& fileName) // load sig file
{
	std::string line;
	std::string clearTextFileName;
	int pubicKeyNumber;
	// read public key
	std::ifstream input(fileName);
	input >> line;
	//input >> line;
	input >> pubicKeyNumber;
	std::cout << line << " " << pubicKeyNumber << std::endl;
	for (int i = 0; i < 4; ++i)	// 3 lines
	{
		input >> line;
		std::cout << line;
		input >> line;
		std::cout << line << std::endl;
	}
	
	clearTextFileName = line;
	// load rsa data
	char byte;
	std::vector<char> signature;
	while (!input.eof())
	{
		input.read(&byte, 1);
		signature.push_back(byte);
	}
	
	std::cout << std::endl << "signature" << std::endl;
	for (auto a : signature)
		std::cout << a;
		
	std::cout << std::endl << "start verify" << std::endl;
	loadPubFile(pubicKeyNumber);
}

void cKeysStorage::savePubFile(unsigned int numberOfKey, const CryptoPP::RSA::PublicKey& pPubKey)
{
	std::ofstream mOutFile;
    std::string mOutName(std::to_string(numberOfKey));
    mOutName += ".pub";
    mOutFile.open(mOutName);
    //save header
    mOutFile << "version 1" << std::endl;
    mOutFile << "crypto rsa" << std::endl;
    mOutFile << "size 4096" << std::endl;
    mOutFile << "END" << std::endl;
    mOutFile.close();
    //save data
    mOutFile.open(mOutName, std::ios::out | std::ios::app | std::ios::binary);
    mOutFile.write(reinterpret_cast<const char*>(&pPubKey), sizeof(pPubKey));
    mOutFile.close();
}

void cKeysStorage::loadPubFile(unsigned int numberOfKey)
{
	std::string fileName(std::to_string(numberOfKey));
	fileName += ".pub";
	std::cout << "Public key file: " << fileName << std::endl;
	std::string line;
	std::ifstream input(fileName);
	for (int i = 0; i < 3; i++)
	{
		input >> line;
		std::cout << line << " ";
		input >> line;
		std::cout << line << std::endl;
	}
	input >> line; // END
	
	// load rsa data
	char byte;
	std::vector<char> binaryKey;
	while (!input.eof())
	{
		input.read(&byte, 1);
		binaryKey.push_back(byte);
	}
	
	mCurrentPublicKey = *reinterpret_cast<CryptoPP::RSA::PublicKey*>(binaryKey.data());
}

//https://gist.github.com/TimSC/5251670
void cKeysStorage::RSASignFile(const std::string& messageFilename, const std::string& signatureFilename, unsigned int numberOfKey)
{
	using namespace CryptoPP;

	AutoSeededRandomPool rng;
	std::ifstream mInputFile(messageFilename, std::ios::binary | std::ios::in);
	// get length of file:
    mInputFile.seekg (0, mInputFile.end);
    int mLength = mInputFile.tellg();
    mInputFile.seekg (0, mInputFile.beg);
    
    std::shared_ptr<char> mBuffer (new char[mLength], [](char* p){delete p;});
    mInputFile.read(mBuffer.get(), mLength);
    mInputFile.close();
    
    CryptoPP::RSA::PrivateKey privateKey = mPrvKeys.at(numberOfKey);
	
	RSASSA_PKCS1v15_SHA_Signer privkey(privateKey);
	SecByteBlock sbbSignature(privkey.SignatureLength());
	privkey.SignMessage(
		rng,
		(byte const*) mBuffer.get(),
		mLength,
		sbbSignature);
 
	//Save result
	//FileSink sink(signatureFilename.c_str());
	//sink.Put((byte const*) mBuffer.get(), mLength);
	//FileSink sinksig(signatureFilename.c_str());
	//sinksig.Put(sbbSignature, sbbSignature.size());
	std::ofstream output(signatureFilename);
	output << "id-nr " << numberOfKey << std::endl;
	output << "key-ver 1" << std::endl;
	output << "key-crypto rsa" << std::endl;
	output << "key-size 4096" << std::endl;
	output << "cleartext-file"<< messageFilename << std::endl;
	output << "END" << std::endl;
	//output.close();
	//output.open(signatureFilename, std::ios::out | std::ios::app | std::ios::binary);
	output.write(reinterpret_cast<const char*>(&sbbSignature), sbbSignature.size());
	output.close();
}


/*
.pub format:
id-instance inst20141120_1242
id-nr 1
key-ver 1
key-crypto rsa
key-size 4096
END
<after this line "END" we paste here the RSA data: of the public key>
   .sig format:
id-instance inst20141120_1242
id-nr 1
key-ver 1
key-crypto rsa
key-size 4096
cleartext-file foo1.log
END
<after this line "END" we paste here the RSA data: of signature>
*/
