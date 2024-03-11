# TDTS04 Lab4 rewritten in C++ and Qt5

This is a full rewrite of the TDTS04 Lab template as well as an implementation
of a solution to the count-to-infinity problem using poisoned reverse.

The template part of this code is very similar to the Java template, with only
minor differences.

It has been fully tested on Linux and on Windows via WSL (but should work on any
system that has the qt5 libraries).

## Solution

Propagate any received update that changes the distance vector of a node.
Also propagate any updated link costs. In the case of poisoned reverse the
updated link cost should be advertised as infinite to any neighboring node.

Further explanations can be found in the code comments in `src/RouterNode.cpp`.

## Requirements

- `Qt5` (already exists on lab computers, package probably called `qtbase5-dev`)


## Compiling and Running

```bash
make
./RouterSimulator
```
