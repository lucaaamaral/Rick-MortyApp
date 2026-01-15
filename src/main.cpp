#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <memory>
#include <glog/logging.h>

#include "core/ApiClient.h"
#include "core/DataStore.h"
#include "ui/QmlBridge.h"

int main(int argc, char *argv[])
{
    // Initialize Google Logging
    google::InitGoogleLogging(argv[0]);
    FLAGS_logtostderr = true;  // Log to stderr instead of files
    FLAGS_colorlogtostderr = true;  // Colorized output
    FLAGS_v = 1;  // Verbose level

    LOG(INFO) << "Starting Rick and Morty Explorer";

    QApplication app(argc, argv);
    app.setApplicationName("Rick and Morty Explorer");
    app.setOrganizationName("RickAndMorty");
    app.setApplicationVersion("1.0.0");

    // Set the style
    QQuickStyle::setStyle("Basic");

    // Create the backend components
    auto apiClient = std::make_unique<rickmorty::ApiClient>();
    auto dataStore = std::make_unique<rickmorty::DataStore>(std::move(apiClient));

    // Create the QML bridge
    QmlBridge bridge(dataStore.get());

    // Set up QML engine
    QQmlApplicationEngine engine;

    // Add import path for our QML modules
    engine.addImportPath(":/qml");
    engine.addImportPath("qrc:/qml");

    // Expose the bridge to QML
    engine.rootContext()->setContextProperty("backend", &bridge);

    // Load the main QML file
    const QUrl url(QStringLiteral("qrc:/qml/Main.qml"));

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
