/*-----------------------------------------------------------------------------------
    CRITERIA3D

    Copyright 2016 Fausto Tomei, Gabriele Antolini,
    Alberto Pistocchi, Marco Bittelli, Antonio Volta, Laura Costantini

    You should have received a copy of the GNU General Public License
    along with Nome-Programma.  If not, see <http://www.gnu.org/licenses/>.

    This file is part of CRITERIA3D.
    CRITERIA3D has been developed under contract issued by A.R.P.A. Emilia-Romagna

    CRITERIA3D is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    CRITERIA3D is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with CRITERIA3D.  If not, see <http://www.gnu.org/licenses/>.

    contacts:
    fausto.tomei@gmail.com
    ftomei@arpae.it
-----------------------------------------------------------------------------------*/

#include <math.h>

#include "commonConstants.h"
#include "meteo.h"


Crit3DClimateParameters::Crit3DClimateParameters()
{
    for (int i=0; i<12; i++)
    {
        tminLapseRate[i] = NODATA;
        tmaxLapseRate[i] = NODATA;
        tDewMinLapseRate[i] = NODATA;
        tDewMaxLapseRate[i] = NODATA;
    }
}

float computeTminHourlyWeight(int myHour)
{
    if (myHour >= 6 && myHour <= 14)
        return (1 - ((float)myHour - 6) / 8);
    else if (myHour > 14)
        return (1 - minValue((24 - (float)myHour) + 6, 12) / 12);
    else
        return (1 - (6 - (float)myHour) / 12);
}

float Crit3DClimateParameters::getClimateLapseRate(meteoVariable myVar, Crit3DDate* myDate, int myHour)
{
    float lapseTmin, lapseTmax;
    int indexMonth = myDate->month - 1;

    if (myVar == airTemperature)
    {
            lapseTmin = tminLapseRate[indexMonth];
            lapseTmax = tmaxLapseRate[indexMonth];
    }
    else if (myVar == airDewTemperature)
    {
        lapseTmin = tDewMinLapseRate[indexMonth];
        lapseTmax = tDewMaxLapseRate[indexMonth];
    }
    else
        return NODATA;

    float tminWeight = computeTminHourlyWeight(myHour);
    return (lapseTmin * tminWeight + lapseTmax * (1 - tminWeight));
}

float tDewFromRelHum(float rhAir, float airT)
{
    if (rhAir == NODATA || airT == NODATA)
        return NODATA;

    rhAir = minValue(100, rhAir);

    double mySaturatedVaporPres = exp((16.78 * airT - 116.9) / (airT + 237.3));
    double actualVaporPres = rhAir / 100. * mySaturatedVaporPres;
    return float((log(actualVaporPres) * 237.3 + 116.9) / (16.78 - log(actualVaporPres)));
}

float relHumFromTdew(float DewT, float airT)
{
    if (DewT == NODATA || airT == NODATA)
        return NODATA;
    else
    {
        double d = 237.3;
        double c = 17.2693882;
        double esp = 1 / (airT + d);
        double myValue = pow((exp((c * DewT) - ((c * airT / (airT + d))) * (DewT + d))), esp);
        myValue *= 100.;
        if (myValue > 100.)
            return 100;
        else if (myValue == 0.)
            return 1.;
        else
            return float(myValue);
    }
}

double computeSatVapPressure(double myT)
{
    return float(0.611 * exp(17.502 * myT / (myT + 240.97)));
}

double dailyExtrRadiation(double myLat, int myDoy)
{
    /*
    2011 GA
    da quaderno FAO
    MJ m-2 d-1
    */

    double OmegaS;                               //[rad] sunset hour angle
    double Phi;                                  //[rad] latitude in radiants
    double delta;                                //[rad] solar declination
    double dr;                                   //[-] inverse Earth-Sun relative distance

    Phi = PI / 180 * myLat;
    delta = 0.4093 * sin((2 * PI / 365) * myDoy - 1.39);
    dr = 1 + 0.033 * cos(2 * PI * myDoy / 365);
    OmegaS = acos(-tan(Phi) * tan(delta));

    return SOLAR_CONSTANT * DAY_SECONDS / 1000000 * dr / PI * (OmegaS * sin(Phi) * sin(delta) + cos(Phi) * cos(delta) * sin(OmegaS));
}

double LatentHeatVaporization(double myTCelsius)
// [J kg-1] latent heat of vaporization
{
    return (2501000. - 2361. * myTCelsius);
}

double Psychro(double myPressure, double myTemp)
// [kPa °C-1] psychrometric instrument constant
{
    return CP * myPressure / (RATIO_WATER_VD * LatentHeatVaporization(myTemp));
}

double pressureFromElevation(double myElevation)
// [kPa]
{
    return 101.3 * pow((1 - 0.0065 * myElevation / 293), 5.26);
}

double emissivityFromVaporPressure(double myVP)
// [] net surface emissivity
{
    return 0.34 - 0.14 * sqrt(myVP);
}

// [Pa K-1] slope of saturation vapor pressure curve
double SaturationSlope(double airTCelsius, double satVapPressure)
{
    return (17.502 * 240.97 * satVapPressure / ((240.97 + airTCelsius) * (240.97 + airTCelsius)));
}


/************************************************************************************
2016 GA
'   myDOY                           [] day of year
'   myLatitude                      [°] latitude in decimal degrees
'   myTmin                          [°C] daily minimum temperature
'   myTmax                          [°C] daily maximum temperature
'   myUmed                          [%] daily average relative humidity
'   myVmed10                        [m s-1] daily average wind intensity
'   mySWGlobRad                     [MJ m-2 d-1] daily global short wave radiation

' comments: G is ignored for now (if heat is active, should be added)
***************************************************************************************/
double ET0_Penman_daily(int myDOY, float myElevation, float myLatitude,
                        float myTmin, float myTmax, float myTminDayAfter,
                        float myUmed, float myVmed10, float mySWGlobRad)
{
        double MAXTRANSMISSIVITY = 0.75;

        double myPressure;                   //[kPa] atmospheric pressure
        double myDailySB;                    //[MJ m-2 d-1 K-4] Stefan Boltzmann constant
        double myPsychro;                    //[kPa °C-1] psychrometric instrument constant
        double myTmed;                       //[°C] daily average temperature
        double myTransmissivity;             //[] global atmospheric trasmissivity
        double myVapPress;                   //[kPa] actual vapor pressure
        double mySatVapPress;                //[kPa] actual average vapor pressure
        double myExtraRad;                   //[MJ m-2 d-1] extraterrestrial radiation
        double mySWNetRad;                   //[MJ m-2 d-1] net short wave radiation
        double myLWNetRad;                   //[MJ m-2 d-1] net long wave emitted radiation
        double myNetRad;                     //[MJ m-2 d-1] net surface radiation
        double delta;                        //[kPa °C-1] slope of vapour pressure curve
        double vmed2;                        //[m s-1] average wind speed estimated at 2 meters
        double EvapDemand;                   //[mm d-1] evaporative demand of atmosphere
        double myEmissivity;                 //[] surface emissivity
        double myLambda;                     //[MJ kg-1] latent heat of vaporization


        if (myTmin == NODATA || myTmax == NODATA || myVmed10 == NODATA || myUmed == NODATA || myUmed < 0 || myUmed > 100 || myTminDayAfter == NODATA)
            return NODATA;

        myTmed = 0.5 * (myTmin + myTmax);

        myExtraRad = dailyExtrRadiation(myLatitude, myDOY);
        if (myExtraRad > 0)
            myTransmissivity = minValue(MAXTRANSMISSIVITY, mySWGlobRad / myExtraRad);
        else
            myTransmissivity = 0;

        myPressure = 101.3 * pow(((293 - 0.0065 * myElevation) / 293), 5.26);

        myPsychro = Psychro(myPressure, myTmed);

        /*
        differs from the one presented in the FAO Irrigation and Drainage Paper N° 56.
        Analysis with several climatic data sets proved that more accurate estimates of ea can be
        obtained using es(Tmed) than with the equation reported in the FAO paper if only mean
        relative humidity is available (G. Van Halsema and G. Muñoz, Personal communication).
        */

        mySatVapPress = 0.6108 * exp(17.27 * myTmed / (myTmed + 237.3));
        myVapPress = mySatVapPress * myUmed / 100;
        delta = SaturationSlope(myTmed, mySatVapPress) / 1000;    //to kPa

        myDailySB = STEFAN_BOLTZMANN * DAY_SECONDS / 1000000;       // to MJ
        myEmissivity = emissivityFromVaporPressure(myVapPress);
        myLWNetRad = myDailySB * (pow(myTmax + 273, 4) + pow(myTmin + 273, 4) / 2) * myEmissivity * (1.35 * (myTransmissivity / MAXTRANSMISSIVITY) - 0.35);

        mySWNetRad = mySWGlobRad * (1 - ALBEDO_CROP_REFERENCE);
        myNetRad = (mySWNetRad - myLWNetRad);

        myLambda = LatentHeatVaporization(myTmed) / 1000000; //to MJ

        vmed2 = myVmed10 * 0.748;

        EvapDemand = 900 / (myTmed + 273) * vmed2 * (mySatVapPress - myVapPress);

        return (delta * myNetRad + myPsychro * EvapDemand / myLambda) / (delta + myPsychro * (1 + 0.34 * vmed2));

}


/*-----------------------------------------------------------------------------------------
http://www.cimis.water.ca.gov/cimis/infoEtoPmEquation.jsp
/----------------------INPUT---------------------------------------------------------------
heigth                                     elevation above mean sea level (meters)
normalizedTransmissivity                   normalized tramissivity [0-1] ()
globalSWRadiation                          net Short Wave radiation (W m-2)
airTemp                                    air temperature (C)
airHum                                     relative humidity (%)
windSpeed                                  wind speed at 2 meters (m s-1)
------------------------------------------------------------------------------------------*/
double ET0_Penman_hourly(double heigth, double normalizedTransmissivity, double globalSWRadiation,
                double airTemp, double airHum, double windSpeed10)
{
    double mySigma;                              //Steffan-Boltzman constant J m-2 h-1 K-4
    double es;                                   //saturation vapor pressure (kPa) at the mean hourly air temperature in C
    double ea;                                   //actual vapor pressure (kPa) at the mean hourly air temperature in C
    double emissivity;                           //net emissivity of the surface
    double netRadiation;                         //net radiation (J m-2 h-1)
    double netLWRadiation;                       //net longwave radiation (J m-2 h-1)
    double g;                                    //soil heat flux density (J m-2 h-1)
    double Cd;                                   //bulk surface resistance and aerodynamic resistance coefficient
    double tAirK;                                //air temperature (Kelvin)
    double windSpeed2;                           //wind speed at 2 meters (m s-1)
    double delta;                                //slope of saturation vapor pressure curve (kPa C-1) at mean air temperature
    double pressure;                             //barometric pressure (kPa)
    double lambda;                               //latent heat of vaporization in (J kg-1)
    double gamma;                                //psychrometric constant (kPa C-1)
    double firstTerm, secondTerm, denominator;


    es = computeSatVapPressure(airTemp);
    ea = airHum * es / 100.0;
    emissivity = emissivityFromVaporPressure(ea);
    tAirK = airTemp + ZEROCELSIUS;
    mySigma = STEFAN_BOLTZMANN * HOUR_SECONDS;
    netLWRadiation = minValue(normalizedTransmissivity, 1) * emissivity * mySigma * (pow(tAirK, 4));

    // from [W m-2] to [J h-1 m-2]
    netRadiation = ALBEDO_CROP_REFERENCE * (3600 * globalSWRadiation) - netLWRadiation;

    // values for grass
    if (netRadiation > 0)
    {   g = 0.1 * netRadiation;
        Cd = 0.24;
    }
    else
    {
        g = 0.5 * netRadiation;
        Cd = 0.96;
    }

    delta = SaturationSlope(airTemp, es) / 1000;    //to kPa;

    pressure = 101.3 * pow(((293 - 0.0065 * heigth) / 293), 5.26);

    gamma = Psychro(pressure, airTemp);
    lambda = LatentHeatVaporization(airTemp);

    windSpeed2 = windSpeed10 * 0.748;

    denominator = delta + gamma * (1 + Cd * windSpeed2);
    firstTerm = delta * (netRadiation - g) / (lambda * denominator);
    secondTerm = (gamma * (37 / tAirK) * windSpeed2 * (es - ea)) / denominator;

    return maxValue(firstTerm + secondTerm, 0);
}


/*--------------------- Input ------------------------------------------
KT                     [-] Samani empirical coefficient
myDoy                  [-] Day number (Jan 1st = 1)
myLat                  [degrees] Latitude
Tmax                   [°C] daily maximum air temperature
Tmin                   [°C] daily minimum air temperature
-------------- Output ------------------------------------------------
ET0_Hargreaves         [mm d-1] potential evapotranspiration
-------------- Notes -------------------------------------------------
Trange minimum         0.25°C  equivalent to 8.5% transimissivity
----------------------------------------------------------------------*/
double ET0_Hargreaves(double KT, double myLat, int myDoy, double tmax, double tmin)
{
    double tavg, deltaT, extraTerrRadiation;

    if (tmax == NODATA || tmin == NODATA || KT == NODATA || myLat == NODATA || myDoy == NODATA)
        return NODATA;

    extraTerrRadiation = dailyExtrRadiation(myLat, myDoy);
    deltaT = maxValue(fabs(tmax - tmin), 0.25);

    tavg = (tmax + tmin) * 0.5;

    return 0.0135 * (tavg + 17.78) * KT * (extraTerrRadiation / 2.456) * sqrt(deltaT);
}