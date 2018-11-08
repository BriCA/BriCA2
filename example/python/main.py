import brica
from brica import Component, VirtualTimePhasedScheduler

import time

from helpers import f1, f2

if __name__ == "__main__":
    c1 = Component("c1", f1)
    c2 = Component("c2", f2)

    c1.make_out_port("hoge")
    c2.make_in_port("fuga")

    brica.connect(c1, "hoge", c2, "fuga")

    s = VirtualTimePhasedScheduler()
    s.add_component(c1, 0)
    s.add_component(c2, 0)

    start = time.time()
    s.step()
    s.step()
    end = time.time()
    print(end - start)
