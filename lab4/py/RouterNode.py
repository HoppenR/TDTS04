#!/usr/bin/env python
import F
from RouterPacket import RouterPacket
from GuiTextArea import GuiTextArea
from RouterSimulator import RouterSimulator

from copy import deepcopy
from typing import Optional
from random import random


class RouterNode():
    # Access simulator variables with:
    # self.sim.POISONREVERSE, self.sim.NUM_NODES, etc.

    def __init__(self, ID: int, sim: RouterSimulator, costs: list[int]):
        self.myGUI = GuiTextArea(
            '  Output window for Router #' + str(ID) + '  ',
        )
        self.sim: RouterSimulator = sim
        self.myID: int = ID
        self.costs: list[int] = costs
        self.distances: list[list[int]] = [
            [999] * self.sim.NUM_NODES
            for _ in range(self.sim.NUM_NODES)
        ]
        self.routes: list[str] = ['-'] * self.sim.NUM_NODES

        # Init distances and routes info
        self.distances[self.myID] = deepcopy(costs)
        for i in range(self.sim.NUM_NODES):
            if self.costs[i] != self.sim.INFINITY:
                self.routes[i] = str(i)
        # Notify other nodes on the network about our distances
        self.notifyNeighbors()

    # --------------------------------------------------

    def updateDistanceCosts(self) -> None:
        for target in range(self.sim.NUM_NODES):
            if target == self.myID:
                continue
            nextRoutes: list[int] = list()
            nextHop: list[int] = list()
            for next in range(self.sim.NUM_NODES):
                if next == self.myID:
                    continue
                nextRoutes.append(
                    self.costs[next] + self.distances[next][target],
                )
                nextHop.append(next)
            minCost: int = self.sim.INFINITY
            minHopID: int
            for next in range(len(nextRoutes)):
                if nextRoutes[next] < minCost:
                    minCost = nextRoutes[next]
                    minHopID = nextHop[next]
            self.distances[self.myID][target] = minCost
            self.routes[target] = str(minHopID)

    # --------------------------------------------------

    def notifyNeighbors(self, fakeindex: Optional[int] = None) -> None:
        if not self.sim.POISONREVERSE:
            for neighborID in range(self.sim.NUM_NODES):
                if neighborID == self.myID:
                    continue
                pkt = RouterPacket(
                    self.myID,
                    neighborID,
                    deepcopy(self.distances[self.myID]),
                )
                self.sendUpdate(pkt)
        else:
            fakevector: list[int] = deepcopy(self.distances[self.myID])
            if fakeindex is not None:
                fakevector[fakeindex] = self.sim.INFINITY
            for neighborID in range(self.sim.NUM_NODES):
                if neighborID == self.myID:
                    continue
                if self.costs[neighborID] == self.sim.INFINITY:
                    continue
                pkt = RouterPacket(
                    self.myID,
                    neighborID,
                    fakevector,
                )
                self.sendUpdate(pkt)

    # --------------------------------------------------

    def recvUpdate(self, node: Optional[RouterPacket]) -> None:
        assert node is not None
        self.distances[node.sourceid] = node.mincost
        oldDistance: list[list[int]] = deepcopy(self.distances)
        self.updateDistanceCosts()
        if oldDistance != self.distances:
            self.notifyNeighbors()

    # --------------------------------------------------

    def sendUpdate(self, node: RouterPacket) -> None:
        assert node is not None
        self.sim.toLayer2(node)

    # --------------------------------------------------

    def printDistanceTable(self) -> None:
        self.myGUI.println(
            f'Current state for router {self.myID} at time ' +
            f'{self.sim.getClocktime():.1f}\n',
        )

        self.myGUI.println('Distancetable')
        self.myGUI.print('    dst |')
        for i in range(self.sim.NUM_NODES):
            self.myGUI.print(f'{i: >5}')
        self.myGUI.println()

        self.myGUI.println('---------' + '-----' * self.sim.NUM_NODES)

        for i in range(self.sim.NUM_NODES):
            self.myGUI.print(f' nbr{i: <4}|')
            for j in range(self.sim.NUM_NODES):
                self.myGUI.print(f'{self.distances[i][j]: >5}')
            self.myGUI.println()
        self.myGUI.println()

        self.myGUI.println('Our distance vector and routes:')
        self.myGUI.print('    dst |')
        for i in range(self.sim.NUM_NODES):
            self.myGUI.print(f'{i: >5}')
        self.myGUI.println()

        self.myGUI.println('---------' + '-----' * self.sim.NUM_NODES)

        self.myGUI.print(' cost   |')
        for i in range(self.sim.NUM_NODES):
            self.myGUI.print(f'{self.distances[self.myID][i]: >5}')
        self.myGUI.println()

        self.myGUI.print(' route  |')
        for i in range(self.sim.NUM_NODES):
            self.myGUI.print(f'{self.routes[i]: >5}')
        self.myGUI.println('\n\n')

    # --------------------------------------------------

    def updateLinkCost(self, dest: int, newcost: int) -> None:
        self.costs[dest] = newcost
        self.updateDistanceCosts()
        self.notifyNeighbors(dest)
