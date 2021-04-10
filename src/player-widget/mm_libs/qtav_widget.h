#ifdef SUPPORT_QTAV

#ifndef QTAV_WIDGET_H
#define QTAV_WIDGET_H

#include <QtAV>
#include <QtAVWidgets>

class QtAVWidget : public QWidget
{
    Q_OBJECT
public:
    QtAVWidget(QWidget *parent = Q_NULLPTR);
    QWidget              *getVideoWidget();
    QtAV::VideoRenderer  *getVideoRenderer();
    void                  ignoreAspectRatio();
    void                  keepAspectRatio();
    void                  keepAspectRatioByExpanding();
signals:
protected:
    QScopedPointer<QtAV::VideoOutput> VideoWidget;  // a deleteLater leads to a crash on playlistchange!
};
#endif

#endif // QTAV_WIDGET_H
