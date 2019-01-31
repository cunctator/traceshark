// SPDX-License-Identifier: (LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only)
/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QCPPOINTER_IMPL_H
#define QCPPOINTER_IMPL_H

#include <QWeakPointer>
#include <QSharedPointer>
#include <QObject>
#include <type_traits>

class QVariant;

template <class T>
class QCPPointer
{
    static_assert(!std::is_pointer<T>::value, "QCPPointer's template type must not be a pointer type");

    template<typename U>
    struct TypeSelector
    {
        typedef QObject Type;
    };
    template<typename U>
    struct TypeSelector<const U>
    {
        typedef const QObject Type;
    };
    typedef typename TypeSelector<T>::Type QObjectType;
    QWeakPointer<QObjectType> wp;

public:
    inline QCPPointer() { }
    inline QCPPointer(T *p) : wp(p) { }
    // compiler-generated copy/move ctor/assignment operators are fine!
    // compiler-generated dtor is fine!

#ifdef Q_QDOC
    // Stop qdoc from complaining about missing function
    ~QCPPointer();
#endif

    inline void swap(QCPPointer &other) { wp.swap(other.wp); }

    inline QCPPointer<T> &operator=(T* p)
    { wp = p; return *this; }

    inline T* data() const
    { return static_cast<T*>( wp.data()); }
    inline T* operator->() const
    { return data(); }
    inline T& operator*() const
    { return *data(); }
    inline operator T*() const
    { return data(); }

    inline bool isNull() const
    { return wp.isNull(); }

    inline void clear()
    { wp.clear(); }
};
template <class T> Q_DECLARE_TYPEINFO_BODY(QCPPointer<T>, Q_MOVABLE_TYPE);

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

#endif // QCPPOINTER_IMPL_H
