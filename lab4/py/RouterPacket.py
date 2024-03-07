#!/usr/bin/env python

from copy import deepcopy


class RouterPacket(object):
    def __init__(self, sourceID: int, destID: int, mincosts: list[int]):
        super(RouterPacket, self).__init__()
        # id of sending router sending this pkt
        self.sourceid: int = sourceID
        # id of router to which pkt being sent (must be an immediate neighbor)
        self.destid: int = destID
        # min cost to node 0 ... 3
        self.mincost: list[int] = deepcopy(mincosts)

    def clone(self):
        return RouterPacket(self.sourceid, self.destid, deepcopy(self.mincost))
