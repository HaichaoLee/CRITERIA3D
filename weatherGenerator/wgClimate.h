#ifndef WG_CLIMATE_H
#define WG_CLIMATE_H

    class Crit3DDate;
    struct TinputObsData;
    struct TwheatherGenClimate;

    bool computeWGClimate(int nrDays, Crit3DDate firstDate, float *inputTMin, float *inputTMax,
                          float *inputPrec, float precThreshold, float minPercData, TwheatherGenClimate* wGen);

    bool climateGenerator(int nrDays, TinputObsData climateDailyObsData, Crit3DDate climateDateIni,
                          Crit3DDate climateDateFin, float precThreshold, float minPercData, TwheatherGenClimate* wGen);

    float sampleStdDeviation(float values[], int nElement);


#endif // WG_CLIMATE_H

