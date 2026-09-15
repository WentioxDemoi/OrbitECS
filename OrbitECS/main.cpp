#include <QGuiApplication>
#include <QQmlApplicationEngine>



// Ici il faudrait que j'instancie front et back dans des uniqueptr puis que je fasse des connects entre
// eux afin qu'ils puissent communiquer
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("OrbitECS", "Main");

    return app.exec();
}
