import tvm
from tvm import runtime
from tvm import relay, autotvm
from tvm.relay.backend import vm
from tvm.contrib import utils
from tvm import rpc

import numpy as np
import onnx

def test_vm_rpc():
    """
    This test checks to make sure you can export a VMExecutable,
    upload it to a remote machine using RPC and then execute it
    on the other machine.
    """

    # Run on Windows machine over RPC (you need to be on kraken)
    run_on_v2000 = True

    mlperf_dict = {
        "mlperf_resnet50_onnx": {
            "path": "resnet50_v1.onnx",
            "layout": "NCHW",
            "shape_dict": {"input_tensor:0": (1, 3, 224, 224)},
            "dtype_dict": {"input_tensor:0": "float32"}
        },
        "mlperf_ssd_resnet34_onnx": {
            "path": "resnet34-ssd1200.onnx",
            "layout": "NCHW",
            "shape_dict": {"image": (1, 3, 1200, 1200)},
            "dtype_dict": {"image": "float32"}
        },
    }

    # wget https://zenodo.org/record/2592612/files/resnet50_v1.onnx
    # model = "mlperf_resnet50_onnx"
    # wget https://zenodo.org/record/3228411/files/resnet34-ssd1200.onnx
    model = "mlperf_ssd_resnet34_onnx"

    # wget https://zenodo.org/record/2592612/files/resnet50_v1.onnx
    if run_on_v2000:
        target = tvm.target.Target("llvm")
    else:
        target = tvm.target.Target("llvm")
    model_path = mlperf_dict[model]["path"]
    shape_dict = mlperf_dict[model]["shape_dict"]
    dtype_dict = mlperf_dict[model]["dtype_dict"]

    # Generate input
    np.random.seed(0)
    input_list = []
    for iname, ishape in shape_dict.items():
        np_data = np.random.uniform(size=ishape).astype(dtype_dict[iname])
        input_list.append(tvm.nd.array(np_data))
    print(input_list)

    # Import model
    onnx_model = onnx.load(model_path)
    mod, params = relay.frontend.from_onnx(onnx_model, shape_dict, freeze_params=True)
    mod = relay.transform.DynamicToStatic()(mod)

    # Compile to VMExecutable.
    vm_exec = vm.compile(mod, target=target, params=params)

    # Local execution
    # ctx = tvm.cpu()
    # vm_exec_1 = vm.compile(mod, target=tvm.target.Target("llvm"), params=params)
    # vm_factory1 = runtime.vm.VirtualMachine(vm_exec_1, ctx)
    # out1 = vm_factory1.invoke("main", *input_list)
    # print("Local execution: success!")

    # Remote execution
    if run_on_v2000:
        remote = autotvm.measure.request_remote("v2000_tristan", "localhost", 9190, timeout=1000)
    else:
        remote = rpc.LocalSession()
    ctx = remote.cpu()

    # create inputs with remote device
    input_list = []
    for iname, ishape in shape_dict.items():
        np_data = np.random.uniform(size=ishape).astype(dtype_dict[iname])
        input_list.append(tvm.nd.array(np_data, device=ctx))

    # Export to Disk
    temp = utils.tempdir()
    path = temp.relpath("vm_library.tar")
    vm_exec.mod.export_library(path)
    # Upload the serialized Executable.
    remote.upload(path)
    # Get a handle to remote Executable.
    rexec = remote.load_module("vm_library.tar")
    # Build a VM out of the executable and context.
    vm_factory2 = runtime.vm.VirtualMachine(rexec, ctx)
    # Invoke its "main" function.
    out2 = vm_factory2.invoke("main", *input_list)
    print("Remote execution: success!")

    assert(np.allclose(out2[1].asnumpy(), out1[1].asnumpy()))
    print("Outputs match!")

test_vm_rpc()
