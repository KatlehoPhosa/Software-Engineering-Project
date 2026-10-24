#pragma once
#include <string>

namespace lms {

/// Password hashing abstraction (Dependency Inversion): the domain and services never
/// call a crypto library directly, and tests can substitute a fast fake.
class IPasswordHasher {
public:
    virtual ~IPasswordHasher() = default;
    /// Returns a self-describing salted hash string (algorithm, parameters and salt included).
    virtual std::string hash(const std::string& password) const = 0;
    virtual bool verify(const std::string& password, const std::string& storedHash) const = 0;
};

}  // namespace lms
