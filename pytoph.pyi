import numpy
from typing import overload

class Frame:
    name: str
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
    def set_translation(self, arg0: numpy.ndarray[numpy.float32[3, 1]]) -> None:
        """set_translation(self: pytoph.Frame, arg0: numpy.ndarray[numpy.float32[3, 1]]) -> None"""
    def translate(self, arg0: numpy.ndarray[numpy.float32[3, 1]]) -> None:
        """translate(self: pytoph.Frame, arg0: numpy.ndarray[numpy.float32[3, 1]]) -> None"""
    @property
    def pose(self) -> numpy.ndarray[numpy.float32[4, 4]]:
        """(arg0: pytoph.Frame) -> numpy.ndarray[numpy.float32[4, 4]]"""
    @property
    def quaternion(self) -> numpy.ndarray[numpy.float32[4, 1]]:
        """(arg0: pytoph.Frame) -> numpy.ndarray[numpy.float32[4, 1]]"""
    @property
    def rotation(self) -> numpy.ndarray[numpy.float32[3, 3]]:
        """(arg0: pytoph.Frame) -> numpy.ndarray[numpy.float32[3, 3]]"""
    @property
    def translation(self) -> numpy.ndarray[numpy.float32[3, 1]]:
        """(arg0: pytoph.Frame) -> numpy.ndarray[numpy.float32[3, 1]]"""
    @property
    def world_pose(self) -> numpy.ndarray[numpy.float32[4, 4]]:
        """(arg0: pytoph.Frame) -> numpy.ndarray[numpy.float32[4, 4]]"""
