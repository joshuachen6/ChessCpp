# ChessCpp
A chess engine written in C++ using SFML and ImGui.

## Features
- Bitboard-based board representation
- Minimax search with Alpha-Beta pruning
- Parallel search using TBB
- ImGui-based user interface with game controls (Undo, Reset, Side selection)
- Engine mode for UCI-like interaction

## Building
This project uses [xmake](https://xmake.io) as its build system.

```bash
xmake f -y
xmake
```

## Running
```bash
xmake run Chess
```
To run in engine mode:
```bash
xmake run Chess --engine
```
