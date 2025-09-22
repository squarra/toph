#include "frame.h"
#include <Eigen/Core>

#define PAR_SHAPES_IMPLEMENTATION
#include "par/par_shapes.h"

namespace toph {

Frame::Frame(std::string name, Eigen::Isometry3f X) : name_(std::move(name)), X_(std::move(X)) {}

void Frame::addChild(const Frame::Ptr &child) {
    if (!child) return;
    child->parent_ = weak_from_this();
    children_.push_back(child);
}

Frame::Ptr Frame::parent() const { return parent_.lock(); }

const std::vector<Frame::Ptr> &Frame::children() const noexcept { return children_; }

Eigen::Isometry3f Frame::worldX() const {
    if (auto p = parent_.lock()) { return p->worldX() * X_; }
    return X_;
}

Frame::Ptr Frame::Cube(const std::string &name, const Eigen::Vector3f &size, const Eigen::Vector3f &color,
                       const Eigen::Isometry3f &X) {
    auto frame = std::make_shared<Frame>(name, X);
    auto cube = par_shapes_create_cube();

    for (int i = 0; i < cube->npoints; i++) {
        cube->points[3 * i + 0] *= size.x() * 0.5f;
        cube->points[3 * i + 1] *= size.y() * 0.5f;
        cube->points[3 * i + 2] *= size.z() * 0.5f;
    }

    frame->vertices.resize(cube->npoints);
    for (int i = 0; i < cube->npoints; i++) {
        frame->vertices[i] = Eigen::Vector3f(cube->points[3 * i + 0], cube->points[3 * i + 1], cube->points[3 * i + 2]);
    }

    frame->faces.resize(cube->ntriangles);
    for (int i = 0; i < cube->ntriangles; i++) {
        frame->faces[i] =
            Eigen::Vector3i(cube->triangles[3 * i + 0], cube->triangles[3 * i + 1], cube->triangles[3 * i + 2]);
    }

    if (cube->normals) {
        frame->normals.resize(cube->npoints);
        for (int i = 0; i < cube->npoints; i++) {
            frame->normals[i] =
                Eigen::Vector3f(cube->normals[3 * i + 0], cube->normals[3 * i + 1], cube->normals[3 * i + 2]);
        }
    } else {
        frame->normals.clear();
    }

    frame->colors.assign(cube->npoints, color);

    par_shapes_free_mesh(cube);

    return frame;
}

std::string Frame::to_string() const {
    auto p = parent_.lock();
    const std::string parent_name = p ? p->name_ : "None";
    std::stringstream ss;
    ss << "Frame(name=" << name_ << ", parent=" << parent_name << ", children=" << children_.size() << ")";
    return ss.str();
}

std::ostream &operator<<(std::ostream &os, const Frame &frame) { return os << frame.to_string(); }

} // namespace toph