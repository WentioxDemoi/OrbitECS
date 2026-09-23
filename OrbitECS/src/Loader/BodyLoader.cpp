#include "BodyLoader.h"

#include <fstream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace {
std::vector<std::string> splitCsvLine(const std::string &line) {
  std::vector<std::string> fields;
  std::stringstream ss(line);
  std::string field;
  while (std::getline(ss, field, ',')) {
    fields.push_back(field);
  }
  return fields;
}

const std::vector<std::string> &randomPhrases() {
  static const std::vector<std::string> phrases = {
      "orbite silencieusement depuis des milliards d'années.",
      "danse au rythme de la gravité du système solaire.",
      "traverse le vide en suivant une trajectoire immuable.",
      "porte les cicatrices de son histoire cosmique.",
      "veille, discret, dans l'obscurité de l'espace."};
  return phrases;
}

std::string buildText(const std::string &name, std::mt19937 &rng) {
  const auto &phrases = randomPhrases();
  std::uniform_int_distribution<std::size_t> dist(0, phrases.size() - 1);
  return name + " " + phrases[dist(rng)];
}

} // namespace

namespace BodyLoader {

LoadedBodies load(std::string_view path) {
  std::ifstream file{std::string(path)};
  if (!file.is_open()) {
    throw std::runtime_error("BodyLoader::load - impossible d'ouvrir " +
                             std::string(path));
  }

  std::string headerLine;
  if (!std::getline(file, headerLine)) {
    throw std::runtime_error("BodyLoader::load - fichier vide : " +
                             std::string(path));
  }


  // Colonne -> index, pour être robuste à un réordonnancement du CSV
  const auto headers = splitCsvLine(headerLine);
  std::unordered_map<std::string, std::size_t> colIndex;
  for (std::size_t i = 0; i < headers.size(); ++i) {
    colIndex[headers[i]] = i;
  }

const std::vector<std::string> required = {
    "name", "x_km", "y_km", "z_km", "vx_km_s", "vy_km_s", "vz_km_s", "mass_kg", "gm_km3_s2", "radius_km\r"
};
  for (const auto &col : required) {
    if (colIndex.find(col) == colIndex.end()) {
      throw std::runtime_error("BodyLoader::load - colonne manquante : " + col);
    }
  }

  std::vector<std::string> names;
  std::vector<double> mass, gm, x, y, z, vx, vy, vz, radius;

  std::string line;
  while (std::getline(file, line)) {
    if (line.empty())
      continue;
    const auto fields = splitCsvLine(line);

    names.push_back(fields[colIndex.at("name")]);
    gm.push_back(std::stod(fields[colIndex.at("gm_km3_s2")]));
    x.push_back(std::stod(fields[colIndex.at("x_km")]));
    y.push_back(std::stod(fields[colIndex.at("y_km")]));
    z.push_back(std::stod(fields[colIndex.at("z_km")]));
    vx.push_back(std::stod(fields[colIndex.at("vx_km_s")]));
    vy.push_back(std::stod(fields[colIndex.at("vy_km_s")]));
    vz.push_back(std::stod(fields[colIndex.at("vz_km_s")]));
    mass.push_back(std::stod(fields[colIndex.at("mass_kg")]));
    radius.push_back(std::stod(fields[colIndex.at("radius_km\r")]));
  }

  const std::size_t count = names.size();

  HeavyBodies heavy(count);
  LightBodies light(0);
  std::vector<BodyMetaData> meta;
  meta.reserve(count);

  std::mt19937 rng{std::random_device{}()};

  for (std::size_t i = 0; i < count; ++i) {
    heavy.mass[i] = mass[i];
    heavy.gm[i] = gm[i];
    heavy.vx[i] = vx[i];
    heavy.vy[i] = vy[i];
    heavy.vz[i] = vz[i];
    heavy.ax[i] = 0.0;
    heavy.ay[i] = 0.0;
    heavy.az[i] = 0.0;
    heavy.name[i] = names[i];

    heavy.dynamic_.x[i] = x[i];
    heavy.dynamic_.y[i] = y[i];
    heavy.dynamic_.z[i] = z[i];

    meta.push_back(BodyMetaData{names[i], mass[i], buildText(names[i], rng), radius[i]});
  }

  return LoadedBodies{std::move(heavy), std::move(light), std::move(meta)};
}
} // namespace BodyLoader