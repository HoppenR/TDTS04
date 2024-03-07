#pragma once

#include "GuiTextArea.h"
#include "RouterNode.h"
#include "RouterPacket.h"

#include <vector>

struct Event {
    Event(const Event&) = delete;
    Event(Event&&) = delete;
    Event& operator=(const Event&) = delete;
    Event& operator=(Event&&) = delete;

    double evtime;          /* event time */
    int evtype;             /* event type code */
    int eventity;           /* entity where event occurs */
    RouterPacket* rtpktptr; /* ptr to packet (if any)  */
    int dest;               /* destination */
    int cost;               /* for link cost change */
    Event* prev{ nullptr };
    Event* next{ nullptr };
};

class RouterNode;

class RouterSimulator {
public:
    RouterSimulator(const RouterSimulator&) = delete;
    RouterSimulator(RouterSimulator&&) = delete;
    RouterSimulator& operator=(const RouterSimulator&) = delete;
    RouterSimulator& operator=(RouterSimulator&&) = delete;

    RouterSimulator();

    static void main(int, char*[]);
    static void initialize(int, char*[]);
    void runSimulation();
    double getClockTime();
    void insertevent(Event*);
    void toLayer2(RouterPacket&);

    // Use command line arguments to configure each variable.
    static int NUM_NODES;      /* defaults to 3 */
    static bool LINKCHANGES;   /* defaults to true */
    static bool POISONREVERSE; /* defaults to true */
    static long SEED;          /* defaults to 1234 */
    static int TRACE;          /* defaults to 3 */

    const int INFINITY = 999;

private:
    GuiTextArea myGUI;
    Event* evlist;
    std::vector<std::vector<int>> connectcosts;
    std::vector<RouterNode> nodes;
    double clocktime;

    // possible events:
    const int FROM_LAYER2 = 2;
    const int LINK_CHANGE = 10;
};
