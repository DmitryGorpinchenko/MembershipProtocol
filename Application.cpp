#include "Application.h"
#include "Log.h"
#include "EmulNet.h"
#include "Member.h"
#include "MP1Node.h"
#include "GlobalTime.h"

#include <string>

Application::Application(const char *config_file)
    : cfg(config_file)
    , log(new Log("dbg.log"))
    , en(new EmulNet(cfg.getMsgDropProb()))
{
    for (int i = 0, count = cfg.getGroupSize(); i < count; ++i) {
        mems.emplace_back(new Member(*en.get(), cfg.getPort()));
        mp1.emplace_back(new MP1Node(*mems.back().get(), *en.get(), *log.get()));
        log->append(mems.back()->addr, "APP");
    }
}

int Application::runUntil(uint64_t time_limit) {
    srand(time(NULL));

    for (auto &clock = GlobalTime::Instance();
         clock.getCurrTime() < time_limit;
         clock.tick())
    {
        simulateMembershipProtocol();
        simulateNodeFailure();
    }

    for (auto &n : mp1) {
        n->nodeFinish();
    }

    return 0;
}

void Application::simulateMembershipProtocol() {
    const auto group_size = cfg.getGroupSize();
    const auto insert_rate = cfg.getInsertRate();
    const auto global_time = GlobalTime::Instance().getCurrTime();

    // introduce nodes into the system
    for (int i = 0; i < group_size; ++i) {
        if (global_time == uint64_t(insert_rate * i)) {
            mp1[i]->nodeStart(getCoordinator()->addr);
        }
    }
    // handle membership protocol duties
    for (int i = 0; i < group_size; ++i) {
        mp1[i]->nodeLoop();
    }
}

void Application::simulateNodeFailure() {
    if (const auto global_time = GlobalTime::Instance().getCurrTime(); global_time == 100) {
        const auto group_size = cfg.getGroupSize();

        if (cfg.isSingleFailure()) {
            const int removed = (rand() % group_size);
            log->append(mems[removed]->addr, std::string("Node failed at time=") + std::to_string(global_time));

            mems[removed]->failed = true;
        }
        else {
            const int removed = rand() % group_size / 2;
            for (int i = removed; i < removed + group_size / 2; ++i) {
                log->append(mems[i]->addr, std::string("Node failed at time=") + std::to_string(global_time));

                mems[i]->failed = true;
            }
        }
    }
}

Member *Application::getCoordinator() const {
    return (!mems.empty()) ? mems.front().get() : nullptr;
}

//

int main(int argc, char *argv[]) {
    if (argc != 2) {
        return -1;
    }
    Application app(argv[1]);
    return app.runUntil(700);
}