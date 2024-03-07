#include "RouterPacket.h"

#include <algorithm>
#include <vector>

using namespace std;

RouterPacket::RouterPacket(int sourceID, int destID, vector<int> mincosts)
    : sourceid{ sourceID }, destid{ destID }, mincost{ std::move(mincosts) } {}

RouterPacket* RouterPacket::clone() const {
    return new RouterPacket{ sourceid, destid, vector<int>{ mincost } };
}
