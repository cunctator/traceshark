/***************************************************************************
**                                                                        **
**  QCustomPlot, an easy to use, modern plotting widget for Qt            **
**  Copyright (C) 2011-2016 Emanuel Eichhammer                            **
**                                                                        **
**  This program is free software: you can redistribute it and/or modify  **
**  it under the terms of the GNU General Public License as published by  **
**  the Free Software Foundation, either version 3 of the License, or     **
**  (at your option) any later version.                                   **
**                                                                        **
**  This program is distributed in the hope that it will be useful,       **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
**  GNU General Public License for more details.                          **
**                                                                        **
**  You should have received a copy of the GNU General Public License     **
**  along with this program.  If not, see http://www.gnu.org/licenses/.   **
**                                                                        **
****************************************************************************
**           Author: Emanuel Eichhammer                                   **
**  Website/Contact: http://www.qcustomplot.com/                          **
**             Date: 13.09.16                                             **
**          Version: 2.0.0-beta                                           **
****************************************************************************/

#ifndef QCP_PLOTTABLE1D_H
#define QCP_PLOTTABLE1D_H

#include "global.h"
#include "datacontainer.h"
#include "plottable.h"

class QCP_LIB_DECL QCPPlottableInterface1D
{
public:
  // introduced pure virtual methods:
  virtual int dataCount() const = 0;
  virtual double dataMainKey(int index) const = 0;
  virtual double dataSortKey(int index) const = 0;
  virtual double dataMainValue(int index) const = 0;
  virtual QCPRange dataValueRange(int index) const = 0;
  virtual QPointF dataPixelPosition(int index) const = 0;
  virtual bool sortKeyIsMainKey() const = 0;
  virtual QCPDataSelection selectTestRect(const QRectF &rect, bool onlySelectable) const = 0;
  virtual int findBegin(double sortKey, bool expandedRange=true) const = 0;
  virtual int findEnd(double sortKey, bool expandedRange=true) const = 0;
};

template <class DataType>
class QCP_LIB_DECL QCPAbstractPlottable1D : public QCPAbstractPlottable, public QCPPlottableInterface1D
{
  // No Q_OBJECT macro due to template class
  
public:
  QCPAbstractPlottable1D(QCPAxis *keyAxis, QCPAxis *valueAxis);
  virtual ~QCPAbstractPlottable1D();
  
  // virtual methods of 1d plottable interface:
  virtual int dataCount() const;
  virtual double dataMainKey(int index) const;
  virtual double dataSortKey(int index) const;
  virtual double dataMainValue(int index) const;
  virtual QCPRange dataValueRange(int index) const;
  virtual QPointF dataPixelPosition(int index) const;
  virtual bool sortKeyIsMainKey() const;
  virtual QCPDataSelection selectTestRect(const QRectF &rect, bool onlySelectable) const;
  virtual int findBegin(double sortKey, bool expandedRange=true) const;
  virtual int findEnd(double sortKey, bool expandedRange=true) const;
  
  // virtual methods:
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=0) const;
  virtual QCPPlottableInterface1D *interface1D() { return this; }
  
protected:
  // property members:
  QSharedPointer<QCPDataContainer<DataType> > mDataContainer;
  
  // helpers for subclasses:
  void getDataSegments(QList<QCPDataRange> &selectedSegments, QList<QCPDataRange> &unselectedSegments) const;
  void drawPolyline(QCPPainter *painter, const QVector<QPointF> &lineData) const;

private:
  Q_DISABLE_COPY(QCPAbstractPlottable1D)
  
};

// include implementation in header since it is a class template:
#include "plottable1d.cpp"

#endif // QCP_PLOTTABLE1D_H
