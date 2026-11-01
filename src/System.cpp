#include "System.h"

namespace lms {

System::System(data::Database& db, IClock& clock, const IPasswordHasher& hasher, std::unique_ptr<IPenaltyStrategy> pen)
    : tx(db), users(db), departments(db), courses(db), teaching(db), enrollments(db), assignments(db),
      extensions(db), submissions(db), marks(db), auditRepo(db),
      penalty(std::move(pen)), factory(hasher),
      events(), audit(auditRepo, marks), notifier(),
      auth(users, hasher),
      accounts(users, departments, tx, factory, hasher),
      catalog(departments, courses, teaching, enrollments, users, tx),
      assignmentService(assignments, courses, teaching, enrollments, extensions, clock, tx),
      portal(enrollments, assignments, submissions, marks, extensions),
      submissionService(submissions, assignments, enrollments, extensions, *penalty, clock, tx),
      marking(submissions, marks, assignments, extensions, teaching, *penalty, events, clock, tx),
      moderation(marks, events, clock, tx) {
    events.attach(&audit);     // every mark event is written to the audit log first...
    events.attach(&notifier);  // ...then students are notified of releases
}

}  // namespace lms
