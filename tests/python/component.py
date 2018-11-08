import unittest

import brica
from brica import connect, Component

from helpers import *


class TestComponent(unittest.TestCase):
    def test_emit_pipe_null(self):
        emit = Component("emit", f_emit)
        pipe = Component("pipe", f_pipe)
        null = Component("null", f_null)

        emit.make_out_port(key)
        pipe.make_in_port(key)
        pipe.make_out_port(key)
        null.make_in_port(key)

        connect(emit, key, pipe, key)
        connect(pipe, key, null, key)

        def collect():
            emit.collect()
            pipe.collect()
            null.collect()

        def execute():
            emit.execute()
            pipe.execute()
            null.execute()

        def expose():
            emit.expose()
            pipe.expose()
            null.expose()

        self.assertTrue(emit.get_output(key) is None)
        self.assertTrue(pipe.get_input(key) is None)
        self.assertTrue(pipe.get_output(key) is None)
        self.assertTrue(null.get_input(key) is None)

        collect()
        execute()
        expose()

        self.assertTrue((emit.get_output(key) == value).all())
        self.assertTrue(pipe.get_input(key) is None)
        self.assertTrue(pipe.get_output(key) is None)
        self.assertTrue(null.get_input(key) is None)

        collect()
        execute()
        expose()

        self.assertTrue((emit.get_output(key) == value).all())
        self.assertTrue((pipe.get_input(key) == value).all())
        self.assertTrue((pipe.get_output(key) == value).all())
        self.assertTrue(null.get_input(key) is None)

        collect()
        execute()
        expose()

        self.assertTrue((emit.get_output(key) == value).all())
        self.assertTrue((pipe.get_input(key) == value).all())
        self.assertTrue((pipe.get_output(key) == value).all())
        self.assertTrue((null.get_input(key) == value).all())


if __name__ == '__main__':
    unittest.main()
