#include <iostream>

// LightBodies.h
struct LightBodies {
    struct Dynamic {
        std::vector<double> x, y, z;

        explicit Dynamic(std::size_t count) : x(count), y(count), z(count) {}

        void swap(Dynamic& other) noexcept {
            x.swap(other.x);
            y.swap(other.y);
            z.swap(other.z);
        }
    };
    // compute — ailleurs, dans World
};