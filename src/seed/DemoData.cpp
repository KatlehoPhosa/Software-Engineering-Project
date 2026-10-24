#include "seed/DemoData.h"

#include <ostream>

#include "common/Exceptions.h"

namespace lms {

void seedDemoData(System& sys, const IClock& clock, std::ostream& out) {
    if (sys.users.count() != 0) throw BusinessRuleError("The database is not empty; demo data was not added.");
    const std::string pw = "Demo#1234";
    sys.accounts.createFirstAdministrator("admin", pw);
    auto admin = sys.users.findByUsername("admin");

    const int sci = sys.catalog.createDepartment(*admin, "Science");
    const int hum = sys.catalog.createDepartment(*admin, "Humanities");

    auto mk = [&](UserRole r, const std::string& name, const std::string& no, int dept) {
        NewUserSpec s;
        s.role = r; s.username = name; s.password = pw; s.departmentId = dept;
        if (r == UserRole::Student) s.studentNo = no;
        if (r == UserRole::Lecturer) s.staffNo = no;
        return sys.accounts.createUser(*admin, s);
    };
    mk(UserRole::HoD, "hod.science", "", sci);
    mk(UserRole::HoD, "hod.humanities", "", hum);
    const int lect = mk(UserRole::Lecturer, "lect.moyo", "L001", 0);
    const int s1 = mk(UserRole::Student, "stu.thabo", "S1001", 0);
    const int s2 = mk(UserRole::Student, "stu.palesa", "S1002", 0);
    const int s3 = mk(UserRole::Student, "stu.lerato", "S1003", 0);

    sys.catalog.createCourse(*admin, "SCI101", "Introductory Science", sci);
    sys.catalog.createCourse(*admin, "HIS101", "World History", hum);
    sys.catalog.assignLecturer(*admin, lect, "SCI101");
    for (int s : {s1, s2, s3}) sys.catalog.enrolStudent(*admin, s, "SCI101");

    auto lecturer = sys.users.findById(lect);
    sys.assignmentService.createAssignment(*lecturer, "SCI101", "Lab report 1", clock.now().addDays(7), 20.0);

    out << "Demo data created. Every account uses the password: " << pw << "\n"
        << "  admin, hod.science, hod.humanities, lect.moyo, stu.thabo, stu.palesa, stu.lerato\n";
}

}  // namespace lms
