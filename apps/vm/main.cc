#include <tvm/runtime/vm/vm.h>
#include <fstream>

int main(int argc, char* argv[]) {
  if (argc != 4) {
    LOG(INFO) << "Usage: " << argv[0] << " executable module params";
    return 1;
  }

  tvm::runtime::Module mod = tvm::runtime::Module::LoadFromFile(argv[2], "so");
  std::ifstream code(argv[1], std::ios::binary | std::ios::ate);
  auto size = code.tellg();
  LOG(INFO) << size;
  std::string str(size, '\0'); // construct string to stream size
  code.seekg(0);
  code.read(&str[0], size);
  auto exec_mod = tvm::runtime::vm::Executable::Load(str, mod);
  const tvm::runtime::vm::Executable* exec = exec_mod.as<tvm::runtime::vm::Executable>();
  tvm::runtime::vm::VirtualMachine vm;
  vm.LoadExecutable(exec);
  vm.GetFunction("invoke", nullptr)();
}
