#include "municipality.h"

Municipality::Municipality(int municipalityID, QString name,QString region,QString address,QString phone,QString email,QString longitude,QString latitude,int radius)
{
    this->municipalityID=municipalityID;
    this->name=name;
    this->region=region;
    this->address=address;
    this->phone=phone;
    this->email=email;
    this->longitude=longitude;
    this->latitude=latitude;
    this->radius=radius;
}
    bool Municipality::addMunicipality()
    {
        QSqlQuery query;
        query.prepare("insert into MUNICIPALITY (NAME, REGION, ADDRESS, PHONE, EMAIL ,LONGITUDE ,LATITUDE,RADIUS) values (:name, :region, :address, :phone, :email ,:longitude ,:latitude ,:radius)");
        query.bindValue(":name", name);
        query.bindValue(":region", region);
        query.bindValue(":address", address);
        query.bindValue(":phone", phone);
        query.bindValue(":email", email);
        query.bindValue(":longitude", longitude);
        query.bindValue(":latitude", latitude);
        query.bindValue(":radius", radius);
        return query.exec();

    }

    bool Municipality::getMunicipalityEmail(QString email)
    {
        QSqlQuery query;
        query.prepare("SELECT * FROM MUNICIPALITY WHERE EMAIL=:email");
        query.bindValue(":email", email);
        if(query.exec() && query.next())
        {
            municipalityID=query.value("MUNICIPALITY_ID").toInt();
            name=query.value("NAME").toString();
            region=query.value("REGION").toString();
            address=query.value("ADDRESS").toString();
            phone=query.value("PHONE").toString();
            this->email=query.value("EMAIL").toString();
            longitude=query.value("LONGITUDE").toString();
            latitude=query.value("LATITUDE").toString();
            radius=query.value("RADIUS").toInt();
            return true;
        }
        return false;
    }

    bool Municipality::getMunicipalityAddress(QString address)
    {
        QSqlQuery query;
        query.prepare("SELECT * FROM MUNICIPALITY WHERE ADDRESS=:address");
        query.bindValue(":address", address);
        if(query.exec() && query.next())
        {
            municipalityID=query.value("MUNICIPALITY_ID").toInt();
            name=query.value("NAME").toString();
            region=query.value("REGION").toString();
            this->address=query.value("ADDRESS").toString();
            phone=query.value("PHONE").toString();
            email=query.value("EMAIL").toString();
            longitude=query.value("LONGITUDE").toString();
            latitude=query.value("LATITUDE").toString();
            radius=query.value("RADIUS").toInt();
            return true;
        }
        return false;
    }
    bool Municipality::getMunicipalityPhone(QString phone)
    {
        QSqlQuery query;
        query.prepare("SELECT * FROM MUNICIPALITY WHERE PHONE=:phone");
        query.bindValue(":phone", phone);
        if(query.exec() && query.next())
        {
            municipalityID=query.value("MUNICIPALITY_ID").toInt();
            name=query.value("NAME").toString();
            region=query.value("REGION").toString();
            address=query.value("ADDRESS").toString();
            this->phone=query.value("PHONE").toString();
            email=query.value("EMAIL").toString();
            longitude=query.value("LONGITUDE").toString();
            latitude=query.value("LATITUDE").toString();
            radius=query.value("RADIUS").toInt();
            return true;
        }
        return false;
    }
    bool Municipality::getMunicipalityName(QString name)
    {
        QSqlQuery query;
        query.prepare("SELECT * FROM MUNICIPALITY WHERE NAME=:name");
        query.bindValue(":name", name);
        if(query.exec() && query.next())
        {
            municipalityID=query.value("MUNICIPALITY_ID").toInt();
            this->name=query.value("NAME").toString();
            region=query.value("REGION").toString();
            address=query.value("ADDRESS").toString();
            phone=query.value("PHONE").toString();
            email=query.value("EMAIL").toString();
            longitude=query.value("LONGITUDE").toString();
            latitude=query.value("LATITUDE").toString();
            radius=query.value("RADIUS").toInt();
            return true;
        }
        return false;
    }

    QSqlQueryModel * Municipality::displayMunicipality()
    {
        QSqlQueryModel * model=new QSqlQueryModel();

        model->setQuery("select * from MUNICIPALITY ORDER BY MUNICIPALITY_ID");
        model->setHeaderData(0,Qt::Horizontal,QObject::tr("Municipality ID"));
        model->setHeaderData(1,Qt::Horizontal,QObject::tr("Name"));
        model->setHeaderData(2,Qt::Horizontal,QObject::tr("Region"));
        model->setHeaderData(3,Qt::Horizontal,QObject::tr("Address"));
        model->setHeaderData(4,Qt::Horizontal,QObject::tr("Phone"));
        model->setHeaderData(5,Qt::Horizontal,QObject::tr("Email"));
        model->setHeaderData(6,Qt::Horizontal,QObject::tr("Longitude"));
        model->setHeaderData(7,Qt::Horizontal,QObject::tr("Latitude"));
        model->setHeaderData(8,Qt::Horizontal,QObject::tr("Radius"));

        return model;
    }

    QSqlQueryModel * Municipality::displaySortedMunicipality()
    {
        QSqlQueryModel * model=new QSqlQueryModel();
        model->setQuery("SELECT MUNICIPALITY_ID, NAME, REGION, ADDRESS, PHONE, EMAIL ,LONGITUDE ,LATITUDE,RADIUS FROM MUNICIPALITY ORDER BY REGION");
        model->setHeaderData(0,Qt::Horizontal,QObject::tr("Municipality ID"));
        model->setHeaderData(1,Qt::Horizontal,QObject::tr("Name"));
        model->setHeaderData(2,Qt::Horizontal,QObject::tr("Region"));
        model->setHeaderData(3,Qt::Horizontal,QObject::tr("Address"));
        model->setHeaderData(4,Qt::Horizontal,QObject::tr("Phone"));
        model->setHeaderData(5,Qt::Horizontal,QObject::tr("Email"));
        model->setHeaderData(6,Qt::Horizontal,QObject::tr("Longitude"));
        model->setHeaderData(7,Qt::Horizontal,QObject::tr("Latitude"));
        model->setHeaderData(8,Qt::Horizontal,QObject::tr("Radius"));

        return model;
    }

    bool Municipality::deleteMunicipality(int municipalityID){
        QSqlQuery query;
        QString id=QString::number(municipalityID);

        query.prepare("Delete from MUNICIPALITY where MUNICIPALITY_ID= :id");

        query.bindValue(":id",id);

        return query.exec();
    }

    bool Municipality::updateMunicipality()
    {
        QSqlQuery query;

        query.prepare("UPDATE MUNICIPALITY SET NAME = :name, REGION = :region, ADDRESS = :address, PHONE = :phone, EMAIL = :email, LONGITUDE = :longitude, LATITUDE = :latitude, RADIUS = :radius WHERE MUNICIPALITY_ID = :municipalityID");
        query.bindValue(":municipalityID", municipalityID);
        query.bindValue(":name", name);
        query.bindValue(":region", region);
        query.bindValue(":address", address);
        query.bindValue(":phone", phone);
        query.bindValue(":email", email);
        query.bindValue(":longitude", longitude);
        query.bindValue(":latitude", latitude);
        query.bindValue(":radius", radius);

        return query.exec();
    }
    QVariantList Municipality::getMunicipalitiesMapData()
    {
        QVariantList list;
        QSqlQuery query;

        query.exec("SELECT m.MUNICIPALITY_ID, m.NAME, m.REGION, m.ADDRESS, m.PHONE, m.EMAIL, m.LATITUDE, m.LONGITUDE, m.RADIUS, COALESCE(AVG(s.BLOCKAGE_RATE), 0) AS avg_risk FROM MUNICIPALITY m LEFT JOIN SEWER s ON s.MUNICIPALITY_ID = m.MUNICIPALITY_ID GROUP BY m.MUNICIPALITY_ID, m.NAME, m.REGION, m.ADDRESS, m.PHONE, m.EMAIL, m.LATITUDE, m.LONGITUDE, m.RADIUS");

        while (query.next()) {
            double risk = query.value("avg_risk").toDouble();
            QString level;
            if      (risk > 60) level = "critique";
            else if (risk > 40) level = "eleve";
            else if (risk > 25) level = "modere";
            else if (risk > 0)  level = "faible";
            else                level = "aucun";

            QVariantMap obj;
            obj["id"]      = query.value("MUNICIPALITY_ID").toInt();
            obj["name"]    = query.value("NAME").toString();
            obj["region"]  = query.value("REGION").toString();
            obj["address"] = query.value("ADDRESS").toString();
            obj["phone"]   = query.value("PHONE").toString();
            obj["email"]   = query.value("EMAIL").toString();
            obj["lat"]     = query.value("LATITUDE").toDouble();
            obj["lng"]     = query.value("LONGITUDE").toDouble();
            obj["radius"]  = query.value("RADIUS").toInt();
            obj["risk"]    = QString::number(risk, 'f', 1);
            obj["level"]   = level;
            obj["visible"] = true;
            list.append(obj);
        }
        return list;
    }

    int Municipality::getNbInterventions(int municipalityID)
    {
        QSqlQuery query;
        query.prepare("SELECT COUNT(*) AS nb FROM INTERVENES i JOIN SEWER s ON s.SEWER_ID = i.SEWER_ID WHERE s.MUNICIPALITY_ID = :id AND EXTRACT(YEAR FROM i.INTERVENTION_DATE) = EXTRACT(YEAR FROM SYSDATE)");
        query.bindValue(":id", municipalityID);
        if (query.exec() && query.next())
            return query.value("nb").toInt();
        return 0;
    }
    //COALESCE retourne la première valeur non NULL parmi ses arguments avg peut être NULL remplacer par 0
    double Municipality::getAvgBlockageRate(int municipalityID)
    {
        QSqlQuery query;
        query.prepare("SELECT COALESCE(AVG(BLOCKAGE_RATE), 0) AS avg_b FROM SEWER WHERE MUNICIPALITY_ID = :id");
        query.bindValue(":id", municipalityID);
        if (query.exec() && query.next())
            return query.value("avg_b").toDouble();
        return 0.0;
    }

    int Municipality::getNbSewers(int municipalityID)
    {
        QSqlQuery query;
        query.prepare("SELECT COUNT(*) AS nb FROM SEWER WHERE MUNICIPALITY_ID = :id");
        query.bindValue(":id", municipalityID);
        if (query.exec() && query.next())
            return query.value("nb").toInt();
        return 0;
    }

    double Municipality::calculateScore(int municipalityID)
    {
        return (getNbInterventions(municipalityID) * 0.4) +
               (getAvgBlockageRate(municipalityID) * 0.4) +
               (getNbSewers(municipalityID)        * 0.2);
    }

    double Municipality::calculateTotalScore()
    {
        QSqlQuery query;
        query.exec("SELECT MUNICIPALITY_ID FROM MUNICIPALITY");
        double total = 0.0;
        while (query.next())
            total += calculateScore(query.value("MUNICIPALITY_ID").toInt());
        return total;
    }

    double Municipality::calculatePercentage(int municipalityID, double totalScore)
    {
        if (totalScore <= 0) return 0.0;
        return (calculateScore(municipalityID) / totalScore) * 100.0;
    }

    double Municipality::calculateBudget(int municipalityID, double totalScore, double totalBudget)
    {
        if (totalScore <= 0) return 0.0;
        return (calculateScore(municipalityID) / totalScore) * totalBudget;
    }

    QSqlQueryModel* Municipality::getBudgetMunicipalityList()
    {
        QSqlQueryModel *model = new QSqlQueryModel();
        model->setQuery("SELECT MUNICIPALITY_ID, NAME, REGION, ADDRESS FROM MUNICIPALITY");
        model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
        model->setHeaderData(1, Qt::Horizontal, QObject::tr("Name"));
        model->setHeaderData(2, Qt::Horizontal, QObject::tr("Region"));
        model->setHeaderData(3, Qt::Horizontal, QObject::tr("Address"));
        return model;
    }
    QString Municipality::getMunicipalityRiskMaquette(int municipalityId)
    {
        QSqlQuery query;
        query.prepare("SELECT COALESCE(AVG(s.BLOCKAGE_RATE), 0) AS avg_risk "
                      "FROM MUNICIPALITY m "
                      "LEFT JOIN SEWER s ON s.MUNICIPALITY_ID = m.MUNICIPALITY_ID "
                      "WHERE m.MUNICIPALITY_ID = :id "
                      "GROUP BY m.MUNICIPALITY_ID");
        query.bindValue(":id", municipalityId);
        query.exec();

        if (query.next()) {
            double risk = query.value("avg_risk").toDouble();
            if (risk > 60)
                return "1";
            else
                return "0";
        }
        return "0";
    }
