from typing import List

from m5.objects import (
    AddrRange,
    BaseCPU,
    BaseMMU,
    IOXBar,
    Port,
    Process,
)
from gem5.components.boards.abstract_system_board import AbstractSystemBoard
from gem5.components.boards.se_binary_workload import SEBinaryWorkload
from gem5.components.memory import SingleChannelDDR4_2400
from gem5.components.processors.base_cpu_core import BaseCPUCore
from gem5.components.processors.base_cpu_processor import BaseCPUProcessor
from gem5.isas import ISA
from gem5.utils.override import overrides
from gem5.components.processors.abstract_processor import AbstractProcessor
from gem5.components.memory.abstract_memory_system import AbstractMemorySystem
from gem5.components.cachehierarchies.abstract_cache_hierarchy import AbstractCacheHierarchy
from typing import (
    List,
    Optional,
    Sequence,
    Tuple,
)

class CustomBoard(AbstractSystemBoard, SEBinaryWorkload):
    def __init__(
        self,
        clk_freq: str,
        processor: AbstractProcessor,
        memory: AbstractMemorySystem,
        pim: AbstractMemorySystem,
        cache_hierarchy: AbstractCacheHierarchy,
    ) -> None:
        super().__init__(
            clk_freq=clk_freq,
            processor=processor,
            memory=memory,
            cache_hierarchy=cache_hierarchy,
        )
        self.pim = pim
        

    @overrides(AbstractSystemBoard)
    def get_mem_ports(self) -> Sequence[Tuple[AddrRange, Port]]:
        """Get the memory ports exposed on this board

        .. note::

            The ports should be returned such that the address ranges are
            in ascending order.
        """
        return self.get_memory().get_mem_ports() + self.pim.get_mem_ports()
    
    @overrides(AbstractSystemBoard)
    def _setup_board(self) -> None:
        pass

    @overrides(AbstractSystemBoard)
    def has_io_bus(self) -> bool:
        return False

    @overrides(AbstractSystemBoard)
    def get_io_bus(self) -> IOXBar:
        raise NotImplementedError(
            "UniqueBoard does not have an IO Bus. "
            "Use `has_io_bus()` to check this."
        )

    @overrides(AbstractSystemBoard)
    def has_dma_ports(self) -> bool:
        return False

    @overrides(AbstractSystemBoard)
    def get_dma_ports(self) -> List[Port]:
        raise NotImplementedError(
            "UniqueBoard does not have DMA Ports. "
            "Use `has_dma_ports()` to check this."
        )

    @overrides(AbstractSystemBoard)
    def has_coherent_io(self) -> bool:
        return False

    @overrides(AbstractSystemBoard)
    def get_mem_side_coherent_io_port(self) -> Port:
        raise NotImplementedError(
            "UniqueBoard does not have any I/O ports. Use has_coherent_io to "
            "check this."
        )

    @overrides(AbstractSystemBoard)
    def _setup_memory_ranges(self) -> None:
        memory = self.get_memory()
        mem_range = [AddrRange(memory.get_size())]
        memory.set_memory_range(mem_range)
        pim = self.pim
        pim_range = [AddrRange(
            memory.get_size(),
            memory.get_size() + pim.get_size(),
        )]
        pim.set_memory_range(pim_range)
        self.mem_ranges = mem_range + pim_range