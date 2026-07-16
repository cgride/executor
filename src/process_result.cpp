/**
 *
 *  @file process_result.cpp
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
#include <cgride/executor/process_result.hpp>

#include <utility>

namespace cgride::executor
{
  std::string_view to_string(ProcessStatus status) noexcept
  {
    switch (status)
    {
    case ProcessStatus::NotStarted:
      return "NotStarted";

    case ProcessStatus::Exited:
      return "Exited";

    case ProcessStatus::FailedToStart:
      return "FailedToStart";

    case ProcessStatus::TimedOut:
      return "TimedOut";

    case ProcessStatus::Cancelled:
      return "Cancelled";
    }

    return "NotStarted";
  }

  ProcessResult ProcessResult::exited(
      int exit_code,
      std::string standard_output,
      std::string standard_error,
      std::chrono::milliseconds duration)
  {
    ProcessResult result;

    result.status_ = ProcessStatus::Exited;
    result.exit_code_ = exit_code;
    result.standard_output_ = std::move(standard_output);
    result.standard_error_ = std::move(standard_error);
    result.duration_ = duration;

    if (exit_code != 0)
    {
      result.error_ = cgride::core::Error(
          cgride::core::ErrorCode::ProcessFailed,
          "Process exited with a non-zero exit code.",
          std::to_string(exit_code));
    }

    return result;
  }

  ProcessResult ProcessResult::failed_to_start(
      cgride::core::Error error,
      std::string standard_error)
  {
    ProcessResult result;

    result.status_ = ProcessStatus::FailedToStart;
    result.standard_error_ = std::move(standard_error);
    result.error_ = std::move(error);

    return result;
  }

  ProcessResult ProcessResult::cancelled(cgride::core::Error error)
  {
    ProcessResult result;

    result.status_ = ProcessStatus::Cancelled;
    result.error_ = std::move(error);

    return result;
  }

  ProcessResult ProcessResult::timed_out(cgride::core::Error error)
  {
    ProcessResult result;

    result.status_ = ProcessStatus::TimedOut;
    result.error_ = std::move(error);

    return result;
  }

  ProcessResult &ProcessResult::status(ProcessStatus status) noexcept
  {
    status_ = status;
    return *this;
  }

  ProcessResult &ProcessResult::exit_code(int exit_code) noexcept
  {
    exit_code_ = exit_code;
    return *this;
  }

  ProcessResult &ProcessResult::standard_output(std::string output)
  {
    standard_output_ = std::move(output);
    return *this;
  }

  ProcessResult &ProcessResult::append_standard_output(std::string_view output)
  {
    standard_output_.append(output);
    return *this;
  }

  ProcessResult &ProcessResult::standard_error(std::string output)
  {
    standard_error_ = std::move(output);
    return *this;
  }

  ProcessResult &ProcessResult::append_standard_error(std::string_view output)
  {
    standard_error_.append(output);
    return *this;
  }

  ProcessResult &ProcessResult::error(cgride::core::Error error)
  {
    error_ = std::move(error);
    return *this;
  }

  ProcessResult &ProcessResult::duration(std::chrono::milliseconds duration) noexcept
  {
    duration_ = duration;
    return *this;
  }

  ProcessStatus ProcessResult::status() const noexcept
  {
    return status_;
  }

  const std::optional<int> &ProcessResult::exit_code() const noexcept
  {
    return exit_code_;
  }

  bool ProcessResult::has_exit_code() const noexcept
  {
    return exit_code_.has_value();
  }

  const std::string &ProcessResult::standard_output() const noexcept
  {
    return standard_output_;
  }

  const std::string &ProcessResult::standard_error() const noexcept
  {
    return standard_error_;
  }

  const std::optional<cgride::core::Error> &ProcessResult::error() const noexcept
  {
    return error_;
  }

  bool ProcessResult::has_error() const noexcept
  {
    return error_.has_value();
  }

  std::chrono::milliseconds ProcessResult::duration() const noexcept
  {
    return duration_;
  }

  bool ProcessResult::success() const noexcept
  {
    return status_ == ProcessStatus::Exited &&
           exit_code_.has_value() &&
           exit_code_.value() == 0;
  }

  bool ProcessResult::failed() const noexcept
  {
    return !success();
  }

  bool ProcessResult::finished() const noexcept
  {
    return status_ == ProcessStatus::Exited ||
           status_ == ProcessStatus::FailedToStart ||
           status_ == ProcessStatus::TimedOut ||
           status_ == ProcessStatus::Cancelled;
  }

} // namespace cgride::executor
