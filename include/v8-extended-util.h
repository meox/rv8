#ifndef __V8_EXTENDED_UTIL__
#define __V8_EXTENDED_UTIL__


#include <sstream>
#include <include/v8.h>

using namespace v8;


void pretty_print(std::stringstream& stream, Value* v);
void pretty_print(Value* v);
std::string load_fromfile(const std::string& fname);
void ReportException(v8::Isolate* isolate, v8::TryCatch* try_catch);

#endif
