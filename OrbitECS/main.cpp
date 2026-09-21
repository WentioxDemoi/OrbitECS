#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtGui/qguiapplication.h>
#include <memory>
#include "BodyLoader.h"
#include "BufferExchange.h"
#include "FrontManager.h"
#include "BackManager.h"

// Ici il faudrait que j'instancie front et back dans des uniqueptr
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    LoadedBodies loaded = BodyLoader::load("data/bodies.xlsx");

    auto exchange = std::make_unique<BufferExchange>(loaded.heavy.dynamic_, loaded.light.dynamic_);
    auto front    = std::make_unique<FrontManager>(exchange.get(), std::move(loaded.meta));
    auto back     = std::make_unique<BackManager>(std::move(loaded.heavy), std::move(loaded.light), exchange.get());
    
    // Start le back (thread)
    // TODO

    // Start le front
    const int rc = app.exec();

    // Stop le back
    // TODO

    return rc;
    
}
