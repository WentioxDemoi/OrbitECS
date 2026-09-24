#include "BodyLoader.h"

#include <fstream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace {

using ColIndex = std::unordered_map<std::string, std::size_t>;

// Split qui conserve les champs vides (y compris le dernier).
std::vector<std::string> splitCsvLine(const std::string &line) {
  std::vector<std::string> fields;
  std::size_t start = 0;
  while (true) {
    const auto pos = line.find(',', start);
    if (pos == std::string::npos) {
      fields.push_back(line.substr(start));
      break;
    }
    fields.push_back(line.substr(start, pos - start));
    start = pos + 1;
  }
  return fields;
}

// getline qui retire le '\r' des fichiers CRLF.
bool getCsvLine(std::istream &in, std::string &line) {
  if (!std::getline(in, line))
    return false;
  if (!line.empty() && line.back() == '\r')
    line.pop_back();
  return true;
}

std::ifstream openCsv(std::string_view path) {
  std::ifstream file{std::string(path)};
  if (!file.is_open()) {
    throw std::runtime_error("BodyLoader - impossible d'ouvrir " +
                             std::string(path));
  }
  return file;
}

// Lit l'en-tête, construit colonne -> index et vérifie les colonnes requises.
ColIndex readHeader(std::ifstream &file, std::string_view path,
                    const std::vector<std::string> &required,
                    std::size_t &columnCount) {
  std::string headerLine;
  if (!getCsvLine(file, headerLine)) {
    throw std::runtime_error("BodyLoader - fichier vide : " +
                             std::string(path));
  }

  const auto headers = splitCsvLine(headerLine);
  columnCount = headers.size();

  ColIndex colIndex;
  for (std::size_t i = 0; i < headers.size(); ++i) {
    colIndex[headers[i]] = i;
  }
  for (const auto &col : required) {
    if (colIndex.find(col) == colIndex.end()) {
      throw std::runtime_error("BodyLoader - colonne manquante '" + col +
                               "' dans " + std::string(path));
    }
  }
  return colIndex;
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

// --- Heavy ---------------------------------------------------------------

struct HeavyLoad {
  HeavyBodies bodies;
  std::vector<BodyMetaData> meta;
};

HeavyLoad loadHeavy(std::string_view path) {
  auto file = openCsv(path);

  std::size_t nCols = 0;
  const auto col = readHeader(file, path,
                              {"name", "x_km", "y_km", "z_km", "vx_km_s",
                               "vy_km_s", "vz_km_s", "mass_kg", "gm_km3_s2",
                               "radius_km"},
                              nCols);

  std::vector<std::string> names;
  std::vector<double> mass, gm, x, y, z, vx, vy, vz, radius;

  std::string line;
  while (getCsvLine(file, line)) {
    if (line.empty())
      continue;
    const auto fields = splitCsvLine(line);
    if (fields.size() < nCols)
      continue;

    names.push_back(fields[col.at("name")]);
    gm.push_back(std::stod(fields[col.at("gm_km3_s2")]));
    x.push_back(std::stod(fields[col.at("x_km")]));
    y.push_back(std::stod(fields[col.at("y_km")]));
    z.push_back(std::stod(fields[col.at("z_km")]));
    vx.push_back(std::stod(fields[col.at("vx_km_s")]));
    vy.push_back(std::stod(fields[col.at("vy_km_s")]));
    vz.push_back(std::stod(fields[col.at("vz_km_s")]));
    mass.push_back(std::stod(fields[col.at("mass_kg")]));
    radius.push_back(std::stod(fields[col.at("radius_km")]));
  }

  const std::size_t count = names.size();

  HeavyBodies heavy(count);
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

    meta.push_back(
        BodyMetaData{names[i], mass[i], buildText(names[i], rng), radius[i]});
  }

  return {std::move(heavy), std::move(meta)};
}

// --- Light ---------------------------------------------------------------

LightBodies loadLight(std::string_view path, int nb_asteroids) {
  auto file = openCsv(path);

  // Seuls l'état (position/vitesse) est requis : masse, rayon, H... sont
  // souvent vides pour les astéroïdes et inutiles pour des corps "test".
  std::size_t nCols = 0;
  const auto col = readHeader(
      file, path, {"x_km", "y_km", "z_km", "vx_km_s", "vy_km_s", "vz_km_s"},
      nCols);

  std::vector<double> x, y, z, vx, vy, vz;

  std::string line;
  int i = 0;

  while (getCsvLine(file, line) && nb_asteroids > i) {
    if (line.empty())
      continue;
    const auto fields = splitCsvLine(line);
    if (fields.size() < nCols)
      continue;

    x.push_back(std::stod(fields[col.at("x_km")]));
    y.push_back(std::stod(fields[col.at("y_km")]));
    z.push_back(std::stod(fields[col.at("z_km")]));
    vx.push_back(std::stod(fields[col.at("vx_km_s")]));
    vy.push_back(std::stod(fields[col.at("vy_km_s")]));
    vz.push_back(std::stod(fields[col.at("vz_km_s")]));
    i++;
  }

  LightBodies light(x.size()); // ax/ay/az initialisés à 0 par le constructeur
  light.dynamic_.x = std::move(x);
  light.dynamic_.y = std::move(y);
  light.dynamic_.z = std::move(z);
  light.vx = std::move(vx);
  light.vy = std::move(vy);
  light.vz = std::move(vz);
  return light;
}

} // namespace

namespace BodyLoader {

LoadedBodies load(std::string_view heavyPath, std::string_view lightPath, int nb_asteroids) {
  auto heavy = loadHeavy(heavyPath);
  auto light = loadLight(lightPath, nb_asteroids);
  return LoadedBodies{std::move(heavy.bodies), std::move(light),
                      std::move(heavy.meta)};
}

} // namespace BodyLoader