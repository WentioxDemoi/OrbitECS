#ifndef BODYMETADATA_H
#define BODYMETADATA_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <QMetaType>

struct BodyMetaData {
    std::string   name;
    double        mass;
    std::string text;
    // Potentiellement d'autres choses par la suite pour affichage scientifique.
    // Pour l'instant la majorité des infos sera dans le texte.
};

#endif