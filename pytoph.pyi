import flags
import numpy

class Frame:
    matrix: numpy.ndarray[numpy.float32[4, 4]]
    name: str
    quaternion: numpy.ndarray[numpy.float32[4, 1]]
    rotation: numpy.ndarray[numpy.float32[3, 3], flags.f_contiguous]
    translation: numpy.ndarray[numpy.float32[3, 1]]
    def __init__(self, name: str) -> None:
        """__init__(self: pytoph.Frame, name: str) -> None"""
    def add_child(self, child: Frame) -> None:
        """add_child(self: pytoph.Frame, child: pytoph.Frame) -> None"""
    def rotate(self, arg0) -> None:
        """rotate(self: pytoph.Frame, arg0: Eigen::AngleAxis<float>) -> None"""
    def translate(self, arg0: numpy.ndarray[numpy.float32[3, 1]]) -> None:
        """translate(self: pytoph.Frame, arg0: numpy.ndarray[numpy.float32[3, 1]]) -> None"""
    @property
    def children(self) -> list[Frame]:
        """(arg0: pytoph.Frame) -> list[pytoph.Frame]"""
    @property
    def parent(self) -> Frame:
        """(arg0: pytoph.Frame) -> pytoph.Frame"""
    @property
    def world_matrix(self) -> numpy.ndarray[numpy.float32[4, 4]]:
        """(arg0: pytoph.Frame) -> numpy.ndarray[numpy.float32[4, 4]]"""
    @property
    def world_quaternion(self) -> numpy.ndarray[numpy.float32[4, 1]]:
        """(arg0: pytoph.Frame) -> numpy.ndarray[numpy.float32[4, 1]]"""
    @property
    def world_rotation(self) -> numpy.ndarray[numpy.float32[3, 3], flags.f_contiguous]:
        """(arg0: pytoph.Frame) -> numpy.ndarray[numpy.float32[3, 3], flags.f_contiguous]"""
    @property
    def world_translation(self) -> numpy.ndarray[numpy.float32[3, 1], flags.writeable]:
        """(arg0: pytoph.Frame) -> numpy.ndarray[numpy.float32[3, 1], flags.writeable]"""
