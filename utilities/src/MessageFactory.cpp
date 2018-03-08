/*
* MessageFactory.cpp
* 
* Author: Binyamin Appelbaum
* Date: 28.02.18
*/

#include "MessageFactory.h"
#include "StatusMessage.h"
#include "NavigationDataMessage.h"
#include "InternalGPSMessage.h"
#include "ErrorsEstimationMessage.h"
#include "EchoMessage.h"
#include "BestVelMessage.h"
#include "BestPosMessage.h"

IMessage<InsData>* MessageFactory::CreateMessage(InsMsgType msgType, int hertz) {
	IMessage<InsData>* msg = nullptr;
	switch (msgType) {
		case _STATUS_MSG_: {
			msg = new StatusMessage(hertz);
			break;
		}
		case _NAVIGATION_DATA_MSG_: {
			msg = new NavigationDataMessage(hertz);
			break;
		}
		case _INTERNAL_GPS_MSG: {
			msg = new InternalGPSMessage(hertz);
			break;
		}
		case _ERROR_ESTIMATION_MSG: {
			msg = new ErrorsEstimationMessage(hertz);
			break;
		}
		case _ECHO_MSG_: {
			msg = new EchoMessage(hertz);
			break;
		}
		default:
			break;
	}
	return msg;
}

IMessage<DgpsData>* MessageFactory::CreateMessage(DgpsMsgType msgType, int hertz) {
	IMessage<DgpsData>* msg = nullptr;
	switch (msgType) {
		case _BEST_POS_: {
			msg = new BestPosMessage(hertz);
			break;
		}
		case _BEST_VEL_: {
			msg = new BestVelMessage(hertz);
			break;
		}
		default:
			break;
	}
	return msg;
}