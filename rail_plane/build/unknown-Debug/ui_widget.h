/********************************************************************************
** Form generated from reading UI file 'widget.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Widget
{
public:
    QVBoxLayout *mainLayout;
    QLabel *titleLabel;
    QFrame *headerCard;
    QVBoxLayout *headerLayout;
    QGroupBox *transportTypeBox;
    QHBoxLayout *transportLayout;
    QRadioButton *flightRadio;
    QRadioButton *trainRadio;
    QRadioButton *bothRadio;
    QSpacerItem *horizontalSpacer;
    QHBoxLayout *cityLayout;
    QLabel *fromLabel;
    QComboBox *fromCityCombo;
    QSpacerItem *horizontalSpacer_2;
    QLabel *toLabel;
    QComboBox *toCityCombo;
    QHBoxLayout *optionsLayout;
    QLabel *dateLabel;
    QDateEdit *dateEdit;
    QSpacerItem *horizontalSpacer_3;
    QCheckBox *allowTransferCheck;
    QHBoxLayout *searchLayout;
    QSpacerItem *horizontalSpacer_4;
    QPushButton *searchButton;
    QSpacerItem *horizontalSpacer_5;
    QFrame *resultCard;
    QVBoxLayout *resultLayout;
    QFrame *detailFrame;
    QVBoxLayout *detailLayout;
    QLabel *detailLabel;
    QSpacerItem *verticalSpacer;
    QVBoxLayout *detailContentLayout;
    QHBoxLayout *urlLayout;
    QLabel *urlLabel;
    QLineEdit *customUrlEdit;
    QPushButton *detailButton;
    QLabel *resultLabel;
    QTextEdit *resultText;
    QLabel *footerLabel;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName(QString::fromUtf8("Widget"));
        Widget->resize(800, 907);
        Widget->setStyleSheet(QString::fromUtf8("QWidget { background-color: #f5f5f7; font-family: Arial, sans-serif; }\n"
"QComboBox { padding: 6px; border: 1px solid #ccc; border-radius: 4px; background-color: white; }\n"
"QComboBox::drop-down { subcontrol-origin: padding; subcontrol-position: right; width: 20px; border-left: none; }\n"
"QComboBox::down-arrow { image: url(down_arrow.png); width: 12px; height: 12px; }\n"
"QComboBox:focus { border: 1px solid #3498db; }\n"
"QPushButton { background-color: #3498db; color: white; border: none; padding: 8px 16px; border-radius: 4px; font-weight: bold; }\n"
"QPushButton:hover { background-color: #2980b9; }\n"
"QPushButton:pressed { background-color: #1f618d; }\n"
"QPushButton#detailButton { background-color: #27ae60; }\n"
"QPushButton#detailButton:hover { background-color: #219955; }\n"
"QCheckBox { color: #333; spacing: 8px; }\n"
"QCheckBox::indicator { width: 16px; height: 16px; border: 1px solid #ccc; border-radius: 3px; }\n"
"QCheckBox::indicator:checked { background-color: #3498db; }\n"
"QRadioButton { color"
                        ": #333; spacing: 8px; }\n"
"QRadioButton::indicator { width: 15px; height: 15px; border: 1px solid #ccc; border-radius: 8px; }\n"
"QRadioButton::indicator:checked { background-color: #3498db; border: 2px solid white; }\n"
"QGroupBox { border: 1px solid #ccc; border-radius: 5px; margin-top: 1em; padding-top: 15px; }\n"
"QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }\n"
"QTextEdit { border: 1px solid #ccc; border-radius: 4px; padding: 8px; background-color: white; color: #333; }\n"
"QLabel { color: #333; }"));
        mainLayout = new QVBoxLayout(Widget);
        mainLayout->setSpacing(15);
        mainLayout->setObjectName(QString::fromUtf8("mainLayout"));
        mainLayout->setContentsMargins(20, 20, 20, 20);
        titleLabel = new QLabel(Widget);
        titleLabel->setObjectName(QString::fromUtf8("titleLabel"));
        titleLabel->setStyleSheet(QString::fromUtf8("font-size: 20px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;"));
        titleLabel->setAlignment(Qt::AlignCenter);

        mainLayout->addWidget(titleLabel);

        headerCard = new QFrame(Widget);
        headerCard->setObjectName(QString::fromUtf8("headerCard"));
        headerCard->setStyleSheet(QString::fromUtf8("QFrame { background-color: white; border-radius: 8px; border: 1px solid #e0e0e0; }"));
        headerCard->setFrameShape(QFrame::StyledPanel);
        headerCard->setFrameShadow(QFrame::Raised);
        headerLayout = new QVBoxLayout(headerCard);
        headerLayout->setSpacing(15);
        headerLayout->setObjectName(QString::fromUtf8("headerLayout"));
        headerLayout->setContentsMargins(15, 15, 15, 15);
        transportTypeBox = new QGroupBox(headerCard);
        transportTypeBox->setObjectName(QString::fromUtf8("transportTypeBox"));
        transportLayout = new QHBoxLayout(transportTypeBox);
        transportLayout->setSpacing(30);
        transportLayout->setObjectName(QString::fromUtf8("transportLayout"));
        transportLayout->setContentsMargins(10, 15, 10, 10);
        flightRadio = new QRadioButton(transportTypeBox);
        flightRadio->setObjectName(QString::fromUtf8("flightRadio"));
        flightRadio->setChecked(true);

        transportLayout->addWidget(flightRadio);

        trainRadio = new QRadioButton(transportTypeBox);
        trainRadio->setObjectName(QString::fromUtf8("trainRadio"));

        transportLayout->addWidget(trainRadio);

        bothRadio = new QRadioButton(transportTypeBox);
        bothRadio->setObjectName(QString::fromUtf8("bothRadio"));

        transportLayout->addWidget(bothRadio);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        transportLayout->addItem(horizontalSpacer);


        headerLayout->addWidget(transportTypeBox);

        cityLayout = new QHBoxLayout();
        cityLayout->setSpacing(10);
        cityLayout->setObjectName(QString::fromUtf8("cityLayout"));
        fromLabel = new QLabel(headerCard);
        fromLabel->setObjectName(QString::fromUtf8("fromLabel"));
        fromLabel->setMinimumSize(QSize(80, 0));

        cityLayout->addWidget(fromLabel);

        fromCityCombo = new QComboBox(headerCard);
        fromCityCombo->setObjectName(QString::fromUtf8("fromCityCombo"));
        fromCityCombo->setMinimumSize(QSize(150, 0));

        cityLayout->addWidget(fromCityCombo);

        horizontalSpacer_2 = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        cityLayout->addItem(horizontalSpacer_2);

        toLabel = new QLabel(headerCard);
        toLabel->setObjectName(QString::fromUtf8("toLabel"));
        toLabel->setMinimumSize(QSize(80, 0));

        cityLayout->addWidget(toLabel);

        toCityCombo = new QComboBox(headerCard);
        toCityCombo->setObjectName(QString::fromUtf8("toCityCombo"));
        toCityCombo->setMinimumSize(QSize(150, 0));

        cityLayout->addWidget(toCityCombo);


        headerLayout->addLayout(cityLayout);

        optionsLayout = new QHBoxLayout();
        optionsLayout->setSpacing(15);
        optionsLayout->setObjectName(QString::fromUtf8("optionsLayout"));
        dateLabel = new QLabel(headerCard);
        dateLabel->setObjectName(QString::fromUtf8("dateLabel"));
        dateLabel->setMinimumSize(QSize(80, 0));

        optionsLayout->addWidget(dateLabel);

        dateEdit = new QDateEdit(headerCard);
        dateEdit->setObjectName(QString::fromUtf8("dateEdit"));
        dateEdit->setMinimumSize(QSize(150, 0));
        dateEdit->setCalendarPopup(true);

        optionsLayout->addWidget(dateEdit);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        optionsLayout->addItem(horizontalSpacer_3);

        allowTransferCheck = new QCheckBox(headerCard);
        allowTransferCheck->setObjectName(QString::fromUtf8("allowTransferCheck"));

        optionsLayout->addWidget(allowTransferCheck);


        headerLayout->addLayout(optionsLayout);

        searchLayout = new QHBoxLayout();
        searchLayout->setObjectName(QString::fromUtf8("searchLayout"));
        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        searchLayout->addItem(horizontalSpacer_4);

        searchButton = new QPushButton(headerCard);
        searchButton->setObjectName(QString::fromUtf8("searchButton"));
        searchButton->setMinimumSize(QSize(0, 36));
        searchButton->setCursor(QCursor(Qt::PointingHandCursor));

        searchLayout->addWidget(searchButton);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        searchLayout->addItem(horizontalSpacer_5);


        headerLayout->addLayout(searchLayout);


        mainLayout->addWidget(headerCard);

        resultCard = new QFrame(Widget);
        resultCard->setObjectName(QString::fromUtf8("resultCard"));
        resultCard->setStyleSheet(QString::fromUtf8("QFrame { background-color: white; border-radius: 8px; border: 1px solid #e0e0e0; }"));
        resultCard->setFrameShape(QFrame::StyledPanel);
        resultCard->setFrameShadow(QFrame::Raised);
        resultLayout = new QVBoxLayout(resultCard);
        resultLayout->setObjectName(QString::fromUtf8("resultLayout"));
        resultLayout->setContentsMargins(15, 15, 15, 15);
        detailFrame = new QFrame(resultCard);
        detailFrame->setObjectName(QString::fromUtf8("detailFrame"));
        detailFrame->setStyleSheet(QString::fromUtf8("QFrame { background-color: #f5f5f7; border-radius: 8px; margin-top: 10px; padding: 10px; }"));
        detailFrame->setFrameShape(QFrame::StyledPanel);
        detailFrame->setFrameShadow(QFrame::Raised);
        detailLayout = new QVBoxLayout(detailFrame);
        detailLayout->setObjectName(QString::fromUtf8("detailLayout"));
        detailLabel = new QLabel(detailFrame);
        detailLabel->setObjectName(QString::fromUtf8("detailLabel"));
        detailLabel->setStyleSheet(QString::fromUtf8("font-weight: bold; font-size: 16px; color: #2c3e50; text-align: center;"));
        detailLabel->setAlignment(Qt::AlignCenter);

        detailLayout->addWidget(detailLabel);

        verticalSpacer = new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Fixed);

        detailLayout->addItem(verticalSpacer);

        detailContentLayout = new QVBoxLayout();
        detailContentLayout->setSpacing(15);
        detailContentLayout->setObjectName(QString::fromUtf8("detailContentLayout"));
        urlLayout = new QHBoxLayout();
        urlLayout->setObjectName(QString::fromUtf8("urlLayout"));
        urlLabel = new QLabel(detailFrame);
        urlLabel->setObjectName(QString::fromUtf8("urlLabel"));
        urlLabel->setStyleSheet(QString::fromUtf8("font-size: 14px; font-weight: bold;"));

        urlLayout->addWidget(urlLabel);

        customUrlEdit = new QLineEdit(detailFrame);
        customUrlEdit->setObjectName(QString::fromUtf8("customUrlEdit"));
        customUrlEdit->setMinimumSize(QSize(0, 35));
        customUrlEdit->setStyleSheet(QString::fromUtf8("QLineEdit { padding: 5px 10px; border: 2px solid #3498db; border-radius: 6px; font-size: 13px; }"));

        urlLayout->addWidget(customUrlEdit);


        detailContentLayout->addLayout(urlLayout);

        detailButton = new QPushButton(detailFrame);
        detailButton->setObjectName(QString::fromUtf8("detailButton"));
        detailButton->setMinimumSize(QSize(0, 45));
        detailButton->setCursor(QCursor(Qt::PointingHandCursor));
        detailButton->setStyleSheet(QString::fromUtf8("QPushButton#detailButton { background-color: #e74c3c; color: white; font-weight: bold; font-size: 14px; border-radius: 6px; } \n"
"QPushButton#detailButton:hover { background-color: #c0392b; }"));

        detailContentLayout->addWidget(detailButton);


        detailLayout->addLayout(detailContentLayout);


        resultLayout->addWidget(detailFrame);

        resultLabel = new QLabel(resultCard);
        resultLabel->setObjectName(QString::fromUtf8("resultLabel"));
        resultLabel->setStyleSheet(QString::fromUtf8("font-weight: bold; font-size: 14px; color: #333;"));

        resultLayout->addWidget(resultLabel);

        resultText = new QTextEdit(resultCard);
        resultText->setObjectName(QString::fromUtf8("resultText"));
        resultText->setMinimumSize(QSize(0, 250));
        resultText->setStyleSheet(QString::fromUtf8("QTextEdit { font-family: 'Consolas', 'Monaco', monospace; }"));
        resultText->setReadOnly(true);

        resultLayout->addWidget(resultText);


        mainLayout->addWidget(resultCard);

        footerLabel = new QLabel(Widget);
        footerLabel->setObjectName(QString::fromUtf8("footerLabel"));
        footerLabel->setStyleSheet(QString::fromUtf8("color: #7f8c8d; margin-top: 5px;"));
        footerLabel->setAlignment(Qt::AlignCenter);

        mainLayout->addWidget(footerLabel);


        retranslateUi(Widget);

        searchButton->setDefault(true);


        QMetaObject::connectSlotsByName(Widget);
    } // setupUi

    void retranslateUi(QWidget *Widget)
    {
        Widget->setWindowTitle(QCoreApplication::translate("Widget", "\344\272\244\351\200\232\347\245\250\344\273\267\346\237\245\350\257\242\347\263\273\347\273\237", nullptr));
        titleLabel->setText(QCoreApplication::translate("Widget", "\344\272\244\351\200\232\347\245\250\344\273\267\346\237\245\350\257\242\347\263\273\347\273\237", nullptr));
        transportTypeBox->setTitle(QCoreApplication::translate("Widget", "\344\272\244\351\200\232\346\226\271\345\274\217", nullptr));
        flightRadio->setText(QCoreApplication::translate("Widget", "\351\243\236\346\234\272", nullptr));
        trainRadio->setText(QCoreApplication::translate("Widget", "\347\201\253\350\275\246", nullptr));
        bothRadio->setText(QCoreApplication::translate("Widget", "\347\273\274\345\220\210\346\237\245\350\257\242", nullptr));
        fromLabel->setText(QCoreApplication::translate("Widget", "\345\207\272\345\217\221\345\237\216\345\270\202:", nullptr));
        toLabel->setText(QCoreApplication::translate("Widget", "\345\210\260\350\276\276\345\237\216\345\270\202:", nullptr));
        dateLabel->setText(QCoreApplication::translate("Widget", "\345\207\272\345\217\221\346\227\245\346\234\237:", nullptr));
        dateEdit->setDisplayFormat(QCoreApplication::translate("Widget", "yyyy\345\271\264MM\346\234\210dd\346\227\245", nullptr));
        allowTransferCheck->setText(QCoreApplication::translate("Widget", "\345\205\201\350\256\270\344\270\255\350\275\254", nullptr));
        searchButton->setText(QCoreApplication::translate("Widget", "\345\274\200\345\247\213\346\220\234\347\264\242", nullptr));
        detailLabel->setText(QCoreApplication::translate("Widget", "\350\257\246\347\273\206\344\277\241\346\201\257\346\237\245\350\257\242", nullptr));
        urlLabel->setText(QCoreApplication::translate("Widget", "\347\275\221\345\235\200:", nullptr));
        customUrlEdit->setPlaceholderText(QCoreApplication::translate("Widget", "\350\276\223\345\205\245\350\207\252\345\256\232\344\271\211URL\346\210\226\344\275\277\347\224\250\347\263\273\347\273\237\347\224\237\346\210\220\347\232\204URL", nullptr));
        detailButton->setText(QCoreApplication::translate("Widget", "\346\237\245\347\234\213\350\257\246\347\273\206\350\210\252\347\217\255\344\277\241\346\201\257\357\274\210\345\220\253\344\270\255\350\275\254\347\253\231\357\274\211", nullptr));
        resultLabel->setText(QCoreApplication::translate("Widget", "\346\237\245\350\257\242\347\273\223\346\236\234", nullptr));
        footerLabel->setText(QCoreApplication::translate("Widget", "\302\251 2023 \344\272\244\351\200\232\347\245\250\344\273\267\346\237\245\350\257\242\347\263\273\347\273\237 | \346\225\260\346\215\256\346\235\245\346\272\220: \346\220\272\347\250\213API", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_H
