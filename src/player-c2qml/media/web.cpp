#include "web.h"

Web::Web(QQmlComponent *mc, QString r_id, Launcher *lc, QObject *parent) : PlayerBaseMedia(mc, r_id, lc, parent)
{
    QString str("import QtQuick 2.12\n \
                 import QtWebView 1.1\n \
                 WebView {\n \
                        id: "+getRegionId()+"_web;\n \
                        anchors.fill: parent;\n \
                   }\n"
    );
    web_item.reset(createMediaItem(mc, str));
}

Web::~Web()
{
}

void Web::loadMedia(BaseMedia *media, Region *reg)
{
    SmilMedia = media;
    region    = reg;
    web_item.data()->setVisible(true);
}

void Web::restart()
{
    stop();
    play();
}

void Web::play()
{
    QString uri = sanitizeUri(SmilMedia->getLoadablePath()) + SmilMedia->getParamsAsQuery();
    web_item.data()->setProperty("url", uri);
    web_item.data()->setVisible(true);
    if (SmilMedia->getLogContentId() != "")
        setStartTime();
}

void Web::stop()
{
    if (!SmilMedia->getLogContentId().isEmpty())
        qInfo(PlayLog).noquote() << createPlayLogXml();

    web_item.data()->setProperty("url", "");
    web_item.data()->setVisible(false);
}

void Web::resume()
{
    web_item.data()->setVisible(true);
}

void Web::pause()
{
     // todo add support for pauseDisplay
    web_item.data()->setVisible(false);
}

void Web::changeSize(int w, int h)
{
    Q_UNUSED(w);Q_UNUSED(h)
}

void Web::setParentItem(QQuickItem *parent)
{
    web_item.data()->setParentItem(parent);
}

QString Web::sanitizeUri(QString uri)
{
    if (uri.isEmpty())
        return "";
    // add file scheme if neccessary
    if (uri.at(0) == "/")
    {
        uri = "file://" + uri;
    }
    return uri;
}
