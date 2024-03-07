#!/usr/bin/env python

import sys
import getopt
import random
import GuiTextArea
import RouterNode
import RouterPacket

from typing import Optional


class Event(object):
    # event time
    evtime: float
    # event type code
    evtype: int
    # entity where event occurs
    eventity: int
    # ptr to packet (if any) assoc w/ this event
    rtpktptr: Optional[RouterPacket.RouterPacket] = None
    # for link cost change
    dest: int
    # for link cost change
    cost: int
    # previous event
    prev: Optional['Event'] = None
    # next event
    next: Optional['Event'] = None

    def __eq__(self, other: object):
        if not isinstance(other, Event):
            return NotImplemented

        return (self.evtime == other.evtime and
                self.evtype == other.evtype and
                self.eventity == other.eventity)

# ******************************************************************
# Project 4: implementing distributed, asynchronous, distance vector routing.
#
# THIS IS THE MAIN ROUTINE. The simulator's output and behavior can be changed
# with the following command line arguments:
#
# -c --changelinks      True/False          To activate changing link costs
# -n --nodes            3, 4, 5             Number of nodes to simulate
# -p --poisonreverse    True/False          To activate poison reverse
# -s --seed             (integer)           Random seed
# -t --trace            1, 2, 3, 4          Debugging levels
#
# This is a Python version by C M Bruhner 2021 of code originally by Kurose
# and Ross, with output GUI orignally added to Java version by Ch. Schuba 2007.
#
# ******************************************************************


class RouterSimulator():
    NUM_NODES: int = 3
    LINKCHANGES: bool = True
    POISONREVERSE: bool = True
    SEED: int = 1234
    TRACE: int = 3

    INFINITY: int = 999
    connectcosts: list[list[int]]

    # ***************** NETWORK EMULATION CODE STARTS BELOW ***********
    # The code below emulates the layer 2 and below network environment:
    #   - emulates the tranmission and delivery (with no loss and no
    #     corruption) between two physically connected nodes
    #   - calls the initializations routines rtinit0, etc., once before
    #     beginning emulation
    #
    # THERE IS NOT REASON THAT ANY STUDENT SHOULD HAVE TO READ OR UNDERSTAND
    # THE CODE BELOW.  YOU SHOLD NOT TOUCH, OR REFERENCE (in your code) ANY
    # OF THE DATA STRUCTURES BELOW.  If you're interested in how I designed
    # the emulator, you're welcome to look at the code - but again, you
    # should not have to, and you defeinitely should not have to modify
    # *****************************************************************

    # the event list
    evlist: Optional[Event] = None

    # possible events:
    FROM_LAYER2: int = 2
    LINK_CHANGE: int = 10

    clocktime: float = 0.000

    @classmethod
    def main(cls, argv: list[str]):
        inputInfo = \
            'RouterSimulator.py ' + \
            '-c <LINKCHANGE (bool)> ' + \
            '-n <NODES (int)> ' + \
            '-p <POISONREVERSE (bool)> ' + \
            '-s <SEED (int)> ' + \
            '-t <TRACE (int)>' + \
            '\n'
        try:
            opts, args = getopt.getopt(
                argv,
                "c:n:p:s:t:",
                ["changelinks=", "nodes=", "poison=", "seed=", "trace="],
            )
        except getopt.GetoptError:
            print(inputInfo)
            sys.exit(2)
        try:
            for opt, arg in opts:
                if opt in ("-c", "--changelinks"):
                    if arg.lower() in ("true", "1", "y", "yes", "t"):
                        cls.LINKCHANGES = True
                    elif arg.lower() in ("false", "0", "n", "no", "f"):
                        cls.LINKCHANGES = False
                if opt in ("-n", "--nodes"):
                    cls.NUM_NODES = int(arg)
                if opt in ("-p", "--poison"):
                    if arg.lower() in ("true", "1", "y", "yes", "t"):
                        cls.POISONREVERSE = True
                    elif arg.lower() in ("false", "0", "n", "no", "f"):
                        cls.POISONREVERSE = False
                if opt in ("-s", "--seed"):
                    cls.SEED = int(arg)
                elif opt in ("-t", "--trace"):
                    cls.TRACE = int(arg)
        except ValueError:
            print(inputInfo)
            sys.exit(2)

        sim = RouterSimulator()
        sim.runSimulation()

    # initialize the simulator
    def __init__(self):
        self.connectcosts: list[list[int]] = [
            [0]*self.NUM_NODES for i in range(self.NUM_NODES)
        ]
        evptr: Optional[Event] = None
        self.myGUI = GuiTextArea.GuiTextArea(
            "  Output window for Router Simulator  ",
        )

        random.seed(self.SEED)
        # initialize time to 0.0
        self.clocktime: float = 0.0

        #  set initial costs
        #  non-defined connections (n-n) defaulted to 0
        if self.NUM_NODES == 3:
            self.connectcosts[0][1] = 4
            self.connectcosts[0][2] = 1
            self.connectcosts[1][0] = 4
            self.connectcosts[1][2] = 50
            self.connectcosts[2][0] = 1
            self.connectcosts[2][1] = 50
        elif self.NUM_NODES == 4:
            self.connectcosts[0][1] = 1
            self.connectcosts[0][2] = 3
            self.connectcosts[0][3] = 7
            self.connectcosts[1][0] = 1
            self.connectcosts[1][2] = 1
            self.connectcosts[1][3] = self.INFINITY
            self.connectcosts[2][0] = 3
            self.connectcosts[2][1] = 1
            self.connectcosts[2][3] = 2
            self.connectcosts[3][0] = 7
            self.connectcosts[3][1] = self.INFINITY
            self.connectcosts[3][2] = 2
        elif self.NUM_NODES == 5:
            self.connectcosts[0][1] = 1
            self.connectcosts[0][2] = 3
            self.connectcosts[0][3] = 7
            self.connectcosts[0][4] = 1
            self.connectcosts[1][0] = 1
            self.connectcosts[1][2] = 1
            self.connectcosts[1][3] = self.INFINITY
            self.connectcosts[1][4] = 1
            self.connectcosts[2][0] = 3
            self.connectcosts[2][1] = 1
            self.connectcosts[2][3] = 2
            self.connectcosts[2][4] = 4
            self.connectcosts[3][0] = 7
            self.connectcosts[3][1] = self.INFINITY
            self.connectcosts[3][2] = 2
            self.connectcosts[3][4] = self.INFINITY
            self.connectcosts[4][0] = 1
            self.connectcosts[4][1] = 1
            self.connectcosts[4][2] = 4
            self.connectcosts[4][3] = self.INFINITY
        else:
            sys.exit('Unsupported number of nodes.')

        self.nodes: list[RouterNode.RouterNode] = [
            RouterNode.RouterNode(i, self, self.connectcosts[i])
            for i in range(self.NUM_NODES)
        ]

        #  initialize future link changes
        if self.LINKCHANGES:
            if self.NUM_NODES == 3:
                evptr = Event()
                evptr.evtime = 40.0
                evptr.evtype = self.LINK_CHANGE
                evptr.eventity = 0
                evptr.rtpktptr = None
                evptr.dest = 1
                evptr.cost = 60
                self.insertevent(evptr)
            elif self.NUM_NODES == 4 or self.NUM_NODES == 5:
                evptr = Event()
                evptr.evtime = 10000.0
                evptr.evtype = self.LINK_CHANGE
                evptr.eventity = 0
                evptr.rtpktptr = None
                evptr.dest = 3
                evptr.cost = 1
                self.insertevent(evptr)

                evptr = Event()
                evptr.evtime = 20000.0
                evptr.evtype = self.LINK_CHANGE
                evptr.eventity = 0
                evptr.rtpktptr = None
                evptr.dest = 1
                evptr.cost = 6
                self.insertevent(evptr)
            else:
                sys.exit('Unsupported number of nodes.')

    def runSimulation(self):
        eventptr: Optional[Event] = None
        while True:
            # get next event to simulate
            eventptr = self.evlist
            if eventptr is None:
                break
            # remove this event from event list
            if self.evlist is not None:
                self.evlist = self.evlist.next
            if self.evlist is not None:
                self.evlist.prev = None
            if self.TRACE > 1:
                self.myGUI.println(
                    "MAIN: rcv event, t=" +
                    str(eventptr.evtime) +
                    " at " +
                    str(eventptr.eventity),
                )
                if eventptr.evtype == self.FROM_LAYER2:
                    assert eventptr.rtpktptr is not None
                    self.myGUI.print(" src:" + str(eventptr.rtpktptr.sourceid))
                    self.myGUI.print(", dest:" + str(eventptr.rtpktptr.destid))
                    self.myGUI.print(", contents:")
                    for i in range(self.NUM_NODES):
                        self.myGUI.print(
                            " " + str(eventptr.rtpktptr.mincost[i]),
                        )
                    self.myGUI.println()

            # update time to next event time
            self.clocktime = eventptr.evtime
            if eventptr.evtype == self.FROM_LAYER2:
                if (eventptr.eventity >= 0 and
                        eventptr.eventity < self.NUM_NODES):
                    self.nodes[eventptr.eventity].recvUpdate(eventptr.rtpktptr)
                else:
                    sys.exit('Panic: unknown event entity\n')
            elif eventptr.evtype == self.LINK_CHANGE:
                # change link costs here if implemented
                self.nodes[eventptr.eventity].updateLinkCost(
                    eventptr.dest, eventptr.cost)
                self.nodes[eventptr.dest].updateLinkCost(
                    eventptr.eventity, eventptr.cost)
            else:
                sys.exit('Panic: unknown event entity\n')

            if self.TRACE > 2:
                for i in range(self.NUM_NODES):
                    self.nodes[i].printDistanceTable()

        self.myGUI.println(
            "\nSimulator terminated at t=" +
            str(self.clocktime) +
            ", no packets in medium\n",
        )

        self.myGUI.myGUI.mainloop()

    def getClocktime(self):
        return self.clocktime

    #  ********************* EVENT HANDLINE ROUTINES *******
    #   The next set of routines handle the event list     *
    #  *****************************************************

    def insertevent(self, p: Event):
        if self.TRACE > 3:
            self.myGUI.println(
                "            INSERTEVENT: time is " +
                str(self.clocktime),
            )
            self.myGUI.println(
                "            INSERTEVENT: future time will be " +
                str(p.evtime),
            )
        # q points to header of list in which p struct inserted
        q: Optional[Event] = self.evlist
        qold: Optional[Event] = None
        if q is None:
            # list is empty
            self.evlist = p
            p.next = None
            p.prev = None
        else:
            qold = q
            while (q is not None and p.evtime > q.evtime):
                qold = q
                q = q.next
            if q is None:
                # end of list
                qold.next = p
                p.prev = qold
                p.next = None
            elif q == self.evlist:
                # front of list
                p.next = self.evlist
                p.prev = None
                p.next.prev = p
                self.evlist = p
            else:
                # middle of list
                p.next = q
                p.prev = q.prev
                assert q.prev is not None
                q.prev.next = p
                q.prev = p

    def printevlist(self):
        q = self.evlist
        self.myGUI.println("--------------\nEvent List Follows:")
        while q is not None:
            self.myGUI.println(
                "Event time: " + str(q.evtime) +
                ", type: " + str(q.evtype) +
                " entity: " + str(q.eventity),
            )
            q = q.next
        self.myGUI.println("--------------")

# ************************** TOLAYER2 ***************
    def toLayer2(self, packet: RouterPacket.RouterPacket):
        # be nice: check if source and destination id's are reasonable
        if packet.sourceid < 0 or packet.sourceid > self.NUM_NODES - 1:
            self.myGUI.println(
                "WARN: illegal source id in your packet, ignoring packet!",
            )
            return
        if packet.destid < 0 or packet.destid > self.NUM_NODES - 1:
            self.myGUI.println(
                "WARN: illegal dest id in your packet, ignoring packet!",
            )
            return
        if packet.sourceid == packet.destid:
            self.myGUI.println(
                "WARN: source and destination id's the same, ignoring packet",
            )
            return
        if self.connectcosts[packet.sourceid][packet.destid] == self.INFINITY:
            self.myGUI.println(
                "WARN: source and destination not connected, ignoring packet",
            )
            return

        # make a copy of the packet student just gave me since may
        # be modified after we return back
        mypktptr: RouterPacket.RouterPacket = packet.clone()

        if (self.TRACE > 2):
            self.myGUI.print(
                "    TOLAYER2: source: " + str(mypktptr.sourceid) +
                " dest: " + str(mypktptr.destid) +
                "             costs:",
            )
            for i in range(self.NUM_NODES):
                self.myGUI.print(str(mypktptr.mincost[i]) + " ")
            self.myGUI.println()

        # create future event for arrival of packet at the other side
        evptr = Event()
        # packet will pop out from layer3
        evptr.evtype = self.FROM_LAYER2
        # event occurs at other entity
        evptr.eventity = packet.destid
        # save ptr to my copy of packet
        evptr.rtpktptr = mypktptr

        # finally, compute the arrival time of packet at the other end.
        # medium can not reorder, so make sure packet arrives between 1
        # and 10 time units after the latest arrival time of packets
        # currently in the medium on their way to the destination
        lastime = self.clocktime
        q: Optional[Event] = self.evlist
        while (q is not None):
            if (q.evtype == self.FROM_LAYER2 and q.eventity == evptr.eventity):
                lastime = q.evtime
            q = q.next
        evptr.evtime = lastime + 9 * random.random() + 1

        if self.TRACE > 2:
            self.myGUI.println(
                "    TOLAYER2: scheduling arrival on other side",
            )

        self.insertevent(evptr)


if __name__ == '__main__':
    RouterSimulator.main(sys.argv[1:])
