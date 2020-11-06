# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
import numpy as np
import tvm
import tvm.testing
from tvm import topi
import tvm.topi.testing


@tvm.testing.parametrize_targets
def test_scatter_nd(ctx, target):
    def check_scatter_nd(data, indices, shape, out):
        implementations = {
            "generic": (lambda x, y: topi.scatter_nd(x, y, shape), topi.generic.schedule_extern),
            "cuda": (lambda x, y: topi.cuda.scatter_nd(x, y, shape), topi.generic.schedule_extern),
        }
        fcompute, fschedule = tvm.topi.testing.dispatch(target, implementations)
        tvm.topi.testing.compare_numpy_tvm([data, indices], out, target, ctx, fcompute, fschedule)

    data = np.array([2, 3, 0])
    indices = np.array([[1, 1, 0], [0, 1, 0]])
    shape = (2, 2)
    out = np.array([[0, 0], [2, 3]])
    check_scatter_nd(data, indices, shape, out)

    data = np.array([[[1, 2], [3, 4]], [[5, 6], [7, 8]]])
    indices = np.array([[0, 1], [1, 1]])
    shape = (2, 2, 2, 2)
    out = np.array([[[[0, 0], [1, 2]], [[0, 0], [3, 4]]], [[[0, 0], [0, 0]], [[0, 0], [0, 0]]]])
    check_scatter_nd(data, indices, shape, out)


if __name__ == "__main__":
    test_scatter_nd(tvm.context("cpu"), tvm.target.Target("llvm"))
