#ifndef LOG_H
#define LOG_H

#include <fstream>
#include <string>

struct Address;

class Log {
public:
    Log(const char *file_name);

    void append(Address addr, const std::string &str);

private:
    std::ofstream os;
};

#endif /* LOG_H */
