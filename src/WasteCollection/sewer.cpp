#include "sewer.h"
Sewer::Sewer(int sID, QString l, int mc, int cc, int wl, int br ,int mID)
{
    sewerID = sID;
    location = l;
    maxCapacity = mc;
    currentCapacity = cc;
    waterLevel = wl;
    blockageRate = br;
    municipalityID=mID;
}

bool Sewer::addSewer()
{
    QSqlQuery query;



    query.prepare("INSERT INTO SEWER "
                  "(LOCATION, MAX_CAPACITY, CURRENT_CAPACITY, "
                  "WATER_LEVEL, BLOCKAGE_RATE, MUNICIPALITY_ID) "
                  "VALUES (:location, :maxCapacity, "
                  ":currentCapacity, :waterLevel, "
                  ":blockageRate, :municipalityID)");

    query.bindValue(":location", location);
    query.bindValue(":maxCapacity", maxCapacity);
    query.bindValue(":currentCapacity", currentCapacity);
    query.bindValue(":waterLevel", waterLevel);
    query.bindValue(":blockageRate", blockageRate);
    query.bindValue(":municipalityID", municipalityID);

    return query.exec();
}

QSqlQueryModel *Sewer::displaySewer()
{
    QSqlQueryModel *model = new QSqlQueryModel();

    model->setQuery(
        "SELECT s.SEWER_ID, "
        "       s.LOCATION, "
        "       s.MAX_CAPACITY, "
        "       s.CURRENT_CAPACITY, "
        "       s.WATER_LEVEL, "
        "       s.BLOCKAGE_RATE, "
        "       m.NAME AS MUNICIPALITY_NAME "
        "FROM SEWER s "
        " LEFT JOIN MUNICIPALITY m ON s.MUNICIPALITY_ID = m.MUNICIPALITY_ID"
        " ORDER BY s.SEWER_ID"
        );

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("Sewer ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Location"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Max Capacity"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Current Capacity"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Water Level"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Blockage Rate"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("Municipality"));

    return model;
}
bool Sewer:: deleteSewer(int sewerID)
{
    QSqlQuery query;

    query.prepare("DELETE FROM SEWER WHERE SEWER_ID = :id");
    query.bindValue(":id", sewerID);



    return query.exec();
}
bool Sewer::updateSewer()
{
    QSqlQuery query;

    if(maxCapacity != 0)
        blockageRate = (currentCapacity * 100) / maxCapacity;
    else
        blockageRate = 0;

    query.prepare("UPDATE SEWER SET LOCATION=:location, "
                  "MAX_CAPACITY=:maxCapacity, "
                  "CURRENT_CAPACITY=:currentCapacity, "
                  "WATER_LEVEL=:waterLevel, "
                  "BLOCKAGE_RATE=:blockageRate, "
                  "MUNICIPALITY_ID=:municipalityID "
                  "WHERE SEWER_ID=:sewerID");

    query.bindValue(":location", location);
    query.bindValue(":maxCapacity", maxCapacity);
    query.bindValue(":currentCapacity", currentCapacity);
    query.bindValue(":waterLevel", waterLevel);
    query.bindValue(":blockageRate", blockageRate);
    query.bindValue(":municipalityID", municipalityID);
    query.bindValue(":sewerID", sewerID);

    return query.exec();
}
QSqlQueryModel* Sewer::displaySortedSewerByBlockage()
{
    QSqlQueryModel *model = new QSqlQueryModel();

    model->setQuery(
        "SELECT s.sewer_id, "
        "       s.location, "
        "       s.max_capacity, "
        "       s.current_capacity, "
        "       s.water_level, "
        "       s.blockage_rate, "
        "       m.NAME "
        "FROM sewer s "
        " LEFT JOIN municipality m ON s.municipality_id = m.municipality_id "
        "ORDER BY s.blockage_rate DESC"
        );

    model->setHeaderData(0, Qt::Horizontal, "Sewer ID");
    model->setHeaderData(1, Qt::Horizontal, "Location");
    model->setHeaderData(2, Qt::Horizontal, "Max Capacity");
    model->setHeaderData(3, Qt::Horizontal, "Current Capacity");
    model->setHeaderData(4, Qt::Horizontal, "Water Level");
    model->setHeaderData(5, Qt::Horizontal, "Blockage Rate");
    model->setHeaderData(6, Qt::Horizontal, "Municipality");

    return model;
}

QSqlQueryModel* Sewer::searchSewerByLocation(QString text)
{
    QSqlQueryModel *model = new QSqlQueryModel();
    QSqlQuery query;

    query.prepare(
        "SELECT s.sewer_id, "
        "       s.location, "
        "       s.max_capacity, "
        "       s.current_capacity, "
        "       s.water_level, "
        "       s.blockage_rate, "
        "       m.NAME "
        "FROM sewer s "
        " LEFT JOIN municipality m ON s.municipality_id = m.municipality_id "
        "WHERE LOWER(s.location) LIKE LOWER(:text)"
        );

    query.bindValue(":text", "%" + text + "%");

    query.exec();
    model->setQuery(std::move(query));

    model->setHeaderData(0, Qt::Horizontal, "Sewer ID");
    model->setHeaderData(1, Qt::Horizontal, "Location");
    model->setHeaderData(2, Qt::Horizontal, "Max Capacity");
    model->setHeaderData(3, Qt::Horizontal, "Current Capacity");
    model->setHeaderData(4, Qt::Horizontal, "Water Level");
    model->setHeaderData(5, Qt::Horizontal, "Blockage Rate");
    model->setHeaderData(6, Qt::Horizontal, "Municipality");

    return model;
}
QSqlQueryModel* Sewer::getRegionsModelForAI()
{
    QSqlQueryModel *model = new QSqlQueryModel();

    model->setQuery(
        "SELECT DISTINCT REGION "
        "FROM MUNICIPALITY "
        "ORDER BY REGION"
        );

    return model;
}
QSqlQueryModel* Sewer::getSewersByRegion(QString region)
{
    QSqlQueryModel *model = new QSqlQueryModel();
    QSqlQuery query;

    query.prepare(
        "SELECT s.SEWER_ID, s.LOCATION "
        "FROM SEWER s "
        " LEFT JOIN MUNICIPALITY m ON s.MUNICIPALITY_ID = m.MUNICIPALITY_ID "
        "WHERE LOWER(m.REGION) = LOWER(:region)"
        );

    query.bindValue(":region", region);
    query.exec();

   model->setQuery(std::move(query));

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("Sewer ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Location"));

    return model;
}
QSqlQueryModel* Sewer::getPrioritizedSewersByRegion(QString region,
                                                    int waterWeight,
                                                    int blockageWeight,
                                                    int chanceOfRain)
{
    QSqlQueryModel *model = new QSqlQueryModel();
    QSqlQuery query;

    query.prepare(
        "SELECT * FROM ( "
        "   SELECT s.SEWER_ID, "
        "          s.LOCATION, "
        "          ((:waterWeight * s.WATER_LEVEL + "
        "            :blockageWeight * s.BLOCKAGE_RATE) * "
        "            (1 + (:chanceOfRain / 100))) AS PRIORITY_SCORE "
        "   FROM SEWER s "
        "   LEFT JOIN MUNICIPALITY m ON s.MUNICIPALITY_ID = m.MUNICIPALITY_ID "
        "   WHERE LOWER(m.REGION) = LOWER(:region) "
        "   ORDER BY PRIORITY_SCORE DESC "
        ") WHERE ROWNUM <= 5"
        );

    query.bindValue(":region", region);
    query.bindValue(":waterWeight", waterWeight);
    query.bindValue(":blockageWeight", blockageWeight);
    query.bindValue(":chanceOfRain", chanceOfRain);

    query.exec();
    model->setQuery(std::move(query));

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("Sewer ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Location"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Priority Score"));

    return model;
}
bool Sewer::getSewerById(int sewerID)
{
    QSqlQuery query;

    query.prepare(
        "SELECT LOCATION, BLOCKAGE_RATE, WATER_LEVEL "
        "FROM SEWER "
        "WHERE SEWER_ID = :id"
        );

    query.bindValue(":id", sewerID);

    if (query.exec() && query.next())
    {
        location = query.value(0).toString();
        blockageRate = query.value(1).toInt();
        waterLevel = query.value(2).toInt();
        return true;
    }

    return false;
}

bool Sewer::addIntervention(int sewerId, int priority)
{
    QSqlQuery query;

    query.prepare(
        "INSERT INTO INTERVENES (TEAM_ID, SEWER_ID, INTERVENTION_DATE, PRIORITY, STATUS) "
        "VALUES (:teamId, :sewerId, SYSDATE + 1, :priority, :status)"
        );

    query.bindValue(":teamId", 0);
    query.bindValue(":sewerId", sewerId);
    query.bindValue(":priority", priority);
    query.bindValue(":status", 0);

    return query.exec();
}
bool Sewer::interventionExists(int sewerId)
{
    QSqlQuery query;

    query.prepare(
        "SELECT 1 "
        "FROM INTERVENES "
        "WHERE SEWER_ID = :sewerId "
        "AND TRUNC(INTERVENTION_DATE) = TRUNC(SYSDATE + 1)"
        );

    query.bindValue(":sewerId", sewerId);

    if (query.exec() && query.next())
        return true;

    return false;
}

bool Sewer::clearCompletedSewers(int teamId, QDate date)
{
    QSqlQuery query;

    query.prepare(
        "UPDATE SEWER "
        "SET CURRENT_CAPACITY = 0 "
        "WHERE SEWER_ID IN ("
        "   SELECT SEWER_ID "
        "   FROM INTERVENES "
        "   WHERE TEAM_ID = :teamId "
        "   AND STATUS = 1 "
        "   AND TRUNC(INTERVENTION_DATE) = TO_DATE(:date, 'YYYY-MM-DD')"
        ")"
        );

    query.bindValue(":teamId", teamId);
    query.bindValue(":date", date.toString("yyyy-MM-dd"));

    return query.exec();
}

bool Sewer::updateWaterLevelFromSensor(int sewerId, int waterLevel)
{
    QSqlQuery query;
    query.prepare("UPDATE SEWER SET WATER_LEVEL = :waterLevel WHERE SEWER_ID = :sewerId");
    query.bindValue(":waterLevel", waterLevel);
    query.bindValue(":sewerId", sewerId);

    return query.exec();
}

bool Sewer::updateCapacityByState(int idSewer, QString state)
{
    // 🔹 charger les données du sewer dans l'objet
    QSqlQuery query;
    query.prepare("SELECT MAX_CAPACITY, CURRENT_CAPACITY, WATER_LEVEL, MUNICIPALITY_ID, LOCATION "
                  "FROM SEWER WHERE SEWER_ID = :id");
    query.bindValue(":id", idSewer);

    if(!query.exec() || !query.next())
        return false;

    // 🔹 remplir les attributs (comme dans ta classe)
    sewerID = idSewer;
    maxCapacity = query.value(0).toInt();
    currentCapacity = query.value(1).toInt();
    waterLevel = query.value(2).toInt();
    municipalityID = query.value(3).toInt();
    location = query.value(4).toString();

    // 🔹 logique inchangée
    if(state.toLower() == "on")
    {
        currentCapacity = maxCapacity ;
    }
    else if(state.toLower() == "off")
    {
        currentCapacity = 0;
    }
    else
    {
        return false;
    }

    // 🔹 recalcul comme dans updateSewer()
    if(maxCapacity != 0)
        blockageRate = (currentCapacity * 100) / maxCapacity;
    else
        blockageRate = 0;

    // 🔹 utiliser updateSewer() pour rester cohérent
    //return updateSewer();
    //QSqlQuery query;

    query.prepare(
        "UPDATE SEWER "
        "SET CURRENT_CAPACITY = :current, BLOCKAGE_RATE = :blockageRate "
        "WHERE SEWER_ID = :sewerID"
        );

    query.bindValue(":sewerID", sewerID);
    query.bindValue(":current", currentCapacity);
    query.bindValue(":blockageRate", blockageRate);

    return query.exec();
}
bool Sewer::getSewerEnvironmentalInput(int sewerID,
                                       int &maxCapacity,
                                       int &currentCapacity,
                                       int &waterLevel,
                                       int &blockageRate,
                                       int &municipalityID)
{
    QSqlQuery query;

    query.prepare(
        "SELECT "
        "   MAX_CAPACITY, "
        "   CURRENT_CAPACITY, "
        "   WATER_LEVEL, "
        "   BLOCKAGE_RATE, "
        "   NVL(MUNICIPALITY_ID, 0) "
        "FROM SEWER "
        "WHERE SEWER_ID = :sewerID"
        );

    query.bindValue(":sewerID", sewerID);

    if (!query.exec() || !query.next())
        return false;

    maxCapacity = query.value(0).toInt();
    currentCapacity = query.value(1).toInt();
    waterLevel = query.value(2).toInt();
    blockageRate = query.value(3).toInt();
    municipalityID = query.value(4).toInt();

    return true;
}


bool Sewer::getMostDangerousSewer(int &sewerId,
                                  int &waterLevel,
                                  int &blockageRate)
{
    QSqlQuery query;

    query.prepare(
        "SELECT * FROM ( "
        "SELECT SEWER_ID, WATER_LEVEL, BLOCKAGE_RATE "
        "FROM SEWER "
        "ORDER BY (WATER_LEVEL + BLOCKAGE_RATE) DESC "
        ") WHERE ROWNUM = 1"
        );

    if (query.exec() && query.next())
    {
        sewerId = query.value(0).toInt();
        waterLevel = query.value(1).toInt();
        blockageRate = query.value(2).toInt();
        return true;
    }

    return false;
}


bool Sewer::getCriticalSewersCount(int &count)
{
    QSqlQuery query;

    query.prepare(
        "SELECT COUNT(*) "
        "FROM SEWER "
        "WHERE WATER_LEVEL >= 80 OR BLOCKAGE_RATE >= 80"
        );

    if (query.exec() && query.next())
    {
        count = query.value(0).toInt();
        return true;
    }

    return false;
}


bool Sewer::getSewerSituationSummary(int &total,
                                     int &critical)
{
    QSqlQuery query;

    query.prepare(
        "SELECT COUNT(*), "
        "SUM(CASE WHEN WATER_LEVEL >= 80 OR BLOCKAGE_RATE >= 80 THEN 1 ELSE 0 END) "
        "FROM SEWER"
        );

    if (query.exec() && query.next())
    {
        total = query.value(0).toInt();
        critical = query.value(1).toInt();
        return true;
    }

    return false;
}


bool Sewer::getHighestRiskMunicipality(QString &municipality,
                                       int &nbSewers,
                                       int &avgWater,
                                       int &avgBlockage,
                                       int &score)
{
    QSqlQuery query;

    query.prepare(
        "SELECT * FROM ( "
        "SELECT m.NAME, "
        "COUNT(s.SEWER_ID), "
        "ROUND(AVG(s.WATER_LEVEL), 0), "
        "ROUND(AVG(s.BLOCKAGE_RATE), 0), "
        "ROUND(AVG((s.WATER_LEVEL + s.BLOCKAGE_RATE) / 2), 0) AS RISK_SCORE "
        "FROM SEWER s "
        "JOIN MUNICIPALITY m ON s.MUNICIPALITY_ID = m.MUNICIPALITY_ID "
        "GROUP BY m.NAME "
        "ORDER BY RISK_SCORE DESC "
        ") WHERE ROWNUM = 1"
        );

    if (query.exec() && query.next())
    {
        municipality = query.value(0).toString();
        nbSewers = query.value(1).toInt();
        avgWater = query.value(2).toInt();
        avgBlockage = query.value(3).toInt();
        score = query.value(4).toInt();
        return true;
    }

    return false;
}
