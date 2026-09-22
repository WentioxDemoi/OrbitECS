// DebugPrint.cpp
#include "DebugPrint.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <stdexcept>

namespace DebugPrint {

void printLoadedBodies(const LoadedBodies& loaded) {
    const auto& heavy = loaded.heavy;
    const std::size_t count = heavy.count_;

    std::cout << "=== HeavyBodies (" << count << " corps) ===\n";
    std::cout << std::left
               << std::setw(10) << "Nom"
               << std::right
               << std::setw(16) << "x (km)"
               << std::setw(16) << "y (km)"
               << std::setw(16) << "z (km)"
               << std::setw(14) << "vx (km/s)"
               << std::setw(14) << "vy (km/s)"
               << std::setw(14) << "vz (km/s)"
               << std::setw(18) << "mass (kg)"
               << "\n";

    std::cout << std::scientific << std::setprecision(6);

    for (std::size_t i = 0; i < count; ++i) {
        std::cout << std::left << std::setw(10) << heavy.name[i]
                   << std::right
                   << std::setw(16) << heavy.dynamic_.x[i]
                   << std::setw(16) << heavy.dynamic_.y[i]
                   << std::setw(16) << heavy.dynamic_.z[i]
                   << std::setw(14) << heavy.vx[i]
                   << std::setw(14) << heavy.vy[i]
                   << std::setw(14) << heavy.vz[i]
                   << std::setw(18) << heavy.mass[i]
                   << "\n";
    }

    std::cout << "\n=== LightBodies (" << loaded.light.count_ << " corps) ===\n";
    std::cout << "(vide pour l'instant)\n";

    std::cout << "\n=== BodyMetaData (" << loaded.meta.size() << " entrées) ===\n";
    for (const auto& m : loaded.meta) {
        std::cout << " - " << m.name << " : " << m.text << "\n";
    }

    std::cout << std::defaultfloat; // reset flags pour ne pas polluer la suite du programme
}

void exportToCsv(const LoadedBodies& loaded, const std::string& path) {
    std::ofstream out(path);
    if (!out.is_open()) {
        throw std::runtime_error("DebugPrint::exportToCsv - impossible d'ouvrir " + path);
    }

    const auto& heavy = loaded.heavy;
    const std::size_t count = heavy.mass.size();

    out << "name,x_km,y_km,z_km,vx_km_s,vy_km_s,vz_km_s,mass_kg\n";
    out << std::scientific << std::setprecision(10);

    for (std::size_t i = 0; i < count; ++i) {
        out << heavy.name[i] << ','
            << heavy.dynamic_.x[i] << ','
            << heavy.dynamic_.y[i] << ','
            << heavy.dynamic_.z[i] << ','
            << heavy.vx[i] << ','
            << heavy.vy[i] << ','
            << heavy.vz[i] << ','
            << heavy.mass[i] << '\n';
    }
}

void exportStepToCsv(const HeavyBodies& heavy, const LightBodies& light,
                      const StateSnapshot& snapshot, int stepIndex,
                      const std::string& path) {
    const bool fileExists = std::ifstream(path).good();

    std::ofstream out(path, std::ios::app);
    if (!out.is_open()) {
        throw std::runtime_error("DebugPrint::exportStepToCsv - impossible d'ouvrir " + path);
    }

    if (!fileExists) {
        out << "step,type,name,x,y,z,vx,vy,vz,mass\n";
    }

    out << std::scientific << std::setprecision(10);

    // Heavy bodies
    for (std::size_t i = 0; i < heavy.mass.size(); ++i) {
        out << stepIndex << ",heavy," << heavy.name[i] << ','
            << snapshot.heavyDynamic.x[i] << ',' << snapshot.heavyDynamic.y[i] << ',' << snapshot.heavyDynamic.z[i] << ','
            << heavy.vx[i] << ',' << heavy.vy[i] << ',' << heavy.vz[i] << ','
            << heavy.mass[i] << '\n';
    }

    // Light bodies (pas de nom ni masse dans LightBodies)
    for (std::size_t k = 0; k < light.ax.size(); ++k) {
        out << stepIndex << ",light,," 
            << snapshot.lightDynamic.x[k] << ',' << snapshot.lightDynamic.y[k] << ',' << snapshot.lightDynamic.z[k] << ','
            << light.vx[k] << ',' << light.vy[k] << ',' << light.vz[k] << ','
            << "" << '\n';
    }
}

} // namespace DebugPrint