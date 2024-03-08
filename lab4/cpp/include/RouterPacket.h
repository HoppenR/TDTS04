#pragma once

#include <vector>

class RouterPacket {
public:
    RouterPacket(const RouterPacket&) = delete;
    RouterPacket(RouterPacket&&) = delete;
    RouterPacket& operator=(const RouterPacket&) = delete;
    RouterPacket& operator=(RouterPacket&&) = delete;

    RouterPacket(int, int, std::vector<int>);
    RouterPacket* clone() const;

    int sourceid;
    int destid;
    std::vector<int> mincost;
};
