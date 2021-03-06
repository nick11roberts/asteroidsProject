#include "asteroid.h"
#include "bullet.h"
#include "mainwindow.h"
#include "scene.h"
#include "station.h"
#include "commandbulletadd.h"
#include "commandbulletdelete.h"
#include "commandbulletmove.h"
#include "commandasteroidadd.h"
#include "commandasteroiddelete.h"
#include "commandasteroidmove.h"
#include "commandstationadd.h"
#include "commandstationdelete.h"
#include "commandstationmove.h"
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QUndoStack>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QTimer>
#include <QKeyEvent>
#include <QtCore/qmath.h>

#define PI 3.14159265

qreal           stationX = 0.5*WINDOW_WIDTH;
qreal           stationY = 0.45*WINDOW_HEIGHT;
qreal           asteroidX = 0.75*WINDOW_WIDTH;
qreal           asteroidY = 0.75*WINDOW_HEIGHT;
qreal           bulletX = 0.25*WINDOW_WIDTH;
qreal           bulletY = 0.25*WINDOW_HEIGHT;
qreal           stationMoveX = 0;
qreal           stationMoveY = 0;
/*************************************************************************************/
/******************** Scene representing the simulated landscape *********************/
/*************************************************************************************/

/************************************ constuctor *************************************/

Scene::Scene( QUndoStack* undoStack ) : QGraphicsScene()
{
  // create timer
  QTimer *timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(manageObjects()));
  //connect(timer, SIGNAL(timeout()), this, SLOT(collisionDetection()));
  timer->start(1000/25); // fire the timer 25 times per second

  // initialise variables
  m_undoStack     = undoStack;

  // create invisible item to provide default top-left anchor to scene
  addLine( 0, 0, 0, 1, QPen(Qt::transparent, 1) );

  // connect selectionChanged signal to selectStations slot
  connect( this, SIGNAL(selectionChanged()), this, SLOT(selectStations()) );

  // set local variables and check if existing station clicked
  qreal           stationX = 0.5*WINDOW_WIDTH;
  qreal           stationY = 0.45*WINDOW_HEIGHT;
  qreal           asteroidX = 0.75*WINDOW_WIDTH;
  qreal           asteroidY = 0.75*WINDOW_HEIGHT;
  qreal           bulletX = 0.25*WINDOW_WIDTH;
  qreal           bulletY = 0.25*WINDOW_HEIGHT;
  qreal           asteroidMoveX = -1;
  qreal           asteroidMoveY = -1;
  qreal           xDest, yDest;

  Station*  station = dynamic_cast<Station*>( itemAt( stationX, stationY ) );
  Asteroid*  asteroid = dynamic_cast<Asteroid*>( itemAt( asteroidX, asteroidY) );
  Bullet*  bullet = dynamic_cast<Bullet*>( itemAt( bulletX, bulletY ) );

  m_undoStack->push( new CommandStationAdd( this, stationX, stationY ) );
  m_undoStack->push( new CommandAsteroidAdd( this, asteroidX, asteroidY ) );
  m_undoStack->push( new CommandBulletAdd( this, bulletX, bulletY ) );
  emit message( QString("Ship add at %1,%2").arg(stationX).arg(stationY) );
  emit message( QString("Asteroid add at %1,%2").arg(asteroidX).arg(asteroidY) );
  emit message( QString("Bullet add at %1,%2").arg(bulletX).arg(bulletY) );

  /*
  asteroid = dynamic_cast<Asteroid*>( itemAt( asteroidX, asteroidY ));
  m_undoStack->push( new CommandAsteroidDelete( this, asteroid ) );

  dynamic_cast<Asteroid*>(itemAt(50,300));
  m_undoStack->push( new CommandAsteroidAdd( this, 50, 300 ) );
  */
}

/********************************** manageObjects ***********************************/

void  Scene::manageObjects()
{
  qreal asteroidMoveX = 2;
  qreal asteroidMoveY = 3;
  qreal bulletMoveX = 4;
  qreal bulletMoveY = 6;
  qreal xDest;
  qreal yDest;

  //***ASTEROID:

  //Calculate new location
  xDest = asteroidX + asteroidMoveX;
  yDest = asteroidY + asteroidMoveY;

  //If asteroid moves off screen, wrap to other side
  if(xDest < 0)             xDest += WINDOW_WIDTH - 1;
  if(xDest > WINDOW_WIDTH)  xDest -= WINDOW_WIDTH;
  if(yDest < 0)             yDest += WINDOW_HEIGHT - 1;
  case Qt::Key_A:  if(yDest > WINDOW_HEIGHT) yDest -= WINDOW_HEIGHT;

  //Delete asteroid from old position
  Asteroid* asteroid = dynamic_cast<Asteroid*>(itemAt(asteroidX, asteroidY));
  m_undoStack->push(new CommandAsteroidDelete(this, asteroid));

  //Draw asteroid at new position
  dynamic_cast<Asteroid*>(itemAt(xDest, yDest));
  m_undoStack->push(new CommandAsteroidAdd(this, xDest, yDest));

  //Update position to new position
  asteroidX = xDest;
  asteroidY = yDest;

  //***BULLET:

  //Calculate new location
  xDest = bulletX + bulletMoveX;
  yDest = bulletY + bulletMoveY;

  //If bullet moves off screen, wrap to other side
  if(xDest < 0)             xDest += WINDOW_WIDTH - 1;
  if(xDest > WINDOW_WIDTH)  xDest -= WINDOW_WIDTH;
  if(yDest < 0)             yDest += WINDOW_HEIGHT - 1;
  if(yDest > WINDOW_HEIGHT) yDest -= WINDOW_HEIGHT;

  //Delete bullet from old position
  Bullet* bullet = dynamic_cast<Bullet*>(itemAt(bulletX, bulletY));
  m_undoStack->push(new CommandBulletDelete(this, bullet));

  //Draw bullet at new position
  dynamic_cast<Bullet*>(itemAt(xDest, yDest));
  m_undoStack->push(new CommandBulletAdd(this, xDest, yDest));

  //Update position to new position
  bulletX = xDest;
  bulletY = yDest;

  //***SHIP:
  //Calculate new location
  xDest = stationX + stationMoveX;
  yDest = stationY + stationMoveY;

  //If ship moves off screen, wrap to other side
  if(xDest < 0)             xDest += WINDOW_WIDTH - 1;
  if(xDest > WINDOW_WIDTH)  xDest -= WINDOW_WIDTH;
  if(yDest < 0)             yDest += WINDOW_HEIGHT - 1;
  if(yDest > WINDOW_HEIGHT) yDest -= WINDOW_HEIGHT;

  //Delete ship from old position
  Station* station = dynamic_cast<Station*>(itemAt(stationX, stationY));
  m_undoStack->push(new CommandStationDelete(this, station));

  //Draw ship at new position
  dynamic_cast<Station*>(itemAt(xDest, yDest));
  m_undoStack->push(new CommandStationAdd(this, xDest, yDest));

  //Update position to new position
  stationX = xDest;
  stationY = yDest;

}

/********************************** mousePressEvent **********************************/

void  Scene::mousePressEvent( QGraphicsSceneMouseEvent* event )
{
  /*// set local variables and check if existing station clicked
  qreal           x = event->scenePos().x();
  qreal           y = event->scenePos().y();
  Station*  station = dynamic_cast<Station*>( itemAt( x, y ) );

  // if station not clicked and right mouse button pressed, create new Station
  if ( station == 0 && event->button() == Qt::LeftButton )
  {
    m_undoStack->push( new CommandStationAdd( this, x, y ) );
    emit message( QString("Station add at %1,%2").arg(x).arg(y) );
  }

  // call base mousePressEvent to handle other mouse press events such as selecting
  QGraphicsScene::mousePressEvent( event );*/
}

/********************************* contextMenuEvent **********************************/

void  Scene::contextMenuEvent( QGraphicsSceneContextMenuEvent* event )
{
  // we only want to display a menu if user clicked a station
  qreal     x       = event->scenePos().x();
  qreal     y       = event->scenePos().y();
  Station*  station = dynamic_cast<Station*>( itemAt( x, y ) );
  if ( station == 0 ) return;

  // display context menu and action accordingly
  QMenu     menu;
  QAction*  deleteAction = menu.addAction("Delete Station");
  if ( menu.exec( event->screenPos() ) == deleteAction )
  {
    m_undoStack->push( new CommandStationDelete( this, station ) );
    emit message( QString("Station deleted at %1,%2").arg(x).arg(y) );
  }
}

/********************************** selectStations ***********************************/

void  Scene::selectStations()
{
  // refresh record of selected stations and their starting positions
  m_stations.clear();
  foreach( QGraphicsItem* item, selectedItems() )
    if ( dynamic_cast<Station*>( item ) )
    m_stations.append( qMakePair( dynamic_cast<Station*>( item ), item->pos() ) );
}

/********************************* mouseReleaseEvent *********************************/

void  Scene::mouseReleaseEvent( QGraphicsSceneMouseEvent* event )
{
  // if any stations moved, then create undo commands
  foreach( StationPos station , m_stations )
    if ( station.first->pos() != station.second )
      m_undoStack->push( new CommandStationMove( station.first,
                             station.second.x(), station.second.y(),
                             station.first->x(), station.first->y() ) );

  // refresh record of selected stations and call base mouseReleaseEvent
  selectStations();
  QGraphicsScene::mouseReleaseEvent( event );
}

/************************************ writeStream ************************************/

void  Scene::writeStream( QXmlStreamWriter* stream )
{
  // write station data to xml stream
  foreach( QGraphicsItem*  item, items() )
  {
    Station*  station = dynamic_cast<Station*>( item );
    if ( station )
    {
      stream->writeEmptyElement( "station" );
      stream->writeAttribute( "x", QString("%1").arg(station->x()) );
      stream->writeAttribute( "y", QString("%1").arg(station->y()) );
    }
  }
}

/************************************ readStream *************************************/

void  Scene::readStream( QXmlStreamReader* stream )
{
  // read station data from xml stream
  while ( !stream->atEnd() )
  {
    stream->readNext();
    if ( stream->isStartElement() && stream->name() == "station" )
    {
      qreal x = 0.0, y = 0.0;
      foreach( QXmlStreamAttribute attribute, stream->attributes() )
      {
        if ( attribute.name() == "x" ) x = attribute.value().toString().toDouble();
        if ( attribute.name() == "y" ) y = attribute.value().toString().toDouble();
      }
      addItem( new Station( x, y ) );
    }
  }
}

void Scene:: keyPressEvent(QKeyEvent *event)
{

    switch(event->key())
    {
        case Qt::Key_A:

        {
            stationRotation += 5;
            if(stationRotation > 359) stationRotation -= 360;
        }

        case Qt::Key_D:

        {
            stationRotation += 5;
            if(stationRotation < 0) stationRotation += 360;
        }

        case Qt::Key_W:

        {
            stationMoveX += 2 * qCos(stationRotation * PI / 180);
            stationMoveY += 2 * qSin(stationRotation * PI / 180);
        }
    }
}


