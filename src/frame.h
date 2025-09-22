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

    std::vector<Eigen::Vector3f> vertices;
    std::vector<Eigen::Vector3i> faces;
    std::vector<Eigen::Vector3f> normals;
    std::vector<Eigen::Vector3f> colors;

    Eigen::Vector3f frameColor{1.0f, 1.0f, 1.0f};

    explicit Frame(std::string name, Eigen::Isometry3f X = Eigen::Isometry3f::Identity());

    void addChild(const Ptr &child);
    Ptr parent() const;
    const std::vector<Ptr> &children() const noexcept;

    const std::string &name() const noexcept { return name_; }
    void setName(const std::string &n) { name_ = n; }

    const Eigen::Isometry3f &X() const noexcept { return X_; }
    Eigen::Isometry3f &mutableX() noexcept { return X_; }
    void setX(const Eigen::Isometry3f &x) { X_ = x; }

    Eigen::Isometry3f worldX() const;

    static Ptr Cube(const std::string &name, const Eigen::Vector3f &size,
                    const Eigen::Vector3f &color = Eigen::Vector3f{1.0f, 1.0f, 1.0f},
                    const Eigen::Isometry3f &X = Eigen::Isometry3f::Identity());

    std::string to_string() const;

  private:
    std::string name_;
    Eigen::Isometry3f X_;

    std::weak_ptr<Frame> parent_;
    std::vector<Ptr> children_;
};

std::ostream &operator<<(std::ostream &os, const Frame &frame);

} // namespace toph