#ifndef CKEYSSTORAGE_H
#define CKEYSSTORAGE_H

#include <string>
#include <vector>
#include <map>
#include <memory>

#include <crypto++/rsa.h>
#include <crypto++/osrng.h>
#include <crypto++/base64.h>
#include <crypto++/files.h>
#include <crypto++/hex.h>

using namespace CryptoPP;

/**
 * @brief Creates a chain of keys. Each new key signed by previous key. Signs files. Verifies files/chain.
 * Generate key (starts chain)
 * Rotate to new key (continues chain)
 * Signs a file.
 * Writes keys.
 * Reads keys (private keys)
*/
class cKeysStorage {

	public:

		cKeysStorage(); ///< starts with no key yet

		cKeysStorage(const std::string &privateKeyFilename); ///< loads the private key and continues the chain

		~cKeysStorage(); ///< simple destructor

			
			/*** 
			 * Signs a disk file with given name. Write signature to signatureFilename.
			 * If signKey==true, then we are signing a new signature file (so we use the key that now is known as previous key)
			 * and normally with signKey==false we are just signing with the current key.
			*/
			void RSASignFile(const std::string& messageFilename, const std::string& signatureFilename, bool signKey);

			/***
			 * Verifies a single signature. The data file is written in signature.
			*/
			bool RSAVerifyFile(const std::string &sigFileName);

			/***
			 * Generates a new key pair, save the pub key to a file.
			*/
			void GenerateRSAKey(unsigned int keyLength, std::string fileName);

			unsigned int getCurrentKey() { return mCurrentKey; } ///< Returns the number of current key.

			void RemoveRSAKey(); ///< forget the current private key

		void saveRSAPrivKey() const;
		void loadRSAPrivKey(std::string filename); // filename == for example "key_1.prv"
		// new format
		void RSASignNormalFile(const std::string& inputFilename, const std::string& signatureFilename, bool signKey);
		bool RSAVerifyNormalFile(const std::string& inputFilename, const std::string& signatureFilename);

	private:
			
			std::map <int, CryptoPP::RSA::PrivateKey> mPrvKeys; ///< list of keys. we usually remember just the current and previous key; e.g. key nr 105 and 106

			/// The caller should operate on the public keys and hold them, e.g. hold variables with them during verification or during creation of chain
			
			void savePubFile(unsigned int numberOfKey, const CryptoPP::RSA::PublicKey& pPubKey, std::string fileName); ///< saves a public key to file
			CryptoPP::RSA::PublicKey loadPubFile(std::string pubKey); ///< returns a loaded from disk public key

		// void savePrivKey();
		unsigned int mCurrentKey; ///< number of current key. Related to the numbers in mPrvKeys.

		AutoSeededRandomPool mRng; ///< random generator
};

#endif // CKEYSSTORAGE_H

