#ifndef VELODYNE_DATA_H
#define VELODYNE_DATA_H

/*
* VelodyneData.h
* Represents the data that is passed between the ICD and the user of the lib
* Author: Binyamin Appelbaum
* Date: 10.01.18
* 
*/

#include <vector>
#include <string>

class VelodyneData {
public:  typedef std::vector<std::pair<double, short> > t_channel_data;

private:
    double m_azimuth;
    t_channel_data m_channels;
    float m_simTime;

public:
    VelodyneData() = default;

    VelodyneData(double azimuth, const t_channel_data& channels, float simTime);

    ~VelodyneData() = default;

    double GetAzimuth() const {
        return m_azimuth;
    }
    
    void SetAzimuth(double azimuth) {
        m_azimuth = azimuth;
    }

    const t_channel_data& GetChannels() const {
        return m_channels;
    }

    void SetChannels(const t_channel_data& channels) {
        m_channels = channels;
    }

    float GetSimTime() const {
        return m_simTime;
    }

    void SetSimTime(float simTime) {
        m_simTime = simTime;
    }

     std::string toString() const;
};



#endif // VELODYNE_DATA_H