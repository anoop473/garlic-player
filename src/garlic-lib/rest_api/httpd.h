#ifndef HTTPD_H
#define HTTPD_H

#include <QCoreApplication>
#include "httplistener.h"
#include "request_mapper.h"

using namespace stefanfrings;

class Httpd : public QObject
{
        Q_OBJECT
    public:
        explicit Httpd(MainConfiguration *mc, QObject *parent = nullptr);
        void init(QCoreApplication *app);
    private:
        MainConfiguration *MyConfiguration;
};

#endif // HTTPD_H
