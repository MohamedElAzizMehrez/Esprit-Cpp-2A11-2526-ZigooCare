#ifndef GWASTECOLLECTION_H
#define GWASTECOLLECTION_H

#include <QMainWindow>
#include <QModelIndex>
#include <QMessageBox>
#include <QQuickWidget>
#include <QQmlContext>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QVBoxLayout>
#include <QVariantList>
#include <QVariantMap>
#include <QQuickItem>
#include <QStandardItemModel>
#include <QMap>
#include <QString>
#include <QBarSet>
#include <QTextToSpeech>

#include "arduino.h"
#include "employee.h"
#include "municipality.h"
#include "sewer.h"
#include "collectionteam.h"
#include "recyclingcenter.h"
#include "arduino.h"
QT_BEGIN_NAMESPACE

namespace Ui {
class GWasteCollection;
}
QT_END_NAMESPACE

class GWasteCollection : public QMainWindow
{
    Q_OBJECT

public:
    GWasteCollection(QWidget *parent = nullptr);
    ~GWasteCollection();
    void closeEvent(QCloseEvent *event);

private slots:

    void handleSerialData();

    // -----------------------------------------------------------------
    // -------------------- Employee Module ----------------------------
    // -----------------------------------------------------------------

    void on_pushButton_show_e1_pressed();

    void on_pushButton_show_e1_released();

    void on_pushButton_reset_e1_clicked();

    void on_pushButton_login_e1_clicked();

    void on_pushButton_fingerprint_e1_clicked();

    void on_comboBox_question_e2_currentTextChanged(const QString &arg1);

    void on_pushButton_confirm_e2_clicked();

    void on_pushButton_show_e2_pressed();

    void on_pushButton_show_e2_released();

    void on_pushButton_reset_e2_clicked();

    void on_pushButton_cancel_e2_clicked();

    void on_pushButton_show_e3_pressed();

    void on_pushButton_show_e3_released();

    void on_pushButton_fingerprint_e3_clicked();

    void on_pushButton_confirm_e3_clicked();

    void on_pushButton_logout_e4_clicked();

    void on_pushButton_municipality_e4_clicked();

    void on_pushButton_sewer_e4_clicked();

    void on_pushButton_center_e4_clicked();

    void on_pushButton_team_e4_clicked();

    void on_pushButton_confirm_e4_clicked();

    void on_tableView_list_e4_doubleClicked(const QModelIndex &index);

    void on_tableView_list_e4_clicked(const QModelIndex &index);

    void on_pushButton_restore_e4_clicked();

    void on_pushButton_sort_e4_clicked();

    void on_lineEdit_search_e4_textChanged(const QString &arg1);

    void on_pushButton_export_e4_clicked();

    //-------------------------------------------------------------------------------
    //-----------------------------Municipality Module-------------------------------
    //-------------------------------------------------------------------------------

    void on_pushButton_map_m1_clicked();

    void on_pushButton_budget_m1_clicked();

    void on_pushButton_management_m2_clicked();

    void on_pushButton_budget_m2_clicked();

    void on_pushButton_management_m3_clicked();

    void on_pushButton_map_m3_clicked();

    void on_pushButton_confirm_m3_clicked();

    void on_pushButton_logout_m3_clicked();

    void on_pushButton_logout_m2_clicked();

    void on_pushButton_logout_m1_clicked();

    void on_pushButton_employee_m3_clicked();

    void on_pushButton_employee_m2_clicked();

    void on_pushButton_employee_m1_clicked();

    void on_pushButton_sewer_m3_clicked();

    void on_pushButton_sewer_m2_clicked();

    void on_pushButton_sewer_m1_clicked();

    void on_pushButton_center_m3_clicked();

    void on_pushButton_center_m1_clicked();

    void on_pushButton_center_m2_clicked();

    void on_pushButton_team_m3_clicked();

    void on_pushButton_team_m2_clicked();

    void on_pushButton_team_m1_clicked();

    void on_pushButton_confirm_m1_clicked();

    void on_pushButton_autoloc_m1_clicked();

    void on_pushButton_cancel_map_m1_clicked();

    void on_pushButton_confirm_map_m1_clicked();

    void onLocationPicked(double lat, double lng);

    void on_pushButton_restor_m1_clicked();

    void on_tableView_listemunicipality_m1_doubleClicked(const QModelIndex &index);

    void on_tableView_listemunicipality_m1_clicked(const QModelIndex &index);

    void on_lineEdit_barrederecherche_m1_textChanged(const QString &arg1);

    void on_pushButton_sort_m1_clicked();

    void on_pushButton_export_m1_clicked();

    void displayMunicipalityStatistics();

    void computeRegionStatistics(QString regions[], int counts[], int &nbRegions);

    void on_lineEdit_barrederecherche_m2_textChanged(const QString &arg1);

    void on_comboBox_floodlevel_m2_currentIndexChanged(int index);

    void initMapWidget();

    void displayBudgetTable(double totalBudget);

    void on_lineEdit_barrederecherche_m3_textChanged(const QString &arg1);

    void on_pushButton_sort_m3_clicked();

    void on_pushButton_export_m3_clicked();


//-------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------Sewer Module---------------------------------------------
//-------------------------------------------------------------------------------------------------------------------

    void clearSewerForm();

    void on_pushButton_employee_S3_clicked();

    void on_pushButton_municipality_S3_clicked();

    void on_pushButton_employee_S2_clicked();

    void on_pushButton_municipality_S2_clicked();

    void on_pushButton_logout_S2_clicked();

    void on_pushButton_logout_S3_clicked();

    void on_pushButton_employee_S1_clicked();

    void on_pushButton_municipality_S1_clicked();

    void on_pushButton_logout_S1_clicked();

    void on_pushButton_SewerAI_S1_clicked();

    void on_pushButton_back_S2_clicked();

    void on_pushButton_back_S3_clicked();

    void on_pushButton_center_S2_clicked();

    void on_pushButton_center_S3_clicked();

    void on_pushButton_center_S1_clicked();

    void on_pushButton_team_S1_clicked();

    void on_pushButton_team_S2_clicked();

    void on_pushButton_team_S3_clicked();

    void on_pushButton_confirm_S1_clicked();

    void on_tableView_S1_doubleClicked(const QModelIndex &index);

    void on_tableView_S1_clicked(const QModelIndex &index);

    void on_pushButton_deselectionner_S1_clicked();

    void generateSewerStats();

    void on_pushButton_sort_S1_clicked();

    void on_lineEdit_search_S1_textChanged(const QString &arg1);

    void on_pushButton_export_S1_clicked();


    void on_comboBoxRegion_S2_currentTextChanged(const QString &arg1);
    void on_slWaterW_S2_valueChanged(int value);

    void on_slBlockageW_S2_valueChanged(int value);

    void on_pushButton_PredictIQ_S2_clicked();

    void on_pushButton_weather_S2_clicked();

     void on_btnGeneratePlan_S2_clicked();

     void on_pushButton_viewMaps_S2_clicked();

     void on_comboBoxSewerID_S3_currentTextChanged(const QString &arg1);

     void on_toolButtonMap_S3_clicked();

     void on_pushButton_SewerAI_S3_clicked();

     void updatePredictionS3();

     void on_dateEdit_S3_dateChanged(const QDate &date);

     void on_dateEdit_to_S3_dateChanged(const QDate &date);

     void on_pushButton_saveplan_S2_clicked();

     bool parseLocation(const QString &location, double &lat, double &lon);

     double distanceBetween(double lat1, double lon1, double lat2, double lon2);

     QList<int> buildOptimizedOrder(const QList<double> &lats, const QList<double> &lons);

     void readidsewer(const QString &line);

     void on_pushButton_cancel_map_s1_clicked();

     void on_pushButton_autoloc_s1_clicked();

     void onLocationPicked_s(double lat, double lng);

     void on_pushButton_confirm_map_s1_clicked();

    //-------------------------------------------------------------------------------
    //-----------------------------CollectionTeam Module-----------------------------
    //-------------------------------------------------------------------------------

     void clearCollectionTeam();

     void on_pushButton_employeem_c1_clicked();

    void on_pushButton_municipalitiesm_c1_clicked();

    void on_pushButton_sewersm_c1_clicked();

    void on_pushButton_rcentersm_c1_clicked();

    void on_pushButton_logout_c1_clicked();

    void on_pushButton_viewteamhealth_c1_clicked();

    void on_pushButton_viewteamranking_c1_clicked();

    void on_pushButton_employeem_c2_clicked();

    void on_pushButton_municipalitiesm_c2_clicked();

    void on_pushButton_sewersm_c2_clicked();

    void on_pushButton_rcentersm_c2_clicked();

    void on_pushButton_logout_c2_clicked();

    void on_pushButton_employeem_c3_clicked();

    void on_pushButton_municipalitiesm_c3_clicked();

    void on_pushButton_sewersm_c3_clicked();

    void on_pushButton_rcentersm_c3_clicked();

    void on_pushButton_logout_c3_clicked();

    void on_pushButton_back_c2_clicked();

    void on_pushButton_back_c3_clicked();

    void on_pushButton_confirm_c1_clicked();

    void on_pushButton_affectteam_c2_clicked();

    void on_tableView_listteams_c1_doubleClicked(const QModelIndex &index);

    void on_tableView_listteams_c1_clicked(const QModelIndex &index);

    void on_pushButton_restore_c1_clicked();

    void on_pushButton_tristatus_c1_clicked();

    void on_lineEdit_barrederecherche_c1_textChanged(const QString &arg1);

    void on_pushButton_export_c1_clicked();

    void setupChartCollectionTeam();

    void checkAuditRequirement(int teamID, int lastRating);

    void on_tableView_rankteams_c3_doubleClicked(const QModelIndex &index);

    //--------------------------------------------------------------------------------
    //----------------------------RECYCLING MODULE------------------------------------
    //--------------------------------------------------------------------------------

    void on_pushButton_saturation_r1_clicked();

    void on_pushButton_emails_r1_clicked();

    void on_pushButton_centerm_r2_clicked();

    void on_pushButton_centerm_r3_clicked();

    void on_pushButton_employeem_r1_clicked();

    void on_pushButton_municipalitiesm_r1_clicked();

    void on_pushButton_sewersm_r1_clicked();

    void on_pushButton_logout_r1_clicked();

    void on_pushButton_employeem_r2_clicked();

    void on_pushButton_municipalitiesm_r2_clicked();

    void on_pushButton_sewersm_r2_clicked();

    void on_pushButton_logout_r2_clicked();

    void on_pushButton_employeem_r3_clicked();

    void on_pushButton_municipalitiesm_r3_clicked();

    void on_pushButton_sewersm_r3_clicked();

    void on_pushButton_logout_r3_clicked();

    void on_pushButton_employeem_r4_clicked();

    void on_pushButton_municipalitiesm_r4_clicked();

    void on_pushButton_sewersm_r4_clicked();

    void on_pushButton_logout_r4_clicked();

    void on_pushButton_collectiontm_r2_clicked();

    void on_pushButton_collectiontm_r3_clicked();

    void on_pushButton_collectiontm_r1_clicked();

    void on_pushButton_collectiontm_r4_clicked();

    void on_pushButton_confirm_r1_clicked();

    void on_tableView_r1_doubleClicked(const QModelIndex &index);

    void on_tableView_r1_clicked(const QModelIndex &index);

    void on_pushButton_deselect_r1_clicked();

    void on_lineEdit_search_r1_textChanged(const QString &arg1);

    void on_pushButton_tristatus_r1_clicked();

    void on_pushButton_export_r1_clicked();

    void resetform_r1();

    void setupChart_r1();

    void predictcalcul_r();

    void saturationsStats_r();

    bool sendMailQt_r(const QString &fromEmail,const QString &fromName,const QString &toEmail,const QString &toName,const QString &subject,const QString &body,
        const QString &smtpHost,quint16 smtpPort,const QString &username,const QString &password,bool useSsl
        );

    void on_pushButton_apply_r4_clicked();

    void on_pushButton_esettings_r3_clicked();

    void on_pushButton_affectation_r4_clicked();

    void fixtabaffect_r();

    void initMailSettings_r();

    void toarduinoCenter_r(QString data);

    void reloadCenterStats_r();

    void on_pushButton_environmentPredict_S1_clicked();

    void on_pushButton_voiceAssistant_clicked();

private:
    Ui::GWasteCollection *ui;
    QString serialBuffer;
    Arduino A;
    int ret;
    QTextToSpeech *zigooSpeech;
    QString generateZigooBotAnswer(const QString &question);
    QString getAiFolderPath() const;
    QString buildZigooBrainContext();
    QString askZigooBrainAI(const QString &question, const QString &context);

    // Employee
    Employee Etmp, Session;
    int questionTentative = 0, answerTentative = 0;
    void applyEmployeePermissions();
    void login();
    void clearEmployeeForm();
    void logout();
    void animateBarSet_e(QBarSet *set);
    void displayEmployeeStatistics();
    int getSmallestAvailableID(const QVector<int>& usedIDs);
    int getFingerprintIDForEmployee();
    void processFingerprint(QString line);

    //municipality
    int Municipality_id_Maquette=1;
    Municipality Mtmp;
    void animateBarSet(QBarSet *set);
    double m_pickedLat = 0.0;
    double m_pickedLng = 0.0;
    QQuickWidget  *m_mapWidget  = NULL;
    bool           m_mapLoaded  = false;
    QVariantList   buildMunicipalityData();
    void           loadMapData();
    QMap<QString, QMap<QString, double>> getRegionBounds() const;
    QStandardItemModel *m_budgetModel = NULL;
    QMap<QString, QMap<QString, double>> regionBounds;
    void clear_m();

    // Sewer
    Sewer Stemp;
    int currentChanceOfRain = 0;
    QString mapsUrlS2;
    QMap<int, int> lastSewerLevels;
    void readSensorValue(const QString &line);
    double s_pickedLat = 0.0;
    double s_pickedLng = 0.0;

    // Collection Team
    CollectionTeam Ctmp;

    // Recycling Center
    RecyclingCenter Rtmp;
    int selectedCenterID = -1,name_r=1,teamid_r=1,statut_r=1,waste_r=1,date_r=1,rname_r=1,fmail_r=1,Active_Center=1;
};
#endif // GWASTECOLLECTION_H
