import sys
import os

from _brica import proxy

def main():
    sys.path.append(os.getcwd())
    proxy(sys.argv[1])
