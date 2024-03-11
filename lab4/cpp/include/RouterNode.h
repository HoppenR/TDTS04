#pragma once

#include "GuiTextArea.h"
#include "RouterPacket.h"
#include "RouterSimulator.h"

#include <string>
#include <vector>

class RouterSimulator;

class RouterNode {
public:
    RouterNode(const RouterNode&) = delete;
    // std::vector<RouterNode> in RouterSimulator.h requires move constructor
    RouterNode(RouterNode&&) = default;
    RouterNode& operator=(const RouterNode&) = delete;
    RouterNode& operator=(RouterNode&&) = delete;

    RouterNode(int, RouterSimulator*, std::vector<int> const&);
    void recvUpdate(RouterPacket&);
    void printDistanceTable();
    void updateLinkCost(int, int);

private:
    void sendUpdate(RouterPacket&);

    GuiTextArea myGUI;
    RouterSimulator* sim;
    int myID;
    std::vector<int> costs;

    // Variables + methods not in the original lab template:
    void updateDistanceCosts();
    void notifyNetwork(int* = nullptr);
    std::vector<std::vector<int>> distances;
    std::vector<std::string> routes;
};
