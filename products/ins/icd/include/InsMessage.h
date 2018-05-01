#ifndef INSMESSAGE_H
#define INSMESSAGE_H

/*
* InsMessage.h
* Ins message to send
* Author: Binyamin Appelbaum
* Date: 14.02.18
*/


#include "InsStructs.h"
#include "IMessage.h"
#include <bitset>
#include <stdio.h> // size_t
#include <cmath> // pow

static const int TIME_MULTIPLY = std::pow(2, 12);
static const int VEL_MULTIPLY = std::pow(2, 8);

class InsData;
class ICommunication; // forward declaration

class InsMessage : public IMessage<InsData>{
protected:

	virtual void FillHeader(/* out */ INS_HEADER& header) const = 0;

	void GetDataValidityBitfield(char* buffer, size_t bufferSize) const;

	void GetSystemStatusBitfield(char* buffer, size_t bufferSize) const;

	void GetSystemAlertBitfield(char* buffer, size_t bufferSize) const;

public:
	InsMessage(float hertz);

	virtual ~InsMessage() = default;

	virtual void FillMessage(const InsData& data) = 0;

	virtual int SendMessage(ICommunication* comm) const override;

	// Get ID (basically for toString method of InsData)
	virtual std::bitset<8> GetMsgBitID() const = 0;
};

#endif // INSMESSAGE_H