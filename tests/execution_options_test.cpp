/**
 *
 *  @file execution_options_test.cpp
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

#include <cgride/core/cancellation.hpp>
#include <cgride/core/event.hpp>
#include <cgride/executor/execution_options.hpp>

int main()
{
  {
    cgride::executor::ExecutionOptions options;

    assert(options.jobs() == 1);
    assert(options.stop_on_failure());
    assert(!options.dry_run());
    assert(options.capture_output());
    assert(options.inherit_environment());
    assert(!options.has_timeout());
    assert(!options.timeout().has_value());
    assert(!options.cancelled());
    assert(options.active());
    assert(!options.has_event_handler());
  }

  {
    cgride::executor::ExecutionOptions options;

    options.jobs(4);

    assert(options.jobs() == 4);

    options.jobs(0);

    assert(options.jobs() == 1);
  }

  {
    cgride::executor::ExecutionOptions options;

    options
        .stop_on_failure(false)
        .dry_run(true)
        .capture_output(false)
        .inherit_environment(false);

    assert(!options.stop_on_failure());
    assert(options.dry_run());
    assert(!options.capture_output());
    assert(!options.inherit_environment());
  }

  {
    cgride::executor::ExecutionOptions options;

    options.timeout(std::chrono::milliseconds{250});

    assert(options.has_timeout());
    assert(options.timeout().has_value());
    assert(options.timeout().value() == std::chrono::milliseconds{250});

    options.clear_timeout();

    assert(!options.has_timeout());
    assert(!options.timeout().has_value());
  }

  {
    cgride::core::CancellationSource source;
    cgride::executor::ExecutionOptions options;

    options.cancellation_token(source.token());

    assert(!options.cancelled());
    assert(options.active());

    source.cancel();

    assert(options.cancelled());
    assert(!options.active());
  }

  {
    cgride::executor::ExecutionOptions options;

    int event_count = 0;
    std::string last_message;

    options.on_event(
        [&](const cgride::core::Event &event)
        {
          ++event_count;
          last_message = event.message();
        });

    assert(options.has_event_handler());

    options.emit_event(
        cgride::core::Event(
            cgride::core::EventKind::BuildStarted,
            "executor started"));

    assert(event_count == 1);
    assert(last_message == "executor started");

    options.emit_event(
        cgride::core::Event(
            cgride::core::EventKind::BuildFinished,
            "executor finished"));

    assert(event_count == 2);
    assert(last_message == "executor finished");
  }

  {
    cgride::executor::ExecutionOptions options;

    assert(!options.has_event_handler());

    options.emit_event(
        cgride::core::Event(
            cgride::core::EventKind::BuildStarted,
            "no handler"));

    assert(!options.has_event_handler());
  }

  return 0;
}
