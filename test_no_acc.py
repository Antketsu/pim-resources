import argparse


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
from gem5.components.processors.simple_processor import SimpleProcessor
from gem5.resources.resource import BinaryResource  
from gem5.components.boards.x86_board import X86Board

# Here we setup a MESI Two Level Cache Hierarchy.


# Here we setup a no-cache hierarchy to better observe the PIM behavior without cache effects.
#cache_hierarchy = NoCache()

cache_hierarchy = PrivateL1SharedL2CacheHierarchy(
    l1d_size="16kB",
    l1d_assoc=8,
    l1i_size="16kB",
    l1i_assoc=8,
    l2_size="256kB",
    l2_assoc=16,
)

# Setup the system memory.
memory = SingleChannelDDR4_2400(size="3GB")

processor = SimpleProcessor(num_cores=1,isa=ISA.X86,cpu_type=CPUTypes.TIMING)

board = X86Board(
    clk_freq="1GHz",
    processor=processor,
    memory=memory,
    cache_hierarchy=cache_hierarchy,
)

kernels_path = "/homelocal/antoma19_local/u/tfm/pim-resources/binaries/add_no_acc"

board.set_se_binary_workload(BinaryResource(kernels_path))
simulator = Simulator(
    board=board,
)
simulator.run()