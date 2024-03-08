#include "RouterPacket.h"

#include <algorithm>
#include <vector>

using namespace std;

RouterPacket::RouterPacket(int sourceID, int destID, vector<int> mincosts)
    : sourceid{ sourceID }, destid{ destID }, mincost{ std::move(mincosts) } {}

/*
 * Clone packet information and allocate it on the heap to safetly pass it
 * from RouterNode::sendUpdate to RouterSimulator::evlist
 */
RouterPacket* RouterPacket::clone() const {
    return new RouterPacket{ sourceid, destid, vector<int>{ mincost } };
}
