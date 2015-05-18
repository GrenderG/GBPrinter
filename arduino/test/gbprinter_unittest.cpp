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

TEST(gbsendpacket, gbc_initialize) {
	SerialMock* serialMock = serialMockInstance();
	stringCapture c;

	EXPECT_CALL(*serialMock, write(Matcher<uint8_t>(_)))
		.WillRepeatedly(Invoke(&c, &stringCapture::captureUInt8));
	GBSendPacket(GBC_INITIALIZE, 0x0000);
	uint8_t expectedPacket[] = {0x88, 0x33, GBC_INITIALIZE, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00};
	// const char *receivedPacket = c.get().c_str();
	EXPECT_EQ(11u, c.get().length());
	// EXPECT_EQ((uint8_t)c.get().c_str()[0], 0x88);
	releaseSerialMock();
}
