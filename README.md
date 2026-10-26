# MiniHarness (Project 3)

A small C++17 agent harness: it runs a conversation loop, lets a (scripted) model call tools, and feeds the tool results back into the conversation.

Project 3 builds on my Project 2 conversation code and adds a tool system:

- **`ToolRegistry`**: a hash table (separate chaining, FNV-1a hash) that stores tools by name
- **`Workspace::resolve`**: path checking so file tools can only touch files inside the workspace folder
- **`calc_evaluate`**: the expression evaluator behind the calculator tool

## Requirements

- Linux or WSL2
- `g++` with C++17 support
- `cmake` 3.16 or newer

## Build

```bash
cmake -S . -B build
cmake --build build -j
```

Optional flags:

| Flag | Effect |
|---|---|
| `-DUSE_P2_REFERENCE=ON` | Use the instructor's Project 2 reference code instead of my own |
| `-DMH_WERROR=ON` | Treat compiler warnings as errors |

## Run

The `--script` file holds the fake model's replies. The user's lines come from stdin.

```bash
# Conversation without tools
./build/miniharness --no-tools --script scripts/greeting.script < tests/e2e/greeting.input

# Conversation with tools
./build/miniharness --script scripts/tools_demo.script < tests/e2e/tools_demo.input

# List the registered tools
./build/miniharness --tools
```

To type your own input, leave off the `<` redirect and finish the session with `bye`.

## Test

```bash
./build/test_p2      # Project 2 regression tests
./build/test_loop    # tool-loop tests
./build/test_p3      # my own tests for the registry, workspace and calculator

cd build && ctest --output-on-failure   # everything, including end-to-end tests
```

Pass a name prefix to run a subset, for example `./build/test_p3 reg_`.

## Project layout

```
include/        headers
src/core/       conversation and sentinel scanner (Project 2)
src/tools/      tool registry, workspace, calculator, built-in tools
src/json/       JSON value type
src/harness/    conversation loop and tool dispatcher
src/model/      scripted model
scripts/        model scripts used by the demos
tests/          unit tests (p2, p3) and end-to-end tests (e2e)
```

## Design notes

- **Registry:** separate chaining with `std::unique_ptr<Tool>` in each bucket. Tools never move when the table resizes, so pointers returned by `find()` stay valid.
- **Hash:** FNV-1a over every character, returned at full width and reduced to a bucket index separately.
- **Resizing:** the bucket count doubles when the load factor would go above the threshold, and every entry is re-inserted with its index recomputed.
- **Workspace safety:** model-supplied paths are treated as untrusted and must resolve to a location inside the workspace root.

## Author

Athena Larson
