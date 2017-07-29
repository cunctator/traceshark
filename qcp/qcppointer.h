/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QCPPOINTER_H
#define QCPPOINTER_H

//#include <QtCore/qsharedpointer.h>
#include <QWeakPointer>

//QT_BEGIN_NAMESPACE

//QT_MODULE(Core)

template <class T>
class QCPPointer
{
    QWeakPointer<T> wp;

public:
    inline QCPPointer() : wp() { }
    inline QCPPointer(T *p) : wp(p) { }
    inline QCPPointer(const QCPPointer<T> &p) : wp(p.wp) { }
    inline ~QCPPointer() { }

    inline QCPPointer<T> &operator=(const QCPPointer<T> &p)
    { wp = p.wp; return *this; }
    inline QCPPointer<T> &operator=(T* p)
    { wp = p; return *this; }

    inline bool isNull() const
    { return wp.isNull(); }

    inline T* operator->() const
    { return wp.data(); }
    inline T& operator*() const
    { return *wp.data(); }
    inline operator T*() const
    { return wp.data(); }
    inline T* data() const
    { return wp.data(); }
};


template <class T>
inline bool operator==(const T *o, const QCPPointer<T> &p)
{ return o == p.operator->(); }

template<class T>
inline bool operator==(const QCPPointer<T> &p, const T *o)
{ return p.operator->() == o; }

template <class T>
inline bool operator==(T *o, const QCPPointer<T> &p)
{ return o == p.operator->(); }

template<class T>
inline bool operator==(const QCPPointer<T> &p, T *o)
{ return p.operator->() == o; }

template<class T>
inline bool operator==(const QCPPointer<T> &p1, const QCPPointer<T> &p2)
{ return p1.operator->() == p2.operator->(); }

template <class T>
inline bool operator!=(const T *o, const QCPPointer<T> &p)
{ return o != p.operator->(); }

template<class T>
inline bool operator!= (const QCPPointer<T> &p, const T *o)
{ return p.operator->() != o; }

template <class T>
inline bool operator!=(T *o, const QCPPointer<T> &p)
{ return o != p.operator->(); }

template<class T>
inline bool operator!= (const QCPPointer<T> &p, T *o)
{ return p.operator->() != o; }

template<class T>
inline bool operator!= (const QCPPointer<T> &p1, const QCPPointer<T> &p2)
{ return p1.operator->() != p2.operator->() ; }

//QT_END_NAMESPACE

#endif // QCPPOINTER_H
