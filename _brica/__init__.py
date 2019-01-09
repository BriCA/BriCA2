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

    msg = chr(r.read()[0])

    while msg != '0':
        if msg == '1':
            inputs = recv_dict(r)
            outputs = f(inputs)
            send_dict(w, outputs)
        if msg == '2':
            s = bytes(ForkingPickler.dumps(f))
            w.write(s)
        msg = chr(r.read()[0])


class Dispatcher(object):
    def __init__(self, name, f):
        self.p = Process(target=run, args=(name, f))
        self.p.start()

    def __del__(self):
        self.p.join()
