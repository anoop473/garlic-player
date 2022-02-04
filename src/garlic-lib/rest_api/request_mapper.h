#ifndef REQUEST_MAPPER_H
#define REQUEST_MAPPER_H

#include <httprequesthandler.h>
#include "tools/main_configuration.h"
#include "v2/auth_controller.h"
#include "v2/systeminformation_controller.h"

using namespace stefanfrings;

class RequestMapper : public HttpRequestHandler
{
        Q_OBJECT
    public:
        RequestMapper(MainConfiguration *mc, QObject* parent = Q_NULLPTR);
        void service(HttpRequest& request, HttpResponse& response);
    private:
        MainConfiguration *MyConfiguration;
        AuthController MyAuthController;
        SystemInformationController MySIController;
        void respond(HttpResponse& response, QString json);
        void responseNotFound(HttpResponse& response);
        void responseAccessViolation(HttpResponse& response);


};


#endif // REQUEST_MAPPER_H
