#include "domain/UserFactory.h"

#include "common/Exceptions.h"
#include "common/Validation.h"

namespace lms {

std::unique_ptr<User> UserFactory::create(const NewUserSpec& spec) const {
    const std::string username = validation::requireUsername(spec.username);
    validation::requirePassword(spec.password, username);
    const std::string hash = hasher_.hash(spec.password);

    UserRecord rec;
    rec.id = 0;
    rec.username = username;
    rec.passwordHash = hash;
    rec.role = spec.role;
    switch (spec.role) {
        case UserRole::Student:
            rec.studentNo = validation::requireNonBlank(spec.studentNo, "Student number", 30);
            break;
        case UserRole::Lecturer:
            rec.staffNo = validation::requireNonBlank(spec.staffNo, "Staff number", 30);
            break;
        case UserRole::HoD:
            if (spec.departmentId <= 0) throw ValidationError("A Head of Department must be linked to a department.");
            rec.departmentId = spec.departmentId;
            break;
        case UserRole::Admin:
            break;
    }
    return hydrate(rec);
}

std::unique_ptr<User> UserFactory::hydrate(const UserRecord& r) {
    switch (r.role) {
        case UserRole::Student:
            return std::make_unique<Student>(r.id, r.username, r.passwordHash, r.studentNo);
        case UserRole::Lecturer:
            return std::make_unique<Lecturer>(r.id, r.username, r.passwordHash, r.staffNo);
        case UserRole::HoD:
            return std::make_unique<HoD>(r.id, r.username, r.passwordHash, r.departmentId);
        case UserRole::Admin:
            return std::make_unique<Admin>(r.id, r.username, r.passwordHash);
    }
    throw ValidationError("Unknown role.");
}

}  // namespace lms
