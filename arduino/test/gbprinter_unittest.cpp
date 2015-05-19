// Copyright 2014 http://switchdevice.com
// This example code is in the public domain.

#include "gtest/gtest.h"
#include "arduino-mock/Arduino.h"
#include "arduino-mock/Serial.h"
#include "arduino-mock/serialHelper.h"

#include "src/gbprinter.h"

using ::testing::_;
using ::testing::Return;
using ::testing::Matcher;
using ::testing::AtLeast;
using ::testing::Invoke;

TEST(cbuffer, initialization) {
	EXPECT_EQ(0u, CBUFFER.start);
	EXPECT_EQ(0u, CBUFFER.end);
}

TEST(cbuffer, cbread) {
	CBUFFER.buffer[0] = 0x01;
	CBUFFER.buffer[1] = 0x02;
	// CBRead reads properly from buffer and advances start pointer
	EXPECT_EQ(0x01, CBRead());
	EXPECT_EQ(0x02, CBRead());
	// CBRead loops over at the end of the buffer
	CBUFFER.start = BUFFER_SIZE - 1;
	CBRead();
	EXPECT_EQ(0u, CBUFFER.start);
}

TEST(cbuffer, cbwrite) {
	// Check that write actually does it and advances buffer accordingly
	CBWrite(0x02);
	CBWrite(0x03);
	EXPECT_EQ(0x02, CBUFFER.buffer[0]);
	EXPECT_EQ(0x03, CBUFFER.buffer[1]);
	// Check that end loops over at the end of the bugger
	CBUFFER.end = BUFFER_SIZE -1;
	CBWrite(0x03);
	CBWrite(0x03);
	EXPECT_EQ(0x03, CBUFFER.buffer[BUFFER_SIZE - 1]);
	EXPECT_EQ(0x03, CBUFFER.buffer[0]);
}

TEST(gbsendpacket, packetsize) {
	SerialMock* serialMock = serialMockInstance();
	stringCapture c;

	EXPECT_CALL(*serialMock, write(Matcher<uint8_t>(_)))
		.WillRepeatedly(Invoke(&c, &stringCapture::captureUInt8));
	
	GBSendPacket(GBC_INITIALIZE, 0x0000);
	EXPECT_EQ(10u, c.get().length());
	c.clear();

	GBSendPacket(GBC_TRANSFER, 0x0008);
	EXPECT_EQ(18u, c.get().length());
	c.clear();

	GBSendPacket(GBC_TRANSFER, 0x0280);
	EXPECT_EQ(650u, c.get().length());
	c.clear();

	releaseSerialMock();
}

TEST(gbsendpacket, GBC_INITIALIZE) {
	SerialMock* serialMock = serialMockInstance();
	stringCapture c;

	EXPECT_CALL(*serialMock, write(Matcher<uint8_t>(_)))
		.WillRepeatedly(Invoke(&c, &stringCapture::captureUInt8));
	GBSendPacket(GBC_INITIALIZE, 0x0000);
	const char *receivedPacket = c.get().c_str();
	// magic
	EXPECT_EQ(0x88, (uint8_t) receivedPacket[0]);
	EXPECT_EQ(0x33, (uint8_t) receivedPacket[1]);
	// command
	EXPECT_EQ(GBC_INITIALIZE, (uint8_t) receivedPacket[2]);
	// compression
	EXPECT_EQ(0x00, (uint8_t) receivedPacket[3]);
	// size
	EXPECT_EQ(0x00, (uint8_t) receivedPacket[4]);
	EXPECT_EQ(0x00, (uint8_t) receivedPacket[5]);
	// checksum
	EXPECT_EQ(0x01, (uint8_t) receivedPacket[6]);
	EXPECT_EQ(0x00, (uint8_t) receivedPacket[7]);
	// gbp status
	EXPECT_EQ(0x00, (uint8_t) receivedPacket[8]);
	EXPECT_EQ(0x00, (uint8_t) receivedPacket[9]);
	releaseSerialMock();
}

TEST(gbsendpacket, GBC_TRANSFER) {
	SerialMock* serialMock = serialMockInstance();
	stringCapture c;
	CBInit();
	uint16_t dataSize = 4;
	for (unsigned int i = 0; i < dataSize; ++i)
	{
		CBUFFER.buffer[i] = 0x02;
	}

	EXPECT_CALL(*serialMock, write(Matcher<uint8_t>(_)))
		.WillRepeatedly(Invoke(&c, &stringCapture::captureUInt8));
	GBSendPacket(GBC_TRANSFER, dataSize);
	const char *receivedPacket = c.get().c_str();
	// magic
	EXPECT_EQ(0x88, (uint8_t) receivedPacket[0]);
	EXPECT_EQ(0x33, (uint8_t) receivedPacket[1]);
	// command
	EXPECT_EQ(GBC_TRANSFER, (uint8_t) receivedPacket[2]);
	// compression
	EXPECT_EQ(0x00, (uint8_t) receivedPacket[3]);
	// size
	EXPECT_EQ(0x04, (uint8_t) receivedPacket[4]);
	EXPECT_EQ(0x00, (uint8_t) receivedPacket[5]);
	// data (dataSize bytes)
	EXPECT_EQ(0x02, (uint8_t) receivedPacket[6]);
	EXPECT_EQ(0x02, (uint8_t) receivedPacket[7]);
	EXPECT_EQ(0x02, (uint8_t) receivedPacket[8]);
	EXPECT_EQ(0x02, (uint8_t) receivedPacket[9]);
	// checksum
	EXPECT_EQ(0x10, (uint8_t) receivedPacket[10]);
	EXPECT_EQ(0x00, (uint8_t) receivedPacket[11]);
	// gbp status
	EXPECT_EQ(0x00, (uint8_t) receivedPacket[12]);
	EXPECT_EQ(0x00, (uint8_t) receivedPacket[13]);
	
	releaseSerialMock();
}
