#include <iostream>
#include <memory>

#include "MathUtils.h"
#include "Utility.h"
#include "sdl/SDLRenderer.h"

SDLRenderer::SDLRenderer(SDL_Window *window, std::pair<size_t, size_t> resolution)
    : renderer(SDL_CreateRenderer(window, -1, 0)),
      resolution(resolution) {

    auto resolutionSize = resolution.first * resolution.second;
    colorBuffer = std::unique_ptr<uint32_t[]>(new uint32_t[resolutionSize]);
    zBuffer = std::unique_ptr<uint32_t[]>(new uint32_t[resolutionSize]);
    perspectiveProjectionMatrix_ = perspectiveProjectionMatrix(
        1.3962634016,
        static_cast<float>(resolution.first) / resolution.second,
        true,
        0.1,
        1000
    );
    screenSpaceProjection_ = screenSpaceProjection(resolution.first, resolution.second);
    renderTarget = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STREAMING,
        resolution.first,
        resolution.second
    );
}

SDLRenderer::~SDLRenderer() {
    SDL_DestroyRenderer(renderer);
}

void SDLRenderer::update(MeshData const &data, float dt) {
    for (auto const &mesh : data) {
        for (size_t i = 0; i < mesh.faces.size(); i++) {
            auto const &face = mesh.faces[i];
            // perspective projection
            Triangle tri = Triangle{
                {perspectiveProjectionMatrix_ * asVec4(mesh.vertices[face.a], 1),
                 perspectiveProjectionMatrix_ * asVec4(mesh.vertices[face.b], 1),
                 perspectiveProjectionMatrix_ * asVec4(mesh.vertices[face.c], 1)},
                face.attributes // std::move() ?
            };

            tri.vertices[0] /= tri.vertices[0](3, 0);
            tri.vertices[1] /= tri.vertices[1](3, 0);
            tri.vertices[2] /= tri.vertices[2](3, 0);

            // screen space projection
            tri = Triangle{
                {screenSpaceProjection_ * tri.vertices[0],
                 screenSpaceProjection_ * tri.vertices[1],
                 screenSpaceProjection_ * tri.vertices[2]
                },
                face.attributes
            };

            // Add switch over render type
            // drawLine(
            //     Eigen::Vector2i(tri.vertices[0].x(), tri.vertices[0].y()),
            //     Eigen::Vector2i(tri.vertices[1].x(), tri.vertices[1].y()),
            //     0xFF11FFFF
            // );
            // drawLine(
            //     Eigen::Vector2i(tri.vertices[1].x(), tri.vertices[1].y()),
            //     Eigen::Vector2i(tri.vertices[2].x(), tri.vertices[2].y()),
            //     0xFF11FFFF
            // );
            // drawLine(
            //     Eigen::Vector2i(tri.vertices[2].x(), tri.vertices[2].y()),
            //     Eigen::Vector2i(tri.vertices[0].x(), tri.vertices[0].y()),
            //     0xFF11FFFF
            // );
            fillTriangle(tri);
        }
    }
}

void SDLRenderer::render() const {
    auto const &[w, h] = resolution;
    SDL_UpdateTexture(
        renderTarget,
        nullptr,
        colorBuffer.get(),
        w * sizeof(uint32_t)
    );
    SDL_RenderCopy(renderer, renderTarget, nullptr, nullptr);
    memset(colorBuffer.get(), (uint32_t)0xFF000000, w * h * sizeof(uint32_t));
    SDL_RenderPresent(renderer);
}

void SDLRenderer::drawPoint(uint32_t color, Eigen::Vector2i position, size_t thickness) noexcept {
    if (thickness == 0) {
        colorBuffer[position.x() + position.y() * resolution.first] = color;
        return;
    }

    int thick = thickness;
    for (int i = -thick; i < thick; i++) {
        for (int j = -thick; j < thick; j++) {
            colorBuffer[position.x() + i + (position.y() + j) * resolution.first] = color;
        }
    }
}

static constexpr float fpart(float num) {
    return num - static_cast<int>(num);
}

static constexpr float oneComplement(float num) {
    return 1 - fpart(num);
}

void SDLRenderer::drawLine(Eigen::Vector2i from, Eigen::Vector2i to, uint32_t color) noexcept {
    int x0{from.x()},
        y0{from.y()},
        x1{to.x()},
        y1{to.y()};
    bool isSteep = std::abs(from.y() - to.y()) > std::abs(from.x() - to.x());
    if (isSteep) {
        std::swap(x0, y0);
        std::swap(x1, y1);
    }
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    int dx{x1 - x0}, dy{y1 - y0};
    float slope = dx == 0
                      ? dy / std::abs(dy)
                      : static_cast<float>(dy) / dx;

    std::function<void(size_t x, size_t y, uint32_t value)> assign = [p = colorBuffer.get(), isSteep, res = resolution](size_t x, size_t y, uint32_t value) {
        if (isSteep) {
            std::swap(x, y);
        }
        p[x + y * res.first] = value;
    };

    int x = x0;
    float y = y0;
    while (x <= x1) {
        int y_ = y;
        std::cout << "oneComplement(std::abs(y))" << oneComplement(std::abs(y)) << " fpart(std::abs(y))) " << fpart(std::abs(y)) << std::endl;
        assign(x, y_, interpolateColorIntensity(color, oneComplement(std::abs(y))));
        assign(x, y_ + 1, interpolateColorIntensity(color, fpart(std::abs(y))));
        x++;
        y += slope;
    }
}

void SDLRenderer::fillTriangle(Triangle tri) noexcept {
    uint32_t color = 0xFFFFFFFF; // Constant color for a while

    auto &t0 = tri.vertices[0];
    auto &t1 = tri.vertices[1];
    auto &t2 = tri.vertices[2];
    if (t0.y() > t1.y()) {
        swap(t0, t1);
    }
    if (t1.y() > t2.y()) {
        swap(t1, t2);
    }
    if (t0.y() > t1.y()) {
        swap(t0, t1);
    }

    if (t0.y() != t1.y()) {
        float inv_slope0 = static_cast<float>(t1.x() - t0.x()) / (t1.y() - t0.y());
        float inv_slope1 = static_cast<float>(t2.x() - t0.x()) / (t2.y() - t0.y());
        float x0{t0.x()}, x1{t0.x()};
        for (size_t y = t0.y(); y <= t1.y(); y++) {
            int begin = std::min(x0, x1);
            int end = std::max(x0, x1);
            while (begin != end) {
                colorBuffer[begin + y * resolution.first] = color;
                begin++;
            }
            x0 += inv_slope0;
            x1 += inv_slope1;
        }
    }
    if (t1.y() != t2.y()) {
        float inv_slope0 = static_cast<float>(t0.x() - t2.x()) / (t2.y() - t0.y());
        float inv_slope1 = static_cast<float>(t1.x() - t2.x()) / (t2.y() - t1.y());
        float x0{t2.x()}, x1{t2.x()};
        for (int y = t2.y(); y > t1.y(); y--) {
            int begin = std::min(x0, x1);
            int end = std::max(x0, x1);
            while (begin != end) {
                colorBuffer[begin + y * resolution.first] = color;
                begin++;
            }
            x0 += inv_slope0;
            x1 += inv_slope1;
        }
    }
}