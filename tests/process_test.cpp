/**
 *
 *  @file process_test.cpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2026, Gaspard Kirira.
 *  All rights reserved.
 *  https://github.com/cgride/executor
 *
 *  Use of this source code is governed by a MIT license
 *  that can be found in the LICENSE file.
 *
 *  Cgride
 *
 */
#include <cassert>
#include <chrono>
#include <string>

#include <cgride/core/command.hpp>
#include <cgride/executor/process.hpp>

int main()
{
  {
    cgride::core::Command command;

    command
        .program("g++")
        .arg("-c")
        .arg("src/main.cpp")
        .arg("-o")
        .arg("build/main.o");

    auto description = cgride::executor::describe_command(command);

    assert(description == "g++ -c src/main.cpp -o build/main.o");
  }

  {
    cgride::core::Command command;

    command
        .program("/usr/bin/my compiler")
        .arg("-DAPP_NAME=hello world")
        .arg("src/main.cpp");

    auto description = cgride::executor::describe_command(command);

    assert(description == "\"/usr/bin/my compiler\" \"-DAPP_NAME=hello world\" src/main.cpp");
  }

  {
    cgride::core::Command command;

    cgride::executor::ExecutionOptions options;

    auto result = cgride::executor::run_process(command, options);

    assert(!result);
    assert(result.error().code() == cgride::core::ErrorCode::InvalidArgument);
    assert(result.error().message() == "Cannot run an invalid command.");
  }

  {
    cgride::core::Command command;

    command
        .program("cgride-dry-run-tool")
        .arg("--version");

    cgride::executor::ExecutionOptions options;

    options.dry_run(true);

    auto result = cgride::executor::run_process(command, options);

    assert(result);

    auto process = result.value();

    assert(process.success());
    assert(process.status() == cgride::executor::ProcessStatus::Exited);
    assert(process.has_exit_code());
    assert(process.exit_code().value() == 0);
    assert(process.standard_output() == "cgride-dry-run-tool --version");
    assert(process.standard_error().empty());
  }

#if !defined(_WIN32)
  {
    cgride::core::Command command;

    command
        .program("/bin/sh")
        .arg("-c")
        .arg("printf 'hello'; printf 'error' >&2");

    cgride::executor::ExecutionOptions options;

    auto result = cgride::executor::run_process(command, options);

    assert(result);

    auto process = result.value();

    assert(process.success());
    assert(process.status() == cgride::executor::ProcessStatus::Exited);
    assert(process.has_exit_code());
    assert(process.exit_code().value() == 0);
    assert(process.standard_output() == "hello");
    assert(process.standard_error() == "error");
    assert(process.finished());
  }

  {
    cgride::core::Command command;

    command
        .program("/bin/sh")
        .arg("-c")
        .arg("printf 'failed' >&2; exit 7");

    cgride::executor::ExecutionOptions options;

    auto result = cgride::executor::run_process(command, options);

    assert(result);

    auto process = result.value();

    assert(!process.success());
    assert(process.failed());
    assert(process.status() == cgride::executor::ProcessStatus::Exited);
    assert(process.has_exit_code());
    assert(process.exit_code().value() == 7);
    assert(process.standard_output().empty());
    assert(process.standard_error() == "failed");
    assert(process.has_error());
    assert(process.error().value().code() == cgride::core::ErrorCode::ProcessFailed);
    assert(process.finished());
  }

  {
    cgride::core::Command command;

    command
        .program("/bin/sh")
        .arg("-c")
        .arg("sleep 2");

    cgride::executor::ExecutionOptions options;

    options.timeout(std::chrono::milliseconds{50});

    auto result = cgride::executor::run_process(command, options);

    assert(result);

    auto process = result.value();

    assert(!process.success());
    assert(process.failed());
    assert(process.status() == cgride::executor::ProcessStatus::TimedOut);
    assert(!process.has_exit_code());
    assert(process.has_error());
    assert(process.error().value().code() == cgride::core::ErrorCode::Timeout);
    assert(process.finished());
  }

  {
    cgride::core::CancellationSource source;
    cgride::core::Command command;

    command
        .program("/bin/sh")
        .arg("-c")
        .arg("sleep 2");

    cgride::executor::ExecutionOptions options;

    options.cancellation_token(source.token());

    source.cancel();

    auto result = cgride::executor::run_process(command, options);

    assert(result);

    auto process = result.value();

    assert(!process.success());
    assert(process.failed());
    assert(process.status() == cgride::executor::ProcessStatus::Cancelled);
    assert(!process.has_exit_code());
    assert(process.has_error());
    assert(process.error().value().code() == cgride::core::ErrorCode::Cancelled);
    assert(process.finished());
  }

  {
    cgride::core::Command command;

    command
        .program("sh")
        .search_in_path(true)
        .arg("-c")
        .arg("printf 'path-ok'");

    cgride::executor::ExecutionOptions options;

    auto result = cgride::executor::run_process(command, options);

    assert(result);

    auto process = result.value();

    assert(process.success());
    assert(process.standard_output() == "path-ok");
  }
#else
  {
    cgride::core::Command command;

    command.program("cmd");

    cgride::executor::ExecutionOptions options;

    auto result = cgride::executor::run_process(command, options);

    assert(result);

    auto process = result.value();

    assert(!process.success());
    assert(process.status() == cgride::executor::ProcessStatus::FailedToStart);
    assert(process.has_error());
    assert(process.error().value().code() == cgride::core::ErrorCode::UnsupportedPlatform);
  }
#endif

  return 0;
}
