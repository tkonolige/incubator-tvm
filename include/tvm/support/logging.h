/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/*!
 * \file tvm/support/logging.h
 * \brief logging utilities on top of dmlc-core
 */
#ifndef TVM_SUPPORT_LOGGING_H_
#define TVM_SUPPORT_LOGGING_H_

#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "./dmlc.h" // for dmlc::DebugLoggingEnabled and dmlc::LogMessageVoidify

// a technique that enables overriding macro names on the number of parameters. This is used
// to define other macros below
#define GET_MACRO(_1, _2, _3, _4, _5, NAME, ...) NAME

/*!
 * \brief COND_X calls COND_X_N where N is the number of parameters passed to COND_X
 * X can be any of CHECK_GE, CHECK_EQ, CHECK, or LOG (defined dmlc-core/include/dmlc/logging.h.)
 * COND_X (but not COND_X_N) are supposed to be used outside this file.
 * The first parameter of COND_X (and therefore, COND_X_N), which we call 'quit_on_assert',
 * is a boolean. The rest of the parameters of COND_X is the same as the parameters of X.
 * quit_on_assert determines the overall behaviour of COND_X. If it's true COND_X
 * quits the program on assertion failure. If it's false, then it moves on and somehow reports
 * the assertion failure back to the macro caller in an appropriate manner (e.g, 'return false'
 * in a function, or 'continue' or 'break' in a loop)
 * The default behavior when quit_on_assertion is false, is to 'return false'. If this is not
 * desirable, the macro caller can pass one more last parameter to COND_X to tell COND_X what
 * to do when when quit_on_assertion is false and the assertion fails.
 *
 * Rationale: These macros were designed to implement functions that have two behaviours
 * in a concise way. Those behaviours are quitting on assertion failures, or trying to
 * move on from assertion failures. Note that these macros hide lots of control flow in them,
 * and therefore, makes the logic of the whole code slightly harder to understand. However,
 * in pieces of code that use these macros frequently, it will significantly shorten the
 * amount of code needed to be read, and we won't need to clutter the main logic of the
 * function by repetitive control flow structure. The first problem
 * mentioned will be improved over time as the developer gets used to the macro.
 *
 * Here is an example of how to use it
 * \code
 * bool f(..., bool quit_on_assertion) {
 *   int a = 0, b = 0;
 *   ...
 *   a = ...
 *   b = ...
 *   // if quit_on_assertion is true, if a==b, continue, otherwise quit.
 *   // if quit_on_assertion is false, if a==b, continue, otherwise 'return false' (default
 * behaviour) COND_CHECK_EQ(quit_on_assertion, a, b) << "some error message when  quiting"
 *   ...
 *   for (int i = 0; i < N; i++) {
 *     a = ...
 *     b = ...
 *     // if quit_on_assertion is true, if a==b, continue, otherwise quit.
 *     // if quit_on_assertion is false, if a==b, continue, otherwise 'break' (non-default
 *     // behaviour, therefore, has to be explicitly specified)
 *     COND_CHECK_EQ(quit_on_assertion, a, b, break) << "some error message when  quiting"
 *   }
 * }
 * \endcode
 */
#define COND_CHECK_GE(...) \
  GET_MACRO(__VA_ARGS__, COND_CHECK_GE_5, COND_CHECK_GE_4, COND_CHECK_GE_3)(__VA_ARGS__)
#define COND_CHECK_EQ(...) \
  GET_MACRO(__VA_ARGS__, COND_CHECK_EQ_5, COND_CHECK_EQ_4, COND_CHECK_EQ_3)(__VA_ARGS__)
#define COND_CHECK(...) \
  GET_MACRO(__VA_ARGS__, COND_CHECK_5, COND_CHECK_4, COND_CHECK_3, COND_CHECK_2)(__VA_ARGS__)
#define COND_LOG(...) \
  GET_MACRO(__VA_ARGS__, COND_LOG_5, COND_LOG_4, COND_LOG_3, COND_LOG_2)(__VA_ARGS__)

// Not supposed to be used by users directly.
#define COND_CHECK_OP(quit_on_assert, x, y, what, op) \
  if (!quit_on_assert) {                              \
    if (!((x)op(y))) what;                            \
  } else /* NOLINT(*) */                              \
    CHECK_##op(x, y)

#define COND_CHECK_EQ_4(quit_on_assert, x, y, what) COND_CHECK_OP(quit_on_assert, x, y, what, ==)
#define COND_CHECK_GE_4(quit_on_assert, x, y, what) COND_CHECK_OP(quit_on_assert, x, y, what, >=)

#define COND_CHECK_3(quit_on_assert, x, what) \
  if (!quit_on_assert) {                      \
    if (!(x)) what;                           \
  } else /* NOLINT(*) */                      \
    CHECK(x)

#define COND_LOG_3(quit_on_assert, x, what) \
  if (!quit_on_assert) {                    \
    what;                                   \
  } else /* NOLINT(*) */                    \
    LOG(x)

#define COND_CHECK_EQ_3(quit_on_assert, x, y) COND_CHECK_EQ_4(quit_on_assert, x, y, return false)
#define COND_CHECK_GE_3(quit_on_assert, x, y) COND_CHECK_GE_4(quit_on_assert, x, y, return false)
#define COND_CHECK_2(quit_on_assert, x) COND_CHECK_3(quit_on_assert, x, return false)
#define COND_LOG_2(quit_on_assert, x) COND_LOG_3(quit_on_assert, x, return false)

namespace tvm {

std::string backtrace();

struct Error : public std::runtime_error {
  explicit Error(const std::string &s) : std::runtime_error(s) {}
};

class InternalError : public std::exception {
 public:
  InternalError(const std::string& file, int lineno, const std::string& message,
        const std::time_t& time = std::time(nullptr), std::string stacktrace = backtrace())
      : file_(file), lineno_(lineno), message_(message), time_(time), stacktrace_(stacktrace) {
    std::ostringstream s;
    s << "[" << std::put_time(std::localtime(&time), "%H:%M:%S") << "] " << file << ":" << lineno
      << ": " << std::endl
      << stacktrace << std::endl
      << message << std::endl;
    full_message_ = s.str();
  }
  const std::string& file() const { return file_; }
  const std::string& message() const { return message_; }
  const std::string& full_message() const { return full_message_; }
  const std::string& stacktrace() const { return stacktrace_; }
  const std::time_t& time() const { return time_; }
  int lineno() const { return lineno_; }
  virtual const char* what() const noexcept { return full_message_.c_str(); }

 private:
  std::string file_;
  int lineno_;
  std::string message_;
  std::time_t time_;
  std::string stacktrace_;
  std::string full_message_;  // holds the full error string
};

class LogFatal {
 public:
  LogFatal(const std::string& file, int lineno) : file_(file), lineno_(lineno) {}
  ~LogFatal() noexcept(false) {
    throw InternalError(file_, lineno_, stream_.str());
  }
  std::ostringstream& stream() { return stream_; }

 private:
  std::ostringstream stream_;
  std::string file_;
  int lineno_;
};

class LogMessage {
 public:
  LogMessage(const std::string& file, int lineno) {
    std::time_t t = std::time(nullptr);
    stream_ << "[" << std::put_time(std::localtime(&t), "%H:%M:%S") << "] " << file << ":" << lineno
            << ": ";
  }
  ~LogMessage() { std::cerr << stream_.str() << std::endl; }
  std::ostringstream& stream() { return stream_; }

 private:
  std::ostringstream stream_;
};

#define LOG(level) LOG_##level
#define LOG_FATAL tvm::LogFatal(__FILE__, __LINE__).stream()
#define LOG_INFO tvm::LogMessage(__FILE__, __LINE__).stream()
#define LOG_ERROR (tvm::LogMessage(__FILE__, __LINE__).stream() << "error: ")
#define LOG_WARNING (tvm::LogMessage(__FILE__, __LINE__).stream() << "warning: ")

#define TVM_CHECK_BINARY_OP(name, op, x, y)                           \
  if (!((x) op (y))) \
  tvm::LogFatal(__FILE__, __LINE__).stream()                       \
      << "Check failed: " << #x " " #op " " #y << ": "

#define CHECK(x)                            \
  if (!(x))                                  \
  tvm::LogFatal(__FILE__, __LINE__).stream() \
      << "Check failed: " #x << " == false: "

#define CHECK_LT(x, y) TVM_CHECK_BINARY_OP(_LT, <, x, y)
#define CHECK_GT(x, y) TVM_CHECK_BINARY_OP(_GT, >, x, y)
#define CHECK_LE(x, y) TVM_CHECK_BINARY_OP(_LE, <=, x, y)
#define CHECK_GE(x, y) TVM_CHECK_BINARY_OP(_GE, >=, x, y)
#define CHECK_EQ(x, y) TVM_CHECK_BINARY_OP(_EQ, ==, x, y)
#define CHECK_NE(x, y) TVM_CHECK_BINARY_OP(_NE, !=, x, y)
#define CHECK_NOTNULL(x)                                                                        \
  ((x) == nullptr ? tvm::LogFatal(__FILE__, __LINE__).stream()                                   \
                        << "Check not null: " #x \
                        << ' ',                                                                  \
   (x) : (x))  // NOLINT(*)

#define LOG_IF(severity, condition) \
  !(condition) ? (void)0 : dmlc::LogMessageVoidify() & LOG(severity)

#if LOG_DEBUG

#define LOG_DFATAL LOG_FATAL
#define DFATAL FATAL
#define DLOG(severity) LOG_IF(severity, ::dmlc::DebugLoggingEnabled())
#define DLOG_IF(severity, condition) LOG_IF(severity, ::dmlc::DebugLoggingEnabled() && (condition))

#else

#define LOG_DFATAL LOG_ERROR
#define DFATAL ERROR
#define DLOG(severity) true ? (void)0 : dmlc::LogMessageVoidify() & LOG(severity)
#define DLOG_IF(severity, condition) \
  (true || !(condition)) ? (void)0 : dmlc::LogMessageVoidify() & LOG(severity)

#endif

#if LOG_DEBUG
#define DCHECK(x) \
  while (false) CHECK(x)
#define DCHECK_LT(x, y) \
  while (false) CHECK((x) < (y))
#define DCHECK_GT(x, y) \
  while (false) CHECK((x) > (y))
#define DCHECK_LE(x, y) \
  while (false) CHECK((x) <= (y))
#define DCHECK_GE(x, y) \
  while (false) CHECK((x) >= (y))
#define DCHECK_EQ(x, y) \
  while (false) CHECK((x) == (y))
#define DCHECK_NE(x, y) \
  while (false) CHECK((x) != (y))
#else
#define DCHECK(x) CHECK(x)
#define DCHECK_LT(x, y) CHECK((x) < (y))
#define DCHECK_GT(x, y) CHECK((x) > (y))
#define DCHECK_LE(x, y) CHECK((x) <= (y))
#define DCHECK_GE(x, y) CHECK((x) >= (y))
#define DCHECK_EQ(x, y) CHECK((x) == (y))
#define DCHECK_NE(x, y) CHECK((x) != (y))
#endif

constexpr const char* kTVM_INTERNAL_ERROR_MESSAGE =
    "---------------------------------------------------------------\n"
    "An internal invariant was violated during the execution of TVM.\n"
    "Please read TVM's error reporting guidelines.\n"
    "More details can be found here: https://discuss.tvm.ai/t/error-reporting/7793.\n"
    "---------------------------------------------------------------\n";

#define ICHECK_INDENT "  "

#define ICHECK_BINARY_OP(name, op, x, y)                           \
  _Pragma("GCC diagnostic push")\
  _Pragma("GCC diagnostic ignored \"-Wsign-compare\"")\
  if (!((x) op (y))) \
  _Pragma("GCC diagnostic pop")\
  tvm::LogFatal(__FILE__, __LINE__).stream()                       \
      << tvm::kTVM_INTERNAL_ERROR_MESSAGE << std::endl             \
      << ICHECK_INDENT << "Check failed: " << #x " " #op " " #y << ": "

#define ICHECK(x)                            \
  if (!(x))                                  \
  tvm::LogFatal(__FILE__, __LINE__).stream() \
      << tvm::kTVM_INTERNAL_ERROR_MESSAGE << ICHECK_INDENT << "Check failed: " #x << " == false: "

#define ICHECK_LT(x, y) ICHECK_BINARY_OP(_LT, <, x, y)
#define ICHECK_GT(x, y) ICHECK_BINARY_OP(_GT, >, x, y)
#define ICHECK_LE(x, y) ICHECK_BINARY_OP(_LE, <=, x, y)
#define ICHECK_GE(x, y) ICHECK_BINARY_OP(_GE, >=, x, y)
#define ICHECK_EQ(x, y) ICHECK_BINARY_OP(_EQ, ==, x, y)
#define ICHECK_NE(x, y) ICHECK_BINARY_OP(_NE, !=, x, y)
#define ICHECK_NOTNULL(x)                                                                        \
  ((x) == nullptr ? tvm::LogFatal(__FILE__, __LINE__).stream()                                   \
                        << tvm::kTVM_INTERNAL_ERROR_MESSAGE << ICHECK_INDENT << "Check not null: " #x \
                        << ' ',                                                                  \
   (x) : (x))  // NOLINT(*)

/*! \brief The diagnostic level, controls the printing of the message. */
enum class DiagnosticLevel : int {
  kBug = 10,
  kError = 20,
  kWarning = 30,
  kNote = 40,
  kHelp = 50,
};

}  // namespace tvm
#endif  // TVM_SUPPORT_LOGGING_H_
