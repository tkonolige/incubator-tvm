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
"""Registration of profiling objects in python."""

from ... import _ffi
from . import _ffi_api
from .. import Object, Device
from typing import Dict, Sequence


@_ffi.register_object("runtime.profiling.Report")
class Report(Object):
    """A container for information gathered during a profiling run.

    Fields
    ----------
    calls : Array[Dict[str, Object]]
        Per-call profiling metrics (function name, runtime, device, ...).

    device_metrics : Dict[Device, Dict[str, Object]]
        Per-device metrics collected over the entire run.
    """

    def csv(self):
        """Convert this profiling report into CSV format.

        This only includes calls and not overall metrics.

        Returns
        -------
        csv : str
            `calls` in CSV format.
        """
        return _ffi_api.AsCSV(self)


@_ffi.register_object("runtime.profiling.MetricCollector")
class MetricCollector(Object):
    """Interface for user defined profiling metric collection."""

    pass


@_ffi.register_object("runtime.profiling.DeviceWrapper")
class DeviceWrapper(Object):
    """Wraps a tvm.runtime.Device"""

    def __init__(self, dev: Device):
        self.__init_handle_by_constructor__(_ffi_api.DeviceWrapper, dev)


@_ffi.register_object("runtime.profiling.PAPIMetricCollector")
class PAPIMetricCollector(MetricCollector):
    """Collects performance counter information using the Performance
    Application Programming Interface (PAPI).
    """

    def __init__(self, metric_names: Dict[Device, Sequence[str]]):
        """
        Parameters
        ----------
        metric_names : Dict[Device, Sequence[str]]
            List of per-device metrics to collect. You can find a list of valid
            metrics by runing `papi_native_avail` from the command line.
        """
        wrapped = dict()
        for dev, names in metric_names.items():
            wrapped[DeviceWrapper(dev)] = names
        self.__init_handle_by_constructor__(_ffi_api.PAPIMetricCollector, wrapped)
