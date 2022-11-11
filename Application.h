#ifndef APPLICATION_H
#define APPLICATION_H

#include "Config.h"

#include <vector>
#include <memory>
#include <cstdint>

class Log;
class EmulNet;
class Member;
class MP1Node;
struct Address;

class Application {
public:
    Application(const char *config_file);

    int runUntil(uint64_t time_limit);

private:
    void simulateMembershipProtocol();
    void simulateNodeFailure();

    Member *getCoordinator() const;

    const Config cfg;

    std::unique_ptr<Log> log;
    std::unique_ptr<EmulNet> en;
    std::vector<std::unique_ptr<Member>> mems;
    std::vector<std::unique_ptr<MP1Node>> mp1;
};

#endif /* APPLICATION_H */
