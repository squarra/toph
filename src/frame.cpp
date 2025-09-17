#include "frame.h"
#include <string>

namespace toph {

Frame::Frame() {}
Frame::Frame(const std::string &name) : name(name) {}
Frame::Frame(const std::string &name, Frame *parent) : name(name), parent(parent) {}

std::shared_ptr<Frame> Frame::addChild(std::shared_ptr<Frame> child) {
    child->parent = this;
    children.push_back(child);
    return child;
}

Eigen::Isometry3f Frame::worldPose() const { return parent ? parent->worldPose() * pose : pose; }

std::string Frame::to_string() const {
    std::string parent_name = parent ? parent->name : "None";
    std::stringstream ss;
    ss << "Frame(name=" << name << ", parent=" << parent_name << ", children=" << children.size() << ")";
    return ss.str();
}

std::ostream &operator<<(std::ostream &os, const Frame &frame) { return os << frame.to_string(); }

} // namespace toph