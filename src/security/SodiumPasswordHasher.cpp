#include "security/SodiumPasswordHasher.h"

#include <sodium.h>

#include "common/Exceptions.h"

namespace lms {

SodiumPasswordHasher::SodiumPasswordHasher() {
    if (sodium_init() < 0) throw LmsError("Cannot initialise the cryptography library.");
}

std::string SodiumPasswordHasher::hash(const std::string& password) const {
    char out[crypto_pwhash_STRBYTES];
    if (crypto_pwhash_str(out, password.c_str(), password.size(), crypto_pwhash_OPSLIMIT_INTERACTIVE,
                          crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
        throw LmsError("Password hashing failed (out of memory).");
    }
    return std::string(out);
}

bool SodiumPasswordHasher::verify(const std::string& password, const std::string& storedHash) const {
    return crypto_pwhash_str_verify(storedHash.c_str(), password.c_str(), password.size()) == 0;
}

}  // namespace lms
