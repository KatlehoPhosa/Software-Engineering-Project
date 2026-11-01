#pragma once
#include "security/IPasswordHasher.h"

namespace lms {

/// Argon2id via libsodium's crypto_pwhash_str().
/// Why: Argon2id won the Password Hashing Competition and is the OWASP-recommended default;
/// it is memory-hard (resists GPU cracking) and libsodium generates a random 16-byte salt
/// and embeds algorithm, parameters and salt in the encoded string, so the single
/// password_hash column is all that has to be stored.
/// The INTERACTIVE limits (64 MiB, 2 passes) keep login well under 2 s on the school's older PCs (NFR-01).
class SodiumPasswordHasher final : public IPasswordHasher {
public:
    SodiumPasswordHasher();  ///< initialises libsodium; throws LmsError on failure
    std::string hash(const std::string& password) const override;
    bool verify(const std::string& password, const std::string& storedHash) const override;
};

}  // namespace lms
