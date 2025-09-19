#pragma once

#include <Eigen/Geometry>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

namespace toph {

class Frame : public std::enable_shared_from_this<Frame> {
  public:
    using Ptr = std::shared_ptr<Frame>;

    explicit Frame(std::string name, Eigen::Isometry3f X = Eigen::Isometry3f::Identity());

    void addChild(const Ptr &child);
    Ptr parent() const;
    const std::vector<Ptr> &children() const noexcept;

    const std::string &name() const noexcept { return name_; }
    void setName(const std::string &n) { name_ = n; }

    const Eigen::Isometry3f &X() const noexcept { return X_; }
    Eigen::Isometry3f& mutableX() noexcept { return X_; }
    void setX(const Eigen::Isometry3f &x) { X_ = x; }

    Eigen::Isometry3f worldX() const;

    std::string to_string() const;

  private:
    std::string name_;
    Eigen::Isometry3f X_;

    std::weak_ptr<Frame> parent_;
    std::vector<Ptr> children_;
};

std::ostream &operator<<(std::ostream &os, const Frame &frame);

} // namespace toph