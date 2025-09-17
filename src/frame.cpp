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

std::string Frame::to_string() const {
    std::string parent_name = parent ? parent->name : "None";
    std::stringstream ss;
    ss << "Frame(name=" << name << ", parent=" << parent_name << ", children=" << children.size() << ")";
    return ss.str();
}

std::ostream &operator<<(std::ostream &os, const Frame &frame) { return os << frame.to_string(); }

Eigen::Isometry3f Frame::pose() const { return local_pose; }
Eigen::Isometry3f Frame::worldPose() const {
    if (parent) {
        return parent->worldPose() * local_pose;
    }
    return local_pose;
}

Eigen::Matrix4f Frame::matrix() const { return local_pose.matrix(); }
Eigen::Vector3f Frame::translation() const { return local_pose.translation(); }
Eigen::Matrix3f Frame::rotation() const { return local_pose.rotation(); }
Eigen::Quaternionf Frame::quaternion() const { return Eigen::Quaternionf(rotation()); }

Eigen::Matrix4f Frame::worldMatrix() const { return worldPose().matrix(); }
Eigen::Vector3f Frame::worldTranslation() const { return worldPose().translation(); }
Eigen::Matrix3f Frame::worldRotation() const { return worldPose().rotation(); }
Eigen::Quaternionf Frame::worldQuaternion() const { return Eigen::Quaternionf(worldRotation()); }

} // namespace toph