#pragma once
#include <memory>

#include "common/Clock.h"
#include "data/sqlite/Database.h"
#include "data/sqlite/SqliteRepositories.h"
#include "domain/PenaltyStrategy.h"
#include "domain/UserFactory.h"
#include "security/IPasswordHasher.h"
#include "service/AccountService.h"
#include "service/AssignmentService.h"
#include "service/AuditService.h"
#include "service/AuthService.h"
#include "service/CatalogService.h"
#include "service/MarkingService.h"
#include "service/ModerationService.h"
#include "service/StudentNotifier.h"
#include "service/StudentPortalService.h"
#include "service/SubmissionService.h"

namespace lms {

/// Composition root: the only place where concrete classes (SQLite repositories, the penalty
/// strategy, the observers) are chosen and wired into the services, which see interfaces only.
class System {
public:
    System(data::Database& db, IClock& clock, const IPasswordHasher& hasher,
           std::unique_ptr<IPenaltyStrategy> penalty = std::make_unique<DailyPercentPenaltyStrategy>());

    // data access layer
    data::SqliteTransactionManager tx;
    data::SqliteUserRepository users;
    data::SqliteDepartmentRepository departments;
    data::SqliteCourseRepository courses;
    data::SqliteTeachingRepository teaching;
    data::SqliteEnrollmentRepository enrollments;
    data::SqliteAssignmentRepository assignments;
    data::SqliteExtensionRepository extensions;
    data::SqliteSubmissionRepository submissions;
    data::SqliteMarkRepository marks;
    data::SqliteAuditRepository auditRepo;

    // domain helpers
    std::unique_ptr<IPenaltyStrategy> penalty;
    UserFactory factory;

    // observers
    MarkEventPublisher events;
    AuditService audit;
    StudentNotifier notifier;

    // services
    AuthService auth;
    AccountService accounts;
    CatalogService catalog;
    AssignmentService assignmentService;
    StudentPortalService portal;
    SubmissionService submissionService;
    MarkingService marking;
    ModerationService moderation;
};

}  // namespace lms
