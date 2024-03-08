#include "RouterSimulator.h"
#include "RouterNode.h"
#include "RouterPacket.h"

#include <QApplication>
#include <getopt.h>
#include <iostream>
#include <string>
#include <vector>

/******************************************************************************
 * Project 4: implementing distributed, asynchronous, distance vector routing.
 *
 * THIS IS THE MAIN ROUTINE. The simulator's output and behavior can be changed
 * with the following command line arguments:
 *
 * -c --changelinks      true/false          To activate changing link costs
 * -n --nodes             3, 4, 5            Number of nodes to simulate
 * -p --poisonreverse    true/false          To activate poison reverse
 * -s --seed               (long)            Random seed
 * -t --trace            1, 2, 3, 4          Debugging levels
 *
 * This is a C++ version by chrlu470 of code provided by Kurose and Ross.
 * Almost all of the code design and comments are taken from their code.
 *
 * This version strives to stay as close as possible to the original Java and
 * Python code with a few differences:
 *   - Windowing system is initialized in RouterSimulator::main
 *   - Events and packets need to be `delete`d from the heap since C++ does not
 *     have garbage collection.
 *   - Long option "--poison" is now called "--poisonreverse"
 *
 * Entry point: RouterSimulator::main(argc, argv)
 * ***************************************************************************/

/* It's better to NOT change the variables here, but instead use command
 * line arguments (see above) to configure each simulation. */
long RouterSimulator::SEED = 1234;
int RouterSimulator::TRACE = 3;
int RouterSimulator::NUM_NODES = 3;
bool RouterSimulator::LINKCHANGES = true;
bool RouterSimulator::POISONREVERSE = true;

/* *************** NETWORK EMULATION CODE STARTS BELOW ******************
 * The code below emulates the layer 2 and below network environment:
 *   - emulates the transmission and delivery (with no loss and no
 *     corruption) between two physically connected nodes
 *   - calls the initializations routines rtinit0, etc., once before
 *     beginning emulation
 *
 * THERE IS NOT REASON THAT ANY STUDENT SHOULD HAVE TO READ OR UNDERSTAND
 * THE CODE BELOW.  YOU SHOLD NOT TOUCH, OR REFERENCE (in your code) ANY
 * OF THE DATA STRUCTURES BELOW.  If you're interested in how I designed
 * the emulator, you're welcome to look at the code - but again, you
 * should not have to, and you defeinitely should not have to modify
 * *********************************************************************/

using namespace std;

void RouterSimulator::main(int argc, char* argv[]) {
    // Initialize the window system Qt5
    QApplication app{ argc, argv };
    RouterSimulator::initialize(argc, argv);
    srand(RouterSimulator::SEED);
    RouterSimulator sim{};
    sim.runSimulation();
    // Display windows until student exits them
    app.exec();
}

RouterSimulator::RouterSimulator()
    : myGUI{ "  Output window for Router Simulator  " }, evlist{ nullptr },
      connectcosts{
          vector<vector<int>>(RouterSimulator::NUM_NODES,
                              vector<int>(RouterSimulator::NUM_NODES)),
      },
      clocktime{ 0.0f } {
    Event* evptr;

    switch (RouterSimulator::NUM_NODES) {
    case 3: {
        connectcosts[0][1] = 4;
        connectcosts[0][2] = 1;
        connectcosts[1][0] = 4;
        connectcosts[1][2] = 50;
        connectcosts[2][0] = 1;
        connectcosts[2][1] = 50;
    } break;
    case 4: {
        connectcosts[0][1] = 1;
        connectcosts[0][2] = 3;
        connectcosts[0][3] = 7;
        connectcosts[1][0] = 1;
        connectcosts[1][2] = 1;
        connectcosts[1][3] = INFINITY;
        connectcosts[2][0] = 3;
        connectcosts[2][1] = 1;
        connectcosts[2][3] = 2;
        connectcosts[3][0] = 7;
        connectcosts[3][1] = INFINITY;
        connectcosts[3][2] = 2;
    } break;
    case 5: {
        connectcosts[0][1] = 1;
        connectcosts[0][2] = 3;
        connectcosts[0][3] = 7;
        connectcosts[0][4] = 1;
        connectcosts[1][0] = 1;
        connectcosts[1][2] = 1;
        connectcosts[1][3] = INFINITY;
        connectcosts[1][4] = 1;
        connectcosts[2][0] = 3;
        connectcosts[2][1] = 1;
        connectcosts[2][3] = 2;
        connectcosts[2][4] = 4;
        connectcosts[3][0] = 7;
        connectcosts[3][1] = INFINITY;
        connectcosts[3][2] = 2;
        connectcosts[3][4] = INFINITY;
        connectcosts[4][0] = 1;
        connectcosts[4][1] = 1;
        connectcosts[4][2] = 4;
        connectcosts[4][3] = INFINITY;
    } break;
    default: {
        cerr << "Unsupported number of nodes." << endl;
        exit(0);
    };
    }

    nodes.reserve(RouterSimulator::NUM_NODES);
    for (int i = 0; i < RouterSimulator::NUM_NODES; i++) {
        nodes.emplace_back(i, this, connectcosts[i]);
    }

    if (RouterSimulator::LINKCHANGES) {
        switch (RouterSimulator::NUM_NODES) {
        case 3: {
            evptr = new Event{};
            evptr->evtime = 40.0;
            evptr->evtype = LINK_CHANGE;
            evptr->eventity = 0;
            evptr->rtpktptr = nullptr;
            evptr->dest = 1;
            evptr->cost = 60;
            insertevent(evptr);
        } break;
        case 4:
        case 5: {
            connectcosts[0][1] = 1;
            evptr = new Event{};
            evptr->evtime = 10000.0;
            evptr->evtype = LINK_CHANGE;
            evptr->eventity = 0;
            evptr->rtpktptr = nullptr;
            evptr->dest = 3;
            evptr->cost = 1;
            insertevent(evptr);

            evptr = new Event{};
            evptr->evtime = 20000.0;
            evptr->evtype = LINK_CHANGE;
            evptr->eventity = 0;
            evptr->rtpktptr = nullptr;
            evptr->dest = 1;
            evptr->cost = 6;
            insertevent(evptr);
        } break;
        default: {
            cerr << "Panic: Number of nodes outside 3-5" << endl;
            exit(0);
        } break;
        }
    }
}

void RouterSimulator::runSimulation() {
    Event* eventptr;
    while (true) {
        // get next event to simulate
        eventptr = evlist;
        if (eventptr == nullptr) {
            break;
        }
        evlist = evlist->next;
        // remove this event from event list
        if (evlist != nullptr) {
            evlist->prev = nullptr;
        }
        if (RouterSimulator::TRACE > 1) {
            myGUI.println("MAIN: rcv event, t=" + to_string(eventptr->evtime) +
                          " at " + to_string(eventptr->eventity));
            if (eventptr->evtype == FROM_LAYER2) {
                myGUI.print(" src:" + to_string(eventptr->rtpktptr->sourceid) +
                            " dest:" + to_string(eventptr->rtpktptr->destid) +
                            ", contents:");
                for (int i = 0; i < RouterSimulator::NUM_NODES; i++) {
                    myGUI.print(" " +
                                to_string(eventptr->rtpktptr->mincost[i]));
                }
                myGUI.println();
            }
        }

        // update time to next event time
        clocktime = eventptr->evtime;
        if (eventptr->evtype == FROM_LAYER2) {
            if (eventptr->eventity >= 0 &&
                eventptr->eventity < RouterSimulator::NUM_NODES) {
                nodes[eventptr->eventity].recvUpdate(*eventptr->rtpktptr);
            } else {
                cerr << "Panic: unknown event entity" << endl;
                exit(1);
            }
            // Dispose of the router packet
            delete eventptr->rtpktptr;
        } else if (eventptr->evtype == LINK_CHANGE) {
            // change link costs here if implemented
            nodes[eventptr->eventity].updateLinkCost(eventptr->dest,
                                                     eventptr->cost);
            nodes[eventptr->dest].updateLinkCost(eventptr->eventity,
                                                 eventptr->cost);
        } else {
            cerr << "Panic: unknown event type" << endl;
            exit(1);
        }

        if (RouterSimulator::TRACE > 2) {
            for (int i = 0; i < RouterSimulator::NUM_NODES; i++) {
                nodes[i].printDistanceTable();
            }
        }

        // Dispose of this event
        delete eventptr;
    }
    myGUI.println("\nSimulator terminated at t=" + to_string(clocktime) +
                  ", no packets in medium");
}

double RouterSimulator::getClockTime() {
    return clocktime;
}

/* ******************** EVENT HANDLING ROUTINES ********************
 *         The next set of routines handle the event list          *
 * ****************************************************************/

void RouterSimulator::insertevent(Event* p) {
    if (RouterSimulator::TRACE > 3) {
        myGUI.println("            INSERTEVENT: time is " +
                      to_string(clocktime));
        myGUI.println("            INSERTEVENT: future time will be " +
                      to_string(p->evtime));
    }
    // q points to header of list in which p struct inserted
    Event* q = evlist;
    Event* qold = nullptr;
    if (q == nullptr) {
        // list is empty
        evlist = p;
        p->next = nullptr;
        p->prev = nullptr;
    } else {
        for (qold = q; q != nullptr && p->evtime > q->evtime; q = q->next) {
            qold = q;
        }
        if (q == nullptr) {
            // end of list
            qold->next = p;
            p->prev = qold;
            p->next = nullptr;
        } else if (q == evlist) {
            // front of list
            p->next = evlist;
            p->prev = nullptr;
            p->next->prev = p;
            evlist = p;
        } else {
            // middle of list
            p->next = q;
            p->prev = q->prev;
            q->prev->next = p;
            q->prev = p;
        }
    }
}

/************************** TOLAYER2 ***************************/
void RouterSimulator::toLayer2(RouterPacket& packet) {
    // be nice: check if source and destination id's are reasonable
    if (packet.sourceid < 0 ||
        packet.sourceid > RouterSimulator::NUM_NODES - 1) {

        myGUI.println(
            "WARN: illegal source id in your packet, ignoring packet!");
        return;
    }
    if (packet.destid < 0 || packet.destid > RouterSimulator::NUM_NODES - 1) {
        myGUI.println("WARN: illegal dest id in your packet, ignoring packet!");
        return;
    }
    if (packet.sourceid == packet.destid) {
        myGUI.println(
            "WARN: source and destination id's the same, ignoring packet");
        return;
    }
    if (connectcosts[packet.sourceid][packet.destid] == INFINITY) {
        myGUI.println(
            "WARN: source and destination not connected, ignoring packet");
        return;
    }

    // make a copy of the packet student just gave me since may
    // be modified after we return back
    // (it may also be deallocated from the stack, depending on
    // how the student has created the object instance)
    RouterPacket* mypktptr = packet.clone();
    if (RouterSimulator::TRACE > 2) {
        myGUI.print("    TOLAYER2: source: " + to_string(mypktptr->sourceid) +
                    " dest: " + to_string(mypktptr->destid) +
                    "             costs:");
        for (int i = 0; i < RouterSimulator::NUM_NODES; i++) {
            myGUI.print(to_string(mypktptr->mincost[i]) + " ");
        }
        myGUI.println();
    }

    // create future event for arrival of packet at the other side
    Event* evptr = new Event{};
    // packet will pop out from layer3
    evptr->evtype = FROM_LAYER2;
    // event occurs at other entity
    evptr->eventity = mypktptr->destid;
    // save ptr to my copy of packet
    evptr->rtpktptr = mypktptr;

    // finally, compute the arrival time of packet at the other end.
    // medium can not reorder, so make sure packet arrives between 1
    // and 10 time units after the latest arrival time of packets
    // currently in the medium on their way to the destination
    double lastime = clocktime;
    for (Event* q = evlist; q != nullptr; q = q->next) {
        if (q->evtype == FROM_LAYER2 && q->eventity == evptr->eventity) {
            lastime = q->evtime;
        }
    }
    evptr->evtime =
        lastime + 9.0f * (static_cast<double>(rand()) / RAND_MAX) + 1.0f;

    if (RouterSimulator::TRACE > 2) {
        myGUI.println("    TOLAYER2: scheduling arrival on other side");
        insertevent(evptr);
    }
}

void RouterSimulator::initialize(int argc, char* argv[]) {
    string inputInfo = "./RouterSimulator "
                       "-c, --change <LINKCHANGE (bool)> "
                       "-n, --nodes <NODES (int)> "
                       "-p, --poisonreverse <POISONREVERSE (bool)> "
                       "-s, --seed <SEED (long)> "
                       "-t, --trace <TRACE (int)>"
                       "\n";

    option longOptions[] = {
        { "changelinks", required_argument, nullptr, 'c' },
        { "nodes", required_argument, nullptr, 'n' },
        { "poisonreverse", required_argument, nullptr, 'p' },
        { "seed", required_argument, nullptr, 's' },
        { "trace", required_argument, nullptr, 't' },
        { nullptr, 0, nullptr, 0 }
    };

    // Create a helper lambda for checking boolean options.
    // Uses the global variable `optarg` from <getopt.h>
    const vector<string> affirmative = { "true", "1", "y", "yes", "t" };
    const vector<string> negative = { "false", "0", "n", "no", "f" };
    auto opt_is = [](vector<string> const& type) {
        return find(type.begin(), type.end(), optarg) != type.end();
    };

    int opt;
    try {
        while ((opt = getopt_long(
                    argc, argv, "c:n:p:s:t:", longOptions, nullptr)) != -1) {
            switch (opt) {
            case 'c': {
                if (opt_is(affirmative)) {
                    RouterSimulator::LINKCHANGES = true;
                } else if (opt_is(negative)) {
                    RouterSimulator::LINKCHANGES = false;
                }
            } break;
            case 'n': {
                RouterSimulator::NUM_NODES = stoi(optarg);
            } break;
            case 'p': {
                if (opt_is(affirmative)) {
                    RouterSimulator::POISONREVERSE = true;
                } else if (opt_is(negative)) {
                    RouterSimulator::POISONREVERSE = false;
                }
            } break;
            case 's': {
                RouterSimulator::SEED = stol(optarg);
            } break;
            case 't': {
                RouterSimulator::TRACE = stoi(optarg);
            } break;
            default: {
                cerr << inputInfo;
                exit(EXIT_FAILURE);
            } break;
            }
        }
    } catch (invalid_argument&) {
        cerr << inputInfo << endl;
        exit(2);
    }
}

int main(int argc, char* argv[]) {
    RouterSimulator::main(argc, argv);
}
