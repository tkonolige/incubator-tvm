#include <tvm/runtime/vm/vm.h>
#include <tvm/runtime/vm/memory_manager.h>

#include <iostream>
#include <fstream>

int main(int argc, char** argv) {
  CHECK_EQ(argc, 3) << "Usage: cpp_benchmark (vm|graph) dir";

  tvm::runtime::Module mod = tvm::runtime::Module::LoadFromFile(std::string(argv[2]) + "/module.so", "so");
  std::ifstream code(std::string(argv[2]) + "/code.ro", std::ios::binary | std::ios::ate);
  auto size = code.tellg();
  std::string str(size, '\0');  // construct string to stream size
  code.seekg(0);
  code.read(&str[0], size);
  auto exec_mod = tvm::runtime::vm::Executable::Load(str, mod);
  const tvm::runtime::vm::Executable* exec = exec_mod.as<tvm::runtime::vm::Executable>();
  tvm::runtime::vm::VirtualMachine vm;
  vm.LoadExecutable(exec);
  vm.GetFunction("init", nullptr)(uint64_t(kDLCPU), uint64_t(0), uint64_t(tvm::runtime::vm::AllocatorType::kPooled));

  auto input = tvm::runtime::NDArray::Empty({1, 3, 224, 224}, {kDLFloat, 32, 1}, {kDLCPU, 0});
  vm.GetFunction("set_input", nullptr)("main", input);
  auto invoke = vm.GetFunction("invoke", nullptr);

  for(int i = 0; i < 100; i++) {
    invoke("main");
  }
}
