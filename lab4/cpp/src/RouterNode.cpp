#include "RouterNode.h"
#include "RouterPacket.h"

#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

/* You can access command line values via the router simulator
 * example: RouterSimulator::POISONREVERSE,
 *
 * and also constants via the `sim` pointer
 * example: sim->INFINITY */

RouterNode::RouterNode(int ID, RouterSimulator* sim, vector<int> const& costs)
    : myGUI{ "  Output window for router #" + to_string(ID) + "  " },
      sim{ sim }, myID{ ID }, costs{ costs },
      distances(sim->NUM_NODES, vector<int>(sim->NUM_NODES, sim->INFINITY)),
      routes(sim->NUM_NODES, "-") {

    // Init distances and routes info
    distances[myID] = costs;
    for (int i = 0; i < sim->NUM_NODES; i++) {
        if (costs[i] != sim->INFINITY) {
            routes[i] = to_string(i);
        }
    }
    notifyNeighbors();
}

//--------------------------------------------------

void RouterNode::updateDistanceCosts() {
    for (int target = 0; target < sim->NUM_NODES; target++) {
        if (target == myID) {
            continue;
        }
        vector<int> nextRoutes;
        vector<int> nextHop;
        nextRoutes.reserve(sim->NUM_NODES);
        nextHop.reserve(sim->NUM_NODES);
        for (int next = 0; next < sim->NUM_NODES; next++) {
            if (next == myID) {
                continue;
            }
            if (distances[next].size() <= target) {
                continue;
            }
            nextRoutes.push_back(costs[next] + distances[next][target]);
            nextHop.push_back(next);
        }
        int minCost = sim->INFINITY;
        int minHopID;
        for (size_t next = 0; next < nextRoutes.size(); next++) {
            if (nextRoutes[next] < minCost) {
                minCost = nextRoutes[next];
                minHopID = nextHop[next];
            }
        }
        this->distances[myID][target] = minCost;
        this->routes[target] = to_string(minHopID);
    }
}

//--------------------------------------------------

void RouterNode::notifyNeighbors(int* fakeindex) {
    if (!sim->POISONREVERSE) {
        for (int neighborID = 0; neighborID < sim->NUM_NODES; neighborID++) {
            if (neighborID == myID) {
                continue;
            }
            RouterPacket pkt{
                myID,
                neighborID,
                distances[myID],
            };
            sendUpdate(pkt);
        }
    } else {
        vector<int> fakevector = distances[myID];
        if (fakeindex != nullptr) {
            fakevector[*fakeindex] = sim->INFINITY;
        }
        for (int neighborID = 0; neighborID < sim->NUM_NODES; neighborID++) {
            if (neighborID == myID) {
                continue;
            }
            if (costs[neighborID] == sim->INFINITY) {
                continue;
            }
            RouterPacket pkt{
                myID,
                neighborID,
                fakevector,
            };
            sendUpdate(pkt);
        }
    }
}

//--------------------------------------------------

void RouterNode::recvUpdate(RouterPacket& pkt) {
    distances[pkt.sourceid] = pkt.mincost;
    vector<vector<int>> oldDistances = distances;
    updateDistanceCosts();
    if (oldDistances != distances) {
        notifyNeighbors();
    }
}

//--------------------------------------------------

void RouterNode::sendUpdate(RouterPacket& pkt) {
    sim->toLayer2(pkt);
}

//--------------------------------------------------

void RouterNode::printDistanceTable() {
    // Use a string builder to avoid expensive myGUI::print calls
    ostringstream stringBuilder;
    stringBuilder << "Current state for " << myID << " at time " << std::fixed
                  << setprecision(1) << sim->getClockTime() << "\n\n";

    stringBuilder << "Distancetable\n";
    stringBuilder << "    dst |";
    for (int i = 0; i < sim->NUM_NODES; i++) {
        stringBuilder << setw(5) << i;
    }
    stringBuilder << '\n';

    stringBuilder << "---------";
    for (int i = 0; i < sim->NUM_NODES; i++) {
        stringBuilder << "-----";
    }
    stringBuilder << '\n';

    for (int i = 0; i < sim->NUM_NODES; i++) {
        stringBuilder << " nbr" << setw(4) << i << '|';
        for (int j = 0; j < sim->NUM_NODES; j++) {
            stringBuilder << setw(5) << distances[i][j];
        }
        stringBuilder << '\n';
    }
    stringBuilder << '\n';

    stringBuilder << "Our distance vector and routes:\n";
    stringBuilder << "    dst |";
    for (int i = 0; i < sim->NUM_NODES; i++) {
        stringBuilder << setw(5) << i;
    }
    stringBuilder << '\n';

    stringBuilder << "---------";
    for (int i = 0; i < sim->NUM_NODES; i++) {
        stringBuilder << "-----";
    }
    stringBuilder << '\n';

    stringBuilder << " cost   |";
    for (int i = 0; i < sim->NUM_NODES; i++) {
        stringBuilder << setw(5) << distances[myID][i];
    }
    stringBuilder << '\n';

    stringBuilder << " route  |";
    for (int i = 0; i < sim->NUM_NODES; i++) {
        stringBuilder << setw(5) << routes[i];
    }
    stringBuilder << "\n\n";
    myGUI.println(stringBuilder.str());
}

//--------------------------------------------------

void RouterNode::updateLinkCost(int dest, int newcost) {
    costs[dest] = newcost;
    updateDistanceCosts();
    notifyNeighbors(&dest);
}
