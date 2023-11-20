#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "zapfileexplorer.h"
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QApplication>
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);


    qmlRegisterType<ZapFileExplorer>("ZapFileExplorer", 1, 0, "ZapFileExplorer");
    qmlRegisterType<AudioEditorItem>("AudioEditorItem", 1, 0, "AudioEditorItem");

    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("ZAPEDIT", "Main");

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, [&](){
        QObject *rootObject = engine.rootObjects().first();
        QQuickWindow *window = qobject_cast<QQuickWindow *>(rootObject);
        ZapFileExplorer::activeWindow = window;
    });
    return app.exec();
}
