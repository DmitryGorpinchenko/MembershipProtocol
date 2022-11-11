#include "Member.h"
#include "GlobalTime.h"

MemberListEntry::MemberListEntry(Address _addr, uint64_t _heartbeat)
    : addr(_addr)
    , heartbeat(_heartbeat)
    , timestamp(GlobalTime::Instance().getCurrTime())
    , marked_failed(false)
{
}

Member::Member(EmulNet &en, short port)
    : addr(en.getNewAddress(port))
    , inited(false)
    , in_group(false)
    , failed(false)
    , heartbeat(0)
{
}