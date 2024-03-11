#include "RouterNode.h"
#include "RouterPacket.h"

#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

/*
 * Initialize the RouterNode.
 * Also synchronize the base costs on the network before the first LINK_CHANGE
 * event by using the notifyNetwork method.
 */
RouterNode::RouterNode(int ID, RouterSimulator* sim, vector<int> const& costs)
    : myGUI{ "  Output window for router #" + to_string(ID) + "  " },
      sim{ sim }, myID{ ID }, costs{ costs },
      distances(sim->NUM_NODES, vector<int>(sim->NUM_NODES, sim->INFINITY)),
      routes(sim->NUM_NODES, "-") {

    // Init distances and routes info.
    distances[myID] = costs;
    for (int i = 0; i < sim->NUM_NODES; i++) {
        if (costs[i] != sim->INFINITY) {
            routes[i] = to_string(i);
        }
    }

    // Notify the network about the starting costs of this node
    notifyNetwork();
}

/*
 * Recalculate the distance costs to all destinations for this node by using:
 * min{ cost(x -> y) + distance(y -> destination) }
 * where x is ourselves and y is any adjacent neighbor.
 */
void RouterNode::updateDistanceCosts() {
    for (int target = 0; target < sim->NUM_NODES; target++) {
        if (target == myID) {
            continue;
        }
        // Calculate all possible routes so that we can find the cheapest one.
        // For each cost: save the first hop at the same index.
        vector<int> targetRoutes;
        vector<int> targetFirstHops;
        targetRoutes.reserve(sim->NUM_NODES);
        targetFirstHops.reserve(sim->NUM_NODES);
        for (int next = 0; next < sim->NUM_NODES; next++) {
            if (next == myID) {
                continue;
            }
            targetRoutes.push_back(costs[next] + distances[next][target]);
            targetFirstHops.push_back(next);
        }
        // Find the minimum cost possible, and its corresponding first hop.
        int minCost = sim->INFINITY;
        int minFirstHopID;
        for (size_t i = 0; i < targetRoutes.size(); i++) {
            if (targetRoutes[i] < minCost) {
                minCost = targetRoutes[i];
                minFirstHopID = targetFirstHops[i];
            }
        }
        this->distances[myID][target] = minCost;
        // Set the route info for informative printing
        this->routes[target] = to_string(minFirstHopID);
    }
}

/*
 * Send our distances to the entire network
 * If POISONREVERSE is true then we should send INFINITY to any node that we are
 * connected to.
 * `fakeindex` defaults to a null pointer.
 */
void RouterNode::notifyNetwork(int* fakeidx) {
    for (int target = 0; target < sim->NUM_NODES; target++) {
        // Prepare a vector that we might need to add poisoned data to
        vector<int> sendvector = distances[myID];
        if (sim->POISONREVERSE && fakeidx != nullptr && target != *fakeidx) {
            sendvector[*fakeidx] = sim->INFINITY;
        }
        if (target == myID) {
            continue;
        }
        if (costs[target] == sim->INFINITY) {
            continue;
        }
        RouterPacket pkt{
            myID,
            target,
            std::move(sendvector),
        };
        sendUpdate(pkt);
    }
}

/*
 * When an update is received, update our distance costs and propagate any
 * updated costs, if there is no change, don't propagate.
 */
void RouterNode::recvUpdate(RouterPacket& pkt) {
    distances[pkt.sourceid] = pkt.mincost;
    vector<vector<int>> oldDistances = distances;
    updateDistanceCosts();
    if (oldDistances != distances) {
        // Send a regular update to the network, without poisoning any data.
        notifyNetwork();
    }
}

/*
 * Send a prepared packet to the simulator.
 */
void RouterNode::sendUpdate(RouterPacket& pkt) {
    sim->toLayer2(pkt);
}

/*
 * Format and print state info about this router node.
 */
void RouterNode::printDistanceTable() {
    // Use a string builder to avoid expensive myGUI.print calls
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

/*
 * Set a new link cost for this node to another node.
 * Since this might cause an update in distance costs when POISONREVERSE is
 * true
 */
void RouterNode::updateLinkCost(int dest, int newcost) {
    costs[dest] = newcost;
    updateDistanceCosts();
    // Pass what node ID to poison data about, if POISONREVERSE is true
    notifyNetwork(&dest);
}
