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

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    networkManager = new QNetworkAccessManager(this);
    
    setupUI();
    loadCityCodes();
    
    connect(searchButton, &QPushButton::clicked, this, &Widget::onSearchClicked);
    connect(networkManager, &QNetworkAccessManager::finished, this, &Widget::onNetworkReply);
    connect(networkManager, &QNetworkAccessManager::sslErrors, this, &Widget::onSslErrors);
    connect(transportTypeGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
            this, &Widget::onTransportTypeChanged);
            
    // 初始化curl进程
    curlProcess = nullptr;
    
    // 添加详细信息按钮（初始为隐藏状态）
    detailButton = new QPushButton("查看详细航班信息（含中转站）", this);
    connect(detailButton, &QPushButton::clicked, this, &Widget::onDetailButtonClicked);
    detailButton->setVisible(false);
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

void Widget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 创建交通方式选择区域
    QGroupBox *transportTypeBox = new QGroupBox("交通方式", this);
    QHBoxLayout *transportLayout = new QHBoxLayout(transportTypeBox);
    
    flightRadio = new QRadioButton("飞机", this);
    trainRadio = new QRadioButton("火车", this);
    bothRadio = new QRadioButton("综合查询", this);
    
    transportTypeGroup = new QButtonGroup(this);
    transportTypeGroup->addButton(flightRadio, 0);
    transportTypeGroup->addButton(trainRadio, 1);
    transportTypeGroup->addButton(bothRadio, 2);
    
    flightRadio->setChecked(true); // 默认选择飞机
    
    transportLayout->addWidget(flightRadio);
    transportLayout->addWidget(trainRadio);
    transportLayout->addWidget(bothRadio);
    
    // 创建城市选择区域
    QHBoxLayout *cityLayout = new QHBoxLayout();
    fromCityCombo = new QComboBox(this);
    toCityCombo = new QComboBox(this);
    cityLayout->addWidget(new QLabel("出发城市:", this));
    cityLayout->addWidget(fromCityCombo);
    cityLayout->addWidget(new QLabel("到达城市:", this));
    cityLayout->addWidget(toCityCombo);
    
    // 创建日期选择
    QHBoxLayout *dateLayout = new QHBoxLayout();
    dateEdit = new QDateEdit(QDate::currentDate().addDays(1), this);
    dateEdit->setCalendarPopup(true);
    dateEdit->setMinimumDate(QDate::currentDate());
    dateLayout->addWidget(new QLabel("出发日期:", this));
    dateLayout->addWidget(dateEdit);
    dateLayout->addStretch();
    
    // 创建选项区域
    QHBoxLayout *optionsLayout = new QHBoxLayout();
    allowTransferCheck = new QCheckBox("允许中转", this);
    optionsLayout->addWidget(allowTransferCheck);
    
    // 创建搜索按钮
    searchButton = new QPushButton("搜索", this);
    
    // 创建结果显示区域
    resultText = new QTextEdit(this);
    resultText->setReadOnly(true);
    
    // 添加所有组件到主布局
    mainLayout->addWidget(transportTypeBox);
    mainLayout->addLayout(cityLayout);
    mainLayout->addLayout(dateLayout);
    mainLayout->addLayout(optionsLayout);
    mainLayout->addWidget(searchButton);
    mainLayout->addWidget(resultText);
    mainLayout->addWidget(detailButton); // 添加详细信息按钮
    
    setLayout(mainLayout);
    setWindowTitle("交通票价查询");
    resize(800, 600);
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
    QDate departDate = dateEdit->date();
    
    if(fromCity == toCity) {
        QMessageBox::warning(this, "警告", "出发城市和到达城市不能相同！");
        return;
    }
    
    QString fromCode = getCityCode(fromCity);
    QString toCode = getCityCode(toCity);
    
    // 根据选择的交通方式进行不同的搜索
    switch(transportTypeGroup->checkedId()) {
    case 0: // 飞机
        searchFlightPrice(fromCode, toCode, direct, departDate);
        break;
    case 1: // 火车
        searchTrainPrice(fromCode, toCode, departDate);
        break;
    case 2: // 综合查询
        searchFlightPrice(fromCode, toCode, direct, departDate);
        // 待实现：火车票查询
        resultText->setText("正在查询机票价格，火车票API尚未实现...");
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
        
        // 尝试使用系统命令(curl)来测试API
        QString url = QString("https://flights.ctrip.com/itinerary/api/12808/lowestPrice?"
                             "flightWay=Oneway&dcity=%1&acity=%2&direct=%3&army=false")
                             .arg(fromCode)
                             .arg(toCode)
                             .arg(direct ? "true" : "false");
        
        resultText->setText("使用命令行工具尝试访问API，这可能绕过TLS问题...\n" + url);
        
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
    
    // 显示解析结果
    QString resultStr = "查询结果：\n";
    resultStr += "====================================\n";
    
    // 首先检查是否有错误信息
    if (obj.contains("status") && obj["status"].toInt() != 0) {
        int status = obj["status"].toInt();
        QString message = obj.contains("msg") ? obj["msg"].toString() : "未知错误";
        resultStr += QString("API返回错误: 状态码=%1, 信息=%2\n").arg(status).arg(message);
        resultText->setText(resultStr + "\n完整返回数据:\n" + QJsonDocument(obj).toJson(QJsonDocument::Indented));
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
                    resultStr += QString("最低票价: %1 CNY (日期: %2)\n")
                                .arg(lowestPrice)
                                .arg(formatDate(lowestPriceDate));
                    
                    // 关于中转站的说明
                    resultStr += "\n注意: 最低价格API不提供中转站信息。";
                    resultStr += "\n要获取包含中转站的详细信息，请点击下面的「查看详细」按钮。\n\n";
                    
                    // 存储最低价格日期，供详细查询使用
                    lowestPriceFlightDate = lowestPriceDate;
                }
                
                // 显示价格列表
                resultStr += "票价日历 (日期 - 价格):\n";
                resultStr += "-----------------------------------\n";
                
                // 排序日期
                QStringList dates = datePriceMap.keys();
                std::sort(dates.begin(), dates.end());
                
                // 每行显示3个日期-价格对
                int count = 0;
                for (const QString &date : dates) {
                    resultStr += QString("%1: %2元  ")
                               .arg(formatDate(date))
                               .arg(datePriceMap[date]);
                    
                    count++;
                    if (count % 3 == 0) {
                        resultStr += "\n";
                    }
                }
                
                if (count % 3 != 0) {
                    resultStr += "\n";
                }
                
                // 创建"查看详细"按钮
                if (!detailButton) {
                    detailButton = new QPushButton("查看详细航班信息（含中转站）", this);
                    connect(detailButton, &QPushButton::clicked, this, &Widget::onDetailButtonClicked);
                    
                    // 添加按钮到界面
                    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(this->layout());
                    if (layout) {
                        // 在结果文本框和搜索按钮之间添加
                        layout->insertWidget(layout->indexOf(resultText) + 1, detailButton);
                    }
                }
                
                // 显示按钮
                detailButton->setVisible(true);
                detailButton->setEnabled(true);
            } else {
                resultStr += "无法解析价格数据格式\n";
            }
        } else {
            resultStr += "未找到价格数据\n";
        }
    } else {
        resultStr += "API返回数据中没有找到data字段\n";
        resultText->setText(resultStr + "\n完整返回数据:\n" + QJsonDocument(obj).toJson(QJsonDocument::Indented));
        return;
    }
    
    resultText->setText(resultStr);
}

void Widget::onDetailButtonClicked()
{
    if (lowestPriceFlightDate.isEmpty()) {
        QMessageBox::information(this, "提示", "没有可查询的航班日期");
        return;
    }
    
    QString fromCode = fromCityCombo->currentData().toString();
    QString toCode = toCityCombo->currentData().toString();
    
    // 显示正在查询的信息
    resultText->setText(resultText->toPlainText() + "\n\n正在查询详细航班信息，包括中转站点...");
    
    // 查询特定日期的详细航班信息
    searchFlightDetails(fromCode, toCode, lowestPriceFlightDate);
}

void Widget::searchFlightDetails(const QString &fromCode, const QString &toCode, const QString &dateStr)
{
    // 这里需要使用不同的API来获取详细航班信息，包括中转站
    // 由于携程没有公开提供详细API文档，这里提供一个示例实现
    
    // 解析日期为 QDate 对象
    QDate date;
    if (dateStr.length() == 8) {
        int year = dateStr.mid(0, 4).toInt();
        int month = dateStr.mid(4, 2).toInt();
        int day = dateStr.mid(6, 2).toInt();
        date = QDate(year, month, day);
    } else {
        QMessageBox::warning(this, "错误", "无效的日期格式");
        return;
    }
    
    // 构建详细信息URL
    QString dateFormatted = date.toString("yyyy-MM-dd");
    
    QString message = QString("要获取包含中转站的详细航班信息，请访问携程网站:\n\n"
                            "https://flights.ctrip.com/online/list/oneway-%1-%2?depdate=%3\n\n"
                            "目前API不直接提供中转站信息，需要在网页上查看。")
                            .arg(fromCode.toLower())
                            .arg(toCode.toLower())
                            .arg(dateFormatted);
    
    QMessageBox::information(this, "详细航班信息", message);
}

QString Widget::formatDate(const QString &dateStr)
{
    // 将"20250523"格式转换为"2025-05-23"
    if (dateStr.length() == 8) {
        return dateStr.mid(0, 4) + "-" + dateStr.mid(4, 2) + "-" + dateStr.mid(6, 2);
    }
    return dateStr;
}

void Widget::searchTrainPrice(const QString &fromCode, const QString &toCode, const QDate &date)
{
    // 这是一个预留函数，当您获取到火车票API后可以实现
    QString dateStr = date.toString("yyyy-MM-dd");
    resultText->setText(QString("火车票查询功能尚未实现。\n\n预计查询参数：\n出发城市代码: %1\n到达城市代码: %2\n出发日期: %3")
                      .arg(fromCode).arg(toCode).arg(dateStr));
    
    // TODO: 实现火车票API调用
    // 示例代码:
    /*
    QString url = QString("https://train-api-example.com/query?"
                          "from=%1&to=%2&date=%3")
                          .arg(fromCode)
                          .arg(toCode)
                          .arg(dateStr);
    
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0 (Windows NT 10.0; Win64; x64)");
    request.setRawHeader("Accept", "application/json");
    
    qDebug() << "Train Request URL:" << url;
    networkManager->get(request);
    */
}

void Widget::onTransportTypeChanged(int id)
{
    // 根据不同的交通方式调整UI
    switch(id) {
    case 0: // 飞机
        allowTransferCheck->setEnabled(true);
        break;
    case 1: // 火车
        allowTransferCheck->setEnabled(false);
        break;
    case 2: // 综合查询
        allowTransferCheck->setEnabled(true);
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
