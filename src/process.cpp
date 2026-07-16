/**
 *
 *  @file process.cpp
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
#include <cgride/executor/process.hpp>

#include <algorithm>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <map>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <cgride/core/error.hpp>

#if !defined(_WIN32)
#include <cerrno>
#include <csignal>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;
#endif

namespace cgride::executor
{
  namespace
  {
    using cgride::core::Error;
    using cgride::core::ErrorCode;

    [[nodiscard]] bool needs_quoting(const std::string &value)
    {
      return value.empty() ||
             value.find_first_of(" \t\n\"'\\") != std::string::npos;
    }

    [[nodiscard]] std::string quote_for_display(const std::string &value)
    {
      if (!needs_quoting(value))
      {
        return value;
      }

      std::string quoted;
      quoted.reserve(value.size() + 2);
      quoted.push_back('"');

      for (const auto character : value)
      {
        if (character == '"' || character == '\\')
        {
          quoted.push_back('\\');
        }

        quoted.push_back(character);
      }

      quoted.push_back('"');

      return quoted;
    }

#if !defined(_WIN32)

    struct PipePair
    {
      int read_fd{-1};
      int write_fd{-1};
    };

    void close_fd(int &fd) noexcept
    {
      if (fd >= 0)
      {
        ::close(fd);
        fd = -1;
      }
    }

    [[nodiscard]] bool make_pipe(PipePair &pipe) noexcept
    {
      int fds[2]{-1, -1};

      if (::pipe(fds) != 0)
      {
        return false;
      }

      pipe.read_fd = fds[0];
      pipe.write_fd = fds[1];

      return true;
    }

    void make_non_blocking(int fd) noexcept
    {
      if (fd < 0)
      {
        return;
      }

      const auto flags = ::fcntl(fd, F_GETFL, 0);

      if (flags >= 0)
      {
        ::fcntl(fd, F_SETFL, flags | O_NONBLOCK);
      }
    }

    [[nodiscard]] std::string errno_message()
    {
      return std::strerror(errno);
    }

    [[nodiscard]] std::string environment_value(
        const cgride::core::Command &command,
        const std::string &key)
    {
      for (const auto &entry : command.environment())
      {
        if (entry.first == key)
        {
          return entry.second;
        }
      }

      const auto *value = std::getenv(key.c_str());

      if (value == nullptr)
      {
        return {};
      }

      return value;
    }

    [[nodiscard]] std::vector<std::string> split_path_list(const std::string &value)
    {
      std::vector<std::string> entries;
      std::stringstream stream(value);
      std::string entry;

      while (std::getline(stream, entry, ':'))
      {
        if (!entry.empty())
        {
          entries.push_back(std::move(entry));
        }
      }

      return entries;
    }

    [[nodiscard]] std::filesystem::path resolve_program_path(
        const cgride::core::Command &command)
    {
      const std::filesystem::path program(command.program());

      if (!command.search_in_path() || program.has_parent_path())
      {
        return program;
      }

      const auto path_value = environment_value(command, "PATH");

      for (const auto &directory : split_path_list(path_value))
      {
        auto candidate = std::filesystem::path(directory) / program;

        std::error_code ec;

        if (std::filesystem::exists(candidate, ec) &&
            std::filesystem::is_regular_file(candidate, ec))
        {
          return candidate;
        }
      }

      return program;
    }

    [[nodiscard]] std::vector<std::string> build_environment_storage(
        const cgride::core::Command &command,
        const ExecutionOptions &options)
    {
      std::map<std::string, std::string> values;

      if (command.inherit_environment() && options.inherit_environment())
      {
        for (auto current = environ; current != nullptr && *current != nullptr; ++current)
        {
          std::string entry(*current);
          const auto separator = entry.find('=');

          if (separator == std::string::npos)
          {
            continue;
          }

          values[entry.substr(0, separator)] = entry.substr(separator + 1);
        }
      }

      for (const auto &entry : command.environment())
      {
        if (!entry.first.empty())
        {
          values[entry.first] = entry.second;
        }
      }

      std::vector<std::string> storage;
      storage.reserve(values.size());

      for (const auto &[key, value] : values)
      {
        storage.push_back(key + "=" + value);
      }

      return storage;
    }

    [[nodiscard]] std::vector<char *> make_pointer_list(std::vector<std::string> &storage)
    {
      std::vector<char *> pointers;
      pointers.reserve(storage.size() + 1);

      for (auto &value : storage)
      {
        pointers.push_back(value.data());
      }

      pointers.push_back(nullptr);

      return pointers;
    }

    [[nodiscard]] std::vector<std::string> build_argument_storage(
        const cgride::core::Command &command)
    {
      std::vector<std::string> storage;

      storage.reserve(command.args().size() + 1);
      storage.push_back(command.program());

      for (const auto &argument : command.args())
      {
        storage.push_back(argument);
      }

      return storage;
    }

    void read_available(int fd, std::string &output)
    {
      if (fd < 0)
      {
        return;
      }

      char buffer[4096];

      while (true)
      {
        const auto count = ::read(fd, buffer, sizeof(buffer));

        if (count > 0)
        {
          output.append(buffer, static_cast<std::size_t>(count));
          continue;
        }

        if (count == 0)
        {
          break;
        }

        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
        {
          break;
        }

        break;
      }
    }

    void terminate_child(pid_t child) noexcept
    {
      if (child <= 0)
      {
        return;
      }

      ::kill(child, SIGTERM);

      for (int attempt = 0; attempt < 20; ++attempt)
      {
        int status = 0;
        const auto waited = ::waitpid(child, &status, WNOHANG);

        if (waited == child)
        {
          return;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
      }

      ::kill(child, SIGKILL);

      int status = 0;
      ::waitpid(child, &status, 0);
    }

    [[nodiscard]] int exit_code_from_status(int status) noexcept
    {
      if (WIFEXITED(status))
      {
        return WEXITSTATUS(status);
      }

      if (WIFSIGNALED(status))
      {
        return 128 + WTERMSIG(status);
      }

      return 1;
    }

    [[nodiscard]] cgride::core::Result<ProcessResult> run_process_posix(
        const cgride::core::Command &command,
        const ExecutionOptions &options)
    {
      if (options.cancelled())
      {
        return ProcessResult::cancelled();
      }

      if (options.dry_run())
      {
        return ProcessResult::exited(
            0,
            describe_command(command),
            {},
            std::chrono::milliseconds{0});
      }

      PipePair stdout_pipe;
      PipePair stderr_pipe;

      if (options.capture_output())
      {
        if (!make_pipe(stdout_pipe))
        {
          return ProcessResult::failed_to_start(
              Error(
                  ErrorCode::ProcessFailed,
                  "Failed to create stdout pipe.",
                  errno_message()));
        }

        if (!make_pipe(stderr_pipe))
        {
          close_fd(stdout_pipe.read_fd);
          close_fd(stdout_pipe.write_fd);

          return ProcessResult::failed_to_start(
              Error(
                  ErrorCode::ProcessFailed,
                  "Failed to create stderr pipe.",
                  errno_message()));
        }
      }

      auto argument_storage = build_argument_storage(command);
      auto argument_pointers = make_pointer_list(argument_storage);

      auto environment_storage = build_environment_storage(command, options);
      auto environment_pointers = make_pointer_list(environment_storage);

      const auto program = resolve_program_path(command);

      const auto start = std::chrono::steady_clock::now();

      const auto child = ::fork();

      if (child < 0)
      {
        close_fd(stdout_pipe.read_fd);
        close_fd(stdout_pipe.write_fd);
        close_fd(stderr_pipe.read_fd);
        close_fd(stderr_pipe.write_fd);

        return ProcessResult::failed_to_start(
            Error(
                ErrorCode::ProcessFailed,
                "Failed to start process.",
                errno_message()));
      }

      if (child == 0)
      {
        if (command.cwd().has_value() && !command.cwd()->empty())
        {
          if (::chdir(command.cwd()->string().c_str()) != 0)
          {
            _exit(127);
          }
        }

        if (options.capture_output())
        {
          close_fd(stdout_pipe.read_fd);
          close_fd(stderr_pipe.read_fd);

          if (::dup2(stdout_pipe.write_fd, STDOUT_FILENO) < 0)
          {
            _exit(127);
          }

          if (::dup2(stderr_pipe.write_fd, STDERR_FILENO) < 0)
          {
            _exit(127);
          }

          close_fd(stdout_pipe.write_fd);
          close_fd(stderr_pipe.write_fd);
        }

        ::execve(program.string().c_str(), argument_pointers.data(), environment_pointers.data());

        _exit(127);
      }

      close_fd(stdout_pipe.write_fd);
      close_fd(stderr_pipe.write_fd);

      make_non_blocking(stdout_pipe.read_fd);
      make_non_blocking(stderr_pipe.read_fd);

      std::string standard_output;
      std::string standard_error;

      int status = 0;
      bool child_finished = false;

      while (!child_finished)
      {
        if (options.capture_output())
        {
          read_available(stdout_pipe.read_fd, standard_output);
          read_available(stderr_pipe.read_fd, standard_error);
        }

        const auto waited = ::waitpid(child, &status, WNOHANG);

        if (waited == child)
        {
          child_finished = true;
          break;
        }

        if (waited < 0 && errno != EINTR)
        {
          const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::steady_clock::now() - start);

          close_fd(stdout_pipe.read_fd);
          close_fd(stderr_pipe.read_fd);

          return ProcessResult::failed_to_start(
                     Error(
                         ErrorCode::ProcessFailed,
                         "Failed while waiting for process.",
                         errno_message()),
                     standard_error)
              .standard_output(std::move(standard_output))
              .duration(duration);
        }

        const auto now = std::chrono::steady_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);

        if (options.cancelled())
        {
          terminate_child(child);

          if (options.capture_output())
          {
            read_available(stdout_pipe.read_fd, standard_output);
            read_available(stderr_pipe.read_fd, standard_error);
          }

          close_fd(stdout_pipe.read_fd);
          close_fd(stderr_pipe.read_fd);

          return ProcessResult::cancelled()
              .standard_output(std::move(standard_output))
              .standard_error(std::move(standard_error))
              .duration(duration);
        }

        if (options.has_timeout() && duration > options.timeout().value())
        {
          terminate_child(child);

          if (options.capture_output())
          {
            read_available(stdout_pipe.read_fd, standard_output);
            read_available(stderr_pipe.read_fd, standard_error);
          }

          close_fd(stdout_pipe.read_fd);
          close_fd(stderr_pipe.read_fd);

          return ProcessResult::timed_out()
              .standard_output(std::move(standard_output))
              .standard_error(std::move(standard_error))
              .duration(duration);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(2));
      }

      if (options.capture_output())
      {
        read_available(stdout_pipe.read_fd, standard_output);
        read_available(stderr_pipe.read_fd, standard_error);
      }

      close_fd(stdout_pipe.read_fd);
      close_fd(stderr_pipe.read_fd);

      const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::steady_clock::now() - start);

      return ProcessResult::exited(
          exit_code_from_status(status),
          std::move(standard_output),
          std::move(standard_error),
          duration);
    }

#endif

  } // namespace

  std::string describe_command(const cgride::core::Command &command)
  {
    std::string output = quote_for_display(command.program());

    for (const auto &argument : command.args())
    {
      output.push_back(' ');
      output += quote_for_display(argument);
    }

    return output;
  }

  cgride::core::Result<ProcessResult> run_process(
      const cgride::core::Command &command,
      const ExecutionOptions &options)
  {
    if (!command.valid())
    {
      return Error(
          ErrorCode::InvalidArgument,
          "Cannot run an invalid command.");
    }

#if defined(_WIN32)
    (void)options;

    return ProcessResult::failed_to_start(
        Error(
            ErrorCode::UnsupportedPlatform,
            "Process execution is not implemented for Windows yet."));
#else
    return run_process_posix(command, options);
#endif
  }

  cgride::core::Result<ProcessResult> run_process(
      const cgride::core::Command &command)
  {
    ExecutionOptions options;
    return run_process(command, options);
  }

} // namespace cgride::executor
