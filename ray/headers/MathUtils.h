#pragma once

#include <type_traits>

#include "Eigen/Dense"

template <typename Vector>
inline constexpr std::decay<Vector>::type projection(
    Vector &&a, Vector &&on
) {
    auto const onNorm = on.normalized();
    return (onNorm * a.dot(onNorm)).eval();
}

template <typename Vector>
inline constexpr std::decay<Vector>::type rejection(
    Vector &&a, Vector &&on
) {
    return (a - projection(a, on)).eval();
}

Eigen::Vector4f inline asVec4(Eigen::Vector3f v, float fillValue = 0.f) {
    return {
        v.x(),
        v.y(),
        v.z(),
        fillValue
    };
}

Eigen::Matrix4xf inline projectionMatrix(
    float fov,
    float aspectRatio,
    bool keepHeight,
    float far,
    float near
) {
    Eigen::Matrix4xf mat = Eigen::Matrix4xf::Zeros();

    auto angleMeasure = tanf(fov / 2);
    mat(0, 0) = 1.f / (aspectRatio * angleMeasure);
    mat(1, 1) = 1.f / angleMeasure;
    mat(2, 2) = far / (far - near);
    mat(2, 3) = -far * near / (far - near);
    mat(3, 2) = 1.f;
    return mat;
}

Eigen::Matrix4xf inline screenSpaceProjection(
    size_t width,
    size_t height
) {
    Eigen::Matrix4xf mat = Eigen::Matrix4xf::Zeros();
    mat(0, 0) = width / 2.f;
    mat(1, 1) = height / 2.f;
    mat(2, 2) = 1.f;
    mat(3, 3) = 1.f;
    mat(1, 3) = width / 2.f;
    mat(2, 3) = height / 2.f;

    return mat;
}