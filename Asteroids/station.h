/***************************************************************************
 *   Copyright (C) 2009 by Richard Crook                                   *
 *   richard@dazzle.plus.com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef STATION_H
#define STATION_H

#include <QGraphicsItem>

/*************************************************************************************/
/******************* Represents a radio station in the simulation ********************/
/*************************************************************************************/

class Station : public QGraphicsItem
{
public:
  Station( qreal, qreal );                          // constructor
  Station();
  void     paint( QPainter*,
                  const QStyleOptionGraphicsItem*,
                  QWidget* );                       // implement virtual paint function for Ship
  QRectF   boundingRect() const
    { return QRectF(-46.5, -53, 73, 58); }           // implement virtual boundingRect
    //{ return QRectF(-6.5, -16.5, 13, 18); }           // implement virtual boundingRect
  int stationRotation;
private:
  qreal xPos;
  qreal yPos;
  qreal xMove;
  qreal yMove;

};

#endif  // STATION_H