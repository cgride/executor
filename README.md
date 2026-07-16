# Cgride Executor

Task graph execution and process running for Cgride.

Cgride Executor is the execution module. It takes a validated Cgride task graph, runs tasks in dependency-first order, executes attached commands, captures process output and returns structured task results.

It does not create build plans, discover toolchains, inspect project targets, decide whether files are dirty, manage cache state or print directly to the terminal.

## Purpose

This module provides the execution-level types used by the Cgride engine:

- process status
- process results
- task status
- task results
- execution options
- command description
- process execution
- graph execution
- stdout and stderr capture
- cancellation handling
- timeout handling
- structured execution events

Cgride Executor keeps execution separate from planning. Higher-level modules can create a `cgride::graph::Graph`, attach `cgride::core::Command` values to graph tasks, then pass that graph to the executor.

## Requirements

- C++23
- CMake 3.22 or newer
- Vix.cpp
- Cgride Core
- Cgride Graph

## Build

```bash
vix build --build-target all
```

## Run tests

```bash
vix tests
```

## Release build

```bash
vix build --preset release --build-target all
```

## Install

```bash
vix build --build-target all
sudo cmake --install build-ninja --prefix /usr/local
```

## CMake usage

```cmake
find_package(cgride-executor CONFIG REQUIRED)

target_link_libraries(my_tool PRIVATE cgride::executor)
```

## Basic example

```cpp
#include <cgride/core/command.hpp>
#include <cgride/executor/executor.hpp>
#include <cgride/graph/graph.hpp>

int main()
{
  cgride::graph::Graph graph("app");

  auto &compile = graph.task(
      cgride::graph::TaskId::from_string("compile:main.cpp"),
      cgride::graph::TaskKind::Compile,
      "Compile main.cpp");

  cgride::core::Command command;

  command
      .program("g++")
      .arg("-c")
      .arg("src/main.cpp")
      .arg("-o")
      .arg("build/main.o");

  compile.command(std::move(command));

  auto result = cgride::executor::execute_graph(graph);

  if (!result)
  {
    return 1;
  }

  for (const auto &task : result.value())
  {
    if (task.failed())
    {
      return 1;
    }
  }

  return 0;
}
```

## Dry-run example

```cpp
#include <cgride/core/command.hpp>
#include <cgride/executor/executor.hpp>
#include <cgride/graph/graph.hpp>

int main()
{
  cgride::graph::Graph graph("dry-run");

  auto &task = graph.task(
      cgride::graph::TaskId::from_string("compile"),
      cgride::graph::TaskKind::Compile);

  task.command(
      cgride::core::Command{}
          .program("g++")
          .arg("-c")
          .arg("src/main.cpp")
          .arg("-o")
          .arg("build/main.o"));

  cgride::executor::ExecutionOptions options;

  options.dry_run(true);

  auto result = cgride::executor::execute_graph(graph, options);

  return result ? 0 : 1;
}
```

## Event example

```cpp
#include <iostream>

#include <cgride/core/event.hpp>
#include <cgride/executor/executor.hpp>

int main()
{
  cgride::executor::ExecutionOptions options;

  options.on_event(
      [](const cgride::core::Event &event)
      {
        std::cout << event.message() << '\n';
      });

  return 0;
}
```

The executor emits events through the configured callback only. It does not write to stdout or stderr directly.

## Process results

`ProcessResult` stores the observable result of running one command:

- status
- exit code
- captured stdout
- captured stderr
- optional structured error
- duration

A process is successful only when it exits normally with code `0`.

```cpp
auto result = cgride::executor::run_process(command);

if (result && result.value().success())
{
  // process exited with code 0
}
```

## Task results

`TaskResult` stores the result of one graph task:

- task id
- task name
- task status
- optional message
- optional process result
- optional structured error
- duration

A task without a command is skipped successfully. This allows the graph to contain logical preparation or grouping tasks.

## Execution options

`ExecutionOptions` controls execution behavior:

- `jobs`
- `stop_on_failure`
- `dry_run`
- `capture_output`
- `inherit_environment`
- `timeout`
- `cancellation_token`
- `on_event`

The first executor implementation is sequential. The `jobs` option is kept in the public API so a later parallel scheduler can be added without changing the integration surface.

## Current platform support

The current process runner executes commands on POSIX systems using `fork`, `exec`, `pipe` and `waitpid`.

On Windows, the process runner currently returns a structured `UnsupportedPlatform` error. Native Windows execution should be implemented later with `CreateProcess`.

## Module boundary

Cgride Executor may depend on:

- `cgride::core`
- `cgride::graph`

Cgride Executor may be used by:

- `cgride::engine`
- `cgride::cli`
- external runtimes
- IDE integrations
- developer tools

Cgride Executor must not depend on:

- `cgride::project`
- `cgride::toolchains`
- `cgride::cache`
- `cgride::engine`
- `cgride::config`
- `cgride::cli`

## Design rule

This module executes already-planned task graphs only.

It should answer:

```txt
Which task is being executed?
Which command should be run for this task?
What did the process return?
What stdout and stderr were captured?
Did the task succeed, fail, skip or cancel?
```

It should not answer:

```txt
Which targets exist?
Which compiler should be used?
Which command should be generated?
Which files are dirty?
Which cache entry should be reused?
How should terminal output be formatted?
```

Those decisions belong to higher-level Cgride modules.

## License

MIT
