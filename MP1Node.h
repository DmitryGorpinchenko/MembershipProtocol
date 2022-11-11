#ifndef MP1NODE_H
#define MP1NODE_H

#include <cstdint>

class Member;
class EmulNet;
class Log;
struct Address;
struct Msg;

enum class MsgType : int32_t {
    JOINREQ,
    JOINREP,
    GOSSIP,
};

/**
 * CLASS NAME: MP1Node
 *
 * DESCRIPTION: Class implementing Membership protocol functionalities for failure detection
 */
class MP1Node {
public:
    MP1Node(Member &_mem, EmulNet &_en, Log &_log);

    Member &getMember() { return mem; }

    void nodeStart(Address joinaddr);
    void nodeFinish();
    void nodeLoop();

private:
    void introduceSelfToGroup(Address joinaddr);
    void checkMessages();
    void recvCallBack(const Msg &msg);
    void nodeLoopOps();

    Member &mem;
    EmulNet &en;
    Log &log;
    bool is_coordinator;
};

#endif /* MP1NODE_H */
