#pragma once

#include <vector>

class RouterPacket {
public:
    // Allow students to access all constructors
    // to avoid any confusion
    RouterPacket(const RouterPacket&) = default;
    RouterPacket(RouterPacket&&) = default;
    RouterPacket& operator=(const RouterPacket&) = default;
    RouterPacket& operator=(RouterPacket&&) = default;

    RouterPacket(int, int, std::vector<int>);
    RouterPacket* clone() const;

    int sourceid;
    int destid;
    std::vector<int> mincost;
};
