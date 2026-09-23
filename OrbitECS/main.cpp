#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QtGui/qguiapplication.h>
#include <memory>
#include <thread>

#include "BackManager.h"
#include "BodyLoader.h"
#include "BufferExchange.h"
#include "FrontManager.h"

#include <iostream>

#include "DebugPrint.h"
#include <filesystem>

// Ici il faudrait que j'instancie front et back dans des uniqueptr
int main(int argc, char *argv[]) {
  QGuiApplication app(argc, argv);

  std::cout << "Working directory: "

            << std::filesystem::current_path()

            << std::endl;

  LoadedBodies loaded =
      BodyLoader::load("../../Thinking/solar_system_initial_state.csv");

  DebugPrint::printLoadedBodies(loaded);

  // Attention à l'odre, dans l'instanciation de back, on utilise un move.
  auto exchange = std::make_unique<BufferExchange>(loaded.heavy.dynamic_,
                                                   loaded.light.dynamic_);
  auto front =
      std::make_unique<FrontManager>(*exchange.get(), std::move(loaded.meta));
  auto back = std::make_unique<BackManager>(
      std::move(loaded.heavy), std::move(loaded.light), *exchange,
      30 /*dt en seconde*/, 720000 /*simSpeedFactor*/);

  // // Start le back (thread)
  std::thread backThread([&back] { back->run(); });

  // Frontend QML

  QQmlApplicationEngine engine;

  // Expose FrontManager au QML avant le chargement de Main.qml, pour que
  // "frontManager.heavyInstancing" etc. soient résolubles dès la création
  // de la scène.
  engine.rootContext()->setContextProperty("frontManager", front.get());

  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
      [] { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
  engine.loadFromModule("OrbitECS", "Main");

  // Démarre la boucle de rendu front (tick à ~60 fps) une fois la scène
  // chargée ; le QTimer ne se déclenchera qu'au premier passage de la boucle
  // d'événements, donc l'ordre par rapport à app.exec() n'a pas d'importance
  // tant que c'est avant.
  front->start();

  const int rc = app.exec();

  // Stop le back
  back->stop();
  backThread.join();

  return rc;
}