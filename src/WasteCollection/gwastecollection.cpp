#include "gwastecollection.h"
#include "ui_gwastecollection.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>
#include <QtCharts>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QGraphicsLayout>
#include <QSortFilterProxyModel>
#include <QDir>
#include <QPdfWriter>
#include <QPainter>
#include <QDateTime>
#include <QCryptographicHash>
#include <QtNetwork/QSslSocket>
#include <QQuickWidget>
#include <QQmlContext>
#include <QVariantList>
#include <QVariantMap>
#include <QVBoxLayout>
#include <QTimer>
#include <QCloseEvent>
#include <QMap>
#include "employee.h"
#include "municipality.h"
#include "sewer.h"
#include "collectionteam.h"
#include "recyclingcenter.h"
#include "arduino.h"
#include <QProcess>
#include <QCoreApplication>
#include <QPixmap>
#include <QDir>
#include <QCoreApplication>
#include <QProcess>
#include <QStandardPaths>
#include <QFileInfo>
#include <QApplication>
#include <QProcessEnvironment>

GWasteCollection::GWasteCollection(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::GWasteCollection)
{
    ui->setupUi(this);

    ui->pushButton_voiceAssistant->setText("ZigooBot   ");
    ui->pushButton_voiceAssistant->setIcon(QIcon(":/images/assistantvocal_transparent.png"));
    ui->pushButton_voiceAssistant->setIconSize(QSize(70, 70));
    ui->pushButton_voiceAssistant->setCursor(Qt::PointingHandCursor);
    ui->pushButton_voiceAssistant->setVisible(false);
    zigooSpeech = new QTextToSpeech(this);

    ret=A.connect_arduino(); // lancer la connexion à arduino
    switch(ret){
    case(0):qDebug() << "arduino is available and connected to : " << A.getarduino_port_name();
        break;
    case(1):qDebug() << "arduino is available but not connected to :" << A.getarduino_port_name();
        break;
    case(-1):qDebug() << "arduino is not available";
    }
    QObject::connect(A.getserial(),SIGNAL(readyRead()),this,SLOT(handleSerialData()));

    ui->tableView_list_e4->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView_list_e4->setModel(Etmp.displayEmployees());
    displayEmployeeStatistics();
    if (ret != 0)
    {
        ui->pushButton_fingerprint_e1->hide();
        ui->label_fingerprint_e1->hide();
        ui->pushButton_fingerprint_e3->hide();
        ui->label_fingerprint_e3->hide();
        ui->label_new_e3->move(50, 80);
        ui->lineEdit_new_e3->move(210, 80);
        ui->label_repeat_e3->move(25, 120);
        ui->lineEdit_repeat_e3->move(210, 120);
        ui->label_eye_e3->move(360, 100);
        ui->pushButton_show_e3->move(358, 100);
    }
    else
        A.write_to_arduino("led_off\n");

    ui->tableView_listemunicipality_m1->setModel(Mtmp.displayMunicipality());
    displayMunicipalityStatistics();
    initMapWidget();
    ui->tableView_listemunicipality_m1->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    ui->comboBox_municialityname_s1->setModel(Mtmp.displayMunicipality());
    ui->comboBox_municialityname_s1->setModelColumn(1);
    ui->tableView_S1->setModel(Stemp.displaySewer());
    generateSewerStats();
    ui->comboBoxRegion_S2->setModel(Stemp.getRegionsModelForAI());
    ui->comboBoxRegion_S2->setModelColumn(0);

    ui->tableView_listteams_c1->setModel(Ctmp.DisplayTeams());
    ui->tableView_loadscore_c2->setModel(Ctmp.displayTeamsToAffect());
    ui->tableView_intervention_c2->setModel(Ctmp.displayIntervenesForAffectC());
    ui->tableView_rankteams_c3->setModel(Ctmp.rankTeams());
    setupChartCollectionTeam();
    ui->comboBox_municialityname_r1->setModel(Mtmp.displayMunicipality());
    ui->comboBox_municialityname_r1->setModelColumn(1);
    ui->tableView_r1->setModel(Rtmp.displayCenter());
    ui->tableView_r1->hideColumn(7);
    ui->tableView_saturated_r2->setModel(Rtmp.getCentersSaturated());
    fixtabaffect_r();
    setupChart_r1();

    ui->SWWasteCollection->setCurrentIndex(0);
    ui->stackedWidget_e->setCurrentIndex(0);
}

GWasteCollection::~GWasteCollection()

{
    delete ui;
}

void GWasteCollection::closeEvent(QCloseEvent *event)
{
    if (ret == 0)
    {
        A.write_to_arduino("c\n");
        A.write_to_arduino("led_off\n");
        A.write_to_arduino("disconnect\n");
        QThread::msleep(100);
        event->accept();
    }
}

void GWasteCollection::handleSerialData()
{
    serialBuffer += QString::fromUtf8(A.read_from_arduino());

    while (serialBuffer.contains("\n")) {
        int index = serialBuffer.indexOf("\n");

        QString line = serialBuffer.left(index).trimmed();
        serialBuffer.remove(0, index + 1);

        if (line.contains("Fingerprint")) {
            processFingerprint(line);
        }
        else if (line.contains("SEWER")) {
            readSensorValue(line);
        }
        else if (line.contains("Center")) {
            toarduinoCenter_r(line);
        }
        else if (line.contains("capacity")) {
            readidsewer(line);
        }
    }
}

void GWasteCollection::on_pushButton_voiceAssistant_clicked()
{

    static bool assistantBusy = false;

    if (assistantBusy)
    {
        QMessageBox::information(this,
                                 "ZigooBot AI",
                                 "ZigooBot is already running. Please wait.");
        return;
    }

    assistantBusy = true;

    auto resetAssistant = [this]()
    {
        ui->pushButton_voiceAssistant->setText("ZigooBot   ");
        ui->pushButton_voiceAssistant->setEnabled(true);
        assistantBusy = false;
    };

    QString aiFolder = getAiFolderPath();

    if (aiFolder.isEmpty())
    {
        resetAssistant();

        QMessageBox::critical(this,
                              "ZigooBot AI",
                              "zigoocareAi folder not found.");
        return;
    }

    QString scriptPath = QDir(aiFolder).filePath("listen_zigoobot.py");

    if (!QFileInfo::exists(scriptPath))
    {
        resetAssistant();

        QMessageBox::critical(this,
                              "ZigooBot AI",
                              "Voice assistant script not found:\n" + scriptPath);
        return;
    }

    QString pythonProgram = QStandardPaths::findExecutable("py");

    if (pythonProgram.isEmpty())
        pythonProgram = QStandardPaths::findExecutable("python");

    if (pythonProgram.isEmpty())
    {
        resetAssistant();

        QMessageBox::critical(this,
                              "ZigooBot AI",
                              "Python executable not found.");
        return;
    }

    ui->pushButton_voiceAssistant->setText("Listening...");
    ui->pushButton_voiceAssistant->setEnabled(false);
    qApp->processEvents();

    QProcess process;
    process.setWorkingDirectory(aiFolder);
    process.start(pythonProgram, QStringList() << scriptPath);

    if (!process.waitForStarted(5000))
    {
        resetAssistant();

        QMessageBox::critical(this,
                              "ZigooBot AI",
                              "Voice assistant could not start:\n" + process.errorString());
        return;
    }

    if (!process.waitForFinished(25000))
    {
        process.kill();
        process.waitForFinished();

        resetAssistant();

        QMessageBox::critical(this,
                              "ZigooBot AI",
                              "Voice recognition timeout.\nPlease try again and speak clearly.");
        return;
    }

    QString output = QString::fromUtf8(process.readAllStandardOutput()).trimmed();
    QString error = QString::fromUtf8(process.readAllStandardError()).trimmed();

    if (!error.isEmpty())
    {
        resetAssistant();

        QMessageBox::warning(this,
                             "ZigooBot AI",
                             error);
        return;
    }

    if (output.isEmpty())
    {
        resetAssistant();

        QMessageBox::warning(this,
                             "ZigooBot AI",
                             "No question detected.\nPlease check your microphone and try again.");
        return;
    }

    QString cleanOutput = output.trimmed();
    QString questionToShow = cleanOutput;

    questionToShow.remove("FR:");
    questionToShow.remove("EN:");
    questionToShow = questionToShow.trimmed();

    QString context = buildZigooBrainContext();

    QString questionForAI;

    if (cleanOutput.startsWith("EN:"))
    {
        questionForAI =
            "ANSWER_LANGUAGE: English\n"
            "IMPORTANT: Answer only in English. Do not answer in French.\n"
            "QUESTION: " + questionToShow;
    }
    else
    {
        questionForAI =
            "ANSWER_LANGUAGE: French\n"
            "IMPORTANT: Réponds uniquement en français. Ne réponds pas en anglais.\n"
            "QUESTION: " + questionToShow;
    }

    ui->pushButton_voiceAssistant->setText("Thinking...");
    qApp->processEvents();

    QString answer = askZigooBrainAI(questionForAI, context);

    if (answer.startsWith("ERROR:"))
    {
        resetAssistant();

        QMessageBox::critical(this,
                              "ZigooBrain AI",
                              answer);
        return;
    }

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("ZigooBot AI Assistant");
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText("Question:\n" + questionToShow +
                   "\n\nAnswer:\n" + answer);
    msgBox.setStandardButtons(QMessageBox::Ok);

    // Voice with Windows PowerShell instead of QTextToSpeech
    QProcess ttsProcess;
    QString powershellProgram = QStandardPaths::findExecutable("powershell");

    if (powershellProgram.isEmpty())
        powershellProgram = "powershell.exe";

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("ZIGOO_TTS_TEXT", answer);

    if (cleanOutput.startsWith("FR:"))
        env.insert("ZIGOO_TTS_LANG", "fr-FR");
    else
        env.insert("ZIGOO_TTS_LANG", "en-US");

    ttsProcess.setProcessEnvironment(env);

    QString ttsCommand =
        "Add-Type -AssemblyName System.Speech; "
        "$s = New-Object System.Speech.Synthesis.SpeechSynthesizer; "
        "$voices = $s.GetInstalledVoices() | Where-Object { $_.VoiceInfo.Culture.Name -like ($env:ZIGOO_TTS_LANG + '*') }; "
        "if ($voices.Count -gt 0) { $s.SelectVoice($voices[0].VoiceInfo.Name) }; "
        "$s.Rate = 0; "
        "$s.Volume = 100; "
        "$s.Speak($env:ZIGOO_TTS_TEXT);";

    ttsProcess.start(powershellProgram,
                     QStringList()
                         << "-NoProfile"
                         << "-ExecutionPolicy"
                         << "Bypass"
                         << "-Command"
                         << ttsCommand);

    msgBox.exec();

    if (ttsProcess.state() != QProcess::NotRunning)
    {
        ttsProcess.kill();
        ttsProcess.waitForFinished(1000);
    }

    resetAssistant();
}

QString GWasteCollection::generateZigooBotAnswer(const QString &question)
{
    QString q = question.toLower();

    bool french =
        q.contains("fr:") ||
        q.contains("quel") ||
        q.contains("quelle") ||
        q.contains("combien") ||
        q.contains("égout") ||
        q.contains("egout") ||
        q.contains("dangereux") ||
        q.contains("critique") ||
        q.contains("résumé") ||
        q.contains("resume") ||
        q.contains("situation");

    Sewer sewerModel;

    if (q.contains("dangerous") ||
        q.contains("most dangerous") ||
        q.contains("highest risk") ||
        q.contains("dangereux") ||
        q.contains("plus dangereux") ||
        q.contains("risque élevé") ||
        q.contains("risque eleve"))
    {
        int sewerId = 0;
        int waterLevel = 0;
        int blockageRate = 0;

        if (sewerModel.getMostDangerousSewer(sewerId, waterLevel, blockageRate))
        {
            if (french)
            {
                return QString(
                    "L'égout le plus dangereux est l'égout numéro %1. "
                    "Son niveau d'eau est de %2 pour cent et son taux de blocage est de %3 pour cent. "
                    "Une intervention urgente est recommandée."
                ).arg(sewerId).arg(waterLevel).arg(blockageRate);
            }

            return QString(
                "The most dangerous sewer is sewer number %1. "
                "It has a water level of %2 percent and a blockage rate of %3 percent. "
                "Urgent intervention is recommended."
            ).arg(sewerId).arg(waterLevel).arg(blockageRate);
        }

        return french
            ? "Je n'ai pas pu trouver l'égout le plus dangereux."
            : "I could not find the most dangerous sewer.";
    }

    if (q.contains("critical") ||
        q.contains("urgent") ||
        q.contains("critique") ||
        q.contains("urgence") ||
        q.contains("urgents") ||
        q.contains("combien"))
    {
        int count = 0;

        if (sewerModel.getCriticalSewersCount(count))
        {
            if (french)
            {
                return QString(
                    "Il y a actuellement %1 égouts critiques qui nécessitent une surveillance ou une intervention urgente."
                ).arg(count);
            }

            return QString(
                "There are currently %1 critical sewers that need urgent monitoring or intervention."
            ).arg(count);
        }

        return french
            ? "Je n'ai pas pu vérifier le nombre d'égouts critiques."
            : "I could not check the number of critical sewers.";
    }

    if (q.contains("summary") ||
        q.contains("situation") ||
        q.contains("resume") ||
        q.contains("résumé") ||
        q.contains("etat") ||
        q.contains("état"))
    {
        int total = 0;
        int critical = 0;

        if (sewerModel.getSewerSituationSummary(total, critical))
        {
            if (french)
            {
                return QString(
                    "ZigooCare surveille actuellement %1 égouts. "
                    "%2 d'entre eux sont considérés comme critiques. "
                    "La municipalité doit prioriser ces égouts, surtout avant les fortes pluies."
                ).arg(total).arg(critical);
            }

            return QString(
                "ZigooCare is currently monitoring %1 sewers. "
                "%2 of them are considered critical. "
                "The municipality should prioritize these sewers, especially before heavy rain."
            ).arg(total).arg(critical);
        }

        return french
            ? "Je n'ai pas pu générer le résumé de la situation."
            : "I could not generate the situation summary.";
    }

    if (q.contains("municipality") ||
        q.contains("municipalities") ||
        q.contains("municipalité") ||
        q.contains("municipalite") ||
        q.contains("zone"))
    {
        QString municipality;
        int nbSewers = 0;
        int avgWater = 0;
        int avgBlockage = 0;
        int score = 0;

        if (sewerModel.getHighestRiskMunicipality(municipality,
                                                  nbSewers,
                                                  avgWater,
                                                  avgBlockage,
                                                  score))
        {
            if (french)
            {
                return QString(
                    "La municipalité la plus risquée est %1. "
                    "Elle contient %2 égouts surveillés, avec un niveau d'eau moyen de %3 pour cent "
                    "et un taux de blocage moyen de %4 pour cent. "
                    "Le score de risque est de %5 pour cent."
                ).arg(municipality)
                 .arg(nbSewers)
                 .arg(avgWater)
                 .arg(avgBlockage)
                 .arg(score);
            }

            return QString(
                "The highest risk municipality is %1. "
                "It has %2 monitored sewers, with an average water level of %3 percent "
                "and an average blockage rate of %4 percent. "
                "The risk score is %5 percent."
            ).arg(municipality)
             .arg(nbSewers)
             .arg(avgWater)
             .arg(avgBlockage)
             .arg(score);
        }

        return french
            ? "Je n'ai pas pu identifier la municipalité la plus risquée."
            : "I could not identify the highest risk municipality.";
    }

    if (q.contains("what should") ||
        q.contains("do now") ||
        q.contains("action") ||
        q.contains("que faire") ||
        q.contains("quoi faire") ||
        q.contains("intervention") ||
        q.contains("recommand"))
    {
        if (french)
        {
            return
                "La municipalité doit d'abord inspecter les égouts ayant le niveau d'eau et le taux de blocage les plus élevés. "
                "Ensuite, les équipes de nettoyage doivent intervenir dans les zones les plus critiques avant l'arrivée de fortes pluies.";
        }

        return
            "The municipality should first inspect the sewers with the highest water level and blockage rate. "
            "Then, cleaning teams should intervene in the most critical zones before heavy rain starts.";
    }

    if (french)
    {
        return
            "Je peux répondre aux questions sur les égouts dangereux, les égouts critiques, la situation générale, "
            "les municipalités à risque et les priorités d'intervention. "
            "Par exemple, demandez-moi : quel est l'égout le plus dangereux ?";
    }

    return
        "I can answer questions about dangerous sewers, critical sewers, the general situation, "
        "high-risk municipalities, and intervention priorities. "
        "For example, ask me: which sewer is the most dangerous?";
}

QString GWasteCollection::buildZigooBrainContext()
{
    QString context;

    context += "ZigooCare project description:\n";
    context += "ZigooCare is a smart city application for Tunisian municipalities. ";
    context += "It manages sewer monitoring, waste collection, recycling centers, ";
    context += "environmental sustainability, flood prevention, intervention planning, ";
    context += "weather risk, and municipal decision support.\n\n";

    context += "Current user role: " + Session.getPosition() + "\n";
    context += "Current rain risk: " + QString::number(currentChanceOfRain) + "%\n\n";

    QSqlQueryModel *sewerModel = Stemp.displaySewer();

    int totalSewers = sewerModel ? sewerModel->rowCount() : 0;
    int criticalSewers = 0;
    int maxRiskScore = -1;
    int mostDangerousSewerId = 0;
    int mostDangerousWater = 0;
    int mostDangerousBlockage = 0;
    double totalWater = 0;
    double totalBlockage = 0;

    if (sewerModel)
    {
        for (int i = 0; i < sewerModel->rowCount(); i++)
        {
            int sewerId = sewerModel->data(sewerModel->index(i, 0)).toInt();
            int waterLevel = sewerModel->data(sewerModel->index(i, 4)).toInt();
            int blockageRate = sewerModel->data(sewerModel->index(i, 5)).toInt();

            totalWater += waterLevel;
            totalBlockage += blockageRate;

            if (waterLevel >= 80 || blockageRate >= 80)
                criticalSewers++;

            int riskScore = waterLevel + blockageRate;

            if (riskScore > maxRiskScore)
            {
                maxRiskScore = riskScore;
                mostDangerousSewerId = sewerId;
                mostDangerousWater = waterLevel;
                mostDangerousBlockage = blockageRate;
            }
        }
    }

    double avgWater = totalSewers > 0 ? totalWater / totalSewers : 0;
    double avgBlockage = totalSewers > 0 ? totalBlockage / totalSewers : 0;

    context += "Sewer module:\n";
    context += "Total monitored sewers: " + QString::number(totalSewers) + "\n";
    context += "Critical sewers: " + QString::number(criticalSewers) + "\n";
    context += "Average water level: " + QString::number(avgWater, 'f', 1) + "%\n";
    context += "Average blockage rate: " + QString::number(avgBlockage, 'f', 1) + "%\n";
    context += "Most dangerous sewer ID: " + QString::number(mostDangerousSewerId) + "\n";
    context += "Most dangerous sewer water level: " + QString::number(mostDangerousWater) + "%\n";
    context += "Most dangerous sewer blockage rate: " + QString::number(mostDangerousBlockage) + "%\n\n";

    QSqlQueryModel *teamModel = Ctmp.DisplayTeams();

    int totalTeams = teamModel ? teamModel->rowCount() : 0;
    int availableTeams = 0;
    int unavailableTeams = 0;

    if (teamModel)
    {
        for (int i = 0; i < teamModel->rowCount(); i++)
        {
            QString status = teamModel->data(teamModel->index(i, 4)).toString().toLower();

            if (status == "available")
                availableTeams++;
            else if (status == "unavailable")
                unavailableTeams++;
        }
    }

    context += "Collection team module:\n";
    context += "Total teams: " + QString::number(totalTeams) + "\n";
    context += "Available teams: " + QString::number(availableTeams) + "\n";
    context += "Unavailable teams: " + QString::number(unavailableTeams) + "\n\n";

    QSqlQueryModel *centerModel = Rtmp.displayCenter();

    int totalCenters = centerModel ? centerModel->rowCount() : 0;
    int saturatedCenters = 0;

    if (centerModel)
    {
        for (int i = 0; i < centerModel->rowCount(); i++)
        {
            QString status = centerModel->data(centerModel->index(i, 6)).toString().toLower();

            if (status == "saturated")
                saturatedCenters++;
        }
    }

    context += "Recycling center module:\n";
    context += "Total recycling centers: " + QString::number(totalCenters) + "\n";
    context += "Saturated recycling centers: " + QString::number(saturatedCenters) + "\n\n";

    context += "Decision rule:\n";
    context += "High water level or high blockage rate means higher flood and environmental risk. ";
    context += "Critical sewers should be inspected before heavy rain. ";
    context += "Available collection teams should be assigned first to the highest-risk zones.\n";

    return context;
}
QString GWasteCollection::askZigooBrainAI(const QString &question, const QString &context)
{
    QString aiFolder = getAiFolderPath();

    if (aiFolder.isEmpty())
        return "ERROR: zigoocareAi folder not found.";

    QString scriptPath = QDir(aiFolder).filePath("zigoobrain_local.py");

    if (!QFileInfo::exists(scriptPath))
        return "ERROR: ZigooBrain local AI script not found:\n" + scriptPath;

    QString pythonProgram = QStandardPaths::findExecutable("py");

    if (pythonProgram.isEmpty())
        pythonProgram = QStandardPaths::findExecutable("python");

    if (pythonProgram.isEmpty())
        return "ERROR: Python executable not found.";

    QJsonObject payload;
    payload["question"] = question;
    payload["context"] = context;

    QJsonDocument doc(payload);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

    QProcess process;
    process.setWorkingDirectory(aiFolder);
    process.start(pythonProgram, QStringList() << scriptPath);

    if (!process.waitForStarted(5000))
        return "ERROR: ZigooBrain AI could not start:\n" + process.errorString();

    process.write(jsonData);
    process.closeWriteChannel();

    if (!process.waitForFinished(120000))
    {
        process.kill();
        process.waitForFinished();
        return "ERROR: ZigooBrain AI timeout. Please check if Ollama is running.";
    }

    QString output = QString::fromUtf8(process.readAllStandardOutput()).trimmed();
    QString error = QString::fromUtf8(process.readAllStandardError()).trimmed();

    if (!error.isEmpty())
        return "ERROR: " + error;

    if (output.isEmpty())
        return "ERROR: No response received from ZigooBrain AI.";

    return output;
}

// -----------------------------------------------------------------
// -------------------- Employee Module ----------------------------
// -----------------------------------------------------------------

// Login Pages

void GWasteCollection::on_pushButton_show_e1_pressed()
{
    ui->lineEdit_password_e1->setEchoMode(QLineEdit::Normal);
}

void GWasteCollection::on_pushButton_show_e1_released()
{
    ui->lineEdit_password_e1->setEchoMode(QLineEdit::Password);
}

void GWasteCollection::on_pushButton_reset_e1_clicked()
{
    if (ret == 0)
    {
        A.write_to_arduino("c\n");
        A.write_to_arduino("led_off\n");
    }
    int id=ui->lineEdit_id_e1->text().toInt();
    if (id == 0)
        QMessageBox::critical(nullptr, QObject::tr("Missing ID"),
                              QObject::tr("You did not enter the employee ID.\n"
                                          "Click Cancel to exit."), QMessageBox::Cancel);
    else if (!Etmp.getEmployeeByID(id))
    {
        ui->lineEdit_id_e1->clear();
        QMessageBox::critical(nullptr, QObject::tr("Invalid ID"),
                              QObject::tr("This employee ID does not exist.\n"
                                          "Click Cancel to exit."), QMessageBox::Cancel);
    }
    else
    {
        ui->lineEdit_id_e1->clear();
        ui->lineEdit_password_e1->clear();
        Session=Etmp;
        if (Etmp.getPassword() == "")
        {
            ui->stackedWidget_e->setCurrentIndex(2);
        }
        else
        {
            QStringList presetQuestions = {
                "What is your mother's maiden name?",
                "What was your first pet's name?",
                "What is your favorite teacher's name?",
                "What city were you born in?",
                "What is your favorite food?"
            };
            QStringList allQuestions = Session.getOtherQuestions() + presetQuestions;
            allQuestions.removeDuplicates();
            std::random_device rd;
            std::mt19937 g(rd());
            std::shuffle(allQuestions.begin(), allQuestions.end(), g);
            QStringList selectedQuestions = allQuestions.mid(0, 5);
            selectedQuestions << Session.getQuestion();
            std::shuffle(selectedQuestions.begin(), selectedQuestions.end(), g);
            ui->comboBox_question_e2->clear();
            ui->comboBox_question_e2->addItem("");
            ui->comboBox_question_e2->addItems(selectedQuestions);
            ui->comboBox_question_e2->setEnabled(true);
            ui->lineEdit_answer_e2->setEnabled(false);
            ui->pushButton_confirm_e2->setEnabled(false);
            ui->lineEdit_new_e2->setEnabled(false);
            ui->lineEdit_repeat_e2->setEnabled(false);
            ui->pushButton_show_e2->setEnabled(false);
            ui->pushButton_reset_e2->setEnabled(false);
            ui->stackedWidget_e->setCurrentIndex(1);
        }
    }
}

void GWasteCollection::applyEmployeePermissions()
{
    bool isAdmin = (Session.getPosition() == "System Administrator");

    QString option = "System Administrator";
    int index = ui->comboBox_position_e4->findText(option);
    if (isAdmin) {
        if (index == -1)
            ui->comboBox_position_e4->addItem(option);
    } else {
        if (index != -1)
            ui->comboBox_position_e4->removeItem(index);
    }

    // Page e4
    ui->pushButton_municipality_e4->setEnabled(isAdmin);
    ui->pushButton_sewer_e4->setEnabled(isAdmin);
    ui->pushButton_team_e4->setEnabled(isAdmin);
    ui->pushButton_center_e4->setEnabled(isAdmin);

    // Page m1
    ui->pushButton_employee_m1->setEnabled(isAdmin);
    ui->pushButton_sewer_m1->setEnabled(isAdmin);
    ui->pushButton_team_m1->setEnabled(isAdmin);
    ui->pushButton_center_m1->setEnabled(isAdmin);

    // Page m2
    ui->pushButton_employee_m2->setEnabled(isAdmin);
    ui->pushButton_sewer_m2->setEnabled(isAdmin);
    ui->pushButton_team_m2->setEnabled(isAdmin);
    ui->pushButton_center_m2->setEnabled(isAdmin);

    // Page m3
    ui->pushButton_employee_m3->setEnabled(isAdmin);
    ui->pushButton_sewer_m3->setEnabled(isAdmin);
    ui->pushButton_team_m3->setEnabled(isAdmin);
    ui->pushButton_center_m3->setEnabled(isAdmin);

    // Page s1
    ui->pushButton_employee_S1->setEnabled(isAdmin);
    ui->pushButton_municipality_S1->setEnabled(isAdmin);
    ui->pushButton_team_S1->setEnabled(isAdmin);
    ui->pushButton_center_S1->setEnabled(isAdmin);

    // Page s2
    ui->pushButton_employee_S2->setEnabled(isAdmin);
    ui->pushButton_municipality_S2->setEnabled(isAdmin);
    ui->pushButton_team_S2->setEnabled(isAdmin);
    ui->pushButton_center_S2->setEnabled(isAdmin);

    // Page s3
    ui->pushButton_employee_S3->setEnabled(isAdmin);
    ui->pushButton_municipality_S3->setEnabled(isAdmin);
    ui->pushButton_team_S3->setEnabled(isAdmin);
    ui->pushButton_center_S3->setEnabled(isAdmin);

    // Page c1
    ui->pushButton_employeem_c1->setEnabled(isAdmin);
    ui->pushButton_municipalitiesm_c1->setEnabled(isAdmin);
    ui->pushButton_sewersm_c1->setEnabled(isAdmin);
    ui->pushButton_rcentersm_c1->setEnabled(isAdmin);

    // Page c2
    ui->pushButton_employeem_c2->setEnabled(isAdmin);
    ui->pushButton_municipalitiesm_c2->setEnabled(isAdmin);
    ui->pushButton_sewersm_c2->setEnabled(isAdmin);
    ui->pushButton_rcentersm_c2->setEnabled(isAdmin);

    // Page c3
    ui->pushButton_employeem_c3->setEnabled(isAdmin);
    ui->pushButton_municipalitiesm_c3->setEnabled(isAdmin);
    ui->pushButton_sewersm_c3->setEnabled(isAdmin);
    ui->pushButton_rcentersm_c3->setEnabled(isAdmin);

    // Page r1
    ui->pushButton_employeem_r1->setEnabled(isAdmin);
    ui->pushButton_municipalitiesm_r1->setEnabled(isAdmin);
    ui->pushButton_sewersm_r1->setEnabled(isAdmin);
    ui->pushButton_collectiontm_r1->setEnabled(isAdmin);

    // Page r2
    ui->pushButton_employeem_r2->setEnabled(isAdmin);
    ui->pushButton_municipalitiesm_r2->setEnabled(isAdmin);
    ui->pushButton_sewersm_r2->setEnabled(isAdmin);
    ui->pushButton_collectiontm_r2->setEnabled(isAdmin);

    // Page r3
    ui->pushButton_employeem_r3->setEnabled(isAdmin);
    ui->pushButton_municipalitiesm_r3->setEnabled(isAdmin);
    ui->pushButton_sewersm_r3->setEnabled(isAdmin);
    ui->pushButton_collectiontm_r3->setEnabled(isAdmin);

    // Page r4
    ui->pushButton_employeem_r4->setEnabled(isAdmin);
    ui->pushButton_municipalitiesm_r4->setEnabled(isAdmin);
    ui->pushButton_sewersm_r4->setEnabled(isAdmin);
    ui->pushButton_collectiontm_r4->setEnabled(isAdmin);
}

void GWasteCollection::login()
{
    if (ret == 0)
    {
        A.write_to_arduino("connect\n");
        A.write_to_arduino(Mtmp.getMunicipalityRiskMaquette(Municipality_id_Maquette).toUtf8()+"\n");
    }
    reloadCenterStats_r();

    applyEmployeePermissions();
    QString username = Session.getFirstName() + " " + Session.getLastName().at(0) + ".";
    ui->label_session->setText("User : " + username);
    ui->pushButton_voiceAssistant->setVisible(true);
    if (Session.getPosition() == "System Administrator" || Session.getPosition() == "Human Resources")
    {
        ui->tableView_list_e4->setModel(Etmp.displayEmployees());
        displayEmployeeStatistics();
        ui->stackedWidget_e->setCurrentIndex(3);
    }
    else if (Session.getPosition() == "Municipal Oversight")
    {
        ui->tableView_listemunicipality_m1->setModel(Mtmp.displayMunicipality());
        ui->SWWasteCollection->setCurrentIndex(1);
        ui->stackedWidget_m1->setCurrentIndex(0);
        ui->stackedWidget_form->setCurrentIndex(0);
    }
    else if (Session.getPosition() == "Ministry Agent")
    {
        ui->tableView_S1->setModel(Stemp.displaySewer());
        ui->SWWasteCollection->setCurrentIndex(2);
        ui->stackedWidgetS->setCurrentIndex(0);
    }
    else if (Session.getPosition() == "Operations Manager")
    {
        ui->tableView_listteams_c1->setModel(Ctmp.DisplayTeams());
        ui->SWWasteCollection->setCurrentIndex(3);
        ui->stackedWidget_c->setCurrentIndex(0);
    }
    else if (Session.getPosition() == "Center Representative")
    {
        ui->tableView_r1->setModel(Rtmp.displayCenter());
        ui->SWWasteCollection->setCurrentIndex(4);
        ui->stackedWidget_r->setCurrentIndex(0);
    }
}

void GWasteCollection::on_pushButton_login_e1_clicked()
{
    if (ret == 0)
    {
        A.write_to_arduino("c\n");
        A.write_to_arduino("led_off\n");
    }

    int id=ui->lineEdit_id_e1->text().toInt();
    if (id == 0)
        QMessageBox::critical(nullptr, QObject::tr("Missing ID"),
                              QObject::tr("You did not enter the employee ID.\n"
                                          "Click Cancel to exit."), QMessageBox::Cancel);
    else if (!Etmp.getEmployeeByID(id))
    {
        ui->lineEdit_id_e1->clear();
        ui->lineEdit_password_e1->clear();
        QMessageBox::critical(nullptr, QObject::tr("Invalid ID"),
                              QObject::tr("This employee ID does not exist.\n"
                                          "Click Cancel to exit."), QMessageBox::Cancel);
    }
    else
    {
        QString password=ui->lineEdit_password_e1->text();
        QByteArray hash = QCryptographicHash::hash((password + QString::number(id)).toUtf8(), QCryptographicHash::Sha256);
        if (Etmp.getPassword() == "")
        {
            ui->lineEdit_id_e1->clear();
            ui->lineEdit_password_e1->clear();
            Session=Etmp;
            ui->stackedWidget_e->setCurrentIndex(2);
        }
        else if (password == "")
        {
            QMessageBox::critical(nullptr, QObject::tr("Missing Password"),
                                  QObject::tr("You did not enter the password.\n"
                                              "Click Cancel to exit."), QMessageBox::Cancel);
        }
        else if (hash.toHex() != Etmp.getPassword())
        {
            ui->lineEdit_password_e1->clear();
            QMessageBox::critical(nullptr, QObject::tr("Invalid Password"),
                                  QObject::tr("The password is incorrect.\n"
                                              "Click Cancel to exit."), QMessageBox::Cancel);
        }
        else
        {
            ui->lineEdit_id_e1->clear();
            ui->lineEdit_password_e1->clear();
            Session=Etmp;
            login();
        }
    }
}

void GWasteCollection::on_comboBox_question_e2_currentTextChanged(const QString &arg1)
{
    if (arg1 != "")
    {
        questionTentative++;
        if (arg1 == Session.getQuestion())
        {
            ui->comboBox_question_e2->setEnabled(false);
            ui->lineEdit_answer_e2->setEnabled(true);
            ui->pushButton_confirm_e2->setEnabled(true);
        }
        else if (questionTentative >= 3)
        {
            QMessageBox::critical(nullptr, QObject::tr("Wrong question"),
                                  QObject::tr("You have reached the maximum number of tries.\n"
                                              "Click Cancel to exit."), QMessageBox::Cancel);
            questionTentative = 0;
            Session.clearEmployee();
            ui->stackedWidget_e->setCurrentIndex(0);
        }
    }
}

void GWasteCollection::on_pushButton_confirm_e2_clicked()
{
    QString answer = ui->lineEdit_answer_e2->text();
    if (answer == "")
    {
        QMessageBox::critical(nullptr, QObject::tr("Invalid answer"),
                              QObject::tr("You have not entered an answer.\n"
                                          "Click Cancel to exit."), QMessageBox::Cancel);
    }
    else
    {
        answerTentative++;
        QByteArray hash = QCryptographicHash::hash((answer.toUpper() + QString::number(Session.getEmployeeID())).toUtf8(), QCryptographicHash::Sha256);
        if (hash.toHex() == Session.getAnswer())
        {
            ui->lineEdit_answer_e2->setEnabled(false);
            ui->pushButton_confirm_e2->setEnabled(false);
            ui->lineEdit_new_e2->setEnabled(true);
            ui->lineEdit_repeat_e2->setEnabled(true);
            ui->pushButton_show_e2->setEnabled(true);
            ui->pushButton_reset_e2->setEnabled(true);
        }
        else if (answerTentative >= 3)
        {
            QMessageBox::critical(nullptr, QObject::tr("Wrong answer"),
                                  QObject::tr("You have reached the maximum number of tries.\n"
                                              "Click Cancel to exit."), QMessageBox::Cancel);
            questionTentative = 0;
            answerTentative = 0;
            ui->lineEdit_answer_e2->clear();
            Session.clearEmployee();
            ui->stackedWidget_e->setCurrentIndex(0);
        }
        else
        {
            QMessageBox::critical(nullptr, QObject::tr("Wrong answer"),
                                  QObject::tr("The answer given is incorrect.\n"
                                              "Click Cancel to exit."), QMessageBox::Cancel);
        }
    }
}

void GWasteCollection::on_pushButton_show_e2_pressed()
{
    ui->lineEdit_new_e2->setEchoMode(QLineEdit::Normal);
    ui->lineEdit_repeat_e2->setEchoMode(QLineEdit::Normal);
}

void GWasteCollection::on_pushButton_show_e2_released()
{
    ui->lineEdit_new_e2->setEchoMode(QLineEdit::Password);
    ui->lineEdit_repeat_e2->setEchoMode(QLineEdit::Password);
}

void GWasteCollection::on_pushButton_reset_e2_clicked()
{
    // Password input
    QString password=ui->lineEdit_new_e2->text();
    bool valid=true;
    if ((password.length() < 8) || (password.length() > 20))
        valid=false;
    else
    {
        int lowercase=0;
        int uppercase=0;
        int digit=0;
        int symbol=0;
        for (int i=0; i<password.length(); i++)
        {
            if ((password[i] >= 'a') && (password[i] <= 'z'))
            {
                lowercase++;
            }
            else if ((password[i] >= 'A') && (password[i] <= 'Z'))
            {
                uppercase++;
            }
            else if ((password[i] >= '0') && (password[i] <= '9'))
            {
                digit++;
            }
            else if ((password[i] == '@') || (password[i] == '$') || (password[i] == '!') || (password[i] == '%') || (password[i] == '*') || (password[i] == '?') || (password[i] == '&'))
            {
                symbol++;
            }
        }
        valid=lowercase && uppercase && digit && symbol;
    }
    if (!valid)
    {
        QMessageBox::critical(nullptr, QObject::tr("Invalid password"),
                              QObject::tr("The password must be between 8 and 20 characters and must include at least one lowercase letter, one uppercase letter, one digit and one symbol (@$!%*?&).\n"
                                          "Click Cancel to exit."), QMessageBox::Cancel);
    }
    else
    {
        // Password repeat
        QString repeat=ui->lineEdit_repeat_e2->text();
        valid=(repeat==password);
        if (!valid)
        {
            QMessageBox::critical(nullptr, QObject::tr("Invalid password"),
                                  QObject::tr("The passwords do not match.\n"
                                              "Click Cancel to exit."), QMessageBox::Cancel);
        }
        else
        {
            QByteArray hash = QCryptographicHash::hash((password + QString::number(Session.getEmployeeID())).toUtf8(), QCryptographicHash::Sha256);
            Session.setPassword(hash.toHex());
            Session.resetPassword();
            questionTentative = 0;
            answerTentative = 0;
            ui->lineEdit_answer_e2->clear();
            ui->lineEdit_new_e2->clear();
            ui->lineEdit_repeat_e2->clear();
            Session.clearEmployee();
            ui->stackedWidget_e->setCurrentIndex(0);
        }
    }
}

void GWasteCollection::on_pushButton_cancel_e2_clicked()
{
    questionTentative = 0;
    answerTentative = 0;
    ui->lineEdit_answer_e2->clear();
    ui->lineEdit_new_e2->clear();
    ui->lineEdit_repeat_e2->clear();
    Session.clearEmployee();
    ui->stackedWidget_e->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_show_e3_pressed()
{
    ui->lineEdit_new_e3->setEchoMode(QLineEdit::Normal);
    ui->lineEdit_repeat_e3->setEchoMode(QLineEdit::Normal);
}

void GWasteCollection::on_pushButton_show_e3_released()
{
    ui->lineEdit_new_e3->setEchoMode(QLineEdit::Password);
    ui->lineEdit_repeat_e3->setEchoMode(QLineEdit::Password);
}

void GWasteCollection::on_pushButton_confirm_e3_clicked()
{
    if (ret == 0)
    {
        A.write_to_arduino("c\n");
        A.write_to_arduino("led_off\n");
    }

    // Password input
    QString password=ui->lineEdit_new_e3->text();
    bool valid=true;
    if ((password.length() < 8) || (password.length() > 20))
        valid=false;
    else
    {
        int lowercase=0;
        int uppercase=0;
        int digit=0;
        int symbol=0;
        for (int i=0; i<password.length(); i++)
        {
            if ((password[i] >= 'a') && (password[i] <= 'z'))
            {
                lowercase++;
            }
            else if ((password[i] >= 'A') && (password[i] <= 'Z'))
            {
                uppercase++;
            }
            else if ((password[i] >= '0') && (password[i] <= '9'))
            {
                digit++;
            }
            else if ((password[i] == '@') || (password[i] == '$') || (password[i] == '!') || (password[i] == '%') || (password[i] == '*') || (password[i] == '?') || (password[i] == '&') || (password[i] == '+'))
            {
                symbol++;
            }
        }
        valid=lowercase && uppercase && digit && symbol;
    }
    if (!valid)
    {
        QMessageBox::critical(nullptr, QObject::tr("Invalid password"),
                              QObject::tr("The password must be between 8 and 20 characters and must include at least one lowercase letter, one uppercase letter, one digit and one symbol (@$!%*?&+).\n"
                                          "Click Cancel to exit."), QMessageBox::Cancel);
    }
    else
    {
        // Password repeat
        QString repeat=ui->lineEdit_repeat_e3->text();
        valid=(repeat==password);
        if (!valid)
        {
            QMessageBox::critical(nullptr, QObject::tr("Invalid password"),
                                  QObject::tr("The passwords do not match.\n"
                                              "Click Cancel to exit."), QMessageBox::Cancel);
        }
        else
        {
            // Question input
            QString question=ui->lineEdit_question_e3->text();
            if ((question.length() < 10) || (password.length() > 50))
                valid=false;
            else
            {
                int i;
                for (i=0; i<question.length(); i++)
                {
                    if (((question[i] >= 'a') && (question[i] <= 'z')) || ((question[i] >= 'A') && (question[i] <= 'Z')))
                        break;
                }
                valid=(i != question.length());
            }
            if (!valid)
            {
                QMessageBox::critical(nullptr, QObject::tr("Invalid question"),
                                      QObject::tr("The question must be between 10 and 50 characters and must include letters.\n"
                                                  "Click Cancel to exit."), QMessageBox::Cancel);
            }
            else
            {
                // Answer input
                QString answer=ui->lineEdit_answer_e3->text();
                if ((answer.length() < 2) || (answer.length() > 15))
                    valid=false;
                else if (answer.trimmed().isEmpty())
                {
                    valid=false;
                }
                if (!valid)
                {
                    QMessageBox::critical(nullptr, QObject::tr("Invalid answer"),
                                          QObject::tr("The answer must be between 2 and 15 characters.\n"
                                                      "Click Cancel to exit."), QMessageBox::Cancel);
                }
                else
                {
                    QByteArray passwordHash = QCryptographicHash::hash((password + QString::number(Session.getEmployeeID())).toUtf8(), QCryptographicHash::Sha256);
                    Session.setPassword(passwordHash.toHex());
                    Session.setQuestion(question);
                    QByteArray answerHash = QCryptographicHash::hash((answer.toUpper() + QString::number(Session.getEmployeeID())).toUtf8(), QCryptographicHash::Sha256);
                    Session.setAnswer(answerHash.toHex());
                    Session.setupEmployee();
                    ui->lineEdit_new_e3->clear();
                    ui->lineEdit_repeat_e3->clear();
                    ui->lineEdit_question_e3->clear();
                    ui->lineEdit_answer_e3->clear();
                    login();
                }
            }
        }
    }
}

// Employee Dashboard Sidebar

void GWasteCollection::clearEmployeeForm()
{
    ui->lineEdit_first_e4->clear();
    ui->lineEdit_last_e4->clear();
    ui->lineEdit_nic_e4->clear();
    ui->lineEdit_email_e4->clear();
    ui->comboBox_position_e4->setCurrentIndex(0);
    ui->lineEdit_search_e4->clear();
    ui->tableView_list_e4->clearSelection();
}

void GWasteCollection::logout()
{
    if (ret == 0) A.write_to_arduino("disconnect\n");

    Session.clearEmployee();
    ui->label_session->clear();
    ui->pushButton_voiceAssistant->setVisible(false);

    // Page e4
    clearEmployeeForm();

    // Page m1/m2/m3
    clear_m();

    // Page s1
    clearSewerForm();

    // Page s2
    ui->comboBoxRegion_S2->setCurrentIndex(0);

    // Page c1
    clearCollectionTeam();

    // Page r1
    ui->lineEdit_name_r1->clear();
    ui->lineEdit_adress_r1->clear();
    ui->lineEdit_email_r1->clear();
    ui->lineEdit_capacity_r1->clear();
    ui->lineEdit_mawc_r1->clear();
    ui->comboBox_municialityname_r1->setCurrentIndex(0);
    ui->radioButton_active_r1->setChecked(false);
    ui->radioButton_inactive_r1->setChecked(false);
    ui->radioButton_saturated_r1->setChecked(false);
    ui->lineEdit_search_r1->clear();
    ui->tableView_r1->clearSelection();

    ui->SWWasteCollection->setCurrentIndex(0);
    ui->stackedWidget_e->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_logout_e4_clicked()
{
    logout();
}

void GWasteCollection::on_pushButton_municipality_e4_clicked()
{
    clearEmployeeForm();
    ui->tableView_listemunicipality_m1->setModel(Mtmp.displayMunicipality());
    displayMunicipalityStatistics();
    ui->SWWasteCollection->setCurrentIndex(1);
    ui->stackedWidget_m1->setCurrentIndex(0);
    ui->stackedWidget_form->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_sewer_e4_clicked()
{
    clearEmployeeForm();
    ui->tableView_S1->setModel(Stemp.displaySewer());
    ui->SWWasteCollection->setCurrentIndex(2);
    ui->stackedWidgetS->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_team_e4_clicked()
{
    clearEmployeeForm();
    ui->tableView_listteams_c1->setModel(Ctmp.DisplayTeams());
    ui->SWWasteCollection->setCurrentIndex(3);
    ui->stackedWidget_c->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_center_e4_clicked()
{
    clearEmployeeForm();
    ui->tableView_r1->setModel(Rtmp.displayCenter());
    ui->SWWasteCollection->setCurrentIndex(4);
    ui->stackedWidget_r->setCurrentIndex(0);
}

// CRUD

void GWasteCollection::on_pushButton_confirm_e4_clicked()
{
    QModelIndex indext = ui->tableView_list_e4->selectionModel()->currentIndex();
    QString position = indext.sibling(indext.row(), 4).data().toString();

    // Check permission
    if ((Session.getPosition() != "System Administrator") && (position == "System Administrator"))
        QMessageBox::critical(nullptr, QObject::tr("Forbidden"),
                              QObject::tr("Only system administrators can edit system administrators.\n"
                                          "Click Cancel to exit."), QMessageBox::Cancel);
    else
    {
        // First Name input
        QString firstName=ui->lineEdit_first_e4->text();
        bool valid=true;
        if ((firstName.length() < 2) || (firstName.length() > 30))
            valid=false;
        else
        {
            for (int i=0; i<firstName.length(); i++)
            {
                if (!(((firstName[i] >= 'a') && (firstName[i] <= 'z')) || ((firstName[i] >= 'A') && (firstName[i] <= 'Z')) || (firstName[i] == ' ')))
                {
                    valid=false;
                    break;
                }
            }
        }
        if (!valid)
        {
            QMessageBox::critical(nullptr, QObject::tr("Invalid first name"),
                                  QObject::tr("The first name must include only letters or spaces.\n"
                                              "Click Cancel to exit."), QMessageBox::Cancel);
        }
        else
        {
            // Last Name input
            QString lastName=ui->lineEdit_last_e4->text();
            if ((lastName.length() < 2) || (lastName.length() > 30))
                valid=false;
            else
            {
                for (int i=0; i<lastName.length(); i++)
                {
                    if (!(((lastName[i] >= 'a') && (lastName[i] <= 'z')) || ((lastName[i] >= 'A') && (lastName[i] <= 'Z')) || (lastName[i] == ' ')))
                    {
                        valid=false;
                        break;
                    }
                }
            }
            if (!valid)
            {
                QMessageBox::critical(nullptr, QObject::tr("Invalid last name"),
                                      QObject::tr("The last name must include only letters or spaces.\n"
                                                  "Click Cancel to exit."), QMessageBox::Cancel);
            }
            else
            {
                // NIC input
                QString nic=ui->lineEdit_nic_e4->text();
                if (nic.length() != 8)
                    valid=false;
                else
                {
                    for (int i=0; i<nic.length(); i++)
                    {
                        if (!((nic[i] >= '0') && (nic[i] <= '9')))
                        {
                            valid=false;
                            break;
                        }
                    }
                }
                if (!valid)
                {
                    QMessageBox::critical(nullptr, QObject::tr("Invalid NIC"),
                                          QObject::tr("The NIC must include exactly 8 digits.\n"
                                                      "Click Cancel to exit."), QMessageBox::Cancel);
                }
                else if (Etmp.getEmployeeByNic(nic) && (!(ui->tableView_list_e4->selectionModel()->hasSelection()) || (nic != indext.sibling(indext.row(), 3).data().toString())))
                {
                    QMessageBox::critical(nullptr, QObject::tr("Invalid NIC"),
                                          QObject::tr("The NIC must be unique.\n"
                                                      "Click Cancel to exit."), QMessageBox::Cancel);
                }
                else
                {
                    // Email input
                    QString email=ui->lineEdit_email_e4->text();
                    if ((email.length() < 6) || email.length() > 254)
                        valid=false;
                    else
                    {
                        int atPos=-1;
                        int dotPos=-1;
                        for (int i=0; i<email.length(); i++)
                        {
                            if (email[i] == '@')
                                atPos=i;
                            else if (email[i] == '.')
                                dotPos=i;
                        }
                        if ((atPos > (dotPos - 2)) || (atPos < 1) || (dotPos > (email.length() - 3)))
                            valid=false;
                    }
                    if (!valid)
                    {
                        QMessageBox::critical(nullptr, QObject::tr("Invalid email"),
                                              QObject::tr("The email must be in a valid format.\n"
                                                          "Click Cancel to exit."), QMessageBox::Cancel);
                    }
                    else if (Etmp.getEmployeeByEmail(email) && (!(ui->tableView_list_e4->selectionModel()->hasSelection()) || (email != indext.sibling(indext.row(), 5).data().toString())))
                    {
                        QMessageBox::critical(nullptr, QObject::tr("Invalid email"),
                                              QObject::tr("The email must be unique.\n"
                                                          "Click Cancel to exit."), QMessageBox::Cancel);
                    }
                    else
                    {
                        // Position input
                        QString position=ui->comboBox_position_e4->currentText();
                        if (position == "")
                            valid=false;
                        if (!valid)
                        {
                            QMessageBox::critical(nullptr, QObject::tr("Invalid position"),
                                                  QObject::tr("You must select a position.\n"
                                                              "Click Cancel to exit."), QMessageBox::Cancel);
                        }
                        else
                        {
                            if (ui->tableView_list_e4->selectionModel()->hasSelection())
                            {
                                //Database update
                                QModelIndex index = ui->tableView_list_e4->selectionModel()->currentIndex();
                                int id = index.sibling(index.row(), 0).data().toInt();
                                Employee E(id, nic, firstName, lastName, position, email, "", "", "", 0);
                                bool test=E.updateEmployee();
                                if(test)
                                {
                                    ui->lineEdit_first_e4->clear();
                                    ui->lineEdit_last_e4->clear();
                                    ui->lineEdit_nic_e4->clear();
                                    ui->lineEdit_email_e4->clear();
                                    ui->comboBox_position_e4->setCurrentIndex(0);
                                    ui->tableView_list_e4->setModel(Etmp.displayEmployees());
                                    displayEmployeeStatistics();
                                    QMessageBox::information(nullptr, QObject::tr("OK"),
                                                             QObject::tr("Updated successfully.\n"
                                                                         "Click Cancel to exit."), QMessageBox::Cancel);
                                }
                                else
                                {
                                    QMessageBox::critical(nullptr, QObject::tr("Not OK"),
                                                          QObject::tr("Updating failed.\n"
                                                                      "Click Cancel to exit."), QMessageBox::Cancel);
                                }
                            }
                            else
                            {
                                // Database insertion
                                Employee E(0, nic, firstName, lastName, position, email, "", "", "", 0);
                                bool test=E.addEmployee();
                                if(test)
                                {
                                    ui->lineEdit_first_e4->clear();
                                    ui->lineEdit_last_e4->clear();
                                    ui->lineEdit_nic_e4->clear();
                                    ui->lineEdit_email_e4->clear();
                                    ui->comboBox_position_e4->setCurrentIndex(0);
                                    ui->tableView_list_e4->setModel(Etmp.displayEmployees());
                                    displayEmployeeStatistics();
                                    QMessageBox::information(nullptr, QObject::tr("OK"),
                                                             QObject::tr("Added successfully.\n"
                                                                         "Click Cancel to exit."), QMessageBox::Cancel);
                                }
                                else
                                {
                                    QMessageBox::critical(nullptr, QObject::tr("Not OK"),
                                                          QObject::tr("Adding failed.\n"
                                                                      "Click Cancel to exit."), QMessageBox::Cancel);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void GWasteCollection::on_tableView_list_e4_clicked(const QModelIndex &index)
{
    QString position = index.sibling(index.row(), 4).data().toString();
    ui->lineEdit_first_e4->setText(index.sibling(index.row(), 1).data().toString());
    ui->lineEdit_last_e4->setText(index.sibling(index.row(), 2).data().toString());
    ui->lineEdit_nic_e4->setText(index.sibling(index.row(), 3).data().toString());
    ui->lineEdit_email_e4->setText(index.sibling(index.row(), 5).data().toString());
    if ((Session.getPosition() != "System Administrator") && (position == "System Administrator"))
        ui->comboBox_position_e4->setCurrentIndex(0);
    else
        ui->comboBox_position_e4->setCurrentText(position);
}

void GWasteCollection::on_pushButton_restore_e4_clicked()
{
    if (ui->tableView_list_e4->selectionModel()->hasSelection())
    {
        ui->tableView_list_e4->clearSelection();
        ui->lineEdit_first_e4->clear();
        ui->lineEdit_last_e4->clear();
        ui->lineEdit_nic_e4->clear();
        ui->lineEdit_email_e4->clear();
        ui->comboBox_position_e4->setCurrentIndex(0);
    }
}

void GWasteCollection::on_tableView_list_e4_doubleClicked(const QModelIndex &index)
{
    int id = index.sibling(index.row(), 0).data().toInt();
    QString position = index.sibling(index.row(), 4).data().toString();

    // Check permission
    if ((Session.getPosition() != "System Administrator") && (position == "System Administrator"))
        QMessageBox::critical(nullptr, QObject::tr("Forbidden"),
                              QObject::tr("Only system administrators can delete system administrators.\n"
                                          "Click Cancel to exit."), QMessageBox::Cancel);
    else
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(
            this,
            "Confirmation",
            "Do you want to delete this employee ?",
            QMessageBox::Yes | QMessageBox::No
            );

        if (reply == QMessageBox::Yes) {
            if (Etmp.deleteEmployee(id)) {
                ui->lineEdit_first_e4->clear();
                ui->lineEdit_last_e4->clear();
                ui->lineEdit_nic_e4->clear();
                ui->lineEdit_email_e4->clear();
                ui->comboBox_position_e4->setCurrentIndex(0);
                ui->tableView_list_e4->setModel(Etmp.displayEmployees());
                displayEmployeeStatistics();
            }
        }
    }
}

// Advanced features

void GWasteCollection::on_pushButton_sort_e4_clicked()
{
    ui->tableView_list_e4->setModel(Etmp.displaySortedEmployees());
}

void GWasteCollection::on_lineEdit_search_e4_textChanged(const QString &arg1)
{
    QSortFilterProxyModel * EModel = new QSortFilterProxyModel(this);
    EModel->setSourceModel(Etmp.displayEmployees());
    EModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    ui->tableView_list_e4->setModel(EModel);
    EModel->setFilterKeyColumn(2);
    EModel->setFilterFixedString(arg1);
}

void GWasteCollection::on_pushButton_export_e4_clicked()
{
    QAbstractItemModel *model = ui->tableView_list_e4->model();
    if (model)
    {
        QString downloadsPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
        QString defaultFileName = downloadsPath + "/Employees_list_export_" + QDate::currentDate().toString("yyyy_MM_dd") + ".csv";
        QString fileName = QFileDialog::getSaveFileName(this, "Export Excel", defaultFileName, "Excel Files (*.csv)");
        if (!fileName.isEmpty())
        {
            QFile file(fileName);
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QMessageBox::critical(this, "Error", "Cannot create file");
            }
            else
            {
                QTextStream out(&file);
                // Headers
                for (int col = 0; col < model->columnCount(); col++) {
                    out << model->headerData(col, Qt::Horizontal).toString();
                    if (col < model->columnCount() - 1)
                        out << ";";
                }
                out << "\n";
                // Data
                for (int row = 0; row < model->rowCount(); row++) {
                    for (int col = 0; col < model->columnCount(); col++) {
                        QString data = model->data(model->index(row, col)).toString();
                        if (col == 3) {
                            data = "=\"" + data + "\"";
                        }
                        data.replace(";", ",");
                        out << data;
                        if (col < model->columnCount() - 1)
                            out << ";";
                    }
                    out << "\n";
                }
                file.close();
            }
        }
    }
}

void GWasteCollection::animateBarSet_e(QBarSet *set)
{
    if (set == nullptr || set->count() == 0)
        return;

    QVector<qreal> finalValues;
    finalValues.reserve(set->count());

    for (int i = 0; i < set->count(); ++i) {
        finalValues.append(set->at(i));
        set->replace(i, 0.0);
    }

    QVariantAnimation* animation = new QVariantAnimation(this);
    animation->setDuration(1500);
    animation->setStartValue(0.0);
    animation->setEndValue(1.0);

    QObject::connect(animation, &QVariantAnimation::valueChanged,
                     this,
                     [set, finalValues](const QVariant &value) {
                         qreal progress = value.toReal();
                         for (int i = 0; i < finalValues.size(); ++i) {
                             set->replace(i, finalValues[i] * progress);
                         }
                     });

    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void GWasteCollection::displayEmployeeStatistics()
{
    QAbstractItemModel *model=Etmp.getNbEmployeesByPosition();

    // Bar set creation
    QBarSet *barSet = new QBarSet("Positions");
    barSet->setColor(QColor(31, 73, 125));
    barSet->setBorderColor(QColor(20, 50, 90));
    for (int i = 0; i < model->rowCount(); i++)
        *barSet << model->data(model->index(i, 1)).toInt();
    animateBarSet_e(barSet);
    QBarSeries *series = new QBarSeries();
    series->append(barSet);
    series->setBarWidth(0.4);

    // Chart creation
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setMargins(QMargins(0, 20, 0, 0));
    chart->legend()->hide();
    chart->setBackgroundVisible(false);
    chart->setPlotAreaBackgroundVisible(false);

    // X Axis
    QBarCategoryAxis *x = new QBarCategoryAxis();
    for (int i = 0; i <model->rowCount(); i++)
        x->append(model->data(model->index(i, 0)).toString());
    x->setTitleText("Positions");
    x->setTitleBrush(QBrush(QColor(50, 72, 95)));
    x->setTitleFont(QFont("Onest", 9, QFont::Bold));
    x->setLinePenColor(QColor(141, 162, 181));
    x->setGridLineVisible(false);
    x->setLabelsAngle(-60);
    chart->addAxis(x, Qt::AlignBottom);
    series->attachAxis(x);

    // Y Axis
    int maxVal = 0;
    for (int i = 0; i < model->rowCount(); i++)
        if (model->data(model->index(i, 1)).toInt() > maxVal)
            maxVal = model->data(model->index(i, 1)).toInt();
    QValueAxis *y = new QValueAxis();
    y->setLabelFormat("%d");
    y->setMin(0);
    y->setMax(maxVal);
    y->setTickCount(maxVal + 1);
    y->setMinorTickCount(0);
    y->setTitleText("Number of employees");
    y->setTitleBrush(QBrush(QColor(50, 72, 95)));
    y->setTitleFont(QFont("Onest", 9, QFont::Bold));
    y->setGridLineColor(QColor(141, 162, 181));
    y->setLinePenColor(QColor(141, 162, 181));
    chart->addAxis(y, Qt::AlignLeft);
    series->attachAxis(y);

    // Update widget
    ui->widget_stats_e4->setChart(chart);
    ui->widget_stats_e4->setRenderHint(QPainter::Antialiasing);
    ui->widget_stats_e4->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

// Arduino

void GWasteCollection::on_pushButton_fingerprint_e1_clicked()
{
    A.write_to_arduino("led_on\n");
    A.write_to_arduino("scan\n");
}

void GWasteCollection::on_pushButton_fingerprint_e3_clicked()
{
    A.write_to_arduino("led_on\n");
    A.write_to_arduino("enroll\n");
}

int GWasteCollection::getSmallestAvailableID(const QVector<int>& usedIDs)
{
    for (int i = 1; i <= 127; i++) {
        if (!usedIDs.contains(i)) {
            return i;
        }
    }

    return -1;
}

int GWasteCollection::getFingerprintIDForEmployee()
{
    int currentID = Session.getFingerprint();

    if (currentID != 0) {
        return currentID;
    }

    QVector<int> usedIDs = Etmp.getAllFingerprintIDs();
    int newID = getSmallestAvailableID(usedIDs);

    return newID;
}

void GWasteCollection::processFingerprint(QString line)
{
    if (line.contains("Found ID #")) {
        A.write_to_arduino("led_off\n");
        int fingerprint = line.split("#")[1].toInt();
        if (!Etmp.getEmployeeByFingerprint(fingerprint))
        {
            QMessageBox::critical(nullptr, QObject::tr("Access denied"),
                                  QObject::tr("This employee ID does not exist anymore.\n"
                                              "Click Cancel to exit."), QMessageBox::Cancel);
        }
        else
        {
            ui->lineEdit_id_e1->clear();
            ui->lineEdit_password_e1->clear();
            Session=Etmp;
            login();
        }
    }
    else if (line.contains("Unknown fingerprint")) {
        QMessageBox::critical(nullptr, QObject::tr("Access denied"),
                              QObject::tr("Unknown fingerprint.\n"
                                          "Click Cancel to exit."), QMessageBox::Cancel);
        A.write_to_arduino("led_off\n");
    }
    else if (line.contains("Please type in the ID")) {
        int fingerprintID = getFingerprintIDForEmployee();
        if (fingerprintID == -1) {
            A.write_to_arduino("128\n");
            QMessageBox::critical(nullptr, QObject::tr("Failed"),
                                  QObject::tr("No available fingerprint slots!\n"
                                              "Click Cancel to exit."), QMessageBox::Cancel);
            A.write_to_arduino("led_off\n");
        }
        else
        {
            Session.setFingerprint(fingerprintID);
            QByteArray id = QByteArray::number(fingerprintID) + "\n";
            A.write_to_arduino(id);
        }
    }
    else if (line.contains("Stored!")) {
        Session.addFingerprint();
        QMessageBox::information(nullptr, QObject::tr("Success"),
                              QObject::tr("Fingerprint added successfully!\n"
                                          "Click Cancel to exit."), QMessageBox::Cancel);
        A.write_to_arduino("led_off\n");
    }
    else if (line.contains("Error")) {
        QMessageBox::critical(nullptr, QObject::tr("Error"),
                              QObject::tr("Fingerprint scanning failed!\n"
                                          "Click Cancel to exit."), QMessageBox::Cancel);
        A.write_to_arduino("led_off\n");
    }
}

//-------------------------------------------------------------------------------
//-----------------------------Municipality Module-------------------------------
//-------------------------------------------------------------------------------

//---------------------navigation buttons--------------

void GWasteCollection::on_pushButton_map_m1_clicked()
{
    ui->stackedWidget_m1->setCurrentIndex(1);
    initMapWidget();
    clear_m();
}

void GWasteCollection::on_pushButton_budget_m1_clicked()
{
    // Vider l'ancien budget à chaque fois qu'on revient sur la page
    if (m_budgetModel) {
        delete m_budgetModel;
        m_budgetModel = nullptr;
    }
    ui->tableView_budget_m3->setModel(nullptr);
    //ui->lineEdit_budget_m3->clear();

    ui->stackedWidget_m1->setCurrentIndex(2);
    clear_m();
}

void GWasteCollection::on_pushButton_management_m2_clicked()
{
    ui->stackedWidget_m1->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_budget_m2_clicked()
{
    // Vider l'ancien budget à chaque fois qu'on revient sur la page
    if (m_budgetModel) {
        delete m_budgetModel;
        m_budgetModel = nullptr;
    }
    ui->tableView_budget_m3->setModel(nullptr);
    //ui->lineEdit_budget_m3->clear();

    ui->stackedWidget_m1->setCurrentIndex(2);
}

void GWasteCollection::on_pushButton_management_m3_clicked()
{
    ui->stackedWidget_m1->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_map_m3_clicked()
{
    ui->stackedWidget_m1->setCurrentIndex(1);
    initMapWidget();
}

void GWasteCollection::on_pushButton_confirm_m3_clicked()
{
    QString budgetStr = ui->lineEdit_budget_m3->text().trimmed();
    bool valid = true;

    if (budgetStr.isEmpty())
    {
        QMessageBox::warning(this, "Input Error", "Please enter the total budget.");
        ui->lineEdit_budget_m3->setFocus();
        valid = false;
    }
    else
    {
        int dotCount = 0;

        for (int i = 0; i < budgetStr.length(); i++)
        {
            if (budgetStr[i] == '.')
            {
                dotCount++;

                if (dotCount > 1)
                {
                    QMessageBox::warning(this, "Input Error",
                                         "Budget can contain only one decimal point.");
                    ui->lineEdit_budget_m3->setFocus();
                    valid = false;
                    break;
                }

                if (i == 0 || i == budgetStr.length() - 1)
                {
                    QMessageBox::warning(this, "Input Error",
                                         "Decimal point cannot be at the beginning or end.");
                    ui->lineEdit_budget_m3->setFocus();
                    valid = false;
                    break;
                }
            }
            else if (!budgetStr[i].isDigit())
            {
                QMessageBox::warning(this, "Input Error",
                                     "Budget must contain only numbers and one decimal point.");
                ui->lineEdit_budget_m3->setFocus();
                valid = false;
                break;
            }
        }
    }
    if (valid)
    {
        double totalBudget = budgetStr.toDouble();

        if (totalBudget <= 0)
        {
            QMessageBox::warning(this, "Input Error",
                                 "Please enter a valid positive budget.");
            ui->lineEdit_budget_m3->setFocus();
            valid = false;
        }
    }
    if (valid)
    {
        displayBudgetTable(budgetStr.toDouble());
    }
}

//-----------side barre buttons-------------------------

void GWasteCollection::on_pushButton_logout_m3_clicked()
{
    logout();
}

void GWasteCollection::on_pushButton_logout_m2_clicked()
{
    logout();
}

void GWasteCollection::on_pushButton_logout_m1_clicked()
{
    logout();
}

void GWasteCollection::on_pushButton_employee_m3_clicked()
{
    ui->tableView_list_e4->setModel(Etmp.displayEmployees());
    displayEmployeeStatistics();
    ui->SWWasteCollection->setCurrentIndex(0);
    ui->stackedWidget_e->setCurrentIndex(3);
}

void GWasteCollection::on_pushButton_employee_m2_clicked()
{
    ui->tableView_list_e4->setModel(Etmp.displayEmployees());
    displayEmployeeStatistics();
    ui->SWWasteCollection->setCurrentIndex(0);
    ui->stackedWidget_e->setCurrentIndex(3);
}

void GWasteCollection::on_pushButton_employee_m1_clicked()
{
    ui->tableView_list_e4->setModel(Etmp.displayEmployees());
    displayEmployeeStatistics();
    ui->SWWasteCollection->setCurrentIndex(0);
    ui->stackedWidget_e->setCurrentIndex(3);
    clear_m();
}

void GWasteCollection::on_pushButton_sewer_m3_clicked()
{
    ui->tableView_S1->setModel(Stemp.displaySewer());
    ui->SWWasteCollection->setCurrentIndex(2);
    ui->stackedWidget_e->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_sewer_m2_clicked()
{
    ui->tableView_S1->setModel(Stemp.displaySewer());
    ui->SWWasteCollection->setCurrentIndex(2);
    ui->stackedWidgetS->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_sewer_m1_clicked()
{
    ui->tableView_S1->setModel(Stemp.displaySewer());
    ui->SWWasteCollection->setCurrentIndex(2);
    ui->stackedWidgetS->setCurrentIndex(0);
    clear_m();
}

void GWasteCollection::on_pushButton_center_m3_clicked()
{
    ui->tableView_r1->setModel(Rtmp.displayCenter());
    ui->SWWasteCollection->setCurrentIndex(4);
    ui->stackedWidget_r->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_center_m1_clicked()
{
    ui->tableView_r1->setModel(Rtmp.displayCenter());
    ui->SWWasteCollection->setCurrentIndex(4);
    ui->stackedWidget_r->setCurrentIndex(0);
    clear_m();
}

void GWasteCollection::on_pushButton_center_m2_clicked()
{
    ui->tableView_r1->setModel(Rtmp.displayCenter());
    ui->SWWasteCollection->setCurrentIndex(4);
    ui->stackedWidget_r->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_team_m3_clicked()
{
    ui->tableView_listteams_c1->setModel(Ctmp.DisplayTeams());
    ui->SWWasteCollection->setCurrentIndex(3);
    ui->stackedWidget_c->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_team_m1_clicked()
{
    ui->tableView_listteams_c1->setModel(Ctmp.DisplayTeams());
    ui->SWWasteCollection->setCurrentIndex(3);
    ui->stackedWidget_c->setCurrentIndex(0);
    clear_m();
}

void GWasteCollection::on_pushButton_team_m2_clicked()
{
    ui->tableView_listteams_c1->setModel(Ctmp.DisplayTeams());
    ui->SWWasteCollection->setCurrentIndex(3);
    ui->stackedWidget_c->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_cancel_map_m1_clicked()
{
    // Revenir au formulaire sans rien faire
    ui->stackedWidget_form->setCurrentIndex(0);
    m_pickedLat = 0.0;
    m_pickedLng = 0.0;
}

//------------------CRUD-----------------

void GWasteCollection::clear_m()
{
    // Page m1
    ui->lineEdit_name_m1->clear();
    ui->comboBox_region_m1->setCurrentIndex(0);
    ui->lineEdit_address_m1->clear();
    ui->lineEdit_phone_m1->clear();
    ui->lineEdit_email_m1->clear();
    ui->lineEdit_latitude_m1->clear();
    ui->lineEdit_longitude_m1->clear();
    ui->lineEdit_radius_m1->clear();
    ui->lineEdit_barrederecherche_m1->clear();
    ui->tableView_listemunicipality_m1->clearSelection();
    // Page m2
    ui->lineEdit_barrederecherche_m2->clear();
    ui->comboBox_floodlevel_m2->setCurrentIndex(0);

    // Page m3
    ui->lineEdit_barrederecherche_m3->clear();
    ui->lineEdit_budget_m3->clear();
}

QMap<QString, QMap<QString, double>> GWasteCollection::getRegionBounds() const
{
    QMap<QString, QMap<QString, double>> regionBounds;
    regionBounds["Tunis"]       = {{"minLat",36.70},{"maxLat",36.95},{"minLng",10.10},{"maxLng",10.35},{"maxRadius",30000}};
    regionBounds["Ariana"]      = {{"minLat",36.82},{"maxLat",37.00},{"minLng",10.10},{"maxLng",10.30},{"maxRadius",25000}};
    regionBounds["Ben Arous"]   = {{"minLat",36.68},{"maxLat",36.82},{"minLng",10.10},{"maxLng",10.35},{"maxRadius",40000}};
    regionBounds["Manouba"]     = {{"minLat",36.75},{"maxLat",36.95},{"minLng", 9.95},{"maxLng",10.20},{"maxRadius",35000}};
    regionBounds["Nabeul"]      = {{"minLat",36.30},{"maxLat",36.85},{"minLng",10.40},{"maxLng",11.10},{"maxRadius",80000}};
    regionBounds["Zaghouan"]    = {{"minLat",35.80},{"maxLat",36.55},{"minLng", 9.80},{"maxLng",10.30},{"maxRadius",70000}};
    regionBounds["Bizerte"]     = {{"minLat",36.85},{"maxLat",37.54},{"minLng", 9.50},{"maxLng",10.20},{"maxRadius",90000}};
    regionBounds["Béja"]        = {{"minLat",36.40},{"maxLat",37.10},{"minLng", 8.80},{"maxLng", 9.70},{"maxRadius",90000}};
    regionBounds["Jendouba"]    = {{"minLat",36.40},{"maxLat",37.20},{"minLng", 8.30},{"maxLng", 9.30},{"maxRadius",85000}};
    regionBounds["Le Kef"]      = {{"minLat",35.80},{"maxLat",36.80},{"minLng", 8.30},{"maxLng", 9.20},{"maxRadius",90000}};
    regionBounds["Siliana"]     = {{"minLat",35.80},{"maxLat",36.50},{"minLng", 9.20},{"maxLng", 9.90},{"maxRadius",80000}};
    regionBounds["Kairouan"]    = {{"minLat",35.30},{"maxLat",36.90},{"minLng", 9.40},{"maxLng",10.20},{"maxRadius",95000}};
    regionBounds["Kasserine"]   = {{"minLat",34.90},{"maxLat",35.70},{"minLng", 8.10},{"maxLng", 9.10},{"maxRadius",100000}};
    regionBounds["Sidi Bouzid"] = {{"minLat",34.50},{"maxLat",35.40},{"minLng", 9.00},{"maxLng",10.00},{"maxRadius",95000}};
    regionBounds["Sousse"]      = {{"minLat",35.60},{"maxLat",36.30},{"minLng",10.20},{"maxLng",10.80},{"maxRadius",75000}};
    regionBounds["Monastir"]    = {{"minLat",35.55},{"maxLat",35.90},{"minLng",10.55},{"maxLng",11.00},{"maxRadius",50000}};
    regionBounds["Mahdia"]      = {{"minLat",34.90},{"maxLat",35.70},{"minLng",10.30},{"maxLng",11.10},{"maxRadius",80000}};
    regionBounds["Sfax"]        = {{"minLat",34.20},{"maxLat",35.30},{"minLng",10.30},{"maxLng",11.00},{"maxRadius",90000}};
    regionBounds["Gafsa"]       = {{"minLat",33.80},{"maxLat",34.80},{"minLng", 7.90},{"maxLng", 9.30},{"maxRadius",90000}};
    regionBounds["Tozeur"]      = {{"minLat",33.40},{"maxLat",34.20},{"minLng", 7.80},{"maxLng", 9.00},{"maxRadius",75000}};
    regionBounds["Kébili"]      = {{"minLat",33.00},{"maxLat",34.10},{"minLng", 8.40},{"maxLng",9.70},{"maxRadius",100000}};
    regionBounds["Gabes"]       = {{"minLat",33.40},{"maxLat",34.50},{"minLng", 9.60},{"maxLng",10.30},{"maxRadius",80000}};
    regionBounds["Médenine"]    = {{"minLat",32.70},{"maxLat",33.60},{"minLng", 9.90},{"maxLng",11.60},{"maxRadius",85000}};
    regionBounds["Tataouine"]   = {{"minLat",30.20},{"maxLat",32.70},{"minLng", 8.80},{"maxLng",11.20},{"maxRadius",100000}};
    return regionBounds;
}

void GWasteCollection::on_pushButton_autoloc_m1_clicked()
{
    // Contrôle : région ne doit pas être vide
    if (ui->comboBox_region_m1->currentText().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Attention", "Veuillez sélectionner une région avant d'ouvrir la carte.");
        return;
    }

    // Charger le QML dans le quickWidget
    ui->quickWidget_loc_m1->setSource(QUrl("qrc:/fichier/map_loc.qml"));
    ui->quickWidget_loc_m1->show();

    // Récupérer l'objet root du QML
    QQuickItem* root = ui->quickWidget_loc_m1->rootObject();
    if (!root) return;

    // Centrer sur la région saisie via géocodage simple
    // (coordonnées approximatives des régions tunisiennes)
    QMap<QString, QPair<double,double>> regionCoords = {
        {"Tunis",       {36.8065,  10.1815}},
        {"Ariana",      {36.8924,  10.1964}},
        {"Ben Arous",   {36.7531,  10.2282}},
        {"Manouba",     {36.8100,  10.0972}},
        {"Nabeul",      {36.4520,  10.7350}},
        {"Zaghouan",    {36.4029,   9.9922}},
        {"Bizerte",     {37.2744,   9.8739}},
        {"Béja",        {36.7256,   9.1817}},
        {"Jendouba",    {36.5011,   8.7757}},
        {"Le Kef",      {36.1822,   8.7147}},
        {"Siliana",     {36.0849,   9.3708}},
        {"Kairouan",    {35.6781,  10.0960}},
        {"Kasserine",   {35.1676,   8.8365}},
        {"Sidi Bouzid", {35.0382,   9.4849}},
        {"Sousse",      {35.8245,  10.6346}},
        {"Monastir",    {35.7643,  10.8113}},
        {"Mahdia",      {35.5047,  11.0622}},
        {"Sfax",        {34.7406,  10.7603}},
        {"Gafsa",       {34.4250,   8.7842}},
        {"Tozeur",      {33.9197,   8.1335}},
        {"Kébili",      {33.7050,   8.9650}},
        {"Gabes",       {33.8814,   9.8978}},
        {"Médenine",    {33.3549,  10.5055}},
        {"Tataouine",   {32.0585,  10.0512}},
        };

    QString region = ui->comboBox_region_m1->currentText().trimmed();
    double lat = 33.8869, lng = 9.5375; // centre Tunisie par défaut

    if (regionCoords.contains(region)) {
        lat = regionCoords[region].first;
        lng = regionCoords[region].second;
    }

    // Centrer la carte sur la région
    QMap<QString, QMap<QString, double>> regionBounds = getRegionBounds();
    double maxDelta = 0.3; // valeur par défaut ~33km
    if (regionBounds.contains(region)) {
        maxDelta = regionBounds[region]["maxRadius"] / 111000.0;
    }

    root->setProperty("markerLat",  lat);
    root->setProperty("markerLng",  lng);
    root->setProperty("centerLat",  lat);
    root->setProperty("centerLng",  lng);
    root->setProperty("maxDelta",   maxDelta);

    // Connecter le signal QML → Qt
    connect(root, SIGNAL(locationPicked(double, double)),
            this, SLOT(onLocationPicked(double, double)));

    // Aller à la page map (index 1)
    ui->stackedWidget_form->setCurrentIndex(1);
}

void GWasteCollection::onLocationPicked(double lat, double lng)
{
    m_pickedLat = lat;
    m_pickedLng = lng;
}

void GWasteCollection::on_pushButton_confirm_map_m1_clicked()
{
    if (m_pickedLat == 0.0 && m_pickedLng == 0.0) {
        QMessageBox::warning(this, "Attention", "Veuillez cliquer sur la carte pour choisir un emplacement.");
        return;
    }

    // Remplir les champs du formulaire
    ui->lineEdit_latitude_m1->setText(QString::number(m_pickedLat, 'f', 6));
    ui->lineEdit_longitude_m1->setText(QString::number(m_pickedLng, 'f', 6));

    // Revenir au formulaire
    ui->stackedWidget_form->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_restor_m1_clicked()
{
    if (ui->tableView_listemunicipality_m1->selectionModel()->hasSelection())
    {
        clear_m();
    }
    //ui->tableView_listemunicipality_m1->setModel(Mtmp.displayMunicipality());
    //displayMunicipalityStatistics();
}


void GWasteCollection::on_pushButton_confirm_m1_clicked()
{
    Municipality Mt;
    QString name        = ui->lineEdit_name_m1->text().trimmed();
    QString region      = ui->comboBox_region_m1->currentText().trimmed();
    QString address     = ui->lineEdit_address_m1->text().trimmed();
    QString phone       = ui->lineEdit_phone_m1->text().trimmed();
    QString email       = ui->lineEdit_email_m1->text().trimmed();
    QString longitudeStr = ui->lineEdit_longitude_m1->text().trimmed();
    QString latitudeStr  = ui->lineEdit_latitude_m1->text().trimmed();
    QString radiusStr    = ui->lineEdit_radius_m1->text().trimmed();

    QMap<QString, QMap<QString, double>> regionBounds = getRegionBounds();

    QModelIndex indext = ui->tableView_listemunicipality_m1->selectionModel()->currentIndex();
    bool valid = true;

    if (name.isEmpty() || name.length() > 100 ||
        (Mt.getMunicipalityName(name) &&
         (!ui->tableView_listemunicipality_m1->selectionModel()->hasSelection() ||
          name != indext.sibling(indext.row(), 1).data().toString())))
    {
        QMessageBox::warning(this, "Input Error",
                             "Name cannot be empty.\nName must be unique.\nMust be at most 100 characters.");
        ui->lineEdit_name_m1->setFocus();
        valid = false;
    }
    else if (region.isEmpty())
    {
        QMessageBox::warning(this, "Input Error", "Region cannot be empty.");
        ui->comboBox_region_m1->setFocus();
        valid = false;
    }
    else if (address.isEmpty() || address.length() > 200 ||
             (Mt.getMunicipalityAddress(address) &&
              (!ui->tableView_listemunicipality_m1->selectionModel()->hasSelection() ||
               address != indext.sibling(indext.row(), 3).data().toString())))
    {
        QMessageBox::warning(this, "Input Error",
                             "Address cannot be empty.\nAddress must be unique.\nMust be at most 200 characters.");
        ui->lineEdit_address_m1->setFocus();
        valid = false;
    }
    else
    {
        QString phoneClean = phone;
        phoneClean.remove(' ');
        phoneClean.remove('+');

        int plusCount = 0;
        for (int i = 0; i < phone.length(); i++)
        {
            if (phone[i] == '+')
            {
                plusCount++;
                if (i != 0)
                {
                    QMessageBox::warning(this, "Input Error",
                                         "The '+' sign must be at the beginning only.");
                    ui->lineEdit_phone_m1->setFocus();
                    valid = false;
                    break;
                }
                if (plusCount > 1)
                {
                    QMessageBox::warning(this, "Input Error",
                                         "Phone number can contain only one '+'.");
                    ui->lineEdit_phone_m1->setFocus();
                    valid = false;
                    break;
                }
            }
            else if (!phone[i].isDigit() && phone[i] != ' ')
            {
                QMessageBox::warning(this, "Input Error",
                                     "Phone can only contain numbers and optionally one '+' at the beginning.");
                ui->lineEdit_phone_m1->setFocus();
                valid = false;
                break;
            }
        }

        if (valid)
        {
            if (phoneClean.length() != 8 && phoneClean.length() != 11)
            {
                QMessageBox::warning(this, "Input Error",
                                     "Phone must be 8 digits (ex: 29999888)\n"
                                     "or 12 characters with country code (ex: +21629999888).");
                ui->lineEdit_phone_m1->setFocus();
                valid = false;
            }
            else if (phoneClean.length() == 11)
            {
                if (!phone.startsWith("+216"))
                {
                    QMessageBox::warning(this, "Input Error",
                                         "International phone must start with +216\n"
                                         "(Tunisia country code).");
                    ui->lineEdit_phone_m1->setFocus();
                    valid = false;
                }
            }
        }

        if (valid)
        {
            if (Mt.getMunicipalityPhone(phone) &&
                (!ui->tableView_listemunicipality_m1->selectionModel()->hasSelection() ||
                 phone != indext.sibling(indext.row(), 4).data().toString()))
            {
                QMessageBox::warning(this, "Input Error", "Phone must be unique.");
                ui->lineEdit_phone_m1->setFocus();
                valid = false;
            }
        }
    }

    if (valid)
    {
        if (email.isEmpty() || email.length() > 100 || email.length() < 6 ||
            (Mt.getMunicipalityEmail(email) &&
             (!ui->tableView_listemunicipality_m1->selectionModel()->hasSelection() ||
              email != indext.sibling(indext.row(), 5).data().toString())))
        {
            QMessageBox::warning(this, "Input Error",
                                 "Email cannot be empty,\nEmail must be unique,\nand must be at most 100 characters.");
            ui->lineEdit_email_m1->setFocus();
            valid = false;
        }
        else
        {
            int atPos  = email.indexOf('@');
            int dotPos = email.lastIndexOf('.');

            if (atPos < 1 || dotPos <= atPos + 1 || dotPos >= email.length() - 2)
            {
                valid = false;
            }

            if (!valid)
            {
                QMessageBox::warning(this, "Input Error",
                                     "Email format is invalid.\n"
                                     "It must contain '@' before '.',\n"
                                     "and have valid characters after '.'.");
                ui->lineEdit_email_m1->setFocus();
            }
        }
    }

    if (valid)
    {
        if (latitudeStr.isEmpty())
        {
            QMessageBox::warning(this, "Input Error", "Latitude cannot be empty.");
            ui->lineEdit_latitude_m1->setFocus();
            valid = false;
        }
        else
        {
            int dotCount = 0;
            for (int i = 0; i < latitudeStr.length(); i++)
            {
                if (latitudeStr[i] == '.')
                {
                    dotCount++;
                    if (dotCount > 1)
                    {
                        QMessageBox::warning(this, "Input Error",
                                             "Latitude can contain only one decimal point.");
                        ui->lineEdit_latitude_m1->setFocus();
                        valid = false;
                        break;
                    }
                    if (i == 0 || i == latitudeStr.length() - 1)
                    {
                        QMessageBox::warning(this, "Input Error",
                                             "Decimal point cannot be at the beginning or end.");
                        ui->lineEdit_latitude_m1->setFocus();
                        valid = false;
                        break;
                    }
                }
                else if (!latitudeStr[i].isDigit())
                {
                    QMessageBox::warning(this, "Input Error",
                                         "Latitude must contain only numbers and one decimal point.");
                    ui->lineEdit_latitude_m1->setFocus();
                    valid = false;
                    break;
                }
            }
        }
    }

    if (valid)
    {
        double lat = latitudeStr.toDouble();
        if (lat < 30.24 || lat > 37.54)
        {
            QMessageBox::warning(this, "Input Error",
                                 "Latitude must be between 30.24 and 37.54 (Tunisian territory).");
            ui->lineEdit_latitude_m1->setFocus();
            valid = false;
        }
    }

    // Validation : Longitude
    if (valid)
    {
        if (longitudeStr.isEmpty())
        {
            QMessageBox::warning(this, "Input Error", "Longitude cannot be empty.");
            ui->lineEdit_longitude_m1->setFocus();
            valid = false;
        }
        else
        {
            int dotCount = 0;
            for (int i = 0; i < longitudeStr.length(); i++)
            {
                if (longitudeStr[i] == '.')
                {
                    dotCount++;
                    if (dotCount > 1)
                    {
                        QMessageBox::warning(this, "Input Error",
                                             "Longitude can contain only one decimal point.");
                        ui->lineEdit_longitude_m1->setFocus();
                        valid = false;
                        break;
                    }
                    if (i == 0 || i == longitudeStr.length() - 1)
                    {
                        QMessageBox::warning(this, "Input Error",
                                             "Decimal point cannot be at the beginning or end.");
                        ui->lineEdit_longitude_m1->setFocus();
                        valid = false;
                        break;
                    }
                }
                else if (!longitudeStr[i].isDigit())
                {
                    QMessageBox::warning(this, "Input Error",
                                         "Longitude must contain only numbers and one decimal point.");
                    ui->lineEdit_longitude_m1->setFocus();
                    valid = false;
                    break;
                }
            }
        }
    }

    if (valid)
    {
        double lng = longitudeStr.toDouble();
        if (lng < 7.52 || lng > 11.60)
        {
            QMessageBox::warning(this, "Input Error",
                                 "Longitude must be between 7.52 and 11.60 (Tunisian territory).");
            ui->lineEdit_longitude_m1->setFocus();
            valid = false;
        }
    }

    if (valid)
    {
        double lat = latitudeStr.toDouble();
        double lng = longitudeStr.toDouble();
        if (regionBounds.contains(region))
        {
            double minLat = regionBounds[region]["minLat"];
            double maxLat = regionBounds[region]["maxLat"];
            double minLng = regionBounds[region]["minLng"];
            double maxLng = regionBounds[region]["maxLng"];

            if (lat < minLat || lat > maxLat || lng < minLng || lng > maxLng)
            {
                QMessageBox::warning(this, "Input Error",
                                     QString("Coordinates do not match the region %1.\n"
                                             "Latitude must be between %2 and %3.\n"
                                             "Longitude must be between %4 and %5.")
                                         .arg(region)
                                         .arg(minLat).arg(maxLat)
                                         .arg(minLng).arg(maxLng));
                ui->lineEdit_latitude_m1->setFocus();
                valid = false;
            }
        }
    }

    if (valid)
    {
        if (radiusStr.isEmpty())
        {
            QMessageBox::warning(this, "Input Error", "Radius cannot be empty.");
            ui->lineEdit_radius_m1->setFocus();
            valid = false;
        }
        else
        {
            for (int i = 0; i < radiusStr.length(); i++)
            {
                if (!radiusStr[i].isDigit())
                {
                    QMessageBox::warning(this, "Input Error", "Radius must contain only numbers.");
                    ui->lineEdit_radius_m1->setFocus();
                    valid = false;
                    break;
                }
            }
        }
    }

    if (valid)
    {
        int radius = radiusStr.toInt();
        if (radius < 100 || radius > 100000)
        {
            QMessageBox::warning(this, "Input Error",
                                 "Radius must be between 100 and 100000 meters "
                                 "(100m minimum, 100km maximum).");
            ui->lineEdit_radius_m1->setFocus();
            valid = false;
        }
    }

    if (valid)
    {
        int radius = radiusStr.toInt();
        if (regionBounds.contains(region))
        {
            int maxRadius = (int)regionBounds[region]["maxRadius"];
            if (radius > maxRadius)
            {
                QMessageBox::warning(this, "Input Error",
                                     QString("Radius cannot exceed %1 meters for the region %2.")
                                         .arg(maxRadius)
                                         .arg(region));
                ui->lineEdit_radius_m1->setFocus();
                valid = false;
            }
        }
    }


    if (valid)
    {
        int radius = radiusStr.toInt();
        Municipality M;
        bool success = false;

        if (ui->tableView_listemunicipality_m1->selectionModel()->hasSelection())
        {
            QModelIndex index = ui->tableView_listemunicipality_m1->selectionModel()->currentIndex();
            int MunicipalityID = index.sibling(index.row(), 0).data().toInt();
            M = Municipality(MunicipalityID, name, region, address, phone, email, longitudeStr, latitudeStr, radius);
            success = M.updateMunicipality();
        }
        else
        {
            M = Municipality(0, name, region, address, phone, email, longitudeStr, latitudeStr, radius);
            success = M.addMunicipality();
        }

        if (success)
        {
            ui->tableView_listemunicipality_m1->setModel(Mtmp.displayMunicipality());
            displayMunicipalityStatistics();
            QMessageBox::information(this, "Success", "Operation successful.");
            clear_m();
            ui->comboBox_municialityname_s1->setModel(Mtmp.displayMunicipality());
            ui->comboBox_municialityname_s1->setModelColumn(1);
            ui->comboBox_municialityname_r1->setModel(Mtmp.displayMunicipality());
            ui->comboBox_municialityname_r1->setModelColumn(1);
        }
        else
        {
            QMessageBox::critical(this, "Error", "Operation failed.");
        }
    }
}

void GWasteCollection::on_tableView_listemunicipality_m1_doubleClicked(const QModelIndex &index)
{
    int municipalityID = index.sibling(index.row(),0).data().toInt();
    if(Municipality_id_Maquette !=municipalityID){
        QMessageBox::StandardButton reply;

        reply = QMessageBox::question(
            this,
            "Confirmation",
            "Do you want to delete this municipality ?",
            QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {

            if (Mtmp.deleteMunicipality(municipalityID)) {

                ui->tableView_listemunicipality_m1->setModel(Mtmp.displayMunicipality());
                displayMunicipalityStatistics();
                clear_m();
                ui->comboBox_municialityname_s1->setModel(Mtmp.displayMunicipality());
                ui->comboBox_municialityname_s1->setModelColumn(1);
                ui->comboBox_municialityname_r1->setModel(Mtmp.displayMunicipality());
                ui->comboBox_municialityname_r1->setModelColumn(1);

            }

        }
    }else{
        QMessageBox::warning(
            this,
            "Deletion Not Allowed",
            "This municipality cannot be deleted because it is part of the Prototype."
            );
    }

}


void GWasteCollection::on_tableView_listemunicipality_m1_clicked(const QModelIndex &index)
{
    ui->lineEdit_name_m1->setText(index.sibling(index.row(),1).data().toString());
    ui->comboBox_region_m1->setCurrentText(index.sibling(index.row(),2).data().toString());
    ui->lineEdit_address_m1->setText(index.sibling(index.row(),3).data().toString());
    ui->lineEdit_phone_m1->setText(index.sibling(index.row(),4).data().toString());
    ui->lineEdit_email_m1->setText(index.sibling(index.row(),5).data().toString());
    ui->lineEdit_longitude_m1->setText(index.sibling(index.row(),6).data().toString());
    ui->lineEdit_latitude_m1->setText(index.sibling(index.row(),7).data().toString());
    ui->lineEdit_radius_m1->setText(index.sibling(index.row(),8).data().toString());

}

//-------------------------------Advanced features--------------------
void GWasteCollection::on_pushButton_sort_m1_clicked()
{
    ui->tableView_listemunicipality_m1->setModel(Mtmp.displaySortedMunicipality());
    displayMunicipalityStatistics();
}

void GWasteCollection::on_lineEdit_barrederecherche_m1_textChanged(const QString &arg1)
{
    QSortFilterProxyModel * Model = new QSortFilterProxyModel(this);
    Model->setSourceModel(Mtmp.displayMunicipality());
    Model->setFilterCaseSensitivity(Qt::CaseInsensitive);
    ui->tableView_listemunicipality_m1->setModel(Model);
    Model->setFilterKeyColumn(1);
    Model->setFilterFixedString(arg1);
    displayMunicipalityStatistics();
}

void GWasteCollection::on_pushButton_export_m1_clicked()
{
    QString filePath = QFileDialog::getSaveFileName(this, "Export Municipalities to PDF",
                                                    QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/municipalities_report "+ QDateTime::currentDateTime().toString("yyyy-MM-dd hh-mm-ss")+" .pdf",
                                                    "PDF Files (*.pdf)");
    if (!filePath.isEmpty())
    {
        QAbstractItemModel *model = ui->tableView_listemunicipality_m1->model();
        if (model)
        {
            QPdfWriter pdfWriter(filePath);
            pdfWriter.setPageSize(QPageSize(QPageSize::A4));
            pdfWriter.setPageOrientation(QPageLayout::Landscape);
            pdfWriter.setPageMargins(QMarginsF(10, 10, 10, 10), QPageLayout::Millimeter);
            pdfWriter.setResolution(96);

            QPainter painter(&pdfWriter);
            if (painter.isActive())
            {
                int pageWidth  = painter.viewport().width();
                int pageHeight = painter.viewport().height();
                int y = 20;

                // --- Titre ---
                painter.setFont(QFont("Arial", 16, QFont::Bold));
                painter.drawText(QRect(0, y, pageWidth, 50), Qt::AlignCenter, "Municipality Report");
                y += 60;

                // --- Date ---
                painter.setFont(QFont("Arial", 9));
                painter.setPen(Qt::gray);
                painter.drawText(QRect(0, y, pageWidth, 30), Qt::AlignCenter,
                                 "Generated on: " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
                painter.setPen(Qt::black);
                y += 50;

                // --- Largeurs colonnes ---
                QList<int> colWidths = {60, 130, 110, 180, 110, 190, 80, 80, 70};
                int totalWidth = 0;
                for (int w : colWidths) totalWidth += w;
                if (totalWidth != pageWidth) {
                    float ratio = (float)pageWidth / totalWidth;
                    for (int i = 0; i < colWidths.size(); i++)
                        colWidths[i] = (int)(colWidths[i] * ratio);
                }

                int colCount = model->columnCount();

                // --- En-têtes ---
                painter.setFont(QFont("Arial", 9, QFont::Bold));
                painter.setBrush(QColor(52, 152, 219));
                painter.setPen(Qt::NoPen);
                painter.drawRect(0, y, pageWidth, 35);
                painter.setPen(Qt::white);
                int x = 0;
                for (int col = 0; col < colCount && col < colWidths.size(); col++) {
                    painter.drawText(QRect(x + 5, y, colWidths[col] - 5, 35),
                                     Qt::AlignVCenter | Qt::AlignCenter | Qt::TextWordWrap,
                                     model->headerData(col, Qt::Horizontal).toString());
                    x += colWidths[col];
                }
                y += 35;

                // --- Données ---
                painter.setFont(QFont("Arial", 8));
                bool alternate = false;
                for (int row = 0; row < model->rowCount(); row++) {
                    int rowHeight = 30;
                    for (int col = 0; col < colCount && col < colWidths.size(); col++) {
                        QRect br = painter.boundingRect(QRect(0, 0, colWidths[col] - 10, 1000),
                                                        Qt::AlignTop | Qt::AlignLeft | Qt::TextWordWrap,
                                                        model->data(model->index(row, col)).toString());
                        rowHeight = qMax(rowHeight, br.height() + 10);
                    }
                    if (y + rowHeight > pageHeight - 40) {
                        pdfWriter.newPage();
                        y = 40;
                    }
                    painter.setPen(Qt::NoPen);
                    painter.setBrush(alternate ? QColor(236, 240, 241) : Qt::white);
                    painter.drawRect(0, y, pageWidth, rowHeight);
                    alternate = !alternate;
                    painter.setPen(Qt::black);
                    x = 0;
                    for (int col = 0; col < colCount && col < colWidths.size(); col++) {
                        painter.drawText(QRect(x + 5, y + 5, colWidths[col] - 10, rowHeight),
                                         Qt::AlignTop | Qt::AlignHCenter | Qt::TextWordWrap,
                                         model->data(model->index(row, col)).toString());
                        x += colWidths[col];
                    }
                    painter.setPen(QColor(189, 195, 199));
                    painter.drawLine(0, y + rowHeight, pageWidth, y + rowHeight);
                    y += rowHeight;
                }

                // ================================================================
                // --- STATISTIQUES via computeRegionStatistics() -----------------
                // ================================================================

                QString regions[23];
                int     counts[23];
                int     nbRegions = 0;

                computeRegionStatistics(regions, counts, nbRegions);

                if (nbRegions > 0)
                {
                    const int chartNeededHeight = 280;
                    if (y + chartNeededHeight + 60 > pageHeight) {
                        pdfWriter.newPage();
                        y = 20;
                    } else {
                        y += 40;
                    }

                    // --- Titre statistiques ---
                    painter.setPen(Qt::black);
                    painter.setFont(QFont("Arial", 16, QFont::Bold));
                    painter.drawText(QRect(0, y, pageWidth, 50), Qt::AlignCenter,
                                     "Statistics of Municipalities per Region");
                    y += 60;
                    // --- Date ---
                    painter.setFont(QFont("Arial", 9));
                    painter.setPen(Qt::gray);
                    painter.drawText(QRect(0, y, pageWidth, 30), Qt::AlignCenter,
                                     "Generated on: " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
                    painter.setPen(Qt::black);
                    y += 50;

                    // --- Dimensions graphique ---
                    const int chartLeft   = 70;
                    const int chartRight  = pageWidth - 30;
                    const int chartTop    = y;
                    const int chartBottom = y + chartNeededHeight - 50;
                    const int chartWidth  = chartRight - chartLeft;
                    const int chartHeight = chartBottom - chartTop;

                    int maxVal = 0;
                    for (int i = 0; i < nbRegions; i++)
                        if (counts[i] > maxVal) maxVal = counts[i];
                    int maxScale = ((maxVal / 5) + 1) * 5;

                    // Fond
                    painter.setBrush(QColor(248, 249, 250));
                    painter.setPen(QPen(QColor(200, 200, 200)));
                    painter.drawRect(chartLeft, chartTop, chartWidth, chartHeight);

                    // --- Grilles + labels axe Y ---
                    painter.setFont(QFont("Arial", 7));
                    for (int t = 0; t < maxScale / 5 + 1; t++) {
                        int val   = t * 5;
                        int yLine = chartBottom - (int)((float)val / maxScale * chartHeight);
                        painter.setPen(QPen(QColor(220, 220, 220), 1, Qt::DashLine));
                        painter.drawLine(chartLeft, yLine, chartRight, yLine);
                        painter.setPen(Qt::black);
                        painter.drawText(QRect(0, yLine - 10, chartLeft - 5, 20),
                                         Qt::AlignRight | Qt::AlignVCenter, QString::number(val));
                    }

                    // --- Barres ---
                    float barAreaW = (float)chartWidth / nbRegions;
                    float barW     = barAreaW * 0.3f;

                    for (int i = 0; i < nbRegions; i++) {
                        int barHeight = (int)((float)counts[i] / maxScale * chartHeight);
                        int barX      = chartLeft + (int)(i * barAreaW + (barAreaW - barW) / 2.0f);
                        int barY      = chartBottom - barHeight;

                        painter.setBrush(QColor(52, 152, 219));
                        painter.setPen(QPen(QColor(20, 100, 180), 1));
                        painter.drawRect(barX, barY, (int)barW, barHeight);

                        painter.setPen(Qt::black);
                        painter.setFont(QFont("Arial", 8, QFont::Bold));
                        painter.drawText(QRect(barX, barY - 18, (int)barW, 18),
                                         Qt::AlignCenter, QString::number(counts[i]));

                        painter.setFont(QFont("Arial", 7));
                        painter.save();
                        painter.translate(barX + (int)(barW / 2), chartBottom + 6);
                        painter.rotate(0);
                        painter.drawText(QRect(0, 0, 90, 20), Qt::AlignLeft | Qt::AlignVCenter,
                                         regions[i]);
                        painter.restore();
                    }

                    // --- Axes ---
                    painter.setPen(QPen(QColor(80, 80, 80), 2));
                    painter.drawLine(chartLeft, chartTop,    chartLeft,  chartBottom);
                    painter.drawLine(chartLeft, chartBottom, chartRight, chartBottom);

                    // --- Titre axe X ---
                    painter.setFont(QFont("Arial", 9, QFont::Bold));
                    painter.setPen(QColor(50, 72, 95));
                    painter.drawText(QRect(chartLeft, chartBottom + 55, chartWidth, 25),
                                     Qt::AlignCenter, "Regions");

                    // --- Titre axe Y ---
                    painter.save();
                    painter.translate(15, chartTop + chartHeight / 2);
                    painter.rotate(-90);
                    painter.drawText(QRect(-70, -12, 140, 24), Qt::AlignCenter, "Nb of Municipalities");
                    painter.restore();
                }

                painter.end();
                QMessageBox::information(this, "Success", "PDF exported successfully!");
            }
        }
    }
}

void GWasteCollection::computeRegionStatistics(QString regions[], int counts[], int &nbRegions)
{
    nbRegions = 0;
    QAbstractItemModel *model = ui->tableView_listemunicipality_m1->model();
    if (model)
    {
        for (int row = 0; row < model->rowCount(); row++)
        {
            QString region = model->data(model->index(row, 2)).toString();
            if (!region.isEmpty())
            {
                bool found = false;
                for (int i = 0; i < nbRegions; i++) {
                    if (regions[i] == region) {
                        counts[i]++;
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    regions[nbRegions] = region;
                    counts[nbRegions]  = 1;
                    nbRegions++;
                }
            }
        }
    }
}

void GWasteCollection::animateBarSet(QBarSet *set)
{
    if (set == nullptr || set->count() == 0)
        return;

    QVector<qreal> finalValues;
    finalValues.reserve(set->count());

    for (int i = 0; i < set->count(); ++i) {
        finalValues.append(set->at(i));
        set->replace(i, 0.0); // démarrer à 0
    }

    QVariantAnimation* animation = new QVariantAnimation(this);
    animation->setDuration(1500);
    animation->setStartValue(0.0);
    animation->setEndValue(1.0);

    QObject::connect(animation, &QVariantAnimation::valueChanged,
                     this,
                     [set, finalValues](const QVariant &value) {
                         qreal progress = value.toReal();
                         for (int i = 0; i < finalValues.size(); ++i) {
                             set->replace(i, finalValues[i] * progress);
                         }
                     });

    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void GWasteCollection::displayMunicipalityStatistics()
{
    QString regions[23];
    int     counts[23];
    int     nbRegions = 0;

    computeRegionStatistics(regions, counts, nbRegions);

    if (nbRegions == 0) return;

    // --- Créer la série de barres ---
    QBarSet *barSet = new QBarSet("Municipalities");
    barSet->setColor(QColor(31, 73, 125));
    barSet->setBorderColor(QColor(20, 50, 90));
    for (int i = 0; i < nbRegions; i++)
        *barSet << counts[i];
    animateBarSet(barSet);
    QBarSeries *series = new QBarSeries();
    series->append(barSet);
    series->setBarWidth(0.4);

    // --- Créer le graphique ---
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setMargins(QMargins(0, 20, 0, 0));
    chart->legend()->hide();
    chart->setBackgroundVisible(false);
    chart->setPlotAreaBackgroundVisible(false);

    // --- Axe X ---
    QBarCategoryAxis *x = new QBarCategoryAxis();
    for (int i = 0; i < nbRegions; i++)
        x->append(regions[i]);
    x->setTitleText("Regions");
    x->setTitleBrush(QBrush(QColor(50, 72, 95)));
    x->setTitleFont(QFont("Onest", 9, QFont::Bold));
    x->setLabelsAngle(-45);
    x->setLinePenColor(QColor(141, 162, 181));
    x->setGridLineVisible(false);
    chart->addAxis(x, Qt::AlignBottom);
    series->attachAxis(x);

    // --- Axe Y ---
    int maxVal = 0;
    for (int i = 0; i < nbRegions; i++)
        if (counts[i] > maxVal) maxVal = counts[i];
    int maxScale = ((maxVal / 5) + 1) * 5;

    QValueAxis *y = new QValueAxis();
    y->setLabelFormat("%d");
    y->setMin(0);
    y->setMax(maxScale);
    y->setTickCount((maxScale / 5) + 1);
    y->setMinorTickCount(0);
    y->setTitleText("Nb of Municipalities");
    y->setTitleBrush(QBrush(QColor(50, 72, 95)));
    y->setTitleFont(QFont("Onest", 9, QFont::Bold));
    y->setGridLineColor(QColor(141, 162, 181));
    y->setLinePenColor(QColor(141, 162, 181));
    chart->addAxis(y, Qt::AlignLeft);
    series->attachAxis(y);

    // --- Afficher ---
    ui->widget_satistique_m1->setChart(chart);
    ui->widget_satistique_m1->setRenderHint(QPainter::Antialiasing);
}

void GWasteCollection::on_lineEdit_barrederecherche_m2_textChanged(const QString &arg1)
{
    if (m_mapWidget && m_mapWidget->rootObject()) {
        m_mapWidget->rootObject()->setProperty("searchQuery", arg1);
    }
}

void GWasteCollection::on_comboBox_floodlevel_m2_currentIndexChanged(int index)
{
    if (m_mapWidget && m_mapWidget->rootObject()) {
        QStringList levels = {"all","critique","eleve","modere","faible","aucun"};
        QString level = levels[index];
        m_mapWidget->rootObject()->setProperty("riskFilter", level);
    }
}

// ═══════════════════════════════════════════════════
// ══════════ INTERACTIVE MAP - MUNICIPALITY ═════════
// ═══════════════════════════════════════════════════

QVariantList GWasteCollection::buildMunicipalityData()
{
    Municipality m;
    return m.getMunicipalitiesMapData();
}

void GWasteCollection::loadMapData()
{
    if (!m_mapWidget) return;
    QQuickItem *root = m_mapWidget->rootObject();
    if (!root) return;
    QVariantList data = buildMunicipalityData();
    root->setProperty("municipalityModel", QVariant::fromValue(data));
}

void GWasteCollection::initMapWidget()
{
    if (!m_mapWidget) {
        m_mapWidget = new QQuickWidget(ui->frame_map_m2);
        m_mapWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
        m_mapWidget->setGeometry(0, 0,
                                 ui->frame_map_m2->width(),
                                 ui->frame_map_m2->height());
        m_mapWidget->setSource(QUrl("qrc:/fichier/map.qml"));
        m_mapWidget->show();

        QTimer::singleShot(500, this, [this]() {
            m_mapLoaded = true;
            loadMapData();
        });

    } else {
        ui->comboBox_floodlevel_m2->setCurrentIndex(0);
        ui->lineEdit_barrederecherche_m2->clear();
        // Réinitialiser le zoom et le centre de la carte
        QQuickItem *root = m_mapWidget->rootObject();
        if (root) {
            QMetaObject::invokeMethod(root, "resetView");
        }
        loadMapData();
    }
}

void GWasteCollection::displayBudgetTable(double totalBudget)
{
    // ── Supprimer l'ancien modèle ─────────────────────────────
    if (m_budgetModel) {
        delete m_budgetModel;
        m_budgetModel = nullptr;
    }

    Municipality m;
    double totalScore = m.calculateTotalScore();
    QSqlQueryModel *listModel = m.getBudgetMunicipalityList();

    m_budgetModel = new QStandardItemModel();
    m_budgetModel->setHorizontalHeaderLabels({
        "ID", "Name", "Region", "Address",
        "Nb Sewers", "Avg Blockage %",
        "Nb Sewers Cleaned", "Percentage", "Budget (DT)"
    });

    for (int row = 0; row < listModel->rowCount(); row++) {
        int     id      = listModel->data(listModel->index(row, 0)).toInt();
        QString name    = listModel->data(listModel->index(row, 1)).toString();
        QString region  = listModel->data(listModel->index(row, 2)).toString();
        QString address = listModel->data(listModel->index(row, 3)).toString();

        m_budgetModel->appendRow({
            new QStandardItem(QString::number(id)),
            new QStandardItem(name),
            new QStandardItem(region),
            new QStandardItem(address),
            new QStandardItem(QString::number(m.getNbSewers(id))),
            new QStandardItem(QString::number(m.getAvgBlockageRate(id), 'f', 1) + " %"),
            new QStandardItem(QString::number(m.getNbInterventions(id))),
            new QStandardItem(QString::number(m.calculatePercentage(id, totalScore), 'f', 1) + " %"),
            new QStandardItem(QString::number(m.calculateBudget(id, totalScore, totalBudget), 'f', 2))
        });
    }

    ui->tableView_budget_m3->setModel(m_budgetModel);
    ui->tableView_budget_m3->horizontalHeader()
        ->setSectionResizeMode(QHeaderView::ResizeToContents);
    delete listModel;
}

void GWasteCollection::on_lineEdit_barrederecherche_m3_textChanged(const QString &arg1)
{
    QSortFilterProxyModel *Model = new QSortFilterProxyModel(this);
    Model->setSourceModel(m_budgetModel);
    Model->setFilterCaseSensitivity(Qt::CaseInsensitive);
    Model->setFilterKeyColumn(1);
    Model->setFilterFixedString(arg1);
    ui->tableView_budget_m3->setModel(Model);
}

void GWasteCollection::on_pushButton_sort_m3_clicked()
{
    if (m_budgetModel) {
        QSortFilterProxyModel *Model = new QSortFilterProxyModel(this);
        Model->setSourceModel(m_budgetModel);
        ui->tableView_budget_m3->setModel(Model);
        Model->sort(8, Qt::DescendingOrder);
    }
}

void GWasteCollection::on_pushButton_export_m3_clicked()
{
    QString filePath = QFileDialog::getSaveFileName(this, "Export Budget to PDF",
                                                    QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/budget_report "+ QDateTime::currentDateTime().toString("yyyy-MM-dd hh-mm-ss")+" .pdf",
                                                    "PDF Files (*.pdf)");
    if (!filePath.isEmpty())
    {
        QAbstractItemModel *model = ui->tableView_budget_m3->model();
        if (model)
        {
            QPdfWriter pdfWriter(filePath);
            pdfWriter.setPageSize(QPageSize(QPageSize::A4));
            pdfWriter.setPageOrientation(QPageLayout::Landscape);
            pdfWriter.setPageMargins(QMarginsF(10, 10, 10, 10), QPageLayout::Millimeter);
            pdfWriter.setResolution(96);

            QPainter painter(&pdfWriter);
            if (painter.isActive())
            {
                int pageWidth  = painter.viewport().width();
                int pageHeight = painter.viewport().height();
                int y = 20;

                // --- Titre ---
                painter.setFont(QFont("Arial", 16, QFont::Bold));
                painter.drawText(QRect(0, y, pageWidth, 50), Qt::AlignCenter, "Budget Allocation Report");
                y += 60;

                // --- Date ---
                painter.setFont(QFont("Arial", 9));
                painter.setPen(Qt::gray);
                painter.drawText(QRect(0, y, pageWidth, 30), Qt::AlignCenter,
                                 "Generated on: " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
                painter.setPen(Qt::black);
                y += 50;

                // --- Largeurs colonnes ---
                QList<int> colWidths = {40, 120, 100, 160, 80, 100, 100, 80, 100};
                int totalWidth = 0;
                for (int w : colWidths) totalWidth += w;
                if (totalWidth != pageWidth) {
                    float ratio = (float)pageWidth / totalWidth;
                    for (int i = 0; i < colWidths.size(); i++)
                        colWidths[i] = (int)(colWidths[i] * ratio);
                }

                int colCount = model->columnCount();

                // --- En-têtes ---
                painter.setFont(QFont("Arial", 9, QFont::Bold));
                painter.setBrush(QColor(50, 72, 95));
                painter.setPen(Qt::NoPen);
                painter.drawRect(0, y, pageWidth, 35);
                painter.setPen(Qt::white);
                int x = 0;
                for (int col = 0; col < colCount && col < colWidths.size(); col++) {
                    painter.drawText(QRect(x + 5, y, colWidths[col] - 5, 35),
                                     Qt::AlignVCenter | Qt::AlignCenter | Qt::TextWordWrap,
                                     model->headerData(col, Qt::Horizontal).toString());
                    x += colWidths[col];
                }
                y += 35;

                // --- Données ---
                painter.setFont(QFont("Arial", 8));
                bool alternate = false;
                for (int row = 0; row < model->rowCount(); row++) {
                    int rowHeight = 30;
                    for (int col = 0; col < colCount && col < colWidths.size(); col++) {
                        QRect br = painter.boundingRect(QRect(0, 0, colWidths[col] - 10, 1000),
                                                        Qt::AlignTop | Qt::AlignLeft | Qt::TextWordWrap,
                                                        model->data(model->index(row, col)).toString());
                        rowHeight = qMax(rowHeight, br.height() + 10);
                    }
                    if (y + rowHeight > pageHeight - 40) {
                        pdfWriter.newPage();
                        y = 40;
                    }
                    painter.setPen(Qt::NoPen);
                    if (alternate) {
                        painter.setBrush(QColor(236, 240, 241));
                    } else {
                        painter.setBrush(Qt::white);
                    }
                    painter.drawRect(0, y, pageWidth, rowHeight);
                    alternate = !alternate;
                    painter.setPen(Qt::black);
                    x = 0;
                    for (int col = 0; col < colCount && col < colWidths.size(); col++) {
                        painter.drawText(QRect(x + 5, y + 5, colWidths[col] - 10, rowHeight),
                                         Qt::AlignTop | Qt::AlignHCenter | Qt::TextWordWrap,
                                         model->data(model->index(row, col)).toString());
                        x += colWidths[col];
                    }
                    painter.setPen(QColor(189, 195, 199));
                    painter.drawLine(0, y + rowHeight, pageWidth, y + rowHeight);
                    y += rowHeight;
                }
                painter.end();
                QMessageBox::information(this, "Export Success",
                                         "Budget exported to PDF successfully !");
            }
        }
    }
}



//-----------------------------------------------------------------------------------------------------------------
//----------------------------------------------Sewer Module-------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------

void GWasteCollection::clearSewerForm()
{
    ui->comboBox_municialityname_s1->setCurrentIndex(0);
    ui->lineEdit_Location_S1->clear();
    ui->lineEdit_waterlevel_S1->clear();
    ui->lineEdit_CurrentcapS1->clear();
    ui->lineEdit_Maxcap_S1->clear();
    ui->lineEdit_search_S1->clear();
    ui->tableView_S1->clearSelection();
    ui->lineEdit_waterlevel_S1->setEnabled(true);

}

void GWasteCollection::on_pushButton_employee_S3_clicked()
{
    ui->tableView_list_e4->setModel(Etmp.displayEmployees());
    displayEmployeeStatistics();
    ui->SWWasteCollection->setCurrentIndex(0);
    ui->stackedWidget_e->setCurrentIndex(3);
}

void GWasteCollection::on_pushButton_municipality_S3_clicked()
{
    ui->tableView_listemunicipality_m1->setModel(Mtmp.displayMunicipality());
    displayMunicipalityStatistics();
    ui->SWWasteCollection->setCurrentIndex(1);
    ui->stackedWidget_m1->setCurrentIndex(0);
    ui->stackedWidget_form->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_logout_S3_clicked()
{
    logout();
}

void GWasteCollection::on_pushButton_employee_S2_clicked()
{
    ui->tableView_list_e4->setModel(Etmp.displayEmployees());
    displayEmployeeStatistics();
    ui->SWWasteCollection->setCurrentIndex(0);
    ui->stackedWidget_e->setCurrentIndex(3);
}

void GWasteCollection::on_pushButton_municipality_S2_clicked()
{
    ui->tableView_listemunicipality_m1->setModel(Mtmp.displayMunicipality());
    displayMunicipalityStatistics();
    ui->SWWasteCollection->setCurrentIndex(1);
    ui->stackedWidget_m1->setCurrentIndex(0);
    ui->stackedWidget_form->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_logout_S2_clicked()
{
    logout();
}

void GWasteCollection::on_pushButton_employee_S1_clicked()
{
    clearSewerForm();
    ui->tableView_list_e4->setModel(Etmp.displayEmployees());
    displayEmployeeStatistics();
    ui->SWWasteCollection->setCurrentIndex(0);
    ui->stackedWidget_e->setCurrentIndex(3);
}

void GWasteCollection::on_pushButton_municipality_S1_clicked()
{
    clearSewerForm();
    ui->tableView_listemunicipality_m1->setModel(Mtmp.displayMunicipality());
    displayMunicipalityStatistics();
    ui->SWWasteCollection->setCurrentIndex(1);
    ui->stackedWidget_m1->setCurrentIndex(0);
    ui->stackedWidget_form->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_logout_S1_clicked()
{
    logout();
}

void GWasteCollection::on_pushButton_SewerAI_S1_clicked()
{
    clearSewerForm();
    ui->stackedWidgetS->setCurrentIndex(1);
}


void GWasteCollection::on_pushButton_back_S2_clicked()
{
    ui->stackedWidgetS->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_back_S3_clicked()
{
    ui->stackedWidgetS->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_center_S2_clicked()
{
    ui->tableView_r1->setModel(Rtmp.displayCenter());
    ui->SWWasteCollection->setCurrentIndex(4);
    ui->stackedWidget_r->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_center_S3_clicked()
{
    ui->tableView_r1->setModel(Rtmp.displayCenter());
    ui->SWWasteCollection->setCurrentIndex(4);
    ui->stackedWidget_r->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_center_S1_clicked()
{
    clearSewerForm();
    ui->tableView_r1->setModel(Rtmp.displayCenter());
    ui->SWWasteCollection->setCurrentIndex(4);
    ui->stackedWidget_r->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_team_S1_clicked()
{
    clearSewerForm();
    ui->tableView_listteams_c1->setModel(Ctmp.DisplayTeams());
    ui->SWWasteCollection->setCurrentIndex(3);
    ui->stackedWidget_c->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_team_S2_clicked()
{
    ui->tableView_listteams_c1->setModel(Ctmp.DisplayTeams());
    ui->SWWasteCollection->setCurrentIndex(3);
    ui->stackedWidget_c->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_team_S3_clicked()
{
    ui->tableView_listteams_c1->setModel(Ctmp.DisplayTeams());
    ui->SWWasteCollection->setCurrentIndex(3);
    ui->stackedWidget_c->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_confirm_S1_clicked()
{
    QString location = ui->lineEdit_Location_S1->text().trimmed();
    bool valid = true;

    //LOCATION
    QStringList parts = location.split(",");

    if (location.isEmpty() || parts.size() != 2)
        valid = false;
    else
    {
        bool okLat = false;
        bool okLng = false;

        double latitude = parts[0].trimmed().toDouble(&okLat);
        double longitude = parts[1].trimmed().toDouble(&okLng);

        // correction automatique si clairement inversés
        if (okLat && okLng && qAbs(latitude) > 90.0 && qAbs(longitude) <= 90.0)
        {
            double temp = latitude;
            latitude = longitude;
            longitude = temp;
        }

        if (!okLat || !okLng || latitude < -90.0 || latitude > 90.0 || longitude < -180.0 || longitude > 180.0)
            valid = false;
        else
        {
            // normalisation du texte sauvegardé pour eviter les erreurs
            location = QString::number(latitude, 'f', 6) + "," + QString::number(longitude, 'f', 6);
            ui->lineEdit_Location_S1->setText(location);
        }
    }

    if(!valid)
    {
        QMessageBox::critical(nullptr,"Invalid location",
                              "Location must be in this format:\n"
                              "latitude,longitude\n\n"
                              "Example: 36.851245,10.203144",
                              QMessageBox::Cancel);
    }
    else
    {
        bool n1;

        // MAX CAPACITY
        int maxCapacity = ui->lineEdit_Maxcap_S1->text().toInt(&n1);

        if(maxCapacity <= 0 || maxCapacity > 100 || n1==false )
            valid = false;

        if(!valid)
        {
            QMessageBox::critical(nullptr,"Invalid Max Capacity",
                                  "Max Capacity must be > 0 and <100",
                                  QMessageBox::Cancel);
        }
        else
        {
            // CURRENT CAPACITY
            int currentCapacity = ui->lineEdit_CurrentcapS1->text().toInt(&n1);

            if(currentCapacity < 0 || currentCapacity >100 || n1==false)
                valid = false;

            if(!valid)
            {
                QMessageBox::critical(nullptr,"Invalid Current Capacity",
                                      "Current Capacity invalid",
                                      QMessageBox::Cancel);
            }
            else
            {
                if(currentCapacity > maxCapacity)
                    valid = false;

                if(!valid)
                {
                    QMessageBox::critical(nullptr,"Error",
                                          "Current Capacity > Max Capacity",
                                          QMessageBox::Cancel);
                }
                else
                {
                    // WATER LEVEL
                    int waterLevel = ui->lineEdit_waterlevel_S1->text().toInt(&n1);

                    if(waterLevel < 0 || waterLevel > 100 || n1==false)
                        valid = false;

                    if(!valid)
                    {
                        QMessageBox::critical(nullptr,"Invalid Water Level",
                                              "Water Level must be between 0 and 100",
                                              QMessageBox::Cancel);
                    }
                    else
                    {

                        int blockageRate;
                        if(maxCapacity != 0)
                            blockageRate = (currentCapacity * 100) / maxCapacity;
                        else
                            blockageRate = 0;
                        int muniindex=ui->comboBox_municialityname_s1->currentIndex();
                        QModelIndex indice=ui->comboBox_municialityname_s1->model()->index(muniindex, 0);
                        int municipalityID = indice.data().toInt();
                        bool test;

                        //UPDATE si ligne sélectionnée
                        if(ui->tableView_S1->selectionModel()->hasSelection())
                        {
                            QModelIndex index =
                                ui->tableView_S1->selectionModel()->currentIndex();

                            int id = index.sibling(index.row(),0).data().toInt();

                            Sewer S(id, location, maxCapacity,
                                    currentCapacity, waterLevel,
                                    blockageRate, municipalityID);

                            test = S.updateSewer();
                            if (ret==0){
                            A.write_to_arduino(Mtmp.getMunicipalityRiskMaquette(Municipality_id_Maquette).toUtf8()+"\n");
                                reloadCenterStats_r();
                            }
                        }
                        else
                        {
                            // INSERT
                            Sewer S(0, location, maxCapacity,
                                    currentCapacity, waterLevel,
                                    blockageRate, municipalityID);

                            test = S.addSewer();
                            if(ret==0){
                            A.write_to_arduino(Mtmp.getMunicipalityRiskMaquette(Municipality_id_Maquette).toUtf8()+"\n");
                                reloadCenterStats_r();
                            }
                        }

                        if(test)
                        {
                            ui->comboBox_municialityname_s1->setCurrentIndex(0);
                            ui->lineEdit_Location_S1->clear();
                            ui->lineEdit_Maxcap_S1->clear();
                            ui->lineEdit_CurrentcapS1->clear();
                            ui->lineEdit_waterlevel_S1->clear();

                            Sewer S;
                            ui->tableView_S1->setModel(S.displaySewer());
                            generateSewerStats();

                            QMessageBox::information(nullptr,"OK",
                                                     "Success",
                                                     QMessageBox::Cancel);
                        }
                        else
                        {
                            QMessageBox::critical(nullptr,"Not OK",
                                                  "Operation failed",
                                                  QMessageBox::Cancel);
                        }
                    }
                }
            }
        }
    }
}
void GWasteCollection::on_tableView_S1_clicked(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    int SewerID = index.sibling(index.row(), 0).data().toInt();

    ui->lineEdit_Location_S1->setText(
        index.sibling(index.row(), 1).data().toString());

    ui->lineEdit_Maxcap_S1->setText(
        index.sibling(index.row(), 2).data().toString());

    ui->lineEdit_CurrentcapS1->setText(
        index.sibling(index.row(), 3).data().toString());

    ui->lineEdit_waterlevel_S1->setText(
        index.sibling(index.row(), 4).data().toString());

    ui->comboBox_municialityname_s1->setCurrentText(
        index.sibling(index.row(), 6).data().toString());

    if (ret == 0 && (SewerID == 1|| SewerID == 2 || SewerID == 3))
    {
        ui->lineEdit_waterlevel_S1->setEnabled(false);
    }
    else
    {
        ui->lineEdit_waterlevel_S1->setEnabled(true);
    }
}

void GWasteCollection::on_tableView_S1_doubleClicked(const QModelIndex &index)
{
    int SewerID = index.sibling(index.row(), 0).data().toInt();

    if (SewerID == 1 || SewerID == 2 || SewerID == 3)
    {
        QMessageBox::warning(this,
                             "Deletion blocked",
                             "Sewers with IDs 1, 2 and 3 cannot be deleted.");
        return;
    }

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(
        this,
        "Confirmation",
        "Do you want to delete this Sewer?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes)
    {
        if (Stemp.deleteSewer(SewerID))
        {
            if(ret==0){
            A.write_to_arduino(Mtmp.getMunicipalityRiskMaquette(Municipality_id_Maquette).toUtf8()+"\n");
                reloadCenterStats_r();
            }
            ui->tableView_S1->setModel(Stemp.displaySewer());
            generateSewerStats();
            on_btnGeneratePlan_S2_clicked();
            ui->comboBox_municialityname_s1->setCurrentIndex(0);
            ui->lineEdit_Location_S1->clear();
            ui->lineEdit_Maxcap_S1->clear();
            ui->lineEdit_CurrentcapS1->clear();
            ui->lineEdit_waterlevel_S1->clear();
        }
    }
}


void GWasteCollection::on_pushButton_deselectionner_S1_clicked()
{
    if (ui->tableView_S1->selectionModel()->hasSelection())
    {
        ui->tableView_S1->clearSelection();
        ui->comboBox_municialityname_s1->setCurrentIndex(0);
        ui->lineEdit_Location_S1->clear();
        ui->lineEdit_Maxcap_S1->clear();
        ui->lineEdit_CurrentcapS1->clear();
        ui->lineEdit_waterlevel_S1->clear();
        ui->lineEdit_waterlevel_S1->setEnabled(true);
    }

}

void GWasteCollection::on_pushButton_sort_S1_clicked()
{
    ui->tableView_S1->setModel(Stemp.displaySortedSewerByBlockage());

}

void GWasteCollection::on_lineEdit_search_S1_textChanged(const QString &arg1)
{
    if(arg1.isEmpty())
        ui->tableView_S1->setModel(Stemp.displaySewer());
    else
        ui->tableView_S1->setModel(Stemp.searchSewerByLocation(arg1));
}
void GWasteCollection::on_pushButton_export_S1_clicked()
{
    QAbstractItemModel *model = ui->tableView_S1->model();
    if (!model) return;

    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Export Sewer Data",
        "",
        "Excel Files (*.csv)"
        );

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Cannot create file");
        return;
    }

    QTextStream out(&file);

    // 🔹 Headers
    for (int col = 0; col < model->columnCount(); col++) {
        out << model->headerData(col, Qt::Horizontal).toString();
        if (col < model->columnCount() - 1)
            out << ";";
    }
    out << "\n";

    // 🔹 Data
    for (int row = 0; row < model->rowCount(); row++) {
        for (int col = 0; col < model->columnCount(); col++) {
            QString data = model->data(model->index(row, col)).toString();
            data.replace(";", ",");
            out << data;

            if (col < model->columnCount() - 1)
                out << ";";
        }
        out << "\n";
    }

    file.close();

    QMessageBox::information(this, "Export",
                             "Sewer Export réussi !");
}void GWasteCollection::generateSewerStats()
{
    // Nettoyage propre du layout existant
    if (QLayout *oldLayout = ui->statisticsWidget_S1->layout())
    {
        QLayoutItem *item;
        while ((item = oldLayout->takeAt(0)) != nullptr)
        {
            if (item->widget())
                item->widget()->deleteLater();
            delete item;
        }
        delete oldLayout;
    }

    QSqlQueryModel *model = Stemp.displaySewer();

    int low = 0, moderate = 0, high = 0, critical = 0;

    for (int row = 0; row < model->rowCount(); row++)
    {
        QModelIndex index = model->index(row, 5); // colonne Blockage Rate
        int rate = model->data(index).toInt();

        if (rate <= 25) low++;
        else if (rate <= 50) moderate++;
        else if (rate <= 75) high++;
        else critical++;
    }

    // Calcul du total pour les pourcentages
    int total = low + moderate + high + critical;
    if (total == 0) total = 1; // éviter division par 0

    QPieSeries *series = new QPieSeries();
    series->setPieSize(0.95);

    series->append("Low", low);
    series->append("Moderate", moderate);
    series->append("High", high);
    series->append("Critical", critical);

    // Récupération des slices
    QPieSlice *sliceLow = series->slices().at(0);
    QPieSlice *sliceModerate = series->slices().at(1);
    QPieSlice *sliceHigh = series->slices().at(2);
    QPieSlice *sliceCritical = series->slices().at(3);

    // Labels avec pourcentages (affichés dans la légende)
    sliceLow->setLabel(QString("Low (%1%)")
                           .arg(qRound((low * 100.0) / total)));

    sliceModerate->setLabel(QString("Moderate (%1%)")
                                .arg(qRound((moderate * 100.0) / total)));

    sliceHigh->setLabel(QString("High (%1%)")
                            .arg(qRound((high * 100.0) / total)));

    sliceCritical->setLabel(QString("Critical (%1%)")
                                .arg(qRound((critical * 100.0) / total)));

    // Création du chart
    QChart *chart = new QChart();
    chart->addSeries(series);

    // Animation
    chart->setAnimationOptions(QChart::SeriesAnimations);

    // Légende
    chart->legend()->setAlignment(Qt::AlignBottom);

    // Style propre
    chart->setMargins(QMargins(0,0,0,0));
    chart->layout()->setContentsMargins(0,0,0,0);
    chart->setBackgroundVisible(false);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setStyleSheet("background: transparent");

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(chartView, 1);

    ui->statisticsWidget_S1->setLayout(layout);
}
void GWasteCollection::on_comboBoxRegion_S2_currentTextChanged(const QString &region)
{
    int waterWeight = ui->slWaterW_S2->value();
    int blockageWeight = ui->slBlockageW_S2 ->value();
    ui->tableViewListpriority_S2->setModel(
        Stemp.getPrioritizedSewersByRegion(region, waterWeight, blockageWeight, currentChanceOfRain)
        );
    ui->tableViewListpriority_S2->resizeColumnsToContents();
    ui->tableViewListpriority_S2->horizontalHeader()->setStretchLastSection(true);
}

void GWasteCollection::on_slWaterW_S2_valueChanged(int value)
{
    ui->lblWaterValue_S2->setText(QString::number(value));

}


void GWasteCollection::on_slBlockageW_S2_valueChanged(int value)
{
    ui->lblblockageValue_S2->setText(QString::number(value));

}

void GWasteCollection::on_pushButton_PredictIQ_S2_clicked()
{
    // récupérer la région de S2
    QString selectedRegion = ui->comboBoxRegion_S2->currentText();

    // la mettre dans S3
    ui->comboBoxSewerID_S3->setCurrentText(selectedRegion);

    // charger les sewers automatiquement
    QSqlQueryModel *model = Stemp.getSewersByRegion(selectedRegion);

    ui->comboBoxSewerID_S3->clear();

    for (int i = 0; i < model->rowCount(); i++)
    {
        QString sewerId = model->index(i, 0).data().toString();
        ui->comboBoxSewerID_S3->addItem(sewerId);
    }
    ui->stackedWidgetS->setCurrentIndex(2);
}




void GWasteCollection::on_pushButton_weather_S2_clicked()
{
    QString region = ui->comboBoxRegion_S2->currentText().trimmed();

    if (region.isEmpty())
    {
        QMessageBox::warning(this, "Warning", "Please select a region first.");
        return;
    }

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QString url = "https://wttr.in/" + QUrl::toPercentEncoding(region) + "?format=j1";

    connect(manager, &QNetworkAccessManager::finished, this,
            [this](QNetworkReply *reply)
            {
                if (!reply)
                {
                    QMessageBox::critical(this, "Weather Error", "No response received.");
                    return;
                }

                if (reply->error() != QNetworkReply::NoError)
                {
                    QMessageBox::critical(this, "Weather Error", reply->errorString());
                    reply->deleteLater();
                    return;
                }

                QJsonDocument jsonResponse = QJsonDocument::fromJson(reply->readAll());
                reply->deleteLater();

                if (!jsonResponse.isObject())
                {
                    QMessageBox::critical(this, "Weather Error", "Invalid weather response.");
                    return;
                }

                QJsonObject root = jsonResponse.object();
                QJsonArray currentConditionsArray = root["current_condition"].toArray();

                if (currentConditionsArray.isEmpty())
                {
                    QMessageBox::critical(this, "Weather Error", "No current weather data found.");
                    return;
                }

                QJsonObject current = currentConditionsArray[0].toObject();

                QString weatherDescription = "Unknown";
                QJsonArray weatherDescriptionArray = current["weatherDesc"].toArray();
                if (!weatherDescriptionArray.isEmpty())
                {
                    weatherDescription = weatherDescriptionArray[0].toObject()["value"].toString();
                }

                QString rainChance = "0";
                QJsonArray weatherArray = root["weather"].toArray();

                if (!weatherArray.isEmpty())
                {
                    QJsonArray hourlyArray = weatherArray[0].toObject()["hourly"].toArray();

                    if (!hourlyArray.isEmpty())
                    {
                        int currentHour = QTime::currentTime().hour();
                        int closestIndex = 0;
                        int smallestDifference = 9999;

                        for (int i = 0; i < hourlyArray.size(); ++i)
                        {
                            QJsonObject hourlyForecast = hourlyArray[i].toObject();
                            int forecastTime = hourlyForecast["time"].toString().toInt();
                            int forecastHour = forecastTime / 100;
                            int difference = qAbs(forecastHour - currentHour);

                            if (difference < smallestDifference)
                            {
                                smallestDifference = difference;
                                closestIndex = i;
                            }
                        }

                        QJsonObject closestHourlyForecast = hourlyArray[closestIndex].toObject();
                        rainChance = closestHourlyForecast["chanceofrain"].toString();
                    }
                }

                bool ok = false;
                int parsedRainChance = rainChance.toInt(&ok);
                currentChanceOfRain = ok ? parsedRainChance : 0;

                ui->labelWeatherInfo_S2->setText(
                    "Condition: " + weatherDescription + "\n"
                                                         "Temperature: " + current["temp_C"].toString() + " °C\n"
                                                     "Humidity: " + current["humidity"].toString() + " %\n"
                                                       "Rain Risk: " + rainChance + " %\n"
                                   "Wind: " + current["windspeedKmph"].toString() + " km/h"
                    );
            });

    manager->get(QNetworkRequest(QUrl(url)));

}

void GWasteCollection::on_btnGeneratePlan_S2_clicked()
{
    QString region = ui->comboBoxRegion_S2->currentText().trimmed();

    if (region.isEmpty())
    {
        QMessageBox::warning(this, "Warning", "Please select a region first.");
        return;
    }

    int waterWeight = ui->slWaterW_S2->value();
    int blockageWeight = ui->slBlockageW_S2->value();

    // Charger les données (tri priorité)
    ui->tableViewListpriority_S2->setModel(
        Stemp.getPrioritizedSewersByRegion(region, waterWeight, blockageWeight, currentChanceOfRain)
        );

    ui->tableViewListpriority_S2->resizeColumnsToContents();
    ui->tableViewListpriority_S2->horizontalHeader()->setStretchLastSection(true);

    QAbstractItemModel *model = ui->tableViewListpriority_S2->model();

    if (!model || model->rowCount() == 0)
    {
        ui->textEditRoutePreview_S2->setPlainText("No route available.");
        mapsUrlS2.clear();
        return;
    }

    //Extraire les données
    QList<QString> sewerIds;
    QList<QString> locations;
    QList<double> lats;
    QList<double> lons;

    for (int i = 0; i < model->rowCount(); i++)
    {
        QString id = model->index(i, 0).data().toString();
        QString loc = model->index(i, 1).data().toString().trimmed();

        double lat, lon;

        if (!parseLocation(loc, lat, lon))
            continue;

        sewerIds.append(id);
        locations.append(loc);
        lats.append(lat);
        lons.append(lon);
    }

    if (sewerIds.isEmpty())
    {
        ui->textEditRoutePreview_S2->setPlainText("No valid route.");
        mapsUrlS2.clear();
        return;
    }

    // Optimiser l’ordre (distance)
    QList<int> order = buildOptimizedOrder(lats, lons);

    // Construire affichage
    QString routeText = "Optimized route:\n\nStart\n\n";
    QStringList mapPoints;

    for (int i = 0; i < order.size(); i++)
    {
        int idx = order[i];

        routeText += QString::number(i + 1) + ". Sewer " + sewerIds[idx] + "\n";
        routeText += "   " + locations[idx] + "\n";

        if (i != order.size() - 1)
            routeText += "   |\n   v\n";

        mapPoints << locations[idx];
    }

    routeText += "\nEnd";

    ui->textEditRoutePreview_S2->setPlainText(routeText);

    // Google Maps
    if (!mapPoints.isEmpty())
        mapsUrlS2 = "https://www.google.com/maps/dir/" + mapPoints.join("/");
    else
        mapsUrlS2.clear();
}
void GWasteCollection::on_pushButton_viewMaps_S2_clicked()
{
    if (mapsUrlS2.isEmpty())
    {
        QMessageBox::warning(this, "Warning", "Generate the optimized plan first.");
        return;
    }
    QDesktopServices::openUrl(QUrl(mapsUrlS2));
}

void GWasteCollection::updatePredictionS3()
{
    QString blockageText = ui->lePastBlockage_S3->text().trimmed();
    QString waterText = ui->lePastWater_S3->text().trimmed();

    if (blockageText.isEmpty() || waterText.isEmpty())
    {
        ui->lblRisk_S3->clear();
        ui->lblDate_S3->clear();
        ui->lblrecommendedA_S3->clear();
        return;
    }

    int blockageRate = blockageText.toInt();
    int waterLevel = waterText.toInt();

    QDate fromDate = ui->dateEdit_S3->date();
    QDate toDate = ui->dateEdit_to_S3->date();

    int days = fromDate.daysTo(toDate);

    if (days < 0)
    {
        ui->lblRisk_S3->setText("Invalid range");
        ui->lblDate_S3->clear();
        ui->lblrecommendedA_S3->clear();
        return;
    }

    double currentRisk = (0.6 * blockageRate) + (0.4 * waterLevel);
    double predictedRisk = currentRisk + (days * 0.25);

    if (predictedRisk > 100)
        predictedRisk = 100;

    double dailyIncrease = 0.25;
    int daysToCritical;

    if (currentRisk >= 80)
        daysToCritical = 0;
    else
        daysToCritical = qCeil((80 - currentRisk) / dailyIncrease);

    QDate estimatedDate = QDate::currentDate().addDays(daysToCritical);

    QString action;

    if (predictedRisk < 40){
        ui->lblRisk_S3->setStyleSheet("color: green; font-weight: bold; font-size: 18px;");
        action = "Routine monitoring";}
    else if (predictedRisk < 70){
        ui->lblRisk_S3->setStyleSheet("color: orange; font-weight: bold; font-size: 18px;");
        action = "Schedule preventive maintenance";}
    else if (predictedRisk < 85){
        ui->lblRisk_S3->setStyleSheet("color: red; font-weight: bold; font-size: 18px;");
        action = "Plan urgent inspection";}
    else{
        ui->lblRisk_S3->setStyleSheet("color: red; font-weight: bold; font-size: 18px;");
        action = "Immediate intervention required";}


    ui->lblRisk_S3->setText(QString::number(predictedRisk, 'f', 0) + "%");
    ui->lblDate_S3->setText(estimatedDate.toString("dd-MM-yyyy"));
    ui->lblrecommendedA_S3->setText(action);


}

void GWasteCollection::on_dateEdit_S3_dateChanged(const QDate &)
{
    updatePredictionS3();
}


void GWasteCollection::on_dateEdit_to_S3_dateChanged(const QDate &)
{
    updatePredictionS3();
}

void GWasteCollection::on_pushButton_SewerAI_S3_clicked()
{
    ui->stackedWidgetS->setCurrentIndex(1);
}

void GWasteCollection::on_comboBoxSewerID_S3_currentTextChanged(const QString &text)
{
    int sewerId = text.toInt();

    Sewer sewer;

    if (sewer.getSewerById(sewerId))
    {
        ui->leLocation_S3->setText(sewer.getLocation());
        ui->lePastBlockage_S3->setText(QString::number(sewer.getBlockageRate()));
        ui->lePastWater_S3->setText(QString::number(sewer.getWaterLevel()));
        updatePredictionS3();
    }
    else
    {
        ui->leLocation_S3->clear();
        ui->lePastBlockage_S3->clear();
        ui->lePastWater_S3->clear();
        ui->lblRisk_S3->clear();
        ui->lblDate_S3->clear();
        ui->lblrecommendedA_S3->clear();

    }
}

void GWasteCollection::on_toolButtonMap_S3_clicked()
{
    QString location = ui->leLocation_S3->text().trimmed();

    if (location.isEmpty())
    {
        QMessageBox::warning(this, "Warning", "No location available.");
        return;
    }

    QString mapsUrl = "https://www.google.com/maps?q=" + location;
    QDesktopServices::openUrl(QUrl(mapsUrl));
}

void GWasteCollection::on_pushButton_saveplan_S2_clicked()
{
    QAbstractItemModel *model = ui->tableViewListpriority_S2->model();

    if (!model || model->rowCount() == 0)
    {
        QMessageBox::warning(this, "Warning", "No plan to save.");
        return;
    }

    QList<int> sewerIds;
    QList<double> lats;
    QList<double> lons;
    for (int i = 0; i < model->rowCount(); i++)
    {
        int sewerId = model->index(i, 0).data().toInt();
        QString location = model->index(i, 1).data().toString().trimmed();

        double lat, lon;
        if (!parseLocation(location, lat, lon))
        {
            QMessageBox::warning(this, "Warning",
                                 "Invalid location for sewer ID " + QString::number(sewerId));
            return;
        }

        sewerIds.append(sewerId);
        lats.append(lat);
        lons.append(lon);
    }

    QList<int> optimizedOrder = buildOptimizedOrder(lats, lons);

    bool ok = true;
    bool doublons = false;

    for (int i = 0; i < optimizedOrder.size(); i++)
    {
        int idx = optimizedOrder[i];
        int sewerId = sewerIds[idx];
        int ordrePassage = i + 1;

        if (Stemp.interventionExists(sewerId))
        {
            doublons = true;
            continue;
        }

        if (!Stemp.addIntervention(sewerId, ordrePassage))
            ok = false;
    }

    if (doublons && ok)
        QMessageBox::warning(this, "Warning",
                             "Some sewers are already planned for tomorrow and cannot be saved again.");
    else if (ok)
        QMessageBox::information(this, "Success",
                                 "Plan saved successfully by optimized route order.");
    else
        QMessageBox::critical(this, "Error",
                              "Some interventions were not saved.");
}
bool GWasteCollection::parseLocation(const QString &location, double &lat, double &lon)
{
    QStringList parts = location.split(",");

    if (parts.size() != 2)
        return false;

    lat = parts[0].trimmed().toDouble();
    lon = parts[1].trimmed().toDouble();

    return true;
}

double GWasteCollection::distanceBetween(double lat1, double lon1, double lat2, double lon2)
{
    double dx = lat1 - lat2;
    double dy = lon1 - lon2;
    return qSqrt(dx * dx + dy * dy);
}

QList<int> GWasteCollection::buildOptimizedOrder(const QList<double> &lats, const QList<double> &lons)
{
    QList<int> order;

    QList<int> remaining;

    int n = lats.size();

    if (n == 0)
        return order;

    for (int i = 0; i < n; i++)
        remaining.append(i);

    // commencer par le premier point (déjà le plus prioritaire)
    order.append(remaining.takeFirst());

    while (!remaining.isEmpty())
    {
        int current = order.last();

        int bestPos = 0;
        double bestDistance = distanceBetween(
            lats[current], lons[current],
            lats[remaining[0]], lons[remaining[0]]
            );

        for (int i = 1; i < remaining.size(); i++)
        {
            double d = distanceBetween(
                lats[current], lons[current],
                lats[remaining[i]], lons[remaining[i]]
                );

            if (d < bestDistance)
            {
                bestDistance = d;
                bestPos = i;
            }
        }

        order.append(remaining.takeAt(bestPos));
    }

    return order;
}

void GWasteCollection::readSensorValue(const QString &line)
{
    QString data = line;
    data.remove("SEWER:");
    QStringList sensors = data.split(';');

    if (sensors.size() != 3)
    {
        return;
    }

    bool allOk = true;
    bool hasChanged = false;

    for (int i = 0; i < sensors.size(); ++i)
    {
        const QString &sensorData = sensors.at(i);

        QStringList parts = sensorData.split(':');
        if (parts.size() != 2)
        {
            allOk = false;
            break;
        }

        bool okId = false, okValue = false;
        int sewerId = parts[0].trimmed().toInt(&okId);
        int rawValue = parts[1].trimmed().toInt(&okValue);

        if (!okId || !okValue)
        {
            allOk = false;
            break;
        }

        if (sewerId < 1 || sewerId > 3)
            continue;

        int percentage = qBound(0, (rawValue * 100) / 700, 100);

        if (lastSewerLevels.contains(sewerId) &&
            lastSewerLevels[sewerId] == percentage)
        {
            continue;
        }

        if (!Stemp.updateWaterLevelFromSensor(sewerId, percentage))
        {
            allOk = false;
            continue;
        }

        lastSewerLevels[sewerId] = percentage;
        hasChanged = true;
    }

    if (allOk && hasChanged)
    {
        int selectedId = -1;

        if (ui->tableView_S1->selectionModel()->hasSelection())
        {
            QModelIndex index = ui->tableView_S1->selectionModel()->currentIndex();
            selectedId = index.sibling(index.row(), 0).data().toInt();
        }

        ui->tableView_S1->setModel(Stemp.displaySewer());

        if (selectedId != -1)
        {
            QAbstractItemModel *model = ui->tableView_S1->model();

            for (int row = 0; row < model->rowCount(); row++)
            {
                int id = model->index(row, 0).data().toInt();

                if (id == selectedId)
                {
                    QModelIndex newIndex = model->index(row, 0);
                    ui->tableView_S1->selectRow(row);
                    ui->tableView_S1->setCurrentIndex(newIndex);
                    break;
                }
            }
        }

        generateSewerStats();
    }
}

void GWasteCollection::readidsewer(const QString &line)
{
    // Exemple reçu : "capacity 1 ON"

    QString data = line.trimmed();

    QStringList parts = data.split(' ');

    if (parts.size() != 3)
        return;

    // Vérifier format
    if (parts[0] != "capacity")
        return;

    bool ok;
    int sewerId = parts[1].toInt(&ok);
    QString state = parts[2].toLower();

    if (!ok)
        return;

    if (sewerId < 1 || sewerId > 3)
        return;

    if (state != "on" && state != "off")
        return;

    // 🔥 appel fonction BD
    bool success = Stemp.updateCapacityByState(sewerId, state);

    if (success)
    {
        // refresh UI
        ui->tableView_S1->setModel(Stemp.displaySewer());
        generateSewerStats();
        initMapWidget();
        A.write_to_arduino(Mtmp.getMunicipalityRiskMaquette(Municipality_id_Maquette).toUtf8()+"\n");
    }

}

QString GWasteCollection::getAiFolderPath() const
{
    QString aiFolder = QDir::cleanPath(
        QCoreApplication::applicationDirPath() + "/../../../../zigoocareAi"
    );

    return aiFolder;
}

void GWasteCollection::on_pushButton_environmentPredict_S1_clicked()
{
    if (!ui->tableView_S1->selectionModel()->hasSelection())
    {
        QMessageBox::warning(this,
                             "AI Environmental Predictor",
                             "Please select a sewer from the table first.");
        return;
    }

    QModelIndex index = ui->tableView_S1->selectionModel()->currentIndex();
    int sewerId = index.sibling(index.row(), 0).data().toInt();

    int maxCapacity = 0;
    int currentCapacity = 0;
    int waterLevel = 0;
    int blockageRate = 0;
    int municipalityID = 0;

    if (!Stemp.getSewerEnvironmentalInput(sewerId,
                                          maxCapacity,
                                          currentCapacity,
                                          waterLevel,
                                          blockageRate,
                                          municipalityID))
    {
        QMessageBox::critical(this,
                              "AI Environmental Predictor",
                              "Unable to load sewer environmental data.");
        return;
    }

    QString aiFolder = QDir::cleanPath(
        QCoreApplication::applicationDirPath() + "/../../../../zigoocareAi"

    );

    QString scriptPath = QDir(aiFolder).filePath("predict_sewer_environment.py");
   QString modelPath = QDir(aiFolder).filePath("models/sewer_environment_model.pkl");

    if (!QFileInfo::exists(scriptPath))
    {
        QMessageBox::critical(this,
                              "AI Script Error",
                              "Python script not found:\n" + scriptPath);
        return;
    }

    if (!QFileInfo::exists(modelPath))
    {
        QMessageBox::critical(this,
                              "AI Model Error",
                              "AI model not found:\n" + modelPath);
        return;
    }

    QString pythonProgram = QStandardPaths::findExecutable("py");

    if (pythonProgram.isEmpty())
        pythonProgram = QStandardPaths::findExecutable("python");

    if (pythonProgram.isEmpty())
    {
        QMessageBox::critical(this,
                              "Python Error",
                              "Python executable not found.");
        return;
    }

    QStringList arguments;
    arguments << scriptPath
              << QString::number(maxCapacity)
              << QString::number(currentCapacity)
              << QString::number(waterLevel)
              << QString::number(blockageRate)
              << QString::number(municipalityID);

    QProcess process;
    process.setWorkingDirectory(aiFolder);
    process.start(pythonProgram, arguments);

    if (!process.waitForStarted(5000))
    {
        QMessageBox::critical(this,
                              "Python Error",
                              "Python process could not start:\n" + process.errorString());
        return;
    }

    if (!process.waitForFinished(30000))
    {
        process.kill();
        process.waitForFinished();

        QMessageBox::critical(this,
                              "AI Environmental Predictor",
                              "Python prediction timeout.");
        return;
    }

    QString output = process.readAllStandardOutput().trimmed();
    QString error = process.readAllStandardError().trimmed();

    if (!error.isEmpty())
    {
        QMessageBox::critical(this,
                              "Python Error",
                              error);
        return;
    }

    if (output.isEmpty())
    {
        QMessageBox::critical(this,
                              "AI Environmental Predictor",
                              "No output received from AI model.");
        return;
    }

    QStringList result = output.split(";");

    if (result.size() < 2)
    {
        QMessageBox::critical(this,
                              "AI Environmental Predictor",
                              "Invalid AI output:\n" + output);
        return;
    }

    QString status = result[0].trimmed();
    double confidence = result[1].toDouble() * 100.0;

    QString meaning;
    QString sdgIndication;
    QString statusColor;

    if (status == "Good")
    {
        statusColor = "#2E7D32";
        meaning = "Stable sewer. No ecological danger detected.";

        sdgIndication =
            "SDG contribution:\n"
            "- ODD 6: Supports clean water and sanitation protection.\n"
            "- ODD 11: Contributes to sustainable urban infrastructure.\n"
            "- ODD 15: Helps prevent soil and terrestrial ecosystem contamination.";
    }
    else if (status == "Moderate")
    {
        statusColor = "#F9A825";
        meaning = "Monitoring required. Environmental surveillance recommended.";

        sdgIndication =
            "SDG indication:\n"
            "- ODD 6: Preventive monitoring is needed to protect sanitation quality.\n"
            "- ODD 11: Supports sustainable city management through early surveillance.\n"
            "- ODD 13: Improves preparedness against climate-related risks.";
    }
    else if (status == "Poor")
    {
        statusColor = "#EF6C00";
        meaning = "Possible environmental impact. Maintenance should be planned.";

        sdgIndication =
            "SDG warning:\n"
            "- ODD 6: Possible risk to clean water and sanitation services.\n"
            "- ODD 11: Urban infrastructure requires maintenance.\n"
            "- ODD 14: Possible pollution risk for aquatic ecosystems.\n"
            "- ODD 15: Possible contamination risk for land ecosystems.";
    }
    else if (status == "Critical")
    {
        statusColor = "#C62828";
        meaning = "Ecological danger detected. Urgent intervention required.";

        sdgIndication =
            "SDG critical alert:\n"
            "- ODD 6: High risk of sanitation failure and water contamination.\n"
            "- ODD 11: Urgent action required for urban environmental safety.\n"
            "- ODD 13: Emergency response improves climate-risk resilience.\n"
            "- ODD 14: Strong risk for aquatic ecosystems.\n"
            "- ODD 15: Strong risk for terrestrial ecosystems.";
    }
    else
    {
        statusColor = "#37474F";
        meaning = "Unknown environmental status. Manual verification required.";
        sdgIndication = "Manual inspection is required.";
    }

    QString confidenceText;

    if (confidence >= 90.0)
        confidenceText = "High confidence prediction.";
    else if (confidence >= 70.0)
        confidenceText = "Acceptable confidence prediction.";
    else
        confidenceText = "Low confidence. Manual verification recommended.";

    QDialog dialog(this);
    dialog.setWindowTitle("AI Environmental Sustainability Predictor");
    dialog.setModal(true);
    dialog.setMinimumSize(650, 430);

    QHBoxLayout *mainLayout = new QHBoxLayout(&dialog);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    QLabel *imageLabel = new QLabel(&dialog);
    imageLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    imageLabel->setFixedWidth(150);

    QPixmap sdgPixmap(":/images/SDG.png");

    if (!sdgPixmap.isNull())
    {
        imageLabel->setPixmap(
            sdgPixmap.scaled(140, 140, Qt::KeepAspectRatio, Qt::SmoothTransformation)
        );
    }
    else
    {
        imageLabel->setText("SDG");
        imageLabel->setStyleSheet(
            "font-size: 24px;"
            "font-weight: bold;"
            "color: #2E7D32;"
            "border: 2px solid #2E7D32;"
            "border-radius: 10px;"
            "padding: 25px;"
        );
    }

    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->setSpacing(12);

    QLabel *titleLabel = new QLabel("AI Environmental Sustainability Predictor", &dialog);
    titleLabel->setStyleSheet(
        "font-size: 18px;"
        "font-weight: bold;"
        "color: #2C4056;"
    );
    rightLayout->addWidget(titleLabel);

    QLabel *predictionLabel = new QLabel(&dialog);
    predictionLabel->setTextFormat(Qt::RichText);
    predictionLabel->setText(
        QString(
            "<div style='font-size:14px;'>"
            "<b>Sewer ID:</b> %1<br><br>"
            "<b>Prediction:</b> "
            "<span style='color:%2; font-weight:bold; font-size:18px;'>%3</span><br>"
            "<b>Confidence:</b> %4%<br>"
            "<b>Reliability:</b> %5"
            "</div>"
        ).arg(QString::number(sewerId),
              statusColor,
              status,
              QString::number(confidence, 'f', 2),
              confidenceText)
    );
    rightLayout->addWidget(predictionLabel);

    QFrame *line = new QFrame(&dialog);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    rightLayout->addWidget(line);

    QLabel *meaningLabel = new QLabel(&dialog);
    meaningLabel->setWordWrap(true);
    meaningLabel->setTextFormat(Qt::RichText);
    meaningLabel->setText(
        "<div style='font-size:14px;'>"
        "<b>Environmental interpretation:</b><br>" +
        meaning +
        "</div>"
    );
    rightLayout->addWidget(meaningLabel);

    QScrollArea *scrollArea = new QScrollArea(&dialog);
    scrollArea->setWidgetResizable(true);
    scrollArea->setMinimumHeight(130);
    scrollArea->setMaximumHeight(170);

    QLabel *sdgLabel = new QLabel(&dialog);
    sdgLabel->setWordWrap(true);
    sdgLabel->setTextFormat(Qt::RichText);

    QString sdgHtml = sdgIndication;
    sdgHtml.replace("\n", "<br>");
    sdgHtml.replace("- ", "• ");

    sdgLabel->setText(
        "<div style='font-size:13px; color:#263238;'>"
        "<b>ODD / SDG indication:</b><br>" +
        sdgHtml +
        "</div>"
    );

    sdgLabel->setStyleSheet(
        "background-color: #F4F8F4;"
        "border: 1px solid #C8DCC8;"
        "border-radius: 8px;"
        "padding: 10px;"
    );

    scrollArea->setWidget(sdgLabel);
    rightLayout->addWidget(scrollArea);

    QPushButton *okButton = new QPushButton("OK", &dialog);
    okButton->setFixedWidth(100);
    okButton->setStyleSheet(
        "QPushButton {"
        "background-color: #2C4056;"
        "color: white;"
        "border-radius: 6px;"
        "padding: 7px 18px;"
        "font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "background-color: #3E5A78;"
        "}"
    );

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    rightLayout->addLayout(buttonLayout);

    QObject::connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);

    mainLayout->addWidget(imageLabel);
    mainLayout->addLayout(rightLayout);

    dialog.exec();
}

//auto location
void GWasteCollection::on_pushButton_cancel_map_s1_clicked()
{
    // Revenir au formulaire sans rien faire
    ui->stackedWidget_form_s1->setCurrentIndex(0);
    s_pickedLat = 0.0;
    s_pickedLng = 0.0;
}
void GWasteCollection::on_pushButton_autoloc_s1_clicked()
{
    // Charger le QML dans le quickWidget
    ui->quickWidget_loc_s1->setSource(QUrl("qrc:/fichier/map_loc.qml"));
    ui->quickWidget_loc_s1->show();

    // Récupérer l'objet root du QML
    QQuickItem* root = ui->quickWidget_loc_s1->rootObject();
    if (!root) return;

    // Centrer sur la région saisie via géocodage simple
    // (coordonnées approximatives des régions tunisiennes)
    int muniindex = ui->comboBox_municialityname_s1->currentIndex();

    if(muniindex < 0)
    {
        QMessageBox::warning(this,
                             "Attention",
                             "Please select a municipality before opening the map.");
        return;
    }

    QString municipalityName =
            ui->comboBox_municialityname_s1->currentText();

    if(!Mtmp.getMunicipalityName(municipalityName))
    {
        QMessageBox::critical(this,
                              "Error",
                              "Municipality not found.");
        return;
    }

    double lat = Mtmp.getLatitude().toDouble();
    double lng = Mtmp.getLongitude().toDouble();

    double maxDelta = Mtmp.getRadius() / 111000.0;

    root->setProperty("markerLat",  lat);
    root->setProperty("markerLng",  lng);
    root->setProperty("centerLat",  lat);
    root->setProperty("centerLng",  lng);
    root->setProperty("maxDelta",   maxDelta);

    // Connecter le signal QML → Qt
    connect(root, SIGNAL(locationPicked(double, double)),
            this, SLOT(onLocationPicked_s(double, double)));

    // Aller à la page map (index 1)
    ui->stackedWidget_form_s1->setCurrentIndex(1);
}


void GWasteCollection::onLocationPicked_s(double lat, double lng)
{
    s_pickedLat = lat;
    s_pickedLng = lng;
}

void GWasteCollection::on_pushButton_confirm_map_s1_clicked()
{
    if (s_pickedLat == 0.0 && s_pickedLng == 0.0) {
        QMessageBox::warning(this, "Attention", "  Please click on the map to choose a location .");
        return;
    }

    // Remplir les champs du formulaire
    ui->lineEdit_latitude_m1->setText(QString::number(s_pickedLat, 'f', 6));
    ui->lineEdit_longitude_m1->setText(QString::number(s_pickedLng, 'f', 6));
    ui->lineEdit_Location_S1->setText(QString::number(s_pickedLat, 'f', 6) + "," +QString::number(s_pickedLng, 'f', 6));
    // Revenir au formulaire
    ui->stackedWidget_form_s1->setCurrentIndex(0);
}
//-----------------------------------------------------------------------------------------------------------------
//----------------------------------------------CollectionTeam Module---------------------------------------------
//---------------------------------------------------------------------------------------------------------------

void GWasteCollection::clearCollectionTeam()
{
    ui->spinBox_teamsize_c1->setValue(0);
    ui->lineEdit_vehicleplate_c1->clear();
    ui->lineEdit_contactnumber_c1->clear();
    ui->lineEdit_price_c1->clear();
    ui->radioButton_available_c1->setChecked(false);
    ui->radioButton_unavailable_c1->setChecked(false);
    ui->lineEdit_barrederecherche_c1->clear();
    ui->tableView_listteams_c1->clearSelection();
}

void GWasteCollection::on_pushButton_employeem_c1_clicked()
{
    clearCollectionTeam();
    ui->tableView_list_e4->setModel(Etmp.displayEmployees());
    displayEmployeeStatistics();
    ui->SWWasteCollection->setCurrentIndex(0);
    ui->stackedWidget_e->setCurrentIndex(3);
}

void GWasteCollection::on_pushButton_municipalitiesm_c1_clicked()
{
    clearCollectionTeam();
    ui->tableView_listemunicipality_m1->setModel(Mtmp.displayMunicipality());
    displayMunicipalityStatistics();
    ui->SWWasteCollection->setCurrentIndex(1);
    ui->stackedWidget_m1->setCurrentIndex(0);
    ui->stackedWidget_form->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_sewersm_c1_clicked()
{
    clearCollectionTeam();
    ui->tableView_S1->setModel(Stemp.displaySewer());
    ui->SWWasteCollection->setCurrentIndex(2);
    ui->stackedWidgetS->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_rcentersm_c1_clicked()
{
    clearCollectionTeam();
    ui->tableView_r1->setModel(Rtmp.displayCenter());
    ui->SWWasteCollection->setCurrentIndex(4);
    ui->stackedWidget_r->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_logout_c1_clicked()
{
    logout();
}

void GWasteCollection::on_pushButton_viewteamhealth_c1_clicked()
{
    clearCollectionTeam();
    ui->stackedWidget_c->setCurrentIndex(2);
    ui->tableView_loadscore_c2->setModel(Ctmp.displayTeamsToAffect());
    ui->tableView_intervention_c2->setModel(Ctmp.displayIntervenesForAffectC());
}

void GWasteCollection::on_pushButton_viewteamranking_c1_clicked()
{
    clearCollectionTeam();
    ui->tableView_rankteams_c3->setModel(Ctmp.rankTeams());
    ui->stackedWidget_c->setCurrentIndex(1);
}

void GWasteCollection::on_pushButton_employeem_c2_clicked()
{
    ui->tableView_list_e4->setModel(Etmp.displayEmployees());
    displayEmployeeStatistics();
    ui->SWWasteCollection->setCurrentIndex(0);
    ui->stackedWidget_e->setCurrentIndex(3);
}

void GWasteCollection::on_pushButton_municipalitiesm_c2_clicked()
{
    ui->tableView_listemunicipality_m1->setModel(Mtmp.displayMunicipality());
    displayMunicipalityStatistics();
    ui->SWWasteCollection->setCurrentIndex(1);
    ui->stackedWidget_m1->setCurrentIndex(0);
    ui->stackedWidget_form->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_sewersm_c2_clicked()
{
    ui->tableView_S1->setModel(Stemp.displaySewer());
    ui->SWWasteCollection->setCurrentIndex(2);
    ui->stackedWidgetS->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_rcentersm_c2_clicked()
{
    ui->tableView_r1->setModel(Rtmp.displayCenter());
    ui->SWWasteCollection->setCurrentIndex(4);
    ui->stackedWidget_r->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_logout_c2_clicked()
{
    logout();
}

void GWasteCollection::on_pushButton_employeem_c3_clicked()
{
    ui->tableView_list_e4->setModel(Etmp.displayEmployees());
    displayEmployeeStatistics();
    ui->SWWasteCollection->setCurrentIndex(0);
    ui->stackedWidget_e->setCurrentIndex(3);
}

void GWasteCollection::on_pushButton_municipalitiesm_c3_clicked()
{
    ui->tableView_listemunicipality_m1->setModel(Mtmp.displayMunicipality());
    displayMunicipalityStatistics();
    ui->SWWasteCollection->setCurrentIndex(1);
    ui->stackedWidget_m1->setCurrentIndex(0);
    ui->stackedWidget_form->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_sewersm_c3_clicked()
{
    ui->tableView_S1->setModel(Stemp.displaySewer());
    ui->SWWasteCollection->setCurrentIndex(2);
    ui->stackedWidgetS->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_rcentersm_c3_clicked()
{
    ui->tableView_r1->setModel(Rtmp.displayCenter());
    ui->SWWasteCollection->setCurrentIndex(4);
    ui->stackedWidget_r->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_logout_c3_clicked()
{
    logout();
}

void GWasteCollection::on_pushButton_back_c2_clicked()
{
    ui->tableView_listteams_c1->setModel(Ctmp.DisplayTeams());
    ui->stackedWidget_c->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_back_c3_clicked()
{
    ui->tableView_listteams_c1->setModel(Ctmp.DisplayTeams());
    ui->stackedWidget_c->setCurrentIndex(0);
}

//CRUD COLLECTIONTEAM

void GWasteCollection::on_pushButton_confirm_c1_clicked()
{
    //teamsize
    int teamSize = ui->spinBox_teamsize_c1->value();
    if (teamSize <= 0) {
        QMessageBox::warning(this, "Error," ,"The team size must be greater than 0.");
        return;
    }
    //vehicleplate
    QString vehiclePlate=ui->lineEdit_vehicleplate_c1->text();
    QRegularExpression plateRegex("^\\d{1,3}\\s?TUN\\s?\\d{1,4}$");
    if (!plateRegex.match(vehiclePlate).hasMatch()) {
        QMessageBox::warning(this, "Error", "Invalid license plate format (ex: 123 TUN 4567).");
        return;
    }
    //contactnumber
    QString contactNumber = ui->lineEdit_contactnumber_c1->text();
    QRegularExpression phoneRegex("^[24579]\\d{7}$"); //Expression Régulière (un filtre)
    if (!phoneRegex.match(contactNumber).hasMatch()) {
        QMessageBox::warning(this, "Erreur", "Le numéro de contact doit contenir 8 chiffres tunisiens valides.");
        return;
    }
    //status
    QString status;
    if (ui->radioButton_available_c1->isChecked()) {
        status = "Available";
    }
    else if (ui->radioButton_unavailable_c1->isChecked()) {
        status = "Unavailable";
    }
    if (status.isEmpty()) {
        QMessageBox::warning(this, "Error","Please select a status.");
        return;
    }
    //price
    bool ok;
    int price=ui->lineEdit_price_c1->text().toInt(&ok);
    if (!ok) {
        QMessageBox::warning(this, "Error", "The price must be a valid number.");
        return;
    }
    if (price <= 0) {
        QMessageBox::warning(this, "Unvalid Value", "The price must be strictly greater than 0 DT.");
        return;
    }

    // 1. Récupération des données du formulaire
    int size = ui->spinBox_teamsize_c1->value();
    QString plate = ui->lineEdit_vehicleplate_c1->text();
    QString contact = ui->lineEdit_contactnumber_c1->text();
    QString statuse = "";
    if (ui->radioButton_available_c1->isChecked()) {
        status = "Available";
    } else if (ui->radioButton_unavailable_c1->isChecked()) {
        status = "Unavailable";
    }

    bool success = false;

    // 2. Logique de modification (Update) ou d'ajout (Add)
    if (ui->tableView_listteams_c1->selectionModel()->hasSelection()) {
        // Mode Modification
        QModelIndex index = ui->tableView_listteams_c1->selectionModel()->currentIndex();
        int id = index.sibling(index.row(), 0).data().toInt(); // Récupère l'ID caché ou visible

        CollectionTeam C(id, size, plate, contact, status, price, 0, 0, 0, 0);
        success = C.updateTeam(); // Assurez-vous d'avoir implémenté updateTeam dans collectionteam.cpp
    } else {
        // Mode Ajout
        CollectionTeam C(0, size, plate, contact, status, price, 0, 0, 0, 0);
        success = C.addTeam();
    }
    // 3. Mise à jour de l'affichage
    if (success) {
        ui->tableView_listteams_c1->setModel(Ctmp.DisplayTeams());
        ui->tableView_loadscore_c2->setModel(Ctmp.displayTeamsToAffect());
            ui->tableView_listteams_c1->clearSelection();
            ui->spinBox_teamsize_c1->setValue(0);
            ui->lineEdit_vehicleplate_c1->clear();
            ui->lineEdit_contactnumber_c1->clear();
            ui->lineEdit_price_c1->clear();
            ui->radioButton_available_c1->setAutoExclusive(false);
            ui->radioButton_available_c1->setChecked(false);
            ui->radioButton_unavailable_c1->setChecked(false);
            ui->radioButton_available_c1->setAutoExclusive(true);
        setupChartCollectionTeam();
        QMessageBox::information(this, "Success", "Operation successful.");
        // Optionnel : vider les champs après succès
        on_pushButton_restore_c1_clicked();
    } else {
        QMessageBox::critical(this, "Error", "Operation Failed.");
    }
}

void GWasteCollection::on_tableView_listteams_c1_doubleClicked(const QModelIndex &index)
{
    int teamID = index.sibling(index.row(), 0).data().toInt();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(
        this,
        "Confirmation",
        "Do you want to delete this team ?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        if (Ctmp.deleteTeam(teamID)) {
            ui->tableView_listteams_c1->setModel(Ctmp.DisplayTeams());
            ui->tableView_loadscore_c2->setModel(Ctmp.displayTeamsToAffect());
            setupChartCollectionTeam();
        }
    }
}

void GWasteCollection::on_tableView_listteams_c1_clicked(const QModelIndex &index)
{
    int teamSize = index.sibling(index.row(), 1).data().toInt();

    ui->spinBox_teamsize_c1->setValue(teamSize);
    ui->lineEdit_vehicleplate_c1->setText(index.sibling(index.row(), 2).data().toString());
    ui->lineEdit_contactnumber_c1->setText(index.sibling(index.row(), 3).data().toString());
    QString status = index.sibling(index.row(), 4).data().toString();
    if (status == "Available") {
        ui->radioButton_available_c1->setChecked(true);
    } else {
        ui->radioButton_unavailable_c1->setChecked(true);
    }
    ui->lineEdit_price_c1->setText(index.sibling(index.row(), 5).data().toString());
}

void GWasteCollection::on_pushButton_restore_c1_clicked()
{
    if (ui->tableView_listteams_c1->selectionModel()->hasSelection())
    {
        ui->tableView_listteams_c1->clearSelection();
        ui->spinBox_teamsize_c1->setValue(0);
        ui->lineEdit_vehicleplate_c1->clear();
        ui->lineEdit_contactnumber_c1->clear();
        ui->lineEdit_price_c1->clear();
        ui->radioButton_available_c1->setAutoExclusive(false);
        ui->radioButton_available_c1->setChecked(false);
        ui->radioButton_unavailable_c1->setChecked(false);
        ui->radioButton_available_c1->setAutoExclusive(true);
    }
}


//Basic Features

void GWasteCollection::on_pushButton_tristatus_c1_clicked()
{
    QSqlQueryModel * sortedModel = Ctmp.sortByStatus();

    if(sortedModel) {
        ui->tableView_listteams_c1->setModel(sortedModel);
    } else {
        QMessageBox::critical(this, "Error", "Failed to sort the list.");
    }
}

void GWasteCollection::on_lineEdit_barrederecherche_c1_textChanged(const QString &arg1)
{
    ui->tableView_listteams_c1->setModel(Ctmp.searchTeamByID(arg1));
}

void GWasteCollection::on_pushButton_export_c1_clicked()
{
    QString filePath = QFileDialog::getSaveFileName(this,"Export Collection Teams to PDF",QDir::homePath() + "/collectionteams_report.pdf","PDF Files (*.pdf)");

    if (filePath.isEmpty()) return;

    QAbstractItemModel *model = ui->tableView_listteams_c1->model();
    if (!model) return;

    QPdfWriter pdfWriter(filePath);
    pdfWriter.setPageSize(QPageSize(QPageSize::A4));
    pdfWriter.setPageOrientation(QPageLayout::Landscape);
    pdfWriter.setPageMargins(QMarginsF(10, 10, 10, 10), QPageLayout::Millimeter);
    pdfWriter.setResolution(96);

    QPainter painter(&pdfWriter);
    if (!painter.isActive()) return;

    int pageWidth = painter.viewport().width();
    int pageHeight = painter.viewport().height();
    int y = 60;

    // --- Titre ---
    painter.setFont(QFont("Arial", 16, QFont::Bold));
    painter.drawText(QRect(0, y, pageWidth, 50), Qt::AlignCenter, "Collection Team Report");
    y += 60;

    // --- Date de génération ---
    painter.setFont(QFont("Arial", 9));
    painter.setPen(Qt::gray);
    painter.drawText(QRect(0, y, pageWidth, 30), Qt::AlignCenter,"Generated on: " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    painter.setPen(Qt::black);
    y += 50;

    // --- Largeurs personnalisées par colonne ---
    // Team ID |    team Size | Vehicle Plate | Contact Number | Status  | Price
    QList<int> colWidths = {60, 60, 180, 180, 150, 100};

    // Vérification que la somme des colonnes ne dépasse pas pageWidth
    int totalWidth = 0;
    for (int w : colWidths) totalWidth += w;

    // Si la somme dépasse, on redimensionne proportionnellement
    if (totalWidth != pageWidth) {
        float ratio = (float)pageWidth / totalWidth;
        for (int i = 0; i < colWidths.size(); i++)
            colWidths[i] = (int)(colWidths[i] * ratio);
    }

    int colCount = model->columnCount();

    // --- En-têtes ---
    painter.setFont(QFont("Arial", 9, QFont::Bold));
    painter.setBrush(QColor(52, 152, 219));
    painter.setPen(Qt::NoPen);
    painter.drawRect(0, y, pageWidth, 35);

    painter.setPen(Qt::white);
    int x = 0;
    for (int col = 0; col < colCount && col < colWidths.size(); col++) {
        QString headerText = model->headerData(col, Qt::Horizontal).toString();
        painter.drawText(QRect(x + 5, y, colWidths[col] - 5, 35),
                         Qt::AlignVCenter | Qt::AlignCenter | Qt::TextWordWrap, headerText);
        x += colWidths[col];
    }
    y += 35;

    // --- Données avec hauteur dynamique ---
    painter.setFont(QFont("Arial", 8));
    bool alternate = false;

    for (int row = 0; row < model->rowCount(); row++) {

        // Calculer la hauteur max de la ligne
        int rowHeight = 30;
        for (int col = 0; col < colCount && col < colWidths.size(); col++) {
            QString cellText = model->data(model->index(row, col)).toString();
            QRect boundingRect = painter.boundingRect(
                QRect(0, 0, colWidths[col] - 10, 1000),
                Qt::AlignTop | Qt::AlignLeft | Qt::TextWordWrap,
                cellText
                );
            rowHeight = qMax(rowHeight, boundingRect.height() + 10);
        }

        // Nouvelle page si nécessaire
        if (y + rowHeight > pageHeight - 40) {
            pdfWriter.newPage();
            y = 40;
        }

        // Fond alterné
        painter.setPen(Qt::NoPen);
        painter.setBrush(alternate ? QColor(236, 240, 241) : Qt::white);
        painter.drawRect(0, y, pageWidth, rowHeight);
        alternate = !alternate;

        // Texte avec wrap
        painter.setPen(Qt::black);
        x = 0;
        for (int col = 0; col < colCount && col < colWidths.size(); col++) {
            QString cellText = model->data(model->index(row, col)).toString();
            QRect cellRect(x + 5, y + 5, colWidths[col] - 10, rowHeight);
            painter.drawText(cellRect, Qt::AlignTop | Qt::AlignHCenter | Qt::TextWordWrap, cellText);
            x += colWidths[col];
        }

        // Ligne séparatrice
        painter.setPen(QColor(189, 195, 199));
        painter.drawLine(0, y + rowHeight, pageWidth, y + rowHeight);
        y += rowHeight;
    }

    painter.end();
    QMessageBox::information(this, "Success", "PDF exported successfully!");
}

void GWasteCollection::setupChartCollectionTeam()
{
    // --- Supprimer l'ancien layout si existant ---
    if (ui->label_imgstat_c1->layout()) {
        QLayoutItem *item;
        while ((item = ui->label_imgstat_c1->layout()->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        delete ui->label_imgstat_c1->layout();
    }

    // --- 2. Calcul des statistiques de statut ---
    int available = 0, unavailable = 0;
    QSqlQueryModel *model = Ctmp.DisplayTeams();
    int rowCount = model->rowCount();

    for (int i = 0; i < rowCount; ++i) {
        // Le statut est à l'index 4 (TEAM_ID=0, SIZE=1, PLATE=2, CONTACT=3, STATUS=4)
        QString statusValue = model->data(model->index(i, 4)).toString();

        if (statusValue.toLower() == "available")
            available++;
        else if (statusValue.toLower() == "unavailable")
            unavailable++;
    }

    // --- 3. Création du graphique ---
    QPieSeries *series = new QPieSeries();

    if (available > 0) series->append("Available", available);
    if (unavailable > 0) series->append("Unavailable", unavailable);

    // --- 4. Couleurs, labels et style ---
    for (int i = 0; i < series->count(); i++) {
        QPieSlice *slice = series->slices().at(i);
        slice->setLabelVisible(true);
        slice->setLabelFont(QFont("Arial", 10, QFont::Bold));

        // Calcul du pourcentage pour le label
        double percentage = (rowCount > 0) ? (slice->value() / rowCount * 100.0) : 0;
        slice->setLabel(QString("%1 (%2%)").arg(slice->label()).arg(int(percentage)));

        if (slice->label().startsWith("Available")) {
            slice->setBrush(QColor(41, 128, 185));
        } else {
            slice->setBrush(QColor(52, 73, 94));
            slice->setExploded(true);            // Fait ressortir le segment indisponible
        }
    }

    // --- 5. Configuration du Chart ---
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Teams Availability Statistics");
    chart->legend()->hide();
    chart->setBackgroundVisible(false);
    chart->setMargins(QMargins(0,0,0,0));

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setStyleSheet("background: transparent;");

    // --- 6. Application au layout ---
    QVBoxLayout *layout = new QVBoxLayout(ui->label_imgstat_c1);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(chartView);
    ui->label_imgstat_c1->setLayout(layout);
    ui->label_imgstat_c1->update();

}

//Advanced Features

void GWasteCollection::on_pushButton_affectteam_c2_clicked()
{
    QItemSelectionModel *selectIntervention = ui->tableView_intervention_c2->selectionModel();
    QItemSelectionModel *selectTeam = ui->tableView_loadscore_c2->selectionModel();

    if (!selectIntervention->hasSelection() || !selectTeam->hasSelection()) {
        QMessageBox::warning(this, "Selection Required",
                             "Please select an intervention and a team.");
        return;
    }

    // 3. Extract data from the selected Intervention row
    // Column 1 is "Intervention Date", Column 5 is "Municipality_id" (Sewer identifier)
    int interRow = selectIntervention->currentIndex().row();
    QString dateStr = ui->tableView_intervention_c2->model()->index(interRow, 1).data().toString();
    QString region = ui->tableView_intervention_c2->model()->index(interRow, 0).data().toString();

    // 4. Extract Team ID from the selected Team row (Column 0 is "Team ID")
    int teamRow = selectTeam->currentIndex().row();
    int teamID = ui->tableView_loadscore_c2->model()->index(teamRow, 0).data().toInt();

    // 5. Convert date string to QDate object to avoid Oracle format errors
    QDate interDate = QDate::fromString(dateStr, "dd/MM/yyyy");
    if (!interDate.isValid()) {
        interDate = QDate::fromString(dateStr, Qt::ISODate); // Fallback for standard formats
    }
    // 6. Call the update function
    if (Ctmp.affectTeamToIntervention(teamID, region, interDate)) {
        QMessageBox::information(this, "Success", "Team " + QString::number(teamID) + " has been assigned.");

        // 7. Refresh the tables to show the update (the assigned row should disappear from intervention table)
        ui->tableView_intervention_c2->setModel(Ctmp.displayIntervenesForAffectC());
        ui->tableView_loadscore_c2->setModel(Ctmp.displayTeamsToAffect());
    } else {
        QMessageBox::critical(this, "Database Error", "Could not assign the team to the intervention.");
    }
}

void GWasteCollection::on_tableView_rankteams_c3_doubleClicked(const QModelIndex &index)
{
    // 1. Get the Team ID from the first column of the selected row
    int teamID  = index.sibling(index.row(), 0).data().toInt();
    // 2. Ask the user for a rating between 1 and 5
    bool ok;
    int rating = QInputDialog::getInt(this, tr("Rate Team"),
                                      tr("Enter rating (1-5):"), 5, 1, 5, 1, &ok);
    if (ok) {
        // 3. Call the update function from your CollectionTeam object (Ctmp)
        if (Ctmp.updateTeamRating(teamID, rating)) {
            QMessageBox::information(this, "Success", "Rating updated successfully!");
            // 4. Refresh the ranking table to show the new score
            ui->tableView_rankteams_c3->setModel(Ctmp.rankTeams());
        } else {
            QMessageBox::critical(this, "Error", "Failed to update rating.");
        }
    }
}


void GWasteCollection::checkAuditRequirement(int teamID, int lastRating) {
    if (lastRating <= 2) {
        QMessageBox::warning(this, "Quality Audit Triggered",
                             QString("Team %1 has received a low rating (%2). An audit has been flagged.").arg(teamID).arg(lastRating));
        // Here you could insert a record into an 'AUDITS' table if you have one
    }
}

//--------------------------------------------------------------------------------
//----------------------------RECYCLING MODULE------------------------------------
//--------------------------------------------------------------------------------

void GWasteCollection::on_pushButton_saturation_r1_clicked()
{
    if(ui->tableView_r1->selectionModel()->hasSelection()){
        resetform_r1();
        ui->tableView_r1->clearSelection();
        ui->tableView_r1->selectionModel()->clear();
    }
    saturationsStats_r();
    ui->stackedWidget_r->setCurrentIndex(1);
}

void GWasteCollection::on_pushButton_emails_r1_clicked()
{
    if(ui->tableView_r1->selectionModel()->hasSelection()){
        resetform_r1();
        ui->tableView_r1->clearSelection();
        ui->tableView_r1->selectionModel()->clear();
    }
    fixtabaffect_r();
    ui->stackedWidget_r->setCurrentIndex(2);
}

void GWasteCollection::on_pushButton_esettings_r3_clicked()
{
    initMailSettings_r();
    ui->stackedWidget_r->setCurrentIndex(3);
}

void GWasteCollection::on_pushButton_affectation_r4_clicked()
{
    ui->stackedWidget_r->setCurrentIndex(2);
}

void GWasteCollection::on_pushButton_centerm_r2_clicked()
{
    ui->tableView_r1->setModel(Rtmp.displayCenter());
    ui->stackedWidget_r->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_centerm_r3_clicked()
{
    ui->tableView_r1->setModel(Rtmp.displayCenter());
    ui->stackedWidget_r->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_employeem_r1_clicked()
{
    if(ui->tableView_r1->selectionModel()->hasSelection()){
        resetform_r1();
        ui->tableView_r1->clearSelection();
        ui->tableView_r1->selectionModel()->clear();
    }
    ui->tableView_list_e4->setModel(Etmp.displayEmployees());
    displayEmployeeStatistics();
    ui->SWWasteCollection->setCurrentIndex(0);
    ui->stackedWidget_e->setCurrentIndex(3);
}

void GWasteCollection::on_pushButton_municipalitiesm_r1_clicked()
{
    if(ui->tableView_r1->selectionModel()->hasSelection()){
        resetform_r1();
        ui->tableView_r1->clearSelection();
        ui->tableView_r1->selectionModel()->clear();
    }
    ui->tableView_listemunicipality_m1->setModel(Mtmp.displayMunicipality());
    displayMunicipalityStatistics();
    ui->SWWasteCollection->setCurrentIndex(1);
    ui->stackedWidget_m1->setCurrentIndex(0);
    ui->stackedWidget_form->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_sewersm_r1_clicked()
{
    if(ui->tableView_r1->selectionModel()->hasSelection()){
        resetform_r1();
        ui->tableView_r1->clearSelection();
        ui->tableView_r1->selectionModel()->clear();
    }
    ui->tableView_S1->setModel(Stemp.displaySewer());
    ui->SWWasteCollection->setCurrentIndex(2);
    ui->stackedWidgetS->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_logout_r1_clicked()
{
    logout();
}

void GWasteCollection::on_pushButton_employeem_r2_clicked()
{
    ui->tableView_list_e4->setModel(Etmp.displayEmployees());
    displayEmployeeStatistics();
    ui->SWWasteCollection->setCurrentIndex(0);
    ui->stackedWidget_e->setCurrentIndex(3);
}

void GWasteCollection::on_pushButton_municipalitiesm_r2_clicked()
{
    ui->tableView_listemunicipality_m1->setModel(Mtmp.displayMunicipality());
    displayMunicipalityStatistics();
    ui->SWWasteCollection->setCurrentIndex(1);
    ui->stackedWidget_m1->setCurrentIndex(0);
    ui->stackedWidget_form->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_sewersm_r2_clicked()
{
    ui->tableView_S1->setModel(Stemp.displaySewer());
    ui->SWWasteCollection->setCurrentIndex(2);
    ui->stackedWidgetS->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_logout_r2_clicked()
{
    logout();
}

void GWasteCollection::on_pushButton_employeem_r3_clicked()
{
    ui->tableView_list_e4->setModel(Etmp.displayEmployees());
    displayEmployeeStatistics();
    ui->SWWasteCollection->setCurrentIndex(0);
    ui->stackedWidget_e->setCurrentIndex(3);
}

void GWasteCollection::on_pushButton_municipalitiesm_r3_clicked()
{
    ui->tableView_listemunicipality_m1->setModel(Mtmp.displayMunicipality());
    displayMunicipalityStatistics();
    ui->SWWasteCollection->setCurrentIndex(1);
    ui->stackedWidget_m1->setCurrentIndex(0);
    ui->stackedWidget_form->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_sewersm_r3_clicked()
{
    ui->tableView_S1->setModel(Stemp.displaySewer());
    ui->SWWasteCollection->setCurrentIndex(2);
    ui->stackedWidgetS->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_logout_r3_clicked()
{
    logout();
}

void GWasteCollection::on_pushButton_employeem_r4_clicked()
{
    ui->tableView_list_e4->setModel(Etmp.displayEmployees());
    displayEmployeeStatistics();
    ui->SWWasteCollection->setCurrentIndex(0);
    ui->stackedWidget_e->setCurrentIndex(3);
}

void GWasteCollection::on_pushButton_municipalitiesm_r4_clicked()
{
    ui->tableView_listemunicipality_m1->setModel(Mtmp.displayMunicipality());
    displayMunicipalityStatistics();
    ui->SWWasteCollection->setCurrentIndex(1);
    ui->stackedWidget_m1->setCurrentIndex(0);
    ui->stackedWidget_form->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_sewersm_r4_clicked()
{
    ui->tableView_S1->setModel(Stemp.displaySewer());
    ui->SWWasteCollection->setCurrentIndex(2);
    ui->stackedWidgetS->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_logout_r4_clicked()
{
    logout();
}
void GWasteCollection::on_pushButton_collectiontm_r2_clicked()
{
    ui->tableView_listteams_c1->setModel(Ctmp.DisplayTeams());
    ui->SWWasteCollection->setCurrentIndex(3);
    ui->stackedWidget_c->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_collectiontm_r3_clicked()
{
    ui->tableView_listteams_c1->setModel(Ctmp.DisplayTeams());
    ui->SWWasteCollection->setCurrentIndex(3);
    ui->stackedWidget_c->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_collectiontm_r1_clicked()
{
    if(ui->tableView_r1->selectionModel()->hasSelection()){
        resetform_r1();
        ui->tableView_r1->clearSelection();
        ui->tableView_r1->selectionModel()->clear();
    }
    ui->tableView_listteams_c1->setModel(Ctmp.DisplayTeams());
    ui->SWWasteCollection->setCurrentIndex(3);
    ui->stackedWidget_c->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_collectiontm_r4_clicked()
{
    ui->tableView_listteams_c1->setModel(Ctmp.DisplayTeams());
    ui->SWWasteCollection->setCurrentIndex(3);
    ui->stackedWidget_c->setCurrentIndex(0);
}

void GWasteCollection::on_pushButton_confirm_r1_clicked()
{
    int t=0;
    bool n1,n2;
    QString name=ui->lineEdit_name_r1->text();
    QString address=ui->lineEdit_adress_r1->text();
    QString email=ui->lineEdit_email_r1->text();
    int maxCapacity=ui->lineEdit_mawc_r1->text().toInt(&n2);
    int currentCapacity=ui->lineEdit_capacity_r1->text().toInt(&n1);
    QString status;
    int row = ui->comboBox_municialityname_r1->currentIndex();
    int municipalityID =ui->comboBox_municialityname_r1->model()->index(row, 0).data().toInt();
    QModelIndex indext = ui->tableView_r1->selectionModel()->currentIndex();

    //name
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter a center name.");
        ui->lineEdit_name_r1->setFocus();
        t=1;
    }else if((name.length() > 100 || name.length() < 3) && t==0){
        QMessageBox::warning(this, "Error", "Please enter a valid center name.\n(min 3 caracters and max 100 caracters)");
        ui->lineEdit_name_r1->setFocus();
        t=1;
    }
    //address
    if (address.isEmpty() && t==0) {
        QMessageBox::warning(this, "Error", "Please enter a center address.");
        ui->lineEdit_adress_r1->setFocus();
        t=1;
    }else if((address.length() > 200 || address.length() < 3) && t==0){
        QMessageBox::warning(this, "Error", "Please enter a valid center address.\n(min 3 caracters and max 200 caracters)");
        ui->lineEdit_name_r1->setFocus();
        t=1;
    }//mail
    if(t==0){
        if ((email.length() < 6) || email.length() > 254)
            t=1;
        else
        {
            int atPos=-1;
            int dotPos=-1;
            for (int i=0; i<email.length(); i++)
            {
                if (email[i] == '@')
                    atPos=i;
                else if (email[i] == '.')
                    dotPos=i;
            }
            if ((atPos > (dotPos - 2)) || (atPos < 1) || (dotPos > (email.length() - 3)))
                t=1;
        }
        if (t==1)
        {
            QMessageBox::critical(nullptr, QObject::tr("Invalid email"),
                                  QObject::tr("The email must be in a valid format.\n"
                                              "Click Cancel to exit."), QMessageBox::Cancel);
            ui->lineEdit_email_r1->setFocus();
        }
        else if (Rtmp.getCenterByEmail(email) && (!(ui->tableView_r1->selectionModel()->hasSelection()) || (email != indext.sibling(indext.row(), 3).data().toString())))
        {
            QMessageBox::critical(nullptr, QObject::tr("Invalid email"),
                                  QObject::tr("The email must be unique.\n"
                                              "Click Cancel to exit."), QMessageBox::Cancel);
            ui->lineEdit_email_r1->setFocus();
            t=1;
        }
    }
    //currentc
    if (ui->lineEdit_capacity_r1->text().isEmpty() && t==0) {
        QMessageBox::warning(this, "Error", "Please enter a center currentCapacity.");
        ui->lineEdit_capacity_r1->setFocus();
        t=1;
    }else if (n1 == false && t==0) {
        QMessageBox::warning(this, "Error", "Please enter a number in center currentCapacity.");
        ui->lineEdit_capacity_r1->setFocus();
        t=1;
    }else if (currentCapacity <0 && t==0) {
        QMessageBox::warning(this, "Error", "Please enter a center currentCapacity greater than 0.");
        ui->lineEdit_capacity_r1->setFocus();
        t=1;
    }else if (currentCapacity > maxCapacity && t==0) {
        QMessageBox::warning(this, "Error", "Please enter a current capacity less than the maximum capacity.");
        ui->lineEdit_capacity_r1->setFocus();
        t=1;
    }
    //maxc
    if (ui->lineEdit_mawc_r1->text().isEmpty() && t==0) {
        QMessageBox::warning(this, "Error", "Please enter a center maxCapacity.");
        ui->lineEdit_mawc_r1->setFocus();
        t=1;
    }else if (n2 == false && t==0) {
        QMessageBox::warning(this, "Error", "Please enter a number in center maxCapacity.");
        ui->lineEdit_mawc_r1->setFocus();
        t=1;
    }else if (maxCapacity < 1 && t==0) {
        QMessageBox::warning(this, "Error", "Please enter a maximum capacity greater than 0.");
        ui->lineEdit_mawc_r1->setFocus();
        t=1;
    }else if (maxCapacity > 9999 && t==0) {
        QMessageBox::warning(this, "Error", "Please enter a maximum capacity lower than 10000.");
        ui->lineEdit_mawc_r1->setFocus();
        t=1;
    }
    //status
    if (ui->radioButton_active_r1->isChecked()) {
        status = "Active";
    }
    else if (ui->radioButton_inactive_r1->isChecked()) {
        status = "Inactive";
    }
    else if (ui->radioButton_saturated_r1->isChecked()) {
        status = "Saturated";
    }
    else if(t==0){
        QMessageBox::warning(this, "Error", "Please select a status");
        t=1;
    }



    if(selectedCenterID==-1 && t==0){
        RecyclingCenter R(0,name,address,email,maxCapacity,currentCapacity,status,municipalityID);
        bool test=R.addCenter();
        if (test){
            ui->tableView_r1->setModel(Rtmp.displayCenter());
            resetform_r1();
            setupChart_r1();
            QMessageBox::information(nullptr, QObject::tr("OK"),
                                     QObject::tr("Add succefully\n"
                                                 "Click Cancel to exit."), QMessageBox::Cancel);
        }else{
            QMessageBox::critical(nullptr, QObject::tr("NOT OK"),
                                  QObject::tr("Adding failed\n"
                                              "Click Cancel to exit."), QMessageBox::Cancel);
        }
    }else if(selectedCenterID!=-1 && t==0){
        RecyclingCenter R(selectedCenterID, name, address, email,maxCapacity, currentCapacity,status, municipalityID);

        if (R.updateCenter(selectedCenterID)) {
            ui->tableView_r1->setModel(Rtmp.displayCenter());
            setupChart_r1();
            resetform_r1();
            reloadCenterStats_r();
            QMessageBox::information(this, "OK", "Center updated successfully\n"
                                                 "Click ok to exit.", QMessageBox::Cancel);
        }
    }
}


void GWasteCollection::on_tableView_r1_doubleClicked(const QModelIndex &index)
{
    int centerID = index.sibling(index.row(),0).data().toInt();
    if(Active_Center!=centerID){
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(
            this,
            "Delete Confirmation",
            "Do you want to delete this center ?",
            QMessageBox::Yes | QMessageBox::No
            );

        if (reply == QMessageBox::Yes) {
            if (Rtmp.deleteCenter(centerID)) {
                ui->tableView_r1->setModel(Rtmp.displayCenter());
                resetform_r1();
                setupChart_r1();
            }
        }
    }else{
        QMessageBox::information(this, "Warning", "You can't delete an active center!\n"
                                                  "Click ok to exit.", QMessageBox::Cancel);
    }
}


void GWasteCollection::on_tableView_r1_clicked(const QModelIndex &index)
{
    selectedCenterID = index.sibling(index.row(),0).data().toInt();
    QString name = index.sibling(index.row(),1).data().toString();
    QString address = index.sibling(index.row(),2).data().toString();
    QString email = index.sibling(index.row(),3).data().toString();
    int maxc = index.sibling(index.row(),4).data().toInt();
    int curc = index.sibling(index.row(),5).data().toInt();
    QString status = index.sibling(index.row(),6).data().toString();
    QString municipality = index.sibling(index.row(),8).data().toString();

    ui->lineEdit_name_r1->setText(name);
    ui->lineEdit_adress_r1->setText(address);
    ui->lineEdit_email_r1->setText(email);
    ui->lineEdit_mawc_r1->setText(QString::number(maxc));
    ui->lineEdit_capacity_r1->setText(QString::number(curc));

    int comboIndex = ui->comboBox_municialityname_r1->findText(municipality);
    if (comboIndex != -1)
        ui->comboBox_municialityname_r1->setCurrentIndex(comboIndex);
    if(status =="Active"){
        ui->radioButton_active_r1->setChecked(true);
    }else if (status=="Inactive"){
        ui->radioButton_inactive_r1->setChecked(true);
    }else if (status=="Saturated"){
        ui->radioButton_saturated_r1->setChecked(true);
    }
}
void GWasteCollection::resetform_r1(){
    selectedCenterID = -1;

    ui->lineEdit_name_r1->clear();
    ui->lineEdit_adress_r1->clear();
    ui->lineEdit_email_r1->clear();
    ui->lineEdit_mawc_r1->clear();
    ui->lineEdit_capacity_r1->clear();
    ui->comboBox_municialityname_r1->setCurrentIndex(0);

    ui->radioButton_active_r1->setAutoExclusive(false);
    ui->radioButton_inactive_r1->setAutoExclusive(false);
    ui->radioButton_saturated_r1->setAutoExclusive(false);

    ui->radioButton_active_r1->setChecked(false);
    ui->radioButton_inactive_r1->setChecked(false);
    ui->radioButton_saturated_r1->setChecked(false);

    ui->radioButton_active_r1->setAutoExclusive(true);
    ui->radioButton_inactive_r1->setAutoExclusive(true);
    ui->radioButton_saturated_r1->setAutoExclusive(true);

}


void GWasteCollection::on_pushButton_deselect_r1_clicked()
{
    if(ui->tableView_r1->selectionModel()->hasSelection()){
        resetform_r1();
        ui->tableView_r1->clearSelection();
        ui->tableView_r1->selectionModel()->clear();
    }
}

void GWasteCollection::on_lineEdit_search_r1_textChanged(const QString &arg1)
{
    if (arg1.isEmpty())
        ui->tableView_r1->setModel(Rtmp.displayCenter());
    else
        ui->tableView_r1->setModel(Rtmp.searchCenterByName(arg1));
}


void GWasteCollection::on_pushButton_tristatus_r1_clicked()
{
    ui->tableView_r1->setModel(Rtmp.sortCenterByStatus());
}


void GWasteCollection::on_pushButton_export_r1_clicked()
{
    QAbstractItemModel *model = ui->tableView_r1->model();
    if (!model) return;

    QString downloadsPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    QString defaultFileName = downloadsPath + "/Center_export_" + QDate::currentDate().toString("yyyy_MM_dd") + ".csv";
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Export Excel",
        defaultFileName,
        "Excel Files (*.csv)"
        );

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Cannot create file");
        return;
    }

    QTextStream out(&file);

    // 🔹 En-têtes
    for (int col = 0; col < model->columnCount(); col++) {
        out << model->headerData(col, Qt::Horizontal).toString();
        if (col < model->columnCount() - 1)
            out << ";"; // Excel FR = ;
    }
    out << "\n";

    // 🔹 Données
    for (int row = 0; row < model->rowCount(); row++) {
        for (int col = 0; col < model->columnCount(); col++) {
            QString data = model->data(model->index(row, col)).toString();
            data.replace(";", ","); // sécurité
            out << data;

            if (col < model->columnCount() - 1)
                out << ";";
        }
        out << "\n";
    }

    file.close();

    QMessageBox::information(this, "Export",
                             "Export Excel Successfully !");
}
void GWasteCollection::setupChart_r1()
{
    if (ui->label_rondstatistic_r1->layout()) {
        QLayoutItem *item;
        while ((item = ui->label_rondstatistic_r1->layout()->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        delete ui->label_rondstatistic_r1->layout();
    }

    // --- calcul des valeurs low, medium, high ---
    int low=0, medium=0, high=0;
    QSqlQueryModel *model = Rtmp.displayCenter();
    int rowCount = model->rowCount();
    for (int i=0; i<rowCount; ++i) {
        int maxCap = model->data(model->index(i,4)).toInt();
        int curCap = model->data(model->index(i,5)).toInt();
        if (maxCap==0) continue;
        double fillRate = (static_cast<double>(curCap)/maxCap)*100.0;
        if (fillRate <50.0) low++;
        else if (fillRate <=80.0) medium++;
        else high++;
    }

    // --- création du graphique ---
    QPieSeries *series = new QPieSeries();

    if(low>0)    series->append("0-50%", low);
    if(medium>0) series->append("50-80%", medium);
    if(high>0)   series->append("80-100%", high);

    // --- couleurs et labels ---
    for(int i=0;i<series->count();i++){
        QPieSlice *slice = series->slices().at(i);
        slice->setLabelVisible(true);
        slice->setLabelFont(QFont("Onest",12,QFont::Bold));
        slice->setLabelBrush(QBrush(Qt::black));

        if(slice->label().startsWith("0-50"))
            slice->setColor(QColor(104, 156, 58));
        else if(slice->label().startsWith("50-80"))
            slice->setColor(QColor(141, 162, 181));
        else if(slice->label().startsWith("80-100"))
            slice->setColor(QColor(50, 72, 95));

        // label avec %
        if(slice->label().startsWith("0-50"))
            slice->setLabel(QString("%1%").arg(int((double)low/rowCount*100)));
        else if(slice->label().startsWith("50-80"))
            slice->setLabel(QString("%1%").arg(int((double)medium/rowCount*100)));
        else if(slice->label().startsWith("80-100"))
            slice->setLabel(QString("%1%").arg(int((double)high/rowCount*100)));
    }

    // --- création du chart ---
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->legend()->hide();
    chart->setBackgroundVisible(false);

    chart->setAnimationOptions(QChart::AllAnimations);
    chart->setAnimationDuration(1000);
    chart->setAnimationEasingCurve(QEasingCurve::OutCubic);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setStyleSheet("background: transparent;");

    // --- nouveau layout ---
    QVBoxLayout *layout = new QVBoxLayout(ui->label_rondstatistic_r1);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);
    layout->addWidget(chartView,1);
    ui->label_rondstatistic_r1->setLayout(layout);
}

void GWasteCollection::predictcalcul_r()
{
    // Récupère les données depuis RecyclingCenter
    QSqlQueryModel *data = Rtmp.getCentersPredictionData();

    // Crée le modèle pour le tableau
    QStandardItemModel *model = new QStandardItemModel();
    model->setHorizontalHeaderLabels({"Center Name", "Remaining Interventions", "Full Capacity In (Hours)","ID"});

    // Remplissage du modèle
    for(int i = 0; i < data->rowCount(); i++)
    {
        int center_id = data->index(i, 0).data().toInt();
        double maxC = data->index(i, 1).data().toDouble();
        double curC = data->index(i, 2).data().toDouble();
        double sumWaste = data->index(i, 3).data().toDouble();  // quantité moyenne par intervention
        QString Cname = data->index(i, 5).data().toString();

        if(sumWaste == 0) continue;
        double nbc = Rtmp.getCentersInMunicipality(data->index(i, 4).data().toDouble());
        double avgWaste = sumWaste / nbc;
        double remaining_capacity = maxC - curC;
        double remaining_interventions = remaining_capacity / avgWaste;   // basé sur la moyenne municipale
        double hours_to_full = 0;
        if(remaining_interventions>=1){
            hours_to_full= qRound(remaining_interventions) * 24;
        }else{
            remaining_interventions=1;
            hours_to_full = 24;
        }

        QStandardItem *itemCenter = new QStandardItem(Cname);
        QStandardItem *itemRemaining = new QStandardItem(QString::number(qRound(remaining_interventions)));
        QStandardItem *itemHours = new QStandardItem(QString::number(hours_to_full));
        QStandardItem *itemId = new QStandardItem(QString::number(center_id));
        itemHours->setData(hours_to_full, Qt::UserRole);

        model->appendRow({itemCenter, itemRemaining, itemHours, itemId});
    }

    // Crée un proxy model pour gérer le tri
    QSortFilterProxyModel *proxy = new QSortFilterProxyModel(this);
    proxy->setSourceModel(model);
    proxy->setSortRole(Qt::UserRole); // <-- tri basé sur le double stocké dans UserRole
    ui->tableView_predict_r2->setModel(proxy);
    ui->tableView_predict_r2->setSortingEnabled(true);
    ui->tableView_predict_r2->setColumnHidden(3, true);

    // Trie immédiat sur la colonne des heures
    proxy->sort(2, Qt::AscendingOrder);

    for(int i = 0; i < proxy->rowCount(); ++i) {
        ui->tableView_predict_r2->verticalHeader()->model()->setHeaderData(i, Qt::Vertical, i + 1);
    }
}

void GWasteCollection::saturationsStats_r()
{
    //tableau de prediction
    predictcalcul_r();
    //tableau de centres saturés
    ui->tableView_saturated_r2->setModel(Rtmp.getCentersSaturated());
    //nb de centres saturés
    int nb_saturated=ui->tableView_saturated_r2->model()->rowCount();
    ui->label_scenters_r2->setText(QString::number(nb_saturated));
    //rate saturation
    int nb_centers=ui->tableView_r1->model()->rowCount();
    int rate = (int)((double)nb_saturated / nb_centers * 100);
    ui->label_saturationr_r2->setText(QString::number(rate));
    //
    QSqlQueryModel *model= Rtmp.getCentersActive();
    int atr=0;
    for(int i = 0; i < model->rowCount(); i++)
    {
        double maxC = model->index(i, 4).data().toDouble();
        double curC = model->index(i, 5).data().toDouble();
        if((curC / maxC)*100>90){
            atr++;
        }
    }
    ui->label_atr_r2->setText(QString::number(atr));
    //le plus suceptible de saturation
    if(ui->tableView_predict_r2->model() != nullptr && ui->tableView_predict_r2->model()->rowCount() > 0) {
        QModelIndex index = ui->tableView_predict_r2->model()->index(0, 3);
        int id = ui->tableView_predict_r2->model()->data(index).toInt();
        ui->label_mostc_r2->setText(QString::number(id));
    }else{
        ui->label_mostc_r2->setText("0");
    }
}

bool GWasteCollection::sendMailQt_r(
    const QString &fromEmail,
    const QString &fromName,
    const QString &toEmail,
    const QString &toName,
    const QString &subject,
    const QString &body,
    const QString &smtpHost,
    quint16 smtpPort,
    const QString &username,
    const QString &password,
    bool useSsl
    ) {
    QSslSocket socket;
    if(useSsl)
        socket.connectToHostEncrypted(smtpHost, smtpPort);
    else
        socket.connectToHost(smtpHost, smtpPort);

    if (!socket.waitForConnected(5000)) {
        qDebug() << "Erreur : impossible de se connecter au serveur SMTP";
        return false;
    }

    auto interaction = [&](const QString &cmd, const QString &expectedResponse) {
        if (!cmd.isEmpty()) socket.write(cmd.toUtf8() + "\r\n");
        if (!socket.waitForReadyRead(5000)) return false;
        return socket.readAll().contains(expectedResponse.toUtf8());
    };

    if (!interaction("", "220")) return false;
    if (!interaction("EHLO localhost", "250")) return false;
    if (!interaction("AUTH LOGIN", "334")) return false;
    if (!interaction(username.toUtf8().toBase64(), "334")) return false;
    if (!interaction(password.toUtf8().toBase64(), "235")) return false;
    if (!interaction("MAIL FROM:<" + fromEmail + ">", "250")) return false;
    if (!interaction("RCPT TO:<" + toEmail + ">", "250")) return false;
    if (!interaction("DATA", "354")) return false;

    QString message = QString(
                          "From: %1 <%2>\n"
                          "To: %3 <%4>\n"
                          "Subject: %5\n\n"
                          "%6\r\n.\r\n"
                          ).arg(fromName, fromEmail, toName, toEmail, subject, body);

    if (!interaction(message, "250")) return false;

    socket.write("QUIT\r\n");
    return true;
}

void GWasteCollection::on_pushButton_apply_r4_clicked()
{
    name_r   = ui->checkBox_name_r4->isChecked();
    teamid_r = ui->checkBox_teamid_r4->isChecked();
    statut_r = ui->checkBox_spinstr_r4->isChecked();
    waste_r  = ui->checkBox_wasteq_r4->isChecked();
    date_r   = ui->checkBox_date_r4->isChecked();
    rname_r  = ui->checkBox_respname_r4->isChecked();
    fmail_r  = ui->checkBox_femail_r4->isChecked();
    QMessageBox::information(this, "OK", "Email Settings changed\n"
                                         "Click ok to exit.", QMessageBox::Cancel);
    initMailSettings_r();
}

void GWasteCollection::fixtabaffect_r(){
    QSqlQueryModel *model = Rtmp.displayIntervenesForAffect();
    ui->tableView_affect_r3->setModel(model);
    ui->tableView_affect_r3->hideColumn(1);
    ui->tableView_affect_r3->hideColumn(4);
    ui->tableView_affect_r3->hideColumn(7);
    int colCombo = model->columnCount();
    model->insertColumn(colCombo);

    int colButton = model->columnCount();
    model->insertColumn(colButton);

    // headers
    model->setHeaderData(colCombo, Qt::Horizontal, "Center");
    model->setHeaderData(colButton, Qt::Horizontal, "Action");

    for(int i = 0; i < model->rowCount(); i++)
    {
        QComboBox *combo = new QComboBox();
        combo->setStyleSheet("color: #32485F;");

        int municipality_id = model->index(i, 7).data().toInt();

        QSqlQueryModel *centersModel = Rtmp.getCenterActiveByMunicipality(municipality_id,model->index(i, 5).data().toInt());

        combo->setModel(centersModel);
        combo->setModelColumn(1);

        ui->tableView_affect_r3->setIndexWidget(model->index(i, colCombo), combo);

        QPushButton *btn = new QPushButton("Assign");
        btn->setStyleSheet(" color: #32485F;");

        connect(btn, &QPushButton::clicked, [=]() {

            if (combo->count() == 0) {
                QMessageBox::warning(this, "Error", "No center available for this municipality!");
                return;
            }
            int selectedRow = combo->currentIndex();
            int centerID = combo->model()->index(selectedRow, 0).data().toInt();
            QString centerName = combo->currentText();
            QString address = combo->model()->index(selectedRow, 2).data().toString();
            QString email = combo->model()->index(selectedRow, 3).data().toString();
            int maxc = combo->model()->index(selectedRow, 4).data().toInt();
            int curc = combo->model()->index(selectedRow, 5).data().toInt();
            int waste = model->index(i, 5).data().toInt();
            curc=curc+waste;
            QString status;
            if(curc==maxc){
                status="Saturated";
            }else{
                status="Active";
            }
            int municipalityID = combo->model()->index(selectedRow, 7).data().toInt();
            int teamID = model->index(i, 0).data().toInt();
            //int nbSewers = model->index(i, 6).data().toInt();

            RecyclingCenter R(centerID,centerName, address, email,maxc, curc,status, municipalityID);

            if (R.updateCenter(centerID)) {
                reloadCenterStats_r();
                R.updateIntervenesStatus(teamID, municipalityID, model->index(i, 3).data().toDate());
                Stemp.clearCompletedSewers(teamID, model->index(i, 3).data().toDate());
                ui->tableView_r1->setModel(Rtmp.displayCenter());
                setupChart_r1();
                resetform_r1();
                int qrate = (int)((double)curc / maxc * 100);
                QString date = QDate::currentDate().toString("dd/MM/yyyy");

                QString monMessage = QString("Automatic report of %1\n\n").arg(date);

                if(name_r==1){
                    monMessage += "Center Name: " + centerName + "\n";
                }

                if(statut_r==1){
                    monMessage += "Center Status: " + status + "\n";
                }

                if(waste_r==1){
                    monMessage += "Quantity of waste coming: " + QString::number(waste) + " T\n";
                }

                if(date_r==1){
                    monMessage += "Date: " + date + "\n";
                }

                if(teamid_r==1){
                    monMessage += "Collection Team ID: " + QString::number(teamID) + "\n";
                }

                if(rname_r==1){
                    monMessage += "Manager: " + Session.getPosition() + "\n";
                }

                if(fmail_r==1){
                    monMessage += "Email: zigoocare@gmail.com \n";
                }

                monMessage += "\nTotal waste rate: " + QString::number(qrate) + "%\n";

                if(qrate > 80){
                    monMessage += "Status: ALERT: Risk of saturation!\n";
                } else {
                    monMessage += "Status: Normal\n";
                }

                bool resultat = sendMailQt_r(
                    "zigoocare@gmail.com",
                    "Admin Recyclage",
                    email,
                    "youssef",
                    "Saturation Alert - " + centerName,
                    monMessage,
                    "smtp.gmail.com",
                    465,
                    "zigoocare@gmail.com",
                    "ewng vxbd qltx fafn",
                    true
                    );

                if(resultat) {
                    QMessageBox::information(this, "OK", "Center assigned successfully\n"
                                                         "Click ok to exit.", QMessageBox::Cancel);
                    fixtabaffect_r();
                }
            }
        });

        ui->tableView_affect_r3->setIndexWidget(model->index(i, colButton), btn);
    }
}
void GWasteCollection::initMailSettings_r(){
    if(name_r==1){
        ui->checkBox_name_r4->setChecked(true);
    }else{
        ui->checkBox_name_r4->setChecked(false);
    }
    if(teamid_r==1){
        ui->checkBox_teamid_r4->setChecked(true);
    }else{
        ui->checkBox_teamid_r4->setChecked(false);
    }
    if(statut_r==1){
        ui->checkBox_spinstr_r4->setChecked(true);
    }else{
        ui->checkBox_spinstr_r4->setChecked(false);
    }
    if(waste_r==1){
        ui->checkBox_wasteq_r4->setChecked(true);
    }else{
        ui->checkBox_wasteq_r4->setChecked(false);
    }
    if(date_r==1){
        ui->checkBox_date_r4->setChecked(true);
    }else{
        ui->checkBox_date_r4->setChecked(false);
    }
    if(rname_r==1){
        ui->checkBox_respname_r4->setChecked(true);
    }else{
        ui->checkBox_respname_r4->setChecked(false);
    }
    if(fmail_r==1){
        ui->checkBox_femail_r4->setChecked(true);
    }else{
        ui->checkBox_femail_r4->setChecked(false);
    }
}

void GWasteCollection::toarduinoCenter_r(QString data) {

    if(data.startsWith("Center-id:")) {
        A.write_to_arduino("OK\n");
        bool ok;
        int id = data.sliced(10).toInt(&ok);
        if(ok) {
            Active_Center=id;
            QSqlQueryModel *model = Rtmp.getCentersPredictionDataForOne(id);

            if(model->rowCount() > 0) {
                double maxC = model->index(0, 1).data().toDouble();
                double curC = model->index(0, 2).data().toDouble();
                double sumWaste = model->index(0, 3).data().toDouble();
                double nbc = Rtmp.getCentersInMunicipality(model->index(0, 4).data().toDouble());
                double avgWaste = sumWaste / nbc;
                double remaining_capacity = maxC - curC;
                double remaining_interventions = remaining_capacity / avgWaste;
                double hours_to_full=0;
                if(remaining_interventions>=1){
                    hours_to_full= qRound(remaining_interventions) * 24;
                }else if(remaining_interventions<1 && remaining_interventions>0){
                    hours_to_full = 24;
                }else{
                    hours_to_full = 0;
                }
                QString status=model->index(0, 6).data().toString();
                if(status == "Inactive"){
                    A.write_to_arduino("Center_Inactive\n");
                }
                else if(maxC==curC || status == "Saturated"){
                    A.write_to_arduino("Center_Saturated\n");
                }else{
                    QString msg1 = QString("Waste:%1/%2\n").arg(curC).arg(maxC);
                    QString msg2;
                    if(hours_to_full!=0){
                        msg2 = QString("Full-in:%1h\n").arg(hours_to_full);
                    }else{
                        msg2 = QString("Full-in:Unknown\n");
                    }
                    A.write_to_arduino(msg1.toUtf8());
                    A.write_to_arduino(msg2.toUtf8());
                }
            }
        }
    }
}
void GWasteCollection::reloadCenterStats_r(){
    if (ret == 0) A.write_to_arduino("give-center-id\n");
}
