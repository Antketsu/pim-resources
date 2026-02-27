from math import log
from typing import (
    Optional,
    Union,
)

from m5.objects import (
    PIMCtrl,
    PIMInterface,
)

from gem5.utils.override import overrides
from gem5.components.memory.abstract_memory_system import AbstractMemorySystem
from gem5.components.memory.memory import (
    ChanneledMemory,
    _try_convert,
)

class PIM(ChanneledMemory):
    """
    This class extends ChanneledMemory and can be used to create HBM based
    memory system where a single physical channel contains two pseudo channels.
    This is supposed to be used with the HBMCtrl and two dram (HBM2) interfaces
    per channel.
    """

    def __init__(
        self,
        num_channels: Union[int, str],
        interleaving_size: Union[int, str],
        size: Optional[str] = None,
        addr_mapping: Optional[str] = None,
    ) -> None:
        """
        :param dram_interface_class: The DRAM interface type to create with
                                     this memory controller.
        :param num_channels: The number of channels that needs to be
                             simulated.
        :param size: Optionally specify the size of the DRAM controller's
                     address space. By default, it starts at 0 and ends at
                     the size of the DRAM device specified.
        :param addr_mapping: Defines the address mapping scheme to be used.
                             If ``None``, it is defaulted to ``addr_mapping``
                             from ``dram_interface_class``.
        :param interleaving_size: Defines the interleaving size of the multi-
                                  channel memory system. By default, it is
                                  equivalent to the atom size, i.e., 64.
        """
        super().__init__(
            PIMInterface,
            num_channels,
            interleaving_size,
            size,
            addr_mapping,
        )

        _num_channels = _try_convert(num_channels, int)

    @overrides(ChanneledMemory)
    def _create_mem_interfaces_controller(self):
        self._dram = [
            self._dram_class(addr_mapping=self._addr_mapping)
            for _ in range(self._num_channels)
        ]
        self.mem_ctrl = [
            PIMCtrl(
                dram=self._dram[i],
            )
            for i in range(self._num_channels)
        ]


def PIMAccelerator(
    size: Optional[str] = "4GiB",
) -> AbstractMemorySystem:
    return PIM(1, 64, size=size)
