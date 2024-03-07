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
    // std::vector requires RouterNode to be move-constructible
    RouterNode(RouterNode&&) = default;
    RouterNode& operator=(const RouterNode&) = delete;
    RouterNode& operator=(RouterNode&&) = delete;

    RouterNode(int, RouterSimulator*, std::vector<int> const&);
    void recvUpdate(RouterPacket&);
    void printDistanceTable();
    void updateLinkCost(int, int);

private:
    void sendUpdate(RouterPacket&);
    void updateDistanceCosts();
    void notifyNeighbors(int* = nullptr);

    GuiTextArea myGUI;
    RouterSimulator* sim;
    int myID;
    std::vector<int> costs;
    std::vector<std::vector<int>> distances;
    std::vector<std::string> routes;
};
