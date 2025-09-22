#include <pybind11/eigen.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "frame.h"
#include "viewer.h"

namespace py = pybind11;
using namespace toph;

PYBIND11_MODULE(pytoph, m) {
    py::class_<Frame, Frame::Ptr>(m, "Frame")
        .def(py::init<const std::string &>(), py::arg("name"))

        .def_property("name", &Frame::name, &Frame::setName)
        .def_property_readonly("parent", &Frame::parent)
        .def_property_readonly("children", &Frame::children)

        .def("add_child", &Frame::addChild, py::arg("child"))

        .def_property(
            "matrix", [](const Frame &f) { return f.X().matrix(); },
            [](Frame &f, const Eigen::Matrix4f &M) { f.setX(Eigen::Isometry3f(M)); })

        .def_property(
            "translation", [](const Frame &f) { return f.X().translation(); },
            [](Frame &f, const Eigen::Vector3f &t) {
                Eigen::Isometry3f X = f.X();
                X.translation() = t;
                f.setX(X);
            })

        .def_property(
            "rotation", [](const Frame &f) { return f.X().rotation(); },
            [](Frame &f, const Eigen::Matrix3f &R) {
                Eigen::Isometry3f X = f.X();
                X.linear() = R;
                f.setX(X);
            })

        .def_property(
            "quaternion",
            [](const Frame &f) {
                Eigen::Quaternionf q(f.X().rotation());
                return Eigen::Vector4f(q.w(), q.x(), q.y(), q.z());
            },
            [](Frame &f, const Eigen::Vector4f &qvec) {
                Eigen::Quaternionf q(qvec[0], qvec[1], qvec[2], qvec[3]);
                Eigen::Isometry3f X = f.X();
                X.linear() = q.normalized().toRotationMatrix();
                f.setX(X);
            })

        .def_property_readonly("world_matrix", [](const Frame &f) { return f.worldX().matrix(); })
        .def_property_readonly("world_translation", [](const Frame &f) { return f.worldX().translation(); })
        .def_property_readonly("world_rotation", [](const Frame &f) { return f.worldX().rotation(); })

        .def_property_readonly("world_quaternion",
                               [](const Frame &f) {
                                   Eigen::Quaternionf q(f.worldX().rotation());
                                   return Eigen::Vector4f(q.w(), q.x(), q.y(), q.z());
                               })

        .def("translate", [](Frame &f, const Eigen::Vector3f &delta) { f.mutableX().pretranslate(delta); })
        .def("rotate", [](Frame &f, const Eigen::AngleAxisf &aa) { f.mutableX().rotate(aa); })

        .def("show",
             [](const Frame::Ptr &self) {
                 Viewer v;
                 v.addFrame(self);
                 v.run();
             })

        .def("__repr__", &Frame::to_string);
}