#ifndef MEMBER_H
#define MEMBER_H

#include "EmulNet.h"

#include <vector>
#include <queue>
#include <cstdint>

struct MemberListEntry {
    Address addr;
    uint64_t heartbeat;
    uint64_t timestamp;
    bool marked_failed;

    MemberListEntry(Address _addr, uint64_t _heartbeat);
};
using MemberList = std::vector<MemberListEntry>;

/**
 * CLASS NAME: Member
 *
 * DESCRIPTION: Class representing a member in the distributed system
 */
class Member {
public:
    Address addr;
    bool inited;
    bool in_group;
    bool failed;
    uint64_t heartbeat;
    MemberList member_list;
    std::queue<Msg> mp1q;

    Member(EmulNet &en, short port);
    
    Member(const Member &) = delete;
    Member &operator =(const Member &) = delete;
};

#endif /* MEMBER_H */
