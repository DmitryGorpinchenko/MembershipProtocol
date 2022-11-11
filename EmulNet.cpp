#include "EmulNet.h"

Address::Address(int32_t _ip, int16_t _port)
    : ip(_ip)
    , port(_port)
{
}

std::string Address::toString() const {
    std::string res;

    auto parts = reinterpret_cast<const int8_t *>(&ip);
    res += std::to_string(parts[0]);
    res += '.';
    res += std::to_string(parts[1]);
    res += '.';
    res += std::to_string(parts[2]);
    res += '.';
    res += std::to_string(parts[3]);
    res += ':';
    res += std::to_string(port);
    
    return res;
}

size_t Address::hashCode() const {
    const size_t h1 = std::hash<int32_t>()(ip);
    const size_t h2 = std::hash<int16_t>()(port);
    return h1 ^ h2;
}

bool Address::operator ==(const Address &other) const {
    return ip == other.ip && port == other.port;
}

bool Address::operator !=(const Address &other) const {
    return !(*this == other);
}

//

const void *MsgData::getData() const {
    return buf.data();
}

size_t MsgData::getSize() const {
    return buf.size();
}

//

EmulNet::EmulNet(double _msg_drop_prob)
    : next_ip(0)
    , msg_drop_prob(_msg_drop_prob)
    , gen(rd())
    , d(_msg_drop_prob)
{
}

Address EmulNet::getNewAddress(short port) {
    return Address(++next_ip, port);
}

bool EmulNet::send(Address to, const Msg &msg, bool use_tcp) {
    if (use_tcp || !dropMsg()) {
        buf[to].push_back(msg);
        return true;
    }
    return false;
}

bool EmulNet::send(Address to, Msg &&msg, bool use_tcp) {
    if (use_tcp || !dropMsg()) {
        buf[to].push_back(std::move(msg));
        return true;
    }
    return false;
}

void EmulNet::recv(Address to, std::queue<Msg> &queue) {
    if (auto it = buf.find(to); it != buf.end()) {
        auto &msgs = (*it).second;
        for (auto &msg : msgs) {
            queue.push(std::move(msg));
        }
        msgs.clear();
    }
}

bool EmulNet::dropMsg() {
    return (msg_drop_prob > 0) && d(gen);
}