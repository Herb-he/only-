#include "widget.h"
#include "ui_widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QDebug>
#include <QGroupBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QDateEdit>
#include <QDate>
#include <QNetworkRequest>
#include <QSslConfiguration>
#include <QJsonArray>
#include <QNetworkReply>
#include <QSslError>
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>
#include <QFrame>
#include <QSizePolicy>
#include <QCheckBox>
#include <QPushButton>
#include <QDialog>
#include <QTimer>
#include <QPixmap>
#include <QLineEdit>
#include <QTextEdit>
#include <QFont>
#include <QMargins>
#include <QCalendarWidget>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    networkManager = new QNetworkAccessManager(this);
    
    // Initialize member variables to point to UI elements
    fromCityCombo = ui->fromCityCombo;
    toCityCombo = ui->toCityCombo;
    allowTransferCheck = ui->allowTransferCheck;
    searchButton = ui->searchButton;
    resultText = ui->resultText;
    flightRadio = ui->flightRadio;
    trainRadio = ui->trainRadio;
    bothRadio = ui->bothRadio;
    calendarButton = ui->calendarButton;
    calendarWidget = ui->calendarWidget;
    selectedDateValue = ui->selectedDateValue;
    detailButton = ui->detailButton;
    customUrlEdit = ui->customUrlEdit;
    
    // Set up calendar widget
    calendarWidget->setVisible(false);
    QDate currentDate = QDate::currentDate();
    QDate tomorrowDate = currentDate.addDays(1);
    calendarWidget->setSelectedDate(tomorrowDate);
    calendarWidget->setMinimumDate(currentDate); // Don't allow selecting dates in the past
    selectedDateValue->setText(tomorrowDate.toString("yyyy-MM-dd"));
    calendarButton->setText(tomorrowDate.toString("yyyy年MM月dd日"));
    
    // Create transport type button group
    transportTypeGroup = new QButtonGroup(this);
    transportTypeGroup->addButton(flightRadio, 0);
    transportTypeGroup->addButton(trainRadio, 1);
    transportTypeGroup->addButton(bothRadio, 2);
    
    // Hide detail button initially
    detailButton->setVisible(false);
    
    // Hide custom URL field - user cannot input custom URLs
    if (customUrlEdit) {
        customUrlEdit->setVisible(false);
    }
    
    // 应用现代化、简约而霸气的UI设计
    applyModernUIStyle();
    
    // Connect signals and slots
    loadCityCodes();
    
    connect(searchButton, &QPushButton::clicked, this, &Widget::onSearchClicked);
    connect(networkManager, &QNetworkAccessManager::finished, this, &Widget::onNetworkReply);
    connect(networkManager, &QNetworkAccessManager::sslErrors, this, &Widget::onSslErrors);
    connect(transportTypeGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
            this, &Widget::onTransportTypeChanged);
    connect(calendarButton, &QPushButton::clicked, this, &Widget::onCalendarButtonClicked);
    connect(calendarWidget, &QCalendarWidget::clicked, this, &Widget::onCalendarDateSelected);
            
    // Initialize curl process
    curlProcess = nullptr;
    
    connect(detailButton, &QPushButton::clicked, this, &Widget::onDetailButtonClicked);
}

Widget::~Widget()
{
    if (curlProcess) {
        if (curlProcess->state() == QProcess::Running) {
            curlProcess->terminate();
            curlProcess->waitForFinished(1000);
        }
        delete curlProcess;
    }
    delete ui;
}

void Widget::loadCityCodes()
{
    // 使用完整的城市代码列表
    QMap<QString, QString> cityCodes = {
        {"北京", "BJS"}, {"上海", "SHA"}, {"广州", "CAN"}, {"深圳", "SZX"}, 
        {"成都", "CTU"}, {"杭州", "HGH"}, {"武汉", "WUH"}, {"西安", "SIA"}, 
        {"重庆", "CKG"}, {"青岛", "TAO"}, {"长沙", "CSX"}, {"南京", "NKG"},
        {"厦门", "XMN"}, {"昆明", "KMG"}, {"大连", "DLC"}, {"天津", "TSN"},
        {"郑州", "CGO"}, {"三亚", "SYX"}, {"济南", "TNA"}, {"福州", "FOC"},
        {"阿勒泰", "AAT"}, {"阿克苏", "AKU"}, {"鞍山", "AOG"}, {"安庆", "AQG"},
        {"安顺", "AVA"}, {"阿拉善左旗", "AXF"}, {"中国澳门", "MFM"}, {"阿里", "NGQ"},
        {"阿拉善右旗", "RHT"}, {"阿尔山", "YIE"}, {"巴中", "BZX"}, {"百色", "AEB"},
        {"包头", "BAV"}, {"毕节", "BFJ"}, {"北海", "BHY"}, {"北京(大兴国际机场)", "BJS,PKX"},
        {"北京(首都国际机场)", "BJS,PEK"}, {"博乐", "BPL"}, {"保山", "BSD"}, {"白城", "DBC"},
        {"布尔津", "KJI"}, {"白山", "NBS"}, {"巴彦淖尔", "RLK"}, {"昌都", "BPX"},
        {"承德", "CDE"}, {"常德", "CGD"}, {"长春", "CGQ"}, {"朝阳", "CHG"},
        {"赤峰", "CIF"}, {"长治", "CIH"}, {"沧源", "CWJ"}, {"常州", "CZX"},
        {"池州", "JUH"}, {"大同", "DAT"}, {"达州", "DAX"}, {"稻城", "DCY"},
        {"丹东", "DDG"}, {"迪庆", "DIG"}, {"大理", "DLU"}, {"敦煌", "DNH"},
        {"东营", "DOY"}, {"大庆", "DQA"}, {"德令哈", "HXD"}, {"鄂尔多斯", "DSN"},
        {"额济纳旗", "EJN"}, {"恩施", "ENH"}, {"二连浩特", "ERL"}, {"阜阳", "FUG"},
        {"抚远", "FYJ"}, {"富蕴", "FYN"}, {"果洛", "GMQ"}, {"格尔木", "GOQ"},
        {"广元", "GYS"}, {"固原", "GYU"}, {"中国高雄", "KHH"}, {"赣州", "KOW"},
        {"贵阳", "KWE"}, {"桂林", "KWL"}, {"红原", "AHJ"}, {"海口", "HAK"},
        {"河池", "HCJ"}, {"邯郸", "HDG"}, {"黑河", "HEK"}, {"呼和浩特", "HET"},
        {"合肥", "HFE"}, {"淮安", "HIA"}, {"怀化", "HJJ"}, {"海拉尔", "HLD"},
        {"哈密", "HMI"}, {"衡阳", "HNY"}, {"哈尔滨", "HRB"}, {"和田", "HTN"},
        {"花土沟", "HTT"}, {"中国花莲", "HUN"}, {"霍林郭勒", "HUO"}, {"惠州", "HUZ"},
        {"汉中", "HZG"}, {"黄山", "TXN"}, {"呼伦贝尔", "XRQ"}, {"中国嘉义", "CYI"},
        {"景德镇", "JDZ"}, {"加格达奇", "JGD"}, {"嘉峪关", "JGN"}, {"井冈山", "JGS"},
        {"金昌", "JIC"}, {"九江", "JIU"}, {"荆门", "JM1"}, {"佳木斯", "JMU"},
        {"济宁", "JNG"}, {"锦州", "JNZ"}, {"建三江", "JSJ"}, {"鸡西", "JXA"},
        {"九寨沟", "JZH"}, {"中国金门", "KNH"}, {"揭阳", "SWA"}, {"库车", "KCA"},
        {"康定", "KGT"}, {"喀什", "KHG"}, {"凯里", "KJH"}, {"库尔勒", "KRL"},
        {"克拉玛依", "KRY"}, {"黎平", "HZH"}, {"澜沧", "JMJ"}, {"龙岩", "LCX"},
        {"临汾", "LFQ"}, {"兰州", "LHW"}, {"丽江", "LJG"}, {"荔波", "LLB"},
        {"吕梁", "LLV"}, {"临沧", "LNJ"}, {"陇南", "LNL"}, {"六盘水", "LPF"},
        {"拉萨", "LXA"}, {"洛阳", "LYA"}, {"连云港", "LYG"}, {"临沂", "LYI"},
        {"柳州", "LZH"}, {"泸州", "LZO"}, {"林芝", "LZY"}, {"芒市", "LUM"},
        {"牡丹江", "MDG"}, {"中国马祖", "MFK"}, {"绵阳", "MIG"}, {"梅州", "MXZ"},
        {"中国马公", "MZG"}, {"满洲里", "NZH"}, {"漠河", "OHE"}, {"南昌", "KHN"},
        {"中国南竿", "LZN"}, {"南充", "NAO"}, {"宁波", "NGB"}, {"宁蒗", "NLH"},
        {"南宁", "NNG"}, {"南阳", "NNY"}, {"南通", "NTG"}, {"攀枝花", "PZI"},
        {"普洱", "SYM"}, {"琼海", "BAR"}, {"秦皇岛", "BPE"}, {"祁连", "HBQ"},
        {"且末", "IQM"}, {"庆阳", "IQN"}, {"黔江", "JIQ"}, {"泉州", "JJN"},
        {"衢州", "JUZ"}, {"齐齐哈尔", "NDG"}, {"日照", "RIZ"}, {"日喀则", "RKZ"},
        {"若羌", "RQA"}, {"神农架", "HPG"}, {"莎车", "QSZ"}, {"沈阳", "SHE"},
        {"石河子", "SHF"}, {"石家庄", "SJW"}, {"上饶", "SQD"}, {"三明", "SQJ"},
        {"十堰", "WDS"}, {"邵阳", "WGN"}, {"松原", "YSQ"}, {"台州", "HYN"},
        {"中国台中", "RMQ"}, {"塔城", "TCG"}, {"腾冲", "TCZ"}, {"铜仁", "TEN"},
        {"通辽", "TGO"}, {"天水", "THQ"}, {"吐鲁番", "TLQ"}, {"通化", "TNH"},
        {"中国台南", "TNN"}, {"中国台北", "TPE"}, {"中国台东", "TTT"}, {"唐山", "TVS"},
        {"太原", "TYN"}, {"五大连池", "DTU"}, {"乌兰浩特", "HLH"}, {"乌兰察布", "UCB"},
        {"乌鲁木齐", "URC"}, {"潍坊", "WEF"}, {"威海", "WEH"}, {"文山", "WNH"},
        {"温州", "WNZ"}, {"乌海", "WUA"}, {"武夷山", "WUS"}, {"无锡", "WUX"},
        {"梧州", "WUZ"}, {"万州", "WXN"}, {"乌拉特中旗", "WZQ"}, {"巫山", "WSK"},
        {"兴义", "ACX"}, {"夏河", "GXH"}, {"中国香港", "HKG"}, {"西双版纳", "JHG"},
        {"新源", "NLT"}, {"忻州", "WUT"}, {"信阳", "XAI"}, {"襄阳", "XFN"},
        {"西昌", "XIC"}, {"锡林浩特", "XIL"}, {"西宁", "XNN"}, {"徐州", "XUZ"},
        {"延安", "ENY"}, {"银川", "INC"}, {"伊春", "LDS"}, {"永州", "LLF"},
        {"榆林", "UYN"}, {"宜宾", "YBP"}, {"运城", "YCU"}, {"宜春", "YIC"},
        {"宜昌", "YIH"}, {"伊宁", "YIN"}, {"义乌", "YIW"}, {"营口", "YKH"},
        {"延吉", "YNJ"}, {"烟台", "YNT"}, {"盐城", "YNZ"}, {"扬州", "YTY"},
        {"玉树", "YUS"}, {"岳阳", "YYA"}, {"张家界", "DYG"}, {"舟山", "HSN"},
        {"扎兰屯", "NZL"}, {"张掖", "YZY"}, {"昭通", "ZAT"}, {"湛江", "ZHA"},
        {"中卫", "ZHY"}, {"张家口", "ZQZ"}, {"珠海", "ZUH"}, {"遵义", "ZYI"}
    };
    
    for(auto it = cityCodes.begin(); it != cityCodes.end(); ++it) {
        fromCityCombo->addItem(it.key(), it.value());
        toCityCombo->addItem(it.key(), it.value());
    }
}

void Widget::onSearchClicked()
{
    QString fromCity = fromCityCombo->currentText();
    QString toCity = toCityCombo->currentText();
    bool direct = !allowTransferCheck->isChecked();
    
    // Get date from the hidden label instead of dateEdit
    QDate departDate = QDate::fromString(selectedDateValue->text(), "yyyy-MM-dd");
    
    if(fromCity == toCity) {
        QMessageBox::warning(this, "警告", "出发城市和到达城市不能相同！");
        return;
    }
    
    QString fromCode = getCityCode(fromCity);
    QString toCode = getCityCode(toCity);
    
    // 确保日期有效
    if (!departDate.isValid()) {
        departDate = QDate::currentDate();
        selectedDateValue->setText(departDate.toString("yyyy-MM-dd"));
        calendarButton->setText(departDate.toString("yyyy年MM月dd日"));
    }
    
    // 根据选择的交通方式进行不同的搜索
    switch(transportTypeGroup->checkedId()) {
    case 0: // 飞机
        searchFlightPrice(fromCode, toCode, direct, departDate);
        break;
    case 1: // 火车
        searchTrainPrice(fromCode, toCode, departDate);
        break;
    case 2: // 综合查询
        // 创建一个对话框来展示综合查询结果
        QDialog *combinedResultsDialog = new QDialog(this);
        combinedResultsDialog->setWindowTitle("综合查询结果");
        combinedResultsDialog->setMinimumSize(800, 600);
        
        // 创建布局
        QVBoxLayout *dialogLayout = new QVBoxLayout(combinedResultsDialog);
        
        // 标题标签
        QLabel *titleLabel = new QLabel(QString("航班与列车综合查询结果 - %1").arg(departDate.toString("yyyy年MM月dd日")), combinedResultsDialog);
        titleLabel->setStyleSheet("font-size: 16pt; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
        titleLabel->setAlignment(Qt::AlignCenter);
        dialogLayout->addWidget(titleLabel);
        
        // 创建分隔线
        QFrame *line = new QFrame(combinedResultsDialog);
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        dialogLayout->addWidget(line);
        
        // 创建两个文本编辑框，分别显示航班和火车信息
        QTextEdit *flightResultText = new QTextEdit(combinedResultsDialog);
        flightResultText->setReadOnly(true);
        flightResultText->setStyleSheet("background-color: #f8f9fa; border: 1px solid #e9ecef;");
        
        QTextEdit *trainResultText = new QTextEdit(combinedResultsDialog);
        trainResultText->setReadOnly(true);
        trainResultText->setStyleSheet("background-color: #f8f9fa; border: 1px solid #e9ecef;");
        
        // 创建标签
        QLabel *flightLabel = new QLabel("航班信息", combinedResultsDialog);
        flightLabel->setStyleSheet("font-size: 14pt; font-weight: bold; color: #3498db;");
        
        QLabel *trainLabel = new QLabel("火车信息", combinedResultsDialog);
        trainLabel->setStyleSheet("font-size: 14pt; font-weight: bold; color: #e74c3c;");
        
        // 添加到布局
        dialogLayout->addWidget(flightLabel);
        dialogLayout->addWidget(flightResultText);
        
        QFrame *line2 = new QFrame(combinedResultsDialog);
        line2->setFrameShape(QFrame::HLine);
        line2->setFrameShadow(QFrame::Sunken);
        dialogLayout->addWidget(line2);
        
        dialogLayout->addWidget(trainLabel);
        dialogLayout->addWidget(trainResultText);
        
        // 显示对话框
        combinedResultsDialog->show();
        
        // 保存原始的resultText，以便稍后恢复
        QTextEdit *originalResultText = resultText;
        
        // 先查询航班
        resultText = flightResultText;
        searchFlightPrice(fromCode, toCode, direct, departDate);
        
        // 然后查询火车
        resultText = trainResultText;
        searchTrainPrice(fromCode, toCode, departDate);
        
        // 恢复原始的resultText
        resultText = originalResultText;
        resultText->setText(QString("<html><body style='font-family: \"SF Pro Text\", -apple-system, BlinkMacSystemFont, sans-serif; color: #1d1d1f;'>"
                                  "<h3 style='font-size: 18px; font-weight: 600;'>综合查询已在新窗口中显示</h3>"
                                  "<p>查询日期: %1</p>"
                                  "<p>出发城市: %2</p>"
                                  "<p>到达城市: %3</p>"
                                  "</body></html>")
                          .arg(departDate.toString("yyyy年MM月dd日"))
                          .arg(fromCity)
                          .arg(toCity));
        break;
    }
}

void Widget::searchFlightPrice(const QString &fromCode, const QString &toCode, bool direct, const QDate &date)
{
    // 使用标准方法
    if (false) {
        // 使用携程API查询最低价格，根据用户提供的格式构建URL
        QString url = QString("https://flights.ctrip.com/itinerary/api/12808/lowestPrice?"
                              "flightWay=Oneway&dcity=%1&acity=%2&direct=%3&army=false")
                              .arg(fromCode)
                              .arg(toCode)
                              .arg(direct ? "true" : "false");
        
        qDebug() << "Flight Request URL:" << url;
        
        // 创建请求对象
        QUrl qurl(url);
        QNetworkRequest request(qurl);
        
        // 添加必要的请求头
        request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36");
        request.setRawHeader("Accept", "application/json, text/plain, */*");
        request.setRawHeader("Accept-Language", "zh-CN,zh;q=0.9");
        request.setRawHeader("Origin", "https://flights.ctrip.com");
        request.setRawHeader("Referer", "https://flights.ctrip.com/");
        
        // 配置SSL/TLS设置
        QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
        sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
        sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone); // 临时禁用SSL验证用于测试
        request.setSslConfiguration(sslConfig);
        
        // 显示正在查询的信息
        resultText->setText("正在查询航班信息，请稍候...");
        
        // 使用GET请求访问API
        networkManager->get(request);
    }
    else {
        // 清空之前可能的输出结果
        curlOutput.clear();
        
        // 如果有之前的进程，先清理
        if (curlProcess) {
            if (curlProcess->state() == QProcess::Running) {
                curlProcess->terminate();
                curlProcess->waitForFinished(1000);
            }
            delete curlProcess;
        }
        
        // 创建新的进程
        curlProcess = new QProcess(this);
        
        // 将日期格式化为API需要的格式 (YYYYMMDD)
        QString dateStr = date.toString("yyyyMMdd");
        
        // 尝试使用系统命令(curl)来测试API，添加日期参数
        QString url = QString("https://flights.ctrip.com/itinerary/api/12808/lowestPrice?"
                             "flightWay=Oneway&dcity=%1&acity=%2&direct=%3&army=false&date=%4")
                             .arg(fromCode)
                             .arg(toCode)
                             .arg(direct ? "true" : "false")
                             .arg(dateStr);
        
        resultText->setText(QString("使用命令行工具尝试访问API，这可能绕过TLS问题...\n查询日期: %1\n%2")
                          .arg(date.toString("yyyy-MM-dd"))
                          .arg(url));
        
        // 创建curl命令
        QString program = "curl";
        QStringList arguments;
        arguments << "-s" // 安静模式，不显示进度
                  << "-H" << "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36"
                  << "-H" << "Accept: application/json, text/plain, */*"
                  << "-H" << "Origin: https://flights.ctrip.com"
                  << "-H" << "Referer: https://flights.ctrip.com/"
                  << url;
        
        // 连接信号槽
        connect(curlProcess, &QProcess::readyReadStandardOutput, this, &Widget::onCurlStandardOutput);
        connect(curlProcess, &QProcess::readyReadStandardError, this, &Widget::onCurlStandardError);
        connect(curlProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, &Widget::onCurlFinished);
        
        // 启动进程
        curlProcess->start(program, arguments);
        if (!curlProcess->waitForStarted(3000)) { // 3秒超时
            resultText->setText(resultText->toPlainText() + "\n\n无法启动curl命令。请确保curl已安装。");
        }
    }
}

void Widget::onCurlStandardOutput()
{
    QByteArray data = curlProcess->readAllStandardOutput();
    curlOutput.append(data);
    qDebug() << "收到部分curl输出，大小:" << data.size();
}

void Widget::onCurlStandardError()
{
    QString error = QString::fromUtf8(curlProcess->readAllStandardError());
    qDebug() << "Curl error:" << error;
    resultText->setText(resultText->toPlainText() + "\n\n访问API出错，错误信息:\n" + error);
}

void Widget::onCurlFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitCode != 0) {
        resultText->setText(resultText->toPlainText() + QString("\n\n命令执行失败，退出代码: %1").arg(exitCode));
    } else {
        // 进程完成后，处理完整的输出
        QString result = QString::fromUtf8(curlOutput);
        qDebug() << "完整的curl输出:" << result;
        
        // 显示原始结果，方便调试
        QString previewText = result;
        if (previewText.length() > 500) {
            previewText = previewText.left(500) + "...";
        }
        resultText->setText(resultText->toPlainText() + "\n\n原始API响应:\n" + previewText);
        
        // 解析返回的结果
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(curlOutput, &parseError);
        if (doc.isNull()) {
            resultText->setText(resultText->toPlainText() + 
                               "\n\n无法解析API结果，错误: " + parseError.errorString() + 
                               "\n位置: " + QString::number(parseError.offset));
        } else {
            resultText->setText(resultText->toPlainText() + "\n\nAPI调用成功，正在处理数据...");
            
            // 使用parseFlightData处理返回数据
            QJsonObject obj = doc.object();
            parseFlightData(obj);
        }
    }
}

void Widget::parseFlightData(const QJsonObject &obj)
{
    qDebug() << "开始解析航班数据...";
    qDebug() << "顶层JSON结构:" << obj.keys();
    
    // 获取当前选择的日期
    QDate selectedDate = QDate::fromString(selectedDateValue->text(), "yyyy-MM-dd");
    
    // 在显示数据之前，确保详情按钮可见
    detailButton->setVisible(true);
    detailButton->setEnabled(false);  // 先设置为禁用状态，等有数据时再启用
    
    // 显示解析结果
    QString resultStr = "<html><body style='font-family: \"SF Pro Text\", -apple-system, BlinkMacSystemFont, sans-serif; color: #1d1d1f; line-height: 1.5; margin: 0; padding: 0;'>";
    
    // 添加查询日期信息
    resultStr += QString("<div style='background-color: #e8f4fd; padding: 16px; border-radius: 12px; margin-bottom: 20px;'>"
                       "<h3 style='font-size: 18px; font-weight: 600; margin: 0; color: #0071e3;'>查询日期: %1</h3>"
                       "</div>")
                .arg(selectedDate.toString("yyyy年MM月dd日"));
    
    // 首先检查是否有错误信息
    if (obj.contains("status") && obj["status"].toInt() != 0) {
        int status = obj["status"].toInt();
        QString message = obj.contains("msg") ? obj["msg"].toString() : "未知错误";
        
        resultStr += QString("<div style='background-color: #fff2f2; padding: 20px; border-radius: 12px; margin-bottom: 24px;'>"
                           "<h3 style='color: #ff453a; margin-top: 0; font-weight: 600;'>请求错误</h3>"
                           "<p style='margin: 8px 0;'>状态码: %1</p>"
                           "<p style='margin: 8px 0;'>错误信息: %2</p>"
                           "</div>").arg(status).arg(message);
        
        resultStr += "</body></html>";
        resultText->setHtml(resultStr);
        return;
    }
    
    // 检查数据是否存在
    if (obj.contains("data") && obj["data"].isObject()) {
        QJsonObject dataObj = obj["data"].toObject();
        qDebug() << "data对象的键:" << dataObj.keys();
        
        // 处理单程价格
        if (dataObj.contains("oneWayPrice") && dataObj["oneWayPrice"].isArray()) {
            QJsonArray priceArray = dataObj["oneWayPrice"].toArray();
            
            if (priceArray.size() > 0 && priceArray[0].isObject()) {
                QJsonObject priceObj = priceArray[0].toObject();
                qDebug() << "oneWayPrice中第一个对象的键数量:" << priceObj.keys().size();
                
                // 将日期-价格数据转换为列表
                QMap<QString, int> datePriceMap;
                int lowestPrice = INT_MAX;
                QString lowestPriceDate;
                
                // 遍历所有日期和价格
                for (auto it = priceObj.begin(); it != priceObj.end(); ++it) {
                    QString date = it.key();
                    int price = it.value().toInt();
                    datePriceMap[date] = price;
                    
                    // 记录最低价格
                    if (price < lowestPrice) {
                        lowestPrice = price;
                        lowestPriceDate = date;
                    }
                }
                
                // 显示最低价格信息
                if (lowestPrice < INT_MAX) {
                    // 添加标题
                    resultStr += QString("<h2 style='font-size: 28px; font-weight: 700; margin-bottom: 24px; color: #1d1d1f;'>%1 至 %2 的航班</h2>")
                                .arg(fromCityCombo->currentText())
                                .arg(toCityCombo->currentText());
                    
                    // 最低价格卡片
                    resultStr += QString("<div style='background-color: #f5f5f7; padding: 24px; border-radius: 16px; margin-bottom: 32px;'>"
                                       "<div style='display: flex; justify-content: space-between; align-items: center;'>"
                                       "<div>"
                                       "<h3 style='font-size: 20px; font-weight: 600; margin: 0 0 8px 0; color: #1d1d1f;'>最低票价</h3>"
                                       "<p style='font-size: 16px; margin: 0; color: #86868b;'>%2</p>"
                                       "</div>"
                                       "<div style='text-align: right;'>"
                                       "<p style='font-size: 36px; font-weight: 700; margin: 0; color: #0071e3;'>¥%1</p>"
                                       "</div>"
                                       "</div>"
                                       "</div>")
                                .arg(lowestPrice)
                                .arg(formatDate(lowestPriceDate));
                    
                    // 关于中转站的说明
                    resultStr += "<div style='background-color: #fff2cc; padding: 16px; border-radius: 16px; margin-bottom: 32px;'>";
                    resultStr += "<p style='margin: 0; font-size: 16px; color: #1d1d1f;'><span style='font-weight: 600;'>注意:</span> 最低价格API不提供中转站信息。要获取包含中转站的详细信息，请点击下方的「查看详细」按钮。</p>";
                    resultStr += "</div>";
                    
                    // 存储最低价格日期，供详细查询使用
                    lowestPriceFlightDate = selectedDate.toString("yyyyMMdd");
                    
                    // 显示详细按钮并启用
                    detailButton->setEnabled(true);
                }
                
                // 显示价格列表
                resultStr += "<h3 style='font-size: 22px; font-weight: 600; margin: 32px 0 16px 0; color: #1d1d1f;'>票价日历</h3>";
                
                // 表格容器
                resultStr += "<div style='background-color: white; border-radius: 16px; overflow: hidden; box-shadow: 0 2px 8px rgba(0,0,0,0.05);'>";
                resultStr += "<table style='width: 100%; border-collapse: collapse; font-size: 16px;'>";
                resultStr += "<thead>"
                           "<tr style='background-color: #f5f5f7;'>"
                           "<th style='padding: 16px; text-align: left; font-weight: 600; color: #1d1d1f; border-bottom: 1px solid #e5e5e5;'>日期</th>"
                           "<th style='padding: 16px; text-align: right; font-weight: 600; color: #1d1d1f; border-bottom: 1px solid #e5e5e5;'>价格 (CNY)</th>"
                           "</tr>"
                           "</thead>";
                resultStr += "<tbody>";
                
                // 排序日期
                QStringList dates = datePriceMap.keys();
                std::sort(dates.begin(), dates.end());
                
                for (const QString &date : dates) {
                    QString rowStyle = "";
                    QString priceStyle = "";
                    
                    if (date == lowestPriceDate) {
                        rowStyle = "background-color: #f2f9ff;";
                        priceStyle = "color: #0071e3; font-weight: 600;";
                    }
                    
                    resultStr += QString("<tr style='%1'>"
                                       "<td style='padding: 16px; border-bottom: 1px solid #e5e5e5;'>%2</td>"
                                       "<td style='padding: 16px; border-bottom: 1px solid #e5e5e5; text-align: right; %3'>¥%4</td>"
                                       "</tr>")
                                .arg(rowStyle)
                                .arg(formatDate(date))
                                .arg(priceStyle)
                                .arg(datePriceMap[date]);
                }
                
                resultStr += "</tbody>";
                resultStr += "</table>";
                resultStr += "</div>";
                
            } else {
                resultStr += "<div style='background-color: #fff2f2; padding: 20px; border-radius: 12px;'>"
                           "<h3 style='color: #ff453a; margin-top: 0; font-weight: 600;'>数据格式错误</h3>"
                           "<p style='margin: 8px 0;'>无法解析价格数据格式</p>"
                           "</div>";
            }
        } else {
            resultStr += "<div style='background-color: #fff2f2; padding: 20px; border-radius: 12px;'>"
                       "<h3 style='color: #ff453a; margin-top: 0; font-weight: 600;'>数据缺失</h3>"
                       "<p style='margin: 8px 0;'>未找到价格数据</p>"
                       "</div>";
        }
    } else {
        resultStr += "<div style='background-color: #fff2f2; padding: 20px; border-radius: 12px;'>"
                   "<h3 style='color: #ff453a; margin-top: 0; font-weight: 600;'>数据结构错误</h3>"
                   "<p style='margin: 8px 0;'>API返回数据中没有找到data字段</p>"
                   "</div>";
    }
    
    resultStr += "</body></html>";
    resultText->setHtml(resultStr);
}

void Widget::onDetailButtonClicked()
{
    // 根据当前选择的交通方式决定跳转到哪个网站
    switch(transportTypeGroup->checkedId()) {
    case 0: // 飞机 - 跳转到携程
        openFlightDetails();
        break;
    case 1: // 火车 - 跳转到12306
        openTrainDetails();
        break;
    case 2: // 综合查询 - 打开两个网站
        openFlightDetails();
        openTrainDetails();
        break;
    }
}

// 打开航班详情
void Widget::openFlightDetails()
{
    // 获取当前选择的日期
    QDate selectedDate = QDate::fromString(selectedDateValue->text(), "yyyy-MM-dd");
    
    QString fromCode = fromCityCombo->currentData().toString();
    QString toCode = toCityCombo->currentData().toString();
    
    // 创建加载对话框
    QDialog *loadingDialog = new QDialog(this, Qt::Dialog | Qt::FramelessWindowHint);
    loadingDialog->setStyleSheet(
        "QDialog { background-color: rgba(255, 255, 255, 0.9); border-radius: 10px; }"
    );
    
    QVBoxLayout *dialogLayout = new QVBoxLayout(loadingDialog);
    
    QLabel *loadingIcon = new QLabel(loadingDialog);
    loadingIcon->setAlignment(Qt::AlignCenter);
    loadingIcon->setPixmap(QPixmap(":/icons/loading.gif").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    
    QLabel *loadingLabel = new QLabel("正在打开携程航班信息...", loadingDialog);
    loadingLabel->setAlignment(Qt::AlignCenter);
    loadingLabel->setStyleSheet("font-size: 14px; color: #333; font-weight: bold; margin: 10px;");
    
    dialogLayout->addWidget(loadingIcon);
    dialogLayout->addWidget(loadingLabel);
    
    // 设置对话框大小和位置
    loadingDialog->setFixedSize(250, 150);
    loadingDialog->move(this->geometry().center() - loadingDialog->rect().center());
    
    // 显示加载对话框
    loadingDialog->show();
    
    // 使用计时器延迟调用浏览器
    QTimer::singleShot(800, [=]() {
        // 构建详细信息URL，使用当前选择的日期
        QString dateFormatted = selectedDate.toString("yyyy-MM-dd");
        
        // 构建携程网站URL
        QString url = QString("https://flights.ctrip.com/online/list/oneway-%1-%2?depdate=%3")
                             .arg(fromCode.toLower())
                             .arg(toCode.toLower())
                             .arg(dateFormatted);
        
        // 不修改现有内容，而是创建一个小提示窗口
        QMessageBox::information(this, "提示", QString("正在打开携程网站查看 %1 的航班详情...").arg(dateFormatted));
        
        // 使用默认浏览器打开URL
        bool success = QDesktopServices::openUrl(QUrl(url));
        
        if (!success) {
            QMessageBox::warning(this, "错误", 
                                "无法打开浏览器。请手动访问以下链接查看详细航班信息:\n\n" + url);
    }
    
        // 关闭加载对话框
        loadingDialog->close();
        loadingDialog->deleteLater();
    });
}

// 打开火车票详情
void Widget::openTrainDetails()
{
    QString fromCity = fromCityCombo->currentText();
    QString toCity = toCityCombo->currentText();
    
    // 获取当前选择的日期
    QDate selectedDate = QDate::fromString(selectedDateValue->text(), "yyyy-MM-dd");
    
    // 将城市代码转换为车站代码
    QString fromCode = getCityCode(fromCity);
    QString toCode = getCityCode(toCity);
    
    QString fromStation = convertToTrainStation(fromCode);
    QString toStation = convertToTrainStation(toCode);
    
    if (fromStation.isEmpty() || toStation.isEmpty()) {
        QMessageBox::warning(this, "错误", "无法识别城市对应的火车站代码，请选择其他城市。");
        return;
    }
    
    // 创建加载对话框
    QDialog *loadingDialog = new QDialog(this, Qt::Dialog | Qt::FramelessWindowHint);
    loadingDialog->setStyleSheet(
        "QDialog { background-color: rgba(255, 255, 255, 0.9); border-radius: 10px; }"
    );
    
    QVBoxLayout *dialogLayout = new QVBoxLayout(loadingDialog);
    
    QLabel *loadingIcon = new QLabel(loadingDialog);
    loadingIcon->setAlignment(Qt::AlignCenter);
    loadingIcon->setPixmap(QPixmap(":/icons/loading.gif").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    
    QLabel *loadingLabel = new QLabel("正在打开12306列车信息...", loadingDialog);
    loadingLabel->setAlignment(Qt::AlignCenter);
    loadingLabel->setStyleSheet("font-size: 14px; color: #333; font-weight: bold; margin: 10px;");
    
    dialogLayout->addWidget(loadingIcon);
    dialogLayout->addWidget(loadingLabel);
    
    // 设置对话框大小和位置
    loadingDialog->setFixedSize(250, 150);
    loadingDialog->move(this->geometry().center() - loadingDialog->rect().center());
    
    // 显示加载对话框
    loadingDialog->show();
    
    // 使用计时器延迟调用浏览器
    QTimer::singleShot(800, [=]() {
        // 构建12306 URL
        QString dateFormatted = selectedDate.toString("yyyy-MM-dd");
    
        // 构建12306网站URL
        QString url = QString("https://kyfw.12306.cn/otn/leftTicket/init?linktypeid=dc&fs=%1,%2&ts=%3,%4&date=%5&flag=N,N,Y")
                             .arg(fromCity)
                             .arg(fromStation)
                             .arg(toCity)
                             .arg(toStation)
                         .arg(dateFormatted);
                         
        // 不修改现有内容，而是创建一个小提示窗口
        QMessageBox::information(this, "提示", QString("正在打开12306网站查看 %1 的列车详情...").arg(dateFormatted));
    
    // 使用默认浏览器打开URL
    bool success = QDesktopServices::openUrl(QUrl(url));
    
    if (!success) {
        QMessageBox::warning(this, "错误", 
                                "无法打开浏览器。请手动访问以下链接查看详细列车信息:\n\n" + url);
    }
        
        // 关闭加载对话框
        loadingDialog->close();
        loadingDialog->deleteLater();
    });
}

void Widget::searchFlightDetails(const QString &fromCode, const QString &toCode, const QString &dateStr)
{
    // 保存最低价格日期以便详情按钮使用
    lowestPriceFlightDate = dateStr;
    
    // 启用详情按钮
    detailButton->setVisible(true);
    detailButton->setEnabled(true);
    
    // 根据当前选择的交通方式设置按钮样式
    onTransportTypeChanged(transportTypeGroup->checkedId());
}

QString Widget::formatDate(const QString &dateStr)
{
    // 将"20250523"格式转换为"2025-05-23"
    if (dateStr.length() == 8) {
        return QString("%1-%2-%3")
              .arg(dateStr.mid(0, 4))
              .arg(dateStr.mid(4, 2)) 
              .arg(dateStr.mid(6, 2));
    }
    return dateStr;
}

void Widget::searchTrainPrice(const QString &fromCode, const QString &toCode, const QDate &date)
{
    // 使用12306 API查询火车票价格
    QString dateStr = date.toString("yyyy-MM-dd");
    
    // 将城市代码转换为车站代码 (城市代码与车站代码格式不同)
    QString fromStation = convertToTrainStation(fromCode);
    QString toStation = convertToTrainStation(toCode);
    
    if (fromStation.isEmpty() || toStation.isEmpty()) {
        resultText->setText("无法识别城市对应的火车站代码，请选择其他城市。");
        return;
    }
    
    // 清空之前可能的输出结果
    curlOutput.clear();
    
    // 如果有之前的进程，先清理
    if (curlProcess) {
        if (curlProcess->state() == QProcess::Running) {
            curlProcess->terminate();
            curlProcess->waitForFinished(1000);
        }
        delete curlProcess;
    }
    
    // 创建新的进程
    curlProcess = new QProcess(this);
    
    // 创建12306 API URL
    QString url = QString("https://kyfw.12306.cn/otn/leftTicketPrice/query?leftTicketDTO.train_date=%1&leftTicketDTO.from_station=%2&leftTicketDTO.to_station=%3&leftTicketDTO.ticket_type=1&randCode=stzh")
                         .arg(date.toString("yyyy-MM-dd"))
                         .arg(fromStation)
                         .arg(toStation);
    
    resultText->setText("正在查询火车票价格，请稍候...\n" + url);
    
    // 创建curl命令
    QString program = "curl";
    QStringList arguments;
    arguments << "-s" // 安静模式，不显示进度
              << "-H" << "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36"
              << "-H" << "Accept: application/json, text/plain, */*"
              << "-H" << "Origin: https://kyfw.12306.cn"
              << "-H" << "Referer: https://kyfw.12306.cn/"
              << url;
    
    // 连接信号槽
    connect(curlProcess, &QProcess::readyReadStandardOutput, this, &Widget::onTrainCurlStandardOutput);
    connect(curlProcess, &QProcess::readyReadStandardError, this, &Widget::onTrainCurlStandardError);
    connect(curlProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &Widget::onTrainCurlFinished);
    
    // 启动进程
    curlProcess->start(program, arguments);
    if (!curlProcess->waitForStarted(3000)) { // 3秒超时
        resultText->setText(resultText->toPlainText() + "\n\n无法启动curl命令。请确保curl已安装。");
    }
}

void Widget::onTrainCurlStandardOutput()
{
    QByteArray data = curlProcess->readAllStandardOutput();
    curlOutput.append(data);
    qDebug() << "收到部分铁路查询输出，大小:" << data.size();
}

void Widget::onTrainCurlStandardError()
{
    QString error = QString::fromUtf8(curlProcess->readAllStandardError());
    qDebug() << "Train API error:" << error;
    resultText->setText(resultText->toPlainText() + "\n\n访问铁路API出错，错误信息:\n" + error);
}

void Widget::onTrainCurlFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitCode != 0) {
        resultText->setText(resultText->toPlainText() + QString("\n\n命令执行失败，退出代码: %1").arg(exitCode));
    } else {
        // 进程完成后，处理完整的输出
        QString result = QString::fromUtf8(curlOutput);
        qDebug() << "完整的火车票查询输出:" << result;
        
        // 显示原始结果，方便调试
        QString previewText = result;
        if (previewText.length() > 500) {
            previewText = previewText.left(500) + "...";
        }
        resultText->setText(resultText->toPlainText() + "\n\n原始火车票API响应:\n" + previewText);
        
        // 解析返回的结果
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(curlOutput, &parseError);
        if (doc.isNull()) {
            resultText->setText(resultText->toPlainText() + 
                               "\n\n无法解析铁路API结果，错误: " + parseError.errorString() + 
                               "\n位置: " + QString::number(parseError.offset));
        } else {
            resultText->setText(resultText->toPlainText() + "\n\n铁路API调用成功，正在处理数据...");
            
            // 解析火车票数据
            QJsonObject obj = doc.object();
            parseTrainData(obj);
        }
    }
}

void Widget::parseTrainData(const QJsonObject &obj)
{
    qDebug() << "开始解析火车票数据...";
    qDebug() << "顶层JSON结构:" << obj.keys();
    
    // 获取当前选择的日期
    QDate selectedDate = QDate::fromString(selectedDateValue->text(), "yyyy-MM-dd");
    
    // 显示解析结果
    QString resultStr = "<html><body style='font-family: \"SF Pro Text\", -apple-system, BlinkMacSystemFont, sans-serif; color: #1d1d1f; line-height: 1.5; margin: 0; padding: 0;'>";
    
    // 添加标题
    resultStr += QString("<h2 style='font-size: 28px; font-weight: 700; margin-bottom: 24px; color: #1d1d1f;'>%1 至 %2 的列车</h2>")
                .arg(fromCityCombo->currentText())
                .arg(toCityCombo->currentText());
    
    // 添加查询日期信息
    resultStr += QString("<div style='background-color: #e8f4fd; padding: 16px; border-radius: 12px; margin-bottom: 20px;'>"
                       "<h3 style='font-size: 18px; font-weight: 600; margin: 0; color: #0071e3;'>查询日期: %1</h3>"
                       "</div>")
                .arg(selectedDate.toString("yyyy年MM月dd日"));
    
    // 检查返回状态
    if (obj.contains("status") && obj["status"].toBool() && obj.contains("data") && obj["data"].isArray()) {
        QJsonArray trainArray = obj["data"].toArray();
        
        if (trainArray.isEmpty()) {
            resultStr += "<div style='background-color: #f5f5f7; padding: 24px; border-radius: 16px; margin-bottom: 24px;'>"
                       "<h3 style='font-size: 20px; font-weight: 600; margin: 0 0 8px 0; color: #1d1d1f;'>没有找到列车</h3>"
                       "<p style='font-size: 16px; margin: 0; color: #86868b;'>在所选日期没有找到符合条件的列车。请尝试选择其他日期或路线。</p>"
                       "</div>";
        } else {
            // 启用详情按钮
            detailButton->setVisible(true);
            detailButton->setEnabled(true);
            
            // 创建表格显示列车信息
            resultStr += "<div style='background-color: white; border-radius: 16px; overflow: hidden; box-shadow: 0 2px 8px rgba(0,0,0,0.05); margin-bottom: 24px;'>";
            resultStr += "<table style='width: 100%; border-collapse: collapse; font-size: 16px;'>";
            resultStr += "<thead>"
                       "<tr style='background-color: #f5f5f7;'>"
                       "<th style='padding: 16px; text-align: left; font-weight: 600; color: #1d1d1f; border-bottom: 1px solid #e5e5e5;'>车次</th>"
                       "<th style='padding: 16px; text-align: left; font-weight: 600; color: #1d1d1f; border-bottom: 1px solid #e5e5e5;'>出发站</th>"
                       "<th style='padding: 16px; text-align: left; font-weight: 600; color: #1d1d1f; border-bottom: 1px solid #e5e5e5;'>到达站</th>"
                       "<th style='padding: 16px; text-align: left; font-weight: 600; color: #1d1d1f; border-bottom: 1px solid #e5e5e5;'>发车时间</th>"
                       "<th style='padding: 16px; text-align: left; font-weight: 600; color: #1d1d1f; border-bottom: 1px solid #e5e5e5;'>到达时间</th>"
                       "<th style='padding: 16px; text-align: left; font-weight: 600; color: #1d1d1f; border-bottom: 1px solid #e5e5e5;'>历时</th>"
                       "<th style='padding: 16px; text-align: left; font-weight: 600; color: #1d1d1f; border-bottom: 1px solid #e5e5e5;'>商务座/特等座</th>"
                       "<th style='padding: 16px; text-align: left; font-weight: 600; color: #1d1d1f; border-bottom: 1px solid #e5e5e5;'>一等座</th>"
                       "<th style='padding: 16px; text-align: left; font-weight: 600; color: #1d1d1f; border-bottom: 1px solid #e5e5e5;'>二等座</th>"
                       "</tr>"
                       "</thead>";
            resultStr += "<tbody>";
            
            for (int i = 0; i < trainArray.size(); ++i) {
                QJsonObject trainObj = trainArray[i].toObject();
                
                if (trainObj.contains("queryLeftNewDTO")) {
                    QJsonObject trainInfo = trainObj["queryLeftNewDTO"].toObject();
                    
                    QString trainCode = trainInfo["station_train_code"].toString();
                    QString fromStation = trainInfo["from_station_name"].toString();
                    QString toStation = trainInfo["to_station_name"].toString();
                    QString startTime = trainInfo["start_time"].toString();
                    QString arriveTime = trainInfo["arrive_time"].toString();
                    QString duration = trainInfo["lishi"].toString();
                    
                    // 高铁/动车特殊样式
                    bool isHighSpeedTrain = trainCode.startsWith("G") || trainCode.startsWith("D") || trainCode.startsWith("C");
                    QString trainCodeStyle = isHighSpeedTrain ? 
                                          "color: #0071e3; font-weight: 600;" : 
                                          "color: #1d1d1f;";
                    
                    // 价格信息
                    QString swzPrice = trainInfo["swz_price"].toString();
                    if (swzPrice == "--") swzPrice = "-";
                    else swzPrice = "¥" + formatPrice(swzPrice);
                    
                    QString zyPrice = trainInfo["zy_price"].toString();
                    if (zyPrice == "--") zyPrice = "-";
                    else zyPrice = "¥" + formatPrice(zyPrice);
                    
                    QString zePrice = trainInfo["ze_price"].toString();
                    if (zePrice == "--") zePrice = "-";
                    else zePrice = "¥" + formatPrice(zePrice);
                    
                    resultStr += QString("<tr>"
                                       "<td style='padding: 16px; border-bottom: 1px solid #e5e5e5; %1'>%2</td>"
                                       "<td style='padding: 16px; border-bottom: 1px solid #e5e5e5;'>%3</td>"
                                       "<td style='padding: 16px; border-bottom: 1px solid #e5e5e5;'>%4</td>"
                                       "<td style='padding: 16px; border-bottom: 1px solid #e5e5e5; font-weight: 600;'>%5</td>"
                                       "<td style='padding: 16px; border-bottom: 1px solid #e5e5e5;'>%6</td>"
                                       "<td style='padding: 16px; border-bottom: 1px solid #e5e5e5;'>%7</td>"
                                       "<td style='padding: 16px; border-bottom: 1px solid #e5e5e5; color: #ff453a;'>%8</td>"
                                       "<td style='padding: 16px; border-bottom: 1px solid #e5e5e5; color: #ff9f0a;'>%9</td>"
                                       "<td style='padding: 16px; border-bottom: 1px solid #e5e5e5; color: #30d158;'>%10</td>"
                                       "</tr>")
                                .arg(trainCodeStyle)
                                .arg(trainCode)
                                .arg(fromStation)
                                .arg(toStation)
                                .arg(startTime)
                                .arg(arriveTime)
                                .arg(duration)
                                .arg(swzPrice)
                                .arg(zyPrice)
                                .arg(zePrice);
                }
            }
            
            resultStr += "</tbody>";
            resultStr += "</table>";
            resultStr += "</div>";
            
            // 添加座位类型说明
            resultStr += "<div style='background-color: #f5f5f7; padding: 24px; border-radius: 16px; margin-bottom: 24px;'>";
            resultStr += "<h3 style='font-size: 20px; font-weight: 600; margin: 0 0 16px 0; color: #1d1d1f;'>座位类型说明</h3>";
            resultStr += "<div style='display: grid; grid-template-columns: repeat(3, 1fr); gap: 16px;'>";
            resultStr += "<div style='background-color: white; padding: 16px; border-radius: 12px;'>"
                       "<p style='margin: 0 0 8px 0; font-weight: 600; color: #ff453a;'>商务座/特等座 (swz)</p>"
                       "<p style='margin: 0; color: #86868b;'>高铁/动车高级座位，提供最佳乘坐体验</p>"
                       "</div>";
            resultStr += "<div style='background-color: white; padding: 16px; border-radius: 12px;'>"
                       "<p style='margin: 0 0 8px 0; font-weight: 600; color: #ff9f0a;'>一等座 (zy)</p>"
                       "<p style='margin: 0; color: #86868b;'>高铁/动车一等座位，舒适宽敞</p>"
                       "</div>";
            resultStr += "<div style='background-color: white; padding: 16px; border-radius: 12px;'>"
                       "<p style='margin: 0 0 8px 0; font-weight: 600; color: #30d158;'>二等座 (ze)</p>"
                       "<p style='margin: 0; color: #86868b;'>高铁/动车二等座位，经济实惠</p>"
                       "</div>";
            resultStr += "</div>";
            resultStr += "</div>";
            
            // 添加提示信息
            resultStr += "<div style='background-color: #fff2cc; padding: 16px; border-radius: 16px;'>";
            resultStr += "<p style='margin: 0; font-size: 16px; color: #1d1d1f;'><span style='font-weight: 600;'>提示:</span> 点击下方的「查看列车详情」按钮，前往12306官网查看更多座位类型和完整信息。</p>";
            resultStr += "</div>";
        }
    } else {
        resultStr += "<div style='background-color: #fff2f2; padding: 24px; border-radius: 16px;'>"
                   "<h3 style='font-size: 20px; font-weight: 600; margin: 0 0 8px 0; color: #ff453a;'>数据获取失败</h3>"
                   "<p style='font-size: 16px; margin: 0;'>无法获取列车信息，请稍后再试。</p>"
                   "</div>";
    }
    
    resultStr += "</body></html>";
    resultText->setHtml(resultStr);
}

QString Widget::formatPrice(const QString &priceStr)
{
    // 将"05970"格式转换为"597.0"
    if (priceStr.length() >= 3) {
        QString intPart = priceStr;
        intPart.remove(0, intPart.length() - 3); // 保留后3位
        QString decPart = priceStr.right(1);      // 最后1位作为小数
        return intPart + "." + decPart;
    }
    return priceStr;
}

QString Widget::convertToTrainStation(const QString &cityCode)
{
    // 将机场三字码转换为铁路站点代码
    QMap<QString, QString> codeMap = {
        {"BJS", "BJP"},  // 北京
        {"SHA", "SHH"},  // 上海
        {"CAN", "GZQ"},  // 广州
        {"SZX", "SZQ"},  // 深圳
        {"CTU", "CDW"},  // 成都
        {"HGH", "HZH"},  // 杭州
        {"WUH", "WHN"},  // 武汉
        {"SIA", "XAY"},  // 西安
        {"CKG", "CQW"},  // 重庆
        {"TAO", "QDK"},  // 青岛
        {"CSX", "CSQ"},  // 长沙
        {"NKG", "NJH"},  // 南京
        {"XMN", "XMS"},  // 厦门
        {"KMG", "KMM"},  // 昆明
        {"DLC", "DLT"},  // 大连
        {"TSN", "TJP"},  // 天津
    };
    
    // 如果cityCode是车站代码而不是机场三字码，直接返回
    if (cityCode.length() == 3 && !cityCode.toUpper().at(0).isLetter()) {
        return cityCode;
    }
    
    // 特殊情况处理
    if (cityCode == "BJP" || cityCode == "VNP") {
        return cityCode;
    }
    
    return codeMap.value(cityCode.toUpper(), "");
}

void Widget::onTransportTypeChanged(int id)
{
    // 根据不同的交通方式调整UI
    QString primaryBlue = "#0071e3";      // Apple 蓝色
    QString darkGray = "#1d1d1f";         // 深灰色
    QString red = "#ff453a";              // Apple 红色
    QString green = "#30d158";            // Apple 绿色
    
    switch(id) {
    case 0: // 飞机
        allowTransferCheck->setEnabled(true);
        detailButton->setText("查看航班详情");
        detailButton->setStyleSheet(QString(
            "QPushButton { background-color: %1; color: white; border: none; padding: 12px 20px; "
            "border-radius: 22px; font-weight: 600; font-size: 17px; } "
            "QPushButton:hover { background-color: %2; } "
            "QPushButton:pressed { background-color: %3; }"
        ).arg(primaryBlue));
        break;
    case 1: // 火车
        allowTransferCheck->setEnabled(false);
        detailButton->setText("查看列车详情");
        detailButton->setStyleSheet(QString(
            "QPushButton { background-color: %1; color: white; border: none; padding: 12px 20px; "
            "border-radius: 22px; font-weight: 600; font-size: 17px; } "
            "QPushButton:hover { background-color: %2; } "
            "QPushButton:pressed { background-color: %3; }"
        ).arg(red));
        break;
    case 2: // 综合查询
        allowTransferCheck->setEnabled(true);
        detailButton->setText("查看详情");
        detailButton->setStyleSheet(QString(
            "QPushButton { background-color: %1; color: white; border: none; padding: 12px 20px; "
            "border-radius: 22px; font-weight: 600; font-size: 17px; } "
            "QPushButton:hover { background-color: %2; } "
            "QPushButton:pressed { background-color: %3; }"
        ).arg(green));
        break;
    }
}

void Widget::onNetworkReply(QNetworkReply *reply)
{
    if(reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        qDebug() << "Response data:" << QString::fromUtf8(data);
        
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isNull()) {
            resultText->setText("JSON解析失败：" + QString::fromUtf8(data));
            return;
        }
        
        QJsonObject obj = doc.object();
        if (obj.contains("data") && obj["data"].isObject()) {
            QJsonObject dataObj = obj["data"].toObject();
            
            QString resultStr = "查询结果：\n";
            resultStr += "====================================\n";
            
            // 处理直飞价格
            if (dataObj.contains("direct") && dataObj["direct"].isObject()) {
                QJsonObject directObj = dataObj["direct"].toObject();
                QString lowestPrice = directObj.contains("lowestPrice") ? 
                                     QString::number(directObj["lowestPrice"].toDouble()) : "无数据";
                QString currency = directObj.contains("currency") ? 
                                  directObj["currency"].toString() : "CNY";
                
                resultStr += QString("【直飞】最低价格: %1 %2\n").arg(lowestPrice).arg(currency);
                
                // 提取直飞航线信息
                if (directObj.contains("price") && directObj["price"].isArray()) {
                    QJsonArray priceArray = directObj["price"].toArray();
                    resultStr += "\n直飞航线详情：\n";
                    
                    for (int i = 0; i < priceArray.size(); ++i) {
                        QJsonObject priceObj = priceArray[i].toObject();
                        QString date = priceObj.contains("date") ? priceObj["date"].toString() : "未知日期";
                        QString price = priceObj.contains("price") ? 
                                       QString::number(priceObj["price"].toDouble()) : "无价格";
                        
                        resultStr += QString("  日期: %1, 价格: %2 %3\n").arg(date).arg(price).arg(currency);
                    }
                }
            } else {
                resultStr += "【直飞】无直飞数据\n";
            }
            
            resultStr += "====================================\n";
            
            // 处理中转价格
            if (dataObj.contains("transfer") && dataObj["transfer"].isObject()) {
                QJsonObject transferObj = dataObj["transfer"].toObject();
                QString lowestPrice = transferObj.contains("lowestPrice") ? 
                                     QString::number(transferObj["lowestPrice"].toDouble()) : "无数据";
                QString currency = transferObj.contains("currency") ? 
                                  transferObj["currency"].toString() : "CNY";
                
                resultStr += QString("【中转】最低价格: %1 %2\n").arg(lowestPrice).arg(currency);
                
                // 提取中转航线信息
                if (transferObj.contains("price") && transferObj["price"].isArray()) {
                    QJsonArray priceArray = transferObj["price"].toArray();
                    resultStr += "\n中转航线详情：\n";
                    
                    for (int i = 0; i < priceArray.size(); ++i) {
                        QJsonObject priceObj = priceArray[i].toObject();
                        QString date = priceObj.contains("date") ? priceObj["date"].toString() : "未知日期";
                        QString price = priceObj.contains("price") ? 
                                       QString::number(priceObj["price"].toDouble()) : "无价格";
                        
                        resultStr += QString("  日期: %1, 价格: %2 %3\n").arg(date).arg(price).arg(currency);
                    }
                }
            } else {
                resultStr += "【中转】无中转数据\n";
            }
            
            resultText->setText(resultStr);
        } else {
            // 如果没有按照预期格式返回，显示原始JSON
            resultText->setText("无法解析响应数据：\n" + QString::fromUtf8(data));
        }
    } else {
        QString errorMsg = QString("查询失败：\n错误代码：%1\n错误信息：%2\nURL：%3")
            .arg(reply->error())
            .arg(reply->errorString())
            .arg(reply->url().toString());
        resultText->setText(errorMsg);
        qDebug() << errorMsg;
    }
    
    reply->deleteLater();
}

void Widget::onSslErrors(QNetworkReply *reply, const QList<QSslError> &errors)
{
    qDebug() << "SSL Errors:" << errors;
    
    // 临时忽略SSL错误，仅用于测试
    reply->ignoreSslErrors();
    
    // 在生产环境中应该检查错误类型并适当处理
    for (const QSslError &error : errors) {
        qDebug() << "SSL Error:" << error.errorString();
    }
}

QString Widget::getCityCode(const QString &cityName)
{
    return fromCityCombo->itemData(fromCityCombo->findText(cityName)).toString();
}

// 应用现代化、简约而霸气的UI设计
void Widget::applyModernUIStyle()
{
    // 设置窗口标题
    this->setWindowTitle("交通查询系统");
    
    // 应用苹果风格的设计
    QString primaryColor = "#0071e3";     // Apple 蓝色
    QString textColor = "#1d1d1f";        // 深灰色文本
    QString bgColor = "#ffffff";          // 白色背景
    QString secondaryBgColor = "#f5f5f7"; // 浅灰色背景
    QString secondaryTextColor = "#86868b"; // 次要文本颜色
    
    // 设置字体
    QFont titleFont("SF Pro Display", 40, QFont::Bold);
    QFont subtitleFont("SF Pro Display", 21);
    QFont headerFont("SF Pro Display", 20, QFont::DemiBold);
    QFont bodyFont("SF Pro Text", 17);
    QFont smallFont("SF Pro Text", 15);
    
    // 应用字体到控件
    if (QLabel *titleLabel = findChild<QLabel*>("titleLabel")) {
        titleLabel->setFont(titleFont);
    }
    
    if (QLabel *subtitleLabel = findChild<QLabel*>("subtitleLabel")) {
        subtitleLabel->setFont(subtitleFont);
    }
    
    // 设置按钮样式
    searchButton->setMinimumHeight(50);
    searchButton->setMinimumWidth(240);
    searchButton->setCursor(Qt::PointingHandCursor);
    
    // 设置详情按钮样式
    detailButton->setMinimumHeight(50);
    detailButton->setCursor(Qt::PointingHandCursor);
    
    // 设置下拉框样式
    fromCityCombo->setMinimumHeight(44);
    toCityCombo->setMinimumHeight(44);
    
    // 设置日期编辑器样式
    selectedDateValue->setMinimumHeight(44);
    
    // 设置结果显示区域样式
    resultText->setMinimumHeight(450);
    
    // 设置窗口大小
    this->setMinimumSize(1200, 900);
    this->resize(1200, 900);
}

void Widget::onCalendarButtonClicked()
{
    if (calendarWidget->isVisible()) {
        calendarWidget->hide();
    } else {
        // Position the calendar widget below the button
        QPoint globalPos = calendarButton->mapToGlobal(QPoint(0, calendarButton->height()));
        QPoint localPos = mapFromGlobal(globalPos);
        
        // Adjust position to ensure calendar is fully visible
        int calendarRight = localPos.x() + calendarWidget->width();
        int calendarBottom = localPos.y() + calendarWidget->height();
        
        if (calendarRight > width()) {
            localPos.setX(width() - calendarWidget->width());
        }
        
        if (calendarBottom > height()) {
            localPos.setY(localPos.y() - calendarWidget->height() - calendarButton->height());
        }
        
        calendarWidget->move(localPos);
        calendarWidget->show();
        calendarWidget->raise();
    }
}

void Widget::onCalendarDateSelected(const QDate &date)
{
    // Ensure the selected date is not in the past
    QDate currentDate = QDate::currentDate();
    QDate selectedDate = date;
    
    if (selectedDate < currentDate) {
        selectedDate = currentDate;
        calendarWidget->setSelectedDate(currentDate);
        QMessageBox::information(this, "日期提示", "不能选择过去的日期，已自动调整为今天。");
    }
    
    // Update the button text to show selected date
    calendarButton->setText(selectedDate.toString("yyyy年MM月dd日"));
    
    // Store the selected date in the hidden label
    selectedDateValue->setText(selectedDate.toString("yyyy-MM-dd"));
    
    // Hide the calendar widget
    calendarWidget->hide();
    
    // Show a message indicating that search is in progress
    resultText->setText(QString("<html><body style='font-family: \"SF Pro Text\", -apple-system, BlinkMacSystemFont, sans-serif; color: #1d1d1f;'>"
                              "<h3 style='font-size: 18px; font-weight: 600;'>正在查询 %1 的票价信息...</h3>"
                              "<p>已选择日期: %2</p>"
                              "<p>请稍候，正在获取最新数据...</p>"
                              "</body></html>")
                      .arg(selectedDate.toString("yyyy年MM月dd日"))
                      .arg(selectedDate.toString("yyyy-MM-dd")));
    
    // Trigger a search with the new date
    onSearchClicked();
}
