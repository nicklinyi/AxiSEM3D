// XTimer.cpp
// created by Kuangdai on 5-Nov-2016 
// Recursive timer

#include "XTimer.h"
#include "XMPI.h"

std::string XTimer::mFileName;
std::fstream XTimer::mFile;
std::vector<MyBoostTimer> XTimer::mTimers;
bool XTimer::mEnabled = false;

void XTimer::initialize(const std::string &fileName, size_t nLevels) {
    mEnabled = false;
    if (XMPI::root()) {
        mFileName = fileName;
        mTimers.clear();
        for (size_t i = 0; i < nLevels; i++) {
            mTimers.push_back(MyBoostTimer());
        }
    }
}

void XTimer::openFile() {
    if (XMPI::root()) {
        if (!mFile.is_open()) {
            mFile.open(mFileName, std::fstream::out);
        }
    }
}

void XTimer::finalize() {
    if (XMPI::root()) {
        if (mFile.is_open()) {
            mFile.close();
        }
    }
}

void XTimer::begin(const std::string &name, size_t level, bool barrier) {
    if (!mEnabled) {
        return;
    }
    if (barrier) {
        XMPI::barrier();
    }
    if (XMPI::root()) {
        for (size_t i = 0; i < level; i++) {
            mFile << "    ";
        }
        mFile << name << " begins..." << std::endl;
        mTimers[level].start();
    }
}

void XTimer::end(const std::string &name, size_t level, bool barrier) {
    if (!mEnabled) {
        return;
    }
    if (barrier) {
        XMPI::barrier();
    }
    if (XMPI::root()) {
        mTimers[level].stop();
        for (size_t i = 0; i < level; i++) {
            mFile << "    ";
        }
        mFile << name << " finishes. Elapsed seconds = " 
            << mTimers[level].elapsed() << std::endl;
    }
}

void XTimer::pause(size_t level) {
    if (!mEnabled) {
        return;
    }
    if (XMPI::root()) {
        mTimers[level].stop();
    }
}

void XTimer::resume(size_t level) {
    if (!mEnabled) {
        return;
    }
    if (XMPI::root()) {
        mTimers[level].resume();
    }
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
void MyBoostTimer::stop() {
    if (running()) {
        mTimePoints.push_back(std::chrono::high_resolution_clock::now());
    }
}

void MyBoostTimer::resume() {
    if (!running()) {
        mTimePoints.push_back(std::chrono::high_resolution_clock::now());
    }
}

void MyBoostTimer::clear() {
    mTimePoints.clear();
}

double MyBoostTimer::elapsed() {
    double elap = 0.;
    size_t pairs = mTimePoints.size() / 2;
    for (size_t i = 0; i < pairs; i++) {
        elap += (std::chrono::duration_cast<std::chrono::duration<double>>
            (mTimePoints[2 * i + 1] - mTimePoints[2 * i])).count();
    }
    if (running()) {
        elap += (std::chrono::duration_cast<std::chrono::duration<double>>
            (std::chrono::high_resolution_clock::now() - 
            mTimePoints[mTimePoints.size() - 1])).count();
    }
    return elap;
}

double MyBoostTimer::getClockResolution() {
    MyBoostTimer cpu;
    std::chrono::high_resolution_clock::time_point start_time, current_time;
    start_time = std::chrono::high_resolution_clock::now();
    current_time = start_time;
    while (current_time == start_time) {
        current_time = std::chrono::high_resolution_clock::now();
    }
    return (std::chrono::duration_cast<std::chrono::duration<double>>
        (current_time - start_time)).count();
}
