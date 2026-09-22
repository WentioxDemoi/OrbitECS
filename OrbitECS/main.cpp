#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtGui/qguiapplication.h>
#include <memory>
#include <thread>

#include "BodyLoader.h"
#include "BufferExchange.h"
#include "FrontManager.h"
#include "BackManager.h"

// Ici il faudrait que j'instancie front et back dans des uniqueptr
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    LoadedBodies loaded = BodyLoader::load("data/bodies.xlsx");

    // Attention à l'odre, dans l'instanciation de back, on utilise un move.
    auto exchange = std::make_unique<BufferExchange>(loaded.heavy.dynamic_, loaded.light.dynamic_);
    // auto front    = std::make_unique<FrontManager>(*exchange.get(), std::move(loaded.meta));
    auto back     = std::make_unique<BackManager>(std::move(loaded.heavy), std::move(loaded.light), *exchange, 5 /*dt en seconde*/);
    
    // Start le back (thread)
    std:std::thread backThread([&back] { back->run(); });

    // Start le front
    const int rc = app.exec();

    // Stop le back
    back->stop();
    backThread.join();

    return rc;
    
}
