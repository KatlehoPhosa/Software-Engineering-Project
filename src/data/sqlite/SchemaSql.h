#pragma once
namespace lms::data {
/// The full CREATE TABLE / CREATE INDEX script (embedded from schema/schema.sql at build time).
const char* schemaSql();
}  // namespace lms::data
