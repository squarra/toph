import flags
import numpy
from typing import overload

class Frame:
    matrix: numpy.ndarray[numpy.float32[4, 4]]
    name: str
    quaternion: numpy.ndarray[numpy.float32[4, 1]]
    rotation: numpy.ndarray[numpy.float32[3, 3], flags.f_contiguous]
    translation: numpy.ndarray[numpy.float32[3, 1]]
    @overload
    def __init__(self) -> None:
        """__init__(*args, **kwargs)
        Overloaded function.

        1. __init__(self: pytoph.Frame) -> None

        2. __init__(self: pytoph.Frame, name: str) -> None
        """
    @overload
    def __init__(self, name: str) -> None:
        """__init__(*args, **kwargs)
        Overloaded function.

        1. __init__(self: pytoph.Frame) -> None

        2. __init__(self: pytoph.Frame, name: str) -> None
        """
    def add_child(self, child: Frame) -> Frame:
        """add_child(self: pytoph.Frame, child: pytoph.Frame) -> pytoph.Frame"""
    def rotate(self, arg0) -> None:
        """rotate(self: pytoph.Frame, arg0: Eigen::AngleAxis<float>) -> None"""
    def translate(self, arg0: numpy.ndarray[numpy.float32[3, 1]]) -> None:
        """translate(self: pytoph.Frame, arg0: numpy.ndarray[numpy.float32[3, 1]]) -> None"""
