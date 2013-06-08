/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylandxcompositeeglintegration.h"

#include "qwaylandxcompositeeglwindow.h"

#include <QtCore/QDebug>

#include "wayland-xcomposite-client-protocol.h"

QT_USE_NAMESPACE

QWaylandGLIntegration * QWaylandGLIntegration::createGLIntegration(QWaylandDisplay *waylandDisplay)
{
    return new QWaylandXCompositeEGLIntegration(waylandDisplay);
}

QWaylandXCompositeEGLIntegration::QWaylandXCompositeEGLIntegration(QWaylandDisplay * waylandDisplay)
    : QWaylandGLIntegration()
    , mWaylandDisplay(waylandDisplay)
{
    qDebug() << "Using XComposite-EGL";
    waylandDisplay->addRegistryListener(&wlDisplayHandleGlobal, this);
}

QWaylandXCompositeEGLIntegration::~QWaylandXCompositeEGLIntegration()
{
    XCloseDisplay(mDisplay);
}

void QWaylandXCompositeEGLIntegration::initialize()
{
}

QWaylandWindow * QWaylandXCompositeEGLIntegration::createEglWindow(QWindow *window)
{
    return new QWaylandXCompositeEGLWindow(window,this);
}

QPlatformOpenGLContext *QWaylandXCompositeEGLIntegration::createPlatformOpenGLContext(const QSurfaceFormat &glFormat, QPlatformOpenGLContext *share) const
{
    return new QWaylandXCompositeEGLContext(glFormat, share, eglDisplay());
}

Display * QWaylandXCompositeEGLIntegration::xDisplay() const
{
    return mDisplay;
}

EGLDisplay QWaylandXCompositeEGLIntegration::eglDisplay() const
{
    return mEglDisplay;
}

int QWaylandXCompositeEGLIntegration::screen() const
{
    return mScreen;
}

Window QWaylandXCompositeEGLIntegration::rootWindow() const
{
    return mRootWindow;
}

QWaylandDisplay * QWaylandXCompositeEGLIntegration::waylandDisplay() const
{
    return mWaylandDisplay;
}
qt_xcomposite * QWaylandXCompositeEGLIntegration::waylandXComposite() const
{
    return mWaylandComposite;
}

const struct qt_xcomposite_listener QWaylandXCompositeEGLIntegration::xcomposite_listener = {
    QWaylandXCompositeEGLIntegration::rootInformation
};

void QWaylandXCompositeEGLIntegration::wlDisplayHandleGlobal(void *data, wl_registry *registry, uint32_t id, const QString &interface, uint32_t version)
{
    Q_UNUSED(version);
    if (interface == "wl_xcomposite") {
        QWaylandXCompositeEGLIntegration *integration = static_cast<QWaylandXCompositeEGLIntegration *>(data);
        integration->mWaylandComposite = static_cast<struct qt_xcomposite *>(wl_registry_bind(registry,id,&qt_xcomposite_interface,1));
        qt_xcomposite_add_listener(integration->mWaylandComposite,&xcomposite_listener,integration);
    }

}

void QWaylandXCompositeEGLIntegration::rootInformation(void *data, qt_xcomposite *xcomposite, const char *display_name, uint32_t root_window)
{
    Q_UNUSED(xcomposite);
    QWaylandXCompositeEGLIntegration *integration = static_cast<QWaylandXCompositeEGLIntegration *>(data);

    integration->mDisplay = XOpenDisplay(display_name);
    integration->mRootWindow = (Window) root_window;
    integration->mScreen = XDefaultScreen(integration->mDisplay);
    integration->mEglDisplay = eglGetDisplay(integration->mDisplay);
    eglBindAPI(EGL_OPENGL_ES_API);
    EGLint minor,major;
    if (!eglInitialize(integration->mEglDisplay,&major,&minor)) {
        qFatal("Failed to initialize EGL");
    }
    eglSwapInterval(integration->eglDisplay(),0);
    qDebug() << "ROOT INFORMATION" << integration->mDisplay << integration->mRootWindow << integration->mScreen;
}

