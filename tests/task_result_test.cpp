/**
 *
 *  @file task_result_test.cpp
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

#include <cgride/executor/task_result.hpp>

int main()
{
  {
    using cgride::executor::TaskStatus;
    using cgride::executor::to_string;

    assert(to_string(TaskStatus::Pending) == std::string_view("Pending"));
    assert(to_string(TaskStatus::Started) == std::string_view("Started"));
    assert(to_string(TaskStatus::Skipped) == std::string_view("Skipped"));
    assert(to_string(TaskStatus::Succeeded) == std::string_view("Succeeded"));
    assert(to_string(TaskStatus::Failed) == std::string_view("Failed"));
    assert(to_string(TaskStatus::Cancelled) == std::string_view("Cancelled"));
  }

  {
    cgride::executor::TaskResult result;

    assert(result.task_id().empty());
    assert(result.task_name().empty());
    assert(result.status() == cgride::executor::TaskStatus::Pending);
    assert(result.message().empty());
    assert(!result.has_process_result());
    assert(!result.process_result().has_value());
    assert(!result.has_error());
    assert(!result.error().has_value());
    assert(result.duration() == std::chrono::milliseconds{0});
    assert(!result.valid());
    assert(!result.success());
    assert(!result.failed());
    assert(!result.finished());
  }

  {
    auto id = cgride::graph::TaskId::from_string("compile:main.cpp");
    cgride::executor::TaskResult result(id);

    assert(result.task_id() == id);
    assert(result.status() == cgride::executor::TaskStatus::Pending);
    assert(result.valid());
  }

  {
    auto result = cgride::executor::TaskResult::pending(
        cgride::graph::TaskId::from_string("prepare"));

    assert(result.task_id() == cgride::graph::TaskId::from_string("prepare"));
    assert(result.status() == cgride::executor::TaskStatus::Pending);
    assert(result.valid());
    assert(!result.success());
    assert(!result.failed());
    assert(!result.finished());
  }

  {
    auto result = cgride::executor::TaskResult::started(
        cgride::graph::TaskId::from_string("compile"));

    assert(result.task_id() == cgride::graph::TaskId::from_string("compile"));
    assert(result.status() == cgride::executor::TaskStatus::Started);
    assert(result.valid());
    assert(!result.success());
    assert(!result.failed());
    assert(!result.finished());
  }

  {
    auto result = cgride::executor::TaskResult::skipped(
        cgride::graph::TaskId::from_string("prepare"),
        "Task has no command.",
        std::chrono::milliseconds{3});

    assert(result.task_id() == cgride::graph::TaskId::from_string("prepare"));
    assert(result.status() == cgride::executor::TaskStatus::Skipped);
    assert(result.message() == "Task has no command.");
    assert(result.duration() == std::chrono::milliseconds{3});
    assert(result.valid());
    assert(result.success());
    assert(!result.failed());
    assert(result.finished());
  }

  {
    auto result = cgride::executor::TaskResult::succeeded(
        cgride::graph::TaskId::from_string("link"),
        std::chrono::milliseconds{17});

    assert(result.task_id() == cgride::graph::TaskId::from_string("link"));
    assert(result.status() == cgride::executor::TaskStatus::Succeeded);
    assert(result.duration() == std::chrono::milliseconds{17});
    assert(!result.has_process_result());
    assert(!result.has_error());
    assert(result.valid());
    assert(result.success());
    assert(!result.failed());
    assert(result.finished());
  }

  {
    auto process = cgride::executor::ProcessResult::exited(
        0,
        "ok\n",
        "",
        std::chrono::milliseconds{9});

    auto result = cgride::executor::TaskResult::succeeded(
        cgride::graph::TaskId::from_string("compile"),
        process);

    assert(result.task_id() == cgride::graph::TaskId::from_string("compile"));
    assert(result.status() == cgride::executor::TaskStatus::Succeeded);
    assert(result.duration() == std::chrono::milliseconds{9});
    assert(result.has_process_result());
    assert(result.process_result().value().success());
    assert(result.process_result().value().standard_output() == "ok\n");
    assert(!result.has_error());
    assert(result.success());
    assert(result.finished());
  }

  {
    auto result = cgride::executor::TaskResult::failed(
        cgride::graph::TaskId::from_string("compile"),
        cgride::core::Error(
            cgride::core::ErrorCode::ProcessFailed,
            "Task failed."),
        std::chrono::milliseconds{11});

    assert(result.task_id() == cgride::graph::TaskId::from_string("compile"));
    assert(result.status() == cgride::executor::TaskStatus::Failed);
    assert(result.duration() == std::chrono::milliseconds{11});
    assert(result.has_error());
    assert(result.error().value().code() == cgride::core::ErrorCode::ProcessFailed);
    assert(result.error().value().message() == "Task failed.");
    assert(!result.success());
    assert(result.failed());
    assert(result.finished());
  }

  {
    auto process = cgride::executor::ProcessResult::exited(
        2,
        "",
        "error\n",
        std::chrono::milliseconds{14});

    auto result = cgride::executor::TaskResult::failed(
        cgride::graph::TaskId::from_string("link"),
        process);

    assert(result.task_id() == cgride::graph::TaskId::from_string("link"));
    assert(result.status() == cgride::executor::TaskStatus::Failed);
    assert(result.duration() == std::chrono::milliseconds{14});
    assert(result.has_process_result());
    assert(result.process_result().value().exit_code().value() == 2);
    assert(result.process_result().value().standard_error() == "error\n");
    assert(result.has_error());
    assert(result.error().value().code() == cgride::core::ErrorCode::ProcessFailed);
    assert(!result.success());
    assert(result.failed());
    assert(result.finished());
  }

  {
    auto result = cgride::executor::TaskResult::cancelled(
        cgride::graph::TaskId::from_string("archive"));

    assert(result.task_id() == cgride::graph::TaskId::from_string("archive"));
    assert(result.status() == cgride::executor::TaskStatus::Cancelled);
    assert(result.has_error());
    assert(result.error().value().code() == cgride::core::ErrorCode::Cancelled);
    assert(result.error().value().message() == "Task execution was cancelled.");
    assert(!result.success());
    assert(result.failed());
    assert(result.finished());
  }

  {
    cgride::executor::TaskResult result;

    result
        .task_id(cgride::graph::TaskId::from_string("custom"))
        .task_name("Generate files")
        .status(cgride::executor::TaskStatus::Succeeded)
        .message("Done.")
        .process_result(cgride::executor::ProcessResult::exited(0))
        .duration(std::chrono::milliseconds{5});

    assert(result.task_id() == cgride::graph::TaskId::from_string("custom"));
    assert(result.task_name() == "Generate files");
    assert(result.status() == cgride::executor::TaskStatus::Succeeded);
    assert(result.message() == "Done.");
    assert(result.has_process_result());
    assert(result.duration() == std::chrono::milliseconds{5});
    assert(result.valid());
    assert(result.success());
  }

  {
    cgride::executor::TaskResult result;

    result.error(
        cgride::core::Error(
            cgride::core::ErrorCode::InvalidState,
            "Invalid task state."));

    assert(result.has_error());
    assert(result.error().value().code() == cgride::core::ErrorCode::InvalidState);
    assert(result.error().value().message() == "Invalid task state.");
  }

  return 0;
}
