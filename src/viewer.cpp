#include "viewer.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <iostream>

namespace toph {

inline Eigen::Matrix4f perspective(float fovyRadians, float aspect, float zNear, float zFar) {
    float f = 1.0f / std::tan(fovyRadians / 2.0f);
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

void framebuffer_size_callback(GLFWwindow *window, int width, int height) { glViewport(0, 0, width, height); }

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

struct GeometryGPU {
    GLuint vao{}, vbo{}, ebo{};
    GLsizei indexCount{};
    bool valid{false};
    bool hasColors{false};
    Eigen::Vector3f constColor{1, 1, 1};

    void uploadMesh(const std::vector<Eigen::Vector3f> &verts, const std::vector<Eigen::Vector3i> &faces,
                    const std::vector<Eigen::Vector3f> &colors, const Eigen::Vector3f &fallbackColor) {
        if (verts.empty())
            return;

        struct V {
            float px, py, pz;
            float cx, cy, cz;
        };
        std::vector<V> vdata(verts.size());

        for (size_t i = 0; i < verts.size(); ++i) {
            vdata[i].px = verts[i].x();
            vdata[i].py = verts[i].y();
            vdata[i].pz = verts[i].z();
            if (i < colors.size()) {
                vdata[i].cx = colors[i].x();
                vdata[i].cy = colors[i].y();
                vdata[i].cz = colors[i].z();
            } else {
                vdata[i].cx = fallbackColor.x();
                vdata[i].cy = fallbackColor.y();
                vdata[i].cz = fallbackColor.z();
            }
        }

        std::vector<unsigned int> idata;
        idata.reserve(faces.size() * 3);
        for (auto &f : faces) {
            idata.push_back(f.x());
            idata.push_back(f.y());
            idata.push_back(f.z());
        }

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vdata.size() * sizeof(V), vdata.data(), GL_STATIC_DRAW);

        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(V), (void *)0);
        glEnableVertexAttribArray(0);
        // color attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(V), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        if (!idata.empty()) {
            glGenBuffers(1, &ebo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, idata.size() * sizeof(unsigned int), idata.data(), GL_STATIC_DRAW);
            indexCount = idata.size();
        } else {
            indexCount = verts.size();
        }

        hasColors = true;
        valid = true;
    }

    void draw() const {
        if (!valid)
            return;
        glBindVertexArray(vao);
        if (ebo) {
            glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        } else {
            glDrawArrays(GL_LINES, 0, indexCount);
        }
    }
};

struct Viewer::Impl {
    int width, height;
    GLFWwindow *window{};
    GLuint shader{};
    GeometryGPU axes;

    std::vector<Frame::Ptr> frames;
    std::vector<GeometryGPU> gpuMeshes;

    // camera state
    Eigen::Vector3f camPos{3, 3, 3};
    Eigen::Vector3f camTarget{0, 0, 0};
    Eigen::Vector3f camUp{0, 1, 0};
    float camYaw = -M_PI / 4; // radians
    float camPitch = M_PI / 4;
    float camDist = 5.0f;

    bool leftButton = false;
    bool rightButton = false;
    double lastX = 0.0, lastY = 0.0;

    Impl(int w, int h, const char *title) : width(w), height(h) {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(width, height, title, nullptr, nullptr);
        glfwMakeContextCurrent(window);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
        glfwSetWindowUserPointer(window, this);
        glfwSetCursorPosCallback(window, [](GLFWwindow *win, double xpos, double ypos) {
            auto *self = static_cast<Impl *>(glfwGetWindowUserPointer(win));
            if (self->leftButton || self->rightButton) {
                double dx = xpos - self->lastX;
                double dy = ypos - self->lastY;

                if (self->leftButton) {
                    // orbit: adjust yaw/pitch
                    self->camYaw += dx * 0.005;
                    self->camPitch += dy * 0.005;
                    self->camPitch = std::clamp(self->camPitch, -1.5f, 1.5f); // avoid flipping
                }
                if (self->rightButton) {
                    // zoom (if Ctrl) or pan (if Shift)
                    if (glfwGetKey(win, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
                        glfwGetKey(win, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS) {
                        self->camDist *= (1.0f - dy * 0.01f); // zoom in/out
                        if (self->camDist < 0.1f)
                            self->camDist = 0.1f;
                    } else {
                        // pan target in screen plane
                        float panScale = self->camDist * 0.001f;
                        Eigen::Vector3f right(cosf(self->camYaw), 0, -sinf(self->camYaw));
                        Eigen::Vector3f up(0, 1, 0);
                        self->camTarget -= right * (float)dx * panScale;
                        self->camTarget += up * (float)dy * panScale;
                    }
                }
            }
            self->lastX = xpos;
            self->lastY = ypos;
        });

        glfwSetMouseButtonCallback(window, [](GLFWwindow *win, int button, int action, int) {
            auto *self = static_cast<Impl *>(glfwGetWindowUserPointer(win));
            if (button == GLFW_MOUSE_BUTTON_LEFT) {
                self->leftButton = (action == GLFW_PRESS);
            }
            if (button == GLFW_MOUSE_BUTTON_RIGHT) {
                self->rightButton = (action == GLFW_PRESS);
            }
            glfwGetCursorPos(win, &self->lastX, &self->lastY);
        });

        glfwSetScrollCallback(window, [](GLFWwindow *win, double, double yoff) {
            auto *self = static_cast<Impl *>(glfwGetWindowUserPointer(win));
            self->camDist *= (1.0f - (float)yoff * 0.1f);
            if (self->camDist < 0.1f)
                self->camDist = 0.1f;
        });

        if (!gladLoadGL())
            throw std::runtime_error("GLAD init fail");

        glEnable(GL_DEPTH_TEST);

        const char *vs = R"(
          #version 330 core
          layout(location=0) in vec3 pos;
          layout(location=1) in vec3 col;

          out vec3 vColor;

          uniform mat4 MVP;

          void main() {
          vColor = col;
          gl_Position = MVP * vec4(pos,1.0);
          }
        )";
        const char *fs = R"(
          #version 330 core
          in vec3 vColor;
          out vec4 FragColor;

          void main() {
          FragColor = vec4(vColor,1.0);
          }
        )";
        auto compile = [](GLenum t, const char *s) {
            GLuint sh = glCreateShader(t);
            glShaderSource(sh, 1, &s, nullptr);
            glCompileShader(sh);
            return sh;
        };
        GLuint vsid = compile(GL_VERTEX_SHADER, vs);
        GLuint fsid = compile(GL_FRAGMENT_SHADER, fs);
        shader = glCreateProgram();
        glAttachShader(shader, vsid);
        glAttachShader(shader, fsid);
        glLinkProgram(shader);
        glDeleteShader(vsid);
        glDeleteShader(fsid);

        // axis marker (3 lines)
        float axis[] = {0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0,
                        0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1};

        glGenVertexArrays(1, &axes.vao);
        glGenBuffers(1, &axes.vbo);
        glBindVertexArray(axes.vao);
        glBindBuffer(GL_ARRAY_BUFFER, axes.vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(axis), axis, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        axes.indexCount = 6;
        axes.valid = true;
    }

    void addFrame(const Frame::Ptr &f) {
        frames.push_back(f);
        GeometryGPU gpu;
        gpu.uploadMesh(f->vertices, f->faces, f->colors, f->frameColor);
        gpuMeshes.push_back(std::move(gpu));
    }

    void renderLoop() {
        GLint mvpLoc = glGetUniformLocation(shader, "MVP");
        const float fov = 45.0f * M_PI / 180.0f;

        while (!glfwWindowShouldClose(window)) {
            processInput(window);

            Eigen::Vector3f eye;
            eye.x() = camTarget.x() + camDist * cosf(camPitch) * cosf(camYaw);
            eye.y() = camTarget.y() + camDist * sinf(camPitch);
            eye.z() = camTarget.z() + camDist * cosf(camPitch) * sinf(camYaw);

            Eigen::Matrix4f view = lookAt(eye, camTarget, camUp);
            Eigen::Matrix4f proj = perspective(fov, (float)width / height, 0.1f, 100.f);

            glClearColor(0.1f, 0.1f, 0.1f, 1.f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glUseProgram(shader);

            for (size_t i = 0; i < frames.size(); ++i) {
                Eigen::Matrix4f model = frames[i]->worldX().matrix();
                Eigen::Matrix4f mvp = proj * view * model;
                glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, mvp.data());

                if (gpuMeshes[i].valid) {
                    gpuMeshes[i].draw();
                } else {
                    // fallback: axis marker lines
                    glBindVertexArray(axes.vao);
                    glDrawArrays(GL_LINES, 0, 6);
                }
            }

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }
};

Viewer::Viewer(int w, int h, const char *t) : impl_(std::make_unique<Impl>(w, h, t)) {}
Viewer::~Viewer() = default;
void Viewer::addFrame(const Frame::Ptr &f) { impl_->addFrame(f); }
void Viewer::run() { impl_->renderLoop(); }

} // namespace toph