import unittest

from multiprocessing import Pool

import numpy as np
import brica
from brica import connect, Component, Timing, VirtualTimeScheduler, MultiprocessPool

key = 'default'
value = np.array([1, 2, 3])


def f_emit(inputs):
    print('emit')
    return {key: value}


def f_pipe(inputs):
    print('pipe')
    return inputs


def f_null(inputs):
    print('null')
    return {}


class TestComponent(unittest.TestCase):
    def test_emit_pipe_null_scheduling(self):
        emit = Component(f_emit)
        pipe = Component(f_pipe)
        null = Component(f_null)

        emit.make_out_port(key)
        pipe.make_in_port(key)
        pipe.make_out_port(key)
        null.make_in_port(key)

        connect(emit, key, pipe, key)
        connect(pipe, key, null, key)

        pool = MultiprocessPool(Pool())

        s = VirtualTimeScheduler(pool)
        t = Timing(0, 1, 0)

        s.add_component(emit, t)
        s.add_component(pipe, t)
        s.add_component(null, t)

        self.assertTrue(emit.get_output(key) == None)
        self.assertTrue(emit.get_out_port_value(key) == None)

        self.assertTrue(pipe.get_input(key) == None)
        self.assertTrue(pipe.get_in_port_value(key) == None)

        self.assertTrue(pipe.get_output(key) == None)
        self.assertTrue(pipe.get_out_port_value(key) == None)

        self.assertTrue(null.get_input(key) == None)
        self.assertTrue(null.get_in_port_value(key) == None)

        s.step()

        self.assertTrue((emit.get_output(key) == value).all())
        self.assertTrue(emit.get_out_port_value(key) == None)

        self.assertTrue(pipe.get_input(key) == None)
        self.assertTrue(pipe.get_in_port_value(key) == None)

        self.assertTrue(pipe.get_output(key) == None)
        self.assertTrue(pipe.get_out_port_value(key) == None)

        self.assertTrue(null.get_input(key) == None)
        self.assertTrue(null.get_in_port_value(key) == None)

        s.step()

        self.assertTrue((emit.get_output(key) == value).all())
        self.assertTrue((emit.get_out_port_value(key) == value).all())

        self.assertTrue((pipe.get_input(key) == value).all())
        self.assertTrue((pipe.get_in_port_value(key) == value).all())

        self.assertTrue((pipe.get_output(key) == value).all())
        self.assertTrue(pipe.get_out_port_value(key) == None)

        self.assertTrue(null.get_input(key) == None)
        self.assertTrue(null.get_in_port_value(key) == None)

        s.step()

        self.assertTrue((emit.get_output(key) == value).all())
        self.assertTrue((emit.get_out_port_value(key) == value).all())

        self.assertTrue((pipe.get_input(key) == value).all())
        self.assertTrue((pipe.get_in_port_value(key) == value).all())

        self.assertTrue((pipe.get_output(key) == value).all())
        self.assertTrue((pipe.get_out_port_value(key) == value).all())

        self.assertTrue((null.get_input(key) == value).all())
        self.assertTrue((null.get_in_port_value(key) == value).all())


if __name__ == '__main__':
    unittest.main()
