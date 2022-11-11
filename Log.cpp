#include "Log.h"
#include "EmulNet.h"
#include "GlobalTime.h"

Log::Log(const char *file_name)
    : os(file_name)
{
}

void Log::append(Address addr, const std::string &str) {
    os << ' ' << addr.toString();
    os << ' ' << '[' << GlobalTime::Instance().getCurrTime() << ']';
    os << ' ' << str << std::endl;
}