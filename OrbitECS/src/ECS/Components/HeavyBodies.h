#include <iostream>

// Concerne uniquement les grosses entitées (Planettes, étoiles, lunes)
struct HeavyBodies {
    struct Dynamic {
        std::vector<double> x, y, z;

        explicit Dynamic(std::size_t count) : x(count), y(count), z(count) {}

        void swap(Dynamic& other) noexcept {
            x.swap(other.x);
            y.swap(other.y);
            z.swap(other.z);
        }
    };
    // mass, compute, vx, vy, vz, acceleration — ailleurs, dans World
};