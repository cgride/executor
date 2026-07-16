/**
 *
 *  @file executor_test.cpp
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
#include <string>
#include <vector>

#include <cgride/core/command.hpp>
#include <cgride/core/event.hpp>
#include <cgride/executor/executor.hpp>
#include <cgride/graph/graph.hpp>

namespace
{
  [[nodiscard]] cgride::core::Command make_echo_command(const std::string &text)
  {
    cgride::core::Command command;

#if defined(_WIN32)
    command
        .program("cmd")
        .arg("/C")
        .arg("echo " + text);
#else
    command
        .program("/bin/sh")
        .arg("-c")
        .arg("printf '" + text + "'");
#endif

    return command;
  }

  [[nodiscard]] cgride::core::Command make_failure_command()
  {
    cgride::core::Command command;

#if defined(_WIN32)
    command
        .program("cmd")
        .arg("/C")
        .arg("exit 7");
#else
    command
        .program("/bin/sh")
        .arg("-c")
        .arg("exit 7");
#endif

    return command;
  }

  [[nodiscard]] bool contains_status(
      const cgride::executor::TaskResults &results,
      const std::string &task_id,
      cgride::executor::TaskStatus status)
  {
    for (const auto &result : results)
    {
      if (result.task_id().value() == task_id &&
          result.status() == status)
      {
        return true;
      }
    }

    return false;
  }

  [[nodiscard]] const cgride::executor::TaskResult *find_result(
      const cgride::executor::TaskResults &results,
      const std::string &task_id)
  {
    for (const auto &result : results)
    {
      if (result.task_id().value() == task_id)
      {
        return &result;
      }
    }

    return nullptr;
  }

} // namespace

int main()
{
  {
    cgride::graph::Graph graph;

    auto result = cgride::executor::execute_graph(graph);

    assert(!result);
    assert(result.error().code() == cgride::core::ErrorCode::InvalidArgument);
    assert(result.error().message() == "Graph has no tasks.");
  }

  {
    cgride::graph::Graph graph("skip-only");

    graph.task(
        cgride::graph::TaskId::from_string("prepare"),
        cgride::graph::TaskKind::Prepare,
        "Prepare build directory");

    auto result = cgride::executor::execute_graph(graph);

    assert(result);

    const auto &results = result.value();

    assert(results.size() == 1);
    assert(results[0].task_id() == cgride::graph::TaskId::from_string("prepare"));
    assert(results[0].task_name() == "Prepare build directory");
    assert(results[0].status() == cgride::executor::TaskStatus::Skipped);
    assert(results[0].message() == "Task has no command.");
    assert(results[0].success());
    assert(results[0].finished());
  }

  {
    cgride::graph::Graph graph("dry-run");

    auto &compile = graph.task(
        cgride::graph::TaskId::from_string("compile"),
        cgride::graph::TaskKind::Compile);

    compile.command(
        cgride::core::Command{}
            .program("cgride-fake-compiler")
            .arg("-c")
            .arg("src/main.cpp")
            .arg("-o")
            .arg("build/main.o"));

    cgride::executor::ExecutionOptions options;
    options.dry_run(true);

    auto result = cgride::executor::execute_graph(graph, options);

#if defined(_WIN32)
    assert(result);

    const auto &results = result.value();

    assert(results.size() == 1);
    assert(results[0].status() == cgride::executor::TaskStatus::Failed);
    assert(results[0].has_error());
#else
    assert(result);

    const auto &results = result.value();

    assert(results.size() == 1);
    assert(results[0].task_id() == cgride::graph::TaskId::from_string("compile"));
    assert(results[0].status() == cgride::executor::TaskStatus::Succeeded);
    assert(results[0].success());
    assert(results[0].has_process_result());
    assert(results[0].process_result().value().standard_output() ==
           "cgride-fake-compiler -c src/main.cpp -o build/main.o");
#endif
  }

#if !defined(_WIN32)
  {
    cgride::graph::Graph graph("success");

    auto &prepare = graph.task(
        cgride::graph::TaskId::from_string("prepare"),
        cgride::graph::TaskKind::Prepare);

    auto &compile = graph.task(
        cgride::graph::TaskId::from_string("compile"),
        cgride::graph::TaskKind::Compile);

    auto &link = graph.task(
        cgride::graph::TaskId::from_string("link"),
        cgride::graph::TaskKind::Link);

    compile.depends_on(prepare.id());
    link.depends_on(compile.id());

    compile.command(make_echo_command("compile-ok"));
    link.command(make_echo_command("link-ok"));

    auto result = cgride::executor::execute_graph(graph);

    assert(result);

    const auto &results = result.value();

    assert(results.size() == 3);

    assert(results[0].task_id() == prepare.id());
    assert(results[0].status() == cgride::executor::TaskStatus::Skipped);

    assert(results[1].task_id() == compile.id());
    assert(results[1].status() == cgride::executor::TaskStatus::Succeeded);
    assert(results[1].has_process_result());
    assert(results[1].process_result().value().standard_output() == "compile-ok");

    assert(results[2].task_id() == link.id());
    assert(results[2].status() == cgride::executor::TaskStatus::Succeeded);
    assert(results[2].has_process_result());
    assert(results[2].process_result().value().standard_output() == "link-ok");
  }

  {
    cgride::graph::Graph graph("failure-stop");

    auto &compile = graph.task(
        cgride::graph::TaskId::from_string("compile"),
        cgride::graph::TaskKind::Compile);

    auto &link = graph.task(
        cgride::graph::TaskId::from_string("link"),
        cgride::graph::TaskKind::Link);

    link.depends_on(compile.id());

    compile.command(make_failure_command());
    link.command(make_echo_command("should-not-run"));

    cgride::executor::ExecutionOptions options;
    options.stop_on_failure(true);

    auto result = cgride::executor::execute_graph(graph, options);

    assert(result);

    const auto &results = result.value();

    assert(results.size() == 1);
    assert(results[0].task_id() == compile.id());
    assert(results[0].status() == cgride::executor::TaskStatus::Failed);
    assert(results[0].has_process_result());
    assert(results[0].process_result().value().has_exit_code());
    assert(results[0].process_result().value().exit_code().value() == 7);
    assert(results[0].failed());
  }

  {
    cgride::graph::Graph graph("failure-continue");

    auto &compile = graph.task(
        cgride::graph::TaskId::from_string("compile"),
        cgride::graph::TaskKind::Compile);

    auto &link = graph.task(
        cgride::graph::TaskId::from_string("link"),
        cgride::graph::TaskKind::Link);

    auto &generate = graph.task(
        cgride::graph::TaskId::from_string("generate"),
        cgride::graph::TaskKind::Generate);

    link.depends_on(compile.id());

    compile.command(make_failure_command());
    link.command(make_echo_command("should-not-run"));
    generate.command(make_echo_command("generate-ok"));

    cgride::executor::ExecutionOptions options;
    options.stop_on_failure(false);

    auto result = cgride::executor::execute_graph(graph, options);

    assert(result);

    const auto &results = result.value();

    assert(results.size() == 3);
    assert(contains_status(results, "compile", cgride::executor::TaskStatus::Failed));
    assert(contains_status(results, "link", cgride::executor::TaskStatus::Failed));
    assert(contains_status(results, "generate", cgride::executor::TaskStatus::Succeeded));

    const auto *link_result = find_result(results, "link");

    assert(link_result != nullptr);
    assert(link_result->has_error());
    assert(link_result->error().value().code() == cgride::core::ErrorCode::InvalidState);
    assert(link_result->error().value().message() == "Task dependency failed.");
    assert(link_result->error().value().detail().has_value());
    assert(link_result->error().value().detail().value() == "compile");
  }

  {
    cgride::graph::Graph graph("events");

    auto &task = graph.task(
        cgride::graph::TaskId::from_string("compile"),
        cgride::graph::TaskKind::Compile);

    task.command(make_echo_command("event-output"));

    std::vector<cgride::core::EventKind> kinds;
    std::string output;

    cgride::executor::ExecutionOptions options;

    options.on_event(
        [&](const cgride::core::Event &event)
        {
          kinds.push_back(event.kind());

          if (event.kind() == cgride::core::EventKind::ProcessOutput)
          {
            output += event.message();
          }
        });

    auto result = cgride::executor::execute_graph(graph, options);

    assert(result);
    assert(result.value().size() == 1);
    assert(result.value()[0].success());
    assert(!kinds.empty());
    assert(output == "event-output");
  }

  {
    cgride::core::CancellationSource source;

    cgride::graph::Graph graph("cancelled");

    auto &task = graph.task(
        cgride::graph::TaskId::from_string("compile"),
        cgride::graph::TaskKind::Compile);

    task.command(make_echo_command("should-not-run"));

    cgride::executor::ExecutionOptions options;
    options.cancellation_token(source.token());

    source.cancel();

    auto result = cgride::executor::execute_graph(graph, options);

    assert(result);

    const auto &results = result.value();

    assert(results.size() == 1);
    assert(results[0].task_id() == task.id());
    assert(results[0].status() == cgride::executor::TaskStatus::Cancelled);
    assert(results[0].has_error());
    assert(results[0].error().value().code() == cgride::core::ErrorCode::Cancelled);
  }
#endif

  return 0;
}
