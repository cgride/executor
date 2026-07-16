/**
 *
 *  @file task_result.cpp
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
#include <cgride/executor/task_result.hpp>

#include <utility>

namespace cgride::executor
{
  std::string_view to_string(TaskStatus status) noexcept
  {
    switch (status)
    {
    case TaskStatus::Pending:
      return "Pending";

    case TaskStatus::Started:
      return "Started";

    case TaskStatus::Skipped:
      return "Skipped";

    case TaskStatus::Succeeded:
      return "Succeeded";

    case TaskStatus::Failed:
      return "Failed";

    case TaskStatus::Cancelled:
      return "Cancelled";
    }

    return "Pending";
  }

  TaskResult::TaskResult(cgride::graph::TaskId task_id)
      : task_id_(std::move(task_id))
  {
  }

  TaskResult TaskResult::pending(cgride::graph::TaskId task_id)
  {
    TaskResult result(std::move(task_id));
    result.status_ = TaskStatus::Pending;
    return result;
  }

  TaskResult TaskResult::started(cgride::graph::TaskId task_id)
  {
    TaskResult result(std::move(task_id));
    result.status_ = TaskStatus::Started;
    return result;
  }

  TaskResult TaskResult::skipped(
      cgride::graph::TaskId task_id,
      std::string message,
      std::chrono::milliseconds duration)
  {
    TaskResult result(std::move(task_id));

    result.status_ = TaskStatus::Skipped;
    result.message_ = std::move(message);
    result.duration_ = duration;

    return result;
  }

  TaskResult TaskResult::succeeded(
      cgride::graph::TaskId task_id,
      std::chrono::milliseconds duration)
  {
    TaskResult result(std::move(task_id));

    result.status_ = TaskStatus::Succeeded;
    result.duration_ = duration;

    return result;
  }

  TaskResult TaskResult::succeeded(
      cgride::graph::TaskId task_id,
      ProcessResult process_result)
  {
    TaskResult result(std::move(task_id));

    result.status_ = TaskStatus::Succeeded;
    result.duration_ = process_result.duration();
    result.process_result_ = std::move(process_result);

    return result;
  }

  TaskResult TaskResult::failed(
      cgride::graph::TaskId task_id,
      cgride::core::Error error,
      std::chrono::milliseconds duration)
  {
    TaskResult result(std::move(task_id));

    result.status_ = TaskStatus::Failed;
    result.error_ = std::move(error);
    result.duration_ = duration;

    return result;
  }

  TaskResult TaskResult::failed(
      cgride::graph::TaskId task_id,
      ProcessResult process_result)
  {
    TaskResult result(std::move(task_id));

    result.status_ = TaskStatus::Failed;
    result.duration_ = process_result.duration();

    if (process_result.has_error())
    {
      result.error_ = process_result.error().value();
    }
    else
    {
      result.error_ = cgride::core::Error(
          cgride::core::ErrorCode::ProcessFailed,
          "Task process failed.");
    }

    result.process_result_ = std::move(process_result);

    return result;
  }

  TaskResult TaskResult::cancelled(
      cgride::graph::TaskId task_id,
      cgride::core::Error error)
  {
    TaskResult result(std::move(task_id));

    result.status_ = TaskStatus::Cancelled;
    result.error_ = std::move(error);

    return result;
  }

  TaskResult &TaskResult::task_id(cgride::graph::TaskId task_id)
  {
    task_id_ = std::move(task_id);
    return *this;
  }

  TaskResult &TaskResult::task_name(std::string name)
  {
    task_name_ = std::move(name);
    return *this;
  }

  TaskResult &TaskResult::status(TaskStatus status) noexcept
  {
    status_ = status;
    return *this;
  }

  TaskResult &TaskResult::message(std::string message)
  {
    message_ = std::move(message);
    return *this;
  }

  TaskResult &TaskResult::process_result(ProcessResult result)
  {
    process_result_ = std::move(result);
    return *this;
  }

  TaskResult &TaskResult::error(cgride::core::Error error)
  {
    error_ = std::move(error);
    return *this;
  }

  TaskResult &TaskResult::duration(std::chrono::milliseconds duration) noexcept
  {
    duration_ = duration;
    return *this;
  }

  const cgride::graph::TaskId &TaskResult::task_id() const noexcept
  {
    return task_id_;
  }

  const std::string &TaskResult::task_name() const noexcept
  {
    return task_name_;
  }

  TaskStatus TaskResult::status() const noexcept
  {
    return status_;
  }

  const std::string &TaskResult::message() const noexcept
  {
    return message_;
  }

  const std::optional<ProcessResult> &TaskResult::process_result() const noexcept
  {
    return process_result_;
  }

  bool TaskResult::has_process_result() const noexcept
  {
    return process_result_.has_value();
  }

  const std::optional<cgride::core::Error> &TaskResult::error() const noexcept
  {
    return error_;
  }

  bool TaskResult::has_error() const noexcept
  {
    return error_.has_value();
  }

  std::chrono::milliseconds TaskResult::duration() const noexcept
  {
    return duration_;
  }

  bool TaskResult::valid() const noexcept
  {
    return task_id_.valid();
  }

  bool TaskResult::success() const noexcept
  {
    return status_ == TaskStatus::Succeeded ||
           status_ == TaskStatus::Skipped;
  }

  bool TaskResult::failed() const noexcept
  {
    return status_ == TaskStatus::Failed ||
           status_ == TaskStatus::Cancelled;
  }

  bool TaskResult::finished() const noexcept
  {
    return status_ == TaskStatus::Skipped ||
           status_ == TaskStatus::Succeeded ||
           status_ == TaskStatus::Failed ||
           status_ == TaskStatus::Cancelled;
  }

} // namespace cgride::executor
