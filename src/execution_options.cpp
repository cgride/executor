/**
 *
 *  @file execution_options.cpp
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
#include <cgride/executor/execution_options.hpp>

#include <utility>

namespace cgride::executor
{
  ExecutionOptions &ExecutionOptions::jobs(std::size_t value) noexcept
  {
    if (value == 0)
    {
      jobs_ = 1;
      return *this;
    }

    jobs_ = value;
    return *this;
  }

  ExecutionOptions &ExecutionOptions::stop_on_failure(bool value) noexcept
  {
    stop_on_failure_ = value;
    return *this;
  }

  ExecutionOptions &ExecutionOptions::dry_run(bool value) noexcept
  {
    dry_run_ = value;
    return *this;
  }

  ExecutionOptions &ExecutionOptions::capture_output(bool value) noexcept
  {
    capture_output_ = value;
    return *this;
  }

  ExecutionOptions &ExecutionOptions::inherit_environment(bool value) noexcept
  {
    inherit_environment_ = value;
    return *this;
  }

  ExecutionOptions &ExecutionOptions::timeout(std::chrono::milliseconds value) noexcept
  {
    timeout_ = value;
    return *this;
  }

  ExecutionOptions &ExecutionOptions::clear_timeout() noexcept
  {
    timeout_.reset();
    return *this;
  }

  ExecutionOptions &ExecutionOptions::cancellation_token(
      cgride::core::CancellationToken token) noexcept
  {
    cancellation_token_ = std::move(token);
    return *this;
  }

  ExecutionOptions &ExecutionOptions::on_event(EventHandler handler)
  {
    event_handler_ = std::move(handler);
    return *this;
  }

  std::size_t ExecutionOptions::jobs() const noexcept
  {
    return jobs_;
  }

  bool ExecutionOptions::stop_on_failure() const noexcept
  {
    return stop_on_failure_;
  }

  bool ExecutionOptions::dry_run() const noexcept
  {
    return dry_run_;
  }

  bool ExecutionOptions::capture_output() const noexcept
  {
    return capture_output_;
  }

  bool ExecutionOptions::inherit_environment() const noexcept
  {
    return inherit_environment_;
  }

  const std::optional<std::chrono::milliseconds> &ExecutionOptions::timeout() const noexcept
  {
    return timeout_;
  }

  bool ExecutionOptions::has_timeout() const noexcept
  {
    return timeout_.has_value();
  }

  const cgride::core::CancellationToken &ExecutionOptions::cancellation_token() const noexcept
  {
    return cancellation_token_;
  }

  bool ExecutionOptions::cancelled() const noexcept
  {
    return cancellation_token_.cancelled();
  }

  bool ExecutionOptions::active() const noexcept
  {
    return cancellation_token_.active();
  }

  bool ExecutionOptions::has_event_handler() const noexcept
  {
    return static_cast<bool>(event_handler_);
  }

  void ExecutionOptions::emit_event(const cgride::core::Event &event) const
  {
    if (event_handler_)
    {
      event_handler_(event);
    }
  }

} // namespace cgride::executor
