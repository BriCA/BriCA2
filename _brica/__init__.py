from pfifo import Reader, Writer

from multiprocessing import Process
from multiprocessing.reduction import ForkingPickler


def recv_dict(r):
    d = {}
    key = r.read()

    while len(key):
        val = r.read()
        if len(val):
            d[key.decode()] = ForkingPickler.loads(val)
        else:
            d[key.decode()] = None
        key = r.read()

    return d


def send_dict(w, d):
    for key in d:
        val = bytes(ForkingPickler.dumps(d[key]))
        w.write(key)
        w.write(val)
    w.write('')


def run(name, f):
    w = Writer(name + 'p', 0o644)
    r = Reader(name + 'c', 0o644)

    while len(r.read()):
        inputs = recv_dict(r)
        outputs = f(inputs)
        send_dict(w, outputs)


class Dispatcher(object):
    def __init__(self, name, f):
        self.p = Process(target=run, args=(name, f))
        self.p.start()

    def __del__(self):
        self.p.join()
