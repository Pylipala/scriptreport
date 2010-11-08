#include "scriptreport.h"

#include <QtCore/QTextStream>
#include <QtGui/QTextDocument>
#include <QtGui/QPainter>
#include <QtGui/QPrinter>
#include <QtScript/QScriptEngine>

#include "sourcetransformer.h"
#include "textstreamobject.h"
#include "scriptable/scriptablereport.h"
#include "scriptable/scriptableengine.h"

#include "scriptreportengine.h"

ScriptReport::ScriptReport(QString scriptName, QObject *parent) :
        QObject(parent),
        m_isPrintErrorEnabled(true),
        m_isRunRequired(true),
        m_isUpdateIntermediateCodeRequired(true),
        m_isInitialized(false),
        m_isInEditingMode(false),
        m_isInDebuggingMode(false),
        m_name(scriptName),
        m_isWriteWithPrintFunctionTooEnabled(false),
        m_scriptableReport(0),
        m_scriptableEngine(0),
        m_scriptReportEngine(0)
{
    construct();
}

ScriptReport::ScriptReport(QTextStream *inputStream, QString scriptName, QObject *parent) :
        QObject(parent),
        m_isPrintErrorEnabled(true),
        m_isRunRequired(true),
        m_isUpdateIntermediateCodeRequired(true),
        m_isInitialized(false),
        m_isInEditingMode(false),
        m_isInDebuggingMode(false),
        m_name(scriptName),
        m_isWriteWithPrintFunctionTooEnabled(false),
        m_scriptableReport(0),
        m_scriptableEngine(0),
        m_scriptReportEngine(0)
{
    construct();
    m_inStreamObject->setStream(inputStream);
}

ScriptReport::ScriptReport(QString input, QString scriptName, QObject *parent) :
        QObject(parent),
        m_isPrintErrorEnabled(true),
        m_isRunRequired(true),
        m_isUpdateIntermediateCodeRequired(true),
        m_isInitialized(false),
        m_isInEditingMode(false),
        m_isInDebuggingMode(false),
        m_name(scriptName),
        m_isWriteWithPrintFunctionTooEnabled(false),
        m_scriptableReport(0),
        m_scriptableEngine(0),
        m_scriptReportEngine(0)
{
    construct();
    m_inStreamObject->setText(input);
}

void ScriptReport::construct() {
    m_engine = new QScriptEngine;

    m_inStreamObject = new TextStreamObject(
            QString::fromLatin1("Input"),
            QIODevice::ReadOnly,
            m_engine);

    m_outHeaderStreamObject = new TextStreamObject(
            QString::fromLatin1("Header"),
            QIODevice::WriteOnly,
            m_engine);

    m_outHeaderFirstStreamObject = new TextStreamObject(
            QString::fromLatin1("HeaderFirst"),
            QIODevice::WriteOnly,
            m_engine);

    m_outHeaderLastStreamObject = new TextStreamObject(
            QString::fromLatin1("HeaderLast"),
            QIODevice::WriteOnly,
            m_engine);

    m_outStreamObject = new TextStreamObject(
            QString::fromLatin1("Content"),
            QIODevice::WriteOnly,
            m_engine);

    m_outFooterStreamObject = new TextStreamObject(
            QString::fromLatin1("Footer"),
            QIODevice::WriteOnly,
            m_engine);

    m_outFooterFirstStreamObject = new TextStreamObject(
            QString::fromLatin1("FooterFirst"),
            QIODevice::WriteOnly,
            m_engine);

    m_outFooterLastStreamObject = new TextStreamObject(
            QString::fromLatin1("FooterLast"),
            QIODevice::WriteOnly,
            m_engine);

    m_printStreamObject = new TextStreamObject(
            QString::fromLatin1("Print"),
            QIODevice::WriteOnly,
            m_engine);
}

ScriptReport::~ScriptReport() {
    delete m_engine;
    delete m_scriptReportEngine;
}

bool ScriptReport::isEditing() const {
    return m_isInEditingMode;
}

void ScriptReport::setEditing(bool editing) {
    m_isInEditingMode = editing;
}

bool ScriptReport::isFinal() const {
    return !m_isInEditingMode;
}

void ScriptReport::setFinal(bool final) {
    m_isInEditingMode = !final;
}

bool ScriptReport::isDebugging() const {
    return m_isInDebuggingMode;
}

void ScriptReport::setDebugging(bool debugging) {
    m_isInDebuggingMode = debugging;
}

bool ScriptReport::isPrintErrorEnabled() const {
    return m_isPrintErrorEnabled;
}

void ScriptReport::setPrintErrorEnabled(bool isPrintErrorEnabled) {
    m_isPrintErrorEnabled = isPrintErrorEnabled;
}

QStringList ScriptReport::arguments() const {
    if (!m_scriptableEngine) {
        return m_scriptableEngine->arguments();
    } else {
        return QStringList();
    }
}

void ScriptReport::setArguments(QStringList arguments) {
    if (!m_scriptableEngine) {
        m_scriptableEngine = new ScriptableEngine(m_engine);
    }
    m_scriptableEngine->setArguments(arguments);
}

QString ScriptReport::previousScript() const {
    return m_previousScript;
}

void ScriptReport::setPreviousScript(QString previousScript) {
    m_previousScript = previousScript;
}

QString ScriptReport::scriptName() const {
    return m_name;
}

void ScriptReport::setScriptName(QString scriptName) {
    m_name = scriptName;
}

bool ScriptReport::isWriteWithPrintFunctionTooEnabled() const {
    return m_isWriteWithPrintFunctionTooEnabled;
}

void ScriptReport::setWriteWithPrintFunctionTooEnabled(bool isWriteWithPrintFunctionTooEnabled) {
    m_isWriteWithPrintFunctionTooEnabled = isWriteWithPrintFunctionTooEnabled;
}

TextStreamObject* ScriptReport::input() const {
    return m_inStreamObject;
}

TextStreamObject* ScriptReport::outputHeader() const {
    return m_outHeaderStreamObject;
}

TextStreamObject* ScriptReport::outputHeaderFirst() const {
    return m_outHeaderFirstStreamObject;
}

TextStreamObject* ScriptReport::outputHeaderLast() const {
    return m_outHeaderLastStreamObject;
}

TextStreamObject* ScriptReport::output() const {
    return m_outStreamObject;
}

TextStreamObject* ScriptReport::outputFooter() const {
    return m_outFooterStreamObject;
}

TextStreamObject* ScriptReport::outputFooterFirst() const {
    return m_outFooterFirstStreamObject;
}

TextStreamObject* ScriptReport::outputFooterLast() const {
    return m_outFooterLastStreamObject;
}

TextStreamObject* ScriptReport::printOutput() const {
    return m_printStreamObject;
}

QScriptEngine* ScriptReport::scriptEngine() /*const*/ {
    if (!m_isInitialized) {
        m_isInitialized = true; // after for prevent an indirect recursive call
        initEngine();
    }
    return m_engine;
}

//void ScriptReport::setScriptEngine(QScriptEngine *scriptEngine) {
//    if (m_engine == scriptEngine) {
//        return;
//    }
//    m_engine = scriptEngine;
//    initEngine(*m_engine);
//}

ScriptReportEngine* ScriptReport::scriptReportEngine() /*const*/ {
    if (!m_scriptReportEngine) {
        initScriptReportEngine();
    }
    return m_scriptReportEngine;
}

QString ScriptReport::intermediateCode() const {
    return m_intermediate;
}

bool ScriptReport::hasUncaughtException() const {
    if (!m_engine) {
        return false;
    }
    return m_engine->hasUncaughtException();
}

QString ScriptReport::errorMessage() const {
    QString message;
    if (!m_engine) {
        return message;
    }
    if (m_engine->hasUncaughtException()) {
        QScriptValue exception = m_engine->uncaughtException();
        message = QString::fromLatin1("Line: %1, Uncaught exception: %2.")
                  .arg(m_engine->uncaughtExceptionLineNumber())
                  .arg(exception.toString());
        QStringList backtrace = m_engine->uncaughtExceptionBacktrace();
        foreach (QString b, backtrace) {
            message.append(QString::fromLatin1("\n    at %1").arg(b));
        }
    }
    return message;
}

void ScriptReport::updateIntermediateCode() {
    m_intermediate.clear();
    QTextStream intermediateStream(&m_intermediate, QIODevice::WriteOnly);

    SourceTransformer st(m_inStreamObject->stream(), &intermediateStream);
    st.transform();
    m_isUpdateIntermediateCodeRequired = false;
    m_isRunRequired = true;
}

void ScriptReport::run() {
    if (!m_isInitialized) {
        m_isInitialized = true; // after for prevent an indirect recursive call to scriptEngine()
        initEngine();
    }

    if (m_isUpdateIntermediateCodeRequired) {
        updateIntermediateCode();
    }

    if (!m_previousScript.isEmpty()) {
        m_engine->evaluate(m_previousScript, QString::fromLatin1("previousScript"));
    }
    m_engine->evaluate(m_intermediate, m_name);

    m_outHeaderStreamObject->stream()->flush();
    m_outStreamObject->stream()->flush();
    m_outFooterStreamObject->stream()->flush();
    m_printStreamObject->stream()->flush();

    m_isRunRequired = false;
}

void ScriptReport::print(QPrinter *printer) {
    if (!printer || !printer->isValid()) {
        return;
    }

    if (m_isRunRequired) {
        run();
    }

    m_scriptableReport->applyConfigurationTo(*printer);
    m_scriptReportEngine->print(this, printer);
}

void ScriptReport::loadPrintConfiguration(QPrinter *printer) {
    if (!printer || !printer->isValid()) {
        return;
    }
    if (!m_scriptableReport) {
        m_scriptableReport = new ScriptableReport(this, m_engine);
    }
    if (!m_scriptableEngine) {
        m_scriptableEngine = new ScriptableEngine(m_engine);
    }
    if (!m_scriptReportEngine) {
        initScriptReportEngine();
    }
    m_scriptableReport->loadConfigurationFrom(*printer);
    m_scriptReportEngine->loadPrintConfiguration(this, printer);
}

void ScriptReport::initEngine() {
    if (!m_scriptReportEngine) {
        initScriptReportEngine();
    }
    if (!m_scriptableReport) {
        QPrinter printer;
        loadPrintConfiguration(&printer);
    }
    m_scriptableReport->initEngine(*m_engine);

    QScriptValue sr = m_engine->newObject();
    m_engine->globalObject().setProperty(QString::fromLatin1("sr"), sr, QScriptValue::Undeletable);

    QScriptValue report = m_engine->newQObject(m_scriptableReport, QScriptEngine::QtOwnership, QScriptEngine::ExcludeChildObjects | QScriptEngine::ExcludeSuperClassContents | QScriptEngine::ExcludeDeleteLater);
    sr.setProperty(QString::fromLatin1("report"), report, QScriptValue::Undeletable);

    QScriptValue engine = m_engine->newQObject(m_scriptableEngine, QScriptEngine::QtOwnership, QScriptEngine::ExcludeChildObjects | QScriptEngine::ExcludeSuperClassContents | QScriptEngine::ExcludeDeleteLater);
    sr.setProperty(QString::fromLatin1("engine"), engine, QScriptValue::Undeletable);

    m_scriptReportEngine->initEngine(this, m_engine);
}

void ScriptReport::initScriptReportEngine() {
    m_scriptReportEngine = new ScriptReportEngine();
}
