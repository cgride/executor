/**
 *
 *  @file process_result_test.cpp
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
#include <string_view>
#include <string>

#include <cgride/executor/process_result.hpp>

int main()
{
  {
    using cgride::executor::ProcessStatus;
    using cgride::executor::to_string;

    assert(to_string(ProcessStatus::NotStarted) == std::string_view("NotStarted"));
    assert(to_string(ProcessStatus::Exited) == std::string_view("Exited"));
    assert(to_string(ProcessStatus::FailedToStart) == std::string_view("FailedToStart"));
    assert(to_string(ProcessStatus::TimedOut) == std::string_view("TimedOut"));
    assert(to_string(ProcessStatus::Cancelled) == std::string_view("Cancelled"));
  }

  {
    cgride::executor::ProcessResult result;

    assert(result.status() == cgride::executor::ProcessStatus::NotStarted);
    assert(!result.has_exit_code());
    assert(!result.exit_code().has_value());
    assert(result.standard_output().empty());
    assert(result.standard_error().empty());
    assert(!result.has_error());
    assert(!result.error().has_value());
    assert(result.duration() == std::chrono::milliseconds{0});
    assert(!result.success());
    assert(result.failed());
    assert(!result.finished());
  }

  {
    auto result = cgride::executor::ProcessResult::exited(
        0,
        "hello\n",
        "",
        std::chrono::milliseconds{12});

    assert(result.status() == cgride::executor::ProcessStatus::Exited);
    assert(result.has_exit_code());
    assert(result.exit_code().value() == 0);
    assert(result.standard_output() == "hello\n");
    assert(result.standard_error().empty());
    assert(!result.has_error());
    assert(result.duration() == std::chrono::milliseconds{12});
    assert(result.success());
    assert(!result.failed());
    assert(result.finished());
  }

  {
    auto result = cgride::executor::ProcessResult::exited(
        42,
        "",
        "compile error\n",
        std::chrono::milliseconds{7});

    assert(result.status() == cgride::executor::ProcessStatus::Exited);
    assert(result.has_exit_code());
    assert(result.exit_code().value() == 42);
    assert(result.standard_error() == "compile error\n");
    assert(result.has_error());
    assert(result.error().value().code() == cgride::core::ErrorCode::ProcessFailed);
    assert(result.error().value().message() == "Process exited with a non-zero exit code.");
    assert(result.error().value().detail().has_value());
    assert(result.error().value().detail().value() == "42");
    assert(result.duration() == std::chrono::milliseconds{7});
    assert(!result.success());
    assert(result.failed());
    assert(result.finished());
  }

  {
    auto result = cgride::executor::ProcessResult::failed_to_start(
        cgride::core::Error(
            cgride::core::ErrorCode::ProcessFailed,
            "Failed to start process.",
            std::string("permission denied")),
        "permission denied\n");

    assert(result.status() == cgride::executor::ProcessStatus::FailedToStart);
    assert(!result.has_exit_code());
    assert(result.standard_error() == "permission denied\n");
    assert(result.has_error());
    assert(result.error().value().code() == cgride::core::ErrorCode::ProcessFailed);
    assert(result.error().value().message() == "Failed to start process.");
    assert(result.error().value().detail().has_value());
    assert(result.error().value().detail().value() == "permission denied");
    assert(!result.success());
    assert(result.failed());
    assert(result.finished());
  }

  {
    auto result = cgride::executor::ProcessResult::cancelled();

    assert(result.status() == cgride::executor::ProcessStatus::Cancelled);
    assert(!result.has_exit_code());
    assert(result.has_error());
    assert(result.error().value().code() == cgride::core::ErrorCode::Cancelled);
    assert(result.error().value().message() == "Process execution was cancelled.");
    assert(!result.success());
    assert(result.failed());
    assert(result.finished());
  }

  {
    auto result = cgride::executor::ProcessResult::timed_out();

    assert(result.status() == cgride::executor::ProcessStatus::TimedOut);
    assert(!result.has_exit_code());
    assert(result.has_error());
    assert(result.error().value().code() == cgride::core::ErrorCode::Timeout);
    assert(result.error().value().message() == "Process execution timed out.");
    assert(!result.success());
    assert(result.failed());
    assert(result.finished());
  }

  {
    cgride::executor::ProcessResult result;

    result
        .status(cgride::executor::ProcessStatus::Exited)
        .exit_code(0)
        .standard_output("hello")
        .append_standard_output(" cgride")
        .standard_error("warn")
        .append_standard_error("ing")
        .duration(std::chrono::milliseconds{21});

    assert(result.status() == cgride::executor::ProcessStatus::Exited);
    assert(result.exit_code().value() == 0);
    assert(result.standard_output() == "hello cgride");
    assert(result.standard_error() == "warning");
    assert(result.duration() == std::chrono::milliseconds{21});
    assert(result.success());
    assert(result.finished());
  }

  {
    cgride::executor::ProcessResult result;

    result.error(
        cgride::core::Error(
            cgride::core::ErrorCode::InternalError,
            "Internal process error."));

    assert(result.has_error());
    assert(result.error().value().code() == cgride::core::ErrorCode::InternalError);
    assert(result.error().value().message() == "Internal process error.");
  }

  return 0;
}
