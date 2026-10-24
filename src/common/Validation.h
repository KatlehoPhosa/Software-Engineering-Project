#pragma once
#include <string>

namespace lms::validation {

std::string trim(const std::string& s);
bool isBlank(const std::string& s);

/// Return the trimmed value or throw ValidationError naming the field.
std::string requireNonBlank(const std::string& value, const std::string& field, std::size_t maxLen = 200);

/// 3-32 characters from [A-Za-z0-9._-].
std::string requireUsername(const std::string& value);
/// 8-128 characters and not equal to the username (case-insensitive).
void requirePassword(const std::string& password, const std::string& username);
/// Course codes such as SCI101: 2-12 letters/digits, stored upper-case.
std::string requireCourseCode(const std::string& value);

/// Strict numeric parsing: whole input must be consumed, result must be in range.
int parseInt(const std::string& text, const std::string& field, int min, int max);
double parseDouble(const std::string& text, const std::string& field, double min, double max);

}  // namespace lms::validation
