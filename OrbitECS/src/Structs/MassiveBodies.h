#include <iostream>

// Concerne uniquement les grosses entitées (Planettes, étoiles, lunes)
struct MassiveBodies {

    // Correspond au buffer que l'on fera tourner
    struct Dynamic {
        // Position X, Y, Z // Changement à chaque tour
        // Vecteurs Vx, Vy, Vz // Changement à chaque tour
    };

    // Correspond aux informations dont on a besoin côté back et non front
    struct Static {
        // Nom
        // Masse
        // Rayon
        // Type
        // Texture
    };

};