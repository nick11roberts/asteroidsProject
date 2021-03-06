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
#include "asteroid.h"
#include "bullet.h"
#include "mainwindow.h"
#include "scene.h"
#include "station.h"
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QAction>
#include <QUndoStack>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QTimer>
#include <QTime>
#include <QList>
#include <QLabel>
#include <QKeyEvent>
#include <QtCore/qmath.h>
#include <iostream>
#include <cmath>
#include <QPainter>

using namespace std;
int stationRotation=90;

#define PI 3.14159265

Station *station;
Bullet *bullet;
Asteroid *asteroid, newAsteroid;

QList <Asteroid*> *asteroidList = new QList<Asteroid*>;
QList <Bullet*> *bulletList = new QList<Bullet*>;

int MAX_NUM_OF_ASTEROIDS = 5;
int MAX_NUM_OF_BULLETS = 50;





qreal           stationX = 0.5*WINDOW_WIDTH;
qreal           stationY = 0.45*WINDOW_HEIGHT;
qreal           asteroidX = 0.75*WINDOW_WIDTH;
qreal           asteroidY = 0.75*WINDOW_HEIGHT;
qreal           bulletX = 0.25*WINDOW_WIDTH;
qreal           bulletY = 0.25*WINDOW_HEIGHT;
qreal           stationMoveX;
qreal           stationMoveY;
qreal           bulletMoveX;
qreal           bulletMoveY;
qreal           asteroidMoveX;
qreal           asteroidMoveY;
qreal           startSize = 2.2;

int             i, j, k, x, y, z, newLifeDelay=0, lives=5;
bool            game;

/*************************************************************************************/
/******************** Scene representing the simulated landscape *********************/
/*************************************************************************************/

/************************************ constuctor *************************************/

Scene::Scene() : QGraphicsScene()
{
  // create timer



    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(advance()));
    connect(timer, SIGNAL(timeout()), this, SLOT(collisionDetection()));
    timer->start(1000/25); // fire the timer 25 times per second

    //Initialize the Scene
    this->setSceneRect(0,0,WINDOW_WIDTH-20, WINDOW_HEIGHT-40);

    // create invisible item to provide default top-left anchor to scene
    addLine( 0, 0, 0, 1, QPen(Qt::transparent, 1) );


    alive=true;
    game=true;
    //Shows Lives
    livesString =livesString.setNum(lives);
    livesString.append(" Lives");
    livesLabel = new QLabel (livesString,0);
    livesLabel->setStyleSheet("QLabel{background: transparent; color :green; font:16px;}");
    livesLabel->move(WINDOW_WIDTH-150,50);
    this->addWidget(livesLabel);





  // connect selectionChanged signal to selectStations slot
  connect( this, SIGNAL(selectionChanged()), this, SLOT(selectStations()) );

  station = dynamic_cast<Station*>( itemAt( stationX, stationY ) );

  station = new Station(stationX,stationY );
  this->addItem(station);
  station->setXMove(.001);
  station->setYMove(.001);

  generateAsteroids();


}


void Scene::generateAsteroids()
{
    //Seed random number generator
    QTime now = QTime::currentTime();
    qsrand(now.msec());
    //Empty the list
    asteroidList->clear();
    //Spawn asteroids
    for(int i = 0; i <= MAX_NUM_OF_ASTEROIDS - 1; i++)
    {
        //Randomly place asteroid and set initial velocity
        if(alive)
        {
            do
            {
                asteroidX =qrand() % WINDOW_WIDTH;
                asteroidY =qrand() % WINDOW_HEIGHT;



            }//end do
            while (pow(asteroidX - station->x(), 2) + pow(asteroidY - station->y(), 2) < pow(50, 2));
        }//end if(alive)
        else
        {
            asteroidX =qrand() % WINDOW_WIDTH;
            asteroidY =qrand() % WINDOW_HEIGHT;
       }

        asteroidMoveX = qrand() % 6 - 2.5;
        asteroidMoveY = qrand() % 6 - 2.5;

        //Instantiate asteroid, add to list, and draw it
        Asteroid *asteroid = new Asteroid(asteroidX, asteroidY, asteroidMoveX, asteroidMoveY, startSize);

        asteroidList->append(asteroid);
        this->addItem(asteroid);
    }//end for
}

void Scene::generateBullets()
{
    if(station->alive and (bulletList->size() <= MAX_NUM_OF_BULLETS - 1))
    {
        //Calculate bullet location, in front of ship's "nose"
        bulletX = station->x() + (-22 * qCos(stationRotation * PI / 180));
        bulletY = station->y() +  (-22 * qSin(stationRotation * PI / 180));

        //Calculate bullet's velocity based on
        bulletMoveX = station->getXMove() + -5 * qCos(stationRotation * PI / 180);
        bulletMoveY = station->getYMove() + -5 * qSin(stationRotation * PI / 180);

        //Instantiate bullet, add to list, and draw it
        Bullet *bullet = new Bullet( bulletX,  bulletY, bulletMoveX, bulletMoveY);
        bulletList->append(bullet);
        this->addItem(bullet); //Add to scene so it can be drawn

    }


}

void Scene::collisionDetection()
{
    //cout<<asteroidList->size()<<"  --- "<<endl;


    Bullet *bullet;
    Asteroid *asteroid;//, newAsteroid;
    qreal aLeft, aRight, aTop, aBottom, bX, bY;//, xOffset=0, yOffset=0;
    //qreal newStationXMove, newStationYMove, newAsteroidXMove, newAsteroidYMove;
    bool removeBullet, removeStation; //isCenterClear;

    //Check if Bullet list is Full
    if( not bulletList->isEmpty())
    {
        //Bullet's Data
        for(k=0; k<=bulletList->size()-1; k=k+1)
        {
            bullet=bulletList->at(k);
            bX=bullet->x();//+xOffset;
            bY=bullet->y();//+yOffset;
            removeBullet= false;

            //check if Bullet is still alive
            if(bullet->getLifespan()<=0)
            {
                removeBullet=true;
            }

            //Asteroids Data
            for (j=0; j<=asteroidList->size()-1; j=j+1)
            {
                asteroid=asteroidList->at(j);


                aLeft=asteroid->x() - 8*asteroid->getSize();
                aRight=asteroid->x() + 8*asteroid->getSize();
                aTop=asteroid->y() - 16*asteroid->getSize();
                aBottom=asteroid->y() + 16*asteroid->getSize();

                //Bullet hits the Asteroid
                if( bX >= aLeft and bX <= aRight and bY >= aTop and bY<=aBottom)
                {
                    removeBullet = true;

                  if (asteroid->getSize()<=.49*startSize)
                    {
                        asteroidList->removeAt(j);
                        delete asteroid;
                        asteroid=NULL;
                    }//end if

                    else
                    {

                        asteroidX=asteroid->x();
                        asteroidY=asteroid->y();

                            //Splitting Asteroid
                            for (z=0; z<=1; z=z+1)
                            {
                               //Location of New Ateroids with respect to Old Asteroid

                                asteroidMoveX=(.85+.3*z)*asteroid->getXMove();
                                asteroidMoveY=(1.15-.3*z)*asteroid->getYMove();
                                asteroidX=asteroidX+20*z;
                                asteroidY=asteroidY+20*z;

                                Asteroid *newAsteroid= new Asteroid (asteroidX, asteroidY, asteroidMoveX, asteroidMoveY, .7*asteroid->getSize());
                                asteroidList->append(newAsteroid);
                                this->addItem(newAsteroid);

                            }//end for z

                        this->removeItem(asteroid);
                        asteroidList->removeAt(j);
                        delete asteroid;
                        asteroid=NULL;

                    }//end else


                }//end if


            }//end for j

            if(removeBullet)
            {
                this->removeItem(bullet);
                bulletList->removeAt(k);
                delete bullet;
                bullet=NULL;
            }//end if
        }//end for k
    }//end if


    //When all steroids are gone create more
    if(asteroidList->isEmpty())
    {
        MAX_NUM_OF_ASTEROIDS=MAX_NUM_OF_ASTEROIDS+1;
        generateAsteroids();
    }

    //if Station hits an Asteroid
    if(alive)
    {
        removeStation=false;

        for(x=0; x<=asteroidList->size()-1; x=x+1)
        {
            asteroid= asteroidList->at(x);

            if(sqrt(pow(asteroid->x()-station->x(),2)+pow(asteroid->y()-station->y(),2))<18*asteroid->getSize())
            {
                removeStation=true;

                if(asteroid->getSize()<=.49*startSize)
                {
                    asteroidList->removeAt(x);
                    delete asteroid;
                    asteroid=NULL;
                }// end if

                else
                {



                        //Splitting Asteroid
                        for (y=0; y<=1; y=y+1)
                        {
                           //Location of New Ateroids with respect to Old Asteroid
                            asteroidX=asteroid->x();
                            asteroidY=asteroid->y();
                            asteroidMoveX=(.85+.3*y)*asteroid->getXMove();
                            asteroidMoveY=(1.15-.3*y)*asteroid->getYMove();

                            Asteroid *newAsteroid= new Asteroid (asteroidX, asteroidY, asteroidMoveX, asteroidMoveY, .7*asteroid->getSize());
                            asteroidList->append(newAsteroid);
                            this->addItem(newAsteroid);

                        }//end for y

                    this->removeItem(asteroid);
                    asteroidList->removeAt(x);
                    delete asteroid;
                    asteroid=NULL;
                }//end else
            }//end if
        }//end for z


        if(removeStation)
        {
            alive=false;
            newLifeDelay=75; //waits 3 seconds to redraw Station
            lives=lives-1;
            livesString = livesString.setNum(lives);
            if(lives!=1)
            {
                livesString.append(" Lives");
            }
            else
            {
                livesString.append(" Life");
                livesLabel->setStyleSheet("QLabel{background: transparent; color :red; font:20px;}");

            }

            livesLabel->setText(livesString);
            if(lives==0)
            {
                gameOverLabel=new QLabel("GAME OVER");
                gameOverLabel->setStyleSheet("QLabel{background: transparent; color :red; font:40px;}");
                gameOverLabel->move(0.5*WINDOW_WIDTH-125, 0.5*WINDOW_HEIGHT-50);
                this->addWidget(gameOverLabel);
                lives=5;
                game=false;

            }



            delete station;
            station=NULL;


        }
    }//end if
   if(not alive)
        newLifeDelay--;

   if(newLifeDelay<=0 and (not alive) and  game)
    {
        alive=true;
        station = new Station(WINDOW_WIDTH/2, WINDOW_HEIGHT/2 );
        this->addItem(station);
        station->setXMove(.001);
        station->setYMove(.001);
        stationRotation=90;

    }

}//end collision detection





/********************************** manageObjects ***********************************/

void  Scene::manageObjects()
{


  qreal xDest=0;
  qreal yDest=0;
  {

  //***ASTEROID:

  //Calculate new location
  xDest = asteroidX + asteroidMoveX;
  yDest = asteroidY + asteroidMoveY;

  //If asteroid moves off screen, wrap to other side
  if(xDest < 0)             xDest += WINDOW_WIDTH - 1;
  if(xDest > WINDOW_WIDTH)  xDest -= WINDOW_WIDTH;
  if(yDest < 0)             yDest += WINDOW_HEIGHT - 1;
  if(yDest > WINDOW_HEIGHT) yDest -= WINDOW_HEIGHT;


  asteroidX = xDest;
  asteroidY = yDest;

  //***SHIP:
  //Calculate new location
  xDest = stationX + stationMoveX;
  yDest = stationY + stationMoveY;


    }
}

/********************************** mousePressEvent **********************************/

void  Scene::mousePressEvent( QGraphicsSceneMouseEvent* event )
{

    if(alive)
    generateBullets();
}

/********************************* contextMenuEvent **********************************/

void  Scene::contextMenuEvent( QGraphicsSceneContextMenuEvent* event )
{

}

/********************************** selectStations ***********************************/

void  Scene::selectStations()
{
}

/********************************* mouseReleaseEvent *********************************/

void  Scene::mouseReleaseEvent( QGraphicsSceneMouseEvent* event )
{
    if(alive)
    generateBullets();
}

/************************************ writeStream ************************************/

void  Scene::writeStream( QXmlStreamWriter* stream )
{
  // write station data to xml streamgenerateBullets();
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
    QList <Asteroid*> *asteroidList = new QList<Asteroid*>;
    QList <Bullet*> *bulletList = new QList<Bullet*>;
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

//--------------------------------------------------------------------------------


void Scene:: keyPressEvent(QKeyEvent *event)
{
    if(alive)
    {
    switch(event->key())
    {

        case Qt::Key_Left:
        {
             stationRotation = (stationRotation - 10) % 360;

             if(stationRotation > 359) stationRotation -= 360; //Defensive
             break;
        }

         case Qt::Key_Right:
         {
                stationRotation = (stationRotation + 10) % 360;

                if(stationRotation < 0) stationRotation += 360; //Defensive
                break;
        }generateBullets();

        case Qt::Key_Up:
        //case Qt::Key_w:
        {
                double accel = 0.3;
                stationMoveX = station->getXMove() + accel*qCos((stationRotation + 180) * PI / 180);
                stationMoveY = station->getYMove() + accel*qSin((stationRotation + 180) * PI / 180);
                station->setXMove(stationMoveX);
                station->setYMove(stationMoveY);


            break;
        }

        case Qt::Key_Down:
         //case Qt::Key_w:
        {
             double accel = 0.5;
             stationMoveX = station->getXMove() + accel*qCos((stationRotation) * PI / 180);
             stationMoveY = station->getYMove() + accel*qSin((stationRotation) * PI / 180);
             station->setXMove(stationMoveX);
             station->setYMove(stationMoveY);

             break;
        }

        case Qt::Key_Space:
        {

            generateBullets();
            break;

        }
    }
    }

}


