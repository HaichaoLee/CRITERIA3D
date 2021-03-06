#include "commonConstants.h"
#include "dbArkimet.h"

#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QString>


DbArkimet::DbArkimet(QString dbName) : DbMeteoPoints(dbName)
{
    queryString = "";
}


QList<VariablesList> DbArkimet::getVariableProperties(QList<int> id)
{
    QList<VariablesList> variableList;

    QString idlist = QString("(%1").arg(id[0]);

    for (int i = 1; i < id.size(); i++)
    {
        idlist = idlist % QString(",%1").arg(id[i]);
    }
    idlist = idlist % QString(")");

    QString statement = QString("SELECT * FROM variable_properties WHERE id_arkimet IN %1").arg(idlist);

    QSqlQuery qry(statement, _db);

    if( !qry.exec() )
        qDebug() << qry.lastError();
    else
    {
        while (qry.next())
        {
            variableList.append(VariablesList(qry.value("id_variable").toInt(), qry.value("id_arkimet").toInt(), qry.value("variable").toString(), qry.value("frequency").toInt() ));
        }
    }

    return variableList;
}


QString DbArkimet::getVarName(int id)
{
    QString varName = NULL;
    QSqlQuery qry(_db);

    qry.prepare( "SELECT variable FROM variable_properties WHERE id_arkimet = :id" );
    qry.bindValue(":id", id);

    if( !qry.exec() )
    {
        qDebug() << qry.lastError();
    }
    else
    {
        qDebug( "getVarName Selected!" );

        if (qry.next())
            varName = qry.value(0).toString();

        else
            qDebug( "Error: dataset not found" );
    }

    return varName;
}


int DbArkimet::getId(QString VarName)
{
    int id = 0;
    QSqlQuery qry(_db);

    qry.prepare( "SELECT id_arkimet FROM variable_properties WHERE variable = :VarName" );
    qry.bindValue(":VarName", VarName);

    if( !qry.exec() )
    {
        qDebug() << qry.lastError();
    }
    else
    {
        if (qry.next())
            id = qry.value(0).toInt();

        else
            qDebug( "Error: dataset not found" );
    }

    return id;
}


QList<int> DbArkimet::getDailyVar()
{
    QList<int> dailyVarList;
    QSqlQuery qry(_db);

    qry.prepare( "SELECT id_arkimet FROM variable_properties WHERE frequency = 86400" );

    if( !qry.exec() )
    {
        qDebug() << qry.lastError();
    }
    else
    {
        qDebug( "getDailyVar Selected!" );

        while (qry.next())
        {
            int id = qry.value(0).toInt();
            dailyVarList << id;
        }
    }

    return dailyVarList;
}


QList<int> DbArkimet::getHourlyVar()
{
    QList<int> hourlyVarList;
    QSqlQuery qry(_db);

    qry.prepare( "SELECT id_arkimet FROM variable_properties WHERE frequency < 86400" );

    if( !qry.exec() )
    {
        qDebug() << qry.lastError();
    }
    else
    {
        while (qry.next())
        {
            int id = qry.value(0).toInt();
            hourlyVarList << id;
        }
    }

    return hourlyVarList;
}



void DbArkimet::initStationsDailyTables(QDate startDate, QDate endDate, QStringList stations)
{

    for (int i = 0; i < stations.size(); i++)
    {
        QString statement = QString("CREATE TABLE IF NOT EXISTS `%1_D` "
                                    "(date_time TEXT, id_variable INTEGER, value REAL, PRIMARY KEY(date_time,id_variable))").arg(stations[i]);

        QSqlQuery qry(statement, _db);
        qry.exec();

        statement = QString("DELETE FROM `%1_D` WHERE date_time >= DATE('%2') AND date_time < DATE('%3', '+1 day')")
                        .arg(stations[i]).arg(startDate.toString("yyyy-MM-dd")).arg(endDate.toString("yyyy-MM-dd"));

        qry = QSqlQuery(statement, _db);
        qry.exec();
    }

}


void DbArkimet::initStationsHourlyTables(QDate startDate, QDate endDate, QStringList stations)
{
    for (int i = 0; i < stations.size(); i++)
    {
        QString statement = QString("CREATE TABLE IF NOT EXISTS `%1_H` (date_time TEXT, id_variable INTEGER, value REAL, PRIMARY KEY(date_time,id_variable))").arg(stations[i]);

        QSqlQuery qry(statement, _db);
        qry.exec();

        statement = QString("DELETE FROM `%1_H` WHERE date_time >= DATE('%2') AND date_time < DATE('%3', '+1 day')")
                        .arg(stations[i]).arg(startDate.toString("yyyy-MM-dd")).arg(endDate.toString("yyyy-MM-dd"));

        qry = QSqlQuery(statement, _db);
        qry.exec();
    }
}


void DbArkimet::createTmpTableHourly()
{
    this->deleteTmpTableHourly();

    QSqlQuery qry(_db);
    qry.prepare("CREATE TABLE TmpHourlyData (KEY TEXT, date_time TEXT, date_time_adj TEXT, id_point TEXT, id_variable INTEGER, variable_name TEXT, value REAL, frequency INTEGER)");
    if( !qry.exec() )
    {
        qDebug() << qry.lastError();
    }
}


void DbArkimet::createTmpTableDaily()
{
    this->deleteTmpTableDaily();

    QSqlQuery qry(_db);
    qry.prepare("CREATE TABLE TmpDailyData (date TEXT, id_point TEXT, id_variable INTEGER, value REAL)");
    if( !qry.exec() )
    {
        qDebug() << qry.lastError();
    }
}


void DbArkimet::deleteTmpTableHourly()
{
    QSqlQuery qry(_db);

    qry.prepare( "DROP TABLE TmpHourlyData" );

    if( !qry.exec() )
    {
        //qDebug() << "DROP TABLE TmpHourlyData" << qry.lastError();
    }
}


void DbArkimet::deleteTmpTableDaily()
{
    QSqlQuery qry(_db);

    qry.prepare( "DROP TABLE TmpDailyData" );

    if( !qry.exec() )
    {
        //qDebug() << "DROP TABLE TmpDailyData" << qry.lastError();
    }
}


void DbArkimet::appendQueryHourly(QString dateTimeStr, QString idPoint, QString idVariable, QString varName, QString value, QString frequency, bool isFirstData)
{
    if (isFirstData)
        queryString = "INSERT INTO TmpHourlyData VALUES ";
    else
        queryString += ",";

    // build an hourly key
    // shift time to end hour
    QDateTime myTime = QDateTime::fromString(dateTimeStr, "yyyy-MM-dd hh:mm:ss");
    if (myTime.time().minute() > 0)
    {
        int hour = myTime.time().hour();
        myTime.setTime(QTime(hour, 0, 0));
        myTime = myTime.addSecs(3600);
    }
    QString key = varName + myTime.toString("yyyyMMddhh") + "_" + idPoint;

    QString dateTimeAdj = myTime.toString("yyyy-MM-dd hh:mm:ss");

    queryString += "('" + key + "'"
            + ",'" + dateTimeStr + "'"
            + ",'" + dateTimeAdj + "'"
            + ",'" + idPoint + "'"
            + "," + idVariable
            + ",'" + varName + "'"
            + "," + value
            + "," + frequency
            + ")";
}


void DbArkimet::appendQueryDaily(QString date, QString idPoint, QString idVar, QString value, bool isFirstData)
{
    if (isFirstData)
        queryString = "INSERT INTO TmpDailyData VALUES ";
    else
        queryString += ",";

    queryString += "('" + date + "'"
            + ",'" + idPoint + "'"
            + "," + idVar
            + "," + value
            + ")";

}

bool DbArkimet::saveDailyData(QDate startDate, QDate endDate)
{
    // insert data into tmpTable
    _db.exec(queryString);

    // query stations with data
    QString statement = QString("SELECT DISTINCT id_point FROM TmpDailyData");
    QSqlQuery qry = _db.exec(statement);

    // create data stations list
    QStringList stations;
    while (qry.next())
        stations.append(qry.value(0).toString());

    // create station tables
    initStationsDailyTables(startDate, endDate, stations);

    // insert data
    foreach (QString id_point, stations)
    {
        statement = QString("INSERT INTO `%1_D` ").arg(id_point);
        statement += QString("SELECT date, id_variable, value FROM TmpDailyData ");
        statement += QString("WHERE id_point = %1").arg(id_point);

        _db.exec(statement);
        if (_db.lastError().type() != QSqlError::NoError)
        {
            qDebug() << _db.lastError();
            return false;
        }
    }

    return true;
}


bool DbArkimet::saveHourlyData()
{
    // insert data into tmpTable
    _db.exec(queryString);

    // clean duplicate data
    QString statement = "DELETE FROM TmpHourlyData WHERE frequency < 3600 ";
    statement += "AND KEY IN (SELECT KEY FROM TmpHourlyData WHERE frequency = 3600)";
    _db.exec(statement);

    QSqlQuery qry = QSqlQuery(_db);

    // query stations
    statement = QString("SELECT DISTINCT id_point FROM TmpHourlyData");
    qry.exec(statement);

    QStringList stations;
    while (qry.next())
        stations.append(qry.value(0).toString());

    // INSERT data with frequency = 3600
    foreach (QString id_point, stations)
    {
        statement = QString("INSERT INTO `%1_H` ").arg(id_point);
        statement += QString("SELECT date_time, id_variable, value FROM TmpHourlyData ");
        statement += QString("WHERE id_point = %1 AND frequency = 3600").arg(id_point);

        _db.exec(statement);
    }

    // DELETE data with frequency = 3600
    statement = QString("DELETE FROM TmpHourlyData WHERE frequency = 3600");
    _db.exec(statement);

    // re-query stations
    stations.clear();
    statement = QString("SELECT DISTINCT id_point FROM TmpHourlyData");
    qry = _db.exec(statement);
    while (qry.next())
        stations.append(qry.value(0).toString());

    // no more data
    if (stations.isEmpty()) return true;

    // WIND DIRECTION
    statement = QString("INSERT INTO `%1_H` ");
    statement += "SELECT date_time, id_variable, value FROM TmpHourlyData WHERE ";
    statement += "id_point = %1 AND variable_name = 'W_DIR' AND strftime('%M', date_time) = '00'";

    foreach (QString station, stations) {
        qry.exec(statement.arg(station));
    }

    // RADIATION: prevailing HH:30 data
    statement = QString("INSERT INTO `%1_H` ");
    statement += " SELECT DATETIME(date_time, '+30 minutes'), id_variable, value FROM TmpHourlyData WHERE ";
    statement += " id_point = %1 AND variable_name = 'RAD' AND strftime('%M', date_time) = '30'";

    foreach (QString station, stations) {
        qry.exec(statement.arg(station));
    }

    // DELETE radiation and wind direction
    statement = QString("DELETE FROM TmpHourlyData WHERE variable_name IN ('RAD', 'W_DIR')");
    qry.exec(statement);

    // TODO SUM PREC

    // la media funziona su tutte le var che hanno AVG nel nome (Temp, RH, Wind intensity)
    statement = QString("INSERT INTO `%1_H`");
    statement += " SELECT date_time_adj, id_variable, avg_value FROM (";
    statement += " SELECT KEY, date_time_adj, id_variable, AVG(value) AS avg_value";
    statement += " FROM TmpHourlyData WHERE id_point = %1 AND variable_name like '%AVG%' GROUP BY KEY )";

    QString delStationStatement = QString("DELETE FROM TmpHourlyData WHERE id_point = %1");

    foreach (QString station, stations)
    {
        if (! qry.exec(statement.arg(station)))
            qDebug() << "error in hourly insert " << station << qry.lastError();

        if (! qry.exec(delStationStatement.arg(station)))
            qDebug() << "error in delete " << station << qry.lastError();
    }

    return true;
}

