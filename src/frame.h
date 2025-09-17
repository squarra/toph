#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <array>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace toph {

struct Frame {
    Frame();
    Frame(const std::string &name);
    Frame(const std::string &name, Frame *parent);

    std::string name;
    Frame *parent = nullptr;
    std::vector<std::shared_ptr<Frame>> children;

    std::shared_ptr<Frame> addChild(std::shared_ptr<Frame> child);

    std::vector<Eigen::Vector3f> vertices;
    std::vector<std::array<unsigned int, 3>> faces;

    Eigen::Isometry3f pose = Eigen::Isometry3f::Identity();
    Eigen::Isometry3f worldPose() const;

    std::string to_string() const;
    friend std::ostream &operator<<(std::ostream &os, const Frame &frame);
};

} // namespace toph