import ctypes
import os
import sys

if sys.platform == 'win32':
    ext = '.dll'
elif sys.platform == 'darwin':
    ext = '.dylib'
else:
    ext = '.so'

_crc = ctypes.CDLL(os.path.join(os.path.dirname(__file__), 'crc' + ext))

#note: update this definition when the equivalent C code is changed
class params_t(ctypes.Structure):
    _fields_ = [('width', ctypes.c_uint8),
               ('poly', ctypes.c_uint64),
               ('refin', ctypes.c_bool),
               ('refout', ctypes.c_bool),
               ('init', ctypes.c_uint64),
               ('xorout', ctypes.c_uint64),
               ('crc_table', ctypes.c_uint64 * 256),
               ('braid_table', (ctypes.c_uint64 * 256) * 8)]

_crc.crc_params.argtypes = [ctypes.c_uint8, ctypes.c_uint64, ctypes.c_uint64, ctypes.c_bool, ctypes.c_bool, ctypes.c_uint64]
_crc.crc_params.restype = params_t

_crc.crc_table.argtypes = [ctypes.POINTER(params_t), ctypes.c_uint64, ctypes.c_char_p, ctypes.c_uint64]
_crc.crc_table.restype = ctypes.c_uint64

_crc.crc_braid.argtypes = [ctypes.POINTER(params_t), ctypes.c_uint64, ctypes.c_char_p, ctypes.c_uint64]
_crc.crc_braid.restype = ctypes.c_uint64

def crc_params(width, poly, init, refin, refout, xorout, check):
    return _crc.crc_params(width, poly, init, refin, refout, xorout)

def crc_table(params, crc, buf):
    return _crc.crc_table(ctypes.byref(params), crc, buf, len(buf))

def crc_braid(params, crc, buf):
    return _crc.crc_braid(ctypes.byref(params), crc, buf, len(buf))

def crc_braid_unaligned(params, crc, buf, shift):
    pointer = ctypes.cast(buf, ctypes.POINTER(ctypes.c_char))
    address = ctypes.addressof(pointer.contents)
    pointer2 = ctypes.cast(address + shift, ctypes.POINTER(ctypes.c_char))

    return _crc.crc_braid(ctypes.byref(params), crc, pointer2, len(buf) - shift)