#include <math.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <QString>
#include <qglobal.h>
#include <qlist.h>
#include <qdebug.h>

#include "heat1D.h"
#include "soilFluxes3D.h"
#include "types.h"
#include "boundary.h"
#include "soilPhysics.h"
#include "commonConstants.h"

Crit3DOut output;

//structure
long NodesNumber;
double TotalDepth ;
double Thickness;
long SimulationStart, SimulationStop;

//surface
double mySurface, Roughness, Plough, RoughnessHeat;

//bottom boundary
double bottomTemperature, bottomTemperatureDepth;

//meteo
double airRelativeHumidity, airTemperature, windSpeed, globalRad;
double precHourlyAmount, precHours, precIniHour, precTemperature;

//initialization
double initialSaturation, initialTemperatureTop, initialTemperatureBottom;

//processes
bool computeWater, computeSolutes, computeHeat;

long CurrentHour;

//soils
int myHorizonNumber;
double ThetaS, ThetaR, Clay, OrganicMatter;

void setTotalDepth(double myValue)
{   TotalDepth = myValue;}

void setThickness(double myValue)
{   Thickness = myValue;}

void setSimulationStart(int myValue)
{   SimulationStart = myValue;}

void setSimulationStop(int myValue)
{   SimulationStop = myValue;}

void setInitialSaturation(double myValue)
{   initialSaturation = myValue;}

void setInitialTemperature(double myValueTop, double myValueBottom)
{   initialTemperatureTop = myValueTop;
    initialTemperatureBottom = myValueBottom;
}

void setBottomTemperature(double myValue, double myDepth)
{
    bottomTemperature = myValue;
    bottomTemperatureDepth = myDepth;
}

void setSoil(double thetaS_, double thetaR_, double clay_, double organicMatter_)
{
    ThetaS = thetaS_;
    ThetaR = thetaR_;
    Clay = clay_;
    OrganicMatter = organicMatter_;
}

void setProcesses(bool computeWaterProcess, bool computeHeat_, bool computeSolutesProcess)
{
    computeWater = computeWaterProcess;
    computeHeat = computeHeat_;
    computeSolutes = computeSolutesProcess;
}

void getHourlyOutput(long myHour, long firstIndex, long lastIndex, QString& myString)
{
    double myWaterContent, h, myTemperature;
    double myDiffusiveHeat, myIsoLatHeat;
	long myIndex;
    double WaterSum = 0;

    myString.append("wc(m3m-3),psi(m),T(degC),diffHeatFlux(Wm-2),isoLatHeat(Wm-2) HOUR: "); // WV (1000*kg m-3) HOUR: ");
    myString.append(QString::number(myHour,'f',0));
    myString.append(QString("\n"));

    for (myIndex = firstIndex ; myIndex <= lastIndex ; myIndex++ )
		{ 
        myWaterContent = soilFluxes3D::getWaterContent(myIndex);
        h = soilFluxes3D::getMatricPotential(myIndex);
        myTemperature = soilFluxes3D::getTemperature(myIndex);
        myDiffusiveHeat = soilFluxes3D::getHeatFlux(myIndex, DOWN, HEATFLUX_DIFFUSIVE);
        myIsoLatHeat = soilFluxes3D::getHeatFlux(myIndex, DOWN, HEATFLUX_LATENT_ISOTHERMAL);

        myString.append(QString::number(myWaterContent,'f',12));
        myString.append(QString(", "));
        myString.append(QString::number(h,'f',6));
        myString.append(QString(", "));

        if (myTemperature != INDEX_ERROR)
            myString.append(QString::number(myTemperature - 273.16,'f',6));
        else
            myString.append(QString::number(myTemperature,'f',6));

        myString.append(QString(", "));
        myString.append(QString::number(myDiffusiveHeat,'f',6));
        myString.append(QString(", "));
        myString.append(QString::number(myIsoLatHeat,'f',6));
        myString.append(QString(", "));

        WaterSum += myWaterContent * myNode[myIndex].volume_area;

        myString.append(QString("\n"));
		}

    myString.append(QString("WaterSum:"));
    myString.append(QString::number(WaterSum,'f',9));
    myString.append(QString(" m3 MBRWater:"));
    myString.append(QString::number(soilFluxes3D::getWaterMBR(),'f',9));
    myString.append(QString(" MBEHeat:"));
    myString.append(QString::number(balanceCurrentTimeStep.heatMBE,'f',9));
    myString.append(QString(" MJ MBRHeat:"));
    myString.append(QString::number(balanceCurrentTimeStep.heatMBR,'f',9));
    myString.append(QString("\n"));
    myString.append(QString("\n"));
}


long getNodesNumber()
{   return (NodesNumber - 1);}

void setSoilHorizonNumber(int myNumber)
{   myHorizonNumber = myNumber;}

void setSurface(double myArea, double myRoughness, double minWaterRunoff, double myRoughnessHeat)
{
    mySurface = myArea;
    Roughness = myRoughness;
    Plough = minWaterRunoff;
    RoughnessHeat = myRoughnessHeat;

    int myResult = soilFluxes3D::setSurfaceProperties(0, Roughness, Plough);
    if (myResult != CRIT3D_OK) printf("\n error in SetSurfaceProperties!");
}


bool initializeSoil(bool useInputSoils)
{
    int myResult;

    // loam (Troy soil db)
    double VG_he        = 0.023;    //m
    double VG_alpha     = 1.76;     //m-1
    double VG_n         = 1.21;
    double mualemTort   = 0.5;
    double KSat         = 1. / (3600. * 100.);

    if (useInputSoils)
    {
        for (int mySoilIndex = 0; mySoilIndex < myHorizonNumber; mySoilIndex++)
            myResult = soilFluxes3D::setSoilProperties(0,mySoilIndex,
                                          myInputSoils[mySoilIndex].VG_alfa,
                                          myInputSoils[mySoilIndex].VG_n,
                                          myInputSoils[mySoilIndex].VG_m,
                                          myInputSoils[mySoilIndex].VG_he,
                                          myInputSoils[mySoilIndex].Theta_r,
                                          myInputSoils[mySoilIndex].Theta_s,
                                          myInputSoils[mySoilIndex].K_sat,
                                          myInputSoils[mySoilIndex].Mualem_L,
                                          myInputSoils[mySoilIndex].OrganicMatter,
                                          myInputSoils[mySoilIndex].Clay);
    }
    else
        myResult = soilFluxes3D::setSoilProperties(0, 1, VG_alpha, VG_n, (1. - 1. / VG_n), VG_he, ThetaR, ThetaS, KSat, mualemTort, OrganicMatter/100., Clay/100.);

    if (myResult != CRIT3D_OK) {
        printf("\n error in SetSoilProperties");
        return(false);
    }
    else return true;
}

bool initializeHeat1D(long *myHourIni, long *myHourFin, bool useInputSoils)
{
    int myResult = 0;
    long indexNode;

    int boundaryType;
    float x = 0.;
    float y = 0.;

    NodesNumber = ceil(TotalDepth / Thickness) + 1;
    double *myDepth = (double *) calloc(NodesNumber, sizeof(double));
    double *myThickness = (double *) calloc(NodesNumber, sizeof(double));

    myThickness[0] = 0;
    myDepth[0] = 0.;

    int myNodeHorizon = 0;

    for (indexNode = 1 ; indexNode<NodesNumber ; indexNode++ )
    {
        myThickness[indexNode] = Thickness;
        if (indexNode == 1) myDepth[indexNode] = myDepth[indexNode-1] - (Thickness / 2) ;
        else  myDepth[indexNode] = myDepth[indexNode-1] - Thickness ;
    }

    myResult = soilFluxes3D::initialize(NodesNumber, (short) NodesNumber, 0, computeWater, computeHeat, computeSolutes);
    if (myResult != CRIT3D_OK) printf("\n error in initialize");

    if (computeHeat) soilFluxes3D::initializeHeat(SAVE_HEATFLUXES_ALL);

    soilFluxes3D::setHydraulicProperties(MODIFIEDVANGENUCHTEN, MEAN_LOGARITHMIC, 10.);

    if (! initializeSoil(useInputSoils)) printf("\n error in setSoilProperties");

    for (indexNode = 0 ; indexNode<NodesNumber ; indexNode++ )
    {
        // elemento superficiale
        if (indexNode == 0)
        {
            myResult = soilFluxes3D::setNode(indexNode, x, y, myDepth[indexNode], mySurface, true, false, BOUNDARY_NONE, 0.0);
            if (myResult != CRIT3D_OK) printf("\n error in setNode!");

            myResult = soilFluxes3D::setNodeSurface(0, 0) ;
            if (myResult != CRIT3D_OK) printf("\n error in setNodeSurface!");

            if (computeWater)
            {
                myResult = soilFluxes3D::setWaterContent(indexNode, 0.);
                if (myResult != CRIT3D_OK) printf("\n error in setWaterContent!");
            }

            if (computeHeat)
            {
                myResult = soilFluxes3D::setTemperature(indexNode, 273.16 + initialTemperatureTop);
                if (myResult != CRIT3D_OK) printf("\n error in setTemperature!");
            }
        }

        // elementi sottosuperficiali
        else
        {
            if (indexNode == 1)
            {
                if (computeHeat)
                    boundaryType = BOUNDARY_HEAT_SURFACE;
                else
                    boundaryType = BOUNDARY_NONE;

                myResult = soilFluxes3D::setNode(indexNode, x, y, myDepth[indexNode], myThickness[indexNode] * mySurface, false, true, boundaryType, 0.0);
                if (myResult != CRIT3D_OK) printf("\n error in setNode!");
            }
            else if (indexNode == NodesNumber - 1)
            {
                myResult = soilFluxes3D::setNode(indexNode, x ,y, myDepth[indexNode], myThickness[indexNode] * mySurface, false, true, BOUNDARY_FREEDRAINAGE, 0.0);
                if (myResult != CRIT3D_OK) printf("\n error in setNode!");

                if (computeHeat)
                {
                    myResult = soilFluxes3D::setFixedTemperature(indexNode, 273.16 + bottomTemperature, bottomTemperatureDepth);
                    if (myResult != CRIT3D_OK) printf("\n error in setFixedTemperature!");
                }
            }
            else
            {
                myResult = soilFluxes3D::setNode(indexNode, x, y, myDepth[indexNode], myThickness[indexNode] * mySurface, false, false, BOUNDARY_NONE, 0.0);																		   
                if (myResult != CRIT3D_OK) printf("\n error in setNode!");
			}											  
									  
            myResult = soilFluxes3D::setNodeLink(indexNode, indexNode - 1 , UP, mySurface);	
            if (myResult != CRIT3D_OK) printf("\n error in setNodeLink!");

            if (useInputSoils)
            {
                if (myDepth[indexNode]<myInputSoils[myNodeHorizon].profInf)
                    if (myNodeHorizon < myHorizonNumber-1)
                        myNodeHorizon++;
            }																									
            else
                myNodeHorizon = 1;

			myResult = soilFluxes3D::setNodeSoil(indexNode, 0, myNodeHorizon);
			if (myResult != CRIT3D_OK) printf("\n error in setNodeSoil!");
																																  
			if (initialSaturation <= 1. && initialSaturation > 0.)
				if (useInputSoils)
					myResult = soilFluxes3D::setWaterContent(indexNode, initialSaturation * (myInputSoils[myNodeHorizon].Theta_s - myInputSoils[myNodeHorizon].Theta_r) + myInputSoils[myNodeHorizon].Theta_r);
				else
					myResult = soilFluxes3D::setWaterContent(indexNode, initialSaturation * (ThetaS - ThetaR) + ThetaR);
			else
				printf("\n error in setWaterContent!");

			if (computeHeat)
			{
				myResult = soilFluxes3D::setTemperature(indexNode,
					273.16 + ((indexNode-1)*(initialTemperatureBottom-initialTemperatureTop)/(NodesNumber-2)+initialTemperatureTop));

				if (myResult != CRIT3D_OK) printf("\n error in SetTemperature!");
			}
        }

		// tutti tranne ultimo nodo confinano con nodo sotto
		if (indexNode < NodesNumber - 1)
		{
			myResult = soilFluxes3D::setNodeLink(indexNode, indexNode + 1 , DOWN, mySurface);
			if (myResult != CRIT3D_OK) printf("\n error in SetNode int sotto!");
		}																 
    }

    soilFluxes3D::setNumericalParameters(1, 600, 100, 10, 12, 5);

    soilFluxes3D::initializeBalance();

    *myHourIni = SimulationStart;
    *myHourFin = SimulationStop;

	return (true);
}

double getCurrentPrec(long myHour)
{ 
        if ((myHour >= precIniHour) && myHour < precIniHour + precHours)
                return (precHourlyAmount);
	else
		return (0.);
}

void setHour(long myHour)
{   CurrentHour = myHour;}

void setSinkSources(double myHourlyPrec)
{
    for (long i=0; i<NodesNumber; i++)
    {
        if (computeHeat) soilFluxes3D::setHeatSinkSource(i, 0);

        if (computeWater)
        {
            if (i == 0)
                soilFluxes3D::setWaterSinkSource(i, (double)myHourlyPrec * mySurface / 3.6e06);
            else
                soilFluxes3D::setWaterSinkSource(i, 0);
        }
    }
}


void Crit3DOut::clean()
{
    nrValues = 0;
    nrLayers = 0;

    errorOutput.clear();
    landSurfaceOutput.clear();
    profileOutput.clear();
}

Crit3DOut::Crit3DOut()
{
    nrValues = 0;
    nrLayers = 0;
    layerThickness = 0.;
}
void getHourlyOutputAllPeriod(long firstIndex, long lastIndex, Crit3DOut *output)
{
    long myIndex;
    double myValue;
    double fluxDiff, fluxLtntIso, fluxLtntTh, fluxAdv, fluxTot;
    double watFluxIsoLiq, watFluxThLiq, watFluxIsoVap, watFluxThVap;

    QPointF myPoint;
    profileStatus myProfile;
    landSurfaceStatus mySurfaceOutput;
    heatErrors myErrors;

    output->nrValues++;
    output->profileOutput.push_back(myProfile);
    output->landSurfaceOutput.push_back(mySurfaceOutput);
    output->errorOutput.push_back(myErrors);

    for (myIndex = firstIndex ; myIndex <= lastIndex ; myIndex++ )
    {
        myPoint.setX(myIndex);

        myValue = soilFluxes3D::getTemperature(myIndex) - 273.16;
        myPoint.setY(myValue);
        output->profileOutput[output->nrValues-1].temperature.push_back(myPoint);

        myValue = soilFluxes3D::getWaterContent(myIndex);
        myPoint.setY(myValue);
        output->profileOutput[output->nrValues-1].waterContent.push_back(myPoint);

        fluxTot = soilFluxes3D::getHeatFlux(myIndex, DOWN, HEATFLUX_TOTAL);
        myPoint.setY(fluxTot == NODATA ? NODATA : fluxTot / mySurface);
        output->profileOutput[output->nrValues-1].totalHeatFlux.push_back(myPoint);

        fluxDiff = soilFluxes3D::getHeatFlux(myIndex, DOWN, HEATFLUX_DIFFUSIVE);
        if (fluxDiff != NODATA) fluxDiff /= mySurface;
        fluxLtntIso = soilFluxes3D::getHeatFlux(myIndex, DOWN, HEATFLUX_LATENT_ISOTHERMAL);
        if (fluxLtntIso != NODATA) fluxLtntIso /= mySurface;
        fluxLtntTh = soilFluxes3D::getHeatFlux(myIndex, DOWN, HEATFLUX_LATENT_THERMAL);
        if (fluxLtntTh != NODATA) fluxLtntTh /= mySurface;
        fluxAdv = soilFluxes3D::getHeatFlux(myIndex, DOWN, HEATFLUX_ADVECTIVE);
        if (fluxAdv != NODATA) fluxAdv /= mySurface;

        myPoint.setY(fluxDiff);
        output->profileOutput[output->nrValues-1].diffusiveHeatFlux.push_back(myPoint);

        myPoint.setY(fluxLtntIso);
        output->profileOutput[output->nrValues-1].isothermalLatentHeatFlux.push_back(myPoint);

        myPoint.setY(fluxLtntTh);
        output->profileOutput[output->nrValues-1].thermalLatentHeatFlux.push_back(myPoint);

        myPoint.setY(fluxAdv);
        output->profileOutput[output->nrValues-1].advectiveheatFlux.push_back(myPoint);

        watFluxIsoLiq = soilFluxes3D::getHeatFlux(myIndex, DOWN, WATERFLUX_LIQUID_ISOTHERMAL);
        if (watFluxIsoLiq != NODATA) watFluxIsoLiq *= 1000.;
        watFluxThLiq  = soilFluxes3D::getHeatFlux(myIndex, DOWN, WATERFLUX_LIQUID_THERMAL);
        if (watFluxThLiq != NODATA) watFluxThLiq *= 1000.;
        watFluxIsoVap = soilFluxes3D::getHeatFlux(myIndex, DOWN, WATERFLUX_VAPOR_ISOTHERMAL);
        if (watFluxIsoVap != NODATA) watFluxIsoVap *= 1000. / WATER_DENSITY;
        watFluxThVap = soilFluxes3D::getHeatFlux(myIndex, DOWN, WATERFLUX_VAPOR_THERMAL);
        if (watFluxThVap != NODATA) watFluxThVap *= 1000. / WATER_DENSITY;

        myPoint.setY(watFluxIsoLiq);
        output->profileOutput[output->nrValues-1].waterIsothermalLiquidFlux.push_back(myPoint);

        myPoint.setY(watFluxThLiq);
        output->profileOutput[output->nrValues-1].waterThermalLiquidFlux.push_back(myPoint);

        myPoint.setY(watFluxIsoVap);
        output->profileOutput[output->nrValues-1].waterIsothermalVaporFlux.push_back(myPoint);

        myPoint.setY(watFluxIsoLiq);
        output->profileOutput[output->nrValues-1].waterThermalVaporFlux.push_back(myPoint);

    }

    myPoint.setX(output->landSurfaceOutput.size() + 1);

    // net radiation (positive downward)
    myValue = soilFluxes3D::getBoundaryRadiativeFlux(1);
    myPoint.setY(myValue);
    output->landSurfaceOutput[output->nrValues-1].netRadiation = myPoint;

    // sensible heat (positive upward)
    myValue = soilFluxes3D::getBoundarySensibleFlux(1);
    myPoint.setY(myValue);
    output->landSurfaceOutput[output->nrValues-1].sensibleHeat = myPoint;

    // latent heat (positive upward)
    myValue = soilFluxes3D::getBoundaryLatentFlux(1);
    myPoint.setY(myValue);
    output->landSurfaceOutput[output->nrValues-1].latentHeat = myPoint;

    //aerodynamic resistance
    myValue = soilFluxes3D::getBoundaryAerodynamicConductance(1);
    myPoint.setY(1./myValue);
    output->landSurfaceOutput[output->nrValues-1].aeroResistance = myPoint;

    //soil surface resistance
    myValue = soilFluxes3D::getBoundarySoilConductance(1);
    myPoint.setY(1./myValue);
    output->landSurfaceOutput[output->nrValues-1].soilResistance = myPoint;

    //errors
    myValue = soilFluxes3D::getHeatMBR();
    myPoint.setY(myValue);
    output->errorOutput[output->nrValues-1].heatMBR = myPoint;

    myValue = soilFluxes3D::getHeatMBE();
    myPoint.setY(myValue);
    output->errorOutput[output->nrValues-1].heatMBE = myPoint;

    myValue = soilFluxes3D::getWaterMBR();
    myPoint.setY(myValue);
    output->errorOutput[output->nrValues-1].waterMBR = myPoint;
}

QString Crit3DOut::getTextOutput(bool getTemp, bool getWater, bool getHeatFlux, bool getSurfBalance, bool getErrors, bool getCond)
{
    QString myString = "";
    float myValue;

    if (getTemp)
        for (int j=0; j<nrLayers; j++)
        {
            myString.append(QString("TempSoil_"));
            myString.append(QString::number(j));
            myString.append(QString(","));
        }

    if (getWater)
        for (int j=0; j<nrLayers; j++)
        {
            myString.append(QString("WaterContent_"));
            myString.append(QString::number(j));
            myString.append(QString(","));
        }

    if (getHeatFlux)
        for (int j=0; j<nrLayers; j++)
        {
            myString.append(QString("HeatFlux_"));
            myString.append(QString::number(j));
            myString.append(QString(","));
        }

    if (getSurfBalance)
    {
        myString.append(QString("srfNetIrrad,"));
        myString.append(QString("srfSnsblHeat,"));
        myString.append(QString("srfLtntHeat,"));
    }

    if (getCond)
    {
        myString.append(QString("aeroResistance,"));
        myString.append(QString("soilResistance,"));
    }

    if (getErrors)
    {
        myString.append(QString("HeatMB,"));
        myString.append(QString("HeatMBE,"));
        myString.append(QString("WaterMBR"));
    }

    myString.append(QString("\n"));

    for (int i=0; i<nrValues; i++)
    {
        if (getTemp)
            for (int j=0; j<nrLayers; j++)
            {
                myValue = profileOutput[i].temperature[j].y();
                myString.append(QString::number(myValue,'f',4));
                myString.append(QString(","));
            }

        if (getWater)
            for (int j=0; j<nrLayers; j++)
            {
                myValue = profileOutput[i].waterContent[j].y();
                myString.append(QString::number(myValue,'f',4));
                myString.append(QString(","));
            }

        if (getHeatFlux)
            for (int j=0; j<nrLayers; j++)
            {
                myValue = profileOutput[i].totalHeatFlux[j].y();
                myString.append(QString::number(myValue,'f',4));
                myString.append(QString(","));
            }

        if (getSurfBalance)
        {
            myValue = landSurfaceOutput[i].netRadiation.y();
            myString.append(QString::number(myValue,'f',2));
            myString.append(QString(","));

            myValue = landSurfaceOutput[i].sensibleHeat.y();
            myString.append(QString::number(myValue,'f',2));
            myString.append(QString(","));

            myValue = landSurfaceOutput[i].latentHeat.y();
            myString.append(QString::number(myValue,'f',2));
            myString.append(QString(","));
        }

        if (getCond)
        {
            myValue = landSurfaceOutput[i].aeroResistance.y();
            myString.append(QString::number(myValue,'f',6));
            myString.append(QString(", "));

            myValue = landSurfaceOutput[i].soilResistance.y();
            myString.append(QString::number(myValue,'f',6));
            myString.append(QString(","));
        }

        if (getErrors)
        {
            myValue = errorOutput[i].heatMBR.y();
            myString.append(QString::number(myValue,'f',12));
            myString.append(QString(","));

            myValue = errorOutput[i].heatMBE.y();
            myString.append(QString::number(myValue,'f',12));
            myString.append(QString(","));

            myValue = errorOutput[i].waterMBR.y();
            myString.append(QString::number(myValue,'f',12));
        }

        myString.append(QString("\n"));

    }

    return myString;
}


bool runHeat1D(double myHourlyTemperature,  double myHourlyRelativeHumidity,
                 double myHourlyWindSpeed, double myHourlyNetIrradiance,
                 double myHourlyPrec)
{
    //double currentRoughness;
    //double surfaceWaterHeight;
    //double roughnessWater = 0.005;

    setSinkSources(myHourlyPrec);

    if (computeHeat)
    {
        soilFluxes3D::setHeatBoundaryHeightWind(1, 2);
        soilFluxes3D::setHeatBoundaryHeightTemperature(1, 1.5);
        soilFluxes3D::setHeatBoundaryTemperature(1, myHourlyTemperature);
        soilFluxes3D::setHeatBoundaryRelativeHumidity(1, myHourlyRelativeHumidity);
        soilFluxes3D::setHeatBoundaryWindSpeed(1, myHourlyWindSpeed);
        soilFluxes3D::setHeatBoundaryNetIrradiance(1, myHourlyNetIrradiance);

        /*
        surfaceWaterHeight = soilFluxes3D::getWaterContent(0);
        if (surfaceWaterHeight > RoughnessHeat)
            currentRoughness = 0.005;
        else if (surfaceWaterHeight > 0.)
            currentRoughness = (roughnessWater - RoughnessHeat) / RoughnessHeat * surfaceWaterHeight + RoughnessHeat;
        else
            currentRoughness = RoughnessHeat;
        */

        soilFluxes3D::setHeatBoundaryRoughness(1, RoughnessHeat);
    }

    if (CurrentHour == 37)
        double a=0;

    soilFluxes3D::computePeriod(HOUR_SECONDS);
    qDebug() << "after running hour:" << CurrentHour << " T1: " << soilFluxes3D::getTemperature(1) - 273.16 << " WC0: " << soilFluxes3D::getWaterContent(0);

	return (true);
}


