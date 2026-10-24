#pragma once
#include <memory>
#include <string>
#include <vector>

#include "domain/Action.h"
#include "domain/Enums.h"

namespace lms {

/// Flat representation of one account (USER row + the role's subtype row).
/// Used to move users between the domain and data-access layers.
struct UserRecord {
    int id = 0;
    std::string username;
    std::string passwordHash;
    UserRole role = UserRole::Student;
    std::string studentNo;   ///< STUDENT only
    std::string staffNo;     ///< LECTURER only
    int departmentId = 0;    ///< HOD only
};

/// Abstract base of the four roles. Polymorphism supplies the role and the permitted actions.
class User {
public:
    virtual ~User() = default;
    User(const User&) = delete;
    User& operator=(const User&) = delete;

    int id() const { return id_; }
    const std::string& username() const { return username_; }
    const std::string& passwordHash() const { return passwordHash_; }

    virtual UserRole role() const = 0;
    virtual std::vector<Action> allowedActions() const = 0;
    virtual UserRecord toRecord() const = 0;

    bool can(Action a) const;
    std::string roleLabel() const { return label(role()); }

protected:
    User(int id, std::string username, std::string passwordHash)
        : id_(id), username_(std::move(username)), passwordHash_(std::move(passwordHash)) {}

private:
    int id_;
    std::string username_;
    std::string passwordHash_;
};

class Student final : public User {
public:
    Student(int id, std::string username, std::string hash, std::string studentNo)
        : User(id, std::move(username), std::move(hash)), studentNo_(std::move(studentNo)) {}
    const std::string& studentNo() const { return studentNo_; }
    UserRole role() const override { return UserRole::Student; }
    std::vector<Action> allowedActions() const override;
    UserRecord toRecord() const override;

private:
    std::string studentNo_;
};

class Lecturer final : public User {
public:
    Lecturer(int id, std::string username, std::string hash, std::string staffNo)
        : User(id, std::move(username), std::move(hash)), staffNo_(std::move(staffNo)) {}
    const std::string& staffNo() const { return staffNo_; }
    UserRole role() const override { return UserRole::Lecturer; }
    std::vector<Action> allowedActions() const override;
    UserRecord toRecord() const override;

private:
    std::string staffNo_;
};

class HoD final : public User {
public:
    HoD(int id, std::string username, std::string hash, int departmentId)
        : User(id, std::move(username), std::move(hash)), departmentId_(departmentId) {}
    int departmentId() const { return departmentId_; }
    UserRole role() const override { return UserRole::HoD; }
    std::vector<Action> allowedActions() const override;
    UserRecord toRecord() const override;

private:
    int departmentId_;
};

class Admin final : public User {
public:
    Admin(int id, std::string username, std::string hash) : User(id, std::move(username), std::move(hash)) {}
    UserRole role() const override { return UserRole::Admin; }
    std::vector<Action> allowedActions() const override;
    UserRecord toRecord() const override;
};

}  // namespace lms
