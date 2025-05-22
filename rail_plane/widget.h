#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QTextEdit>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRadioButton>
#include <QButtonGroup>
#include <QDateEdit>
#include <QDate>
#include <QSslError>
#include <QProcess>
#include <limits> // 用于INT_MAX

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void onSearchClicked();
    void onNetworkReply(QNetworkReply *reply);
    void onTransportTypeChanged(int id);
    void onSslErrors(QNetworkReply *reply, const QList<QSslError> &errors);
    void onCurlStandardOutput();
    void onCurlStandardError();
    void onCurlFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onDetailButtonClicked();

private:
    Ui::Widget *ui;
    QNetworkAccessManager *networkManager;
    QComboBox *fromCityCombo;
    QComboBox *toCityCombo;
    QCheckBox *allowTransferCheck;
    QPushButton *searchButton;
    QTextEdit *resultText;
    QRadioButton *flightRadio;
    QRadioButton *trainRadio;
    QRadioButton *bothRadio;
    QButtonGroup *transportTypeGroup;
    QDateEdit *dateEdit;
    QProcess *curlProcess;
    QByteArray curlOutput;
    QPushButton *detailButton = nullptr;
    QString lowestPriceFlightDate;
    
    void setupUI();
    void loadCityCodes();
    QString getCityCode(const QString &cityName);
    void searchFlightPrice(const QString &fromCode, const QString &toCode, bool direct, const QDate &date);
    void searchTrainPrice(const QString &fromCode, const QString &toCode, const QDate &date);
    void parseFlightData(const QJsonObject &obj);
    QString formatDate(const QString &dateStr);
    void searchFlightDetails(const QString &fromCode, const QString &toCode, const QString &dateStr);
};
#endif // WIDGET_H
