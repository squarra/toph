#include <pybind11/eigen.h>
#include <pybind11/pybind11.h>

#include "frame.h"

namespace py = pybind11;

PYBIND11_MODULE(pytoph, m) {
    py::class_<toph::Frame, std::shared_ptr<toph::Frame>>(m, "Frame")
        .def(py::init<>())
        .def(py::init<const std::string &>(), py::arg("name"))

        .def("add_child", &toph::Frame::addChild, py::arg("child"))

        .def_property(
            "matrix", [](const toph::Frame &f) { return f.pose.matrix(); },
            [](toph::Frame &f, const Eigen::Matrix4f &M) { f.pose = Eigen::Isometry3f(M); })
        .def_property(
            "translation", [](const toph::Frame &f) { return f.pose.translation(); },
            [](toph::Frame &f, const Eigen::Vector3f &t) { f.pose.translation() = t; })
        .def_property(
            "rotation", [](const toph::Frame &f) { return f.pose.rotation(); },
            [](toph::Frame &f, const Eigen::Matrix3f &R) { f.pose.linear() = R; })
        .def_property(
            "quaternion",
            [](const toph::Frame &f) {
                Eigen::Quaternionf q(f.pose.rotation());
                return Eigen::Vector4f(q.w(), q.x(), q.y(), q.z());
            },
            [](toph::Frame &f, const Eigen::Vector4f &qvec) {
                Eigen::Quaternionf q(qvec[0], qvec[1], qvec[2], qvec[3]);
                f.pose.linear() = q.normalized().toRotationMatrix();
            })

        .def("translate", [](toph::Frame &f, const Eigen::Vector3f &delta) { f.pose.pretranslate(delta); })
        .def("rotate", [](toph::Frame &f, const Eigen::AngleAxisf &aa) { f.pose.rotate(aa); })

        .def_readwrite("name", &toph::Frame::name)
        .def("__repr__", &toph::Frame::to_string);
}