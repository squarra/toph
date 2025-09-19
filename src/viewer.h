#pragma once
#include "frame.h"
#include <memory>
#include <vector>

namespace toph {

class Viewer {
  public:
    Viewer(int width = 800, int height = 600, const char *title = "Toph Viewer");
    ~Viewer();

    void addFrame(const Frame::Ptr &frame);

    void run(); // main loop

  private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace toph