/* *****************************************************************************
Copyright (c) 2016-2017, The Regents of the University of California (Regents).
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.

REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS 
PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, 
UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

*************************************************************************** */

// Written: fmckenna

#include "UniformDistribution.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDebug>
#include <QDoubleValidator>
#include <SimCenterGraphPlot.h>
#include <math.h>
#include <QPushButton>
#include <QFileDialog>

UniformDistribution::UniformDistribution(QString inpType, QWidget *parent) :RandomVariableDistribution(parent)
{

    //
    // create the main horizontal layout and add the input entries
    //

    QHBoxLayout *mainLayout = new QHBoxLayout();
    QPushButton *showPlotButton = new QPushButton("Show PDF");

    this->inpty=inpType;

    if (inpty==QString("Parameters"))
    {
        a = this->createTextEntry(tr("Min."), mainLayout);
        a->setValidator(new QDoubleValidator);
        b  = this->createTextEntry(tr("Max."), mainLayout);
        b->setValidator(new QDoubleValidator);
        mainLayout->addWidget(showPlotButton);

    } else if (inpty==QString("Moments")) {

        mean = this->createTextEntry(tr("Mean"), mainLayout);
        mean->setValidator(new QDoubleValidator);        
        standardDev = this->createTextEntry(tr("Standard Dev"), mainLayout);
        standardDev->setValidator(new QDoubleValidator);
        mainLayout->addWidget(showPlotButton);

    } else if (inpty==QString("Dataset")) {


        dataDir = this->createTextEntry(tr("Data dir"), mainLayout);
        dataDir->setMinimumWidth(200);
        dataDir->setMaximumWidth(200);

        QPushButton *chooseFileButton = new QPushButton("Choose");
        mainLayout->addWidget(chooseFileButton);

        // Action
        connect(chooseFileButton, &QPushButton::clicked, this, [=](){
                dataDir->setText(QFileDialog::getOpenFileName(this,tr("Open File"),"C://", "All files (*.*)"));
        });
    }


    mainLayout->addStretch();

    // set some defaults, and set layout for widget to be the horizontal layout
    mainLayout->setSpacing(10);
    mainLayout->setMargin(0);
    this->setLayout(mainLayout);

    thePlot = new SimCenterGraphPlot(QString("x"),QString("Probability Densisty Function"),500, 500);

    if (inpty==QString("Parameters")) {
        connect(a,SIGNAL(textEdited(QString)), this, SLOT(updateDistributionPlot()));
        connect(b,SIGNAL(textEdited(QString)), this, SLOT(updateDistributionPlot()));
        connect(showPlotButton, &QPushButton::clicked, this, [=](){ thePlot->hide(); thePlot->show();});
    } else if (inpty==QString("Moments")) {
        connect(mean,SIGNAL(textEdited(QString)), this, SLOT(updateDistributionPlot()));
        connect(standardDev,SIGNAL(textEdited(QString)), this, SLOT(updateDistributionPlot()));
        connect(showPlotButton, &QPushButton::clicked, this, [=](){ thePlot->hide(); thePlot->show();});
    }
}

UniformDistribution::~UniformDistribution()
{
    delete thePlot;
}

bool
UniformDistribution::outputToJSON(QJsonObject &rvObject){

    if (inpty==QString("Parameters")) {
        // check for error condition, a entry had no value
        if ((a->text().isEmpty())||(b->text().isEmpty())) {
            emit sendErrorMessage("ERROR: UniformDistribution - data has not been set");
            return false;
        }
        rvObject["lowerbound"]=a->text().toDouble();
        rvObject["upperbound"]=b->text().toDouble();
    } else if (inpty==QString("Moments")) {
        if ((mean->text().isEmpty())||(standardDev->text().isEmpty())) {
            emit sendErrorMessage("ERROR: UniformDistribution - data has not been set");
            return false;
        }
        rvObject["mean"]=mean->text().toDouble();
        rvObject["standardDev"]=standardDev->text().toDouble();
    } else if (inpty==QString("Dataset")) {
        if (dataDir->text().isEmpty()) {
            emit sendErrorMessage("ERROR: UniformDistribution - data has not been set");
            return false;
        }
        rvObject["dataDir"]=QString(dataDir->text());
    }
    return true;
}

bool
UniformDistribution::inputFromJSON(QJsonObject &rvObject){

    //
    // for all entries, make sure i exists and if it does get it, otherwise return error
    //

    if (this->inpty==QString("Parameters")) {
        if (rvObject.contains("lowerbound")) {
            double theMuValue = rvObject["lowerbound"].toDouble();
            a->setText(QString::number(theMuValue));
        } else {
            emit sendErrorMessage("ERROR: UniformDistribution - no \"a\" entry");
            return false;
        }
        if (rvObject.contains("upperbound")) {
            double theSigValue = rvObject["upperbound"].toDouble();
            b->setText(QString::number(theSigValue));
        } else {
            emit sendErrorMessage("ERROR: UniformDistribution - no \"a\" entry");
            return false;
        }
      } else if (this->inpty==QString("Moments")) {

        if (rvObject.contains("mean")) {
            double theMeanValue = rvObject["mean"].toDouble();
            mean->setText(QString::number(theMeanValue));
        } else {
            emit sendErrorMessage("ERROR: UniformDistribution - no \"mean\" entry");
            return false;
        }
        if (rvObject.contains("standardDev")) {
            double theStdValue = rvObject["standardDev"].toDouble();
            standardDev->setText(QString::number(theStdValue));
        } else {
            emit sendErrorMessage("ERROR: UniformDistribution - no \"mean\" entry");
            return false;
        }
    } else if (this->inpty==QString("Dataset")) {

      if (rvObject.contains("dataDir")) {
          QString theDataDir = rvObject["dataDir"].toString();
          dataDir->setText(theDataDir);
      } else {
          emit sendErrorMessage("ERROR: UniformDistribution - no \"mean\" entry");
          return false;
      }
    }

    this->updateDistributionPlot();
    return true;
}

QString 
UniformDistribution::getAbbreviatedName(void) {
  return QString("Normal");
}

void
UniformDistribution::updateDistributionPlot() {
    double aa=0, bb=0, me=0, st=0;
    if ((this->inpty)==QString("Parameters")) {
        aa=a->text().toDouble(); // Let us follow dakota's parameter definitions
        bb=b->text().toDouble();
        me = 1/2*(aa+bb);
        st = sqrt(1/12)*(bb-aa);
     } else if ((this->inpty)==QString("Moments")) {
        me = mean->text().toDouble();
        st = standardDev->text().toDouble();
        aa = me - sqrt(12)*st/2;
        bb = me + sqrt(12)*st/2;
    }


    if (bb > aa) {

        QVector<double> x(103);
        QVector<double> y(103);
        double delta = (bb-aa)/10;
        x[0]=aa-delta; x[101]=bb;
        x[1]=aa; x[102]=bb+delta;
        y[0]=y[100]=y[101]=y[102]=0.;
        for (int i=1; i<100; i++) {
            double xi = aa + (i-1)*(bb-aa)/98;
            x[i+1] = xi;
            y[i+1] =1.0/(bb-aa);
        }
        thePlot->clear();
        thePlot->addLine(x,y);
    }
}
