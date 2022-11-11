#ifndef EMULNET_H
#define EMULNET_H

#include <vector>
#include <queue>
#include <unordered_map>
#include <random>
#include <string>
#include <cstdint>
#include <cstring>

struct Address {
    const int32_t ip = 0;
    const int16_t port = 0;

    Address() = default;
    Address(int32_t _ip, int16_t _port);

    std::string toString() const;

    size_t hashCode() const;
    bool operator ==(const Address &other) const;
    bool operator !=(const Address &other) const;
};

namespace std {

template <> struct hash<Address> {
    size_t operator()(const Address &addr) const {
        return addr.hashCode();
    }
};

}

class MsgData {
public:
    const void *getData() const;
    size_t getSize() const;

    template <typename T>
    void append(const T &data) {
        const size_t old_size = buf.size();
        buf.resize(old_size + sizeof(data));
        memcpy(buf.data() + old_size, &data, sizeof(data));
    }

    template <typename T>
    T scan(size_t &cursor) const {
        T data;
        memcpy(&data, buf.data() + cursor, sizeof(data));
        cursor += sizeof(data);
        return data;
    }

private:
    std::vector<int8_t> buf;
};

struct Msg {
    Address from;
    MsgData data;
};

class EmulNet {
public:
    EmulNet(double _msg_drop_prob);

    EmulNet(const EmulNet &) = delete;
    EmulNet &operator =(const EmulNet &) = delete;
    
    Address getNewAddress(short port);
    bool send(Address to, const Msg &msg, bool use_tcp);
    bool send(Address to, Msg &&msg, bool use_tcp);
    void recv(Address to, std::queue<Msg> &queue);

private:
    bool dropMsg();

    int next_ip;
    double msg_drop_prob;
    std::random_device rd;
    std::mt19937 gen;
    std::bernoulli_distribution d;

    std::unordered_map<Address, std::vector<Msg>> buf;
};

#endif /* EMULNET_H */
