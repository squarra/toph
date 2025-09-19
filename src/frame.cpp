#include "frame.h"

namespace toph {

Frame::Frame(std::string name, Eigen::Isometry3f X) : name_(std::move(name)), X_(std::move(X)) {}

void Frame::addChild(const Frame::Ptr &child) {
    if (!child)
        return;
    child->parent_ = weak_from_this();
    children_.push_back(child);
}

Frame::Ptr Frame::parent() const { return parent_.lock(); }

const std::vector<Frame::Ptr> &Frame::children() const noexcept { return children_; }

Eigen::Isometry3f Frame::worldX() const {
    if (auto p = parent_.lock()) {
        return p->worldX() * X_;
    }
    return X_;
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