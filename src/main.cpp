#include "frame.h"
#include <Eigen/Geometry>
#include <iostream>

int main() {
    using namespace toph;

    // Create two frames, link_2 is child of link_1
    Frame link_1("link_1");
    Frame link_2("link_2", &link_1);

    // --- Local transforms ---
    // set translation directly
    link_1.pose.translation() = Eigen::Vector3f(1.0f, 2.0f, 3.0f);
    // rotate about Z
    link_1.pose.rotate(Eigen::AngleAxisf(0.5f, Eigen::Vector3f::UnitZ()));

    std::cout << link_1 << std::endl;
    std::cout << "link_1 local translation: " << link_1.pose.translation().transpose() << std::endl;
    std::cout << "link_1 local rotation:\n" << link_1.pose.rotation() << std::endl;

    // Child local pose
    link_2.pose.translation() = Eigen::Vector3f(0.0f, 1.0f, 0.0f);
    link_2.pose.rotate(Eigen::AngleAxisf(0.25f, Eigen::Vector3f::UnitX()));

    std::cout << link_2 << std::endl;
    std::cout << "link_2 local translation: " << link_2.pose.translation().transpose() << std::endl;
    std::cout << "link_2 local rotation:\n" << link_2.pose.rotation() << std::endl;

    // --- World transforms (computed on-demand) ---
    std::cout << "link_1 world translation: " << link_1.worldPose().translation().transpose() << std::endl;
    std::cout << "link_2 world translation: " << link_2.worldPose().translation().transpose() << std::endl;
    std::cout << "link_2 world matrix:\n" << link_2.worldPose().matrix() << std::endl;

    return 0;
}
