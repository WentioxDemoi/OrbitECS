#include "BodyLoader.h"

#include <fstream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <chrono>
#include <sstream>
#include <utility>

namespace {

using ColIndex = std::unordered_map<std::string, std::size_t>;

std::vector<std::string> splitCsvLine(const std::string &line) {
  std::vector<std::string> fields;
  std::string current;
  bool inQuotes = false;

  for (std::size_t i = 0; i < line.size(); ++i) {
    const char c = line[i];
    if (inQuotes) {
      if (c == '"') {
        if (i + 1 < line.size() && line[i + 1] == '"') {
          current += '"';
          ++i;
        } else {
          inQuotes = false;
        }
      } else {
        current += c;
      }
    } else if (c == '"') {
      inQuotes = true;
    } else if (c == ',') {
      fields.push_back(std::move(current));
      current.clear();
    } else {
      current += c;
    }
  }
  fields.push_back(std::move(current));
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

// --- Heavy ---------------------------------------------------------------

struct HeavyLoad {
  HeavyBodies bodies;
  std::vector<BodyMetaData> meta;
  std::chrono::system_clock::time_point epoch;
};

HeavyLoad loadHeavy(std::string_view path) {
  auto file = openCsv(path);

  std::size_t nCols = 0;
  const auto col =
      readHeader(file, path,
                 {"name", "x_km", "y_km", "z_km", "vx_km_s", "vy_km_s",
                  "vz_km_s", "mass_kg", "gm_km3_s2", "radius_km", "text"},
                 nCols);

  std::vector<std::string> names, text;
  std::vector<double> mass, gm, x, y, z, vx, vy, vz, radius;

  std::chrono::system_clock::time_point epoch;
  bool isEpoch = false;

  std::string line;

  while (getCsvLine(file, line)) {
    if (line.empty())
      continue;
    const auto fields = splitCsvLine(line);
    if (fields.size() < nCols)
      continue;

    if (!isEpoch) {
        std::istringstream iss(fields[col.at("epoch")]);
        std::tm tm{};
        iss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
        if (iss.fail()) { /* erreur */ }
        epoch = std::chrono::system_clock::from_time_t(timegm(&tm));
        isEpoch = true;
    }

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
    text.push_back(fields[col.at("text")]);
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
        BodyMetaData{names[i], mass[i], text[i], radius[i]});
  }

  return {std::move(heavy), std::move(meta), std::move(epoch)};
}

// --- Light ---------------------------------------------------------------

LightBodies loadLight(std::string_view path) {
  auto file = openCsv(path);

  // Seuls l'état (position/vitesse) est requis : masse, rayon, H... sont
  // souvent vides pour les astéroïdes et inutiles pour des corps "test".
  std::size_t nCols = 0;
  const auto col = readHeader(
      file, path, {"x_km", "y_km", "z_km", "vx_km_s", "vy_km_s", "vz_km_s"},
      nCols);

  std::vector<double> x, y, z, vx, vy, vz;

  std::string line;

  while (getCsvLine(file, line)) {
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

LoadedBodies load(std::string_view heavyPath, std::string_view lightPath) {
  auto heavy = loadHeavy(heavyPath);
  auto light = loadLight(lightPath);
  return LoadedBodies{std::move(heavy.bodies), std::move(light),
                      std::move(heavy.meta), std::move(heavy.epoch)};
}

} // namespace BodyLoader