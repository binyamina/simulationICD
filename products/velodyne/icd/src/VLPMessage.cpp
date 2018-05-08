/*
* VLPMessage.cpp
* VLP message to send
* Author: Binyamin Appelbaum
* Date: 07.03.18
*/

#include "VLPMessage.h"
#include "VLPConfig.h"
#include "LoggerProxy.h"
#include "ICommunication.h"
#include <cstring> // memset, memcpy
#include <boost/range/irange.hpp> // boost::irange

static const int DISTANCE_MULT = 500;
static const int AZIMUTH_MULT = 100;
static const int SECOND_TO_MICROSECOND  = 1e6;

VLPMessage::VLPMessage(float hertz, int returnMode, int dataSource): IMessage(hertz) {
	FillFactory(returnMode, dataSource);
} 

void VLPMessage::FillMessage(const VelodyneData& data) {
	// convert data from 24*16 to 12*32
    auto blocks = CombineBlocks(data.GetBlocks());
    // Fill timestamp of the first
    FillTimeStamp(blocks[0].GetSimTime());
     // for every block: fill azimuth, fill data records
    for (const auto& block : blocks) {
        FillAzimuth(block.GetAzimuth());
        FillChannels(block.GetChannels());
        m_packetIndex++;
    }
}

std::vector<VelodyneData::VelodyneBlock> VLPMessage::CombineBlocks(const std::vector<VelodyneData::VelodyneBlock>& origBlocks) const {
    std::vector<VelodyneData::VelodyneBlock> combinedBlocks;
    VelodyneData::VelodyneBlock currBlock;
    for (int i = 0; i < origBlocks.size(); i++) {
        if ((i % 2) == 0) {
            currBlock = origBlocks[i];
        }
        else {
            auto channels = MapChannels(currBlock.GetChannels());
            auto additionalChannels = MapChannels(origBlocks[i].GetChannels());
            channels.insert(channels.end(), additionalChannels.begin(), additionalChannels.end());
            currBlock.SetChannels(channels);
            combinedBlocks.push_back(currBlock);
            currBlock = VelodyneData::VelodyneBlock();
        }
    }
    return combinedBlocks;
}

int VLPMessage::GetMessageSize() const {
	return sizeof(VLPDataPacket);
}

int VLPMessage::SendMessage(ICommunication* comm) const {
	char buf[sizeof(VLPDataPacket)]{};
    memcpy(buf, &m_packet, sizeof(m_packet));
    return comm->SendData(buf, sizeof(buf));
}

void VLPMessage::InitMessage() {
    m_packetIndex = 0;
	std::fill_n(m_packet.dataBlocks, NUM_OF_VLP_DATA_BLOCKS, VLPDataPacket::VLPDataBlock());
    std::fill_n(m_packet.timeStamp, 4, 0);
}

void VLPMessage::FillFactory(int returnMode, int dataSource) {
    m_packet.factory[0] = returnMode;
    m_packet.factory[1] = dataSource;    
}

void VLPMessage::FillTimeStamp(float timeStamp) {
    // TODO check the exact time stamp to send
    unsigned long microseconds = timeStamp * 1e6; 
    ToByteArray((unsigned long)microseconds, m_packet.timeStamp, sizeof(m_packet.timeStamp));
}

void VLPMessage::FillAzimuth(float azimuth) {
    // convert the angle * 100 (in order to save double information) to array on the suitable block of the packet
    ToByteArray((unsigned int) (azimuth * AZIMUTH_MULT),
        m_packet.dataBlocks[m_packetIndex].azimuth, sizeof(m_packet.dataBlocks[m_packetIndex].azimuth));
}

void VLPMessage::FillChannels(const VelodyneData::t_channel_data& channels) {
    for (auto i : boost::irange<size_t>(0, channels.size())) {
         // convert the distance * 500 (in order to save double information) to array on the suitable block of the packet
        ToByteArray((unsigned int)(channels[i].first * DISTANCE_MULT), 
            m_packet.dataBlocks[m_packetIndex].dataRecords[i].distance, sizeof(m_packet.dataBlocks[m_packetIndex].dataRecords[i].distance));
    }
}

VelodyneData::t_channel_data VLPMessage::MapChannels(const VelodyneData::t_channel_data& channels) const {
    int channelsSize = channels.size();
    VelodyneData::t_channel_data newChannels(channelsSize);
    // put the size/2 first elements in the correspoindig 2*i indexes on the new vector (0 -> 0, 1 -> 2, 2 -> 4, etc)
    for (auto i : boost::irange(0,channelsSize / 2)) {
        newChannels[i*2] = channels[i];
    }
    // fill the rest indexes of the new vector with the last size/2 elements of the original vector ( 1 -> 8, 3 -> 9, 5 -> 10, etc)
    for (int i = 1, j = channelsSize / 2; i < channelsSize; i += 2, j++) {
        newChannels[i] = channels[j];
    }
    return newChannels;
}

template <typename T>
bool VLPMessage::ToByteArray(T num, unsigned char* ret, size_t size) const {
    if (ret == nullptr) {
        ERRLOG << "nullptr\n";
        return false;
    }
    // drop the right-most bytes and convert to new right most byte. after the loop - the bytes will be reversed
    for (int i = 0; i < size; i++) {
        ret[i] = (int)((num >> (8*i)) & 0xFF);
    }
    return true;
}

template <typename Func>
double VLPMessage::FormatBlock(const unsigned char* arr, size_t size, Func func) const {
    long num = 0;
    for (int i = 0; i < size; i++) {
        num += ((long)arr[i] << i*8);
    }
    return func(num);
}

void VLPMessage::printPacketData() const {
    auto azimuthFunc = [](double num) -> double {return (double)num/AZIMUTH_MULT; };
    auto distanceFunc = [](double num) -> double {return (double)num/DISTANCE_MULT; };
    auto tsFunc = [](double num) -> double {return (double)num/SECOND_TO_MICROSECOND; };
    auto defFunc = [](double num) -> double {return num; };

    for (auto const& block : m_packet.dataBlocks) {
        LOG << "Azimuth: " << FormatBlock(block.azimuth, sizeof(block.azimuth), azimuthFunc) << "\n";
        int i = 0;
        for (auto const& channel : block.dataRecords) {
            std::stringstream ss;
            ss << "  Distance " << i++ << ": "  << FormatBlock(channel.distance, sizeof(channel.distance), distanceFunc);
            // ss << "  Reflectivity: " << channel.reflectivity;
            LOG << ss.str() << "\n";
        }
        LOG << "\n";
    }
    DBGLOG << "Return mode: " << VLPConfig::retModeToStr.find((VLPConfig::ReturnMode)m_packet.factory[0])->second << "\n";
    DBGLOG << "Data source: " << VLPConfig::dataSourceToStr.find((VLPConfig::DataSource)m_packet.factory[1])->second << "\n";
    LOG << "*********** Simulation time: " << FormatBlock(m_packet.timeStamp, sizeof(m_packet.timeStamp), tsFunc) << " *********************\n";
}
