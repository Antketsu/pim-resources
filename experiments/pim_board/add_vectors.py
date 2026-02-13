from gem5.components.cachehierarchies.classic.private_l1_shared_l2_cache_hierarchy import (
    PrivateL1SharedL2CacheHierarchy
)
from gem5.components.cachehierarchies.classic.no_cache import NoCache
from gem5.components.memory.single_channel import SingleChannelDDR4_2400
from gem5.components.processors.cpu_types import CPUTypes
from gem5.isas import ISA
from gem5.resources.resource import obtain_resource
from gem5.simulate.simulator import Simulator
from gem5.resources.resource import Resource, DiskImageResource
from gem5.simulate.exit_event import ExitEvent
from pim_board import PIMBoard
from gem5.components.processors.simple_processor import SimpleProcessor
from gem5.resources.resource import BinaryResource  
from pim import PIMAccelerator

# Here we setup a MESI Two Level Cache Hierarchy.

cache_hierarchy = PrivateL1SharedL2CacheHierarchy(
    l1d_size="16kB",
    l1d_assoc=8,
    l1i_size="16kB",
    l1i_assoc=8,
    l2_size="256kB",
    l2_assoc=16,
)

# Here we setup a no-cache hierarchy to better observe the PIM behavior without cache effects.
#cache_hierarchy = NoCache()

# Setup the system memory.
memory = SingleChannelDDR4_2400(size="3GB")

processor = SimpleProcessor(num_cores=1,isa=ISA.X86,cpu_type=CPUTypes.ATOMIC)

pim = PIMAccelerator(size="3GB")

board = PIMBoard(
    clk_freq="1GHz",
    processor=processor,
    memory=memory,
    cache_hierarchy=cache_hierarchy,
    pim=pim,
)


def exit_handler():
    print("Exit event triggered - mapping memory region in the simulated process")
    process = processor.get_cores()[0].core.workload[0]
    # VA, PA, Size, Cacheable
    process.map(0x10000000, 0xC4000000, 0x1000000, False) # PIM region
    process.map(0x20000000, 0xD0000000, 0x1000000, False) # Vector A
    process.map(0x30000000, 0xD1000000, 0x1000000, False) # Vector B
    process.map(0x40000000, 0xD2000000, 0x1000000, False) # Vector C
    print("Mapped memory region at VA 0x10000000 to PA 0xC4000000")
    print("Mapped memory region at VA 0x20000000 to PA 0xD0000000")
    print("Mapped memory region at VA 0x30000000 to PA 0xD1000000")
    print("Mapped memory region at VA 0x40000000 to PA 0xD2000000")
    yield False

#board.set_se_binary_workload(obtain_resource("x86-hello64-static"))
board.set_se_binary_workload(BinaryResource("/homelocal/antoma19_local/u/tfm/pim-resources/binaries/add_vectors/add_vectors"))
simulator = Simulator(
    board=board,
    on_exit_event= {
        ExitEvent.EXIT: exit_handler(),
    }
)
simulator.run()