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
#include <backtrace.h>
#include <cxxabi.h>
#include <tvm/support/logging.h>

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

struct frame {};

struct backtrace_info {
  std::vector<std::string> lines;
  size_t max_size;
  std::string error_message;
};

static backtrace_state* _backtrace_state = nullptr;

std::string demangle_name(const char* name) {
  int status = 0;
  size_t length = std::string::npos;
  std::unique_ptr<char, void (*)(void* __ptr)> demangled_name = {
      abi::__cxa_demangle(name, 0, &length, &status), &std::free};
  if (demangled_name && status == 0 && length > 0) {
    return demangled_name.get();
  } else {
    return name;
  }
}

void tvm_backtrace_error(void* data, const char* msg, int errnum) {
  auto stack_trace = reinterpret_cast<backtrace_info*>(data);
  stack_trace->error_message = msg;
}

void tvm_backtrace_syminfo_callback(void* data, uintptr_t pc, const char* symname, uintptr_t symval,
                                    uintptr_t symsize) {
  auto str = reinterpret_cast<std::string*>(data);

  if (symname != nullptr) {
    *str = demangle_name(symname);
  } else {
    std::ostringstream s;
    s << "0x" << std::setfill('0') << std::setw(sizeof(uintptr_t) * 2) << std::hex << pc;
    *str = s.str();
  }
}
int tvm_backtrace_full_callback(void* data, uintptr_t pc, const char* filename, int lineno,
                                const char* symbol) {
  auto stack_trace = reinterpret_cast<backtrace_info*>(data);
  std::stringstream s;

  std::string symbol_str = "<unknown>";
  if (symbol != nullptr) {
    symbol_str = demangle_name(symbol);
  } else {
    // see if syminfo gives anything
    backtrace_syminfo(_backtrace_state, pc, tvm_backtrace_syminfo_callback, tvm_backtrace_error,
                      &symbol_str);
  }
  s << symbol_str;

  if (filename != nullptr) {
    s << std::endl << "        at " << filename;
    if (lineno != 0) {
      s << ":" << lineno;
    }
  }
  // Skip tvm::backtrace and tvm::LogFatal::~LogFatal at the beginning of the trace as they don't
  // add anything useful to the backtrace.
  if (!(stack_trace->lines.size() == 0 &&
        (symbol_str.find("tvm::backtrace", 0) == 0 || symbol_str.find("tvm::LogFatal", 0) == 0))) {
    stack_trace->lines.push_back(s.str());
  }
  // TVMFuncCall denotes the API boundary so we stop there. Exceptions should be caught there.
  if (symbol_str == "TVMFuncCall" || stack_trace->lines.size() >= stack_trace->max_size) {
    return 1;
  }
  return 0;
}

std::string tvm::backtrace() {
  backtrace_info bt;
  bt.max_size = 100;
  if (_backtrace_state == nullptr) {
    _backtrace_state = backtrace_create_state(NULL, 1, tvm_backtrace_error, &bt);
  }
  backtrace_full(_backtrace_state, 0, tvm_backtrace_full_callback, tvm_backtrace_error, &bt);

  std::ostringstream s;
  s << "Stack trace:\n";
  for (size_t i = 0; i < bt.lines.size(); i++) {
    s << "  " << i << ": " << bt.lines[i] << "\n";
  }

  return s.str();
}
