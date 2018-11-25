import unittest

import brica
from brica import connect, Component, Timing, VirtualTimeScheduler


key = 'default'
value = [1, 2, 3]


def f_emit(inputs):
    return {key: value}


def f_pipe(inputs):
    return inputs


def f_null(inputs):
    return {}


class TestVirtualTimeScheduler(unittest.TestCase):
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

        s = VirtualTimeScheduler()
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

        self.assertTrue((emit.get_output(key) == value))
        self.assertTrue(emit.get_out_port_value(key) == None)

        self.assertTrue(pipe.get_input(key) == None)
        self.assertTrue(pipe.get_in_port_value(key) == None)

        self.assertTrue(pipe.get_output(key) == None)
        self.assertTrue(pipe.get_out_port_value(key) == None)

        self.assertTrue(null.get_input(key) == None)
        self.assertTrue(null.get_in_port_value(key) == None)

        s.step()

        self.assertTrue((emit.get_output(key) == value))
        self.assertTrue((emit.get_out_port_value(key) == value))

        self.assertTrue((pipe.get_input(key) == value))
        self.assertTrue((pipe.get_in_port_value(key) == value))

        self.assertTrue((pipe.get_output(key) == value))
        self.assertTrue(pipe.get_out_port_value(key) == None)

        self.assertTrue(null.get_input(key) == None)
        self.assertTrue(null.get_in_port_value(key) == None)

        s.step()

        self.assertTrue((emit.get_output(key) == value))
        self.assertTrue((emit.get_out_port_value(key) == value))

        self.assertTrue((pipe.get_input(key) == value))
        self.assertTrue((pipe.get_in_port_value(key) == value))

        self.assertTrue((pipe.get_output(key) == value))
        self.assertTrue((pipe.get_out_port_value(key) == value))

        self.assertTrue((null.get_input(key) == value))
        self.assertTrue((null.get_in_port_value(key) == value))


if __name__ == '__main__':
    unittest.main()
