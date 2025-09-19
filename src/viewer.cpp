#include "viewer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <Eigen/Core>
#include <Eigen/Geometry>

#include <array>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace toph {

inline Eigen::Matrix4f perspective(float fovyRadians, float aspect, float zNear, float zFar) {
    const float f = 1.0f / std::tan(fovyRadians / 2.0f);
    Eigen::Matrix4f m = Eigen::Matrix4f::Zero();
    m(0, 0) = f / aspect;
    m(1, 1) = f;
    m(2, 2) = (zFar + zNear) / (zNear - zFar);
    m(2, 3) = (2.0f * zFar * zNear) / (zNear - zFar);
    m(3, 2) = -1.0f;
    return m;
}

inline Eigen::Matrix4f lookAt(const Eigen::Vector3f &eye, const Eigen::Vector3f &center, const Eigen::Vector3f &up) {
    Eigen::Vector3f f = (center - eye).normalized();
    Eigen::Vector3f s = f.cross(up.normalized()).normalized();
    Eigen::Vector3f u = s.cross(f);

    Eigen::Matrix4f m = Eigen::Matrix4f::Identity();
    m.block<1, 3>(0, 0) = s.transpose();
    m.block<1, 3>(1, 0) = u.transpose();
    m.block<1, 3>(2, 0) = -f.transpose();

    Eigen::Matrix4f trans = Eigen::Matrix4f::Identity();
    trans.block<3, 1>(0, 3) = -eye;
    return m * trans;
}

constexpr char VERTEX_SHADER_SRC[] = R"(
  #version 330 core
  layout(location=0) in vec3 a_position;
  layout(location=1) in vec3 a_color;

  out vec3 v_color;

  uniform mat4 u_modelViewProjection;

  void main() {
    v_color = a_color;
    gl_Position = u_modelViewProjection * vec4(a_position, 1.0);
  }
)";

constexpr char FRAGMENT_SHADER_SRC[] = R"(
  #version 330 core
  in vec3 v_color;
  out vec4 FragColor;

  void main() {
    FragColor = vec4(v_color, 1.0);
  }
)";

struct GeometryGPU {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    GLsizei indexCount = 0;

    GeometryGPU() = default;

    ~GeometryGPU() {
        if (vbo)
            glDeleteBuffers(1, &vbo);
        if (ebo)
            glDeleteBuffers(1, &ebo);
        if (vao)
            glDeleteVertexArrays(1, &vao);
    }

    GeometryGPU(const GeometryGPU &) = delete;
    GeometryGPU &operator=(const GeometryGPU &) = delete;

    GeometryGPU(GeometryGPU &&other) noexcept
        : vao(other.vao), vbo(other.vbo), ebo(other.ebo), indexCount(other.indexCount) {
        other.vao = 0;
        other.vbo = 0;
        other.ebo = 0;
        other.indexCount = 0;
    }

    // Move assignment operator
    GeometryGPU &operator=(GeometryGPU &&other) noexcept {
        if (this != &other) {
            this->~GeometryGPU(); // Free existing resources
            vao = std::exchange(other.vao, 0);
            vbo = std::exchange(other.vbo, 0);
            ebo = std::exchange(other.ebo, 0);
            indexCount = std::exchange(other.indexCount, 0);
        }
        return *this;
    }

    bool isValid() const { return vao != 0; }

    void uploadMesh(const std::vector<Eigen::Vector3f> &verts, const std::vector<Eigen::Vector3i> &faces,
                    const std::vector<Eigen::Vector3f> &colors, const Eigen::Vector3f &fallbackColor);

    void draw() const;
};

void GeometryGPU::uploadMesh(const std::vector<Eigen::Vector3f> &verts, const std::vector<Eigen::Vector3i> &faces,
                             const std::vector<Eigen::Vector3f> &colors, const Eigen::Vector3f &fallbackColor) {
    if (verts.empty())
        return;

    // Interleaved vertex data structure
    struct Vertex {
        Eigen::Vector3f pos;
        Eigen::Vector3f color;
    };

    std::vector<Vertex> vertexData;
    vertexData.reserve(verts.size());
    for (size_t i = 0; i < verts.size(); ++i) {
        vertexData.push_back({verts[i], (i < colors.size()) ? colors[i] : fallbackColor});
    }

    std::vector<unsigned int> indexData;
    indexData.reserve(faces.size() * 3);
    for (const auto &face : faces) {
        indexData.push_back(face.x());
        indexData.push_back(face.y());
        indexData.push_back(face.z());
    }

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(Vertex), vertexData.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, pos));
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, color));
    glEnableVertexAttribArray(1);

    if (!indexData.empty()) {
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(unsigned int), indexData.data(),
                     GL_STATIC_DRAW);
        indexCount = static_cast<GLsizei>(indexData.size());
    } else {
        indexCount = static_cast<GLsizei>(verts.size());
    }
}

void GeometryGPU::draw() const {
    if (!isValid())
        return;
    glBindVertexArray(vao);
    if (ebo) {
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
    } else {
        glDrawArrays(GL_LINES, 0, indexCount); // Assuming non-indexed are lines
    }
}

// --- Viewer Implementation (PIMPL) ------------------------------------------

struct Viewer::Impl {
    // Window and rendering state
    int width_;
    int height_;
    GLFWwindow *window_ = nullptr;
    GLuint shaderProgram_ = 0;
    GeometryGPU fallbackAxes_;

    // Scene data
    std::vector<Frame::Ptr> frames_;
    std::vector<GeometryGPU> gpuMeshes_;

    // Camera state
    Eigen::Vector3f cameraTarget_{0.0f, 0.0f, 0.0f};
    Eigen::Vector3f cameraUp_{0.0f, 0.0f, 1.0f}; // Z-up convention
    float cameraYaw_ = M_PI / 4.0f;
    float cameraPitch_ = M_PI / 4.0f;
    float cameraDistance_ = 5.0f;

    // Mouse interaction state
    bool isLeftMouseDown_ = false;
    bool isRightMouseDown_ = false;
    double lastMouseX_ = 0.0;
    double lastMouseY_ = 0.0;

    Impl(int width, int height, const char *title);
    ~Impl();

    void addFrame(const Frame::Ptr &f);
    void run();

  private:
    void initWindow(const char *title);
    void initGraphics();
    void initShaders();
    void initCallbacks();
    void initFallbackGeometry();

    void handleWindowInput();
    void onCursorMove(double xpos, double ypos);
    void onMouseButton(int button, int action);
    void onScroll(double yoffset);

    void calculateViewProjectionMatrices(Eigen::Matrix4f &view, Eigen::Matrix4f &projection);
};

Viewer::Impl::Impl(int width, int height, const char *title) : width_(width), height_(height) {
    initWindow(title);
    initGraphics();
    initShaders();
    initCallbacks();
    initFallbackGeometry();
}

Viewer::Impl::~Impl() {
    if (window_) {
        glfwDestroyWindow(window_);
    }
}

void Viewer::Impl::initWindow(const char *title) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window_ = glfwCreateWindow(width_, height_, title, nullptr, nullptr);
    if (!window_) {
        throw std::runtime_error("Failed to create GLFW window");
    }
    glfwMakeContextCurrent(window_);
}

void Viewer::Impl::initGraphics() {
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        throw std::runtime_error("Failed to initialize GLAD");
    }
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, width_, height_);
}

void Viewer::Impl::initShaders() {
    auto compileShader = [](GLenum type, const char *source) -> GLuint {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);
        return shader;
    };

    GLuint vs = compileShader(GL_VERTEX_SHADER, VERTEX_SHADER_SRC);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, FRAGMENT_SHADER_SRC);

    shaderProgram_ = glCreateProgram();
    glAttachShader(shaderProgram_, vs);
    glAttachShader(shaderProgram_, fs);
    glLinkProgram(shaderProgram_);
    // Add linking error checking here

    glDeleteShader(vs);
    glDeleteShader(fs);
}

void Viewer::Impl::initCallbacks() {
    glfwSetWindowUserPointer(window_, this);

    glfwSetFramebufferSizeCallback(window_, [](GLFWwindow *, int w, int h) { glViewport(0, 0, w, h); });

    glfwSetCursorPosCallback(window_, [](GLFWwindow *win, double x, double y) {
        static_cast<Impl *>(glfwGetWindowUserPointer(win))->onCursorMove(x, y);
    });

    glfwSetMouseButtonCallback(window_, [](GLFWwindow *win, int btn, int act, int) {
        static_cast<Impl *>(glfwGetWindowUserPointer(win))->onMouseButton(btn, act);
    });

    glfwSetScrollCallback(window_, [](GLFWwindow *win, double, double y) {
        static_cast<Impl *>(glfwGetWindowUserPointer(win))->onScroll(y);
    });
}

void Viewer::Impl::initFallbackGeometry() {
    // 3 lines for X(red), Y(green), Z(blue) axes
    const std::vector<Eigen::Vector3f> verts = {
        {0, 0, 0}, {1, 0, 0}, // X
        {0, 0, 0}, {0, 1, 0}, // Y
        {0, 0, 0}, {0, 0, 1}  // Z
    };
    const std::vector<Eigen::Vector3f> colors = {
        {1, 0, 0}, {1, 0, 0}, // Red
        {0, 1, 0}, {0, 1, 0}, // Green
        {0, 0, 1}, {0, 0, 1}  // Blue
    };
    fallbackAxes_.uploadMesh(verts, {}, colors, {});
}

void Viewer::Impl::addFrame(const Frame::Ptr &frame) {
    frames_.push_back(frame);
    GeometryGPU gpuMesh;
    gpuMesh.uploadMesh(frame->vertices, frame->faces, frame->colors, frame->frameColor);
    gpuMeshes_.push_back(std::move(gpuMesh));
}

void Viewer::Impl::handleWindowInput() {
    if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS || glfwGetKey(window_, GLFW_KEY_Q) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window_, true);
    }
}

void Viewer::Impl::onCursorMove(double xpos, double ypos) {
    if (!isLeftMouseDown_ && !isRightMouseDown_)
        return;

    const double dx = xpos - lastMouseX_;
    const double dy = ypos - lastMouseY_;

    if (isLeftMouseDown_) { // Orbit
        cameraYaw_ -= static_cast<float>(dx) * 0.005f;
        cameraPitch_ += static_cast<float>(dy) * 0.005f;
        cameraPitch_ = std::clamp(cameraPitch_, -1.57f, 1.57f); // Avoid gimbal lock
    }

    if (isRightMouseDown_) { // Pan
        const float panScale = cameraDistance_ * 0.001f;
        Eigen::Matrix4f view, proj;
        calculateViewProjectionMatrices(view, proj);
        Eigen::Matrix3f invView = view.topLeftCorner<3, 3>().transpose();
        Eigen::Vector3f right = invView.col(0);
        Eigen::Vector3f up = invView.col(1);

        cameraTarget_ -= right * static_cast<float>(dx) * panScale;
        cameraTarget_ += up * static_cast<float>(dy) * panScale;
    }

    lastMouseX_ = xpos;
    lastMouseY_ = ypos;
}

void Viewer::Impl::onMouseButton(int button, int action) {
    if (button == GLFW_MOUSE_BUTTON_LEFT)
        isLeftMouseDown_ = (action == GLFW_PRESS);
    if (button == GLFW_MOUSE_BUTTON_RIGHT)
        isRightMouseDown_ = (action == GLFW_PRESS);
    glfwGetCursorPos(window_, &lastMouseX_, &lastMouseY_);
}

void Viewer::Impl::onScroll(double yoffset) {
    cameraDistance_ *= (1.0f - static_cast<float>(yoffset) * 0.1f);
    cameraDistance_ = std::max(0.1f, cameraDistance_);
}

void Viewer::Impl::calculateViewProjectionMatrices(Eigen::Matrix4f &view, Eigen::Matrix4f &projection) {
    Eigen::Vector3f eye;
    eye.x() = cameraTarget_.x() + cameraDistance_ * std::cos(cameraPitch_) * std::cos(cameraYaw_);
    eye.y() = cameraTarget_.y() + cameraDistance_ * std::cos(cameraPitch_) * std::sin(cameraYaw_);
    eye.z() = cameraTarget_.z() + cameraDistance_ * std::sin(cameraPitch_);

    view = lookAt(eye, cameraTarget_, cameraUp_);

    int currentWidth, currentHeight;
    glfwGetFramebufferSize(window_, &currentWidth, &currentHeight);
    const float aspect = (currentHeight > 0) ? (float)currentWidth / currentHeight : 1.0f;
    const float fovRadians = 45.0f * M_PI / 180.0f;
    projection = perspective(fovRadians, aspect, 0.1f, 100.f);
}

void Viewer::Impl::run() {
    GLint mvpLocation = glGetUniformLocation(shaderProgram_, "u_modelViewProjection");

    while (!glfwWindowShouldClose(window_)) {
        handleWindowInput();

        Eigen::Matrix4f viewMatrix, projectionMatrix;
        calculateViewProjectionMatrices(viewMatrix, projectionMatrix);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgram_);

        for (size_t i = 0; i < frames_.size(); ++i) {
            Eigen::Matrix4f modelMatrix = frames_[i]->worldX().matrix();
            Eigen::Matrix4f mvp = projectionMatrix * viewMatrix * modelMatrix;
            glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, mvp.data());

            if (gpuMeshes_[i].isValid()) {
                gpuMeshes_[i].draw();
            } else {
                fallbackAxes_.draw();
            }
        }

        glfwSwapBuffers(window_);
        glfwPollEvents();
    }
}

// --- Public Viewer API (forwarding to PIMPL) --------------------------------

Viewer::Viewer(int w, int h, const char *t) {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }
    pimpl_ = std::make_unique<Impl>(w, h, t);
}

Viewer::~Viewer() {
    pimpl_.reset();
    glfwTerminate();
}

void Viewer::addFrame(const Frame::Ptr &frame) { pimpl_->addFrame(frame); }

void Viewer::run() { pimpl_->run(); }

} // namespace toph