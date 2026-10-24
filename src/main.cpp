#include <cstdlib>
#include <iostream>
#include <string>

#include "common/Exceptions.h"
#include "presentation/Application.h"
#include "security/SodiumPasswordHasher.h"
#include "seed/DemoData.h"

namespace {
void usage() {
    std::cout << "Moderated LMS v1.0.0\n"
                 "Usage: lms [--db <file>] [--seed-demo] [--utc-offset-minutes <n>]\n"
                 "  --db <file>               database file (default: lms.db, or env LMS_DB)\n"
                 "  --seed-demo               fill an EMPTY database with sample users, course and assignment\n"
                 "  --utc-offset-minutes <n>  local time zone for typing/showing times (default 120 = Lesotho)\n";
}
}  // namespace

int main(int argc, char** argv) {
    std::string dbPath = std::getenv("LMS_DB") ? std::getenv("LMS_DB") : "lms.db";
    int offset = 120;
    bool seed = false;
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if (a == "--db" && i + 1 < argc) dbPath = argv[++i];
        else if (a == "--seed-demo") seed = true;
        else if (a == "--utc-offset-minutes" && i + 1 < argc) offset = std::atoi(argv[++i]);
        else { usage(); return a == "--help" ? 0 : 2; }
    }
    try {
        lms::data::Database db(dbPath);
        db.initialiseSchema();
        lms::SystemClock clock;
        lms::SodiumPasswordHasher hasher;
        lms::System sys(db, clock, hasher);
        lms::Console con(std::cin, std::cout, offset, true);
        if (seed) {
            lms::seedDemoData(sys, clock, std::cout);
            return 0;
        }
        lms::Application app(sys, con);
        app.run();
        return 0;
    } catch (const lms::LmsError& e) {
        std::cerr << "Fatal error: " << e.what() << '\n';
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error: " << e.what() << '\n';
        return 1;
    }
}
