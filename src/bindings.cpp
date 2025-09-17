#include <pybind11/eigen.h>
#include <pybind11/pybind11.h>

#include "frame.h"

namespace py = pybind11;

PYBIND11_MODULE(pytoph, m) {
    py::class_<toph::Frame, std::shared_ptr<toph::Frame>>(m, "Frame")
        .def(py::init<>())
        .def(py::init<const std::string &>(), py::arg("name"))

        .def("add_child", &toph::Frame::addChild, py::arg("child"))

        .def_property_readonly("matrix", [](toph::Frame &f) { return f.local_pose.matrix(); })
        .def_property_readonly("pose", &toph::Frame::matrix)
        .def_property_readonly("world_pose", &toph::Frame::worldMatrix)
        .def_property_readonly("translation", &toph::Frame::translation)
        .def_property_readonly("rotation", &toph::Frame::rotation)
        .def_property_readonly("quaternion",
                               [](const toph::Frame &f) {
                                   Eigen::Quaternionf q(f.quaternion());
                                   return Eigen::Vector4f(q.w(), q.x(), q.y(), q.z());
                               })

        .def("set_translation", [](toph::Frame &f, const Eigen::Vector3f &t) { f.local_pose.translation() = t; })
        .def("translate", [](toph::Frame &f, const Eigen::Vector3f &delta) { f.local_pose.pretranslate(delta); })
        .def("rotate", [](toph::Frame &f, const Eigen::AngleAxisf &aa) { f.local_pose.rotate(aa); })

        .def_readwrite("name", &toph::Frame::name)
        .def("__repr__", &toph::Frame::to_string);
}
