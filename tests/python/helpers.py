import numpy as np

key = 'default'
value = np.array([1, 2, 3])


def f_emit(inputs):
    return {key: value}


def f_pipe(inputs):
    return inputs


def f_null(inputs):
    return {}


