#pragma once

#include "frame.h"
#include <memory>
#include <vector>

namespace toph {

class Viewer {
  public:
    Viewer(int width = 800, int height = 600, const char *title = "Toph Viewer");
    ~Viewer();

    Viewer(const Viewer &) = delete;
    Viewer &operator=(const Viewer &) = delete;

    void addFrame(const Frame::Ptr &frame);

    void run();

  private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};

} // namespace toph