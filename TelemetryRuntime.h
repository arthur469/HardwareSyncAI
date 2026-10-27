#pragma once
#include <QObject>
#include <future>
#include <set>
#include <QProcess>
#include <QTimer>
#include <QElapsedTimer>
#include <QMap>
#include <QSet>
#include <nlohmann/json.hpp>
#include "RGBControllerInterface.h"
using json=nlohmann::json;
class TelemetryRuntime: public QObject {
public:
    explicit TelemetryRuntime(QObject* parent=nullptr);
    ~TelemetryRuntime();
    void start();
    void shutdown();
    double value(const std::string& key) const;
    bool modelKnown(const std::string& key) const;
    bool readControl();
    bool setPause(bool pause,const std::string& reason);
    bool beginFrame();
    void endFrame();
    bool eligible(RGBControllerInterface* controller,bool native=false);
    void remember(RGBControllerInterface* controller,bool wrote=true);
    void clearControllerState();
    bool claim(RGBControllerInterface* controller,unsigned zone,const void* owner);
    void assign(const void* owner,const std::string& name,const std::vector<std::pair<RGBControllerInterface*,unsigned>>& zones);
    std::string conflict(RGBControllerInterface* controller,unsigned zone,const void* owner) const;
    std::string state(const std::string& name) const;
    void release(const void* owner);
    json status() const;
    void record(const std::string& measure,const std::string& state);
    void forget(const std::string& name){states.erase(name);}
    json configuration;
    bool paused=true;
    std::string pauseReason="Waiting for valid control state";
    std::string generation;
    quint64 writes=0;
private:
    QProcess process;
    QTimer watchdog;
    std::future<void> statusWrite;
    QByteArray buffer;
    json snapshot=json::object();
    qint64 received=0;
    std::string collectorError="Collector not started";
    std::map<RGBControllerInterface*,std::vector<RGBColor>> expected;
    std::set<RGBControllerInterface*> manual;
    std::map<std::pair<RGBControllerInterface*,unsigned>,const void*> owners;
    std::map<const void*,std::pair<std::string,std::vector<std::pair<RGBControllerInterface*,unsigned>>>> assignments;
    std::map<std::string,std::string> states;
    void* lockHandle=nullptr;
    bool configured=false;
};
TelemetryRuntime& telemetryRuntime();

