#ifndef COMMONCONSTANTS_H
#define COMMONCONSTANTS_H

/*-----------------------------------------------------------------------------------

    CRITERIA 3D
    Copyright (C) 2011 Fausto Tomei, Gabriele Antolini, Antonio Volta
    Alberto Pistocchi, Marco Bittelli

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
    gantolini@arpae.it
    avolta@arpae.it
    alberto.pistocchi@gecosistema.it
    marco.bittelli@unibo.it
-----------------------------------------------------------------------------------*/

    //general functions
    #ifndef minValue
        #define minValue(a, b) (((a) < (b))? (a) : (b))
    #endif

    #ifndef maxValue
        #define maxValue(a, b) (((a) > (b))? (a) : (b))
    #endif

    #ifndef NODATA
        #define NODATA -9999
    #endif

    #ifndef HOUR_SECONDS
        #define HOUR_SECONDS 3600.0
    #endif

    #ifndef DAY_SECONDS
        #define DAY_SECONDS 86400.0
    #endif

    //criteria3D
    #define NOLINK -1

    #define PROCESS_WATER 0
    #define PROCESS_HEAT 1
    #define PROCESS_SOLUTES 2

    #define UP 1
    #define DOWN 2
    #define LATERAL 3

    #define INDEX_ERROR -1111
    #define MEMORY_ERROR -2222
    #define TOPOGRAPHY_ERROR -3333
    #define BOUNDARY_ERROR -4444
    #define MISSING_DATA_ERROR -9999
    #define PARAMETER_ERROR -7777

    #define CRIT3D_OK 1

    #define VANGENUCHTEN 0
    #define MODIFIEDVANGENUCHTEN 1
    #define CAMPBELL 2
    #define HUNTFRACTAL 3

    #define MEAN_GEOMETRIC 0
    #define MEAN_LOGARITHMIC 1

    //#define BOUNDARY_SURFACE 1
    #define BOUNDARY_RUNOFF 2
    #define BOUNDARY_FREEDRAINAGE 3
    #define BOUNDARY_FREELATERALDRAINAGE 4
    #define BOUNDARY_PRESCRIBEDTOTALPOTENTIAL 5
    #define BOUNDARY_HEAT 10
    #define BOUNDARY_SOLUTEFLUX 11
    #define BOUNDARY_NONE 99

    #define RELAXATION 1

    #define MAX_SOILS 1024
    #define MAX_SURFACES 1024
    #define MAX_HORIZONS 20

    // maximum number of solutes
    #define MAX_SOLUTES   6

    #define MAX_NUMBER_APPROXIMATIONS 50
    #define MAX_NUMBER_ITERATIONS 1000


    // -------------------ASTRONOMY--------------------
    // [J s-1 m-2] solar constant
    #define SOLAR_CONSTANT  1367
    // [m s-2] GRAVITY acceleration
    #define	GRAVITY	9.80665
    // [m s-1]
    #define LIGHT_SPEED 299792458

    // -------------------CHEMISTRY--------------------
    // [mol-1] Avogadro number
    #define AVOGADRO 6.022E23
    // [J s]
    #define PLANCK 6.626E-34

    // -----------------TERMODYNAMICS-------------------
    // [K] zero Celsius
    #define	ZEROCELSIUS	273.15
    // [] ratio molecular weight of water vapour/dry air
    #define RATIO_WATER_VD 0.622
    // [J K-1 mol-1] universal gas constant
    #ifndef R_GAS
        #define R_GAS 8.31447215
    #endif
    // [J m-2 K-4] Stefan-Boltzmann constant
    #define STEFAN_BOLTZMANN 5.670400E-8
    // [J kg-1 K-1] specific heat at constant pressure
    #define CP 1013

    // -----------------MATHEMATICS---------------------
    #ifndef PI
        #define PI  3.14159265358979
    #endif
    #ifndef EPSILON
        #define EPSILON 1E-5
    #endif
    #define EULER 2.718281828459
    #define DEG_TO_RAD 0.0174532925
    #define RAD_TO_DEG 57.2957795
    #define FUNCTION_CODE_SPHERICAL 0


#endif // COMMONCONSTANTS_H