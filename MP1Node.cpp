#include "MP1Node.h"
#include "Log.h"
#include "Member.h"
#include "EmulNet.h"
#include "GlobalTime.h"

#include <cstring>
#include <algorithm>
#include <random>

namespace {

constexpr uint64_t c_t_remove = 20;
constexpr uint64_t c_t_fail = 5;
constexpr size_t c_fanout_size = 2;

struct MemberInfo {
    Address addr;
    uint64_t heartbeat;
};

void LogNodeAdd(Address this_node, Address added_node, Log &log) {
    std::string str;
    str += "Node ";
    str += added_node.toString();
    str += " joined at time ";
    str += std::to_string(GlobalTime::Instance().getCurrTime());
    log.append(this_node, str);
}

void LogNodeRemove(Address this_node, Address removed_node, Log &log) {
    std::string str;
    str += "Node ";
    str += removed_node.toString();
    str += " removed at time ";
    str += std::to_string(GlobalTime::Instance().getCurrTime());
    log.append(this_node, str);
}

void SendMemberList(const std::vector<Address> &to_list, MsgType ty, bool use_tcp, const Member &mem, EmulNet &en) {
    std::vector<MemberInfo> infos;
    infos.push_back({mem.addr, mem.heartbeat});
    for (const auto &m : mem.member_list) {
        if (!m.marked_failed) {
            infos.push_back({m.addr, m.heartbeat});
        }
    }
    const uint64_t sz = infos.size();

    MsgData data;
    data.append(ty);
    data.append(sz);
    for (const auto &i : infos) {
        data.append(i);
    }

    for (const auto &to : to_list) {
        en.send(to, Msg{mem.addr, data}, use_tcp);
    }
}

std::vector<MemberInfo> ScanMemberInfoList(const MsgData &data, size_t cursor) {
    const auto sz = data.scan<uint64_t>(cursor);

    std::vector<MemberInfo> res;
    res.reserve(sz);
    for (uint64_t i = 0; i < sz; ++i) {
        res.push_back(data.scan<MemberInfo>(cursor));
    }
    return res;
}

std::vector<Address> Fanout(const MemberList &list, size_t fanout_size) {
    MemberList alive_list;
    for (const auto &m : list) {
        if (!m.marked_failed) {
            alive_list.push_back(m);
        }
    }
    if (alive_list.empty()) {
        return {};
    }

    MemberList sublist;
    std::sample(alive_list.begin(), alive_list.end(), std::back_inserter(sublist), fanout_size, std::mt19937(std::random_device()()));

    std::vector<Address> res;
    res.reserve(sublist.size());
    for (const auto &m : sublist) {
        res.push_back(m.addr);
    }
    return res;
}

bool AddUnique(MemberInfo info, MemberList& list) {
    for (const auto &m : list) {
        if (m.addr == info.addr) {
            return false;
        }
    }
    list.emplace_back(info.addr, info.heartbeat);
    return true;
}

void Update(MemberInfo info, MemberList& list) {
    for (auto &m : list) {
        if (m.addr == info.addr) {
            if (info.heartbeat > m.heartbeat) {
                m.heartbeat = info.heartbeat;
                m.timestamp = GlobalTime::Instance().getCurrTime();
                m.marked_failed = false;
            }
            break;
        }
    }
}

}

MP1Node::MP1Node(Member &_mem, EmulNet &_en, Log &_log)
    : mem(_mem)
    , en(_en)
    , log(_log)
    , is_coordinator(false)
{
}

/**
 * FUNCTION NAME: nodeStart
 *
 * DESCRIPTION: This function bootstraps the node
 * 				Called by the application layer.
 */
void MP1Node::nodeStart(Address joinaddr) {
    if (!mem.inited) {
        mem.inited = true;
        mem.in_group = false;
        mem.failed = false;
        mem.heartbeat = 0;
        mem.member_list.clear();

        introduceSelfToGroup(joinaddr);
    }
}

void MP1Node::nodeFinish() {
    if (mem.inited) {
        mem.inited = false;
        mem.in_group = false;
        mem.member_list.clear();
    }
}

/**
 * FUNCTION NAME: nodeLoop
 *
 * DESCRIPTION: Executed periodically at each member
 * 				Check your messages in queue and perform membership protocol duties
 */
void MP1Node::nodeLoop() {
    if (mem.inited && !mem.failed) {
        checkMessages();
        nodeLoopOps();
    }
}

void MP1Node::introduceSelfToGroup(Address joinaddr) {
    if (mem.addr == joinaddr) {
        // I am the group booter (first process to join the group). Boot up the group
        log.append(mem.addr, "Starting up group...");

        mem.in_group = true;
        is_coordinator = true;
    }
    else {
        log.append(mem.addr, "Trying to join...");

        const auto ty = MsgType::JOINREQ;
        const auto hb = mem.heartbeat;
        MsgData data;
        data.append(ty);
        data.append(hb);

        en.send(joinaddr, Msg{mem.addr, std::move(data)}, true);
    }
}

void MP1Node::checkMessages() {
    en.recv(mem.addr, mem.mp1q);

    while (!mem.mp1q.empty()) {
        recvCallBack(mem.mp1q.front());
        mem.mp1q.pop();
    }
}

void MP1Node::recvCallBack(const Msg &msg) {
    size_t cursor = 0;
    const auto ty = msg.data.scan<MsgType>(cursor);

    if (ty == MsgType::JOINREQ) {
        if (is_coordinator) {
            const auto hb = msg.data.scan<uint64_t>(cursor);
            auto &l = mem.member_list;
            if (AddUnique({msg.from, hb}, l)) {
                LogNodeAdd(mem.addr, msg.from, log);

                ++mem.heartbeat;
                SendMemberList({msg.from}, MsgType::JOINREP, true,  mem, en);
            }
        }
    }
    else if (ty == MsgType::JOINREP) {
        if (!is_coordinator) {
            auto &l = mem.member_list;
            l.clear();
            for (const auto &i : ScanMemberInfoList(msg.data, cursor)) {
                if (i.addr != mem.addr) {
                    if (AddUnique(i, l)) {
                        LogNodeAdd(mem.addr, i.addr, log);
                    }
                }
            }
            mem.in_group = true;
        }
    }
    else if (ty == MsgType::GOSSIP) {
        if (mem.in_group) {
            auto &l = mem.member_list;
            for (const auto &i : ScanMemberInfoList(msg.data, cursor)) {
                if (i.addr != mem.addr) {
                    if (AddUnique(i, l)) {
                        LogNodeAdd(mem.addr, i.addr, log);
                    }
                    else {
                        Update(i, l);
                    }
                }
            }
        }
    }
}

/**
 * FUNCTION NAME: nodeLoopOps
 *
 * DESCRIPTION: Check if any node hasn't responded within a timeout period and then delete the nodes
 * 				Propagate your membership list
 */
void MP1Node::nodeLoopOps() {
    if (!mem.in_group) {
        return;
    }

    const auto global_time = GlobalTime::Instance().getCurrTime();

    MemberList new_list;
    for (auto m : mem.member_list) {
        const bool remove = m.marked_failed && (global_time > m.timestamp + c_t_remove);
        if (!remove) {
            const bool mark_failed = !m.marked_failed && (global_time > m.timestamp + c_t_fail);
            if (mark_failed) {
                m.timestamp = global_time;
                m.marked_failed = true;
            }
            new_list.push_back(m);
        } else {
            LogNodeRemove(mem.addr, m.addr, log);
        }
    }
    mem.member_list = std::move(new_list);

    ++mem.heartbeat;
    SendMemberList(Fanout(mem.member_list, c_fanout_size), MsgType::GOSSIP, false, mem, en);
}
