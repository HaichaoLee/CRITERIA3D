#include <QString>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDate>

#include "commonConstants.h"
#include "dbToolsMOSES.h"
#include "utilities.h"
#include "meteoPoint.h"


/*!
 * \brief A crop is irrigated if irriRatio > 0
 * \param dbCrop
 * \param idCrop
 * \param myError
 * \return irriRatio  [-]
 */
float getMOSESIrriRatio(QSqlDatabase* dbCrop, QString idCrop, std::string *myError)
{
    *myError = "";

    QString queryString = "SELECT irri_ratio FROM moses_crop WHERE id_moses = '" + idCrop + "'";

    QSqlQuery query = dbCrop->exec(queryString);
    query.last();

    if (! query.isValid())
    {
        if (query.lastError().number() > 0)
            *myError = query.lastError().text().toStdString();
        return(NODATA);
    }

    float myRatio = 0;

    if (getValue(query.value("irri_ratio"), &(myRatio)))
        return myRatio;
    else
        return NODATA;
}


/*!
 * \brief get criteria idCrop from MOSES code
 * \param dbCrop
 * \param idCropMOSES
 * \param myError
 * \return idCrop (string)
 */
QString getMOSESIdCrop(QSqlDatabase* dbCrop, QString idCropMOSES, std::string *myError)
{
    *myError = "";
    QString queryString = "SELECT * FROM moses_crop WHERE id_moses = '" + idCropMOSES + "'";

    QSqlQuery query = dbCrop->exec(queryString);
    query.last();

    if (! query.isValid())
    {
        if (query.lastError().number() > 0)
            *myError = query.lastError().text().toStdString();
        return "";
    }

    QString idCrop;
    getValue(query.value("id_crop"), &idCrop);

    return idCrop;
}


/*!
 * \brief readMOSESMeteoData
 * \param query
 * \param myError
 * \return
 * \note meteoPoint have to be initialized BEFORE function
 */
bool readMOSESDailyData(QSqlQuery *query, Crit3DMeteoPoint *meteoPoint, std::string *myError)
{
    const int MAX_MISSING_DAYS = 3;
    float tmed, prec, et0;
    QDate myDate, expectedDate, previousDate;
    Crit3DDate date;

    float tmin = NODATA;
    float tmax = NODATA;
    float prevTmin = NODATA;
    float prevTmax = NODATA;
    int nrMissingTemp = 0;
    int nrMissingPrec = 0;

    query->first();
    myDate = query->value("date").toDate();
    expectedDate = myDate;
    previousDate = myDate.addDays(-1);

    do
    {
        myDate = query->value("date").toDate();

        if (! myDate.isValid())
        {
            *myError = "Wrong date format: " + query->value("date").toString().toStdString();
            return false;
        }

        if (myDate != previousDate)
        {
            if (myDate != expectedDate)
            {
                if (expectedDate.daysTo(myDate) > MAX_MISSING_DAYS)
                {
                    *myError = "Wrong METEO: too many missing data." + expectedDate.toString().toStdString();
                    return false;
                }
                else
                {
                    // fill missing data
                    while (myDate != expectedDate)
                    {
                        tmin = prevTmin;
                        tmax = prevTmax;
                        tmed = (tmin + tmax) * 0.5;
                        prec = 0;
                        et0 = NODATA;

                        date = getCrit3DDate(expectedDate);
                        meteoPoint->setMeteoPointValueD(date, dailyAirTemperatureMin, tmin);
                        meteoPoint->setMeteoPointValueD(date, dailyAirTemperatureMax, tmax);
                        meteoPoint->setMeteoPointValueD(date, dailyAirTemperatureAvg, tmed);
                        meteoPoint->setMeteoPointValueD(date, dailyPrecipitation, prec);
                        meteoPoint->setMeteoPointValueD(date, dailyPotentialEvapotranspiration, et0);

                        expectedDate = expectedDate.addDays(1);
                    }
                }
            }

            prevTmax = tmax;
            prevTmin = tmin;

            // mandatory
            getValue(query->value("tmin"), &tmin);
            getValue(query->value("tmax"), &tmax);
            if ((tmin == NODATA) || (tmax == NODATA))
            {
                if (nrMissingTemp < MAX_MISSING_DAYS)
                {
                    nrMissingTemp++;
                    if (tmin == NODATA) tmin = prevTmin;
                    if (tmax == NODATA) tmax = prevTmax;
                }
                else
                {
                    *myError = "Wrong METEO: too many missing data " + myDate.toString().toStdString();
                    return false;
                }
            }
            else nrMissingTemp = 0;

            getValue(query->value("prec"), &prec);
            if (prec == NODATA)
            {
                if (nrMissingPrec < MAX_MISSING_DAYS)
                {
                    nrMissingPrec++;
                    prec = 0;
                }
                else
                {
                    *myError = "Wrong METEO: too many missing data " + myDate.toString().toStdString();
                    return false;
                }
            }
            else nrMissingPrec = 0;

            // not mandatory
            getValue(query->value("tavg"), &tmed);
            getValue(query->value("etp"), &et0);
            if (tmed == NODATA) tmed = (tmin + tmax) * 0.5;

            date = getCrit3DDate(myDate);
            if (meteoPoint->obsDataD[0].date.daysTo(date) < meteoPoint->nrObsDataDaysD)
            {
                meteoPoint->setMeteoPointValueD(date, dailyAirTemperatureMin, (float)tmin);
                meteoPoint->setMeteoPointValueD(date, dailyAirTemperatureMax, (float)tmax);
                meteoPoint->setMeteoPointValueD(date, dailyAirTemperatureAvg, (float)tmed);
                meteoPoint->setMeteoPointValueD(date, dailyPrecipitation, (float)prec);
                meteoPoint->setMeteoPointValueD(date, dailyPotentialEvapotranspiration, (float)et0);
            }
            else
            {
                *myError = "Wrong METEO: index out of range.";
                return false;
            }

            previousDate = myDate;
            expectedDate = myDate.addDays(1);
        }

    } while(query->next());

    return true;
}
