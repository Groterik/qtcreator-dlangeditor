#include "dlangdebughelper.h"

#include <QDebug>
#include <QElapsedTimer>

#include <limits>

using namespace DlangEditor;

namespace {
class TotalStatistics
{
public:

    ~TotalStatistics() {
        qDebug() << "Total statistics: \n";
        for (StatisticsMap::const_iterator it = counters.begin(); it != counters.end(); ++it) {
            qDebug() << it.key() << ":\tmin = " << it.value().min() << "\tmax = "
                     << it.value().max() << "\tnum = " << it.value().num() << "\tavg = " << it.value().avg();
        }
    }

    void AddElapsedTime(const QString& s, int t) {
        counters[s].addValue(t);
    }

private:
    struct Statistics {
        int m_sum;
        int m_num;
        int m_max;
        int m_min;
        void addValue(int v) {
            m_sum += v;
            if (v > m_max) m_max = v;
            if (v < m_min) m_min = v;
            ++m_num;
        }

        double avg() const {
            return static_cast<double>(m_sum)/m_num;
        }

        int max() const {
            return m_max;
        }

        int min() const {
            return m_num ? m_min : 0;
        }

        int num() const {
            return m_num;
        }

        Statistics() : m_sum(0), m_num(0), m_max(0), m_min(std::numeric_limits<int>::max()) {}
    };

    typedef QMap<QString, Statistics> StatisticsMap;
    StatisticsMap counters;
};

TotalStatistics *staticStats = 0;

TotalStatistics& stat() {
    if (!staticStats) {
        staticStats = new TotalStatistics;
    }
    return *staticStats;
}
}


DlangDebugHelper::DlangDebugHelper(const char *func, const char *str)
    : str(str), func(func)
{
    init();
}

DlangDebugHelper::DlangDebugHelper(const char *func, const QString &str)
    : str(str), func(func)
{
    init();
}

DlangDebugHelper::~DlangDebugHelper()
{
    qint64 t = timer.elapsed();
    qDebug() << "[out] " << func << ' ' << t;
    stat().AddElapsedTime(func, t);
}

void DlangDebugHelper::init()
{
    qDebug() << "[in] " << func << ' ' << str;
    timer.start();
}
