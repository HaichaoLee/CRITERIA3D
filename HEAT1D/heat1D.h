#define PROGRESSION_ARITHMETIC 0
#define PROGRESSION_GEOMETRIC 1

#include <qpoint.h>
#include <qvector.h>
#include <qdatetime.h>
#include "graphs.h"

struct Qsoil{
    double profSup;
    double profInf;
    double VG_alfa;			// [m-1] alfa Van Genutchen
    double VG_n;			// [-] n Van Genuchten
    double VG_m;			// [-] n Van Genuchten ]0,1[
    double VG_he;			// [m] air-entry potential [0,1]
    double VG_Sc;			// [-] reduction factor for modified VG formulation
    double Theta_s;			// [m3 m-3] theta saturo
    double Theta_r;			// [m3 m-3] theta residuo
    double K_sat;			// [m s-1] conducibilita' idraulica satura
    double Mualem_L;		// [-] parametro tortuosita' della formula conducibilita' di Mualem
    double Clay;			// [-] normalized Clay content [0,1]
    double OrganicMatter;	// [-] normalized Organic matter content [0,1)
    double CEC;				// [meq hg-1] Cation Excange Capacity [0,150]
    double PH;				// [-] PH [3,10]

    double Roughness;		// [s/m^0.33] superficie: scabrezza superficiale (parametro alfa eq. di Manning)
    double Plough;			// [m] superficie: altezza acqua minima(pozzanghera) prima che inizi il deflusso superficiale
    } ;


extern Qsoil *myInputSoils;

void setSurface(double myArea, double myRoughness, double minWaterRunoff, double myRoughnessHeat);
void setSoil(double thetaS_, double thetaR_, double clay_, double organicMatter_);
void setTotalDepth(double myValue);
void setThickness(double myValue);
void setSimulationStart(int myValue);
void setSimulationStop(int myValue);
void setInitialSaturation(double myValueTop, double myValueBottom);
void setInitialTemperature(double myValueTop, double myValueBottom);
void setBottomTemperature(double myValue, double myDepth);
void setHeatProcesses(bool computeHeatAdvection, bool computeHeatLatent, bool computeHeatSensible);
void setProcesses(bool computeWaterProcess, bool computeHeatProcess, bool computeSolutesProcess);
void setProcessesHeat(bool computeLatent_, bool computeAdvection_);
void setHour(long myHour);
double getCurrentPrec(long myHour);
void getHourlyOutputAllPeriod(long firstIndex, long lastIndex, Crit3DOut *output);
long getNodesNumber();
void setSoilHorizonNumber(int myHorizonNumber);

bool initializeHeat1D(long *myHourIni, long *myHourFin, bool useInputSoils);
bool runHeat1D(double myHourlyTemperature,  double myHourlyRelativeHumidity,
                 double myHourlyWindSpeed, double myHourlyNetIrradiance,
                 double myHourlyPrec);


