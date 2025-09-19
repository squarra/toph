#include "viewer.h"
#include "frame.h"
#include <memory>

// int main() {
//     using namespace toph;

//     auto root = std::make_shared<Frame>("root");
//     auto cube = std::make_shared<Frame>("cube");
//     cube->mutableX().translation() << 1, 0, 0;
//     // 8 cube vertices
//     cube->vertices = {{-0.5, -0.5, -0.5}, {0.5, -0.5, -0.5}, {0.5, 0.5, -0.5}, {-0.5, 0.5, -0.5},
//                       {-0.5, -0.5, 0.5},  {0.5, -0.5, 0.5},  {0.5, 0.5, 0.5},  {-0.5, 0.5, 0.5}};
//     cube->faces = {
//         {0, 1, 2}, {2, 3, 0}, {4, 5, 6}, {6, 7, 4}, // front/back
//         {0, 1, 5}, {5, 4, 0}, {2, 3, 7}, {7, 6, 2}, // sides
//         {0, 3, 7}, {7, 4, 0}, {1, 2, 6}, {6, 5, 1}  // top/bottom
//     };
//     // optional solid color
//     cube->frameColor = {0.4f, 0.7f, 0.9f};

//     root->addChild(cube);

//     Viewer v;
//     v.addFrame(root); // root: marker only
//     v.addFrame(cube); // cube geometry
//     v.run();
// }

int main() {
    using namespace toph;
    auto root = std::make_shared<Frame>("root");
    Viewer v;
    v.addFrame(root);
    v.run();
}